// Engine coordinate helpers, 0x40a5d4-0x40a6e4.
#include <elf/funcs.h>

extern "C" RectPod fn_40a5d4(TStage *e, RectPod &r)
{
    RectPod t;
    t.l = r.l + e->offset.x;
    t.t = r.t + e->offset.y;
    t.r = r.r + e->offset.x;
    t.b = r.b + e->offset.y;
    return t;
}

extern "C" RectPod fn_40a63c(TStage *e, RectPod &r)
{
    RectPod t;
    t.l = r.l - e->offset.x;
    t.t = r.t - e->offset.y;
    t.r = r.r - e->offset.x;
    t.b = r.b - e->offset.y;
    return t;
}

extern "C" TPoint fn_40a6a4(TStage *e, TPoint &p)
{
    TPoint t;
    t.x = p.x - e->offset.x;
    t.y = p.y - e->offset.y;
    return t;
}
