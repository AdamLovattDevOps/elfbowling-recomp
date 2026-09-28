// TStage: blit a cast to the screen DC or into the front buffer, 0x40b3b0.
#include <elf/funcs.h>

extern "C" char fn_40b3b0(TStage *e, char direct, RectPod *dst, TGraphicCast *c, TPoint *src)
{
    char ok = 1;
    int w = dst->r - dst->l;
    int h = dst->b - dst->t;
    if (!direct) {
        dst->l -= g_460200;
        dst->r -= g_460200;
        dst->t -= g_460204;
        dst->b -= g_460204;
        if (!c->masked) {
            fn_402ed4(&e->front, (TPoint *)dst, &c->bmp, (RECT *)src, SRCCOPY);
        } else {
            fn_402ed4(&e->front, (TPoint *)dst, &c->mask, (RECT *)src, SRCAND);
            fn_402ed4(&e->front, (TPoint *)dst, &c->bmp, (RECT *)src, SRCPAINT);
        }
        e->dirtyFlag2 = 1;
    } else {
        fn_40b234(e);
        fn_40b290(e);
        if (!c->masked) {
            SelectObject(g_4601f8, c->bmp.handle);
            ok = BitBlt(g_4601f4, dst->l, dst->t, w, h, g_4601f8, src->x, src->y, SRCCOPY) != 0;
        } else {
            SelectObject(g_4601f8, c->mask.handle);
            ok = BitBlt(g_4601f4, dst->l, dst->t, w, h, g_4601f8, src->x, src->y, SRCAND) != 0;
            SelectObject(g_4601f8, c->bmp.handle);
            ok &= BitBlt(g_4601f4, dst->l, dst->t, w, h, g_4601f8, src->x, src->y, SRCPAINT) != 0;
        }
        fn_40b2c8(e);
        fn_40b264(e);
    }
    return ok;
}
