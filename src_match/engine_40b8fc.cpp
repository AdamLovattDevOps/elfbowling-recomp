// TStage blit-to-screen and tick, 0x40b8fc-0x40bd3c.
#include <elf/funcs.h>

// Alias: this unit takes the ERect (copy-ctor) return where funcs.h has
// RectPod. See docs/HEADERS.md.
extern "C" ERect fn_40a5d4_E(TStage *e, ERect &r);

extern "C" char fn_40b8fc(ERect &dst, HBITMAP bmp, TPoint &src)
{
    int w, h;
    char ok;
    ERect r;
    r = fn_40a5d4_E(g_4601c8, dst);
    w = r.r - r.l;
    h = r.b - r.t;
    fn_40b234(g_4601c8);
    fn_40b290(g_4601c8);
    SelectObject(g_4601f8, bmp);
    ok = BitBlt(g_4601f4, r.l, r.t, w, h, g_4601f8, src.x, src.y, SRCCOPY) != 0;
    fn_40b2c8(g_4601c8);
    fn_40b264(g_4601c8);
    return ok;
}

extern "C" char fn_40bcb8(TStage *e)
{
    fn_40d148(e->sound);
    if (e->next)
        fn_40be34(e);
    if (e->webtrack)
        fn_40e7a4(e->webtrack);
    if (!e->paused && e->nscenes > 0 && e->scene) {
        fn_409dac(e->scene);
        fn_40b9b8(e);
        fn_40ba68(e);
    }
    return e->running;
}
