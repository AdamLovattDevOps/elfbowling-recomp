// Game unit, pin/elf logic 0x414240-0x41665c.
#include <stdlib.h>
#include <elf/funcs.h>


extern "C" void fn_414240(TScene *self, int idx)
{
    PinState *p = &g_460240[idx];
    if (idx == g_4606c8 || g_4606c8 == -1) { p->state = 2; return; }
    if (idx == g_4606d0 || g_4606d0 == -1) { p->state = 3; return; }
    if (idx == g_4606e4 || g_4606e4 == -1) { p->state = 4; return; }
    if (idx == g_4606e8 || g_4606e8 == -1) { p->state = 5; return; }
    if (idx == g_4606d8 || g_4606d8 == -1) { p->state = 6; return; }
    if (idx == g_4606d4 || g_4606d4 == -1) { p->state = 8; return; }
    if (g_4606dc != -2) { p->state = 7; return; }
    if (p->state == 0) {
        if (idx == 9 || rand() % 100 < 30)
            p->state = 1;
    }
}

extern "C" void fn_41433c(TScene *self, TGraphicSprite *s)
{
    if ((s->queueLen > 0) == false) {
        TPoint p = s->pos;
        p.x += rand() % 5 - 2;
        fn_406a80(s, p);
    }
    fn_4077c4(s, 1, rand() % 3000 + 1500, fn_41433c);
}

extern "C" void fn_4143b8(TScene *self, int idx)
{
    TGraphicSprite *s = g_46053c->items[idx];
    fn_407820(s, 1);
}

extern "C" void fn_4143dc(TScene *self, int idx)
{
    TGraphicSprite *s = g_46053c->items[idx];
    fn_4077c4(s, 1, rand() % 3000 + 1500, fn_41433c);
}

extern "C" void fn_414418(TScene *self, int idx)
{
    TGraphicSprite *s;
    fn_413490(idx, 0);
    fn_4134d4(idx, 0);
    fn_412e70(idx, 0);
    s = g_460538->items[idx];
    fn_40779c(s);
    fn_407834(s);
    s = g_46053c->items[idx];
    fn_407834(s);
    s = g_460540->items[idx];
    fn_407834(s);
    s = g_460544->items[idx];
    fn_40779c(s);
    fn_407834(s);
    s = g_460548->items[idx];
    fn_407834(s);
    s = g_46054c->items[idx];
    fn_407834(s);
    fn_4143b8(self, idx);
    fn_4141d0(self, idx);
    fn_412fdc(idx);
}


extern "C" void fn_414510(TScene *self, TGraphicSprite *unused)
{
    for (int i = 0; i < 10; i++) {
        if (g_460578[i] && g_4606f8[i]) {
            fn_412fdc(i);
            fn_4141d0(self, i);
            fn_413490(i, 0);
            fn_412e70(i, 0);
        }
    }
}

extern "C" void fn_414570(TScene *self)
{
    TGraphicSprite *s = fn_40a598(self, "BlackLine");
    fn_4077c4(s, 0, 200, fn_414510);
}

extern "C" void fn_4145a4(TScene *self)
{
    int n = 0;
    for (int i = 0; i < 10; i++) {
        g_4606f8[i] = 0;
        if (g_460578[i]) {
            PinState *p = &g_460240[i];
            bool skip = g_4605bc == 0 && (p->state == 3 || p->state == 8);
            skip |= p->state == 6;
            skip |= rand() % 2 < 0;
            skip |= g_460597;
            if (!skip) {
                g_4606f8[i] = 1;
                fn_4141d0(self, i);
                fn_413490(i, 2);
                fn_412f18(i, 100, 6, 2);
                n++;
            }
        }
    }
    if (n)
        fn_40cb34(self->stage->sound, self, g_460538->items[0], "ElfScream.wav", 0x2a, 0);
}

extern "C" void fn_4146b8(TScene *self)
{
    if (!g_4606ec) {
        fn_4145a4(self);
        g_4606ec = 1;
    }
}

extern "C" void fn_4146d8(TScene *self, int idx)
{
    TGraphicSprite *s;
    s = g_460538->items[idx];
    fn_406c50(s, 0);
    fn_408908(s);
    s = g_460540->items[idx];
    fn_406c50(s, 0);
    fn_408908(s);
    s = g_46053c->items[idx];
    fn_406c50(s, 0);
    fn_408908(s);
    fn_407a68(s, Classes::Rect(0, 0, 640, 480));
    s = g_460544->items[idx];
    fn_406c50(s, 0);
    fn_408908(s);
    s = g_46054c->items[idx];
    fn_406c50(s, 0);
    fn_408908(s);
    s = g_460548->items[idx];
    fn_406c50(s, 0);
    fn_408908(s);
}

extern "C" void fn_4147e8(TScene *self, int idx, bool fast)
{
    fn_4146d8(self, idx);
    int n = fast ? 80 : 200;
    fn_412f68(idx, random(n) + n, 0, 2);
    fn_412f18(idx, random(n) + n, 0, 2);
}

extern "C" void fn_41486c(TScene *self, int idx)
{
    fn_414024(self, idx);
    fn_4143dc(self, idx);
}


extern "C" void fn_414890(TScene *self)
{
    for (int i = 0; i < 12; i++)
        g_460704[i] = -1;
    int n = 0;
    for (int j = n; j < 10; j++) {
        int r = rand() % 10;
        while (g_460704[r] != -1)
            r = rand() % 10;
        g_460704[r] = j;
    }
}

extern "C" void fn_41490c(TScene *self, int frame)
{
    g_4606d0 = -2;
    g_4606c8 = -2;
    g_4606d8 = -2;
    g_4606dc = -2;
    g_4606e0 = -2;
    g_4606d4 = -2;
    g_4606e4 = -2;
    g_4606e8 = -2;
    g_4606cc = 0;
    int k = g_460704[frame];
    if (k == 3) {
        g_4606d4 = 9;
        g_4606c8 = rand() % 2 + 7;
        g_4606cc = rand() % 2;
    } else if (k == 0) {
        g_4606dc = 9;
        g_4606e0 = rand() % 2 + 7;
    } else if (k == 6) {
        g_4606e4 = -1;
    } else if (k == 7) {
        g_4606e8 = -1;
        g_4606c8 = rand() % 2 + 7;
        g_4606cc = 2;
    } else if (k == 9) {
        g_4606d8 = rand() % 3 + 7;
    } else if (k == 1) {
        g_4606d0 = -1;
    } else if (k == 4) {
        g_4606c8 = rand() % 2 + 7;
        g_4606cc = 0;
    } else if (k == 5) {
        g_4606c8 = rand() % 2 + 7;
        g_4606cc = 1;
    } else if (k == 2) {
        fn_4163d4(self, 0);
    } else if (k == 8) {
        fn_4163d4(self, 1);
    }
    for (int i = 0; i < 10; i++) {
        PinState *p = &g_460240[i];
        p->f4 = 0;
        p->state = 0;
        fn_414418(self, i);
        fn_414240(self, i);
    }
}

extern "C" void fn_414b30(TScene *self)
{
    g_4606ec = 0;
    for (int i = 0; i < 10; i++)
        if (g_460578[i])
            fn_41486c(self, i);
}

extern "C" void fn_414b68(TScene *self, int idx)
{
    TGraphicSprite *s;
    s = g_460538->items[idx];
    fn_408924(s);
    s = g_460540->items[idx];
    fn_408924(s);
    s = g_46053c->items[idx];
    fn_408924(s);
    fn_407a68(s, Classes::Rect(0, 0, 640, 480));
    s = g_460544->items[idx];
    fn_408924(s);
    s = g_46054c->items[idx];
    fn_408924(s);
    s = g_460548->items[idx];
    fn_408924(s);
}

extern "C" void fn_414c28(TScene *self, int idx)
{
    TGraphicSprite *s;
    s = g_460538->items[idx];
    fn_408924(s);
    s = g_460540->items[idx];
    fn_408924(s);
    s = g_460544->items[idx];
    fn_408924(s);
    s = g_46054c->items[idx];
    fn_408924(s);
}

extern "C" void fn_414c90(TScene *self)
{
    for (int i = 0; i < 10; i++)
        fn_414b68(self, i);
}

extern "C" void fn_414cb4(TScene *self, TGraphicSprite *s)
{
    fn_4130a4(s->lane);
    fn_413168(s->lane);
    int v = s->frame;
    int f;
    if (v < 10)
        f = rand() % 2 + 8;
    else
        f = rand() % 2 + 12;
    fn_4134d4(s->lane, f);
}


extern "C" void fn_414d30(TScene *self, TGraphicSprite *s)
{
    int idx = s->lane;
    int v = g_4605b0[g_4605b4 ? g_458840[idx] : idx];
    fn_4141d0(self, idx);
    fn_4143b8(self, idx);
    fn_414c28(self, idx);
    switch (v) {
    case 2: v = 3000; break;
    case 3: v = 8000; break;
    default: v = 0; break;
    }
    v = (g_4605b4 ? -1 : 1) * v;
    TPoint pt = Classes::Point(v, -18000);
    int base = rand() % 2 ? 6 : 10;
    int f = rand() % 2 + base;
    fn_4134d4(idx, f);
    fn_413244(idx, pt);
    g_46053c->items[idx]->onLeave = fn_414cb4;
}

extern "C" void fn_414e50(TScene *self, TGraphicSprite *s)
{
    fn_414d30(self, s);
}

extern "C" void fn_414e64(TScene *self, int row)
{
    int lo, hi;
    switch (row) {
    case 0: lo = 0; hi = 4; break;
    case 1: lo = 4; hi = 7; break;
    case 2: lo = 7; hi = 9; break;
    case 3: lo = 9; hi = 10; break;
    }
    for (int i = lo; i < hi; i++) {
        if (g_460578[i] && !g_460582[i]) {
            int d = g_4605b8[g_4605b4 ? g_458840[i] : i];
            if (d == 0)
                fn_414d30(self, g_46053c->items[i]);
            else
                fn_4077c4(g_46053c->items[i], 3, d, fn_414e50);
            g_460578[i] = 0;
            g_46058c[i] = 1;
        }
    }
}

extern "C" void fn_414f70(TScene *self)
{
    for (int i = 0; i < 10; i++) {
        if (g_460578[i])
            fn_4146d8(self, i);
        else
            fn_414b68(self, i);
    }
}

extern "C" void fn_414fb0(TScene *self)
{
    for (int i = 0; i < 10; i++) {
        if (g_460578[i]) {
            switch (rand() % 3) {
            case 0: fn_4147e8(self, i, true); break;
            case 1: fn_4147e8(self, i, false); break;
            }
        }
    }
}

extern "C" void fn_41500c(TScene *self)
{
    for (int i = 0; i < 10; i++) {
        if (g_460578[i]) {
            fn_414418(self, i);
            fn_4134d4(i, 0);
            fn_412e70(i, 0);
            fn_4133cc(i);
        }
    }
}

extern "C" void fn_415060(TScene *self)
{
    for (int i = 0; i < 10; i++) {
        TGraphicSprite *s;
        s = g_460538->items[i];
        fn_4078a8(s);
        fn_407834(s);
        s = g_460540->items[i];
        fn_4078a8(s);
        fn_407834(s);
        s = g_46053c->items[i];
        fn_4078a8(s);
        fn_407834(s);
        fn_407a68(s, Classes::Rect(0, 0, 640, 480));
        s = g_460544->items[i];
        fn_4078a8(s);
        fn_407834(s);
        s = g_46054c->items[i];
        fn_4078a8(s);
        fn_407834(s);
        s = g_460548->items[i];
        fn_4078a8(s);
        fn_407834(s);
    }
}

extern "C" void fn_415170(TScene *self)
{
    fn_414c90(self);
    for (int i = 0; i < 10; i++) {
        TGraphicSprite *a, *b, *c;
        a = g_460538->items[i];
        a->lane = i;
        b = g_460540->items[i];
        b->lane = i;
        c = g_46053c->items[i];
        c->lane = i;
        a = g_460544->items[i];
        a->lane = i;
        b = g_46054c->items[i];
        b->lane = i;
        c = g_460548->items[i];
        c->lane = i;
    }
}


extern "C" void fn_41523c(int idx, int v)
{
    TGraphicSprite *s;
    s = g_460570->items[idx];
    s->animate = 0;
    fn_406c50(s, v);
    s = g_460574->items[idx];
    s->animate = 0;
    fn_406c50(s, v);
}

extern "C" void fn_415294(int idx, int a, SpriteCb b)
{
    TGraphicSprite *s;
    s = g_460570->items[idx];
    s->animate = 0;
    fn_407fdc(s, a, b);
    s = g_460574->items[idx];
    s->animate = 0;
    fn_407fdc(s, a, 0);
}

extern "C" void fn_4152f0(int idx)
{
    TGraphicSprite *s;
    s = g_460570->items[idx];
    fn_40806c(s);
    s = g_460574->items[idx];
    fn_40806c(s);
}

extern "C" void fn_415328(TScene *self, TGraphicSprite *s)
{
    fn_412cfc(self);
    fn_4077c4(s, 3, 100, fn_4128b4);
}

extern "C" void fn_41534c(TScene *self, TGraphicSprite *arg)
{
    for (int i = 0; i < g_460570->count; i++) {
        TGraphicSprite *s = g_460570->items[i];
        fn_4078a8(s);
        fn_407834(s);
    }
    fn_415328(self, arg);
}

extern "C" void fn_4153a0(TScene *self, TGraphicSprite *arg)
{
    for (int i = 0; i < 10; i++) {
        if (g_460578[i]) {
            TGraphicSprite *s = g_460574->items[i];
            TPoint p1 = Classes::Point(0, -2000);
            fn_4086e4(s, p1.x, p1.y);
            arg = g_46053c->items[i];
            TPoint p2 = Classes::Point(0, -8000);
            fn_4086e4(arg, p2.x, p2.y);
            arg = g_460548->items[i];
            TPoint p3 = Classes::Point(0, -2000);
            fn_4086e4(arg, p3.x, p3.y);
        }
    }
}

extern "C" void fn_41545c(TScene *self, TGraphicSprite *arg)
{
    int n = 0;
    for (int i = 0; i < 10; i++) {
        TGraphicSprite *s = g_460570->items[i];
        s->onLeave = 0;
        if (g_460578[i]) {
            fn_4079dc(s, Classes::Point(0, -8000), 30, 0, 0);
            if (n++ == 0) {
                s->onCell = (SpriteCb)fn_4153a0;
                s->onLeave = fn_41534c;
            }
            fn_4141d0(self, i);
            fn_4143b8(self, i);
        }
    }
    fn_40cb34(self->stage->sound, self, arg, "Rackpins.wav", 14, 0);
    fn_414fb0(self);
}

extern "C" void fn_41553c(TScene *self, TGraphicSprite *arg)
{
    fn_412d2c(self);
    for (int i = 0; i < g_460570->count; i++) {
        TGraphicSprite *s = g_460570->items[i];
        fn_4078a8(s);
    }
    fn_41500c(self);
    fn_4077c4(arg, 0, 400, (SpriteCb)fn_41545c);
}

extern "C" void fn_4155a0(TScene *self)
{
    for (int i = 0; i < g_460574->count; i++) {
        if (g_460578[i]) {
            TGraphicSprite *s = g_460574->items[i];
            TPoint p = Classes::Point(0, 2000);
            fn_4086e4(s, p.x, p.y);
        }
    }
}

extern "C" void fn_415600(TScene *self)
{
    int n = 0;
    for (int i = 0; i < g_460570->count; i++) {
        if (g_460578[i]) {
            TGraphicSprite *s = g_460574->items[i];
            fn_406c00(s);
            s = g_460570->items[i];
            fn_406c00(s);
            TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
            Windows::TRect r = Classes::Rect(pt.x - 2, pt.y - 1, pt.x + 2, pt.y + 0x150);
            Windows::TRect r2;
            r2 = r;
            s->clip = r2;
            fn_4079dc(s, Classes::Point(0, 8000), 30, 0, 0);
            s->onLeave = 0;
            if (n++ == 0) {
                s->onLeave = (SpriteCb)fn_41553c;
                s->onCell = (SpriteCb)fn_4155a0;
            }
        }
    }
    TGraphicSprite *first = g_460570->items[0];
    if (n > 0)
        fn_40cb34(self->stage->sound, self, first, "Rackpins.wav", 14, 0);
    else
        fn_415328(self, first);
}

extern "C" void fn_415780(TScene *self)
{
    for (int i = 0; i < g_460570->count; i++) {
        TGraphicSprite *s = g_460570->items[i];
        fn_4078a8(s);
        fn_407834(s);
    }
    fn_414b30(self);
    fn_4127c0(self);
}

extern "C" void fn_4157d8(TScene *self)
{
    for (int i = 0; i < g_460574->count; i++) {
        PinState *p = &g_460240[i];
        if (g_4605bc == 0 && p->state == 6)
            continue;
        TGraphicSprite *s = g_460574->items[i];
        TPoint pt = Classes::Point(0, -2000);
        fn_4086e4(s, pt.x, pt.y);
    }
}

extern "C" void fn_415848(TScene *self, TGraphicSprite *s)
{
    if (g_460734++ < 2) {
        fn_415294(s->lane, 0, (SpriteCb)fn_415848);
        fn_40cb34(self->stage->sound, self, s, "Bounce.wav", 0x1f, 0);
        return;
    }
    fn_415294(s->lane, 1, 0);
    fn_40cb34(self->stage->sound, self, s, "HeadPop.wav", 0x1f, 0);
}

extern "C" void fn_4158d8(TScene *self, TGraphicSprite *arg)
{
    int n = 0;
    bool all = true;
    for (int i = 0; i < g_460570->count; i++) {
        if (g_460578[i]) {
            bool handled = false;
            PinState *p = &g_460240[i];
            TGraphicSprite *s = g_460570->items[i];
            if (p->state == 6) {
                if (g_4605bc == 0) {
                    fn_413ffc(i);
                    fn_415294(i, 0, (SpriteCb)fn_415848);
                    fn_40cb34(self->stage->sound, self, arg, "Bounce.wav", 0x1f, 0);
                    handled = true;
                    all = false;
                    g_460734 = 0;
                } else {
                    fn_41523c(i, 1);
                    fn_413ffc(i);
                }
            }
            if (!handled) {
                fn_4079dc(s, Classes::Point(0, -8000), 30, 0, 0);
                s->onLeave = 0;
                if (n++ == 0) {
                    s->onCell = (SpriteCb)fn_4157d8;
                    s->onLeave = (SpriteCb)fn_415780;
                }
            }
        }
    }
    if (all)
        fn_40cb34(self->stage->sound, self, arg, "Rackpins.wav", 14, 0);
}

extern "C" void fn_415a40(TScene *self, TGraphicSprite *arg)
{
    fn_41500c(self);
    fn_4077c4(arg, 0, 400, (SpriteCb)fn_4158d8);
}

extern "C" void fn_415a68(TScene *self, TGraphicSprite *arg)
{
    for (int i = 0; i < g_460570->count; i++) {
        TGraphicSprite *s = g_460570->items[i];
        fn_4078a8(s);
    }
    fn_412a94(self);
    fn_4077c4(arg, 0, 450, fn_415a40);
}

extern "C" void fn_415ac4(TScene *self)
{
    for (int i = 0; i < 10; i++) {
        if (g_460578[i]) {
            TGraphicSprite *s = g_46053c->items[i];
            TPoint p1 = Classes::Point(0, 8000);
            fn_4086e4(s, p1.x, p1.y);
            s = g_460548->items[i];
            TPoint p2 = Classes::Point(0, 2000);
            fn_4086e4(s, p2.x, p2.y);
            s = g_460574->items[i];
            TPoint p3 = Classes::Point(0, 2000);
            fn_4086e4(s, p3.x, p3.y);
        }
    }
}

extern "C" void fn_415b80(TScene *self, TGraphicSprite *arg)
{
    TGraphicSprite *s;
    fn_415060(self);
    int n = 0;
    for (int i = 0; i < 10; i++) {
        fn_4152f0(i);
        fn_41523c(i, 0);
        s = g_46053c->items[i];
        TPoint p = s->pos;
        p.y -= 0x150;
        fn_406a80(s, p);
        s = g_460548->items[i];
        p = s->pos;
        p.y -= 0x54;
        fn_406a80(s, p);
        s = g_460574->items[i];
        fn_406c00(s);
        s = g_460570->items[i];
        fn_406c00(s);
        if (g_460578[i]) {
            TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
            Windows::TRect r = Classes::Rect(pt.x - 2, pt.y - 1, pt.x + 2, pt.y + 0x150);
            Windows::TRect r2;
            r2 = r;
            s->clip = r2;
            fn_4079dc(s, Classes::Point(0, 8000), 30, 0, 0);
            s->onLeave = 0;
            if (n++ == 0) {
                s->onLeave = fn_415a68;
                s->onCell = (SpriteCb)fn_415ac4;
            }
        }
    }
    fn_414f70(self);
    fn_414fb0(self);
    fn_40cb34(self->stage->sound, self, arg, "Rackpins2.wav", 0x22, 0);
}

extern "C" void fn_415d7c(TScene *self)
{
    if (g_4605bc == 0)
        fn_41490c((TScene *)self, g_460598);
    TGraphicSprite *s = g_460570->items[0];
    fn_4077c4(s, 0, 300, (SpriteCb)fn_415b80);
}

extern "C" void fn_415dc0(TScene *self, TGraphicSprite *arg)
{
    fn_4077c4(arg, 0, 100, fn_415eb0);
}

extern "C" void fn_415ddc(TScene *self, TGraphicSprite *s)
{
    fn_408908(s);
    TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
    int base = g_460748 == 0 ? 0 : 6;
    if (pt.x < 160)
        fn_407fdc(s, rand() % 3 + base, (SpriteCb)fn_415dc0);
    else
        fn_407fdc(s, rand() % 2 + base + 3, (SpriteCb)fn_415dc0);
    if (g_460748 == 0)
        fn_40cb34(self->stage->sound, self, s, "FrogCroak.wav", 7, 0);
}


extern "C" void fn_415eb0(TScene *self, TGraphicSprite *s)
{
    if (g_460748 == 1 && g_460750->count > g_46074c) {
        TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
        int d = pt.x - 160;
        if (__abs__(d) < 60) {
            TGraphicSprite *e;
            int k = g_46074c++;
            e = g_460750->items[k];
            fn_408908(e);
            fn_406a80(e, pt);
            fn_406c50(e, rand() % 3);
        }
    }
    fn_415ddc((TScene *)self, s);
}

extern "C" void fn_415f80(TScene *self)
{
    TGraphicSprite *s = fn_40a598(self, "Kalvin");
    fn_406c00(s);
    fn_408924(s);
    fn_40806c(s);
    fn_407834(s);
}

extern "C" void fn_415fc0(TScene *self, TGraphicSprite *s)
{
}

extern "C" void fn_415fc8(TScene *self, TGraphicSprite *arg)
{
    fn_415f80(self);
    arg = fn_40a598(self, "Bird");
    TPoint p = arg->pos;
    TPoint pt = Classes::Point(arg->x / 1000, arg->y / 1000);
    fn_407858(arg, 200, 3, 3);
    fn_406970(arg, pt, p, 50, 30, 100, 0, 100);
    fn_40cb34(self->stage->sound, self, arg, "BirdDie.wav", 7, 0);
}

extern "C" void fn_41608c(TScene *self, TGraphicSprite *s)
{
    s->animate = 0;
    fn_406c50(s, 2);
    fn_4077c4(s, 0, 200, (SpriteCb)fn_415fc8);
}

extern "C" void fn_4160c0(TScene *self, TGraphicSprite *s)
{
    s->animate = 0;
    fn_406c50(s, 2);
    fn_4077c4(s, 0, 200, fn_41608c);
}

extern "C" void fn_4160f4(TScene *self)
{
    TGraphicSprite *s = fn_40a598(self, "Kalvin");
    int base = g_460748 == 0 ? 0 : 6;
    if (s->shown && !g_460744) {
        TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
        if (s->frame != 6) {
            fn_40806c(s);
            fn_407834(s);
            fn_407fdc(s, base + 5, 0);
            if (g_460748 == 0)
                fn_40cb34(self->stage->sound, self, s, "FrogCroak.wav", 7, 0);
        } else {
            s = fn_40a598(self, "Bird");
            fn_406c00(s);
            fn_408908(s);
            TPoint p2 = Classes::Point(s->x / 1000, s->y / 1000);
            pt.y -= 10;
            fn_407858(s, 200, 0, 3);
            fn_406970(s, p2, pt, 20, 30, 80, fn_4160c0, 100);
            fn_40cb34(self->stage->sound, self, s, "BirdDie.wav", 7, 0);
        }
        g_460744 = 1;
    }
}

extern "C" void fn_416288(TScene *self, TGraphicSprite *s)
{
    if (!g_460738 && !g_460744) {
        TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
        if (pt.y <= g_46073c) {
            TGraphicSprite *k = fn_40a598(self, "Kalvin");
            if (fn_406868(k, s)) {
                TPoint kp = Classes::Point(k->x / 1000, k->y / 1000);
                if (g_460748 == 0) {
                    int d = kp.x - pt.x;
                    if (__abs__(d) < 10) {
                        fn_40806c(k);
                        fn_407834(k);
                        fn_406c50(k, 6);
                        fn_40cb34(self->stage->sound, self, k, "FrogUh.wav", 12, 0);
                        fn_4077c4(k, 3, 230, fn_415fc0);
                        goto done;
                    }
                }
                fn_4160f4(self);
            }
        done:
            if (pt.y <= g_460740)
                g_460738 = 1;
        }
    }
}

extern "C" void fn_4163d4(TScene *self, int mode)
{
    g_460748 = mode;
    TGraphicSprite *s = fn_40a598(self, "Bird");
    fn_406c00(s);
    fn_408924(s);
    fn_40806c(s);
    fn_407834(s);
    s->animate = 0;
    fn_415f80(self);
    s = fn_40a598(self, "Kalvin");
    g_460738 = 1;
    g_460744 = 0;
    if (g_460598 >= 0 && rand() % 100 < 100) {
        TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
        g_46073c = pt.y;
        g_460740 = pt.y - 10;
        fn_4077c4(s, 0, rand() % 1000 + 2400, (SpriteCb)fn_415ddc);
        g_460738 = 0;
    }
}

extern "C" void fn_4164d8(TScene *self)
{
    fn_415f80(self);
    for (int i = 0; i < g_460750->count; i++) {
        TGraphicSprite *s = g_460750->items[i];
        fn_408924(s);
        fn_406c00(s);
    }
    g_46074c = 0;
    TGraphicSprite *b = fn_40a598(self, "Bird");
    fn_406c00(b);
    fn_408924(b);
    fn_40806c(b);
    fn_407834(b);
    b->animate = 0;
}

extern "C" void fn_416570(char *pins)
{
    fn_401cf4("%c   %c   %c   %c\n", pins[3] ? '3' : '.', pins[2] ? '2' : '.', pins[1] ? '1' : '.', pins[0] ? '0' : '.');
    fn_401cf4("  %c   %c   %c\n", pins[6] ? '6' : '.', pins[5] ? '5' : '.', pins[4] ? '4' : '.');
    fn_401cf4("    %c   %c\n", pins[8] ? '8' : '.', pins[7] ? '7' : '.');
    fn_401cf4("      %c\n\n", pins[9] ? '9' : '.');
}

extern "C" bool fn_4169a0(TScene *s)
{
    if (g_460598 < 9)
        return false;
    int *p = g_4605c8[9].v;
    int kind = 0;
    if (p[0] == 10)
        kind = 2;
    else if (p[0] + p[1] == 10)
        kind = 1;
    else
        return p[0] >= 0 && p[1] >= 0;
    int *q = g_4605c8[10].v;
    int *r = g_4605c8[11].v;
    if (kind == 1)
        return q[0] >= 0;
    if (q[0] == 10)
        return r[0] >= 0;
    return q[0] >= 0 && q[1] >= 0;
}


extern "C" void fn_416a60(TScene *self)
{
    for (int i = 0; i < g_460568->count; i++) {
        TGraphicSprite *s = g_460568->items[i];
        fn_4078a8(s);
        fn_407834(s);
        fn_408924(s);
    }
}

extern "C" void fn_416ab0(TScene *self)
{
    for (int i = 0; i < g_46056c->count; i++) {
        TGraphicSprite *s = g_46056c->items[i];
        fn_4078a8(s);
        fn_407834(s);
        fn_408924(s);
    }
}

extern "C" void fn_416b00(TScene *self, TGraphicSprite *arg)
{
    fn_416ab0(self);
    fn_416a60(self);
    fn_40cbd0(self->stage->sound, self);
    if (g_4605c0 == 0)
        fn_40cb34(self->stage->sound, self, arg, "GutterBall.wav", 0x2c, 0);
    else
        fn_40cb34(self->stage->sound, self, arg, "Pins.wav", 0x2e, 0);
    fn_414570(self);
    int t = fn_4124f8(self, g_460597);
    fn_412c60(self);
    fn_4160f4(self);
    fn_4077c4(arg, 0, t + 500, fn_4128d4);
}
