// Sprite helpers 0x4060d4-0x407ffc (old: Actor = TGraphicSprite).
#include <elf/funcs.h>

// fn_402748 (add) / fn_40278c (subtract) taking ERect (funcs.h has RECT; the
// by-value copy differs, the call target is a fixup).
extern "C" ERect fn_402748_t(ERect a, ERect b);
extern "C" ERect fn_40278c_t(ERect a, ERect b);

extern "C" void fn_4060d4(TGraphicSprite *a, TPoint pt)
{
    a->pos = pt;
    a->fpos = Classes_Point(pt.x * 1000, pt.y * 1000);
}

extern "C" void fn_406564(TGraphicSprite *a, char c)
{
    for (int i = 0; i < a->ncasts; i++)
        fn_404f94(a->casts[i], c);
}

extern "C" int fn_406678(TGraphicSprite *a, TGraphicCast *c)
{
    int r = -1;
    if (a->ncasts < 50) {
        r = a->ncasts;
        a->casts[a->ncasts++] = c;
        c->used = 1;
    }
    return r;
}

extern "C" char fn_406a08(TGraphicSprite *a, const char *v)
{
    TGraphicCast *c = a->casts[a->frame];
    char r = fn_405710((TTextCast *)c, v);
    fn_4080a0(a);
    return r;
}

extern "C" char fn_406a44(TGraphicSprite *a, const char *v)
{
    TGraphicCast *c = a->casts[a->frame];
    char r = fn_4057f8((TTextCast *)c, v);
    fn_4080a0(a);
    return r;
}

extern "C" void fn_406a80(TGraphicSprite *a, TPoint p)
{
    int dx = p.x * 1000 - a->fpos.x;
    int dy = p.y * 1000 - a->fpos.y;
    fn_4086e4(a, dx, dy);
}

extern "C" void fn_406c50(TGraphicSprite *a, int i)
{
    if (i != a->frame && i < a->ncasts) {
        fn_4080a0(a);
        a->frame = i;
        fn_4080a0(a);
    }
}

extern "C" char fn_40717c(TGraphicSprite *a, ERect *r)
{
    TGraphicCast *c = a->casts[a->frame];
    return fn_406ee0(a, c, r);
}

extern "C" char fn_4071a8(TGraphicSprite *a, ERect *r)
{
    TGraphicCast *c = a->talkCasts[a->frame];
    return fn_406ee0(a, c, r);
}

extern "C" char fn_407690(TGraphicSprite *a, ERect *r)
{
    TGraphicCast *c = a->casts[a->frame];
    return fn_407540(a, c, r);
}

extern "C" char fn_4076bc(TGraphicSprite *a, ERect *r)
{
    TGraphicCast *c = a->talkCasts[a->frame];
    return fn_407540(a, c, r);
}

extern "C" void fn_4079dc(TGraphicSprite *p, TPoint pt, int c, char d, char e)
{
    fn_407924(p, pt, c, d, e, 0);
}

extern "C" void fn_407a00(TGraphicSprite *p, int x, int y, int c, char d, char e, int f)
{
    fn_407924(p, Classes_Point(x * 1000, y * 1000), c, d, e, f);
}

extern "C" void fn_407a44(TGraphicSprite *p, TPoint pt, int c, char d, char e)
{
    fn_407a00(p, pt.x, pt.y, c, d, e, 0);
}

extern "C" void fn_407a68(TGraphicSprite *p, TRect r)
{
    fn_4080a0(p);
    p->bounds = ELF_AS(RECT, r);
    fn_4080a0(p);
}

extern "C" char fn_407a98(TGraphicSprite *a, RECT *out)
{
    *out = a->casts[a->frame]->bounds;
    char r = fn_406c8c(a, (ERect *)out);
    return r;
}

extern "C" void fn_407748(TGraphicSprite *t)
{
    t->animating = 1;
    t->phase = 0;
    t->nextTick = fn_4024bc(0);
    fn_4076e8(t);
}

extern "C" void fn_407774(TGraphicSprite *t, int period)
{
    t->period = period == 0 ? 0xa0 : period;
    fn_407748(t);
}

extern "C" void fn_40779c(TGraphicSprite *t)
{
    if (t->animating) {
        t->phase = 1;
        fn_4076e8(t);
        t->animating = 0;
    }
}

extern "C" void fn_4077c4(TGraphicSprite *t, int i, int delay, SpriteCb data)
{
    t->due[i] = fn_4024bc(delay);
    t->timerCb[i] = data;
}

extern "C" char fn_4077ec(TGraphicSprite *t, int i, int now)
{
    char r = 0;
    if (t->due[i])
        r = t->due[i] <= now;
    return r;
}

extern "C" void fn_407820(TGraphicSprite *t, int i)
{
    t->due[i] = 0;
}

extern "C" void fn_407834(TGraphicSprite *t)
{
    for (int i = 0; i < 4; i++)
        t->due[i] = 0;
}

extern "C" void fn_407f48(TGraphicSprite *p)
{
    p->f64 = 0;
    p->f58 = 0;
    p->stepTick = fn_4024bc(0);
    fn_407df0(p, 0);
}

extern "C" void fn_407fdc(TGraphicSprite *t, int a, SpriteCb b)
{
    t->queueLen = 0;
    fn_407ffc(t, a, b);
}

extern "C" void fn_406c00(TGraphicSprite *a)
{
    int dx = a->pos.x * 1000 - a->fpos.x;
    int dy = a->pos.y * 1000 - a->fpos.y;
    fn_4086e4(a, dx, dy);
}

extern "C" int fn_4066c4(TGraphicSprite *a, TGraphicCast *c, TPoint pt)
{
    int r = fn_406678(a, c);
    if (r >= 0) {
        a->pos = pt;
        a->fpos = Classes_Point(a->pos.x * 1000, a->pos.y * 1000);
    }
    return r;
}

extern "C" int fn_406724(TGraphicSprite *a, const char *name)
{
    int r = -1;
    TGraphicCast *c = fn_4059c4(a->stage->casts, name);
    if (c)
        r = fn_406678(a, c);
    else
        fn_401f30("AddCast - cannot find cast ", name);
    return r;
}

extern "C" int fn_406780(TGraphicSprite *a, const char *name, TPoint pt)
{
    int r = -1;
    TGraphicCast *c = fn_4059c4(a->stage->casts, name);
    if (c)
        r = fn_4066c4(a, c, pt);
    else
        fn_401f30("AddCast - cannot find cast ", name);
    return r;
}

extern "C" void fn_4076e8(TGraphicSprite *a)
{
    a->phase = (a->phase + 1) % 2;
    TGraphicCast *c = a->talkCasts[a->frame];
    if (c)
        fn_4080a0(a);
    a->nextTick = fn_4024cc(a->nextTick, a->period);
}

extern "C" void fn_407858(TGraphicSprite *a, int delay, int cast, int c)
{
    a->animate = 1;
    a->animPeriod = delay;
    a->animFirst = cast;
    a->animCount = c;
    a->nextAnim = fn_4024bc(delay);
    fn_406c50(a, cast);
}

extern "C" void fn_407f78(TGraphicSprite *a)
{
    if (a->queueLen > 0) {
        for (int i = 0; i < a->queueLen - 1; i++)
            a->queue[i] = a->queue[i + 1];
        a->queueLen--;
        if (a->queueLen > 0)
            fn_407f48(a);
        else
            fn_40806c(a);
    }
}

extern "C" void fn_406970(TGraphicSprite *a, TPoint p1, TPoint p2, int f1e0, int speed, int f80, SpriteCb f88, int f84)
{
    a->moveFrom = p1;
    a->moveTo = p2;
    a->moveProgress = 0;
    a->f84 = f84 * 100;
    a->moveSpeed = 10000 / speed;
    a->f80 = f80 * 100;
    a->onMoveDone = f88;
    a->movePeriod = f1e0;
    a->f30 = a->nextMove = fn_4024bc(0);
    a->f65 = 1;
    fn_406a80(a, p1);
}

extern "C" void fn_4078a8(TGraphicSprite *a)
{
    a->move = 0;
    a->vel = Classes_Point(Classes_Point(0, 0).x * 1000, Classes_Point(0, 0).y * 1000);
    a->ay = 0;
    a->dvy = 0;
    a->ax = 0;
    a->dvx = 0;
}

extern "C" int fn_407ad8(TGraphicSprite *a, int f0, int n)
{
    int r = -1;
    if (a->nseq < 24) {
        TSeq *s = a->seqs[a->nseq] = new TSeq;
        s->steps = new TSeqStep[n];
        s->used = 0;
        s->n = n;
        s->repeat = f0;
        s->fc = 0;
        s->done = 0;
        r = a->nseq++;
    } else
        fn_401f0c("StartBuildSequence: Too many sequences.");
    return r;
}

extern "C" void fn_407df0(TGraphicSprite *a, int i)
{
    a->step = i;
    TSeq *s = a->seqs[a->queue[0]];
    TSeqStep *st = s->steps + i;
    a->stepTick = fn_4024cc(a->stepTick, st->delay);
    fn_406c50(a, st->cast);
    fn_4086e4(a, st->x, st->y);
}

extern "C" void fn_407924(TGraphicSprite *a, TPoint pt, int c, char d, char e, int f)
{
    a->move = 1;
    a->onCell = 0;
    a->outX = a->outY = 0;
    a->movePeriod = c;
    a->f30 = fn_4024bc(0);
    a->nextMove = fn_4024bc(c);
    a->bounceX = d;
    a->bounceY = e;
    a->vel = pt;
    a->ay = f;
    a->dvy = a->ay;
    a->ax = 0;
    a->dvx = 0;
}

extern "C" char fn_407c40(TGraphicSprite *a, int cast, int reps, int n, int x, int y, unsigned short delay)
{
    char r = 1;
    TSeq *s = a->seqs[a->nseq - 1];
    for (int i = 0; i < reps; i++)
        for (int j = 0; j < n; j++) {
            s->steps[s->used].x = x;
            s->steps[s->used].y = y;
            s->steps[s->used].delay = delay;
            s->steps[s->used].cast = cast + j;
            s->used++;
        }
    return r;
}

extern "C" char fn_407b74(TGraphicSprite *a, int cast, int reps, int n, int x, int y, unsigned short delay)
{
    char r = 1;
    TSeq *s = a->seqs[a->nseq - 1];
    for (int i = 0; i < reps; i++)
        for (int j = 0; j < n; j++) {
            s->steps[s->used].x = x * 1000;
            s->steps[s->used].y = y * 1000;
            s->steps[s->used].delay = delay;
            s->steps[s->used].cast = cast + j;
            s->used++;
        }
    return r;
}

// Advance the current movement sequence when its step is due
extern "C" void fn_407e74(TGraphicSprite *a, int now)
{
    if (a->queueLen > 0 && a->stepTick <= now) {
        TSeq *s = a->seqs[a->queue[0]];
        a->step++;
        if (a->step >= s->used) {
            a->f58++;
            if ((!s->repeat || a->f58 < s->repeat) && !a->f64) {
                a->step = 0;
                fn_407df0(a, a->step);
            } else {
                SpriteCb cb = s->done;
                fn_407f78(a);
                if (cb)
                    cb(a->stage->scene, a);
            }
        } else
            fn_407df0(a, a->step);
    }
}

extern "C" void fn_40659c(TGraphicSprite *a, int x, int y, char vis, unsigned char mask)
{
    if (a->f0f0.x == 999999999) {
        a->f0f0.x = x - a->fpos.x / 1000;
        a->f0f0.y = y - a->fpos.y / 1000;
    }
    a->fpos = Classes_Point(x * 1000, y * 1000);
    a->pos = Classes_Point(a->fpos.x / 1000, a->fpos.y / 1000);
    a->shown = vis;
    for (int i = 0; i < a->ncasts; i++)
        fn_404f94(a->casts[i], mask);
}


// Pixel-accurate collision test between the current casts of a and b
extern "C" char fn_406868(TGraphicSprite *a, TGraphicSprite *b)
{
    ERect ra = ELF_AS(ERect, a->casts[a->frame]->bounds);
    ERect rb = ELF_AS(ERect, b->casts[b->frame]->bounds);
    char hit = 0;
    if (fn_40717c(a, &ra) && fn_40717c(b, &rb) && fn_402654(&ra, rb)) {
        rb = ra;
        fn_407690(a, &ra);
        fn_407690(b, &rb);
        hit = fn_403e8c(a->casts[a->frame], (RECT *)&ra, b->casts[b->frame], (RECT *)&rb);
    }
    return hit;
}

// Copy all sequences of src, scaling step offsets down by div
extern "C" void fn_407d04(TGraphicSprite *a, TGraphicSprite *src, int div)
{
    for (int i = 0; i < src->nseq; i++) {
        TSeq *s = src->seqs[i];
        int k = fn_407ad8(a, s->repeat, s->n);
        TSeq *d = a->seqs[k];
        d->used = s->used;
        rtl_memmove(d->steps, s->steps, d->used * sizeof(TSeqStep));
        for (int j = 0; j < d->used; j++) {
            d->steps[j].x = d->steps[j].x / div;
            d->steps[j].y = d->steps[j].y / div;
        }
    }
}


// Convert a cast-relative rect to screen coordinates
extern "C" void fn_406dd0(TGraphicSprite *a, ERect *r)
{
    TGraphicCast *c = a->casts[a->frame];
    *r = fn_40278c_t(*r, Classes_TRect(c->hot.x, c->hot.y, c->hot.x, c->hot.y));
    *r = fn_402748_t(*r, Classes_TRect(a->fpos.x / 1000, a->fpos.y / 1000, a->fpos.x / 1000, a->fpos.y / 1000));
}


// Current cast bounds in screen coordinates, clipped to the actor's clip rect
extern "C" char fn_407040(TGraphicSprite *a, ERect *r)
{
    TGraphicCast *c = a->casts[a->frame];
    *r = fn_40278c_t(*r, Classes_TRect(c->hot.x, c->hot.y, c->hot.x, c->hot.y));
    *r = fn_402748_t(*r, Classes_TRect(a->fpos.x / 1000, a->fpos.y / 1000, a->fpos.x / 1000, a->fpos.y / 1000));
    char ok = fn_402654(r, ELF_AS(ERect, a->bounds));
    return ok;
}

// Current cast bounds in screen coordinates, clipped to the screen
extern "C" char fn_406c8c(TGraphicSprite *a, ERect *r)
{
    TGraphicCast *c = a->casts[a->frame];
    *r = fn_40278c_t(*r, Classes_TRect(c->hot.x, c->hot.y, c->hot.x, c->hot.y));
    *r = fn_402748_t(*r, Classes_TRect(a->fpos.x / 1000, a->fpos.y / 1000, a->fpos.x / 1000, a->fpos.y / 1000));
    char ok = fn_402654(r, ELF_AS(ERect, a->stage->screen));
    return ok;
}


// Eased move from f68 to f70; progress f78 runs 0..10000
extern "C" void fn_406ac4(TGraphicSprite *a, TScene *arg)
{
    a->moveProgress = tmin_if(a->moveProgress + a->moveSpeed, 10000);
    if (a->moveProgress >= a->f80)
        a->moveSpeed = tmax_if(100, a->moveSpeed * 2 / 3);
    if (a->moveProgress >= a->f84) {
        if (a->onMoveDone)
            a->onMoveDone(arg, a);
        a->onMoveDone = 0;
    }
    if (a->moveProgress < 10000) {
        TPoint p;
        p.x = a->moveFrom.x - (a->moveFrom.x - a->moveTo.x) * a->moveProgress / 10000;
        p.y = a->moveFrom.y - (a->moveFrom.y - a->moveTo.y) * a->moveProgress / 10000;
        fn_406a80(a, p);
    } else {
        a->f65 = 0;
        fn_406a80(a, a->moveTo);
    }
}

// Screen rect -> cast-relative rect, clipped to the cast bounds
extern "C" char fn_407540(TGraphicSprite *a, TGraphicCast *c, ERect *r)
{
    fn_402654(r, ELF_AS(ERect, a->bounds));
    *r = fn_402748_t(*r, Classes_TRect(c->hot.x, c->hot.y, c->hot.x, c->hot.y));
    *r = fn_40278c_t(*r, Classes_TRect(a->fpos.x / 1000, a->fpos.y / 1000, a->fpos.x / 1000, a->fpos.y / 1000));
    char ok = fn_402654(r, ELF_AS(ERect, c->bounds));
    return ok;
}

// TGraphicCast-relative rect -> screen rect, clipped to the clip rect and the screen
extern "C" char fn_406ee0(TGraphicSprite *a, TGraphicCast *c, ERect *r)
{
    *r = fn_40278c_t(*r, Classes_TRect(c->hot.x, c->hot.y, c->hot.x, c->hot.y));
    *r = fn_402748_t(*r, Classes_TRect(a->fpos.x / 1000, a->fpos.y / 1000, a->fpos.x / 1000, a->fpos.y / 1000));
    char ok = fn_402654(r, ELF_AS(ERect, a->bounds));
    if (ok)
        ok = fn_402654(r, ELF_AS(ERect, a->stage->screen));
    return ok;
}


extern "C" void fn_406378(TGraphicSprite *a, TStage *e, const char *name, TPoint pt)
{
    a->stage = e;
    a->type = 1;
    a->bounds = ELF_AS(RECT, e->screen);
    ELF_AS(RECT, a->clip) = ELF_AS(RECT, e->screen);
    a->ncasts = 0;
    a->child = 0;
    a->keepParms = 0;
    a->f1d9 = 0;
    a->f1d0 = 0;
    a->lane = 0;
    a->draggable = 0;
    a->clickable = 0;
    a->onClick = 0;
    a->onRClick = 0;
    a->onGrab = 0;
    a->onDrag = 0;
    a->onDrop = 0;
    fn_4060d4(a, pt);
    a->nseq = 0;
    a->f0f0 = Classes_Point(999999999, 999999999);
    fn_402410(a->name, name, 0x18);
    for (int i = 0; i < 50; i++)
        a->casts[i] = a->talkCasts[i] = 0;
    fn_40610c(a);
}

// A real member function: with "this", Borland loads idx before this->ncasts
// (the extern "C" form loads a->ncasts first and does not match).
int TGraphicSprite::AddTalkingIndex(const char *name, int idx)
{
    int r = -1;
    TGraphicCast *c = fn_4059c4(stage->casts, name);
    if (c) {
        if (idx >= 0 && idx < ncasts) {
            talkCasts[idx] = c;
            c->used = 1;
            fn_404f94(c, 1);
            r = idx;
        }
    } else
        fn_401f30("AddTalkingIndex: cannot find cast ", name);
    return r;
}
// MATCH 4067e0 @TGraphicSprite@AddTalkingIndex$qpxci

// Push the rect back on screen horizontally
extern "C" char fn_4071d4(TGraphicSprite *a, ERect *r)
{
    char ok = 0;
    TGraphicCast *c = a->casts[a->frame];
    *r = fn_40278c_t(*r, Classes_TRect(c->hot.x, c->hot.y, c->hot.x, c->hot.y));
    *r = fn_402748_t(*r, Classes_TRect(a->fpos.x / 1000, a->fpos.y / 1000, a->fpos.x / 1000, a->fpos.y / 1000));
    if (a->stage->screen.left > r->left) {
        r->right = tmin_if(r->right, a->stage->screen.left);
        ok = 1;
    } else if (a->stage->screen.right < r->right) {
        r->left = tmax_rev(r->left, a->stage->screen.right);
        ok = 1;
    }
    return ok;
}

// Push the rect back on screen vertically
extern "C" char fn_407384(TGraphicSprite *a, ERect *r)
{
    char ok = 0;
    TGraphicCast *c = a->casts[a->frame];
    *r = fn_40278c_t(*r, Classes_TRect(c->hot.x, c->hot.y, c->hot.x, c->hot.y));
    *r = fn_402748_t(*r, Classes_TRect(a->fpos.x / 1000, a->fpos.y / 1000, a->fpos.x / 1000, a->fpos.y / 1000));
    if (a->stage->screen.top > r->top) {
        r->bottom = tmin_if(r->bottom, a->stage->screen.top);
        ok = 1;
    } else if (a->stage->screen.bottom < r->bottom) {
        r->top = tmax_if(r->top, a->stage->screen.bottom);
        ok = 1;
    }
    return ok;
}

extern "C" void fn_407820(TGraphicSprite *t, int i);

// Reset animation and movement state
extern "C" void fn_40610c(TGraphicSprite *a)
{
    a->frame = 0;
    a->animating = 0;
    a->nextTick = 0;
    a->f30 = 0;
    a->nextMove = 0;
    a->nextAnim = 0;
    a->phase = 0;
    a->period = 0;
    a->queueLen = 0;
    a->step = 0;
    a->stepTick = 0;
    a->f58 = 0;
    a->timer = 0;
    a->f64 = 0;
    a->f1dc = 0;
    a->f65 = 0;
    a->onMoveDone = 0;
    a->moveFrom = a->moveTo = Classes_Point(0, 0);
    a->moveProgress = 0;
    a->moveSpeed = 0;
    a->f80 = 0;
    a->f84 = 0;
    a->enabled = 1;
    a->fpos = Classes_Point(a->pos.x * 1000, a->pos.y * 1000);
    a->onCell = 0;
    a->vel = Classes_Point(Classes_Point(0, 0).x * 1000, Classes_Point(0, 0).y * 1000);
    a->fling = Classes_Point(0, 0);
    a->maxvy = 999999999;
    a->ay = a->dvy = 0;
    a->ax = a->dvx = 0;
    a->movePeriod = a->animPeriod = 0;
    a->dragTime = 0;
    a->move = 0;
    a->bounceX = 0;
    a->bounceY = 0;
    a->animFirst = 0;
    a->animCount = 0;
    a->animate = 0;
    a->onLeave = 0;
    a->grab = a->rclick = Classes_Point(999999999, 999999999);
    a->outX = a->outY = 0;
    for (int i = 0; i < 4; i++)
        fn_407820(a, i);
}
