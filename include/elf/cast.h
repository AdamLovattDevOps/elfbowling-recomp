// elf/cast.h - graphic casts (TGraphicCast, TTextCast) and the cast list
// (TCastMgr). RTTI names: TGraphicCast, TTextCast, TCastMgr.
//
// Old local names: Cast, GCast, Image, Bmp (packres: a Cast seen at +0x40),
// TextBox, Img / Frame (a TTextCast seen at +0x250), Obj804 / List200
// (TCastMgr).
#ifndef ELF_CAST_H
#define ELF_CAST_H

#include <elf/types.h>
#include <elf/bitmap.h>

struct TStage;

// A bitmap with an optional mask, a name and a hotspot (0xb8 bytes).
// Init: fn_4050b4(cast, stage, name, masked, hot, flipX, flipY).
struct TGraphicCast {
    TStage *stage;              // 0x00 (old: eng)
    unsigned char keepBg;       // 0x04 1 = keep background pixels when cropping (old: f4)
                                //      engine_40b158 calls it the mask mode
    char used;                  // 0x05
    char name[0x19];            // 0x06
    unsigned char masked;       // 0x1f 0 none, 1 or 2 mask mode. MUST stay unsigned:
                                //      `masked == 1` is cmp byte (unsigned) vs movsx/dec (char).
                                //      engine_40b158/40b3b0 declared it char: cast there.
    RECT r20;                   // 0x20 right/bottom are the size
    RECT bounds;                // 0x30 (sprites_4092bc/engine_40b5bc read it as ERect: ELF_AS)
    Bitmap bmp;                 // 0x40 image
    Bitmap mask;                // 0x78 mask
    TPoint hot;                 // 0xb0 hotspot

    // Real ctor parameter lists (the stage comes first; ctors_4052dc.cpp's
    // old (name, int, char) spelling is the same ABI).
    TGraphicCast(TStage *stage, const char *name, char masked);                          // 0x4052dc
    TGraphicCast(TStage *stage, const char *name, char masked, char flipX, char flipY);  // 0x405380
    TGraphicCast(TStage *stage, TGraphicCast *src, int div);                             // 0x405428 "#src" scaled copy
};

ELF_CHECK_OFS(TGraphicCast, keepBg, 0x04);
ELF_CHECK_OFS(TGraphicCast, name, 0x06);
ELF_CHECK_OFS(TGraphicCast, masked, 0x1f);
ELF_CHECK_OFS(TGraphicCast, bounds, 0x30);
ELF_CHECK_OFS(TGraphicCast, bmp, 0x40);
ELF_CHECK_OFS(TGraphicCast, mask, 0x78);
ELF_CHECK_OFS(TGraphicCast, hot, 0xb0);
ELF_CHECK_SIZE(TGraphicCast, 0xb8);

// A cast that text is drawn into (0x284 bytes). ctor 0x4056c0.
// Setters: fn_405978 style, fn_40598c colour, fn_405954 clear.
struct TTextCast : TGraphicCast {
    int fb8;                    // 0xb8
    char _unkbc[0x24c - 0xbc];  // 0xbc
    int f24c;                   // 0x24c
    int lineHeight;             // 0x250 (old: f250)
    int f254;                   // 0x254
    char f258;                  // 0x258
    char f259;                  // 0x259
    char _pad25a[2];
    TPoint f25c;                // 0x25c used as a POINT (old: int f25c, f260)
    char *bufStart;             // 0x264 (old: f264)
    char *bufPtr;               // 0x268 write pointer (old: f268)
    int f26c;                   // 0x26c
    int f270;                   // 0x270
    int lineSpacing;            // 0x274 (old: f274)
    char align;                 // 0x278 DrawText alignment (old: f278)
    char _pad279[3];
    TPoint f27c;                // 0x27c used as a POINT (old: int f27c, f280)

    TTextCast(TStage *stage, const char *name, int w, int h, char f);  // 0x4056c0
};

ELF_CHECK_OFS(TTextCast, fb8, 0xb8);
ELF_CHECK_OFS(TTextCast, lineHeight, 0x250);
ELF_CHECK_OFS(TTextCast, f25c, 0x25c);
ELF_CHECK_OFS(TTextCast, lineSpacing, 0x274);
ELF_CHECK_OFS(TTextCast, f27c, 0x27c);
ELF_CHECK_SIZE(TTextCast, 0x284);

// The stage's cast list (0x808 bytes), ctor 0x40604c; searched by name with
// stricmp. fn_405a1c loads every graphic cast.
struct TCastMgr {
    int count;                      // 0x000
    TGraphicCast *items[0x200];     // 0x004 (old: List200 int items[]; fn_4059a0 now takes the cast)
    TStage *stage;                  // 0x804 (old: f804)

    TCastMgr(TStage *stage);        // 0x40604c (old: Obj804(void *))
};

ELF_CHECK_OFS(TCastMgr, items, 0x004);
ELF_CHECK_OFS(TCastMgr, stage, 0x804);
ELF_CHECK_SIZE(TCastMgr, 0x808);

#endif
