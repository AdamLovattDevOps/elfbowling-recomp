# Native build triage (`make native`)

`make native` compiles every `src_match/*.cpp` separately with clang++ (arm64 macOS, 64-bit) and links `build/elfbowl`. The top-level target runs `make -k -C port native`, so one run reports every error. The rules are in port/Makefile under "native game build".

| Piece | What it is |
|---|---|
| flags | `-std=gnu++17 -DPORT -fms-extensions -fsigned-char -fno-strict-aliasing -fwrapv -include bcb_rtl.h -Iport/include -Iinclude`, with `-O1 -g`. Six `-Wno-*` only: unknown pragmas, `return-type-c-linkage` (extern "C" functions that return ERect/TPoint), `deprecated-copy-with-user-provided-copy` (ERect's copy ctor), `mismatched-tags`, and zlib's `register`. With `-Wall -Wextra`, src_match/ shows no other warning class, so nothing else is hidden. |
| per-unit logs | `build/native/obj/<unit>.log` |
| link | the unit objects, `port/native/game_native.cpp`, `port/vcl/elf_glue.cpp`, the generated `build/native/game_data.cpp`, `vcl_main.o`, `libvcl.a`, `libshim.a`, SDL2, SDL2_ttf and the system zlib (`-lz`) |
| `NATIVE_OVERLAY=dir` | compiles `dir/X.cpp` in place of `src_match/X.cpp` where one exists, into a separate `obj-overlay/` directory. Use it to try a source-side fix before it lands. It is never set by default. |

## Status (last run)

- **Compile:** all 87 units compile, with no warnings.
- **Link:** fails on one item. zlib_41e2cc declares five zlib statics that nothing defines (row S5 below).
- **Run:** built with the three source-side fixes S5–S7 in a scratch overlay, the game runs headless and reaches the title menu and the lane screen (see "Runtime" below).

The first run (before batch E landed) had 12 units failing and 204 undefined symbols. The "status" column shows where each item stands now.

## Triage table

"Where" says who owns the fix:
- **port:** port/, done by me;
- **src:** include/ or src_match/, for the migration agent.

| # | Error class | Files affected | Fix | Where | Status |
|---|---|---|---|---|---|
| P1 | g_ globals undefined at link: game data lives at fixed .data addresses in the original (139 undefined at first link) | every unit | **port/tools/gen_game_data.py** generates `build/native/game_data.cpp` from game.h (types) plus the exe (values); see "Game data" below | port | done: 150 game.h globals plus 7 zlib tables |
| P2 | extern "C" `fn_XXXXXX` for member-matched functions undefined: `TSpriteGroup::fn_40971c..4097e8`, `TScene::fn_40a1fc/40a520/40a598`, `TStage::fn_40ae80/40af80/40b9b8/40ba68/40bf2c/40bf84/40c160`, `TSoundMgr::fn_40c880`, `TPackedResources::fn_40d250`, and `fn_4067e0` (= `TGraphicSprite::AddTalkingIndex`) | callers in most units | one-line wrappers in port/native/game_native.cpp | port | done (17) |
| P3 | per-unit aliases undefined: `fn_40252c_E/_TRect`, `fn_402580_E`, `fn_4025e0_E`, `fn_402748_t`, `fn_40278c_t`, `fn_402840_E`, `fn_4028b8_E`, `fn_402e30_t`, `fn_401d0c_pt`, `fn_407a44_pt`, `fn_40a5d4_E`, `fn_40a63c_E`. bcc32 never links, so they only exist as spellings. | range_*, sprites_*, engine_*, game_*, egg | forwarders in game_native.cpp that convert ERect↔RECT/RectPod, and map TPoints to the int pairs in their 32-bit stack order (`fn_401d0c_pt(p,a,b)` → `fn_401d0c(p.x,p.y,a.x,a.y,b.x,b.y)`; `fn_407a44_pt` narrows its last two to char) | port | done (15) |
| P4 | C++ special members that the headers declare but whose bodies were matched as free functions: `TSound::~TSound` (fn_40c5dc), `TSaveCasts::~TSaveCasts` (fn_40e210), `TSceneButton::TSceneButton` (0x4099d4, `mov eax,[ebp+8]; ret`) | sound_40cf24, packres_*, scene_* | definitions in game_native.cpp (dtor = the free function with flags 0) | port | done |
| P5 | `vtable for TPackedStream` undefined: the key function `Read` has no C++ body. Read is matched as fn_40dee0 and Seek as fn_40e004, over TPackedStreamData. | packres_40e0a8, 40d4bc, sound_40c1f0 | `TPackedStream::Read/Seek` in game_native.cpp forward to fn_40dee0/fn_40e004. A static_assert checks that TPackedStream and TPackedStreamData have the same native layout. | port | done |
| P6 | fn_40e19c undefined (libmap called it `is_open`; VCL_PORT item 10) | packres_40d3f8 (fn_40d568) | inflateEnd on the stream's z_stream, found by field name instead of the 32-bit +0x10. First written in game_native.cpp; batch E then defined it in zlib_41e2cc.cpp (`#ifndef __BORLANDC__`, against zlib's own z_stream), so the port copy was removed to avoid a duplicate symbol. | port, then src | done (in zlib_41e2cc) |
| P7 | `void *dummy_new(int,int)` defined in both packres_40f104 and packres_40f2a4, so the symbol is duplicated at link (it only makes bcc32 emit the inline dtors) | packres_40f104, packres_40f2a4 | port workaround: `-Ddummy_new=dummy_new_40f2a4` for packres_40f2a4 (port/Makefile `NATIVE_FLAGS_packres_40f2a4`). The real fix is `static`, or `#ifdef __BORLANDC__` around both copies. | port (workaround); src (real fix) | worked around |
| P8 | zlib_41e2cc redeclares `memcpy(void*,const void*,unsigned)` and `calloc(unsigned,unsigned)`, which conflicts with the libc prototypes that the forced `bcb_rtl.h` pulls in | zlib_41e2cc | natively include `<string.h>`/`<stdlib.h>` instead: an `unsigned` argument where libc takes `size_t` leaves the upper half of x0 unspecified under AAPCS64. (Worked around for a while by building the unit without `-include bcb_rtl.h`; the `NATIVE_RTL_<unit> := none` hook stays in port/Makefile.) | src | fixed by batch E (`#ifdef __BORLANDC__` around the redeclarations); port workaround removed |
| P9 | frame dump for headless runs | shim | `PORT_DUMP_FRAMES=N` / `PORT_DUMP_DIR` in port/src/gdi.c (`port_present`). Also `PORT_INPUT` scripted clicks and keys in port/vcl/forms.cpp. docs/SHIM.md lists both. | port | done |
| P10 | missing shim APIs / VCL members / Borland RTL | none | no missing API, VCL member or RTL function turned up once the local VCL copies were gone. The `TClientSocket` "no member OnConnect/Host/..." errors came from webtrack's local class copies, not from the port. | port | nothing needed |
| S1 | `__closure` declarators and implicit `this` binding (`typedef void __fastcall (__closure *T)(...)`; `form->OnMouseMove = MouseMove`; `self->Update`) | stage.h (so every VCL unit), engine_40a978, threads_40f674/684, webtrack | `ELF_CLOSURE` / `ELF_METHOD` (VCL_PORT items 2, 3, 7, 8) | src | fixed by batch E |
| S2 | `__property` and local copies of VCL classes (`TObject`, `TComponent`, `TThread`, sockets) that conflict with port/include/vcl | webtrack_40e5ec, threads_40f604/40f734, engine_40a978 (duplicate `TMyThread`) | use `<vcl/scktcomp.hpp>` / `<vcl/classes.hpp>` and elf/thread.h | src | fixed by batch E |
| S3 | Borland implicit int (`WINAPI WinMain(...)`); `Form1` not declared | elf_40128c | `int WINAPI WinMain`; `#define Form1 g_4601c0` (VCL_PORT item 5) | src | fixed by batch E |
| S4 | template deduction `tmax_if(0L, LONG)`: `0L` is 64-bit long, LONG is int32 | range_401d0c | `(LONG)0` | src | fixed |
| S5 | **zlib statics without an address: `g_fixed_mem_460798` (inflate_huft), `g_fixed_bl`, `g_fixed_bd`, `g_fixed_tl`, `g_fixed_td`.** They are extern in the unit. inflate_huft is a zlib-local type, and the last four have no address to take a value from, so the generator cannot define them. | zlib_41e2cc | define them in the unit, zero-initialised (they are zlib `local` bss variables), for example `#ifndef __BORLANDC__` definitions next to the declarations | src | **open: last link error** |
| S6 | **z_stream layout differs from ZStream natively.** zlib_41e2cc has `typedef unsigned long uLong` (64-bit on LP64), so `total_in`/`total_out`/`adler`/`reserved` are 8 bytes. packres.h's ZStream uses `unsigned` (4). The two units disagree on every field after `avail_in`. | zlib_41e2cc, packres.h | make both use a 32-bit type natively. For example, zlib's `uLong` → `unsigned int` under `#ifndef __BORLANDC__` (same code on bcc32, where long is 32-bit). | src | open |
| S7 | **hard-coded 32-bit struct size passed to zlib:** `fn_420984(&z, "1.0", 0x38)`. inflateInit2_ checks `stream_size != sizeof(z_stream)` and returns Z_VERSION_ERROR natively. Every packed bitmap then fails ("Unable to crate a bitmap image for ArmsDance1.bmp", the game's own MessageBox). | packres_40e0a8 | `sizeof(ZStream)` (the same constant on bcc32) | src | open (confirmed at runtime: with S5 alone the game stops at that MessageBox; with S5+S6+S7 it runs) |
| S8 | pointer-sized callback passed as DWORD: `fn_40c6d0(..., DWORD cb, ...)`. It truncated the waveOut callback to 32 bits, and the first sound (clicking PLAY) jumped to 0x1f280. | sound_40c5dc, funcs.h | `DWORD_PTR` for `cb`/`inst` and for fn_40c718's parameters (SHIM.md) | src | fixed by batch E (seen and verified at runtime) |
| S9 | `TWebTrackNew` stand-in with `char pad[0x18]`: 32-bit size, while the native TWebTrack is 40 bytes, so `new` would under-allocate. The stand-in's ctor was also undefined. | init_40f350 | construct the real class (stage.h ctor declaration) | src | fixed by batch E |
| S10 | `p_Application` with C++ linkage, not defined anywhere | range_401cec (and webtrack) | `g_45fee4` (VCL_PORT item 9) | src | fixed |
| S11 | `memset(&p->hdr, 0, 0x20)` and the `0x20` `cbwh` arguments: 32-bit `sizeof(WAVEHDR)`. The native WAVEHDR is 48 bytes, so `lpNext`/`reserved` are not cleared. Harmless today, because the shim's waveOutWrite sets them. | sound_40c960 | `sizeof(WAVEHDR)` (0x20 on bcc32) | src | open, low priority |
| S12 | raw VMT call `ctl->vt[0x7c / 4](ctl)` on a TControl: 32-bit Delphi slot numbering, with no native meaning | scene_409c50 | guard it or map it to the VCL method; it is dead code (`TScene::nbuttons` stays 0) | src | open, dead code |
| S13 | `TPackedStream` does not override `SetSize`. The original VMT has fn_40dfd0 (`outsize = v`) in slot 0; natively the base `TStream::SetSize` runs. | packres.h | declare `virtual void SetSize(int)` in TPackedStream (port or unit body = fn_40dfd0). Nothing calls it today. | src | open, low priority |
| S14 | `char _unk28[4]` in TPackedResources (0x28) sits between pointer fields and could hide a pointer. Other `_unk` gaps are file-format bytes (PackHdr, ParmEntry, CastEntry, DibFile) or Borland-only stand-ins (TStageForm, TSceneCtl), and are fine natively. | packres.h | identify the field if a unit ever touches it; nothing does now | src | watch |

Classes that were expected and not found:
- **Offset-based field access.** No unit reads fields through `(char *)p + 0x..` any more.
- **Pointer/int truncation compile errors.** The only survivor was S8, which is a truncation through a `DWORD` parameter, not a cast.
- **Borland keywords beyond `__closure`/`__property`.**

## Game data (`port/tools/gen_game_data.py`)

In the original, the game's globals are fixed addresses in `.data`:
- 0x455000..0x460200 is initialised;
- the rest of the 0x11000-byte section is zero-filled, like .bss.

Native code cannot keep those addresses (pointers are 8 bytes), so every global becomes its own C++ definition. `port/native/game_data.cpp` holds the definitions plus a table of file offsets (no exe bytes) that loads the initial values from the user's exe at startup (`elf_game_data_load`); `make -C port game-data` regenerates it after game.h or src_match/ change.

Sources:
- **Declarations:** every `extern` in game.h, and every `extern "C" g_*_XXXXXX` a unit declares that game.h lacks (the zlib tables `g_border_45bf2c`, `g_cplens_45c0f0`, ...).
- **Excluded:** `g_45fee0/4/8`, the VCL cells that port/vcl/entry.cpp defines.
- **Address:** the last six hex digits of the name.
- **Size:** `[]` arrays are sized by the gap to the next known symbol (game.h, every `g_*` in src_match/ and the VCL cells). A pointer table stops at the first dword that is neither relocated nor 0. Examples: `g_456ee8[12]`, `g_45aaf4[29]`, `g_45ab68[26]`, `g_458884[6][3][10]`, `g_4601cc[8]`.

Values:

| Kind | Emitted as |
|---|---|
| integers, bool, char, and arrays of them | brace initialisers from the raw bytes |
| pointer: 0 | `nullptr` |
| pointer: relocated (the PE `.reloc` table decides), into a known global | an address expression |
| pointer: relocated, any other .data address | a C string literal read from the exe (the credit, rules and story texts; the taunt wav names) |
| pointer: relocated, into .text | `fn_XXXXXX` (function pointers only) |
| FrameScore, PinState | the original bytes, `memcpy`'d in by a static constructor, after a `static_assert` that the native size equals the 32-bit size |
| anything in the zero-filled part | zero-initialised |

Checks and limits:
- **Checks:** `g_455531 = 255` and `g_455554 = -1` come out as expected, and the string tables match `pe.cstr`.
- **Limitation:** code that indexes past one global into the next (fine in the flat .data) would break natively. None was found: each table's loop bound (26, 29, 12, ...) fits its gap.
- **Unresolved:** the generator lists what it cannot define at the end of the file, currently the five zlib statics of S5.

## Runtime

Run with the S5–S7 fixes in a scratch overlay (`make -C port native NATIVE_OVERLAY=...`):

```
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ELFBOWL_EXE="orig/1999/Elf Bowling.exe" \
  PORT_DUMP_FRAMES=50 PORT_INPUT="8000:click:240,430;14000:key:32;20000:key:32" timeout 28 build/elfbowl
```

- **Frames 25–100:** the NVision Design splash, a logo and "AGGRESSIVE SOLUTIONS FOR THE INTERNET AND MULTIMEDIA." on black.
- **From about frame 500:** the title menu.
  - NStorm logo, Santa portrait, the "elf bowling" logo with ball and pins, and the copyright line.
  - The parchment scroll with "An Internet Christmas Story" scrolling up.
  - Two elves, one with the "ELVES LOCAL 502 ON STRIKE!" sign.
  - Animated Christmas lights, falling snow, and the PLAY / RULES / QUIT buttons.
  - Frames 1000–3700 show the story text scrolling ("'Twas the night before Christmas, ...").
- **After the PLAY click (8 s):** the lane screen.
  - The "Santa / instormomatic" scoreboard for frames 1–10, with EXIT.
  - The lane with the elves as pins at the far end, and the aim slider with "Press the space bar when the slider is here...".
  - The ten-pin indicator, and a close-up of the ten elves on the right.
  - Before any key, a close-up elf holds up a "SANTA SUX!" sign (a taunt, 14 s run, frame 1700).
  - After Space, the close-up elves react to the throw (frame 2700). By frame 3450 the slider is moving again.
  - No score appeared on the board in the frames I checked.

Logs: only the expected stubs. Wininet `LoadLibraryA` → NULL (the web tracker stays off), `FindWindowA("Shell_TrayWnd")` → NULL, and Apple Chancery / Arial Bold for the fonts. No shim errors.
