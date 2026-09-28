# Port plan

The plan is **one source tree that builds two ways**:

1. **Matching:** `bcc32 -Od -k` under Wine gives byte-identical functions. This is the proof that the source is correct.
2. **Native:** clang or gcc on macOS, Linux, Windows and later iOS or Vita, against a small platform shim on SDL2.

This is the same end state as jungle-recomp, but here the game logic *is* the matched source, not a reimplementation.

## Phase A: consolidate headers (the matching build must stay at 100%)

- Move every struct into `include/` with real field types, and remove every `char pad[..]`. Keep a field-offset comment on each member.
- Keep `static_assert`-style offset checks, guarded by `#ifdef __BORLANDC__`, so the matching build proves the 32-bit layout. The native build is then free to use 64-bit pointers.
- Remove the per-file duplicate declarations of callees. Give each `fn_XXXXXX` one prototype, with a real name alias as it gets understood (for example `#define Actor_SetPos fn_406a80`).
- Gate: `make match` stays 100% OK after every step.
- Step 1 is done: the canonical headers are in `include/elf/`, and 11 pilot units use them. docs/HEADERS.md has the class map, the old-to-new name map, the migration procedure and the batching plan for the remaining 76 files.

## Phase B: platform shim (`port/`)

The whole external surface is in `notes/port_deps.tsv`. It is small:

| Area | Calls used by game code | SDL2 mapping |
|---|---|---|
| GDI | CreateDIBSection, CreateCompatibleDC, SelectObject, BitBlt, palettes, FillRect, DrawTextA, fonts (EnumFontFamilies, CreateFontIndirect), SetTextColor/BkMode | 8-bit DIB framebuffers in memory; blits in software; present through an SDL texture. Text through a bundled bitmap font or SDL_ttf. |
| WinMM | waveOutOpen/Prepare/Write/Unprepare/Reset/Close, mmio chunk parsing | SDL audio queue; the WAVs are 8-bit 11 kHz and 16-bit 44 kHz PCM |
| Resources | FindResource, LoadResource, LockResource, SizeofResource | Serve `NVDPACKFILE`, `PARMS` and `CASTS` from the user's own `Elf Bowling.exe`, read with the logic in tools/unpack.py, reimplemented in C |
| Kernel | TLS, GetTickCount, GlobalMemoryStatus, lstr* | trivial |
| Registry and shell | RegOpenKey/RegQueryValue, ShellExecute, WinExec (web links) | stub, or open the URL |
| VCL | TForm (window, client size, show), TApplication (Initialize, CreateForm, Run), TThread, TResourceStream, TStream, AnsiString, TClientSocket | a small C++ class set in `port/vcl/` covering only the members used |
| Winsock / TClientSocket | TWebTrack's "phone home" tracker | no-op; the target server has been gone for decades |
| RTL | new/delete, str*, mem*, rand/srand, time, zlib (matched, in-tree) | host libc |

## Phase C: native build

- `make native` builds `build/elfbowl` from the same sources with `-DPORT`. The user supplies the original `Elf Bowling.exe`; no copyrighted assets go in the repo.
- Carry over the jungle-recomp extras only after the game plays: gamepad, window scaling or integer scaling, e2e bot.
