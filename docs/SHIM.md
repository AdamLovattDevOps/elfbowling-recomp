# Win32 shim (`port/`)

`port/` is a small Win32 subset on SDL2 for the native build. It covers only the `imp` rows of `notes/port_deps.tsv`, with the semantics the game code in `src_match/` relies on. It is not Wine.

| Path | Contents |
|---|---|
| `port/include/windows.h`, `mmsystem.h` | types, constants and prototypes (C and C++) |
| `port/include/port.h` | port-only entry points: `port_init`, `port_present`, `port_screen_dump_bmp`, `port_res_open`, `port_log`, `port_shutdown` |
| `port/src/gdi.c` | GDI and USER: DCs, DIB sections, palettes, BitBlt, FillRect, fonts and text, the screen |
| `port/src/winmm.c` | waveOut on SDL audio, `mmioStringToFOURCCA` |
| `port/src/res.c` | PE resource reader over the user's `Elf Bowling.exe` |
| `port/src/kernel.c` | kernel, registry and shell calls; logging; SDL setup |
| `port/tests/` | `shim_test.c` (headless end-to-end test), `check_shim.py` (independent Python check), `header_cxx.cpp` (C++ compile check) |

Build and test (SDL2 and SDL2_ttf from `sdl2-config`; system zlib for the test only):

```
make -C port test            # EXE="path/to/Elf Bowling.exe" to override
```

The test runs headless with `SDL_VIDEODRIVER=dummy` and `SDL_AUDIODRIVER=disk`. Outputs:

- `build/port_test.bmp`: the presented screen.
- `build/port/mix.raw`: the shim's mixer output.
- `build/port/audio.raw`: the SDL disk driver's file.
- `build/port/report.txt`: the facts the Python check reads.

## Types

- Win32 widths on every host:
  - `DWORD`, `LONG`, `UINT` and `BOOL` are 32-bit. `LONG` is `int32_t`, not `long`.
  - `LPARAM`, `WPARAM`, `DWORD_PTR`, `UINT_PTR` and `LONG_PTR` are pointer-sized.
- Handles are distinct pointer types (`DECLARE_HANDLE`, as with `STRICT`). `HGDIOBJ` and `HANDLE` are `void *`.
- Byte-exact file and ABI structs: `BITMAPINFOHEADER`, `BITMAPFILEHEADER` (pack 2), `RGBQUAD`, `PALETTEENTRY`, `LOGFONTA`, `ENUMLOGFONTA` and `WAVEFORMATEX` (pack 1, 18 bytes, as the Windows headers declare it).
- `MEMORYSTATUS` keeps the 32-byte Win32 layout with `DWORD` fields, because the game sets `dwLength = 32`.
- `WAVEHDR` has the 64-bit layout. The `cbwh` size arguments are ignored, because the game passes the 32-bit `0x20`.
- `windows.h` includes `port.h`.

## GDI

**Objects.** Every GDI handle points to a small tagged struct: bitmap, palette, font or brush.

- Stock objects are static and `DeleteObject` ignores them. The stock objects are the `WHITE`/`LTGRAY`/`GRAY`/`DKGRAY`/`BLACK`/`NULL` brushes, `SYSTEM_FONT` and `DEFAULT_PALETTE` (the 20 static colours).
- `SelectObject` returns the previous object of the same kind.
  - A bitmap can be selected into only one memory DC at a time.
  - Palettes must go through `SelectPalette`, as on Windows.
- `DeleteObject` on an object still selected into a DC fails and logs. It does not free the object.

**DCs.**

| Call | Result |
|---|---|
| `CreateCompatibleDC` | memory DC holding the stock 1×1 bitmap |
| `GetDC(any HWND)` | a fresh DC on the one screen, with default state; `ReleaseDC` frees it |
| `CreateICA("DISPLAY")` | an information-only DC |

Default DC state: text colour black, background colour white, `OPAQUE`, the default palette, `SYSTEM_FONT` and `WHITE_BRUSH`.

**DIB sections.** `CreateDIBSection` supports `BI_RGB` at 8, 24 and 32 bpp, in both orientations (`biHeight > 0` is bottom-up, `< 0` is top-down).

- Rows are padded to 4 bytes and the pixels start zeroed.
- The 8-bit colour table (`biClrUsed`, or 256 when it is 0) is **copied** at creation, as GDI does. Later edits to the caller's `BITMAPINFO` do not reach existing DIBs.
- The game creates all its DIBs from one shared `BITMAPINFO` (`g_45552c`) and writes the bits directly, bottom-up.
- Not supported (logged, returns NULL): `DIB_PAL_COLORS`, file-mapping sections, 1/4/16 bpp and RLE.

**The screen.** The screen is one 8-bit palettized surface of the `port_init` size (default 640×480), top-down.

- `GetDeviceCaps` reports `BITSPIXEL` 8 and `PLANES` 1, which is what `fn_40f3dc` checks. It also reports `SIZEPALETTE` 256, `RASTERCAPS` with `RC_PALETTE`, and `HORZRES`/`VERTRES` as the screen size.
- `port_present()` converts the screen through the **system palette** into an ARGB8888 SDL texture and presents it, or presents the 3x canvas (docs/HIRES.md). Without a renderer it blits to the window surface. With no window (headless) it only converts.
- The window is resizable. The renderer's logical size letterboxes the picture to the screen's aspect and maps mouse events back to screen coordinates. Alt+Enter or F11 toggles full screen (desktop mode); F9 switches between the 3x art and the classic frame.
- A game controller plays the game through the same input path (vcl/forms.cpp): A is Space plus a left click at a pointer, Start is Return, and B or Back is Esc. The sticks and d-pad move the pointer, which `port_present()` draws over the frame while a controller drives it (`port_set_pointer_source`). The events go through `dispatch()`, so the game sees ordinary key and mouse input.

**Palettes.**

- `CreatePalette` copies up to 256 entries.
- `SelectPalette` records the palette and the `bForceBackground` flag.
- `RealizePalette` on a DC that holds a foreground, non-stock palette writes the entries into the system palette **1:1**. It does not reserve the 20 static colours, so the game's 256-colour palette shows exactly.
- The game does `SelectPalette(screen, pal, FALSE)`, `RealizePalette`, blit, then `SelectPalette(old, TRUE)`. The system palette keeps the game palette after that last call, as on a real 8-bit display.
- Before any realization, the system palette holds the 20 static colours at 0–9 and 246–255, and black elsewhere.

**BitBlt.** Supports `SRCCOPY`, `SRCAND` and `SRCPAINT`, which are the ones `src_match` uses (engine_40b3b0 and engine_40b8fc). It also supports `SRCINVERT`, `NOTSRCCOPY`, `BLACKNESS` and `WHITENESS`.

- It clips to both surfaces.
- The source is converted to the destination format first, then the ROP is applied byte-wise:

| Source → destination | Conversion |
|---|---|
| 8-bit → 8-bit | indices are copied unchanged (identity mapping) |
| 8-bit → 24/32-bit | through the source colour table |
| 24/32-bit → 8-bit | nearest colour in the destination's table |
| 24/32-bit → 24/32-bit | straight copy |

- 8-bit → 8-bit is what a palette device does when the DIB colour table equals the realized palette. The game guarantees that equality, because all its DIBs use the palette's colours.
- `SRCAND`/`SRCPAINT` therefore act on palette indices. The game's masks depend on this: index 0 is black and 0xFF is white in its palettes.

**Nearest colour** (FillRect brushes, text colours, RGB → 8-bit): an exact match wins; otherwise the least squared RGB distance, with ties going to the lowest index.

**FillRect** fills with the brush colour mapped to the target. A `(HBRUSH)(COLOR_x + 1)` system-colour brush draws grey and logs.

## Text

The game creates seven fonts in `TStage::TStage` (engine_40a978):

- Arial 16/700, 44/700, 14/700, 16/200 and 20/700.
- Lucida Handwriting 19/600.
- Lucida Calligraphy 19/600.

The game finds each face through `EnumFontFamiliesA`. Its callback `fn_4035f4` accepts:

- Lucida Handwriting or Calligraphy only as `elfFullName` "Lucida … Italic" with `elfStyle` "Italic";
- any other face only with `elfStyle` "Regular".

If a face is not enumerated, no font is created and later font indices are off by one. `EnumFontFamiliesA` therefore enumerates the known faces synthetically:

| Face | Styles enumerated |
|---|---|
| Arial | Regular, Bold, Italic, Bold Italic |
| Lucida Handwriting / Calligraphy | Italic only, with `lfItalic = 1` |

A known face is always enumerated, even when no font file is present. Unknown faces are not enumerated. The callback receives a real `ENUMLOGFONTA` and a plausible `TEXTMETRICA`.

**Font files.** The file is chosen when the font is first used. Directories are searched in this order:

1. `$PORT_FONT_DIR`
2. `/System/Library/Fonts/Supplemental`, `/System/Library/Fonts`, `/Library/Fonts`, `~/Library/Fonts`
3. The common Linux font directories
4. `C:/Windows/Fonts`

| Face | Files tried, in order |
|---|---|
| Arial | `Arial.ttf`/`arial.ttf`/Liberation Sans/DejaVu Sans; bold: `Arial Bold.ttf`/`arialbd.ttf`/… |
| Lucida Handwriting | `LHANDW.TTF`, then Apple Chancery, then URW Chancery |
| Lucida Calligraphy | `LCALLIG.TTF`, then Apple Chancery, then URW Chancery |
| anything else | Arial's list |

- On this Mac, Apple Chancery stands in for both Lucida faces.
- A weight of 600 or more uses the bold file when there is one; otherwise SDL_ttf synthesizes bold.

**Size.** A positive `lfHeight` is the **cell** height (ascent + descent), as in GDI. The font is opened at that point size and rescaled so that `TTF_FontHeight` equals it. A negative `lfHeight` is used directly as the em size.

**DrawTextA** handles:

- `DT_LEFT`/`DT_CENTER`/`DT_RIGHT` and `DT_WORDBREAK` (greedy breaks at spaces; a word wider than the rectangle stays whole);
- hard breaks at `\n` (a `\r` is skipped);
- `DT_SINGLELINE` with `DT_VCENTER`/`DT_BOTTOM`;
- `DT_NOCLIP`, `DT_CALCRECT`, and `&` prefix stripping unless `DT_NOPREFIX` is set.

It returns the text height: lines × `TTF_FontHeight`.

- Glyphs are drawn **without antialiasing** (`TTF_RenderText_Solid`), because Windows does not smooth text on 8-bit surfaces. The only colours drawn are the text colour and, with `OPAQUE`, the background colour. Both are mapped to the DIB's colour table.
- With `OPAQUE`, each line's text box is filled with the background colour first. The game never calls `SetBkColor`, so this is white.
- Text is treated as Latin-1, which matches the game's Windows-1252 strings except for the 0x80–0x9F range.

## WinMM

The shim opens one SDL output device on first use and shares it: S16 stereo, 1024-sample buffers, at `$PORT_AUDIO_RATE` (default 44100).

- Each `HWAVEOUT` is a slot with an `SDL_AudioStream` that converts its `WAVEFORMATEX` (PCM 8/16-bit, mono or stereo) to the device format.
- The SDL callback mixes all open slots with clamping.
- The game opens and closes a waveOut for every sound (sound_40c960), so opening is cheap: it takes a slot and needs no SDL device open.

| Call | Semantics |
|---|---|
| `waveOutGetNumDevs` | 1 if the SDL audio device opens, else 0. `PORT_NOSOUND=1` forces 0, which exercises the game's timer-based fallback in `fn_40d148`. |
| `waveOutOpen` | `WAVE_FORMAT_QUERY` only validates the format and leaves `*phwo` untouched. Supports `CALLBACK_NULL` and `CALLBACK_FUNCTION`. `WOM_OPEN` is delivered synchronously. |
| `waveOutPrepareHeader` / `waveOutUnprepareHeader` | set/clear `WHDR_PREPARED`. Unprepare while `WHDR_INQUEUE` returns `WAVERR_STILLPLAYING`. |
| `waveOutWrite` | needs `WHDR_PREPARED` (else `WAVERR_UNPREPARED`); clears `WHDR_DONE`, sets `WHDR_INQUEUE`, queues the header. Loop flags are ignored and logged. |
| `waveOutReset` | clears the stream and completes every queued header at once: `WHDR_DONE` plus `WOM_DONE`, on the caller's thread. |
| `waveOutClose` | `WAVERR_STILLPLAYING` while headers are queued; otherwise delivers `WOM_CLOSE` and frees the slot. |
| `mmioStringToFOURCCA` | up to 4 characters, padded with spaces (`"fmt"` → `'fmt '`); `MMIO_TOUPPER` is honoured. |

**Completion.**

- A header is done when its last converted byte has been handed to the device buffer.
  - At that point `WHDR_DONE` is set, `WHDR_INQUEUE` is cleared, and the `CALLBACK_FUNCTION` gets `WOM_DONE` on the SDL audio thread.
  - Windows calls it on a driver thread, so the threading matches.
- The game's callback `fn_40c718` only stores `g_460208->done`. The game polls that value every tick (`fn_40d148` → `fn_40c804`), which suits this model.
- `WOM_DONE` therefore fires up to one device buffer before the sound is audibly finished: about 23 ms at 44.1 kHz, and about 40 ms in the test at 11 kHz. This keeps queued sounds free of gaps.
- When the last queued header has been fed, the stream is flushed so that the resampler's tail is not lost.

`$PORT_AUDIO_DUMP=<file>` writes the mixed output (raw S16 stereo at the device rate), for tests.

## Resources

`port_res_open(path)` loads the whole exe and flattens its resource tree (type → name → language) into a table.

- The path defaults to `$ELFBOWL_EXE`, then `Elf Bowling.exe` in the working directory.
- The first `FindResourceA` opens it lazily.
- Only PE32 is accepted.

| Call | Semantics |
|---|---|
| `FindResourceA` | matches a name or type given as `MAKEINTRESOURCE`, as `"#123"`, or as a string compared case-insensitively (the game asks for `"PackedFile"`; the exe stores `PACKEDFILE`). Takes the first language. The module handle is ignored. |
| `LoadResource` / `LockResource` | return a pointer into the loaded image (writable; the game only reads it) |
| `SizeofResource` | the data entry size |

- Tested: NVDPACKFILE/PACKEDFILE, NVDPARMFILE/PARMS, NVDCASTFILE/CASTS, RCDATA/TFORM1 and STRING #4087.
- Calling `port_res_open` again invalidates earlier `HRSRC`s.
- No asset is ever written into the repo.

## Kernel, registry and shell

| Call | Semantics |
|---|---|
| `GetTickCount` | `SDL_GetTicks64` truncated to 32 bits: milliseconds since SDL's timer started, wrapping like Win32 |
| `GlobalMemoryStatus` | host RAM (`SDL_GetSystemRAM`) clamped to 2 GB, 3/4 reported available, load 25%; values are approximate |
| `TlsAlloc`/`TlsFree`/`TlsGetValue`/`TlsSetValue` | 64 slots on `SDL_TLSID`. SDL ids are never released, so `TlsFree` clears the value and marks the slot reusable. |
| `LocalAlloc`/`LocalFree` | malloc/calloc (`LMEM_ZEROINIT`); every HLOCAL is the pointer, and `LMEM_MOVEABLE` is treated as fixed |
| `lstrcpyA`/`lstrcatA`/`lstrlenA` | libc; NULL-safe like Win32 |
| `LoadLibraryA` | stub: NULL. The web tracker (`fn_40e800`) then reports "no Internet connection". |
| `GetProcAddress` | stub: NULL |
| `FreeLibrary` | stub: TRUE |
| `RegOpenKeyExA` / `RegQueryValueA` | stub: `ERROR_FILE_NOT_FOUND` |
| `RegCloseKey` | `ERROR_SUCCESS` |
| `ShellExecuteA` | returns 31 (failure) and logs. With `PORT_OPEN_URLS=1` an http(s) URL is opened through `SDL_OpenURL` and the call returns 42. |
| `WinExec` | stub: returns 2 and runs nothing |
| `FindWindowA` | stub: NULL, so the game's taskbar hide/restore (`fn_401df8`/`fn_401e20`) does nothing |
| `ShowWindow` | shows or hides the SDL window for the main HWND; logs anything else |

Stubs log through `port_log` (`[shim] …` on stderr). `PORT_LOG=0` silences it.

## Environment

| Variable | Effect |
|---|---|
| `ELFBOWL_EXE` | path of the original exe |
| `PORT_FONT_DIR` | first directory searched for TrueType files |
| `PORT_AUDIO_RATE` | device sample rate (default 44100) |
| `PORT_AUDIO_DUMP` | raw dump of the mixed audio |
| `PORT_NOSOUND=1` | `waveOutGetNumDevs` returns 0 |
| `PORT_OPEN_URLS=1` | `ShellExecuteA` opens http(s) URLs |
| `PORT_LOG=0` | no shim logging |
| `PORT_DUMP_FRAMES=N` | every Nth `port_present` writes the screen to `$PORT_DUMP_DIR/NNNN.bmp` (NNNN = the present count) |
| `PORT_DUMP_DIR` | frame dump directory, default `build/frames` (created) |
| `PORT_INPUT` | scripted input for headless runs (VCL loop, forms.cpp): `ms:click:x,y;ms:key:sym;...`, times from the first pump; `click` is a left press and release at client (x, y), `key` a press and release of an SDL keycode (13 Return, 27 Esc, 32 space) |
| `PORT_WINPUT` | scripted events in **window** coordinates, pushed through SDL's queue so the renderer's letterbox mapping applies (gdi.c): `ms:click:x,y;ms:key:sym;ms:size:w,h`, times from the first present |
| `PORT_FULLSCREEN=1` | start full screen (desktop mode; the Linux AppImage's AppRun sets it unless it is already set, so `PORT_FULLSCREEN=0` gives a window) |
| `PORT_WINDOW_SIZE` | initial window size `WxH` (default 640x480, or up to 2x the screen when the 3x art shows; the window is resizable) |
| `PORT_DUMP_WINDOW=1` | frame dumps read back the window (letterbox included) instead of the presented surface |
| `PORT_DUMP_1X=1` | with the 3x art showing, each dump also writes `NNNN_1x.bmp`, the 1x frame |
| `ELFBOWL_CLASSIC=1` | start with the original 1x frame (F9 switches to the 3x art; docs/HIRES.md) |
| `ELFBOWL_HIRES` | directory of the 3x art cache (default `build/hires/x3`, or `hires/x3` beside the executable) |

## Game-code issues found while compiling `src_match/` against these headers

These are for Phase A/C. The shim headers are not the cause.

- 64-bit pointer truncation:
  - `(int)ShellExecuteA(...)` in menu_41008c;
  - `DWORD proc = (DWORD)fn_40c718` in sound_40c960;
  - `(int)c` in range_403540.
- `tmax(0L, r->left)` in range_401d0c: `0L` is `long` but `LONG` is `int32_t`, so template deduction fails. It needs `(LONG)0`.
- `fn_40c718` is declared with `DWORD` parameters, while the waveOut callback type uses `DWORD_PTR`. It works on arm64 and x86-64, but the prototype should change.
- `fn_40c6d0` takes the callback as `DWORD`, so it has to become `DWORD_PTR`.
- `stricmp` comes from the Borland RTL, not from `windows.h`. The port's RTL layer should map it to `strcasecmp`.
- All `char pad[..]` struct layouts assume 32-bit pointers; the Phase A header work fixes that.
