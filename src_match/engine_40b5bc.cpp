// TStage clear/blit/render helpers, 0x40b5bc-0x40b8fc.
#include <elf/funcs.h>

extern "C" char fn_40b5bc(TStage *e, char direct, ERect *r)
{
    char ok = 1;
    int w, h;
    if (!direct) {
        w = r->r - r->l;
        h = r->b - r->t;
        g_460200 = r->l;
        g_460204 = r->t;
        ERect t = fn_43a8dc(0, 0, w, h);
        fn_40343c(&e->front, (RECT *)&t, e->bgcolor);
        e->dirtyFlag2 = 1;
    } else {
        fn_40b234(e);
        fn_40b290(e);
        int stock = e->bgcolor == 0 ? BLACK_BRUSH : WHITE_BRUSH;
        HGDIOBJ br = GetStockObject(stock);
        SelectObject(g_4601f4, br);
        int res = FillRect(g_4601f4, (RECT *)r, (HBRUSH)br);
        ok = res != 0;
        fn_40b2c8(e);
        fn_40b264(e);
    }
    return ok;
}

extern "C" void fn_40b6b4(TStage *e, ERect *r)
{
    if (e->bgimage == 0)
        fn_40343c(&e->back, (RECT *)r, e->bgcolor);
    else
        fn_403048(&e->back, (RECT *)r, &e->bgimage->bmp);
    e->dirtyFlag = 1;
}

extern "C" void fn_40b708(TStage *e, ERect area)
{
    fn_40b6b4(e, &area);
    TScene *sc = e->scene;
    for (int i = 0; i < sc->nsprites; i++) {
        TGraphicSprite *s = sc->sprites[i];
        if ((s->type == 1 || s->type == 3 || s->type == 2) && s->shown) {
            ERect *p = (ERect *)&s->casts[s->frame]->bounds;
            ERect r;
            r = *p;
            if (fn_40717c(s, &r) && fn_402654(&r, area)) {
                ERect r2;
                r2 = r;
                if (fn_407690(s, &r2))
                    fn_40b2ec(e, &r, s->casts[s->frame], &r2, s->f1dc);
            }
            if (s->animating && s->talkCasts[s->frame] != 0 && s->phase == 1) {
                ERect *p2 = (ERect *)&s->talkCasts[s->frame]->bounds;
                ERect r3;
                r3 = *p2;
                if (fn_4071a8(s, &r3) && fn_402654(&r3, area)) {
                    ERect r4;
                    r4 = r3;
                    if (fn_4076bc(s, &r4))
                        fn_40b2ec(e, &r3, s->talkCasts[s->frame], &r4, 0);
                }
            }
        }
    }
}
