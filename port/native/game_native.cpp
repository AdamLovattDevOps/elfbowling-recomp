// Native-only definitions the game needs that no src_match/ unit provides in a
// native build (docs/NATIVE_TRIAGE.md, "port" rows):
//
//   1. extern "C" wrappers for the member-matched functions. funcs.h gives each
//      `member X::fn_XXXXXX (MATCH)` a cdecl prototype with `this` first, which is
//      what the other units call; bcc32 never links, so the matching build does
//      not need them.
//   2. The per-unit aliases (fn_XXXXXX_E / _t / _pt): the same function under a
//      second spelling that makes bcc32 emit the original call-site code (ERect by
//      value instead of RECT, TPoints instead of int pairs). Natively each one
//      forwards to the canonical definition, converting the arguments the way the
//      32-bit stack layout lines them up.
//   3. C++ special members the headers declare whose bodies were matched as free
//      functions (deleting destructors, the empty TSceneButton ctor).
//   4. TPackedStream's virtuals (Read = fn_40dee0, Seek = fn_40e004 were matched as free
//      functions over TPackedStreamData; natively they must also be the C++ overrides, and
//      Read is the key function that emits the vtable).
//   5. (fn_40e19c now lives in zlib_41e2cc.cpp; see the note below.)
#include <vcl/vcl.h>     // first, so that packres.h declares the VCL class TPackedStream
#include <elf/funcs.h>

#include <stddef.h>

// ---- 1. member-matched functions ------------------------------------------------------------

extern "C" {

void fn_40971c(TSpriteGroup *self) { self->fn_40971c(); }
void fn_409754(TSpriteGroup *self, int t) { self->fn_409754(t); }
void fn_409790(TSpriteGroup *self, int start, int n) { self->fn_409790(start, n); }
void fn_4097e8(TSpriteGroup *self, int start, int n, int t) { self->fn_4097e8(start, n, t); }

TButtonSprite *fn_40a1fc(TScene *self, const char *name, const char *b, const char *c, TPoint p)
{
    return self->fn_40a1fc(name, b, c, p);
}
int fn_40a520(TScene *self, const char *nm) { return self->fn_40a520(nm); }
TGraphicSprite *fn_40a598(TScene *self, const char *nm) { return self->fn_40a598(nm); }

void fn_40ae80(TStage *self, ERect r) { self->fn_40ae80(r); }
void fn_40af80(TStage *self, ERect r) { self->fn_40af80(r); }
void fn_40b9b8(TStage *self) { self->fn_40b9b8(); }
void fn_40ba68(TStage *self) { self->fn_40ba68(); }
TScene *fn_40bf2c(TStage *self, const char *name) { return self->fn_40bf2c(name); }
TScene *fn_40bf84(TStage *self, const char *name) { return self->fn_40bf84(name); }
int fn_40c160(TStage *self, const char *name) { return self->fn_40c160(name); }

char fn_40c880(TSoundMgr *self, char flag) { return self->fn_40c880(flag); }
int fn_40d250(TPackedResources *self) { return self->fn_40d250(); }

// 0x4067e0 TGraphicSprite::AddTalkingIndex (range_4060d4); callers ignore the result
void fn_4067e0(TGraphicSprite *s, const char *cast, int i) { s->AddTalkingIndex(cast, i); }

} // extern "C"

// ---- 2. aliases -----------------------------------------------------------------------------

static inline RECT to_rect(const ERect &e)
{
    RECT r;
    r.left = e.left;
    r.top = e.top;
    r.right = e.right;
    r.bottom = e.bottom;
    return r;
}
static inline ERect to_erect(const RECT &r)
{
    ERect e;
    e.left = r.left;
    e.top = r.top;
    e.right = r.right;
    e.bottom = r.bottom;
    return e;
}
static inline ERect to_erect(const RectPod &r)
{
    ERect e;
    e.left = r.left;
    e.top = r.top;
    e.right = r.right;
    e.bottom = r.bottom;
    return e;
}

extern "C" {

// range_401d0c's own rect helpers under the ERect copy-ctor spelling
bool fn_40252c_TRect(ERect a, ERect b) { return fn_40252c(to_rect(a), to_rect(b)); }
char fn_40252c_E(ERect a, ERect b) { return fn_40252c(to_rect(a), to_rect(b)); }
char fn_402580_E(ERect a, ERect b) { return fn_402580(to_rect(a), to_rect(b)); }
ERect fn_4025e0_E(ERect a, ERect b) { return to_erect(fn_4025e0(to_rect(a), to_rect(b))); }
ERect fn_402748_t(ERect a, ERect b) { return to_erect(fn_402748(to_rect(a), to_rect(b))); }
ERect fn_40278c_t(ERect a, ERect b) { return to_erect(fn_40278c(to_rect(a), to_rect(b))); }
TPoint fn_402840_E(ERect r, TPoint p) { return fn_402840(to_rect(r), p); }
TPoint fn_4028b8_E(ERect r, TPoint p) { return fn_4028b8(to_rect(r), p); }
// ERect and RECT have the same layout (four ints); fn_402e30 writes through the pointer
void fn_402e30_t(SIZE *s, ERect *r) { fn_402e30(s, reinterpret_cast<RECT *>(r)); }

// three TPoints by value are the six dwords of fn_401d0c(unused, x, y0, x0, y1, x1)
int fn_401d0c_pt(TPoint p, TPoint a, TPoint b) { return fn_401d0c(p.x, p.y, a.x, a.y, b.x, b.y); }

// fn_407a44 reads its last two arguments as bytes
void fn_407a44_pt(TGraphicSprite *s, TPoint v, int a, int b, int c) { fn_407a44(s, v, a, (char)b, (char)c); }

// engine_40a5d4 takes and returns RectPod; the callers see ERect (same layout)
ERect fn_40a5d4_E(TStage *e, ERect &r) { return to_erect(fn_40a5d4(e, reinterpret_cast<RectPod &>(r))); }
ERect fn_40a63c_E(TStage *e, ERect &r) { return to_erect(fn_40a63c(e, reinterpret_cast<RectPod &>(r))); }

} // extern "C"

// ---- 3. special members matched as free functions -------------------------------------------

// 0x40c5dc is TSound's deleting destructor (flags & 1 frees). The C++ dtor is its body.
TSound::~TSound() { fn_40c5dc(this, 0); }
// 0x40e210 likewise for TSaveCasts
TSaveCasts::~TSaveCasts() { fn_40e210(this, 0); }
// 0x4099d4: `mov eax,[ebp+8]; ret` (the array-element ctor of TScene::buttons)
TSceneButton::TSceneButton() {}

// ---- 4. TPackedStream -----------------------------------------------------------------------

int __fastcall TPackedStream::Read(void *Buffer, int Count)
{
    return fn_40dee0(reinterpret_cast<TPackedStreamData *>(this), static_cast<char *>(Buffer), Count);
}
int __fastcall TPackedStream::Seek(int Offset, Word Origin)
{
    return fn_40e004(reinterpret_cast<TPackedStreamData *>(this), Offset, Origin);
}
// (Write is packres_40f104's `return 0`, the same body as fn_40dfec.)

// ---- 5. fn_40e19c ---------------------------------------------------------------------------

// 0x40e19c (inflateEnd on the TPackedStream's z_stream) was defined here first; batch E then
// put the native definition into zlib_41e2cc.cpp (#ifndef __BORLANDC__), next to inflateEnd
// and zlib's own z_stream type, so it is no longer defined here (it would be a duplicate).

// The VCL class and the plain view of it must agree natively too.
static_assert(offsetof(TPackedStreamData, z) == sizeof(void *) * 2 + 8, "TPackedStreamData layout");
static_assert(sizeof(TPackedStream) == sizeof(TPackedStreamData), "TPackedStream vs TPackedStreamData");
static_assert(offsetof(TPackedStream, z) == offsetof(TPackedStreamData, z), "TPackedStream::z");
