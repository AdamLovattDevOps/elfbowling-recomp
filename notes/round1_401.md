# Round 1 notes, range 0x401056–0x407ffc (agent report)

## Codegen quirks (bcc32 5.3, -Od -k)
- **Trailing if/else:** as the last statement of a void function, each branch gets its own epilogue. Add an explicit `return;` after it to get `jmp end`.
- **Returning a call result:** `return c ? f() : g();` gives the jmp form. `if(c) return f(); else return g();` duplicates the epilogue.
- **For loops:**
  - bound in a local, parameter or constant: the test is at the bottom; non-constant bounds also get a test at the top;
  - bound read through a pointer or global: jmp to the test at the bottom;
  - constant bound: no initial test.
- **bool locals** load with `movsx`. `return h & v;` gives `and; setne al; and eax,1`.
- **Struct copies:** a whole-struct POINT copy is two dwords and reuses the register. Setting fields one by one reloads `[ebp+8]` each time. A 16-byte RECT copy is `rep movsd`. A struct returned by value goes through a hidden pointer at `[ebp+8]`.
- **Chained assignment:** `a = b = 0` stores b first.
- **Rect and Point:** `Classes::Rect` and `Point` are `__fastcall`; the hidden result pointer is pushed after the stack arguments. The original sometimes builds the value in a named local first.
- **min/max:** `lea reg,[a]` is an inline template taking const references; use local tmin/tmax templates.
- **Signedness:** matters for `add [g],eax` versus load/add/store, and for `sar` versus `shr`.
- **Locals** are laid out in declaration order starting at [ebp-4].
- **new:** `new T` calls `@$bnew$qui`. 0x448a94 is really `operator new[]` (libmap mislabels it as `@$bdele$qpv`).

## Structs
- **Bitmap** (0x38): w, h, bpp, RECT@0xc, RECT@0x1c, HBITMAP@0x2c, bits@0x30, BITMAPINFOHEADER*@0x34.
- **Cast** (TGraphicCast): name@6, bounds RECT@0x30, Bitmap bmp@0x40, Bitmap mask@0x78.
- **Actor:** pos POINT@0xf8, fixed-point pos (×1000)@0x100, seq queue@0x118, Seq* seqs[24]@0x168, engine@0x1d4, casts[50]@0x1f4, talk casts[50]@0x2bc.
- **Seq** (0x18): steps@0x14. **Step** (12): x, y, cast u8@8, delay u16@0xa.
- **Engine:** cast list@0x8cc (count, then items[0x200], searched with stricmp).

## Globals
- 0x4601c8 screen object (HDC at 0x818)
- 0x455524 engine/sound object
- 0x4555ac font count; 0x4601cc HFONT[]

# Round 2 (same range, agent report)

## Codegen quirks
- **Constructors:** `__InitExceptBlockLDTC` frames come from ordinary C++ constructors (`Sprite::Sprite()`, `Actor::Actor(...)`), matched with `// MATCH <addr> @Class@$bctr$q...`. A base-class ctor is a plain `push this; call; pop ecx`. Mangled names: `tagPOINT` mangles as `8tagPOINT`; list the symbols with omf.load (publics and virtuals) if unsure.
- **Cast ctors:** `mov [ebp-0x28], this+0x40; mov [ebp-0x2c], this+0x78` right after the EH setup are two unused pointer locals (`Bitmap *pb = &bmp; Bitmap *pm = &mask;`). They are probably inlined member constructors in the original.
- **Returned POINT/RECT pushed from a named local** (`push [ebp-0x38]; push [ebp-0x3c]`) versus through `lea` of a temp: use a named local (`POINT ctr = Classes_Point(...); f(this, ctr);`) for the first form and pass the call directly for the second.
- **Member functions and `this`:** in a real member, `idx < count` loads idx first and indexes `[this + idx*4]`. The extern "C" `a->count` form loads the field first. This fixed 0x4067e0 (`Actor::AddTalkingIndex`, `@Actor@AddTalkingIndex$qpxci`). When a comparison or index against a struct field comes out swapped, try a member function.
- **Compound division:** `v %= d` / `d /= 10` load the divisor first (`mov ecx,d; mov eax,v`). `v = v % d` loads v first and gives `idiv [mem]`.
- **Byte arithmetic into a char field:** `st.cast = cast + j` with an `int cast` parameter computes the value first (`mov dl,[cast]; add dl,[j]`) and the address second. With a `char` parameter the order is reversed.
- **`unsigned char` vs `char` compares:** `c->masked == 1` with a signed char gives `movsx; dec; jne`. With unsigned char it gives `cmp byte [..],1`. Cast.masked is unsigned char (0 none, 1 or 2 mask mode).
- **TRect (VCL):** struct copies that go through a stored temp pointer (`mov [ebp-n],src; mov esi,[ebp-n]; rep movsd`), and by-value RECT arguments copied with `rep movsd` to the stack, mean a type with a user copy constructor. Use `struct TRect { TRect(){} TRect(const TRect &o){ *this = o; } int left,top,right,bottom; };`. A plain RECT passes as four `push` instructions. `Classes::Rect` returning TRect as an argument is built straight in the outgoing stack slot (`add esp,-0x10; ...; lea ecx,[esp+4]`).
- **TRect to RECT:** a field-by-field copy into a temp followed by `rep movsd` into a RECT local is an inline `operator RECT() const` on TRect (fn_4037e4).
- **Frame layout:** scalar locals and copy-ctor temp pointers get the high slots in order of appearance. Aggregates (RECT/TRect/POINT locals, call-result temps) are placed below all scalars.
- **Inline min/max temps:** temps for by-reference arguments are created right to left, so in `tmin(9, v / div)` the `v/div` temp gets the higher slot. The `&r->right` argument gets a pointer temp, while `&r->left` (offset 0) uses r directly. fn_4037e4 needs a second template `if (t2 > t1) return t1; else return t2;` for its y clamp.
- **Unexplained frame padding:** fn_402a7c's second branch has 8 dead bytes before its 3-byte RGB temp. It matches with an unused `int unused[2];`.
- **Callee aliases:** when a file already defines a callee with an incompatible signature (RECT vs TRect, void vs char return, int vs POINT), declare an alias such as `fn_40252c_TRect` or `fn_406a80_p`. The call target is a fixup, so the name does not matter.

## Struct updates
- **Sprite base** (ctor 0x40609c): visible u8@0, int@4, name[0x19]@8 (strncpy 0x18), f21@0x21.
- **Actor** (0x384 bytes and up; ctors 0x4064d0/0x406520, init 0x406378): f8c@0x8c; fbc@0xbc; fc8..fcb bytes@0xc8; fcc@0xcc; clip RECT fd0@0xd0 and fe0@0xe0 (both set from engine->screen); ff0 POINT@0xf0 (999999999 = unset); queue is [12]@0x118, then f148 and f14c..f15c@0x14c, f160; f1c8 int; f1d0/f1d8/f1d9/f1dc/f1dd/f1de are bytes. The move callback f88 is `void(*)(int arg, Actor*)`. Movement uses f68 (from), f70 (to), f78 (progress 0..10000), f7c (speed), f80, f84.
- **Seq:** f10 is `SeqDone done` = `void(*)(void *scene, Actor*)`, called when the sequence ends. f0 is the repeat count, f4 the steps used.
- **Engine:** screen RECT@0x824; scene@0x848; sounds@0x8c8; casts@0x8cc.
- **Cast** (TGraphicCast): masked is unsigned char@0x1f; r20 RECT@0x20 (right/bottom are the size); hot POINT@0xb0 (hotspot). The ctors are (name, int, char) 0x4052dc, (name, int, char, char, char) 0x405380, and (name, Cast *src, int div) 0x405428 (a scaled copy, named "#src"). init is 0x4050b4(this, name, int, char, POINT, char, char).
- **TextBox : Cast** (ctor 0x4056c0): f274 line spacing int, f278 char (DrawText align).
- **Obj804** (ctor 0x40604c): f0, f804; it registers with g_455524 through fn_40dc1c.

# Round 3 (same range, agent report)

## Codegen quirks
- **Chained struct assignment from a call:** `t->f25c = t->f27c = Classes_Point(0, 0);` builds the POINT straight into f27c, then copies it loading the source base register first (`mov edx,this; mov ecx,this; mov eax,[edx+27c]; mov [ecx+25c],eax`). Two separate statements load the destination first. The same holds for RECT: `c->bounds = c->r20 = r;` copies r->r20 with `rep movsd`, then reuses `edi-0x10` as the source of the second copy.
- **Rect from a named local:** `RECT r = Classes_Rect(...); a = b = r;` gives `lea esi,[ebp-n]` from the local. Assigning the call result directly builds it inside the first field instead.
- **`while (!flag)` with a `char` flag just set to 0** is emitted as a do-while (no jump to the test). With `bool` it gets the normal jmp-to-test (fn_403f04/fn_404268). A loop whose test is at the top *and* bottom, with the body first reached by falling through (fn_405a1c), is `if (found) do { ... } while (found);`.
- **Inline member for `x == 0 ? &x : 0`:** an extra pointer local (`mov [ebp-8],c->eng`) before the compare is an inline member function called on `c->eng` (fn_4050b4: `int *pk() { return f81c == 0 ? &f81c : 0; }`).
- **Reversed min/max compare:** a clamp that loads the *second* operand first and returns the first when it is larger needs `if (t2 < t1) return t1; else return t2;` (fn_4071d4). For RECT (LONG) fields against TRect ints, pass `(const int &)field` so the template deduces.
- **Original bugs to keep:** fn_404974/fn_404c54 bound the bottom-up row scan by `r20.left`, not `r20.top`. fn_402030 tests `src[0].rgbRed == 0xff` three times.
- **`char buf[132]`** in fn_405a1c: the frame needs 0x84 bytes for the name buffer.
- **`new` with real ctors:** a local `struct GCast : Cast { GCast(Engine*, const char*, char); ... };` with ctors declared (not defined) reproduces the `new` + EH-state sequence. The EH state words match without any destructor.
- **TForm1 with the real VCL:** `#include <vcl.h>` works in the harness (the INCLUDE\VCL.H stub redirects to vcl\vcl.h). `class TForm1 : public TForm` with the three `__published` handlers and a ctor `: TForm(Owner) { fn_401508(); }` matches 0x401528, and the implicit virtual dtor matches 0x40176c. It also emits byte-identical inlines for `Forms::TForm` ctor (0x401588), `TComponent::UpdateRegistry` (0x4017b8), `DelphiInterface<IUnknown>` dtor (0x401c8c), `TForm` dtor (0x401884) and `DelphiInterface<IOleForm>` dtor (0x401a2c). The last two only fail on length: funcs.tsv runs them into the RTTI/EH data that follows. Entries were added to notes/func_ends.tsv (401884->4018dc, 401a2c->401a6c); they need a funcs.tsv rebuild, then two `// MATCH` lines in main_401508.cpp (listed in a comment there).

## Struct updates
- **Cast:** eng (Engine*) is at 0; f4 is unsigned char (1 = keep background pixels when cropping). Init 0x4050b4 is `(Cast*, Engine*, const char *name, char masked, POINT hot, char flipX, char flipY)`. The ctors in ctors_4052dc.cpp therefore really take the engine first; their parameter names there are wrong, but the code matches.
- **Engine:** f81c int (its address is passed to the packed-resource loader when it is 0), bpp@0x8ac, casts (List200*)@0x8cc.
- **TextBox:** the Cast prefix (eng, name@6, r20@0x20); f258/f259 are chars; f25c/f27c are used as POINTs.
- **Actor:** f90/f98/fa0 POINTs, fa8 int (999999999 = unset), fbc POINT, fc4 int, f148 is a char (set to 1 on reset). fn_40610c is the full reset.
- **Obj804:** f804 is the Engine* (fn_405a1c, LoadAllGraphicCasts).
- **Globals:** 0x45552c is a BITMAPINFO* (header plus 256 RGBQUADs, filled by fn_402030). 0x455530/0x455531 are the transparent/background palette indices.

# Unit counters (@Unit@Cn_m) and the project file

- **The counters are made by the linker, not the compiler.** `#pragma package(smart_init)` makes bcc32 write a single `COMENT` record (class 0xfb, subtype 0x0a, then the unit name taken from the source file name, e.g. `\x0aBar`). It adds no code or data to the .obj, so obj-level matching can never see it. That is why the earlier smart_init attempt seemed to do nothing.
- **What ILINK32 builds from that record:** ILINK32 3.0 (format strings `@%s@C%d_%d`, `@%s@B%d_%d`, `@%s@D%d_%d`, and "unitCookieSegNames" in ILINK32.DLL) synthesizes for each such unit:
  - BSS cookies `@Unit@B<mod>_<k>` (the counter dword, e.g. 0x4601c4 for Main);
  - two 16-byte code thunks, appended to the end of the unit's _TEXT contribution. The init thunk is `push ebp; mov ebp,esp; sub [cnt],1; jb; pop ebp; ret`; the exit thunk is the same with `add`;
  - `_INIT_`/`_EXIT_` table entries for both thunks at priority 0x1e.
- **Numbering:** `<mod>` is the module's index in the link order (c0w32=1, ...; Main=4, Sprites=5, ..., Preopening=12). `<k>` is a running number across the unit's cookie symbols. It starts one higher when the unit has initialized `_DATA` (a `D` cookie is added), so C4_4/C4_5 means Main has data, and C8_3/C8_4 means About has none.
- **Verified** by compiling two tiny .cpp files and linking them with `ILINK32 -aa -Tpe -m c0w32.obj Foo.obj Bar.obj,...,import32.lib cw32.lib`. With the pragma in Bar.cpp, the map shows `Bar::B3_1 Bar::B3_2 Bar::C3_3 Bar::C3_4`, and the exe contains the exact `832d..01 7202 5d c3 5d c3` / `8305..01 7202 ...` thunks. Adding `int g_d = 5;` shifts them to B3_2/B3_3/C3_4/C3_5. The test is in scratch/rmain/link/ (link.sh).
- **Side effect:** smart_init also enables smart linking, so unreferenced functions of the unit are dropped from the exe.
- **Recipe:** put `#pragma package(smart_init)` after `#pragma hdrstop` in every game unit (Main, Sprites, Init, Threads, About, Game, Opening, Preopening), as the BCB3 IDE templates do. It does not change the codegen of anything else (main_401508.cpp still matches 826/826 with it). The counter thunks stay classified `gen` in notes/data_in_code_*.txt, since they only appear after a real link.
- **Project file (Elf.cpp):** there are no `@Elf@C3_x` counters, so the project source has no smart_init. That is consistent with the IDE-generated project file.
- **0x401284 is lib code, not the project unit.** `sub [0x4601b8],1; ret` with no frame is the dcc32 (Delphi) initialization stub of SysInit.pas. The Pascal "finalization" proc (try/finally around `inc [cnt]`) at 0x401254 is its partner; libmap labels that proc `Webconst@Finalization` because every Pascal unit has the identical pattern (e.g. 0x421b5c followed by 0x421b8c `sub [0x461838],1; ret`). No `#pragma option` can produce it from C++. It is marked `gen`.
- **WinMain (0x40128c)** matches byte for byte as the stock IDE project source (src_match/elf_40128c.cpp), 190 bytes including the catch handler, which ends at 0x40134a with `ret 0x10`. It is recorded only after tools/funcs.py stops splitting at 0x40131a. func_ends.tsv already has 40128c->40134c, but funcs.py only adds starts and never removes the inner one. The `Sysutils::Exception` dtor inline (0x40142c) matches via MATCH.
