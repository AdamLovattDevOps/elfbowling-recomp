// Constructors: TSprite base 0x40609c, TGraphicSprite 0x4064d0/0x406520,
// TCastMgr 0x40604c, TGraphicCast 0x4052dc/0x405380/0x405428, TTextCast
// 0x4056c0. Real C++ ctors (EH frame via __InitExceptBlockLDTC).
#include <elf/funcs.h>

TCastMgr::TCastMgr(TStage *stage)
{
    this->stage = stage;
    count = 0;
    if (g_455524)
        fn_40dc1c(g_455524, stage);
}

// The `Bitmap *pb = &bmp` locals produce the two extra pointer stores.
TGraphicCast::TGraphicCast(TStage *stage, const char *name, char masked)
{
    Bitmap *pb = &bmp;
    Bitmap *pm = &mask;
    fn_4050b4(this, stage, name, masked, Classes_Point(0, 0), 0, 0);
    TPoint ctr = Classes_Point((r20.right - r20.left) / 2, (r20.bottom - r20.top) / 2);
    fn_40500c(this, ctr);
}

TGraphicCast::TGraphicCast(TStage *stage, const char *name, char masked, char flipX, char flipY)
{
    Bitmap *pb = &bmp;
    Bitmap *pm = &mask;
    fn_4050b4(this, stage, name, masked, Classes_Point(0, 0), flipX, flipY);
    TPoint ctr = Classes_Point((r20.right - r20.left) / 2, (r20.bottom - r20.top) / 2);
    fn_40500c(this, ctr);
}

// Scaled copy of src, named "#<src name>"
TGraphicCast::TGraphicCast(TStage *stage, TGraphicCast *src, int div)
{
    Bitmap *pb = &bmp;
    Bitmap *pm = &mask;
    fn_4050b4(this, stage, 0, 3, Classes_Point(0, 0), 0, 0);
    this->name[0] = '#';
    rtl_strcpy(this->name + 1, src->name);
    fn_402bb8(&bmp, &src->bmp, div);
    hot.x = src->hot.x / div;
    hot.y = src->hot.y / div;
    fn_4024e8(&r20, &src->r20, div);
    fn_4024e8(&bounds, &src->bounds, div);
    fn_404f94(this, src->masked);
}

// The name is set by fn_40552c after the unnamed base init.
TTextCast::TTextCast(TStage *stage, const char *name, int w, int h, char f) : TGraphicCast(stage, 0, f)
{
    fn_40552c(this, w, h, name);
    keepBg = 1;
}

TSprite::TSprite()
{
    type = 0;
    shown = 1;
    index = 0;
}

TGraphicSprite::TGraphicSprite(const char *name, TStage *stage, TPoint pos)
{
    fn_406378(this, stage, name, pos);
}

TGraphicSprite::TGraphicSprite(const char *name, TStage *stage)
{
    fn_406378(this, stage, name, Classes_Point(0, 0));
}
// MATCH 40604c @TCastMgr@$bctr$qp6TStage
// MATCH 4052dc @TGraphicCast@$bctr$qp6TStagepxcc
// MATCH 405380 @TGraphicCast@$bctr$qp6TStagepxcccc
// MATCH 405428 @TGraphicCast@$bctr$qp6TStagep12TGraphicCasti
// MATCH 4056c0 @TTextCast@$bctr$qp6TStagepxciic
// MATCH 40609c @TSprite@$bctr$qv
// MATCH 406520 @TGraphicSprite@$bctr$qpxcp6TStage8tagPOINT
// MATCH 4064d0 @TGraphicSprite@$bctr$qpxcp6TStage
