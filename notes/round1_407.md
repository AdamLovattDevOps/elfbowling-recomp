# Round 1 notes, range 0x407ffc–0x40e5ec (agent report)

This range is the Sprites, TStage (engine), TSoundMgr, TSound, TPackedResources and TPackedStream code.

## Class names (from RTTI)
TGraphicSprite, TButtonSprite, TTextCast, TStage, TSoundMgr, TCastMgr, TSaveParms, TResourceStream, TSound, TSaveCasts, TPackedResources, TPackedStream.

## Quirks
- **Rect passed by value with `rep movsd`** needs `struct Rect{int l,t,r,b; Rect(){} Rect(const Rect&o){*this=o;}}`.
  - `Rect r = *p` copies through a pointer temp.
  - `Rect r; r = *p;` copies directly.
- **Loops over `this->arr[i]` with `i < this->n`:** the index loads first only in a member function. Try a free function first, then a member with MATCH.
- **Destructors:**
  - A declared destructor makes the constructor emit `mov word [ebp-0x14],8`.
  - `delete p` calls dtor(p,3).
  - Deleting destructors match as free `(p, int flags)` functions with `if(flags&1) fn_448a2c(p)`.
- **Two max templates:** `a<b?b:a` and `a>b?a:b`. The references spill b first.
- **Delphi virtual call:** `st->vt[1](st,buf,n)` through a `__fastcall` function-pointer table.
- **TShiftState:** a 1-byte struct with an inline `bool Contains()`.
- **Mislabelled library entries:**
  - 0x448a2c is `operator delete` (libmap says `_close.c`).
  - 0x434b6c / 0x434b74 are TControl::Hide / Show.
  - 0x4099d4 is the Button constructor.

## Structs
The full layouts are in the agent report, summarised here:
- **Sprite (0x384):**
  - shown@0, index@4, name@8
  - type u8@0x21 (3 = talking)
  - frame@0x24; position ×1000@0x100/0x104
  - queue[12]@0x118; sequences@0x168
  - engine@0x1d4; casts[50]@0x1f4; talk casts[50]@0x2bc
- **TalkActor (0x3cc)** adds sound pairs@0x394 and talk state@0x3ac–0x3c8.
- **Group:** name[0x1c], count@0x1c, scene@0x28, items[64]@0x2c.
- **Scene:**
  - active@0, times@4/8, tick count@0xc, return scene@0x14
  - callbacks@0x18–0x30, period@0x34
  - sprite count@0x38, name[0x100]@0x3c, engine@0x13c, sprites[255]@0x140
  - buttons: count@0x53c, [20]@0x540 (0x1c each)
- **TStage:**
  - webtrack@0; dirty areas[80]@0x18; offstage[40]@0x518
  - back / front Bitmaps@0x7a8 / 0x7e0; HDC@0x818; palette@0x81c; form@0x820; screen Rect@0x824
  - scene count@0x844; current scene@0x848; scenes[20]@0x84c
  - mouse points@0x89c–0x8c4; bpp@0x8ac; sound manager@0x8c8
- **TSoundMgr:**
  - {TSound*, keep}[100]@4; playing@0x324
  - HWAVEOUT@0x33c; WAVEHDR@0x340
  - queue[20] (0x1c each)@0x368
- **TSound:** name@1, WAVEFORMATEX@0x1a, milliseconds@0x2c, size@0x30, data@0x34.
- **TPackedResources:** HGLOBAL, header, entries (40 bytes each), used flags, count, output / input buffers, name@0x2c, cursor@0xb0.
- **TPackedStream:** output buffer@4, output size@8, z_stream@0x10, size@0x48, position@0x4c.
- **Globals:**
  - 0x4601c8 stage; 0x460208 sound manager; 0x455524 packed resources; 0x455528 TSaveParms
  - 0x4601f4 / 0x4601f8 screen DC / memory DC; 0x46020c webtrack

## Round 2

All ten remaining game functions in 0x407ffc–0x40e5ec now match: 40e0a8, 40de8c, 40d4bc, 40cbf0, 40d6f0, 40c1f0, 4092bc, 40a978, 4081b8 and 408940. There are no unmatched `kind=game` entries left in the range.

### Quirks
- **Destination-first stores (40cbf0, solved).** The callee and argument types were not the cause. The order `mov ecx,[q]; mov eax,[arg]; mov [ecx],eax` (destination pointer first) appears whenever the pointer local cannot be kept in a register:
  - `T * volatile q` gives it;
  - taking `&q` anywhere gives it;
  - a store inside a `try` region gives it, including the body of a constructor that has an EH frame, which is why the TPackedStream and TStage ctors show it.

  An ordinary pointer, reference or parameter loads the value first. 40cbf0 matches with `QEntry * volatile q`.
- **TPackedStream (VCL subclass):** `#include <vcl/classes.hpp>` works as is.
  - The C++ ctor on a TStream subclass is **cdecl**: `push args; push 1; push [VMT]`, with the ctor calling _ClassCreate itself.
  - A declared `__fastcall virtual ~TPackedStream()` is needed for the `mov word [ebp-0x14],8`.
  - The file also emits the inline `Classes::TStream::TStream()` at 0x40de8c, which matches via MATCH.
- **`new TPackedStream(...)` result** goes through a named temp: `TPackedStream *t = new ...; s = t;`.
- **Delphi virtuals and properties on real VCL objects** match directly:
  - `s->Read(&x, 4)` gives `mov ebx,[eax]; call [ebx+4]`;
  - `rs->Position -= 3` gives GetPosition/SetPosition;
  - `delete rs` gives `call [ecx-4]` with dl=3;
  - `new Classes::TResourceStream((int)HInstance, name, "Wave")` gives the AnsiString temp with its EH states.
- **`x = x / 256`** gives the `test/jns/add 0xff/sar` form. `x /= 256` gives `idiv`.
- **`size += 0x400` versus load/add/store:**
  - an int addend gives `add [mem],imm`;
  - an **unsigned** addend (`256 * sizeof(RGBQUAD)`, or `0x400u`) gives `mov edx,[m]; add edx,K; mov [m],edx`.
- **`-abs(v)`** negates in place (`neg eax`). `abs(v) * -1` (or `-1 * abs(v)`) gives `mov ecx,eax; neg ecx`, which is what 4081b8 has.
- **`if (c) d = tmax(..); else d = tmin(..);`** stores to d in both arms. A ternary leaves the first arm's result in eax.
- **BCB3 CONTROLS.HPP event offsets:** in our build, FOnMouseDown/Move/Up come out 0xc bytes later than in the game's VCL, which has them at 0x6c/0x74/0x7c. TStage (40a978) therefore uses a local `TForm` stand-in: `__closure` fields at those offsets, plus non-virtual `__fastcall` SetColor/GetClientWidth/GetClientHeight. Assigning a TStage member function to those fields produces the closure temp pair (`[ebp-0x3c]=code, [ebp-0x38]=this`).
- **TStage ctor:**
  - `Area dirty[80]` / `Area offstage[40]` with an out-of-line `Area::Area()` (0x40adc4) give `_vector_new_ldtc_`.
  - `Bitmap *pb=&back, *pf=&front;` are the same unused member-pointer locals as in the Cast ctors.
  - `dragStart = Point(0,0)` builds straight into the field (ecx = &field).
- **`Point(x/1000, y/1000)` passed by value** to `fn_402800/402820(Rect, TPoint)` gives `lea eax,[tmp]; push [eax+4]; push [eax]`. `s->pos = fn_402840(s->clip, s->pos)` needs pos to be a real TPoint (an anonymous union with x/y).
- **Inline setter** `e->SetDrag(s)` (`void SetDrag(Sprite *p){ drag = p; }`) gives the `[ebp-0xc]` parameter copy in 408940.

### Structs / layout added
- **Sprite:**
  - onCell cb@0x8c; grab TPoint@0x90 (999999999 = none); rclick@0x98
  - vx/vy@0xa0/0xa4, maxvy@0xa8, ay@0xac, dvy@0xb0, ax@0xb4, dvx@0xb8
  - fling TPoint@0xbc, dragTime@0xc4
  - bounceX/Y@0xc8/0xc9, outX/outY@0xca/0xcb
  - clip Rect@0xe0; enabled@0x148
  - callbacks onClick/onRClick/onGrab/onDrag/onDrop/onLeave@0x14c–0x160
  - draggable@0x1dd
- **Button** (4092bc): onClick/onRClick/onEnter/onLeave@0x384–0x390; blink sound@0x3a4/0x3a8; blink@0x3ac; on/off times@0x3b0/0x3b4; count@0x3b8/0x3bc; lit@0x3c0; nextTime@0x3c4; hover@0x3c8.
- **Engine:**
  - wt@0; flags 0x0c/0x0d/0x0f; color@0x14; onExit@0x79c; drag sprite@0x840
  - mouse/down/up@0x8b0/0x8b8/0x8c0; dragDelta@0x8a4
  - thread (TMyThread)@0x8d0; size 0x8d4
  - Scene period@0x34
- **TPackedStream** (0x50): out@4, outsize@8, used@0xc, z_stream@0x10, size@0x48, pos@0x4c.
- **Globals:** g_45552c BITMAPINFO buffer; g_455534/g_455538 are byte counters (source / cropped) in 40d6f0.
