// TGraphicSprite methods, 0x407ffc-0x408940: sequence queue, dirty rects,
// movement (fn_4081b8), timers and show/hide.
//
// The rects are ERect (copy ctor "*this = o"), which is what makes by-value
// passing use rep movsd onto the stack instead of 4 pushes.
#include <stdlib.h>
#include <elf/funcs.h>

// fn_402840 taking the clip as ERect (funcs.h has RECT; the by-value copy
// differs, the call target is a fixup).
extern "C" TPoint fn_402840_E(ERect r, TPoint p);

extern "C" char fn_407ffc(TGraphicSprite *s, int idx, SpriteCb v)
{
    char r = 0;
    if (idx < s->nseq && s->queueLen < 12) {
        s->seqs[idx]->done = v;
        s->queue[s->queueLen++] = idx;
        if (s->queueLen == 1) {
            s->timer = -1;
            fn_407f48(s);
            r = 1;
        }
    }
    return r;
}

extern "C" void fn_40806c(TGraphicSprite *s)
{
    s->queueLen = 0;
    if (s->timer >= 0) {
        fn_406c50(s, s->timer);
        s->timer = -1;
    }
}

extern "C" char fn_4080a0(TGraphicSprite *s)
{
    ERect *p = &ELF_AS(ERect, s->casts[s->frame]->bounds);
    char a;
    char b;
    ERect r;
    r = *p;
    ERect cur;
    cur = r;
    a = fn_40717c(s, &cur);
    if (a)
        fn_40ae80(s->stage, cur);
    if (s->f1d0) {
        cur = r;
        b = fn_4071d4(s, &cur);
        if (b)
            fn_40af80(s->stage, cur);
        cur = r;
        b = fn_407384(s, &cur);
        if (b)
            fn_40af80(s->stage, cur);
    }
    return a;
}

extern "C" char fn_4081b8(TGraphicSprite *s)
{
    fn_4080a0(s);
    if (fn_402800(ELF_AS(ERect, s->clip), fn_43a8c4(s->x / 1000, s->y / 1000)))
        s->outX = 0;
    if (fn_402820(ELF_AS(ERect, s->clip), fn_43a8c4(s->x / 1000, s->y / 1000)))
        s->outY = 0;
    TPoint v;
    int d;
    if (s->vx > 0)
        d = tmax(s->vx - s->dvx, 0);
    else
        d = tmin(s->vx + s->dvx, 0);
    s->dvx += s->ax;
    v.x = d;
    v.y = s->vy + s->dvy;
    s->dvy += s->ay;
    if (s->vy + s->dvy > s->maxvy)
        s->dvy = s->maxvy - s->vy;
    TPoint old = s->fpos;
    s->x += v.x;
    s->y += v.y;
    s->fpos = fn_402840_E(ELF_AS(ERect, s->clip), s->fpos);
    if ((old.x / 1000 != s->x / 1000 || old.y / 1000 != s->y / 1000) && s->onCell)
        s->onCell(s->stage->scene, s);
    ERect r = ELF_AS(ERect, s->casts[s->frame]->bounds);
    fn_407040(s, &r);
    if (s->bounceX) {
        if (r.r >= s->clip.right)
            s->vx = abs(s->vx) * -1;   // "-abs()" negates in place; "* -1" gives mov ecx,eax; neg ecx
        else if (r.l <= s->clip.left)
            s->vx = abs(s->vx);
    }
    if (s->bounceY) {
        if (r.b >= s->clip.bottom)
            s->vy = abs(s->vy) * -1;
        else if (r.t <= s->clip.top)
            s->vy = abs(s->vy);
    }
    if (!s->outX) {
        if (!fn_402800(ELF_AS(ERect, s->clip), fn_43a8c4(s->x / 1000, s->y / 1000)) && s->onLeave) {
            s->outX = 1;
            s->onLeave(s->stage->scene, s);
        }
    }
    if (!s->outY) {
        if (!fn_402820(ELF_AS(ERect, s->clip), fn_43a8c4(s->x / 1000, s->y / 1000)) && s->onLeave) {
            s->outY = 1;
            s->onLeave(s->stage->scene, s);
        }
    }
    char res = fn_4080a0(s);
    if (s->child)
        fn_4086e4(s->child, s->x - old.x, s->y - old.y);
    return res;
}

extern "C" char fn_4086e4(TGraphicSprite *s, int dx, int dy)
{
    char r;
    fn_4080a0(s);
    s->x += dx;
    s->y += dy;
    r = fn_4080a0(s);
    if (s->child)
        fn_4086e4(s->child, dx, dy);
    return r;
}

extern "C" void fn_408740(TGraphicSprite *s, int dx, int dy)
{
    fn_4086e4(s, dx * 1000, dy * 1000);
}

extern "C" char fn_408760(TGraphicSprite *s, int k)
{
    char r = 0;
    for (int i = 0; i < 4; i++) {
        if (fn_4077ec(s, i, k)) {
            fn_407820(s, i);
            s->timerCb[i](s->stage->scene, s);
            r = 1;
        }
    }
    return r;
}

extern "C" char fn_4087c8(TGraphicSprite *s, TScene *a, int t)
{
    int n;
    if (s->queueLen > 0) {
        fn_407e74(s, t);
    } else {
        if (s->animate && s->nextAnim < t) {
            n = s->frame + 1;
            if (s->animFirst + s->animCount <= n)
                n = s->animFirst;
            fn_406c50(s, n);
            s->nextAnim = fn_4024cc(s->nextAnim, s->animPeriod);
        }
        if (s->move && s->nextMove < t) {
            fn_4081b8(s);
            s->f30 = t;
            s->nextMove = fn_4024cc(s->nextMove, s->movePeriod);
        }
        if (s->f65 && s->nextMove < t) {
            fn_406ac4(s, a);
            s->f30 = t;
            s->nextMove = fn_4024cc(s->nextMove, s->movePeriod);
        }
    }
    if (s->animating && s->nextTick < t)
        fn_4076e8(s);
    return 1;
}

extern "C" void fn_408908(TGraphicSprite *s)
{
    if (!ELF_AS(bool, s->shown)) {
        ELF_AS(bool, s->shown) = true;
        fn_4080a0(s);
    }
}

extern "C" void fn_408924(TGraphicSprite *s)
{
    if (ELF_AS(bool, s->shown) == true) {
        ELF_AS(bool, s->shown) = false;
        fn_4080a0(s);
    }
}
