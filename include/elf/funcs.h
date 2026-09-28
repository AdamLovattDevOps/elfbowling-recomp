// elf/funcs.h - one prototype per fn_XXXXXX (game code and the library
// entry points the game calls by address), with readable alias macros.
//
// First generated (Phase A step 1) from every definition and declaration in
// the pre-migration src_match/; from now on EDIT IT BY HAND (see
// docs/HEADERS.md "funcs.h"). Rules used to pick "the" signature:
//   - old local struct names are mapped to the canonical classes
//     (docs/HEADERS.md "Name map");
//   - declarations are grouped by compatible parameter lists (void * is
//     compatible with any pointer); the largest group wins, a definition
//     counting extra (except the empty stubs in range_401cec.cpp, whose
//     parameter lists are arbitrary);
//   - within the group, each void * parameter takes the most common
//     specific pointer type seen.
// The "alt:" lines list the other spellings found and where. A unit that
// needs another spelling (because it changes the call-site codegen: a
// TPoint by value vs two ints, a char vs bool return, RECT vs ERect by
// value) declares a local ALIAS, e.g. `extern "C" char fn_40252c_R(ERect a,
// ERect b);` (the call target is a fixup and xref_check.py matches the fn_
// prefix). Record such aliases in docs/HEADERS.md.
//
// "member X::fn_ (MATCH)": the function is defined as a real C++ member in
// the matching build. The extern "C" prototype here is the same ABI (cdecl,
// `this` pushed last) and is what other units call; the native port gives
// it a one-line wrapper.
//
// Include after the class headers (elf/game.h includes them all).
#ifndef ELF_FUNCS_H
#define ELF_FUNCS_H

#include <elf/game.h>
#include <elf/cast.h>
#include <elf/thread.h>

class TPackedStream;

// ==== game code (kind=game in build/funcs.tsv) ====

// 0x401508  def main_401508.cpp
extern "C" void fn_401508();

// 0x4015f8  def main_401508.cpp
extern "C" void __fastcall fn_4015f8(void *form, void *sender);

// 0x401610  def main_401508.cpp
extern "C" void __fastcall fn_401610(void *form, void *sender, unsigned short &key, ShiftState shift);

// 0x401644  def main_401508.cpp
extern "C" void __fastcall fn_401644(void *form, void *sender, unsigned short &key, ShiftState shift);

// 0x401cec  def range_401cec.cpp
//   alt: void fn_401cec(int)  [range_401cec]
extern "C" void fn_401cec();

// 0x401cf4  def range_401cec.cpp
//   alt: void fn_401cf4(int)  [range_401cec]
extern "C" void fn_401cf4(const char *fmt, ...);

// 0x401cfc  def range_401cec.cpp
//   alt: void fn_401cfc(int)  [range_401cec]
extern "C" void fn_401cfc();

// 0x401d04  def range_401cec.cpp
extern "C" void fn_401d04(const char *fmt, ...);

// 0x401d0c  def range_401d0c.cpp
//   alt: int fn_401d0c(TPoint, TPoint, TPoint)  [game_416bd8, game_417074]
extern "C" int fn_401d0c(int unused, int x, int y0, int x0, int y1, int x1);

// 0x401d48  def range_401d0c.cpp
extern "C" void fn_401d48(char *dst, const char *src, char a, char b);

// 0x401df8  def range_401cec.cpp
extern "C" void fn_401df8();

// 0x401e20  def range_401cec.cpp
extern "C" void fn_401e20();

// 0x401e48  def range_401d0c.cpp
//   alt: void fn_401e48(char *, int, int, int)  [menu_41008c, unit_411, webtrack_40e5ec]
extern "C" void fn_401e48(char *d, int v, int digits, char strip);
#define Str_FormatNumber fn_401e48

// 0x401f0c  def range_401cec.cpp
extern "C" void fn_401f0c(const char *msg);
#define Elf_Fatal fn_401f0c

// 0x401f30  def range_401d0c.cpp
//   alt: void fn_401f30(const char *, TSpriteGroup*)  [sprites_4096dc]
extern "C" void fn_401f30(const char *a, const char *b);

// 0x401f70  def range_401d0c.cpp
extern "C" void fn_401f70(const char *a, const char *b, const char *c);

// 0x401fb4  def range_401d0c.cpp
extern "C" BITMAPINFOHEADER *fn_401fb4(Bitmap *b);
#define Bitmap_Header fn_401fb4

// 0x402030  def range_401d0c.cpp
//   alt: int fn_402030(TStreamVmt *, void*)  [packres_40d980]
extern "C" int fn_402030(TStreamVmt *st, HPALETTE *pal);
#define Palette_Read fn_402030

// 0x402194  def range_401d0c.cpp
extern "C" void fn_402194();

// 0x402224  def range_401d0c.cpp
extern "C" void fn_402224();

// 0x4022fc  def range_401cec.cpp
extern "C" void fn_4022fc(const char *msg);
#define Log_Msg fn_4022fc

// 0x402318  def range_401cec.cpp
extern "C" void fn_402318(const char *msg, const char *name);
#define Log_Msg2 fn_402318

// 0x402338  def range_401d0c.cpp
//   alt: void* fn_402338(int, const char*)  [egg_40fa1c, engine_40a978, packres_40d2c8, sound_40c1f0, webtrack_40e5ec]
//   alt: char* fn_402338(int, const char*)  [game_41c674]
extern "C" void *fn_402338(unsigned size, const char *what);
#define Mem_Alloc fn_402338

// 0x402380  def range_401cec.cpp
//   alt: void fn_402380(void*)  [range_401cec]
extern "C" void fn_402380(void *p, const char *who);
#define Mem_Free fn_402380

// 0x40239c  def range_401d0c.cpp
extern "C" int fn_40239c(const char *a, const char *b);
#define Name_Match fn_40239c

// 0x402410  def range_401cec.cpp
//   alt: void fn_402410(char *, const char *, int)  [packres_40db00, packres_40dce0, scene_409844, sound_40c544]
//   alt: char* fn_402410(char *, const char *, unsigned)  [range_401cec, range_403540, range_4060d4]
extern "C" char *fn_402410(char *d, const char *s, int n);
#define Str_CopyN fn_402410

// 0x402434  def range_401d0c.cpp
extern "C" char *fn_402434(char *s);
#define Name_Increment fn_402434

// 0x4024bc  def range_401cec.cpp
extern "C" int fn_4024bc(int a);
#define Tick_After fn_4024bc

// 0x4024cc  def range_401cec.cpp
extern "C" int fn_4024cc(int a, int b);
#define Tick_Add fn_4024cc

// 0x4024e8  def range_401d0c.cpp
extern "C" void fn_4024e8(RECT *d, RECT *s, int n);
#define Rect_Div fn_4024e8

// 0x40252c  def range_401d0c.cpp
//   alt: char fn_40252c(ERect, ERect)  [engine_40af80, scene_409c50]
//   alt: bool fn_40252c_TRect(ERect, ERect)  [range_401d0c]
extern "C" bool fn_40252c(RECT a, RECT b);

// 0x402580  def range_401d0c.cpp
//   alt: char fn_402580(ERect, ERect)  [engine_40ae80, engine_40af80]
extern "C" bool fn_402580(RECT a, RECT b);
#define Rect_Touch fn_402580

// 0x4025e0  def range_401d0c.cpp
//   alt: ERect fn_4025e0(ERect, ERect)  [engine_40ae80, engine_40af80]
extern "C" RECT fn_4025e0(RECT a, RECT b);
#define Rect_Union fn_4025e0

// 0x402654  def range_401d0c.cpp
extern "C" char fn_402654(ERect *r, ERect b);
#define Rect_Clip fn_402654

// 0x402748  def range_401d0c.cpp
//   alt: ERect fn_402748_t(ERect, ERect)  [range_4060d4]
extern "C" RECT fn_402748(RECT a, RECT b);

// 0x40278c  def range_401d0c.cpp
//   alt: ERect fn_40278c_t(ERect, ERect)  [range_4060d4]
extern "C" RECT fn_40278c(RECT a, RECT b);

// 0x4027d0  def range_401cec.cpp
//   alt: int fn_4027d0(RECT, int, int)  [range_401cec]
extern "C" char fn_4027d0(ERect r, TPoint p);

// 0x402800  def range_401cec.cpp
//   alt: int fn_402800(RECT, int)  [range_401cec]
extern "C" char fn_402800(ERect r, TPoint p);

// 0x402820  def range_401cec.cpp
//   alt: int fn_402820(RECT, int, int)  [range_401cec]
extern "C" char fn_402820(ERect r, TPoint p);

// 0x402840  def range_401d0c.cpp
//   alt: TPoint fn_402840(ERect, TPoint)  [sprites_407ffc]
extern "C" TPoint fn_402840(RECT r, TPoint p);

// 0x4028b8  def range_401d0c.cpp
//   alt: TPoint fn_4028b8(ERect, TPoint)  [sprites_408940]
extern "C" TPoint fn_4028b8(RECT r, TPoint p);

// 0x40290c  def range_401d0c.cpp
extern "C" void fn_40290c(Bitmap *b);
#define Bitmap_Free fn_40290c

// 0x40298c  def range_401d0c.cpp
extern "C" void fn_40298c(Bitmap *b, int w, int h, int bpp);

// 0x4029c8  def range_401d0c.cpp
extern "C" void fn_4029c8(Bitmap *b, int w, int h, int bpp);
#define Bitmap_Create fn_4029c8

// 0x402a58  def range_401d0c.cpp
//   alt: void fn_402a58(void *, void*)  [packres_40d578, packres_40d6f0]
extern "C" void fn_402a58(Bitmap *b, DibFile *f);

// 0x402a7c  def range_401d0c.cpp
extern "C" void fn_402a7c(int bpp, char *row, int bytes);

// 0x402b4c  def range_401d0c.cpp
extern "C" void fn_402b4c(Bitmap *d, Bitmap *s);
#define Bitmap_Copy fn_402b4c

// 0x402bb8  def range_401d0c.cpp
extern "C" void fn_402bb8(Bitmap *d, Bitmap *s, int n);
#define Bitmap_ScaledCopy fn_402bb8

// 0x402d48  def range_401d0c.cpp
extern "C" void fn_402d48(unsigned char *d, unsigned char *s, int n);

// 0x402dbc  def range_401d0c.cpp
extern "C" void fn_402dbc(unsigned char *d, unsigned char *s, int n);

// 0x402e30  def range_401d0c.cpp
//   alt: void fn_402e30_t(SIZE *, ERect*)  [range_401d0c]
//   alt: void fn_402e30(SIZE *, ERect*)  [range_403540]
extern "C" void fn_402e30(SIZE *s, RECT *r);

// 0x402ed4  def range_401d0c.cpp
//   alt: void fn_402ed4(void *, void *, void *, void *, DWORD)  [engine_40b158]
//   alt: void fn_402ed4(void *, RectPod *, Bitmap *, TPoint *, DWORD)  [engine_40b3b0]
//   alt: void fn_402ed4(Bitmap *, RECT *, Bitmap *, RECT *, DWORD)  [range_403540]
extern "C" void fn_402ed4(Bitmap *dst, TPoint *dp, Bitmap *src, RECT *sr, DWORD rop);
#define Bitmap_Blit fn_402ed4

// 0x403048  def range_401d0c.cpp
//   alt: void fn_403048(void *, ERect *, void*)  [engine_40b5bc]
extern "C" void fn_403048(Bitmap *b, RECT *r, Bitmap *src);
#define Bitmap_FillRect fn_403048

// 0x403240  def range_401d0c.cpp
extern "C" void fn_403240(Bitmap *b, RECT *r);

// 0x40343c  def range_403540.cpp
//   alt: void fn_40343c(void *, ERect *, int)  [engine_40b5bc]
extern "C" void fn_40343c(Bitmap *b, RECT *r, int which);

// 0x403540  def range_403540.cpp
extern "C" void fn_403540(Bitmap *b, int color);

// 0x403574  def range_403540.cpp
//   alt: void fn_403574(void *, int, int, int, int)  [engine_40bfe4]
extern "C" HBITMAP fn_403574(Bitmap *b, int w, int h, int bpp, int which);

// 0x4035f4  def range_403540.cpp
extern "C" int CALLBACK fn_4035f4(const LOGFONT *lf, const TEXTMETRIC *tm, DWORD type, LPARAM lp);

// 0x4036c8  def range_403540.cpp
//   alt: void fn_4036c8(const char *, int, int)  [engine_40a978]
extern "C" int fn_4036c8(const char *face, int height, int weight);

// 0x403768  def range_403540.cpp
extern "C" void fn_403768();

// 0x403794  def range_403540.cpp
extern "C" void fn_403794(HDC dc, int idx);

// 0x4037c8  def range_403540.cpp
extern "C" void fn_4037c8(HDC dc);

// 0x4037e4  def range_403540.cpp
extern "C" int fn_4037e4(Bitmap *b, const char *text, int x, int y, int font, int color, char align);

// 0x403940  def range_403540.cpp
extern "C" char *fn_403940(Bitmap *b, int y);

// 0x4039ac  def range_403540.cpp
extern "C" void fn_4039ac(Bitmap *b, void *dst, int y);

// 0x403a40  def range_403540.cpp
extern "C" void fn_403a40(Bitmap *b, const void *src, int y);

// 0x403ad4  def range_403540.cpp
extern "C" char fn_403ad4(Bitmap *b, RECT *r);

// 0x403b6c  def range_403540.cpp
extern "C" char fn_403b6c(Bitmap *b, RECT *r);

// 0x403c34  def range_403540.cpp
extern "C" char fn_403c34(Bitmap *b, RECT *a);

// 0x403c60  def range_403540.cpp
extern "C" char fn_403c60(Bitmap *a, RECT *ar, Bitmap *b, RECT *br);

// 0x403d38  def range_403540.cpp
extern "C" char fn_403d38(Bitmap *a, RECT *ar, Bitmap *b, RECT *br);

// 0x403e54  def range_403540.cpp
extern "C" char fn_403e54(Bitmap *b, RECT *a, Bitmap *c, RECT *d);

// 0x403e8c  def range_403540.cpp
//   alt: char fn_403e8c(TGraphicCast *, ERect *, TGraphicCast *, ERect*)  [range_4060d4]
extern "C" char fn_403e8c(TGraphicCast *a, RECT *ax, TGraphicCast *b, RECT *bx);

// 0x403f04  def range_403540.cpp
extern "C" void fn_403f04(TGraphicCast *c);

// 0x404268  def range_403540.cpp
extern "C" void fn_404268(TGraphicCast *c);

// 0x4045f0  def range_403540.cpp
extern "C" void fn_4045f0(TGraphicCast *c);

// 0x404718  def range_403540.cpp
extern "C" void fn_404718(TGraphicCast *c);

// 0x4048a0  def range_403540.cpp
extern "C" void fn_4048a0(TGraphicCast *c);

// 0x404974  def range_403540.cpp
extern "C" void fn_404974(TGraphicCast *c, char crop);

// 0x404c54  def range_403540.cpp
extern "C" void fn_404c54(TGraphicCast *c, char crop);

// 0x404f50  def range_403540.cpp
extern "C" void fn_404f50(TGraphicCast *c, char f);

// 0x404f94  def range_403540.cpp
//   alt: void fn_404f94(void *, unsigned char)  [range_4060d4]
extern "C" void fn_404f94(TGraphicCast *c, unsigned char m);

// 0x40500c  def range_403540.cpp
//   alt: void fn_40500c_t(TTextCast *, TPoint)  [range_403540]
extern "C" void fn_40500c(TGraphicCast *c, TPoint pt);

// 0x4050b4  def range_403540.cpp
//   alt: void fn_4050b4(TGraphicCast *, const char *, int, char, TPoint, char, char)  [ctors_4052dc]
extern "C" void fn_4050b4(TGraphicCast *c, TStage *eng, const char *name, char m, TPoint pt, char e, char f);

// 0x4052ac  def range_403540.cpp
//   alt: char fn_4052ac_c(TGraphicCast *, int *, const char *, char, char)  [range_403540]
//   alt: char fn_4052ac(char *, int, int, char, char)  [range_403540 definition]
//   hand-set (batch A): (cast, TStage::pk() = &stage->palette or 0, resource name, flipX, flipY)
extern "C" char fn_4052ac(TGraphicCast *c, void *pal, const char *name, char e, char f);

// 0x40552c  def range_403540.cpp
//   alt: void fn_40552c(void *, int, int, int)  [ctors_4052dc]
extern "C" void fn_40552c(TTextCast *t, int w, int h, const char *name);

// 0x405710  def range_403540.cpp
//   alt: char fn_405710(void *, int)  [range_4060d4]
extern "C" char fn_405710(TTextCast *t, const char *text);

// 0x4057f8  def range_403540.cpp
//   alt: char fn_4057f8(void *, int)  [range_4060d4]
extern "C" char fn_4057f8(TTextCast *t, const char *text);

// 0x4058f0  def range_403540.cpp
extern "C" void fn_4058f0(TTextCast *t);

// 0x405954  def range_403540.cpp
//   alt: void fn_405954(void*)  [egg_40fa1c, menu_41008c, unit_411]
extern "C" void fn_405954(TTextCast *p);
#define TextCast_Clear fn_405954

// 0x405978  def range_403540.cpp
//   alt: void fn_405978(void *, int)  [egg_40fa1c, menu_41008c]
extern "C" void fn_405978(TTextCast *p, int v);
#define TextCast_SetStyle fn_405978

// 0x40598c  def range_403540.cpp
//   alt: void fn_40598c(void *, int)  [menu_41008c]
extern "C" void fn_40598c(TTextCast *p, int v);
#define TextCast_SetColor fn_40598c

// 0x4059a0  def range_403540.cpp
extern "C" void fn_4059a0(TCastMgr *l, TGraphicCast *v);

// 0x4059c4  def range_403540.cpp
//   alt: void* fn_4059c4(void *, const char*)  [range_4060d4]
extern "C" TGraphicCast *fn_4059c4(TCastMgr *l, const char *name);

// 0x405a1c  def range_403540.cpp
//   alt: void fn_405a1c(void*)  [init_40f350]
extern "C" char fn_405a1c(TCastMgr *o);

// 0x4060d4  def range_4060d4.cpp
extern "C" void fn_4060d4(TGraphicSprite *a, TPoint pt);

// 0x40610c  def range_4060d4.cpp
//   alt: void fn_40610c(TButtonSprite*)  [sprites_408f84]
extern "C" void fn_40610c(TGraphicSprite *a);
#define Sprite_Reset fn_40610c

// 0x406378  def range_4060d4.cpp
extern "C" void fn_406378(TGraphicSprite *a, TStage *e, const char *name, TPoint pt);
#define Sprite_Init fn_406378

// 0x406564  def range_4060d4.cpp
extern "C" void fn_406564(TGraphicSprite *a, char c);

// 0x40659c  def range_4060d4.cpp
//   alt: void fn_40659c(TGraphicSprite *, int, int, char, char)  [packres_40dbe0]
extern "C" void fn_40659c(TGraphicSprite *a, int x, int y, char vis, unsigned char mask);

// 0x406678  def range_4060d4.cpp
extern "C" int fn_406678(TGraphicSprite *a, TGraphicCast *c);

// 0x4066c4  def range_4060d4.cpp
//   alt: void fn_4066c4(TGraphicSprite *, TTextCast *, TPoint)  [scene_40a3a4]
extern "C" int fn_4066c4(TGraphicSprite *a, TGraphicCast *c, TPoint pt);

// 0x406724  def range_4060d4.cpp
//   alt: void fn_406724(TGraphicSprite *, const char*)  [game_418274, opening_41d184, scene_40a0d8, sprites_409100]
//   alt: void fn_406724(void *, const char*)  [menu_41008c]
extern "C" int fn_406724(TGraphicSprite *a, const char *name);

// 0x406780  def range_4060d4.cpp
//   alt: void fn_406780(TGraphicSprite *, const char *, int, int)  [scene_409fc8]
extern "C" int fn_406780(TGraphicSprite *a, const char *name, TPoint pt);

// 0x4067e0
//   alt: void fn_4067e0(void *, const char *, int)  [menu_41008c]
extern "C" void fn_4067e0(TGraphicSprite *s, const char *cast, int i);

// 0x406868  def range_4060d4.cpp
//   alt: bool fn_406868(TGraphicSprite *, TGraphicSprite*)  [game_414240]
extern "C" char fn_406868(TGraphicSprite *a, TGraphicSprite *b);

// 0x406970  def range_4060d4.cpp
//   alt: void fn_406970(TGraphicSprite *, TPoint, TPoint, int, int, int, int, int)  [range_4060d4]
extern "C" void fn_406970(TGraphicSprite *a, TPoint p1, TPoint p2, int f1e0, int speed, int f80, SpriteCb f88, int f84);

// 0x406a08  def range_4060d4.cpp
//   alt: void fn_406a08(TGraphicSprite *, const char*)  [egg_40fa1c, game_41c674, unit_411]
//   alt: void fn_406a08(void *, const char*)  [menu_41008c, score_4120cc]
//   alt: char fn_406a08(TGraphicSprite *, int)  [range_4060d4]
extern "C" char fn_406a08(TGraphicSprite *a, const char *v);
#define Sprite_SetText fn_406a08

// 0x406a44  def range_4060d4.cpp
//   alt: void fn_406a44(TGraphicSprite *, const char*)  [egg_40fa1c]
//   alt: char fn_406a44(TGraphicSprite *, int)  [range_4060d4]
extern "C" char fn_406a44(TGraphicSprite *a, const char *v);

// 0x406a80  def range_4060d4.cpp
//   alt: void fn_406a80(TGraphicSprite *, int, int)  [range_4060d4]
//   alt: void fn_406a80(void *, TPoint)  [score_4120cc]
extern "C" void fn_406a80(TGraphicSprite *spr, TPoint p);
#define Sprite_SetPos fn_406a80

// 0x406ac4  def range_4060d4.cpp; hand-set: arg is the scene passed on to onMoveDone (batch A)
//   alt: void fn_406ac4(TGraphicSprite *, int)  [range_4060d4, sprites_407ffc]
extern "C" void fn_406ac4(TGraphicSprite *a, TScene *arg);

// 0x406c00  def range_4060d4.cpp
//   alt: void fn_406c00(void*)  [menu_41008c, score_4120cc]
extern "C" void fn_406c00(TGraphicSprite *a);

// 0x406c50  def range_4060d4.cpp
//   alt: void fn_406c50(void *, int)  [score_4120cc]
//   alt: void fn_406c50(TButtonSprite *, int)  [sprites_4092bc]
extern "C" void fn_406c50(TGraphicSprite *a, int i);
#define Sprite_SetFrame fn_406c50

// 0x406c8c  def range_4060d4.cpp
extern "C" char fn_406c8c(TGraphicSprite *a, ERect *r);

// 0x406dd0  def range_4060d4.cpp
extern "C" void fn_406dd0(TGraphicSprite *a, ERect *r);

// 0x406ee0  def range_4060d4.cpp
extern "C" char fn_406ee0(TGraphicSprite *a, TGraphicCast *c, ERect *r);

// 0x407040  def range_4060d4.cpp
//   alt: char fn_407040(TButtonSprite *, ERect*)  [sprites_4092bc]
extern "C" char fn_407040(TGraphicSprite *a, ERect *r);

// 0x40717c  def range_4060d4.cpp
extern "C" char fn_40717c(TGraphicSprite *a, ERect *r);

// 0x4071a8  def range_4060d4.cpp
extern "C" char fn_4071a8(TGraphicSprite *a, ERect *r);

// 0x4071d4  def range_4060d4.cpp
extern "C" char fn_4071d4(TGraphicSprite *a, ERect *r);

// 0x407384  def range_4060d4.cpp
extern "C" char fn_407384(TGraphicSprite *a, ERect *r);

// 0x407540  def range_4060d4.cpp
extern "C" char fn_407540(TGraphicSprite *a, TGraphicCast *c, ERect *r);

// 0x407690  def range_4060d4.cpp
extern "C" char fn_407690(TGraphicSprite *a, ERect *r);

// 0x4076bc  def range_4060d4.cpp
extern "C" char fn_4076bc(TGraphicSprite *a, ERect *r);

// 0x4076e8  def range_4060d4.cpp
extern "C" void fn_4076e8(TGraphicSprite *a);

// 0x407748  def range_4060d4.cpp
extern "C" void fn_407748(TGraphicSprite *t);

// 0x407774  def range_4060d4.cpp
//   alt: void fn_407774(void *, int)  [menu_41008c, score_4120cc]
extern "C" void fn_407774(TGraphicSprite *t, int period);

// 0x40779c  def range_4060d4.cpp
//   alt: void fn_40779c(void*)  [menu_41008c, score_4120cc]
extern "C" void fn_40779c(TGraphicSprite *t);

// 0x4077c4  def range_4060d4.cpp
//   alt: void fn_4077c4(void *, int, int, void*)  [egg_40fa1c, menu_41008c, score_4120cc]
//   alt: void fn_4077c4(TGraphicSprite *, int, int, int)  [range_4060d4]
extern "C" void fn_4077c4(TGraphicSprite *t, int i, int delay, SpriteCb data);
#define Sprite_SetTimer fn_4077c4

// 0x4077ec  def range_4060d4.cpp
extern "C" char fn_4077ec(TGraphicSprite *t, int i, int now);

// 0x407820  def range_4060d4.cpp
//   alt: void fn_407820(void *, int)  [score_4120cc]
extern "C" void fn_407820(TGraphicSprite *t, int i);

// 0x407834  def range_4060d4.cpp
//   alt: void fn_407834(void*)  [score_4120cc]
extern "C" void fn_407834(TGraphicSprite *t);

// 0x407858  def range_4060d4.cpp
//   alt: void fn_407858(void *, int, int, int)  [menu_41008c, score_4120cc]
extern "C" void fn_407858(TGraphicSprite *a, int delay, int cast, int c);

// 0x4078a8  def range_4060d4.cpp
//   alt: void fn_4078a8(void*)  [egg_40fa1c, score_4120cc]
extern "C" void fn_4078a8(TGraphicSprite *a);

// 0x407924  def range_4060d4.cpp
//   alt: void fn_407924(void *, TPoint, int, char, char, int)  [score_4120cc]
extern "C" void fn_407924(TGraphicSprite *a, TPoint pt, int c, char d, char e, int f);

// 0x4079dc  def range_4060d4.cpp
//   alt: void fn_4079dc(TGraphicSprite *, TPoint, int, int, int)  [game_414240]
extern "C" void fn_4079dc(TGraphicSprite *p, TPoint pt, int c, char d, char e);

// 0x407a00  def range_4060d4.cpp
extern "C" void fn_407a00(TGraphicSprite *p, int x, int y, int c, char d, char e, int f);

// 0x407a44  def range_4060d4.cpp
//   alt: void fn_407a44(void *, TPoint, int, int, int)  [egg_40fa1c]
//   hand-set (batch A): the definition reads d/e as char; callers pass TPoint + literals
extern "C" void fn_407a44(TGraphicSprite *p, TPoint pt, int c, char d, char e);

// 0x407a68  def range_4060d4.cpp
//   alt: void fn_407a68(void *, TRect)  [menu_41008c, score_4120cc]
//   alt: void fn_407a68(TGraphicSprite *, RECT)  [range_4060d4]
extern "C" void fn_407a68(TGraphicSprite *spr, TRect r);

// 0x407a98  def range_4060d4.cpp
//   alt: void fn_407a98(TGraphicSprite *, TRect&)  [egg_40fa1c]
//   alt: void fn_407a98(TGraphicSprite *, TRect*)  [game_416f10, game_41c674]
//   alt: char fn_407a98(TGraphicSprite *, TRect*)  [game_418274]
//   alt: void fn_407a98(void *, TRect*)  [menu_41008c]
extern "C" char fn_407a98(TGraphicSprite *a, RECT *out);

// 0x407ad8  def range_4060d4.cpp
extern "C" int fn_407ad8(TGraphicSprite *a, int f0, int n);

// 0x407b74  def range_4060d4.cpp
extern "C" char fn_407b74(TGraphicSprite *a, int cast, int reps, int n, int x, int y, unsigned short delay);

// 0x407c40  def range_4060d4.cpp
extern "C" char fn_407c40(TGraphicSprite *a, int cast, int reps, int n, int x, int y, unsigned short delay);

// 0x407d04  def range_4060d4.cpp
extern "C" void fn_407d04(TGraphicSprite *a, TGraphicSprite *src, int div);

// 0x407df0  def range_4060d4.cpp
extern "C" void fn_407df0(TGraphicSprite *a, int i);

// 0x407e74  def range_4060d4.cpp
extern "C" void fn_407e74(TGraphicSprite *a, int now);

// 0x407f48  def range_4060d4.cpp
extern "C" void fn_407f48(TGraphicSprite *p);

// 0x407f78  def range_4060d4.cpp
extern "C" void fn_407f78(TGraphicSprite *a);

// 0x407fdc  def range_4060d4.cpp
//   alt: void fn_407fdc(TGraphicSprite *, int, int)  [range_4060d4]
//   alt: void fn_407fdc(void *, int, void*)  [score_4120cc]
extern "C" void fn_407fdc(TGraphicSprite *t, int a, SpriteCb b);

// 0x407ffc  def sprites_407ffc.cpp; hand-set: v is the TSeq::done callback (batch A)
//   alt: char fn_407ffc(TGraphicSprite *, int, int)  [sprites_407ffc]
//   alt: void fn_407ffc(void *, int, int)  [range_4060d4]
extern "C" char fn_407ffc(TGraphicSprite *s, int idx, SpriteCb v);

// 0x40806c  def sprites_407ffc.cpp
//   alt: void fn_40806c(void*)  [range_4060d4, score_4120cc]
extern "C" void fn_40806c(TGraphicSprite *s);

// 0x4080a0  def sprites_407ffc.cpp
//   alt: void fn_4080a0(TGraphicSprite*)  [engine_40b158]
//   alt: void fn_4080a0(void*)  [range_4060d4]
extern "C" char fn_4080a0(TGraphicSprite *s);

// 0x4081b8  def sprites_407ffc.cpp
extern "C" char fn_4081b8(TGraphicSprite *s);
#define Sprite_UpdateMove fn_4081b8

// 0x4086e4  def sprites_407ffc.cpp
//   alt: void fn_4086e4(TGraphicSprite *, int, int)  [game_414240]
//   alt: void fn_4086e4(void *, int, int)  [range_4060d4]
extern "C" char fn_4086e4(TGraphicSprite *s, int dx, int dy);

// 0x408740  def sprites_407ffc.cpp
extern "C" void fn_408740(TGraphicSprite *s, int dx, int dy);

// 0x408760  def sprites_407ffc.cpp
extern "C" char fn_408760(TGraphicSprite *s, int k);

// 0x4087c8  def sprites_407ffc.cpp
//   alt: char fn_4087c8(TGraphicSprite *, int, int)  [sprites_407ffc]
extern "C" char fn_4087c8(TGraphicSprite *s, TScene *a, int t);

// 0x408908  def sprites_407ffc.cpp
//   alt: void fn_408908(void*)  [menu_41008c, score_4120cc]
extern "C" void fn_408908(TGraphicSprite *s);

// 0x408924  def sprites_407ffc.cpp
//   alt: void fn_408924(void*)  [menu_41008c, score_4120cc]
extern "C" void fn_408924(TGraphicSprite *s);

// 0x408940  def sprites_408940.cpp
//   alt: void fn_408940(TGraphicSprite *, TStage *, int, char *, char*)  [sprites_408940]
extern "C" void fn_408940(TGraphicSprite *s, TStage *e, int now, char *clickL, char *clickR, char *c);
#define Sprite_UpdateMouse fn_408940

// 0x408f84  def sprites_408f84.cpp
//   alt: void fn_408f84(TGraphicSprite*)  [scene_4099dc]
extern "C" void fn_408f84(TButtonSprite *s);

// 0x409000  def sprites_408f84.cpp
//   alt: void fn_409000(void *, int, int, int, int)  [menu_41008c]
extern "C" void fn_409000(TButtonSprite *s, int a, int b, int c, int d);

// 0x409068  def sprites_408f84.cpp; hand-set: id is a sound name passed to fn_40cdec (batch A)
//   alt: void fn_409068(TButtonSprite *, int, int)  [sprites_408f84, menu_41008c]
extern "C" void fn_409068(TButtonSprite *s, const char *id, int arg);

// 0x4090b4  def sprites_408f84.cpp
//   alt: void fn_4090b4(void *, const char *, int)  [menu_41008c]
//   alt: void fn_4090b4(TButtonSprite *, int, int)  [sprites_408f84]
extern "C" void fn_4090b4(TButtonSprite *s, const char *id, int arg);

// 0x4091ac  def sprites_408f84.cpp
extern "C" void fn_4091ac(TButtonSprite *s);

// 0x409234  def sprites_408f84.cpp
//   alt: void fn_409234(TGraphicSprite*)  [about_40f7fc]
extern "C" void fn_409234(TButtonSprite *s);

// 0x4092bc  def sprites_4092bc.cpp
//   alt: void fn_4092bc(TGraphicSprite *, TStage *, int, char *, char *, char*)  [scene_409dac]
//   alt: void fn_4092bc(TButtonSprite *, TStage *, int, char *, char*)  [sprites_4092bc]
extern "C" void fn_4092bc(TButtonSprite *b, TStage *e, int now, char *clickL, char *clickR, char *c);
#define Button_Update fn_4092bc

// 0x4096dc  def sprites_4096dc.cpp
extern "C" char fn_4096dc(TSpriteGroup *g, TGraphicSprite *s);

// 0x40971c  member TSpriteGroup::fn_40971c (MATCH); def sprites_40971c.cpp
extern "C" void fn_40971c(TSpriteGroup *self);

// 0x409754  member TSpriteGroup::fn_409754 (MATCH); def sprites_40971c.cpp
//   alt: void fn_409754(void *, int)  [exit_411af4, score_4120cc, unit_411]
extern "C" void fn_409754(TSpriteGroup *self, int t);

// 0x409790  member TSpriteGroup::fn_409790 (MATCH); def sprites_40971c.cpp
extern "C" void fn_409790(TSpriteGroup *self, int start, int n);

// 0x4097e8  member TSpriteGroup::fn_4097e8 (MATCH); def sprites_40971c.cpp
//   alt: void fn_4097e8(void *, int, int, int)  [exit_411af4, unit_411]
extern "C" void fn_4097e8(TSpriteGroup *self, int start, int n, int t);

// 0x4099dc  def scene_4099dc.cpp
extern "C" void fn_4099dc(TScene *sc);

// 0x409a28  def scene_4099dc.cpp
extern "C" void fn_409a28(TScene *sc);

// 0x409a6c  def scene_4099dc.cpp
extern "C" void fn_409a6c(TScene *sc);
#define Scene_Start fn_409a6c

// 0x409b44  def scene_4099dc.cpp
extern "C" void fn_409b44(TScene *sc);
#define Scene_Stop fn_409b44

// 0x409bb8  def scene_4099dc.cpp
//   alt: void fn_409bb8(TScene *, void*)  [about_40f7fc]
//   alt: void fn_409bb8(TScene *, TScene*)  [exit_411af4, game_41c674, opening_41e01c, unit_411]
extern "C" void fn_409bb8(TScene *sc, TStage *e);
#define Scene_SetStage fn_409bb8

// 0x409bcc  def scene_4099dc.cpp
//   alt: void fn_409bcc(TScene *, void*)  [scene_409fc8]
extern "C" void fn_409bcc(TScene *sc, TGraphicSprite *s);
#define Scene_AddSprite fn_409bcc

// 0x409c00  def scene_4099dc.cpp
extern "C" void fn_409c00(TScene *sc, TSceneButton *b);

// 0x409c18  def scene_4099dc.cpp
extern "C" void fn_409c18(TScene *sc);

// 0x409c50  def scene_409c50.cpp
extern "C" void fn_409c50(TScene *sc);

// 0x409dac  def scene_409dac.cpp
extern "C" int fn_409dac(TScene *sc);

// 0x409fa8  def sprites_409fa8.cpp; hand-set: sets TScene::onKeyDown (0x24); sprites_409fa8 stores an int
//   alt: void fn_409fa8(TScene *, void*)  [about_40f7fc]
//   alt: void fn_409fa8(void *, void*)  [menu_41008c]
//   alt: void fn_409fa8(TScene *, int)  [sprites_409fa8]
extern "C" void fn_409fa8(TScene *sc, SceneKeyCb cb);

// 0x409fb8  def sprites_409fa8.cpp; hand-set: sets TScene::onKeyUp (0x28)
//   alt: void fn_409fb8(TScene *, int)  [sprites_409fa8]
extern "C" void fn_409fb8(TScene *sc, SceneKeyCb cb);

// 0x409fc8  def scene_409fc8.cpp
extern "C" TGraphicSprite *fn_409fc8(TScene *sc, const char *name, const char *cast, TPoint p);

// 0x40a06c  def scene_409fc8.cpp
//   alt: void fn_40a06c(TScene *, const char *, const char*)  [about_40f7fc]
//   alt: void* fn_40a06c(void *, const char *, const char*)  [menu_41008c]
extern "C" TGraphicSprite *fn_40a06c(TScene *sc, const char *name, const char *cast);

// 0x40a0d8  def scene_40a0d8.cpp
extern "C" TGraphicSprite *fn_40a0d8(TScene *sc, const char *name, const char *srcName);

// 0x40a1fc  member TScene::fn_40a1fc (MATCH); def scene_40a1fc.cpp
extern "C" TButtonSprite *fn_40a1fc(TScene *self, const char *name, const char *b, const char *c, TPoint p);

// 0x40a288  def scene_409fc8.cpp
extern "C" TButtonSprite *fn_40a288(TScene *sc, const char *name, const char *b, const char *c, TPoint pt);

// 0x40a2c0  def scene_409fc8.cpp
//   alt: TButtonSprite* fn_40a2c0(void *, const char *, const char *, const char*)  [menu_41008c]
extern "C" TButtonSprite *fn_40a2c0(TScene *sc, const char *name, const char *b, const char *c);

// 0x40a348  def scene_409fc8.cpp
extern "C" void fn_40a348(TScene *sc, const char *n1, const char *n2);

// 0x40a3a4  def scene_40a3a4.cpp
//   alt: TGraphicSprite* fn_40a3a4(void *, const char *, void *, int, int, int, int, int)  [menu_41008c]
extern "C" TGraphicSprite *fn_40a3a4(TScene *sc, const char *name, TGraphicSprite *ref, char f, int ml, int mr, int mt, int mb);

// 0x40a520  member TScene::fn_40a520 (MATCH); def scene_40a520.cpp
extern "C" int fn_40a520(TScene *self, const char *nm);
#define Scene_FindSpriteIndex fn_40a520

// 0x40a598  member TScene::fn_40a598 (MATCH); def scene_40a520.cpp
//   alt: TGraphicSprite* fn_40a598(void *, const char*)  [egg_40fa1c, unit_411]
//   alt: void* fn_40a598(TScene *, const char*)  [menu_41008c, score_4120cc]
extern "C" TGraphicSprite *fn_40a598(TScene *self, const char *nm);
#define Scene_FindSprite fn_40a598

// 0x40a5d4  def engine_40a5d4.cpp
//   alt: ERect fn_40a5d4(TStage *, ERect&)  [engine_40af80, engine_40b8fc, engine_40ba68]
extern "C" RectPod fn_40a5d4(TStage *e, RectPod &r);

// 0x40a63c  def engine_40a5d4.cpp
//   alt: ERect fn_40a63c(TStage *, ERect&)  [engine_40af80]
extern "C" RectPod fn_40a63c(TStage *e, RectPod &r);

// 0x40a6a4  def engine_40a5d4.cpp
extern "C" TPoint fn_40a6a4(TStage *e, TPoint &p);

// 0x40a6e4  def engine_40a6e4.cpp
extern "C" void __fastcall fn_40a6e4(TStage *e, void *sender, char shift, int x, int y);
#define Stage_MouseMove fn_40a6e4

// 0x40a764  def engine_40a6e4.cpp
extern "C" void __fastcall fn_40a764(TStage *e, void *sender, char button, char shift, int x, int y);
#define Stage_MouseDown fn_40a764

// 0x40a7e4  def engine_40a6e4.cpp
extern "C" void __fastcall fn_40a7e4(TStage *e, void *sender, char button, char shift, int x, int y);
#define Stage_MouseUp fn_40a7e4

// 0x40a844  def engine_40a844.cpp
extern "C" void fn_40a844(TStage *e, int color);

// 0x40a948  def engine_40a948.cpp
extern "C" void fn_40a948(TStage *e, int flags);

// 0x40ae30  def stage_40ae30.cpp
//   alt: void fn_40ae30(void *, TScene*)  [about_40f7fc]
//   alt: void fn_40ae30(TScene *, TScene*)  [exit_411af4, game_41c674, opening_41e01c, unit_411]
extern "C" void fn_40ae30(TStage *e, TScene *p);

// 0x40ae60  def stage_40ae30.cpp
extern "C" void fn_40ae60(TStage *st);

// 0x40ae70  def stage_40ae30.cpp
extern "C" void fn_40ae70(TStage *st);

// 0x40ae80  member TStage::fn_40ae80 (MATCH); def engine_40ae80.cpp
extern "C" void fn_40ae80(TStage *self, ERect r);
#define Stage_AddDirty fn_40ae80

// 0x40af80  member TStage::fn_40af80 (MATCH); def engine_40af80.cpp
extern "C" void fn_40af80(TStage *self, ERect r);

// 0x40b158  def engine_40b158.cpp
extern "C" void fn_40b158(TStage *e);

// 0x40b1d4  def engine_40b158.cpp
//   alt: void fn_40b1d4(void*)  [webtrack_40e5ec]
extern "C" void fn_40b1d4(TStage *e);

// 0x40b210  def engine_40b158.cpp
extern "C" void fn_40b210(TStage *e, void *a, void *b, void *c, DWORD rop);

// 0x40b234  def engine_40b158.cpp
extern "C" void fn_40b234(TStage *e);

// 0x40b264  def engine_40b158.cpp
extern "C" void fn_40b264(TStage *e);

// 0x40b290  def engine_40b158.cpp
extern "C" void fn_40b290(TStage *e);

// 0x40b2c8  def engine_40b158.cpp
extern "C" void fn_40b2c8(TStage *e);

// 0x40b2ec  def engine_40b158.cpp
extern "C" void fn_40b2ec(TStage *e, ERect *a, TGraphicCast *img, ERect *c, char f);

// 0x40b3b0  def engine_40b3b0.cpp
extern "C" char fn_40b3b0(TStage *e, char direct, RectPod *dst, TGraphicCast *c, TPoint *src);

// 0x40b5bc  def engine_40b5bc.cpp
extern "C" char fn_40b5bc(TStage *e, char direct, ERect *r);

// 0x40b6b4  def engine_40b5bc.cpp
extern "C" void fn_40b6b4(TStage *e, ERect *r);

// 0x40b708  def engine_40b5bc.cpp
extern "C" void fn_40b708(TStage *e, ERect area);

// 0x40b8fc  def engine_40b8fc.cpp
extern "C" char fn_40b8fc(ERect &dst, HBITMAP bmp, TPoint &src);

// 0x40b9b8  member TStage::fn_40b9b8 (MATCH); def engine_40ae80.cpp
extern "C" void fn_40b9b8(TStage *self);

// 0x40ba68  member TStage::fn_40ba68 (MATCH); def engine_40ba68.cpp
extern "C" void fn_40ba68(TStage *self);

// 0x40bcb8  def engine_40b8fc.cpp
extern "C" char fn_40bcb8(TStage *e);

// 0x40bd3c  def engine_40bd3c.cpp
extern "C" void __fastcall fn_40bd3c(TStage *e, void *sender, unsigned short &key, ShiftState shift);

// 0x40bda4  def engine_40bd3c.cpp
extern "C" void __fastcall fn_40bda4(TStage *e, void *sender, unsigned short &key, ShiftState shift);

// 0x40bdec  def engine_40bdec.cpp
//   alt: void fn_40bdec(TScene*)  [about_40f7fc]
extern "C" char fn_40bdec(TScene *sc);

// 0x40be34  def engine_40bdec.cpp
extern "C" TScene *fn_40be34(TStage *e);

// 0x40bef4  def engine_40bdec.cpp
extern "C" TScene *fn_40bef4(TStage *e, TScene *next, TScene *ret);

// 0x40bf14  def engine_40bdec.cpp
extern "C" TScene *fn_40bf14(TStage *e, TScene *next);

// 0x40bf2c  member TStage::fn_40bf2c (MATCH); def engine_40ae80.cpp
//   alt: void fn_40bf2c(void *, const char*)  [exit_411af4, game_41c674, opening_41e01c, unit_411]
//   alt: void fn_40bf2c(TStage *, const char*)  [menu_41008c, score_4120cc]
extern "C" TScene *fn_40bf2c(TStage *self, const char *name);

// 0x40bf84  member TStage::fn_40bf84 (MATCH); def engine_40ae80.cpp
//   alt: void fn_40bf84(void *, const char*)  [menu_41008c]
extern "C" TScene *fn_40bf84(TStage *self, const char *name);

// 0x40bfe4  def engine_40bfe4.cpp
//   alt: void fn_40bfe4(TStage *, const char*)  [init_40f350]
extern "C" TScene *fn_40bfe4(TStage *e, const char *first);

// 0x40c130  def engine_40c130.cpp
//   alt: void fn_40c130(void*)  [game_41c674, menu_41008c, opening_41e01c]
extern "C" void fn_40c130(TStage *e);

// 0x40c160  member TStage::fn_40c160 (MATCH); def engine_40ae80.cpp
extern "C" int fn_40c160(TStage *self, const char *name);

// 0x40c1b4  def engine_40c130.cpp
extern "C" void fn_40c1b4();

// 0x40c1f0  def sound_40c1f0.cpp
//   alt: void fn_40c1f0(void *, const char*)  [sound_40c544]
extern "C" bool fn_40c1f0(TSound *snd, const char *name);
#define Sound_Load fn_40c1f0

// 0x40c5dc  def sound_40c5dc.cpp
extern "C" void fn_40c5dc(TSound *s, int flags);
#define Sound_Delete fn_40c5dc

// 0x40c60c  def sound_40c5dc.cpp
extern "C" void fn_40c60c(HWAVEOUT h);

// 0x40c628  def sound_40c5dc.cpp
extern "C" void fn_40c628(HWAVEOUT h, WAVEHDR *hdr, UINT sz);

// 0x40c64c  def sound_40c5dc.cpp
extern "C" void fn_40c64c(HWAVEOUT *h);

// 0x40c670  def sound_40c5dc.cpp
extern "C" MMRESULT fn_40c670(HWAVEOUT h, WAVEHDR *hdr, UINT sz);

// 0x40c6a0  def sound_40c5dc.cpp
extern "C" MMRESULT fn_40c6a0(HWAVEOUT h, WAVEHDR *hdr, UINT sz);

// 0x40c6d0  def sound_40c5dc.cpp
extern "C" MMRESULT fn_40c6d0(HWAVEOUT *h, UINT dev, WAVEFORMATEX *fmt, DWORD_PTR cb, DWORD_PTR inst, DWORD flags);

// 0x40c718  def sound_40c5dc.cpp
extern "C" void CALLBACK fn_40c718(HWAVEOUT h, UINT msg, DWORD_PTR inst, DWORD_PTR p1, DWORD_PTR p2);

// 0x40c740  def sound_40c5dc.cpp
extern "C" void fn_40c740();

// 0x40c75c  def sound_40c5dc.cpp
extern "C" void fn_40c75c(TSoundMgr *p);
#define SoundMgr_Stop fn_40c75c

// 0x40c804  def sound_40c5dc.cpp
extern "C" void fn_40c804(TSoundMgr *p, char flag);

// 0x40c880  member TSoundMgr::fn_40c880 (MATCH); def sound_40c880.cpp
//   alt: void fn_40c880(TSoundMgr *, char)  [sound_40c5dc]
extern "C" char fn_40c880(TSoundMgr *self, char flag);

// 0x40c960  def sound_40c960.cpp
extern "C" char fn_40c960(TSoundMgr *p, TSound *s);

// 0x40ca94  def sound_40ca94.cpp
extern "C" void fn_40ca94(TSoundMgr *p);

// 0x40cab4  def sound_40ca94.cpp
extern "C" int fn_40cab4(TSoundMgr *p);

// 0x40cae0  def sound_40ca94.cpp
//   alt: void fn_40cae0(TSoundMgr *, int, TButtonSprite *, int, int, int)  [sprites_408f84]
//   alt: char fn_40cae0(TSoundMgr *, int, TButtonSprite *, int, int, int)  [sprites_4092bc]
extern "C" char fn_40cae0(TSoundMgr *p, void *owner, TGraphicSprite *s, int id, int prio, SoundDoneCb cb);
#define SoundMgr_PlayId fn_40cae0

// 0x40cb34  def sound_40ca94.cpp
//   alt: void fn_40cb34(void *, TScene *, void *, const char *, int, int)  [egg_40fa1c, menu_41008c, score_4120cc]
//   alt: void fn_40cb34(void *, TScene *, TGraphicSprite *, const char *, int, int)  [game_414240, game_416f10, game_417074, game_41c674, opening_41e01c]
//   alt: char fn_40cb34(TSoundMgr *, int, TGraphicSprite *, const char *, int, int)  [sound_40ca94]
extern "C" char fn_40cb34(TSoundMgr *p, void *owner, TGraphicSprite *s, const char *name, int prio, SoundDoneCb cb);
#define SoundMgr_Play fn_40cb34

// 0x40cb7c  def sound_40ca94.cpp
//   alt: void fn_40cb7c(void *, TScene *, const char*)  [game_416f10]
//   alt: void fn_40cb7c(TSoundMgr *, int, const char*)  [sound_40ca94]
extern "C" void fn_40cb7c(TSoundMgr *p, void *owner, const char *name);

// 0x40cbd0  def sound_40ca94.cpp
//   alt: void fn_40cbd0(void *, TScene*)  [game_414240, game_416f10]
//   alt: void fn_40cbd0(TSoundMgr*)  [sound_40ca94]
extern "C" void fn_40cbd0(TSoundMgr *p, void *unused);

// 0x40cbf0  def sound_40cbf0.cpp
//   alt: char fn_40cbf0(TSoundMgr *, int, void *, int, int, int)  [sound_40c880]
extern "C" char fn_40cbf0(TSoundMgr *p, void *owner, TGraphicSprite *s, int id, int prio, SoundDoneCb cb);

// 0x40ccd0  def sound_40ca94.cpp
//   alt: void fn_40ccd0(TSoundMgr *, int)  [sound_40ca94]
extern "C" void fn_40ccd0(TSoundMgr *p, void *owner);

// 0x40cd80  def sound_40ca94.cpp
extern "C" void fn_40cd80(TSoundMgr *p);

// 0x40cddc  def sound_40ca94.cpp
extern "C" char fn_40cddc(TSoundMgr *p);

// 0x40cdec  def sound_40ca94.cpp
//   alt: int fn_40cdec(TSoundMgr *, int)  [sprites_408f84]
extern "C" int fn_40cdec(TSoundMgr *p, const char *name);

// 0x40ce60  def sound_40ca94.cpp
extern "C" int fn_40ce60(TSoundMgr *p, const char *name);

// 0x40cebc  def sound_40ca94.cpp
extern "C" int fn_40cebc(TSoundMgr *p);

// 0x40cf24  def sound_40cf24.cpp
extern "C" void fn_40cf24(TSoundMgr *p);

// 0x40cfb0  def sound_40cf24.cpp
//   alt: int fn_40cfb0(TSoundMgr *, const char *, char)  [sound_40cf24]
extern "C" int fn_40cfb0(TSoundMgr *p, const char *name, char keep);
#define SoundMgr_Load fn_40cfb0

// 0x40d058  def sound_40ca94.cpp
//   alt: void fn_40d058(void *, const char *, int)  [init_40f350]
extern "C" void fn_40d058(TSoundMgr *p, const char *name, int arg);

// 0x40d080  def sound_40ca94.cpp
//   alt: void fn_40d080(void *, const char *, int)  [init_40f350]
extern "C" void fn_40d080(TSoundMgr *p, const char *name, int arg);

// 0x40d0a8  def sound_40ca94.cpp
extern "C" char fn_40d0a8(TSoundMgr *p, void *owner, TGraphicSprite *s, int id, int prio, int dur, SoundDoneCb cb);

// 0x40d100  def sound_40ca94.cpp
//   alt: void fn_40d100(void *, TScene *, void *, const char *, int, int, int)  [menu_41008c, score_4120cc]
//   alt: char fn_40d100(TSoundMgr *, int, TGraphicSprite *, const char *, int, int, int)  [sound_40ca94]
extern "C" char fn_40d100(TSoundMgr *p, void *owner, TGraphicSprite *s, const char *name, int prio, int dur, SoundDoneCb cb);

// 0x40d148  def sound_40ca94.cpp
//   alt: void fn_40d148(void*)  [engine_40b8fc]
extern "C" void fn_40d148(TSoundMgr *p);

// 0x40d250  member TPackedResources::fn_40d250 (MATCH); def packres_40d250.cpp
extern "C" int fn_40d250(TPackedResources *self);

// 0x40d2c8  def packres_40d2c8.cpp
extern "C" char fn_40d2c8(TPackedResources *pr);

// 0x40d3f8  def packres_40d3f8.cpp
extern "C" int fn_40d3f8(TPackedResources *pr, const char *name);

// 0x40d464  def packres_40d3f8.cpp
extern "C" void *fn_40d464(void *opaque, unsigned items, unsigned size);

// 0x40d488  def packres_40d3f8.cpp
extern "C" void fn_40d488(void *opaque, void *p);

// 0x40d498  def packres_40d3f8.cpp
extern "C" void fn_40d498(ZStream *z);

// 0x40d4bc  def packres_40d4bc.cpp
//   alt: TStreamVmt* fn_40d4bc(TPackedResources *, const char*)  [packres_40d980]
//   alt: Classes::TStream* fn_40d4bc(TPackedResources *, const char*)  [sound_40c1f0]
extern "C" TPackedStream *fn_40d4bc(TPackedResources *pr, const char *name);
#define PackRes_Open fn_40d4bc

// 0x40d568  def packres_40d3f8.cpp
//   alt: int fn_40d568(void *, void*)  [packres_40d3f8]
//   alt: void fn_40d568(TPackedResources *, TStreamVmt*)  [packres_40d980]
//   alt: int fn_40d568(TPackedResources *, Classes::TStream*)  [sound_40c1f0]
extern "C" int fn_40d568(TPackedResources *a, void *s);   // s: the stream from fn_40d4bc (any spelling)
#define PackRes_Close fn_40d568

// 0x40d578  def packres_40d578.cpp
//   alt: void fn_40d578(TPackedResources *, TStreamVmt *, TGraphicCast *, void *, const char *, char, char)  [packres_40d578, packres_40d6f0]
extern "C" void fn_40d578(TPackedResources *pr, TStreamVmt *st, TGraphicCast *b, BmpFileHdr *pal, const char *name, char swap, char flip);

// 0x40d6f0  def packres_40d6f0.cpp
//   alt: void fn_40d6f0(TPackedResources *, void *, TStreamVmt *, TGraphicCast *, BmpFileHdr *, const char *, char, char)  [packres_40d6f0]
extern "C" void fn_40d6f0(TPackedResources *pr, CastEntry *c, TStreamVmt *st, TGraphicCast *b, BmpFileHdr *h, const char *name, char swap, char flip);

// 0x40d980  def packres_40d980.cpp
//   alt: void fn_40d980(void *, void *, void *, int, int, char, char)  [range_403540]
extern "C" HBITMAP fn_40d980(TPackedResources *pr, TGraphicCast *b, const char *name, const char *res, void *pal, char swap, char flip);

// 0x40db00  def packres_40db00.cpp
//   alt: void fn_40db00(TPackedResources *, void *, void*)  [packres_40dbe0]
//   alt: char fn_40db00(void *, const char *, char*)  [range_403540]
extern "C" char fn_40db00(TPackedResources *pr, const char *ext, char *out);

// 0x40dbe0  def packres_40dbe0.cpp
//   alt: void fn_40dbe0(TPackedResources *, void *, void*)  [packres_40dbe0]
//   alt: char fn_40dbe0(void *, const char *, char*)  [range_403540]
extern "C" char fn_40dbe0(TPackedResources *pr, const char *ext, char *out);   // resets the cursor, then fn_40db00

// 0x40dc04  def packres_40dbe0.cpp
extern "C" void fn_40dc04(const char *msg);

// 0x40dc1c  def packres_40dc1c.cpp
//   alt: void fn_40dc1c(void *, void*)  [ctors_4052dc]
extern "C" char fn_40dc1c(TPackedResources *pr, void *arg);

// 0x40dee0  def packres_40dee0.cpp
extern "C" int __fastcall fn_40dee0(TPackedStreamData *s, char *buf, int n);

// 0x40dfd0  def packres_40dbe0.cpp
extern "C" void __fastcall fn_40dfd0(TPackedStreamData *s, int v);

// 0x40dfec  def packres_40dbe0.cpp
extern "C" int __fastcall fn_40dfec(TPackedStreamData *s, const void *buf, int n);

// 0x40e004  def packres_40dbe0.cpp
extern "C" int __fastcall fn_40e004(TPackedStreamData *s, int off, unsigned short origin);

// 0x40e210  def packres_40dbe0.cpp
extern "C" void fn_40e210(TSaveCasts *c, int flags);

// 0x40e250  def packres_40dbe0.cpp
extern "C" char fn_40e250(TSaveCasts *c);

// 0x40e2a0  def packres_40dbe0.cpp
extern "C" char fn_40e2a0(TSaveCasts *c, const char *name);

// 0x40e304  def packres_40dbe0.cpp
extern "C" CastEntry *fn_40e304(TSaveCasts *c, const char *name);

// 0x40e354  def packres_40dbe0.cpp
//   alt: char fn_40e354(TSaveCasts*)  [packres_40dbe0]
extern "C" char fn_40e354(TSaveCasts *c, void *arg);

// 0x40e3d4  def packres_40dbe0.cpp
extern "C" char fn_40e3d4(TSaveParms *p);

// 0x40e424  def packres_40dbe0.cpp
extern "C" char fn_40e424(TSaveParms *p, const char *name);

// 0x40e488  def packres_40dbe0.cpp
extern "C" ParmEntry *fn_40e488(TSaveParms *p, const char *name);

// 0x40e4d8  def packres_40dbe0.cpp
//   alt: void fn_40e4d8(void *, TScene*)  [scene_4099dc]
extern "C" char fn_40e4d8(TSaveParms *sp, TScene *sc);

// 0x40e55c  def packres_40dbe0.cpp
//   alt: char fn_40e55c(TSaveParms*)  [packres_40dbe0]
extern "C" char fn_40e55c(TSaveParms *p, TStage *e);

// 0x40e584  def packres_40dbe0.cpp
extern "C" void __fastcall fn_40e584(int err);

// 0x40e5ac  def packres_40dbe0.cpp
extern "C" void __fastcall fn_40e5ac(void *self, void *sender, void *socket);

// 0x40e5cc  def packres_40dbe0.cpp
extern "C" void __fastcall fn_40e5cc(void *self, void *sender, void *socket);

// 0x40e5ec  def webtrack_40e5ec.cpp
extern "C" void __fastcall fn_40e5ec(TWebTrack *self, void *sender, void *socket, int ev, int &code);

// 0x40e624  def webtrack_40e5ec.cpp
extern "C" void __fastcall fn_40e624(TWebTrack *self, void *sender, void *socket);

// 0x40e664  def webtrack_40e5ec.cpp
extern "C" void __fastcall fn_40e664(TWebTrack *self, void *sender, void *socket);

// 0x40e684  def webtrack_40e5ec.cpp
extern "C" void __fastcall fn_40e684(TWebTrack *self, void *sender, void *socket);

// 0x40e6ac  def webtrack_40e5ec.cpp
extern "C" void __fastcall fn_40e6ac(TWebTrack *self, void *sender, void *socket);

// 0x40e6d4  def webtrack_40e5ec.cpp
extern "C" void fn_40e6d4(TWebTrack *self);

// 0x40e714  def webtrack_40e5ec.cpp
extern "C" void fn_40e714(TWebTrack *self);

// 0x40e7a4  def webtrack_40e5ec.cpp
//   alt: void fn_40e7a4(void*)  [engine_40b8fc]
extern "C" void fn_40e7a4(TWebTrack *self);

// 0x40e800  def webtrack_40e5ec.cpp
extern "C" bool fn_40e800();

// 0x40ebd0  def webtrack_40e5ec.cpp
extern "C" void fn_40ebd0(TWebTrack *self, int flags);

// 0x40ec20  def webtrack_40e5ec.cpp (batch E: no parameter; main_401508 calls it with none)
extern "C" void fn_40ec20();

// 0x40ec34  def webtrack_40e5ec.cpp
extern "C" bool fn_40ec34(void *form, int w, int h);

// 0x40f350  def init_40f350.cpp
extern "C" void fn_40f350(TStage *g);

// 0x40f364  def init_40f350.cpp
extern "C" void fn_40f364(TStage *g);

// 0x40f3c0  def init_40f350.cpp
extern "C" void fn_40f3c0();

// 0x40f3dc  def init_40f350.cpp
extern "C" void fn_40f3dc();

// 0x40f460  def init_40f350.cpp
extern "C" void fn_40f460(void *mainForm);

// 0x40f674  def threads_40f674.cpp
extern "C" void __fastcall fn_40f674(TMyThread *self);

// 0x40f7fc  def about_40f7fc.cpp
extern "C" void fn_40f7fc(TScene *s, void *sender, unsigned short &key, ShiftState shift);

// 0x40f828  def about_40f7fc.cpp
extern "C" void fn_40f828(TScene *s);

// 0x40f830  def about_40f7fc.cpp
extern "C" void fn_40f830(TScene *s, TGraphicSprite *k);

// 0x40f85c  def about_40f7fc.cpp
extern "C" void fn_40f85c(TScene *s, char again);

// 0x40f934  def about_40f7fc.cpp
//   alt: void fn_40f934(void*)  [about_40f7fc]
extern "C" void fn_40f934(TStage *game);

// 0x40fa1c  def egg_40fa1c.cpp
extern "C" void fn_40fa1c(TScene *s, TGraphicSprite *unused);

// 0x40fa2a  def egg_40fa1c.cpp
extern "C" void fn_40fa2a(TScene *s, TGraphicSprite *spr);

// 0x40fa87  def egg_40fa1c.cpp
extern "C" void fn_40fa87(TScene *s, TGraphicSprite *spr);

// 0x40fae4  def egg_40fa1c.cpp
extern "C" void fn_40fae4(TScene *s, TGraphicSprite *spr);

// 0x40fb41  def egg_40fa1c.cpp
extern "C" void fn_40fb41(TScene *s, TGraphicSprite *spr);

// 0x40fb9e  def egg_40fa1c.cpp
extern "C" void fn_40fb9e(TScene *s, TGraphicSprite *spr);

// 0x40fbfb  def egg_40fa1c.cpp
extern "C" void fn_40fbfb(TScene *s);

// 0x40fc09  def egg_40fa1c.cpp
extern "C" void fn_40fc09(TScene *s);

// 0x40fc3c  def egg_40fa1c.cpp
extern "C" void fn_40fc3c(TScene *s, int first, int unused, const char ** lines, int n);

// 0x40fd94  def egg_40fa1c.cpp
extern "C" void fn_40fd94(TScene *s, const char ** lines, int n);

// 0x40feb0  def egg_40fa1c.cpp
extern "C" void fn_40feb0(TScene *s);

// 0x41008c  def menu_41008c.cpp
extern "C" void fn_41008c(TScene *s, int a, unsigned short *key);

// 0x4100e4  def menu_41008c.cpp
extern "C" LONG fn_4100e4(HKEY root, const char *sub, char *out);

// 0x410144  def menu_41008c.cpp
extern "C" bool fn_410144(const char *url);

// 0x410278  def menu_41008c.cpp
extern "C" void fn_410278(TScene *s);

// 0x4102e4  def menu_41008c.cpp
extern "C" void fn_4102e4(TScene *s);

// 0x410350  def menu_41008c.cpp
extern "C" void fn_410350(TScene *s);

// 0x4103bc  def menu_41008c.cpp
extern "C" void fn_4103bc(TScene *s, TGraphicSprite *k);

// 0x410400  def menu_41008c.cpp
extern "C" void fn_410400(TScene *s, TGraphicSprite *k);

// 0x410440  def menu_41008c.cpp
extern "C" void fn_410440(TScene *s, TGraphicSprite *k);

// 0x41047c  def menu_41008c.cpp
extern "C" void fn_41047c(TScene *s, TGraphicSprite *k);

// 0x410610  def menu_41008c.cpp
extern "C" void fn_410610(TScene *s, TGraphicSprite *k);

// 0x410648  def menu_41008c.cpp
extern "C" void fn_410648(TScene *s, TGraphicSprite *k);

// 0x410680  def menu_41008c.cpp
extern "C" void fn_410680(TScene *s);

// 0x410690  def menu_41008c.cpp
extern "C" void fn_410690(TScene *s, TGraphicSprite *k);

// 0x4106fc  def menu_41008c.cpp
extern "C" void fn_4106fc(TScene *s);

// 0x41070c  def menu_41008c.cpp
extern "C" void fn_41070c(TScene *s);

// 0x410764  def menu_41008c.cpp
extern "C" void fn_410764(TScene *s);

// 0x4108bc  def menu_41008c.cpp
extern "C" void fn_4108bc(TScene *s, TGraphicSprite *spr);

// 0x410914  def menu_41008c.cpp
extern "C" void fn_410914(TScene *s);

// 0x410968  def menu_41008c.cpp; hand-set: pilot menu_41008c (SpriteCb target)
extern "C" void fn_410968(TScene *s, TGraphicSprite *spr);

// 0x410978  def menu_41008c.cpp; hand-set: pilot menu_41008c (SpriteCb target)
extern "C" void fn_410978(TScene *s, TGraphicSprite *spr);

// 0x410a40  def menu_41008c.cpp
extern "C" void fn_410a40(TScene *s);

// 0x410a74  def menu_41008c.cpp
extern "C" void fn_410a74(TScene *s);

// 0x410b90  def menu_41008c.cpp
extern "C" void fn_410b90(TScene *s);

// 0x410bd0  def menu_41008c.cpp
extern "C" void fn_410bd0(TScene *s);

// 0x410c18  def menu_41008c.cpp
extern "C" void fn_410c18(TScene *s);

// 0x410e8c  def menu_41008c.cpp
extern "C" void fn_410e8c(TScene *s);

// 0x411194  def menu_41008c.cpp
extern "C" void fn_411194(TScene *s);

// 0x4111dc  def menu_41008c.cpp
extern "C" void fn_4111dc(TScene *s);

// 0x411218  def menu_41008c.cpp
//   alt: void fn_411218(void*)  [exit_411af4, unit_411]
extern "C" void fn_411218(TScene *s);

// 0x4112b8  def menu_41008c.cpp
//   alt: void fn_4112b8(void*)  [exit_411af4, unit_411]
extern "C" void fn_4112b8(TScene *s, char loaded);

// 0x411bac  def unit_411.cpp
//   alt: void fn_411bac(void *, void*)  [game_418274]
extern "C" void fn_411bac(TScene *g, TGraphicSprite *k);

// 0x411bdc  def unit_411.cpp; hand-set: pilot unit_411: the score scene
extern "C" void fn_411bdc(TScene *s);

// 0x411c24  def unit_411.cpp; hand-set: pilot unit_411
extern "C" void fn_411c24(TScene *s, int frame);

// 0x411c3c  def unit_411.cpp; hand-set: pilot unit_411
extern "C" void fn_411c3c(TScene *s, int score);

// 0x411c4c  def unit_411.cpp; hand-set: pilot unit_411
extern "C" void fn_411c4c(TScene *s);

// 0x411c98  def unit_411.cpp; hand-set: pilot unit_411
extern "C" void fn_411c98(TScene *s);

// 0x41208c  def unit_411.cpp
//   alt: void fn_41208c(void *, int)  [exit_411af4, unit_411]
extern "C" void fn_41208c(TScene *s, int pins);

// 0x4120b4  def unit_411.cpp; hand-set: pilot unit_411
//   alt: void fn_4120b4(void*)  [exit_411af4, unit_411]
extern "C" void fn_4120b4(TScene *s);

// 0x4120cc  def score_4120cc.cpp
extern "C" void fn_4120cc(TScene *s);

// 0x412260  def score_4120cc.cpp
extern "C" void fn_412260(TScene *s);

// 0x4122e0  def score_4120cc.cpp
extern "C" void fn_4122e0(TScene *s, TGraphicSprite *spr);

// 0x412358  def score_4120cc.cpp
//   alt: void fn_412358()  [game_416f10]
extern "C" void fn_412358();

// 0x41237c  def score_4120cc.cpp
//   alt: int fn_41237c()  [score_4120cc]
extern "C" int fn_41237c(TScene *self);

// 0x4124a4  def score_4120cc.cpp
extern "C" void fn_4124a4(TScene *s, TGraphicSprite *spr);

// 0x4124f8  def score_4120cc.cpp
extern "C" int fn_4124f8(TScene *s, bool left);

// 0x4125f0  def score_4120cc.cpp
extern "C" void fn_4125f0(TScene *s);

// 0x412618  def score_4120cc.cpp
extern "C" void fn_412618(TScene *s);

// 0x4126f8  def score_4120cc.cpp
extern "C" void fn_4126f8(TScene *s, TGraphicSprite *spr);

// 0x412740  def score_4120cc.cpp
extern "C" void fn_412740(TScene *s);

// 0x412768  def score_4120cc.cpp
extern "C" void fn_412768(TScene *s);

// 0x4127c0  def score_4120cc.cpp
extern "C" void fn_4127c0(TScene *s);

// 0x412818  def score_4120cc.cpp
extern "C" void fn_412818(TScene *s);

// 0x412834  def score_4120cc.cpp
extern "C" void fn_412834(TScene *s, TGraphicSprite *unused);

// 0x412850  def score_4120cc.cpp
//   alt: void fn_412850(TScene *, void*)  [score_4120cc]
extern "C" void fn_412850(TScene *s, TGraphicSprite *spr);

// 0x4128b4  def score_4120cc.cpp
//   alt: void fn_4128b4(TScene*)  [score_4120cc]
extern "C" void fn_4128b4(TScene *s, TGraphicSprite *s1);

// 0x4128c4  def score_4120cc.cpp
extern "C" void fn_4128c4(TScene *s, TGraphicSprite *unused);

// 0x4128d4  def score_4120cc.cpp
//   alt: void fn_4128d4(TScene *, void*)  [score_4120cc]
extern "C" void fn_4128d4(TScene *s, TGraphicSprite *spr);

// 0x412904  def score_4120cc.cpp
//   alt: void fn_412904(TScene *, int)  [game_416f10]
extern "C" void fn_412904(TScene *s, unsigned short key);

// 0x41296c  def score_4120cc.cpp
//   alt: void fn_41296c(void *, int, unsigned short *, int)  [game_418274]
extern "C" void fn_41296c(TScene *s, void *sender, unsigned short &key, ShiftState shift);

// 0x412a74  def score_4120cc.cpp
extern "C" void fn_412a74(TScene *s, unsigned short key);

// 0x412a7c  def score_4120cc.cpp
//   alt: void fn_412a7c(void *, int, unsigned short*)  [game_418274]
extern "C" void fn_412a7c(TScene *s, void *sender, unsigned short &key, ShiftState shift);

// 0x412a94  def score_4120cc.cpp
extern "C" void fn_412a94(TScene *s);

// 0x412b44  def score_4120cc.cpp
extern "C" void fn_412b44(TScene *s);

// 0x412bc4  def score_4120cc.cpp
//   alt: void fn_412bc4()  [score_4120cc]
extern "C" void fn_412bc4(TScene *self);

// 0x412c30  def score_4120cc.cpp
//   alt: int fn_412c30()  [score_4120cc]
extern "C" int fn_412c30(TScene *self);

// 0x412c60  def score_4120cc.cpp
extern "C" void fn_412c60(TScene *s);

// 0x412c80  def score_4120cc.cpp
extern "C" void fn_412c80(TScene *s);

// 0x412cc8  def score_4120cc.cpp
//   alt: bool fn_412cc8()  [score_4120cc]
extern "C" bool fn_412cc8(TScene *self);

// 0x412cfc  def score_4120cc.cpp
extern "C" void fn_412cfc(TScene *s);

// 0x412d2c  def score_4120cc.cpp
extern "C" void fn_412d2c(TScene *s);

// 0x412d5c  def score_4120cc.cpp
extern "C" void fn_412d5c(TScene *s, TGraphicSprite *unused);

// 0x412d74  def score_4120cc.cpp
extern "C" void fn_412d74(TScene *s, TGraphicSprite *spr);

// 0x412d90  def score_4120cc.cpp
extern "C" void fn_412d90(TScene *s);

// 0x412e08  def score_4120cc.cpp
extern "C" void fn_412e08(TScene *s);

// 0x412e70  def score_4120cc.cpp
extern "C" void fn_412e70(int i, int frame);

// 0x412ec8  def score_4120cc.cpp
extern "C" void fn_412ec8(int i, int a, int b, int c);

// 0x412f18  def score_4120cc.cpp
extern "C" void fn_412f18(int i, int a, int b, int c);

// 0x412f68  def score_4120cc.cpp
extern "C" void fn_412f68(int i, int a, int b, int c);

// 0x412fdc  def score_4120cc.cpp
extern "C" void fn_412fdc(int i);

// 0x4130a4  def score_4120cc.cpp
extern "C" void fn_4130a4(int i);

// 0x413168  def score_4120cc.cpp
extern "C" void fn_413168(int i);

// 0x413244  def score_4120cc.cpp
extern "C" void fn_413244(int i, TPoint d);

// 0x4133cc  def score_4120cc.cpp
extern "C" void fn_4133cc(int i);

// 0x413490  def score_4120cc.cpp
extern "C" void fn_413490(int i, int frame);

// 0x4134d4  def score_4120cc.cpp
extern "C" void fn_4134d4(int i, int frame);

// 0x41353c  def score_4120cc.cpp
extern "C" void fn_41353c(int i, int a, SpriteCb done);

// 0x4135ac  def score_4120cc.cpp
extern "C" void fn_4135ac(int i, int a, SpriteCb done);

// 0x4135f4  def score_4120cc.cpp
extern "C" void fn_4135f4(int i);

// 0x41362c  def score_4120cc.cpp
extern "C" void fn_41362c(int i);

// 0x41368c  def score_4120cc.cpp
extern "C" void fn_41368c(int i);

// 0x4136c4  def score_4120cc.cpp
extern "C" void fn_4136c4(TScene *s, TGraphicSprite *e);

// 0x413720  def score_4120cc.cpp
extern "C" void fn_413720(TScene *s, TGraphicSprite *e);

// 0x413788  def score_4120cc.cpp
extern "C" void fn_413788(TScene *s, TGraphicSprite *e);

// 0x4137d0  def score_4120cc.cpp
extern "C" void fn_4137d0(TScene *s, TGraphicSprite *e);

// 0x4137f8  def score_4120cc.cpp
extern "C" void fn_4137f8(TScene *s, TGraphicSprite *e);

// 0x413834  def score_4120cc.cpp
extern "C" void fn_413834(TScene *s);

// 0x4138a0  def score_4120cc.cpp
extern "C" void fn_4138a0(TScene *s, TGraphicSprite *e);

// 0x4138f8  def score_4120cc.cpp
extern "C" void fn_4138f8(TScene *s, TGraphicSprite *e);

// 0x413a04  def score_4120cc.cpp
extern "C" void fn_413a04(TScene *s, TGraphicSprite *e);

// 0x413a48  def score_4120cc.cpp
extern "C" void fn_413a48(TScene *s, TGraphicSprite *e);

// 0x413a8c  def score_4120cc.cpp
extern "C" void fn_413a8c(TScene *s, TGraphicSprite *e);

// 0x413af4  def score_4120cc.cpp
extern "C" void fn_413af4(TScene *s, TGraphicSprite *e);

// 0x413b4c  def score_4120cc.cpp
extern "C" void fn_413b4c(TScene *s, TGraphicSprite *e);

// 0x413bac  def score_4120cc.cpp
extern "C" void fn_413bac(TScene *s, TGraphicSprite *e);

// 0x413bd8  def score_4120cc.cpp
extern "C" void fn_413bd8(TScene *s, TGraphicSprite *e);

// 0x413c1c  def score_4120cc.cpp
extern "C" void fn_413c1c(TScene *s, TGraphicSprite *e);

// 0x413ca0  def score_4120cc.cpp
extern "C" void fn_413ca0(TScene *s, TGraphicSprite *e);

// 0x413d08  def score_4120cc.cpp
extern "C" void fn_413d08(TScene *s, TGraphicSprite *e);

// 0x413d20  def score_4120cc.cpp
extern "C" void fn_413d20(TScene *s, TGraphicSprite *e);

// 0x413d64  def score_4120cc.cpp
extern "C" void fn_413d64(TScene *s, TGraphicSprite *e);

// 0x413de4  def score_4120cc.cpp
extern "C" void fn_413de4(TScene *s, TGraphicSprite *e);

// 0x413e18  def score_4120cc.cpp
extern "C" void fn_413e18(TScene *s, TGraphicSprite *e);

// 0x413e40  def score_4120cc.cpp
extern "C" void fn_413e40(TScene *s, TGraphicSprite *e);

// 0x413ee4  def score_4120cc.cpp
extern "C" void fn_413ee4(TScene *s, TGraphicSprite *e);

// 0x413f20  def score_4120cc.cpp
extern "C" void fn_413f20(TScene *s, TGraphicSprite *e);

// 0x413ffc  def score_4120cc.cpp
extern "C" void fn_413ffc(int i);

// 0x414024  def score_4120cc.cpp
extern "C" void fn_414024(TScene *s, int i);

// 0x4141d0  def score_4120cc.cpp
extern "C" void fn_4141d0(TScene *s, int i);

// 0x414240  def game_414240.cpp
extern "C" void fn_414240(TScene *self, int idx);

// 0x41433c  def game_414240.cpp
extern "C" void fn_41433c(TScene *self, TGraphicSprite *s);

// 0x4143b8  def game_414240.cpp
extern "C" void fn_4143b8(TScene *self, int idx);

// 0x4143dc  def game_414240.cpp
extern "C" void fn_4143dc(TScene *self, int idx);

// 0x414418  def game_414240.cpp
extern "C" void fn_414418(TScene *self, int idx);

// 0x414510  def game_414240.cpp
extern "C" void fn_414510(TScene *self, TGraphicSprite *unused);

// 0x414570  def game_414240.cpp
extern "C" void fn_414570(TScene *self);

// 0x4145a4  def game_414240.cpp
extern "C" void fn_4145a4(TScene *self);

// 0x4146b8  def game_414240.cpp
extern "C" void fn_4146b8(TScene *self);

// 0x4146d8  def game_414240.cpp
extern "C" void fn_4146d8(TScene *self, int idx);

// 0x4147e8  def game_414240.cpp
extern "C" void fn_4147e8(TScene *self, int idx, bool fast);

// 0x41486c  def game_414240.cpp
extern "C" void fn_41486c(TScene *self, int idx);

// 0x414890  def game_414240.cpp
extern "C" void fn_414890(TScene *self);

// 0x41490c  def game_414240.cpp
extern "C" void fn_41490c(TScene *self, int frame);

// 0x414b30  def game_414240.cpp
extern "C" void fn_414b30(TScene *self);

// 0x414b68  def game_414240.cpp
extern "C" void fn_414b68(TScene *self, int idx);

// 0x414c28  def game_414240.cpp
extern "C" void fn_414c28(TScene *self, int idx);

// 0x414c90  def game_414240.cpp
extern "C" void fn_414c90(TScene *self);

// 0x414cb4  def game_414240.cpp
extern "C" void fn_414cb4(TScene *self, TGraphicSprite *s);

// 0x414d30  def game_414240.cpp
extern "C" void fn_414d30(TScene *self, TGraphicSprite *s);

// 0x414e50  def game_414240.cpp
extern "C" void fn_414e50(TScene *self, TGraphicSprite *s);

// 0x414e64  def game_414240.cpp
extern "C" void fn_414e64(TScene *self, int row);

// 0x414f70  def game_414240.cpp
extern "C" void fn_414f70(TScene *self);

// 0x414fb0  def game_414240.cpp
extern "C" void fn_414fb0(TScene *self);

// 0x41500c  def game_414240.cpp
extern "C" void fn_41500c(TScene *self);

// 0x415060  def game_414240.cpp
extern "C" void fn_415060(TScene *self);

// 0x415170  def game_414240.cpp
extern "C" void fn_415170(TScene *self);

// 0x41523c  def game_414240.cpp
extern "C" void fn_41523c(int idx, int v);

// 0x415294  def game_414240.cpp
extern "C" void fn_415294(int idx, int a, SpriteCb b);

// 0x4152f0  def game_414240.cpp
extern "C" void fn_4152f0(int idx);

// 0x415328  def game_414240.cpp
extern "C" void fn_415328(TScene *self, TGraphicSprite *s);

// 0x41534c  def game_414240.cpp
extern "C" void fn_41534c(TScene *self, TGraphicSprite *arg);

// 0x4153a0  def game_414240.cpp
extern "C" void fn_4153a0(TScene *self, TGraphicSprite *arg);

// 0x41545c  def game_414240.cpp
extern "C" void fn_41545c(TScene *self, TGraphicSprite *arg);

// 0x41553c  def game_414240.cpp
extern "C" void fn_41553c(TScene *self, TGraphicSprite *arg);

// 0x4155a0  def game_414240.cpp
extern "C" void fn_4155a0(TScene *self);

// 0x415600  def game_414240.cpp
extern "C" void fn_415600(TScene *self);

// 0x415780  def game_414240.cpp
extern "C" void fn_415780(TScene *self);

// 0x4157d8  def game_414240.cpp
extern "C" void fn_4157d8(TScene *self);

// 0x415848  def game_414240.cpp
extern "C" void fn_415848(TScene *self, TGraphicSprite *s);

// 0x4158d8  def game_414240.cpp
extern "C" void fn_4158d8(TScene *self, TGraphicSprite *arg);

// 0x415a40  def game_414240.cpp
extern "C" void fn_415a40(TScene *self, TGraphicSprite *arg);

// 0x415a68  def game_414240.cpp
extern "C" void fn_415a68(TScene *self, TGraphicSprite *arg);

// 0x415ac4  def game_414240.cpp
extern "C" void fn_415ac4(TScene *self);

// 0x415b80  def game_414240.cpp
extern "C" void fn_415b80(TScene *self, TGraphicSprite *arg);

// 0x415d7c  def game_414240.cpp
extern "C" void fn_415d7c(TScene *self);

// 0x415dc0  def game_414240.cpp
extern "C" void fn_415dc0(TScene *self, TGraphicSprite *arg);

// 0x415ddc  def game_414240.cpp
extern "C" void fn_415ddc(TScene *self, TGraphicSprite *s);

// 0x415eb0  def game_414240.cpp
extern "C" void fn_415eb0(TScene *self, TGraphicSprite *s);

// 0x415f80  def game_414240.cpp
extern "C" void fn_415f80(TScene *self);

// 0x415fc0  def game_414240.cpp
extern "C" void fn_415fc0(TScene *self, TGraphicSprite *s);

// 0x415fc8  def game_414240.cpp
extern "C" void fn_415fc8(TScene *self, TGraphicSprite *arg);

// 0x41608c  def game_414240.cpp
extern "C" void fn_41608c(TScene *self, TGraphicSprite *s);

// 0x4160c0  def game_414240.cpp
extern "C" void fn_4160c0(TScene *self, TGraphicSprite *s);

// 0x4160f4  def game_414240.cpp
extern "C" void fn_4160f4(TScene *self);

// 0x416288  def game_414240.cpp
extern "C" void fn_416288(TScene *self, TGraphicSprite *s);

// 0x4163d4  def game_414240.cpp
extern "C" void fn_4163d4(TScene *self, int mode);

// 0x4164d8  def game_414240.cpp
extern "C" void fn_4164d8(TScene *self);

// 0x416570  def game_414240.cpp
extern "C" void fn_416570(char *pins);

// 0x41665c  def game_41665c.cpp
//   alt: int fn_41665c()  [game_41665c]
//   alt: void fn_41665c(TScene*)  [game_416f10]
extern "C" int fn_41665c(TScene *self);

// 0x4169a0  def game_414240.cpp
//   alt: bool fn_4169a0()  [game_414240]
extern "C" bool fn_4169a0(TScene *s);

// 0x416a60  def game_414240.cpp
extern "C" void fn_416a60(TScene *self);

// 0x416ab0  def game_414240.cpp
extern "C" void fn_416ab0(TScene *self);

// 0x416b00  def game_414240.cpp
extern "C" void fn_416b00(TScene *self, TGraphicSprite *arg);

// 0x416bd8  def game_416bd8.cpp
extern "C" bool fn_416bd8(TScene *self, TPoint p);

// 0x416f10  def game_416f10.cpp
extern "C" void fn_416f10(TScene *self, TGraphicSprite *arg);

// 0x416f40  def game_416f10.cpp
extern "C" void fn_416f40(TScene *self, TGraphicSprite *arg);

// 0x416f5c  def game_416f10.cpp
extern "C" void fn_416f5c(TScene *self, TGraphicSprite *arg);

// 0x416fac  def game_416f10.cpp
extern "C" void fn_416fac(TScene *self, TGraphicSprite *arg);

// 0x417074  def game_417074.cpp
extern "C" void fn_417074(TScene *self, TGraphicSprite *s);

// 0x41753c  def game_416f10.cpp
extern "C" void fn_41753c(TScene *self);

// 0x41768c  def game_416f10.cpp
extern "C" void fn_41768c(TScene *self, TGraphicSprite *arg);

// 0x4176a0  def game_416f10.cpp
extern "C" void fn_4176a0(TScene *self, TGraphicSprite *arg);

// 0x4176bc  def game_416f10.cpp
extern "C" void fn_4176bc(TScene *self);

// 0x417734  def game_416f10.cpp
extern "C" void fn_417734(TScene *self, TGraphicSprite *arg);

// 0x417748  def game_416f10.cpp
extern "C" void fn_417748(TScene *self, TGraphicSprite *arg);

// 0x417770  def game_416f10.cpp
extern "C" void fn_417770(TScene *self, TGraphicSprite *s);

// 0x4177f0  def game_416f10.cpp
extern "C" void fn_4177f0(TScene *self);

// 0x4178b4  def game_416f10.cpp
extern "C" void fn_4178b4(TScene *self, TGraphicSprite *arg);

// 0x4178c4  def game_416f10.cpp
extern "C" void fn_4178c4(TScene *self);

// 0x417994  def game_416f10.cpp
extern "C" void fn_417994(TScene *self, TGraphicSprite *arg);

// 0x4179a8  def game_416f10.cpp
extern "C" void fn_4179a8(TScene *self, TGraphicSprite *arg);

// 0x4179bc  def game_416f10.cpp
extern "C" void fn_4179bc(TScene *self, TGraphicSprite *arg);

// 0x4179d8  def game_416f10.cpp
extern "C" void fn_4179d8(TScene *self, TGraphicSprite *arg);

// 0x417a00  def game_416f10.cpp
extern "C" void fn_417a00(TScene *self, TGraphicSprite *s);

// 0x417c74  def game_416f10.cpp
extern "C" void fn_417c74(TScene *self, TGraphicSprite *s);

// 0x417d10  def game_416f10.cpp
extern "C" void fn_417d10(TScene *self, TGraphicSprite *arg);

// 0x417d38  def game_416f10.cpp
extern "C" void fn_417d38(TScene *self, TGraphicSprite *arg);

// 0x417d98  def game_416f10.cpp
extern "C" void fn_417d98(TScene *self, TGraphicSprite *s);

// 0x417ed4  def game_416f10.cpp
extern "C" void fn_417ed4(TScene *self);

// 0x417f34  def game_416f10.cpp
extern "C" void fn_417f34(TScene *self);

// 0x417f74  def game_416f10.cpp
extern "C" void fn_417f74(TScene *self);

// 0x417fc4  def game_416f10.cpp
extern "C" void fn_417fc4(TScene *self);

// 0x418008  def game_416f10.cpp
extern "C" void fn_418008(TScene *self);

// 0x41803c  def game_416f10.cpp
extern "C" void fn_41803c(TScene *self);

// 0x418044  def game_416f10.cpp
extern "C" void fn_418044(TScene *self);

// 0x418070  def game_416f10.cpp
extern "C" void fn_418070(TScene *self, TGraphicSprite *unused);

// 0x418080  def game_416f10.cpp
extern "C" void fn_418080(TScene *self, TGraphicSprite *arg);

// 0x4180cc  def game_416f10.cpp
extern "C" void fn_4180cc(TScene *self, int delay);

// 0x418104  def game_416f10.cpp
extern "C" void fn_418104(TScene *self);

// 0x418134  def game_416f10.cpp
extern "C" void fn_418134(TScene *self, TGraphicSprite *arg);

// 0x418180  def game_416f10.cpp
extern "C" void fn_418180(TScene *self);

// 0x4181a8  def game_416f10.cpp
//   alt: void fn_4181a8(void*)  [game_41c674]
extern "C" void fn_4181a8(TScene *self);

// 0x418260  def game_416f10.cpp
extern "C" void fn_418260(TScene *self);

// 0x418274  def game_418274.cpp
//   alt: void fn_418274(void*)  [game_41c674]
extern "C" void fn_418274(TScene *self, char loaded);

// 0x41c674  def game_41c674.cpp
//   alt: void fn_41c674(TStage*)  [init_40f350]
extern "C" void fn_41c674(TStage *self);

// 0x41c714  def game_41c674.cpp
extern "C" void fn_41c714(TScene *self, TGraphicSprite *arg);

// 0x41c730  def game_41c674.cpp
extern "C" void fn_41c730(TScene *self);

// 0x41c906  def game_41c674.cpp
extern "C" void fn_41c906(TScene *self);

// 0x41c940  def game_41c674.cpp
extern "C" void fn_41c940(TScene *self, void *sender, unsigned short &key, ShiftState shift);

// 0x41c98c  def game_41c674.cpp
extern "C" void fn_41c98c(TScene *self, TGraphicSprite *unused);

// 0x41c9a0  def game_41c674.cpp
extern "C" void fn_41c9a0(TScene *self, TGraphicSprite *unused);

// 0x41c9bc  def game_41c674.cpp
extern "C" void fn_41c9bc(TScene *self, TGraphicSprite *unused);

// 0x41c9cc  def game_41c674.cpp
extern "C" void fn_41c9cc(TScene *self, TGraphicSprite *unused);

// 0x41c9e8  def game_41c674.cpp
extern "C" void fn_41c9e8(TScene *self, TGraphicSprite *arg);

// 0x41ca30  def game_41c674.cpp
extern "C" void fn_41ca30(TScene *self, TGraphicSprite *s);

// 0x41ca40  def game_41c674.cpp
extern "C" void fn_41ca40(TScene *self, TGraphicSprite *s);

// 0x41ca84  def game_41c674.cpp
extern "C" void fn_41ca84(TScene *self, TGraphicSprite *s);

// 0x41cb9c  def game_41c674.cpp
extern "C" void fn_41cb9c(TScene *self);

// 0x41cbf8  def game_41c674.cpp
extern "C" void fn_41cbf8(TScene *self, int start, int unused);

// 0x41ccb4  def game_41c674.cpp
extern "C" void fn_41ccb4(TScene *self, int start, int unused);

// 0x41cd70  def game_41c674.cpp
extern "C" void fn_41cd70(TScene *self);

// 0x41cf48  def game_41c674.cpp
extern "C" void fn_41cf48(TScene *self);

// 0x41cf58  def game_41c674.cpp
extern "C" void fn_41cf58(TScene *self, TGraphicSprite *s);

// 0x41cfb0  def game_41c674.cpp
extern "C" void fn_41cfb0(TScene *self);

// 0x41d004  def game_41c674.cpp
extern "C" void fn_41d004(TScene *self, TGraphicSprite *arg);

// 0x41d018  def game_41c674.cpp
extern "C" void fn_41d018(TScene *self, TGraphicSprite *arg);

// 0x41d040  def game_41c674.cpp
extern "C" void fn_41d040(TScene *self, TGraphicSprite *arg);

// 0x41d06c  def game_41c674.cpp
extern "C" void fn_41d06c(TScene *self, TGraphicSprite *arg);

// 0x41d0c0  def game_41c674.cpp
//   alt: void fn_41d0c0(void*)  [opening_41e01c]
extern "C" void fn_41d0c0(TScene *self);

// 0x41d184  def opening_41d184.cpp
//   alt: void fn_41d184(void*)  [opening_41e01c]
extern "C" void fn_41d184(TScene *self, char loaded);

// 0x41e01c  def opening_41e01c.cpp
//   alt: void fn_41e01c(TStage*)  [init_40f350]
extern "C" void fn_41e01c(TStage *self);

// 0x41e0bc  def opening_41e01c.cpp
extern "C" void fn_41e0bc(TScene *self, void *sender, unsigned short &key, ShiftState shift);

// 0x41e108  def opening_41e01c.cpp
extern "C" void fn_41e108(TScene *self, TGraphicSprite *unused);

// 0x41e124  def opening_41e01c.cpp
extern "C" void fn_41e124(TScene *self, TGraphicSprite *s);

// 0x41e170  def opening_41e01c.cpp
//   alt: void fn_41e170(void*)  [opening_41e01c]
extern "C" void fn_41e170(TScene *self);

// 0x41e1a4  def opening_41e01c.cpp
extern "C" void fn_41e1a4(TScene *self, char loaded);

// 0x41e22c  def opening_41e01c.cpp
//   alt: void fn_41e22c(TStage*)  [init_40f350]
extern "C" void fn_41e22c(TStage *self);

// 0x41e2cc  def zlib_41e2cc.cpp
extern "C" unsigned long fn_41e2cc(unsigned long adler, const unsigned char *buf, unsigned len);

// 0x420984
extern "C" int fn_420984(ZStream *z, const char *ver, int size);

// 0x42099c
extern "C" int fn_42099c(ZStream *z, int flush);
#define zlib_inflate fn_42099c

// 0x421adc  def zlib_41e2cc.cpp
extern "C" void fn_421adc(void *p);

// ==== library / VCL / RTL entry points called by address ====

// 0x40e19c  NOT library code (libmap's @std@%basic_ifstream...@is_open$qv is a
//   false match): inflateEnd (fn_4207e6) on the stream's z_stream,
//   `return inflateEnd(&((TPackedStreamData *)s)->z)` (push s+0x10). Caller:
//   fn_40d568 (packres_40d3f8) with the TPackedStream. Not matched (outside
//   the game ranges); native definition in zlib_41e2cc.cpp (#ifndef __BORLANDC__).
extern "C" int fn_40e19c(void *s);

// 0x40f684  def threads_40f684.cpp; lib: fn_40f684
extern "C" void __fastcall fn_40f684(TMyThread *self);

// 0x411af4  def exit_411af4.cpp; lib: fn_411af4
//   alt: void fn_411af4(TStage*)  [init_40f350]
extern "C" void fn_411af4(TStage *g);

// 0x42fdcc
extern "C" int __fastcall fn_42fdcc(void *screen);
#define TScreen_GetHeight fn_42fdcc

// 0x42fdd4
extern "C" int __fastcall fn_42fdd4(void *screen);
#define TScreen_GetWidth fn_42fdd4

// 0x434158
extern "C" void __fastcall fn_434158(TStageForm *c, int v);
#define TControl_SetLeft fn_434158

// 0x434178
extern "C" void __fastcall fn_434178(TStageForm *c, int v);
#define TControl_SetTop fn_434178

// 0x434198
extern "C" void __fastcall fn_434198(TStageForm *c, int v);
#define TControl_SetWidth fn_434198

// 0x4341b8
extern "C" void __fastcall fn_4341b8(TStageForm *c, int v);
#define TControl_SetHeight fn_4341b8

// 0x434238
extern "C" int __fastcall fn_434238(void *ctl);
#define TControl_GetClientWidth fn_434238

// 0x43427c
extern "C" int __fastcall fn_43427c(void *ctl);
#define TControl_GetClientHeight fn_43427c

// 0x4348e8
extern "C" void __fastcall fn_4348e8(TStageForm *c);
#define TControl_BringToFront fn_4348e8

// 0x434b6c
extern "C" void __fastcall fn_434b6c(void *ctl);
#define TControl_Hide fn_434b6c

// 0x434b74
extern "C" void __fastcall fn_434b74(void *c);
#define TControl_Show fn_434b74

// 0x438384
extern "C" HWND __fastcall fn_438384(void *ctl);
#define TWinControl_GetHandle fn_438384

// 0x43a8c4
extern "C" TPoint __fastcall fn_43a8c4(int x, int y);
#define Classes_Point_ fn_43a8c4

// 0x43a8dc
extern "C" ERect __fastcall fn_43a8dc(int l, int t, int r, int b);
#define Classes_Rect_ fn_43a8dc

// 0x448a2c
extern "C" void fn_448a2c(void *p);
#define rtl_operator_delete fn_448a2c

// ==== VCL methods under plain-function spellings (C++ linkage) ====
// The Borland linker resolves these to the VCL methods named in the
// comments (the call targets are fixups). The native port defines the same
// spellings (port/include/vcl/elfcompat.h, port/vcl/entry.cpp).
// TThread_Synchronize needs Classes::TThreadMethod: elf/thread.h.
extern void __fastcall TCustomForm_Close(void *form);           // 0x42f47c Forms::TCustomForm::Close (init_40f350)
extern void __fastcall TApplication_Terminate(void *app);       // 0x431730 Forms::TApplication::Terminate (init_40f350)
extern int __fastcall TApplication_MessageBox(void *app, const char *text, const char *caption, int flags);
                                                                // Forms::TApplication::MessageBox (range_401cec)
extern void __fastcall TCustomForm_SetClientWidth(void *form, int v);   // Forms::TCustomForm::SetClientWidth (webtrack)
extern void __fastcall TCustomForm_SetClientHeight(void *form, int v);  // Forms::TCustomForm::SetClientHeight (webtrack)
extern void __fastcall TCustomWinSocket_SendBuf(void *sock, void *buf, int count);  // Scktcomp (webtrack)
extern int __fastcall TCustomWinSocket_ReceiveLength(void *sock);
extern int __fastcall TCustomWinSocket_ReceiveBuf(void *sock, void *buf, int count);

// ==== RTL entry points declared by alias ====
// Units call these through extern "C" rtl_* names so that bcc32 emits a plain
// call (no <string.h> inline/intrinsic form, no C++ overloads). The linker
// maps them to the RTL; the native port maps them to libc.
extern "C" {
void *rtl_malloc(unsigned n);
void rtl_free(void *p);
void rtl_delete(void *p);
void *rtl_memcpy(void *d, const void *s, unsigned n);
void *rtl_memmove(void *d, const void *s, unsigned n);
void *rtl_memset(void *d, int c, unsigned n);
unsigned rtl_strlen(const char *s);
char *rtl_strcpy(char *d, const char *s);
char *rtl_strcat(char *d, const char *s);
char *rtl_strncpy(char *d, const char *s, unsigned n);
char *rtl_strrchr(const char *s, int c);
int rtl_strcmp(const char *a, const char *b);
int rtl_stricmp(const char *a, const char *b);
long rtl_time(long *t);
void rtl_srand(unsigned seed);
void rtl_exit(int code);
}
// 0x448a94 is operator new[] (libmap mislabels it @$bdele$qpv).
extern "C" void *fn_448a94(unsigned n);
#define rtl_operator_new_array fn_448a94

#endif
