# Round 1 notes, range 0x414240–0x421b5c (agent report)

- **Units:** Game (414240–41c704), Opening, Preopening, then **zlib inflate 1.0** from 0x41e2cc to 0x421b12. zlib matched using the real source from zlib.net/fossils/zlib-0.99.tar.gz (which unpacks to zlib-1.0), with the K&R code converted to ANSI.
- **Resourcestring records** at 421b2c–421b54 are data.
- **Local layout:** POD values are placed in source order. Objects of classes with a constructor (TRect) and their temporaries come after them, in order of first appearance. Block scopes never reuse slots.
- **TRect:** use the real windows.hpp form, `TRect(){}` plus the anonymous union.
- **POINT arguments:** passing a struct versus `p.x, p.y` gives the same pushes but different register rotation afterwards. Try both.
- **Conditions and bools:**
  - `(x > 0) == false` gives `setg; and; test`.
  - `b |= expr` on a bool gives `or byte [ebp-n], reg`.
  - A comma expression in a loop condition makes a bool (`seta`) in C++.
- **Constant ternary:** `c ? A : B` compiles as `mov A; test; jcc; add B-A`. Choose the condition's polarity to match.
- **Idioms:**
  - `n++ == 0` gives `mov edx,[n]; inc [n]; test edx`.
  - Use `__abs__()`, not `abs()`, which adds a copy.
  - `random(n)` inlines with a temp.
- **Members** are cdecl with `this` pushed last. Char varargs are pushed with `movsx`.
- **Library functions libmap misses:** 0x44cda4 is `rand` and 0x44938d is `strchr`.
- **Sound calls:** `fn_40cb34(snd, scene, actor, "x.wav", vol, 0)` and `fn_40cfb0(snd, name, 0)`.
- **Scene/Game** (size 0x770): sprite count@0x38, engine@0x13c (sound@engine+0x8c8), sprites@0x140.
- **Globals:**
  - pins present @0x460578[10], knocked @0x46058c[10]
  - frames `int[12][4]` @0x4605c8
  - shuffle @0x460704[12]

## Round 2

All five remaining functions matched: 41665c (836 bytes), 416bd8 (824), 417074 (1224), 41d184 (3736), 418274 (17408). Each is in its own file: `src_match/game_41665c.cpp`, `game_416bd8.cpp`, `game_417074.cpp`, `game_418274.cpp` and `opening_41d184.cpp`. The range 0x414240–0x421b5c now has no unmatched game functions.

- **Frame layout of the scene loaders (418274, 41d184):**
  - `new Group(...)` compiler temporaries come first, at [ebp-4], [ebp-8] and so on, one per `new` in the function.
  - Next is the EH registration record, then the named POD locals in declaration order.
  - Locals declared in a `for` or a block get a fresh slot every time. Nothing is reused, even across sibling loops.
  - TRect locals and char arrays come after all PODs, in order of first appearance. char arrays count as "objects" here, alongside TRect.
- **`new Group(name, self, pattern)`:** declare `Group(const char*, void*, const char*)` without defining it, and make the struct 0x12c bytes. The EH state words (`mov word [ebp-0x58], N`) then come out right by themselves.
- **Inline member with a copied parameter:** the pattern `mov edx,[a]; mov [tmp],edx; mov ecx,[tmp]; mov eax,[b]; mov [eax+0xcc],ecx` is an inline member call `b->Link(a)` with `void Link(Spr *p) { fcc = p; }`. Each expansion gets its own stack slot.
- **Chained compound assignment:** `pos.x = x0 += -35;` is needed to get `add [m], -0x23`. Writing `x0 -= 35` gives `sub [m], 0x23`.
- **Empty-bodied if:** `if (fn_416bd8(self, pos)) { }` leaves a dangling `test al,al` with no jump, as in 417074.
- **Uninitialised Pt local passed by value:** only `.y` is set before the call, and the original pushes the garbage `.x` (416bd8). Declare `Pt q; q.y = ...;` and pass `q`.
- **fn_401d0c(Pt p, Pt a, Pt b):** it interpolates using `p.y`. The named parameters in `range_401d0c.cpp` are shifted relative to this.
- **0x449098 is memmove** and **0x449264 is strcpy.** Both show up as `_TEXT` in the disassembly.
- **Scene loader signature:** `fn(Game *self, bool loaded)`. The body is `snd = self->sprites->snd; ...sound preloads...; if (!loaded) { ... }`.
- **Workflow:** for long call runs, a script that turns disassembly into C (push list → call, stored return → assignment) got 418274 to OK after one fix pass. It is in the scratch directory, not in tools/.
