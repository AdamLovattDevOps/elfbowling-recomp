# Matching workflow

The goal is C++ source that BCB3 `bcc32 -Od` compiles to the **same bytes** as the original function. Only the bytes covered by linker fixups (addresses) are ignored.

## Tools

| Command | Purpose |
|---|---|
| `.venv/bin/python tools/disasm.py 40bf2c` | Annotated disassembly of the function at that address. Shows callees, imports and string literals. |
| `.venv/bin/python tools/match.py src_match/FILE.cpp -v` | Compile one file and diff every `fn_XXXXXX` in it. Prints OK or DIFF with a hex dump. |
| `make progress` | Matched game bytes out of total game bytes. |
| `build/funcs.tsv` | Function list: start, end, size, name, kind. Only `kind=game` entries need matching. |
| `build/libmap.tsv` | Library (VCL/RTL) functions, identified by name. Call these by their real API. |

## Rules

- Name every matched function `extern "C" <ret> fn_XXXXXX(...)`, where XXXXXX is its address in lowercase hex. The harness finds functions by that name.
- Declare callees as `extern "C" ... fn_YYYYYY(...);`, even when they are not matched yet. The call bytes are fixups, so the callee's name does not affect the match.
- Library calls (VCL, RTL, Win32 imports):
  - Declare a Win32 import the way `<windows.h>` does. Including `<windows.h>` is also fine.
  - A call to a mangled VCL method (for example `@Forms@TForm@...`) usually means a member call on a VCL object. Until the real VCL headers are wired in, fake it with a matching `extern` declaration.
- Structs: declare them locally in the file, with fields named by offset (`char pad[0x13c]; void *f13c;`). Rename a field when its meaning is clear, for example `sprites` for the sprite group. Keep layouts consistent with other files that touch the same struct; grep `src_match/` first.
- Borland `-Od` facts:
  - Every argument and local is reloaded from `[ebp±n]` at each use.
  - A register is chosen per expression, rotating through eax, edx, ecx.
  - Functions are cdecl (`add esp, N` after the call).
  - Padding to a 4-byte boundary is 0x90, and the harness allows it.
- Arguments pushed in the order `push [ebp+0xc]; push [ebp+8]` are a normal right-to-left cdecl call: `f(arg8, argc)`.
- A function that does not match after a reasonable effort: leave it out of `src_match/`, or keep it under `src_nonmatch/` with a comment. Never commit a DIFF into `src_match/`.
- Some `kind=game` entries are really data: RTTI, class-name strings, or exception tables. When the disassembly is nonsense (for example `add [eax], al` runs or ASCII), record the address in `notes/data_in_code.txt` and skip it.
- One source file per contiguous address range: `src_match/<unit>_<firstaddr>.cpp`. The unit names come from exports: Main, Sprites, Init, Threads, About, Game, Opening, Preopening. Name the file by range when the unit is unknown.

## Learned (round 1)

- The build flags are `-Od -k`. `-k` forces a standard stack frame, so even empty functions get `push ebp/mov ebp,esp`.
- `__fastcall` extern "C" functions produce the symbol `@fn_XXXXXX`, and the harness accepts it. Delphi register-convention calls such as `Classes::Rect` and TForm1 event handlers use `__fastcall`.
- Codegen quirks and struct layouts: see `notes/round1_*.md`.
- Real C++ members, such as constructors with EH frames, virtual methods and operators, can be matched by keeping the natural C++ and adding `// MATCH <addr> <mangled symbol>` in the file. Find the mangled name with `tdump` or by reading the harness error. The harness then compares that symbol against the address.
- `tools/syms.py FILE.cpp` lists the mangled code symbols in a file, for use in MATCH directives. Run `match.py --no-record` on scratch experiments; results are only recorded for files under `src_match/` anyway.
