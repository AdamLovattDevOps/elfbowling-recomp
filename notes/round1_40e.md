# Round 1 notes, range 0x40e5ec–0x414240 (agent report)

The range covers TWebTrack, Init, Threads, About, the ElfCrew easter egg, the main menu, and the score/pins/elves unit. There is no zlib here.

## Quirks
- **Alignment:** the easter-egg unit has functions at odd addresses. Put `#pragma option -a1` at the top of the file.
- **Locals declared mid-block** are allocated below later inner-block locals and temporaries.
- **Switch on an expression** that the original spills to the stack: use a named local (`int which=i/2; switch(which)`).
- **Ternaries:**
  - `b?1:0` gives `mov r,1; cmp; jne; dec r`.
  - `c?0:K` gives `xor; cmp; je; add K`.
  - A bool array needs `?true:false`.
- **Inline helper returning bool** gives `test; setne al; and eax,1; test al,al`.
- **`cmp byte [x],6`** needs `unsigned char`; plain `char` gives movsx.
- **TRect:** copy BCB3 `Windows::TRect` (ctor plus union) to get the `rep movsd` copies. `Classes::Rect(...)` passed as an argument is built in the argument slot.
- **Extra temporary on a copy:** the copy comes from an inline setter taking TRect by value.
- **stdlib min/max/abs:** `<stdlib.h>` min/max templates give the `lea` reference pattern; abs gives cdq/xor/sub with a temp.
- **VCL event handlers:** `extern "C" __fastcall` (eax/edx/ecx, then stack, callee pops).
- **TThread::Synchronize:** `typedef void __fastcall (__closure *TThreadMethod)();`
- **`new T(args)`** with a real constructor reproduces the EH-frame new sequence.
- **Unsolved:** register rotation after `Classes::Point`/`Rect` calls, in 40fd94, 40feb0, 413168 and 413244 (see `src_nonmatch/`).

## Structs
- **TWebTrack** (0x18): buf, request, len@8, state@0x10 (1 connected … 6 error), TClientSocket*@0x14.
- **Screen:** onClick@0x1c; app@0x13c (App: mouse TPoint@0x8c0, sound@0x8c8, music@0x8cc).
- **Sprite:**
  - name@8, frame@0x24, clip TRect@0xe0
  - pos×1000@0x100/0x104
  - onDone@0x160, lane@0x1c8, frames[]@0x1f4
- **SpriteGroup:** count@0x1c, items[]@0x2c.
- **Key/Button:** down@0, enabled@0x148, onClick@0x388.
- **FrameScore** (16) ×12 @0x4605c8: roll[2], total (-1 = unscored), strike@0xc, spare@0xd.
- **Pin** (8) ×10 @0x460240: state u8 (6 = down).

## Globals
- 0x456c38 game; 0x4601c0 play form; 0x460214 main form.
- 0x46059c score; 0x460598 frame; 0x4605bc roll.
- 0x4605a0 cheated; 0x4605a1–a4 cheats (Ctrl+X/D/S/G, cleared by Ctrl+N).
- 0x460578 pins standing[10]; 0x460688/0x46068c aim.
- Sprite groups: 0x460538–0x460564.

## Round 2

### Quirks
- **Register rotation after `Point()`/`Rect()` (solved).** The start register of a statement is the one after the previous statement's last "chosen" register. A `__fastcall` struct-returning call with division arguments leaves the next two statements both starting at eax. The fix was in the *callee declaration*, not the Point call: `fn_406a80(spr, TPoint p)` and `fn_407924(spr, TPoint p, int, char, char, int)` take the point **by value**. Pushing a struct (`push [p.y]; push [p.x]`) resets the rotation the way the original does; two separate int args do not. `fn_413244(int i, TPoint d)` also takes its point by value (`d.x /= 4`). If a rotation mismatch follows a call, check whether a callee argument pair is really a TPoint/TRect.
- **Real VCL headers:** `#include <vcl.h>` fails. VCL\*.HPP include `<Windows.hpp>` etc. with angle brackets and need `-I INCLUDE\VCL`. `#pragma option -I` is rejected. Work around it by copying the needed declarations locally:
  - Keep the exact virtual order: the full TObject list; then TPersistent AssignTo, DefineProperties, Assign; then TComponent Loaded, Notification, ReadState, SetName = VMT +0x18.
  - Use `__declspec(delphiclass/pascalimplementation, package)`.
  - Use `__declspec(delphireturn, package)` on AnsiString. Without it, AnsiString arguments to pascal methods are built on the stack instead of in an `[ebp-n]` temp passed as `mov edx,[eax]`.
- **A user-declared destructor** (even one that is only declared) makes a constructor emit `mov word [ebp-ctx],8` right after `__InitExceptBlockLDTC`.
- **`int x = boolfunc(); if (x)`** declared mid-block gives `movsx ecx,al; mov [ebp-0x30],ecx; cmp [ebp-0x30],0`, with the slot below the EH record.
- **Delphi threadvar:** `extern T __thread Var;` gives `call <tls helper 0x40110c>; mov [eax+off],imm` (Scktcomp::SocketErrorProc).
- **`new TResourceStream`** in the TPackedStream unit is why the inline dtors `TCustomMemoryStream::~` and `TStream::~` are emitted there. `delete` of a TResourceStream* is enough to generate them.
- **Delphi-class ctor** (`TMyThread(bool) : TThread(b) {}`) matches as-is: _ClassCreate, dl flag, `add [ebp-0xc],2`, _AfterConstruction.
- The mangled bool in a symbol is `4bool` (`@TMyThread@$bctr$qqr4bool`), not `o`.

### Structs / layout
- TCustomSocket events: FOnLookup@0x38, Connect@0x40, Connecting@0x48, Disconnect@0x50, Listen@0x58, Accept@0x60, Read@0x68, Write@0x70, Error@0x78. TWebTrack wires Connect=40e5ac, Connecting=40e664, Disconnect=40e624, Error=40e5ec, Lookup=40e5cc, Read=40e684, Write=40e6ac.
- TWebTrack: f0c is a bool; fn_40ebd0 is really `~TWebTrack`.
- FrameScore +0x0c/+0x0d are "roll drawn on board" flags. unit_411 names them shown0/shown1; the score file calls them strike/spare.
- SpriteGroup is 0x12c bytes: items[64]@0x2c; ctor fn_409844(name, screen, pattern).
- Button: onClick@0x388, onDown@0x38c.
- App+0x10 is cleared at the end of menu setup (fn_4112b8).

### Boundary-only blockers (code byte-identical, kept in src_nonmatch/)
- 40f104 TPackedStream dtor (75 B), 40f1e4 TStream dtor (87 B), 40f734 TMyThread dtor (75 B): each is followed by its EH table. The ends have been appended to notes/func_ends.tsv (40f150, 40f23c, 40f780), but build/funcs.tsv has not been regenerated.
