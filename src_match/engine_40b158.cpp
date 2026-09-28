// TStage drawing helpers, 0x40b158-0x40b3b0.
#include <elf/funcs.h>

extern "C" void fn_40b158(TStage *e)
{
    TScene *sc = e->scene;
    if (sc) {
        for (int i = 0; i < sc->nsprites; i++) {
            TGraphicSprite *s = sc->sprites[i];
            if ((s->type == 1 || s->type == 3 || s->type == 2) && s->shown && s->f1d0)
                fn_4080a0(s);
        }
    }
}

extern "C" void fn_40b1d4(TStage *e)
{
    fn_40ae80(e, ELF_AS(ERect, e->screen));
    fn_40b158(e);
}

extern "C" void fn_40b210(TStage *e, void *a, void *b, void *c, DWORD rop)
{
    fn_402ed4(&e->back, (TPoint *)a, (Bitmap *)b, (RECT *)c, rop);
}

extern "C" void fn_40b234(TStage *e)
{
    g_4601f4 = GetDC(fn_438384(e->form));
    g_4601f8 = CreateCompatibleDC(g_4601f4);
}

extern "C" void fn_40b264(TStage *e)
{
    DeleteDC(g_4601f8);
    ReleaseDC(fn_438384(e->form), g_4601f4);
}

extern "C" void fn_40b290(TStage *e)
{
    if (e->bpp == 8) {
        g_4601fc = SelectPalette(g_4601f4, e->palette, 0);
        RealizePalette(g_4601f4);
    }
}

extern "C" void fn_40b2c8(TStage *e)
{
    if (e->bpp == 8)
        SelectPalette(g_4601f4, g_4601fc, 1);
}

// `f` is passed by engine_40b5bc and unused here.
extern "C" void fn_40b2ec(TStage *e, ERect *a, TGraphicCast *img, ERect *c, char f)
{
    if (!img->masked) {
        fn_40b210(e, a, &img->bmp, c, SRCCOPY);
    } else if (img->keepBg == 0) {
        fn_40b210(e, a, &img->mask, c, SRCAND);
        fn_40b210(e, a, &img->bmp, c, SRCPAINT);
    } else if (img->keepBg == 1) {
        fn_40b210(e, a, &img->mask, c, SRCAND);
        fn_40b210(e, a, &img->bmp, c, SRCPAINT);
    }
    e->dirtyFlag = 1;
}
