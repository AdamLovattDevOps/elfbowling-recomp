# Canonical headers (Phase A)

The headers in `include/elf/` hold one definition of each engine class, of
each game global and of each `fn_XXXXXX`. A migrated unit includes them and
has no local structs, no local `extern` declarations of callees and no local
declarations of globals. `make verify` must stay at 100 % and 0 wrong after
every file.

Status: 87 of 87 files migrated (batches A–E done). zlib_41e2cc is
third-party code with its own zlib typedefs and stays off the elf headers
(see "Batch E").

## Header map

| Header | Contents |
|---|---|
| `elf/types.h` | natively `<vcl/vcl.h>` first; `<windows.h>`, the bcc32 `DWORD_PTR`/`intptr_t`/`uintptr_t` typedefs, the Borland `ELF_CLOSURE`/`ELF_METHOD`, `TPoint`, the four rect types, `Classes::Rect/Point` and their alias spellings, `ShiftState`, the `tmin`/`tmax` variants, `ELF_AS`, `ELF_CHECK_OFS`/`ELF_CHECK_SIZE` |
| `elf/bitmap.h` | `Bitmap` (the 0x38-byte DIB), `DibFile`, `RGB` |
| `elf/cast.h` | `TGraphicCast`, `TTextCast`, `TCastMgr` |
| `elf/sprite.h` | `TSeqStep`, `TSeq`, `TSprite` (base), `TGraphicSprite`, `TButtonSprite`, `TSpriteGroup`, `SpriteCb` |
| `elf/scene.h` | `TScene`, `TSceneButton`, `SceneCb`, `SceneStartCb`, `SceneKeyCb` |
| `elf/stage.h` | `TStage`, `Area`, `TStageForm` (bcc32 stand-in; natively `Forms::TForm`), `TStageMouseEvent`/`TStageMouseMoveEvent` (`ELF_CLOSURE`), `TWebTrack` (the layout other units see, with its ctor), `StageCb` |
| `elf/sound.h` | `TSoundMgr`, `TSound`, `TSoundSlot`, `TSoundQueueEntry`, `SoundDoneCb` |
| `elf/thread.h` | `TMyThread` and `TThread_Synchronize` (VCL only: needs `<vcl/classes.hpp>` first; otherwise a forward declaration). funcs.h includes it |
| `elf/packres.h` | `TPackedResources`, `PackEntry`, `PackHdr`, `ZStream`, `TPackedStreamData`, `TPackedStream` (VCL only), `TStreamVmt`, `TSaveParms`, `ParmEntry`, `TSaveCasts`, `CastEntry`, `BmpFileHdr` |
| `elf/game.h` | every `g_XXXXXX` global (extern "C"), `FrameScore`, `PinState`, readable global macros; includes all the class headers |
| `elf/funcs.h` | one prototype per `fn_XXXXXX` (596 game functions, 18 library entry points), alias macros, the VCL methods under plain-function spellings (`TCustomForm_Close`, `TApplication_Terminate`, `TApplication_MessageBox`, `TCustomForm_SetClientWidth/Height`, `TCustomWinSocket_*`), and the `rtl_*` entry points; includes `game.h`, `cast.h` and `thread.h` |

A unit normally needs only `#include <elf/funcs.h>`.

Each struct has its offset in a comment on every field and
`ELF_CHECK_OFS`/`ELF_CHECK_SIZE` lines after it. These are `#ifdef
__BORLANDC__` negative-array typedefs, so a wrong offset is a compile error
("Array must have at least one element"). Gaps that no code touches are
`char _unkXXX[n]`; alignment padding is `char _padXXX[n]`.

## Class map

| Class | Size | Header | ctor / key functions | RTTI |
|---|---|---|---|---|
| `TStage` | 0x8d4 | stage.h | ctor 0x40a978, mouse 0x40a6e4/764/7e4, dirty 0x40ae80 | yes |
| `TScene` | 0x770 | scene.h | ctor 0x4098e4, start 0x409a6c, stop 0x409b44, find 0x40a520/0x40a598 | no (our name) |
| `TSceneButton` | 0x1c | scene.h | ctor 0x4099d4 | no (our name) |
| `TSprite` | 0x22 (base) | sprite.h | ctor 0x40609c | no (our name) |
| `TGraphicSprite` | 0x384 | sprite.h | ctors 0x4064d0 / 0x406520, init 0x406378, reset 0x40610c | yes |
| `TButtonSprite` | 0x3cc | sprite.h | ctor 0x409100, reset 0x408f84, update 0x4092bc | yes |
| `TSpriteGroup` | 0x12c | sprite.h | ctor 0x409844, members 0x40971c.. | no (our name) |
| `TSeq` / `TSeqStep` | 0x18 / 12 | sprite.h | | no |
| `TGraphicCast` | 0xb8 | cast.h | ctors 0x4052dc / 0x405380 / 0x405428, init 0x4050b4 | yes |
| `TTextCast` | 0x284 | cast.h | ctor 0x4056c0 | yes |
| `TCastMgr` | 0x808 | cast.h | ctor 0x40604c | yes |
| `Bitmap` | 0x38 | bitmap.h | create 0x4029c8, free 0x40290c | no |
| `TSoundMgr` | 0x5a8 | sound.h | ctor 0x40d1ac, queue step 0x40c880 | yes |
| `TSound` | 0x38 | sound.h | ctor 0x40c544, dtor 0x40c5dc, load 0x40c1f0 | yes |
| `TPackedResources` | 0xb8 | packres.h | ctor 0x40dce0, dtor 0x40dd90 | yes |
| `TPackedStream` | 0x50 | packres.h (VCL) | ctor 0x40e0a8, Read 0x40dee0 | yes |
| `TSaveParms` / `TSaveCasts` | | packres.h | ctors 0x40e37c / 0x40e1b0 | yes |
| `TWebTrack` | 0x18 | stage.h (layout) | ctor 0x40e888, dtor 0x40ebd0 | yes |
| `TMyThread` | TThread | thread.h (VCL) | ctor 0x40f604, Update 0x40f674, Execute 0x40f684, dtor 0x40f734 | yes |

Findings made while merging the layouts:
- **There is no separate "talking actor" class.** The 0x3cc-byte class built
  by the ctor at 0x409100 (old names `TalkActor`, and `Actor` in
  scene_409fc8, scene_40a1fc and sprites_408f84) is `TButtonSprite`. The fields
  that sprites_408f84 sets (`snd1..3`, `f3ac..f3c8`) are the hover, click and
  blink state that 0x4092bc reads. `TSprite::type == 3` means a button. The
  "talking" part of a sprite is `TGraphicSprite::talkCasts` (0x2bc) and
  `AddTalkingIndex` (0x4067e0).
- The object at `TStage+0x8cc` is `TCastMgr` (old `Obj804`, `List200`; init_40f350
  called it `music`).
- `fn_409fa8`/`fn_409fb8` (sprites_409fa8.cpp, old `S24`) set
  `TScene::onKeyDown`/`onKeyUp`. They are not sprite functions.
- `TStage::down` (0x8b8) and `up` (0x8c0) are the mouse-down and mouse-up
  positions (engine_40a6e4 writes them). sprites_4092bc called them
  `lclick`/`rclick`, and menu_41008c called 0x8c0 `mouse`.
- `TButtonSprite` 0x384 fires on `down` and 0x388 on `up` (named `onPress` and
  `onRelease`). menu, about and game_418274 store their click handler at 0x388.
- The engine calls every `SpriteCb` as `(stage->scene, sprite)`. For that
  reason `SpriteCb` is `void (*)(TScene *, TGraphicSprite *)`, and the game's
  handlers `fn(TScene *self, TGraphicSprite *s)` need no casts.

## Name map (old local name → canonical)

| Old | Canonical | Exceptions |
|---|---|---|
| `Engine`, `Stage`, `App`, `GameCore`, `MenuApp`, `GameObj`, `Sprites` (game_*/opening_*: the object at scene+0x13c) | `TStage` | |
| `Screen` | `TScene` | range_401d0c: `TStage` |
| `Game` | `TScene` | init_40f350: `TStage`. unit_411 also passed a `Game *` to `fn_40ae30`, and that one is the stage |
| `Scene`, `ScreenObj`, `MenuScreen` | `TScene` | |
| `Sprite`, `Spr`, `Sprite2`, `TextSprite`, `MenuText`, `LaneSprite`, `EggSprite`, `Key`, `Spr160`, `Obj7a` | `TGraphicSprite` | |
| `Actor` | `TGraphicSprite` | scene_409fc8, scene_40a1fc, sprites_408f84: `TButtonSprite` |
| `TalkActor`, `MenuButton`, `Btn`, `Button` | `TButtonSprite` | scene_409844, scene_4099dc, scene_409c50: `Button` is `TSceneButton` |
| `S24` | `TScene` | |
| `Cast`, `GCast`, `Image`, `Bmp` | `TGraphicCast` | packres `Bmp` is a cast seen at +0x40 (`bmp.width`) |
| `TextBox`, `Img`, `Frame` | `TTextCast` | sprites_407ffc/408940: `Frame` is `TSeq` |
| `Seq`, `Step` | `TSeq`, `TSeqStep` | |
| `Group`, `SpriteGroup`, `TextGroup`, `LaneGroup`, `Group160` | `TSpriteGroup` | |
| `Player`, `Sounds` | `TSoundMgr` | |
| `Sound`, `SndEntry`, `QEntry` | `TSound`, `TSoundSlot`, `TSoundQueueEntry` | |
| `PackRes`, `PackedFile` | `TPackedResources` | |
| `PStream` | `TPackedStreamData` | the VCL units keep `TPackedStream` |
| `Obj804`, `List200` | `TCastMgr` | |
| `WebTrack` | `TWebTrack` | |
| `SaveParms`, `SaveCasts` | `TSaveParms`, `TSaveCasts` | |
| `TStream` (the local one-field VMT struct) | `TStreamVmt` | VCL units: `Classes::TStream` |
| `Area`, `Empty` | `Area` | |
| `Pin`, `Slot` | `PinState` | |
| `Control` (engine_40a844), `TForm` (engine_40a978) | `TStageForm` | |
| `BmpHdr` | `BmpFileHdr` | packres_40d6f0's `BmpHdr` is the same bytes read at +0x14 = `bi.biWidth` |
| `Pt`, `tagPOINT`, `POINT` | `TPoint` (= `tagPOINT`) | |
| `Rect` with `Rect(const Rect&)` | `ERect` | |
| `Rect` POD (`int l,t,r,b`), `Rc` | `RectPod` | |
| `TRect` with `TRect(const TRect&)` (0x401-0x407 units) | `ERect` | |
| `TRect` windows.hpp form (union, no copy ctor) | `TRect` = `Windows::TRect` | |
| `SprFn`, `TimerFn`, `SpriteCb`, `SeqDone`, `BtnCb`, `BtnFn` | `SpriteCb` | |
| `DoneCb` | `SoundDoneCb` | |
| `MoveDone` | `SpriteCb` (`onMoveDone` is called as `(scene, sprite)`; `fn_406ac4` takes the `TScene *`) | |
| `SceneCb`, `SceneFn` | `SceneCb` | |
| `SceneCb2` | `SceneStartCb` | |
| `KeyCb`, `KeyFn`, `KeyFn2` | `SceneKeyCb` | KeyFn has a different parameter spelling (`int` for the ShiftState, a pointer for the key). Check the call-site bytes |
| `EngineCb` | `StageCb` | |
| `ReadFn` | `TStreamReadFn` | |

Field names: every field comment says `(old: ...)` with the old spellings,
for example `TGraphicSprite::lane (old: f1c8)`, `TScene::stage (old: engine,
app, sprites)` and `TStage::sound (old: snd, sounds, f8c8)`. Search the header
for the old name.

Globals: use the address name. Names made from a word and an address map as
follows: `g_group_XXXXXX` → `g_XXXXXX`, `g_slots_460240` → `g_460240`,
`g_present_460578` → `g_460578`, `g_flag_4606f8` → `g_4606f8`,
`g_order_460704` → `g_460704`, `g_map_458840` → `g_458840`, `g_lines_45aaf4` →
`g_45aaf4`, `g_frames_4605c8[i][k]` → `g_4605c8[i].v[k]`, and
`g_frame9_460658` → `g_4605c8[9].v` (and so on for frames 10 and 11).

## Rect types (types.h)

bcc32 -Od generates different code depending on how a rect type is
declared, even when the layout is the same. For that reason the headers have
four rect types:

| Type | Declaration | Use it when |
|---|---|---|
| `RECT` | Win32 POD, LONG fields | a by-value argument is pushed as four dwords |
| `RectPod` | POD, `int` fields, names `l,t,r,b` and `left,top,right,bottom` | same code as RECT, but int (for `tmin` deduction) and the short names |
| `ERect` | empty ctor plus a **user copy ctor**, both name sets, `operator RECT() const` | by-value arguments are copied with `rep movsd`, `ERect r = *p` goes through a pointer temp, returns build in place |
| `TRect` (`Windows::TRect`) | windows.hpp form: empty ctor, union (`Left..`, `TopLeft/BottomRight`, plus the lower-case aliases) | results of `Classes::Rect`, and the game, egg and score units |

Each struct field has exactly one of these types (see the field comment).
When a unit needs a different one, reinterpret the lvalue with
`ELF_AS(ERect, c->bounds)`. This expands to `*(ERect *)&(c->bounds)` and
generates the same bytes (verified in sprites_4092bc). Do not add inline
accessor functions for this: an inline member call adds a `this` temp and
changes the code.

The copy-ctor type is called `ERect`, not `Rect`, because the VCL units see
the function `Classes::Rect` through `using namespace Classes`.

`Classes::Rect`/`Classes::Point` are declared only when `<vcl/classes.hpp>`
has not been included. For the other return spellings, use `Classes_Rect`
(RECT), `Classes_TRect` (ERect) or `Classes_Point`, or `fn_43a8dc` (ERect) and
`fn_43a8c4` (TPoint) from funcs.h.

## min/max templates (types.h)

| Name | Body | Old spellings |
|---|---|---|
| `tmin` / `tmax` | `a < b ? a : b` / `a > b ? a : b` | ternary `tmin`/`tmax` |
| `tmax_lt` | `a < b ? b : a` | packres_40d250, packres_40d3f8 `tmax` |
| `tmin_if` / `tmax_if` | `if (t1 < t2) return t1; else return t2;` | range_401d0c/403540/4060d4 `tmin`/`tmax`; `<stdlib.h>` `min`/`max` have this body too |
| `tmin_rev` | `if (t2 > t1) return t1; else return t2;` | range_403540 `tmin2` |
| `tmax_rev` | `if (t2 < t1) return t1; else return t2;` | range_4060d4 `tmaxr` |

Rename each use to the variant with the same body. Keep `<stdlib.h>` where
the unit calls `min`, `max`, `abs` or `random`.

## funcs.h

funcs.h is **hand-maintained from now on.** The one-off generator used for
the first version is in `scratch/ph_a1/` (genfuncs.py, emit.py,
write_funcs.py). It reads the pre-migration copies in
`scratch/ph_a1/orig_src/`. scratch/ is git-ignored, so treat the generator as a
local reference only.

Each entry has this shape:

```
// 0x40a598  member TScene::fn_40a598 (MATCH); def scene_40a520.cpp
//   alt: void *fn_40a598(TScene *, const char*)  [menu_41008c, score_4120cc]
extern "C" TGraphicSprite *fn_40a598(TScene *self, const char *nm);
#define Scene_FindSprite fn_40a598
```

The `alt:` lines are the other spellings found in the pre-migration sources,
with the units that used them. They tell you where to expect a conflict.

When a unit's use conflicts with funcs.h:
1. **Pointer types only** (`void *` against `TScene *`, or `Spr *` against
   `TGraphicSprite *`): change the unit. Change the definition's parameter
   type, or cast at the call. A pointer cast changes no code.
2. **The definition has fewer parameters than the callers pass**, and the
   extras are unused: add the parameters to the definition. cdecl callees
   ignore them, and no code changes (for example `fn_4169a0(TScene *s)` and
   `fn_4092bc(..., char *unused)`).
3. **The definition's spelling is more accurate than funcs.h** (for example a
   pointer where funcs.h has `int`): edit funcs.h. First grep every migrated
   unit that calls the function, then rebuild those units.
4. **The call-site code really differs** (a TPoint by value against two
   ints, a `char` return against a `bool` or `int` one that the caller tests,
   `RECT` against `ERect` by value): keep a local **alias** declaration in
   the unit, `extern "C" <sig> fn_XXXXXX_<suffix>(...);`, and list it below.
   The call target is a fixup, and xref_check.py matches the `fn_XXXXXX`
   prefix, so the alias is still checked.
5. Member-matched functions (`TScene::fn_40a598`, `TStage::fn_40ae80`, ...)
   also have an extern "C" prototype. It is the same ABI (cdecl, `this`
   pushed last) and is what other units call. Inside the class's own member
   bodies, an unqualified call resolves to the member, which is what the
   matching code wants.

Current aliases (all in range_401d0c.cpp): `fn_40252c_TRect(ERect, ERect)`
and `fn_402e30_t(SIZE *, ERect *)`.

Batch B aliases (the ERect copy-ctor spelling, where funcs.h has RECT or RectPod):

| Alias | Units |
|---|---|
| `char fn_402580_E(ERect, ERect)` | engine_40ae80 |
| `ERect fn_4025e0_E(ERect, ERect)` | engine_40ae80, engine_40af80 |
| `char fn_40252c_E(ERect, ERect)` | engine_40af80, scene_409c50 |
| `ERect fn_40a5d4_E(TStage *, ERect &)` | engine_40af80, engine_40b8fc, engine_40ba68 |
| `ERect fn_40a63c_E(TStage *, ERect &)` | engine_40af80 |

Hand-set entries (marked `hand-set:` in funcs.h): 0x409fa8/0x409fb8 (take a
`SceneKeyCb`), 0x410968/0x410978 (take a `TGraphicSprite *`), and
0x411bdc..0x4120b4 (take a `TScene *`).

## Migration procedure (per file)

1. Copy the file to scratch and work there:
   `cp src_match/F.cpp scratch/<you>/F.cpp`. Keep a pristine copy of the
   original.
2. Remove the local `extern` declarations that the headers now provide. The
   helper `.venv/bin/python scratch/ph_a1/strip.py IN OUT` deletes every
   single-line `extern` of a canonical `fn_XXXXXX`, `rtl_*` or `g_XXXXXX`, and
   keeps aliases and anything unknown. It prints what it removed.
3. Delete the local structs, typedefs and templates. Add
   `#include <elf/funcs.h>`, and keep the unit's `<stdlib.h>` and
   `<string.h>`. VCL units put `<vcl/classes.hpp>` (or `<vcl.h>`) **before**
   the elf headers.
4. Rename the types (name map), fields (`old:` comments), globals (address
   names) and templates (variant table). The regex-driven edits used for the
   pilots are a good start. `\bSpr\b` → `TGraphicSprite` and similar
   renames are safe.
5. Fix the type conflicts:
   - `void *` sprite/scene locals → typed pointers;
   - callbacks: `(void *)fn_x` → `(SpriteCb)fn_x` (or `SceneCb`, `SceneKeyCb`).
     No cast is needed when the handler is already `fn(TScene *, TGraphicSprite *)`;
   - `bool` against `char` fields and globals: try the canonical type first
     (game_414240 matched with `bool g_460578[]` where it had `char`). If a
     byte store or compare changes, use `ELF_AS(char, x)` at that use;
   - `unsigned char` must stay unsigned where the code compares it (`cmp byte`
     against `movsx`): `TSprite::type`, `TGraphicCast::masked`,
     `PinState::state`, `TSeqStep::cast`;
   - rect flavour: `ELF_AS(...)` (see above);
   - text casts: `(TTextCast *)spr->casts[spr->frame]`;
   - byte arithmetic on a cast pointer (`img[i] + 0x30`): `(char *)s->casts[i] + 0x30`.
6. Do not redeclare libc functions (`stricmp`, `malloc`, `memset`, ...).
   `<windows.h>` via types.h already declares several of them, and a second
   `extern "C"` declaration with another signature is an error. Use the libc
   name or the `rtl_*` alias from funcs.h, as the original file did.
7. Member-matched classes: the mangled names change with the class and
   parameter types. Run `.venv/bin/python tools/syms.py scratch/<you>/F.cpp`
   and update every `// MATCH` line (for example `@Group@fn_40971c$qv` →
   `@TSpriteGroup@fn_40971c$qv`). Ctor parameter lists in the headers are
   the real ones, which sometimes differ from the old local spelling (for
   example `TGraphicCast(TStage *, const char *, char)` for the old
   `Cast(const char *, int, char)`). The pushes are the same.
8. Check the file:
   `.venv/bin/python tools/match.py scratch/<you>/F.cpp --no-record`. It
   must show the same number of `OK` lines as the original and no DIFF.
   Then copy it to src_match/ and run
   `.venv/bin/python tools/match.py src_match/F.cpp && .venv/bin/python tools/xref_check.py src_match/F.cpp`.
   Finish with `make verify`.
9. Save the pristine original to `scratch/ph_a1/orig_src/F.cpp`, so the
   pre-migration spellings stay available.

Note: match.py compiles to `build/obj/<basename>.obj`, so a scratch file
with the same basename overwrites the src_match object. Always rerun match on
the src_match file before running xref_check.py.

## bcc32 quirks found during the pilot

- **Frame primer (fn_402a7c).** The gap above fn_402a7c's 3-byte `RGB`
  local (and so its `add esp,-N`) depends on compiler state left by the
  functions compiled before it in the unit. The dependence is chaotic: an
  extra empty `#include`, or `#include <windows.h>` before `<elf/funcs.h>`,
  moves it between 0x2c, 0x30, 0x6c, 0x70 and 0x80. The old `int unused[2]`
  was an accidental fit for the original file's state. A non-game function
  with one `int` local placed just before it (`elf_frame_primer_402a7c` in
  range_401d0c.cpp; match.py ignores names that are not `fn_`) fixed it for
  batches A–D, but batch E's two new `#define`s in types.h (any macro, even
  unused) moved the frame to 0x80 again. **Batch E fix:** the 8 bytes are two
  separate `int` locals (`int unused1; int unused2;`) instead of `int
  unused[2]`. That form gives 0x2c with the old and the new types.h, with and
  without the primer (the primer is kept). If another function's frame
  size changes after a header change and nothing else differs, try splitting
  its arrays into scalars, or the same primer.
- **Include order.** VCL headers must come first. bcc32 5.3 has no `#pragma
  once`, so include guards do all the work. The headers can be included in
  any order and more than once.
- **Unused parameters are free.** Adding an unused trailing parameter to a
  cdecl definition does not change its bytes (checked on fn_4169a0 and
  fn_4092bc).
- **Unions and extra name sets are free.** Anonymous unions that give a field
  two names (`FrameScore`, `ERect`, `TRect` lower-case aliases, the
  `TGraphicSprite` 0x0a0/0x100 unions) change no code.
- **Declarations are free.** The ~600 prototypes, the globals and the class
  declarations with ctors changed no code in any pilot file (apart from the
  frame primer case above).

## Pilot migration

| File | Resolutions |
|---|---|
| engine_40a5d4.cpp | POD rect → `RectPod`; `ox/oy` → `offset.x/.y` |
| sound_40c5dc.cpp | `qarg/qsprite/qcb/f374` → `queue[0].owner/.sprite/.cb/.seq`; `fn_40c880` returns char in funcs.h, but the caller ignores the result |
| scene_4099dc.cpp | `f8b8..f8c4` → `down.x/.y, up.x/.y`; `fn_408f84((TButtonSprite *)s)` |
| unit_411.cpp | `Game`/`Key` → `TScene`/`TGraphicSprite`; `fn_40bf2c(g->stage, ...)`; text casts; dropped four unused declarations |
| range_401d0c.cpp | local copy-ctor `TRect` → `ERect` (aliases kept); `tmin/tmax` → `tmin_if/tmax_if`; `TStreamVmt`; frame primer for fn_402a7c |
| menu_41008c.cpp | callbacks → `SpriteCb`/`SceneCb`/`SceneKeyCb`; `btn->onDown/onClick` → `onEnter/onRelease`; `((MenuApp*)s->app)->f10` → `s->stage->bgimage`; `crew->game->mouse` → `crew->stage->up`; `(RECT *)&r` for fn_407a98 |
| game_414240.cpp | 64 declarations removed; all globals renamed to address names (`g_4605c8[9].v`, ...); `void *self` → `TScene *self`; `fn_4169a0` gained its unused `TScene *` |
| sprites_40971c.cpp | member class `Group` → `TSpriteGroup`; MATCH names updated |
| scene_40a520.cpp | member class `Scene` → `TScene`; MATCH names updated; local `stricmp` declaration dropped |
| packres_40d250.cpp | member class `PackRes` → `TPackedResources`; `tmax` → `tmax_lt` |
| sprites_4092bc.cpp | `ERect r = ELF_AS(ERect, c->bounds)`; `lclick/rclick` → `down/up`; `onClick/onRClick/onLeave` → `onPress/onRelease/onExit`; unused 6th parameter |

## Batch B migration (stage and scene)

All 24 files are migrated and match (`make verify` 100 %, 0 xref wrong).

| File | Resolutions |
|---|---|
| engine_40a6e4 | `f840` → `drag` |
| engine_40a844 | `Control` → `TStageForm`; `ox/oy` → `offset`; gained the unused `int color` the ctor passes (funcs.h spelling) |
| engine_40a948 | `hidTaskbar` → `fullscreen` |
| engine_40a978 (VCL) | local `TForm` → `TStageForm` (stand-in in stage.h); `Player`/`Obj804`/`WebTrack` → `TSoundMgr`/`TCastMgr`/`TWebTrack`; `onExit` is `StageCb`; MATCH `@TStage@$bctr$qp10TStageFormiicpqv$vp9TWebTrack`. The local `TMyThread` class stays (resolved in batch E) |
| engine_40adc4 | `Empty` → `Area`; MATCH `@Area@$bctr$qv` |
| engine_40ae80 | member class → `TStage`; `areas[i]` → `ELF_AS(ERect, dirty[i])`; `backbmp` → `back.handle`; `stricmp` → `rtl_stricmp`; aliases `fn_402580_E`, `fn_4025e0_E` |
| engine_40af80 | `areas2` → `ELF_AS(ERect, offstage[i])`, `ELF_AS(ERect, screen)`; four `_E` aliases |
| engine_40b158 | `Image` → `TGraphicCast` (`f4` → `keepBg`, `&img->bmp`); `fn_40b2ec` gained the unused `char f` that engine_40b5bc passes; pointer casts for `fn_402ed4` |
| engine_40b3b0 | `Cast` → `TGraphicCast`; `dirty2` → `dirtyFlag2` |
| engine_40b5bc | `img[]`/`ovl[]` + 0x30 → `&casts[i]->bounds` / `&talkCasts[i]->bounds` as `ERect *`; `f28/f3c` → `animating/phase`; `(RECT *)` for fn_40343c/fn_403048 |
| engine_40b8fc | `f7a0` → `next`; `sounds` → `sound`; alias `fn_40a5d4_E` |
| engine_40ba68 | `ELF_AS(ERect, sp->casts[f]->bounds)`; `frontbmp` → `front.handle`; `ELF_AS(TPoint, q)` for fn_40b8fc |
| engine_40bd3c | `KeyCb` → `SceneKeyCb` |
| engine_40bdec | `f83e` → `mousedown` |
| engine_40bfe4 | `SaveParms` → `TSaveParms`; `f818` → `dc` |
| engine_40c130 | `onStop` → `onExit` |
| stage_40ae30 | `nitems/items` → `nscenes/scenes`; `void *` → `TScene *` |
| scene_409844 | member classes → `TSpriteGroup`, `TScene`; `f18/f1c` → `onDown/onUp`; `onStart = (SceneStartCb)start` |
| scene_409c50 | `Button` → `TSceneButton`; `Ctl` → `TSceneCtl` (new stand-in in scene.h); alias `fn_40252c_E` |
| scene_409dac | `f8b8..f8c4` → `down/up`; `f1dd/f1de` → `draggable/clickable`; `(TButtonSprite *)` for fn_4092bc |
| scene_409fc8 | `Sprite`/`Actor` → `TGraphicSprite`/`TButtonSprite`; `fn_406780(s, cast, p)` with the TPoint by value (same bytes as two ints); `fpos` → `f0f0`; the `int a` of fn_40a288/fn_40a2c0 is the `const char *name` |
| scene_40a0d8 | `img[i] + 6` → `casts[i]->name` |
| scene_40a1fc | `Actor` → `TButtonSprite`; MATCH `@TScene@fn_40a1fc$qpxct1t18tagPOINT` |
| scene_40a3a4 | `TextBox` → `TTextCast`; `ref->x/y` from the 0x100 union |

Header changes (all backward compatible):
- scene.h: `TScene` 0x18/0x1c are unions `load`/`onDown` and `f1c`/`onUp`
  (fn_409dac calls them for a background click). New `TSceneCtl` stand-in;
  `TSceneButton::ctl` is `TSceneCtl *` (was `void *`; no unit assigned it).
  `TScene::fn_40a1fc` takes `const char *name` (was `int a`). The ctor's
  parameters are renamed `start`/`start2` (the types are the same).
- stage.h: `TStage::pk()` (above); comments on `palette`, `onExit` and
  `TStageForm`.
- funcs.h (own entries only): `fn_40a1fc`, `fn_40a288`, `fn_40a2c0` take
  `const char *name`; resolved `alt:` lines of batch B units removed.

### Notes (batch B)

None left: TMyThread moved to elf/thread.h and the TScene ctor takes a
`SceneStartCb` (both batch E, see there).

## Batch A migration (sprites and casts)

All 10 files are migrated and match (`make verify` 100 %, 0 xref wrong).
None of them has a pointer-to-int cast left, so no `intptr_t` was needed.

| File | Resolutions |
|---|---|
| sprites_409fa8 | `S24` → `TScene`; `f24/f28` → `onKeyDown/onKeyUp` (`SceneKeyCb`) |
| sprites_4096dc | `Group`/`Sprite` → `TSpriteGroup`/`TGraphicSprite`; `fn_401f30(msg, g->name)` (funcs.h takes `const char *`; name is at +0) |
| sprites_409100 | `TalkActor` → `TButtonSprite`; `f384..f390` → `onPress/onRelease/onEnter/onExit`; MATCH `@TButtonSprite@$bctr$qp6TStagepxct2t28tagPOINT` |
| sprites_408f84 | `Actor` → `TButtonSprite`; `snd1/2/3` → `hoverSnd/clickSnd/blinkSnd`; `f3ac..f3c8` → `blink..hover`; `Sounds::f598..` → `TSoundMgr::defA..`; `fn_409068` takes `const char *id` (like `fn_4090b4`) |
| range_401cec | stubs use the funcs.h parameter lists; `fn_402380` gained its unused `who`; `fn_4027d0/402800/402820` are `char (ERect, TPoint)` (same bytes as `int (RECT, int, int)`); the VCL `TApplication_MessageBox`/`p_Application` stay local (resolved in batch E) |
| ctors_4052dc | `Obj804`/`Cast`/`TextBox`/`Sprite`/`Actor` → `TCastMgr`/`TGraphicCast`/`TTextCast`/`TSprite`/`TGraphicSprite`; the old first `name` argument is the stage; MATCH names updated (`@TGraphicCast@$bctr$qp6TStagepxcc`, ...) |
| range_4060d4 | `Actor` → `TGraphicSprite` with all field renames; local copy-ctor `TRect` → `ERect`; `(TRect *)&x` → `ELF_AS(ERect, x)`; `fe0 = screen` → `ELF_AS(RECT, a->clip) = ELF_AS(RECT, e->screen)`; `fn_406a80` takes a `TPoint`; `fn_406ac4` takes the `TScene *` and calls `onMoveDone` directly (no `MoveDone` cast); `fn_406a08/406a44` take `const char *` and cast the cast to `TTextCast *`; `fn_407a68` takes a `TRect` and stores `ELF_AS(RECT, r)`; `tmin/tmax/tmaxr` → `tmin_if/tmax_if/tmax_rev`; aliases `fn_402748_t`, `fn_40278c_t` |
| sprites_407ffc | `Sprite` → `TGraphicSprite`; `frames/f10` → `seqs/done`; `f848` → `stage->scene`; `f1f4[i] + 0x30` → `&ELF_AS(ERect, casts[i]->bounds)`; `ELF_AS(ERect, s->clip)` for the by-value rects; `ELF_AS(bool, s->shown)`; `fn_407ffc` takes a `SpriteCb`, `fn_4087c8` a `TScene *`; alias `fn_402840_E` |
| sprites_408940 | as sprites_407ffc; `Engine::SetDrag` → `TStage::SetDrag`; gained the unused 6th `char *` (funcs.h); alias `fn_4028b8_E` |
| range_403540 | `Cast`/`TextBox`/`List200`/`Obj804`/`GCast` → `TGraphicCast`/`TTextCast`/`TCastMgr`/`TCastMgr`/`TGraphicCast`; `f25c/f260` → `f25c.x/.y` (TPoint); `fn_4059a0` takes the cast (no `(int)c`); `fn_4052ac` is `(cast, void *pal, name, flipX, flipY)` and gets `stage->pk()`; `(TPoint *)&t->bounds` for fn_402ed4's dest point; `tmin/tmin2` → `tmin_if/tmin_rev`; alias `fn_402e30_t` |

Batch A aliases:

| Alias | Units |
|---|---|
| `ERect fn_402748_t(ERect, ERect)`, `ERect fn_40278c_t(ERect, ERect)` | range_4060d4 |
| `void fn_402e30_t(SIZE *, ERect *)` | range_403540 (and range_401d0c) |
| `TPoint fn_402840_E(ERect, TPoint)` | sprites_407ffc |
| `TPoint fn_4028b8_E(ERect, TPoint)` | sprites_408940 |

Header changes (all backward compatible):
- sprite.h, cast.h: comments only (`seqs[24]` confirmed, `onMoveDone`,
  `MoveDoneCb` now unused but kept, `TSprite::shown/type`, `TCastMgr::items`).
- funcs.h (own entries only, all marked `hand-set (batch A)`):
  `fn_409068(TButtonSprite *, const char *id, int)` (was `int id`; menu_41008c
  passes `0`); `fn_406ac4(TGraphicSprite *, TScene *)` (was `int`);
  `fn_407ffc(TGraphicSprite *, int, SpriteCb)` (was `int`);
  `fn_407a44(TGraphicSprite *, TPoint, int, char, char)` (was `..., int, int, int`:
  the definition reads the last two as bytes; callers pass literals);
  `fn_4052ac(TGraphicCast *, void *pal, const char *name, char, char)` (was
  `char *, int, int, char, char`); `fn_4059a0(TCastMgr *, TGraphicCast *)`
  (was `int`). No other unit called these with an incompatible spelling when
  `make verify` ran.

### Notes (batch A, no action needed)

The VCL shell item (TApplication_MessageBox / p_Application) was resolved in
batch E. What is left is informational:

- **fn_402840 / fn_4028b8 / fn_402748 / fn_40278c (range_401d0c.cpp):** every
  sprite caller passes the ERect copy-ctor spelling, so four aliases exist.
  No action needed unless someone wants the funcs.h entries to carry ERect
  instead of RECT (that would move the aliases to range_401d0c instead).
- **fn_402ed4 (range_401d0c):** funcs.h has `TPoint *dp`; range_403540 passes
  `&t->bounds` (a RECT) and casts. Fine as is.

## Batch C migration (sound and packed resources)

All 21 files are migrated and match (`make verify` 100 %, 0 xref wrong). No
aliases were needed.

| File | Resolutions |
|---|---|
| sound_40c1f0 (VCL) | `Sound` → `TSound`; `(int)HInstance` → `(intptr_t)HInstance`; `fn_40d568` takes the `Classes::TStream *` directly (now `void *s`) |
| sound_40c544 | `Sound`/`Player` → `TSound`/`TSoundMgr`; `f0/f32c` → `used/endTime`; MATCH `@TSound@$bctr$qpxc`, `@TSoundMgr@$bctr$qv` |
| sound_40c880 | member class → `TSoundMgr`; MATCH `@TSoundMgr@fn_40c880$qc` |
| sound_40c960 | `(DWORD)fn_40c718` → `(DWORD_PTR)fn_40c718`; `hdr.lpData = (char *)s->data` |
| sound_40ca94 | `Player`/`Sprite` → `TSoundMgr`/`TGraphicSprite`; `f1cc` → `period`; `f32c` → `endTime`; `int owner/cb` → `void *`/`SoundDoneCb`; `fn_40cbd0` gained its unused `void *` (callers pass the scene); `stricmp` → `rtl_stricmp` |
| sound_40cbf0 | `QEntry * volatile q` → `TSoundQueueEntry * volatile q` (kept volatile) |
| sound_40cf24 | `delete`/`new TSound`; `fn_40cfb0` is `int (TSoundMgr *, const char *, char keep)` |
| packres_40d2c8 | `(PackHdr *)LockResource(...)` |
| packres_40d3f8 | unused local `tmax` dropped; `<stdlib.h>` for malloc/free; `stricmp` → `rtl_stricmp` |
| packres_40d4bc (VCL) | `data + packed`/`offset` → `(char *)hdr + dataOffset`/`csize` (see PackEntry below) |
| packres_40d578 | `Bmp` → `TGraphicCast` (`b->bmp.width/height/bpp/bits`); `line` → `inbuf`; `(DibFile *)pal` for fn_402a58 |
| packres_40d6f0 | `Clip` → `CastEntry` (`r1c/src` → `rect/keep`); `BmpHdr` → `BmpFileHdr` (`h->bi.biWidth/biHeight/biBitCount`); `r4c/r5c` → `bmp.r0c/bmp.r1c` |
| packres_40d980 | `(TStreamVmt *)fn_40d4bc(...)`; `hbmp` → `bmp.handle`; `(HPALETTE *)pal` for fn_402030 |
| packres_40db00 | `stricmp` → `rtl_stricmp` |
| packres_40dbe0 | `fn_40dbe0` returns the `char` of fn_40db00 (same bytes; range_403540 tests it); `PStream` → `TPackedStreamData` (`f8` → `outsize`); `tmin/tmax` ternary; `fn_40e354`/`fn_40e55c` gained their unused second parameters; `f1d8` → `keepParms` |
| packres_40dc1c | `new TSaveCasts()` |
| packres_40dce0 | member classes → `TPackedResources`/`TSaveCasts`/`TSaveParms`; `f1c/f24` → `outbuf/inbuf`; MATCH names updated; local `g_45c41c` kept (resolved in batch E) |
| packres_40dee0 | `PStream` → `TPackedStreamData` |
| packres_40e0a8 (VCL) | class from packres.h; **also compares 0x40f104** (see below) |
| packres_40f104, packres_40f2a4 (VCL) | hand-copied `System::TObject`/`Classes::TStream`/`TCustomMemoryStream`/`TResourceStream` and the stub `TPackedStream` replaced by the real `<vcl/classes.hpp>` and packres.h |

**TPackedStream dtor (0x40f104).** It is compiler-generated: an explicit
`~TPackedStream() {}` compiles to 88 bytes against the original 76. packres.h
therefore no longer declares a dtor, and the implicit one is emitted by the
unit that defines the ctor (packres_40e0a8), which now carries `// MATCH 40f104
@TPackedStream@$bdtr$qqrv`. packres_40f104 keeps the TStream dtor (0x40f1e4).
The two other VCL users of the class (packres_40d4bc, sound_40c1f0) still
match without the declaration.

Header changes (all backward compatible):
- sound.h: `TSoundMgr::loopOwner` is `void *` (was `int`); comments on
  `SoundDoneCb`, `owner`, `cb`; extra `ELF_CHECK_*` lines. Guarded
  `DWORD_PTR`/`intptr_t`/`uintptr_t` typedefs for bcc32 (BCB3 has neither
  `<basetsd.h>` nor `<stdint.h>`); moved to types.h in batch E.
- packres.h: `PackEntry` 0x18/0x20 are `csize`/`dataOffset` (the draft had
  `offset`/`packed`, swapped against tools/unpack.py and fn_40d4bc; no other
  unit used them); `PackHdr::dataStart` (0x14); `CastEntry` has `name[0x1c]`,
  `RECT rect` (0x1c) and `RECT keep` (0x2c); `TPackedStream` has no declared
  dtor (above).
- funcs.h (own entries only): `void *owner` and `SoundDoneCb cb` in the sound
  play functions (above); `fn_40cbd0(TSoundMgr *, void *unused)`;
  `int fn_40cfb0(TSoundMgr *, const char *, char keep)` (was `void (void *,
  const char *, int)`; callers pass literals); `int fn_40d568(TPackedResources
  *, void *s)` (was `TStreamVmt *`); `char fn_40dbe0(TPackedResources *, const
  char *ext, char *out)` (was `void`). The not-yet-migrated game units that call
  `fn_40cfb0(void *snd, ...)` must pass the `TSoundMgr *` (`stage->sound`)
  when they migrate.

### Notes (batch C)

The types.h, game.h (`g_45c41c`) and sound_40c5dc (`DWORD_PTR`) requests
were resolved in batch E. One note stays:

- **SoundDoneCb (sound.h, no change made):** it stays `(void *,
  TGraphicSprite *)` because sound_40c5dc calls it with a `void *`. No caller
  passes a non-null callback, so the handler type is unconfirmed.

## Batch D migration (game screens)

All 13 files are migrated and match (`make verify` 100 %, 0 xref wrong). None
of them has a pointer-to-int cast, and the only runtime calls left are ANSI C
(`rand`, `srand`, `time`, `abs`, `strcpy`, `strchr`, `strlen`, `memmove`), so
no `intptr_t` or `rtl_*` change was needed. The globals use the readable
game.h names (`g_frame`, `g_roll`, `g_aimCell`, ...) where one exists.

| File | Resolutions |
|---|---|
| about_40f7fc | `Screen`/`Key`/`Button` → `TScene`/`TGraphicSprite`/`TButtonSprite`; `k->down` → `shown`; `b->onClick` → `onRelease`; key handler has the `SceneKeyCb` spelling; `(SceneCb)fn_40f85c` for the TScene ctor |
| egg_40fa1c | `#pragma option -a1` moved **after** the includes (before them it packs the header structs and every `ELF_CHECK` fails; function alignment still matches); `frames[f]` → `(TTextCast *)casts[frame]`; `onDone` → `onLeave`; `Classes_Rect` → `Classes::Rect` (TRect); `(RECT *)&r` for fn_407a98; `fn_40fa1c` gained an unused `TGraphicSprite *` (it is a timer callback); alias `fn_407a44_pt` |
| exit_411af4 | `Game` → `TStage` (init_40f350 passes the stage); `(SceneCb)fn_4112b8` |
| init_40f350 | `Game`/`GameObj` → `TStage`; `music` → `casts`; `new TStage(...)`, `new TPackedResources(...)`; `p_Screen` → `g_45fee8`, `p_Application` → `g_45fee4` (new in game.h); `fn_40f3c0()` is a `StageCb` (unused parameter dropped); `TWebTrack` built through a local same-size stand-in (resolved in batch E) |
| score_4120cc | `Screen`/`Sprite`/`SpriteGroup`/`LaneSprite`/`LaneGroup`/`Pin` → `TScene`/`TGraphicSprite`/`TSpriteGroup`/`PinState`; `strike/spare` → `shown0/shown1`; `f1da` → `animate`; `Classes_Rect` → `Classes::Rect`; key handlers `fn_41296c`/`fn_412a7c` have the `SceneKeyCb` spelling, and `HasCtrl(shift)` (`(shift & 4) != 0` on an int) became `shift.Contains(2)` (same bytes); `GetPos`/`SetClip` inline members via a local subclass (resolved in batch E) |
| game_41665c | `g_present_460578`/`g_map_458840` → address names; `(char *)g_460582` for fn_416570; gained the unused `TScene *` (game_416f10 passes it) |
| game_416bd8 | alias `fn_401d0c_pt` (three TPoints by value) |
| game_416f10 | field renames (`f8c/f160/f24/f100/rc/fd0` → `onCell/onLeave/frame/x/clip/ELF_AS(TRect, bounds)`); `(RECT *)&r` for fn_407a98; `fn_418070` gained an unused `TGraphicSprite *`; alias `fn_407a44_pt` |
| game_417074 | as game_416f10; aliases `fn_401d0c_pt` |
| game_418274 | `Btn` → `TButtonSprite` (`onClick` → `onRelease`); `f18` → `load`; `f1d8` → `keepParms`; `new TSpriteGroup(...)`; `Link()` inline member via a local subclass (resolved in batch E). The `KeyFn` spelling is gone: `fn_409fa8(self, fn_41296c)` only pushes the address |
| game_41c674 | `Game::nspr/spr[]` → `nsprites/sprites[]`; `f250` → `TTextCast::lineHeight`; key handler `fn_41c940` has the `SceneKeyCb` spelling; `fn_41c98c/41c9a0/41c9bc/41c9cc` gained an unused `TGraphicSprite *` (button/timer callbacks); `fn_41c674` takes the `TStage *`; alias `fn_407a44_pt` |
| opening_41d184 | as game_418274; `f1d8` → `keepParms`; `st->frames[]` → `(TTextCast *)st->casts[]` |
| opening_41e01c | `onclick` (0x150) → `onRClick`; key handler `fn_41e0bc` has the `SceneKeyCb` spelling; `fn_41e108` gained an unused `TGraphicSprite *`; `fn_41e01c`/`fn_41e22c` take the `TStage *` |

**KeyFn against SceneKeyCb.** Every key handler definition in batch D
(`fn_40f7fc`, `fn_41296c`, `fn_412a7c`, `fn_41c940`, `fn_41e0bc`) now has the
`SceneKeyCb` spelling `(TScene *, void *sender, unsigned short &key,
ShiftState shift)` and matches: a reference compiles like the old pointer,
and `shift.Contains(2)` compiles like the old `(int)shift & 4` test. No
`(SceneKeyCb)` cast is left.

Batch D aliases:

| Alias | Units |
|---|---|
| `int fn_401d0c_pt(TPoint, TPoint, TPoint)` | game_416bd8, game_417074 |
| `void fn_407a44_pt(TGraphicSprite *, TPoint v, int, int, int)` | egg_40fa1c, game_416f10, game_41c674 (the two-int spelling in funcs.h gives a different register rotation: 5 functions DIFF) |

Header changes (all backward compatible):
- game.h: `g_45fee4` (`&Forms::Application`); `g_45c41c` (zlib error hook,
  batch C request); 27 readable alias macros (`g_cheatX..G`, `g_aimCell`,
  `g_aimPeriod`, `g_aiming`, `g_pinsDown`, `g_gameKey`, `g_gutterBall/Left/Right`,
  `g_ballOffset`, `g_ballRow`, `g_ballRowY`, `g_tauntPending`, `g_deerHit`,
  `g_deerUp`, `g_deerFrame`, `g_hintShown`, `g_hintCount`, `g_eggText`,
  `g_eggLineH`, `g_eggLen`, `g_scrollText`, `g_scrollLineH`, `g_scrollLen`).
- funcs.h (own entries only): key handlers `fn_40f7fc`, `fn_41296c`,
  `fn_412a7c`, `fn_41c940`, `fn_41e0bc` → `SceneKeyCb` spelling; `fn_411af4`,
  `fn_41c674`, `fn_41e01c`, `fn_41e22c` take `TStage *`; `fn_40f3c0()`;
  `fn_412358()` (game_416f10 calls it with no argument); unused
  `TGraphicSprite *` added to the callbacks `fn_40fa1c`, `fn_412834`,
  `fn_4128c4`, `fn_412d5c`, `fn_418070`, `fn_41c98c`, `fn_41c9a0`,
  `fn_41c9bc`, `fn_41c9cc`, `fn_41e108`; `TGraphicSprite *` (was `void *`)
  in `fn_40fa2a..fn_40fb9e`, `fn_4122e0`, `fn_4124a4`, `fn_4126f8`,
  `fn_412d74`; `SpriteCb done` (was `void *`) in `fn_41353c`, `fn_4135ac`.
  Definitions gained their unused `TScene *` where funcs.h had it
  (`fn_41237c`, `fn_412bc4`, `fn_412c30`, `fn_412cc8`, `fn_41665c`).

### Notes (batch D)

The TScene ctor, sprite inline, TWebTrack ctor and VCL shell requests were
resolved in batch E. One note stays:

- **funcs.h fn_401d0c / fn_407a44 (range_401d0c / range_4060d4):** the
  aliases above stay unless the definitions take TPoints.

## Batch E migration (VCL shell, threads, zlib) and cross-header requests

All 8 files are migrated and match (`make verify` 650/650, 0 xref wrong).
Every src_match file now passes the native syntax check
(`clang++ -std=c++17 -fsyntax-only -include bcb_rtl.h -Wno-unknown-pragmas
-Iport/include -Iinclude`); the only diagnostics left are
`-Wreturn-type-c-linkage` warnings for the extern "C" functions that return
`ERect`.

| File | Resolutions |
|---|---|
| main_401508 (VCL) | `<vcl.h>` then `<elf/funcs.h>`; local `ShiftState`/`Engine`/prototypes dropped (`Engine` → `TStage`, `g_456c38`); native-only `VCL_PUBLISHED(TForm1, VCL_METHOD_AS(FormPaint, fn_4015f8), ...)` and `VCL_REGISTER_CLASS(TForm1)` |
| elf_40128c (VCL) | `int WINAPI WinMain`; `<elf/funcs.h>` after `hdrstop`; native-only `#define Form1 g_4601c0` |
| threads_40f604 (VCL) | local `TObject`/`TThread` copies → `<vcl/classes.hpp>` + elf/thread.h; native-only `Execute`/`Update` bodies calling fn_40f684/fn_40f674 |
| threads_40f674, threads_40f684 (VCL) | `MyThread` struct view and the local `TThreadMethod` closure → `TMyThread`; `self->Terminated` (thread.h republishes the protected property on bcc32 with `__property Terminated;`: the same `[eax+0xc]` read); `TThread_Synchronize(self, ELF_METHOD(self, Update, fn_40f674))` |
| threads_40f734 (VCL) | as threads_40f604; the repeated ctor (needed so bcc32 emits the implicit dtor 0x40f734 here) is `#ifdef __BORLANDC__`, so the native build has one ctor |
| webtrack_40e5ec (VCL) | local `System`/`Classes`/`Scktcomp` copies → the real `<vcl/scktcomp.hpp>` (BCB3 SCKTCOMP.HPP; its layout gives the same bytes); `WebTrack` struct view → the real `TWebTrack` class for the handlers too; `client->socket` → `client->Socket`; the seven handlers bound with `ELF_METHOD(this, OnXXX, fn_40eXXX)`; `*p_Application` → `(Classes::TComponent *)*g_45fee4`; ctor `(const char *, const char *, bool)`, MATCH `@TWebTrack@$bctr$qpxct14bool`; `fn_40ec20()` lost its unused parameter (funcs.h too; main_401508 calls it with none); `TControl_GetClientWidth/Height` are funcs.h's fn_434238/fn_43427c macros |
| zlib_41e2cc | stays off the elf headers (its `z_stream` and zlib typedefs conflict with packres.h's `ZStream` and the funcs.h `fn_420984`/`fn_42099c` prototypes). Native-compilable: the `memcpy`/`calloc`/`free` redeclarations are bcc32-only (natively `<string.h>`/`<stdlib.h>`); `register` removed (every byte unchanged); `g_45c41c` has the game.h type `void (*)(const char *)`; native-only `fn_40e19c` (below). The prototypes were already ANSI |

Cross-header requests from batches A–D:

| Request | Done |
|---|---|
| types.h | natively `#include <vcl/vcl.h>` before `<windows.h>`; the bcc32 `ELF_CLOSURE`/`ELF_METHOD` (the exact old spelling: `typedef R __fastcall (__closure *Name) Params` and `(Obj)->Member`); the `DWORD_PTR`/`intptr_t`/`uintptr_t` typedefs moved from sound.h (same `ELF_PTR_TYPES` guard) |
| TScene ctor | `TScene(const char *, int, SceneStartCb, SceneCb)`; MATCH `@TScene@$bctr$qpxcipqp6TScenec$vpqp6TScene$v`. The five `(SceneCb)` casts are gone. The loaders fn_40f85c, fn_4112b8, fn_418274, fn_41d184, fn_41e1a4 take `char` (was `bool`; same bytes, funcs.h too): making `SceneStartCb` take `bool` instead changes the call in fn_409a6c |
| sprite.h | `TGraphicSprite::Link`, `SetClip` and the out-of-class inline `GetPos`; `TLinkSprite` (game_418274) and `TLaneSprite` (score_4120cc) removed |
| stage.h | `TWebTrack(const char *host, const char *path, bool f)` in the layout (the real ctor's spelling; init_40f350 passes 1); `TWebTrackNew` removed. The mouse-event typedefs are `ELF_CLOSURE`; natively `typedef Forms::TForm TStageForm` (the struct stand-in is bcc32-only) |
| engine_40a978 | the three `ELF_METHOD(this, MouseXxx, fn_40a6e4/40a764/40a7e4)` assignments; local `TMyThread` removed |
| TMyThread | new elf/thread.h (VCL only), included by funcs.h |
| VCL shell | funcs.h section "VCL methods under plain-function spellings": `TCustomForm_Close`, `TApplication_Terminate`, `TApplication_MessageBox`, `TCustomForm_SetClientWidth/Height`, `TCustomWinSocket_SendBuf/ReceiveLength/ReceiveBuf` (the C++ spellings of port/include/vcl/elfcompat.h). init_40f350 and range_401cec lost their local copies; `p_Application` → `g_45fee4` everywhere |
| sound_40c5dc | `fn_40c6d0(..., DWORD_PTR cb, DWORD_PTR inst, DWORD flags)`, `fn_40c718(HWAVEOUT, UINT, DWORD_PTR inst, DWORD_PTR p1, DWORD_PTR p2)` (funcs.h too) |
| packres_40dce0 | local `g_45c41c` removed (game.h) |

Other port blockers fixed:
- menu_41008c: `(int)ShellExecuteA(...)` → `(intptr_t)`.
- range_401d0c: `tmax_if(0L, ...)` → `tmax_if((LONG)0, ...)`; fn_402a7c's
  `int unused[2]` → two `int`s (see "Frame primer").
- **fn_40e19c** is not library code. libmap's `std::basic_ifstream::is_open`
  label is a false match: the body is `push s+0x10; call fn_4207e6`, i.e.
  `inflateEnd(&((TPackedStreamData *)s)->z)`. funcs.h says so now. It lies
  outside the game ranges, so it is not matched; zlib_41e2cc.cpp defines it
  natively (`#ifndef __BORLANDC__`) against the stream layout, so the 32-bit
  offset is not hard-coded.

Native-build triage items (docs/NATIVE_TRIAGE.md), all without a bcc32 byte change:
- **S5:** zlib_41e2cc defines the five zlib `local` statics
  (`g_fixed_mem_460798`, `g_fixed_bl/bd/tl/td`) natively; on bcc32 they stay
  `extern`.
- **S6:** zlib's `uLong` is `unsigned int` natively (`unsigned long` on
  bcc32), so `z_stream` has ZStream's layout.
- **S7:** packres_40e0a8 passes `sizeof(ZStream)` (0x38 on bcc32) to
  fn_420984 (inflateInit_ checks it).
- **S11:** sound_40c960 uses `sizeof(WAVEHDR)` (0x20 on bcc32) for the
  memset and the two `cbwh` arguments.
- **P7:** the two `dummy_new` helpers are `dummy_new_40f104` and
  `dummy_new_40f2a4` (they only make bcc32 emit the inline dtors).
- **S12:** scene_409c50's `ctl->vt[0x7c / 4](ctl)` (dead code) is
  `#ifdef __BORLANDC__`.
- **S13:** natively, TPackedStream declares `virtual void SetSize(int)`
  (VMT slot 0 of the original, fn_40dfd0); the body is in packres_40e0a8.cpp.
  On bcc32 a new virtual would change the vtable, so it is native-only.

## Open questions

- ~~`TStage::palette` HPALETTE or int~~ **Resolved (batch B):** `HPALETTE`
  (engine_40b158 passes it to `SelectPalette`). stage.h now has the inline
  `int *TStage::pk() { return palette == 0 ? (int *)&palette : 0; }` for
  range_403540. A scratch copy of range_403540 with `HPALETTE f81c` and this
  body matched in full (10522/10522 bytes).
- ~~`TGraphicSprite::seqs` 24 or 26~~ **Resolved (batch A):** 24.
  fn_407ad8 refuses a 25th sequence ("StartBuildSequence: Too many
  sequences."), sprites_407ffc indexes `seqs[idx]` only when `idx < nseq`,
  and sprites_408940 never touches the array. Both units match with `TSeq
  *seqs[24]`.
- ~~`TStage::pk()` for range_403540~~ **Resolved (batches A and B):**
  range_403540 calls `c->stage->pk()` and matches.
- ~~`TSoundQueueEntry::owner`/`cb` int or pointers~~ **Resolved (batch C):**
  pointers. The owner is an opaque `void *` cookie (the game passes its
  `TScene *` or 0; fn_40ccd0 compares it, fn_40c804 hands it back as the
  callback's first argument), and the callback is `SoundDoneCb`. Every play
  entry point (`fn_40cae0/40cb34/40cb7c/40cbf0/40ccd0/40d0a8/40d100`) takes
  `void *owner` and `SoundDoneCb cb`, and `TSoundMgr::loopOwner` is `void *`.
  All callers pass a pointer or a literal 0, so no call site changed.
- ~~`KeyFn` against `SceneKeyCb`~~ **Resolved (batch B):** the only call
  through the pointer is engine_40bd3c/40bda4, which matches with
  `SceneKeyCb`. The units that store a handler (game_418274, opening_*) only
  push its address to `fn_409fa8`/`fn_409fb8`, so `(SceneKeyCb)fn_x` there
  changes no code. **Batch D:** the handler definitions now use the
  `SceneKeyCb` spelling too and still match (`shift.Contains(2)` compiles
  like the old `int` test), so no cast is left.
- funcs.h `alt:` lines with a different **return** type
  (`char`/`bool`/`int`/`void`) show where a caller tests the result. Those
  callers may need an alias.
- webtrack_40e5ec.cpp defines the real `TWebTrack` class and sets
  `ELF_NO_TWEBTRACK`. stage.h's layout and the real class have the same
  ctor, but natively they are still two definitions of one class (an ODR
  violation that links, because only the ctor symbol is shared).
- zlib_41e2cc.cpp has the real zlib typedefs and includes no elf header.

## Batching plan (all batches done)

Each batch owns the headers listed with it, and it alone edits the structs
in them. Every batch may add or fix **funcs.h entries for functions that its
own files define**. Change another batch's funcs.h entries only by asking
that batch (or serialise such changes). game.h globals belong to batch D.
Run `make verify` before handing back.

| Batch | Owns | Files |
|---|---|---|
| A: sprites and casts (10) | sprite.h, cast.h, bitmap.h | ctors_4052dc, range_401cec, range_403540, range_4060d4, sprites_407ffc, sprites_408940, sprites_408f84, sprites_409100, sprites_4096dc, sprites_409fa8 |
| B: stage and scene (24) | stage.h, scene.h | engine_40a6e4, engine_40a844, engine_40a948, engine_40a978 (VCL), engine_40adc4, engine_40ae80, engine_40af80, engine_40b158, engine_40b3b0, engine_40b5bc, engine_40b8fc, engine_40ba68, engine_40bd3c, engine_40bdec, engine_40bfe4, engine_40c130, stage_40ae30, scene_409844, scene_409c50, scene_409dac, scene_409fc8, scene_40a0d8, scene_40a1fc, scene_40a3a4 |
| C: sound and packed resources (21) | sound.h, packres.h | sound_40c1f0 (VCL), sound_40c544, sound_40c880, sound_40c960, sound_40ca94, sound_40cbf0, sound_40cf24, packres_40d2c8, packres_40d3f8, packres_40d4bc (VCL), packres_40d578, packres_40d6f0, packres_40d980, packres_40db00, packres_40dbe0, packres_40dc1c, packres_40dce0, packres_40dee0, packres_40e0a8 (VCL), packres_40f104 (VCL), packres_40f2a4 (VCL) |
| D: game screens (13) | game.h | about_40f7fc, egg_40fa1c, exit_411af4, init_40f350, score_4120cc, game_41665c, game_416bd8, game_416f10, game_417074, game_418274, game_41c674, opening_41d184, opening_41e01c |
| E: VCL shell, threads, zlib (8, done) | thread.h; the batch A–D cross-header requests | main_401508, elf_40128c, threads_40f604, threads_40f674, threads_40f684, threads_40f734, webtrack_40e5ec, zlib_41e2cc |

Suggested order inside each batch: start with the small files, and leave the
VCL ones for last. Batch E is mostly VCL declarations. Its main win is
removing the duplicated `TObject`/`TStream`/`TThread` copies, perhaps into a
new `elf/vclstub.h`, which batch E would own. zlib_41e2cc may reasonably stay
as it is (it is matched third-party code).
