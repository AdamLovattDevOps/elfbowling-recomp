// TGraphicSprite mouse handling (click, right-click, drag with fling
// velocity), 0x408940.
//
// The rects are ERect (copy ctor "*this = o"), which is what makes by-value
// passing use rep movsd onto the stack instead of 4 pushes.
#include <stdlib.h>
#include <elf/funcs.h>

// fn_4028b8 taking the clip as ERect (funcs.h has RECT; the by-value copy
// differs, the call target is a fixup).
extern "C" TPoint fn_4028b8_E(ERect r, TPoint p);

extern "C" void fn_408940(TGraphicSprite *s, TStage *e, int now, char *clickL, char *clickR, char *unused)
{
    TGraphicCast *c = s->casts[s->frame];
    ERect r = ELF_AS(ERect, c->bounds);
    if (fn_407040(s, &r)) {
        if (*clickL && fn_4027d0(r, e->down)) {
            if (s->draggable && s->grab.x == 999999999) {
                s->grab = fn_43a8c4(s->x / 1000, s->y / 1000);
                s->fling = fn_43a8c4(0, 0);
                s->dragTime = fn_4024bc(0);
                s->outX = s->outY = 0;
                e->SetDrag(s);
                if (s->onGrab)
                    s->onGrab(e->scene, s);
            }
            if (!s->draggable)
                s->grab = e->down;
            e->down.x = -1;
            e->down.y = -1;
            *clickL = 0;
            if (s->enabled && s->onClick)
                s->onClick(e->scene, s);
        }
        if (*clickR && fn_4027d0(r, e->up)) {
            s->rclick = e->up;
            e->up.x = -1;
            e->up.y = -1;
            *clickR = 0;
            if (s->enabled && s->onRClick)
                s->onRClick(e->scene, s);
        }
    }
    if (e->drag == s && s->draggable) {
        TPoint p;
        if (fn_402800(ELF_AS(ERect, s->clip), fn_43a8c4(s->x / 1000, s->y / 1000)))
            s->outX = 0;
        if (fn_402820(ELF_AS(ERect, s->clip), fn_43a8c4(s->x / 1000, s->y / 1000)))
            s->outY = 0;
        p.x = s->grab.x + e->dragDelta.x;
        p.y = s->grab.y + e->dragDelta.y;
        p = fn_4028b8_E(ELF_AS(ERect, s->clip), p);
        if (s->x / 1000 != p.x || s->y / 1000 != p.y) {
            int t = fn_4024bc(0);
            int dt = tmax(t - s->dragTime, 1);
            int period = e->scene->period;
            s->fling.x = (p.x - s->x / 1000) * 1000 / dt / period;
            s->fling.y = (p.y - s->y / 1000) * 1000 / dt / period;
            s->dragTime = t;
            fn_406a80(s, p);
            if (s->onDrag)
                s->onDrag(e->scene, s);
            if (!s->outX) {
                if (!fn_402800(ELF_AS(ERect, s->clip), fn_43a8c4(s->x / 1000, s->y / 1000)) && s->onLeave) {
                    s->outX = 1;
                    s->onLeave(e->scene, s);
                }
            }
            if (!s->outY) {
                if (!fn_402820(ELF_AS(ERect, s->clip), fn_43a8c4(s->x / 1000, s->y / 1000)) && s->onLeave) {
                    s->outY = 1;
                    s->onLeave(e->scene, s);
                }
            }
        }
    } else if (s->grab.x != 999999999) {
        s->grab = fn_43a8c4(999999999, 999999999);
        if (s->onDrop)
            s->onDrop(e->scene, s);
    }
}
