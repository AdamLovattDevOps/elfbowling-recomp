// Game unit, 0x416f10 onwards. Struct layouts shared with game_414240.cpp.
#include <stdlib.h>
#include <elf/funcs.h>

// Alias of fn_407a44: the velocity is passed as a TPoint by value (the
// register rotation differs from the two-int spelling in funcs.h).
extern "C" void fn_407a44_pt(TGraphicSprite *s, TPoint v, int a, int b, int c);

extern "C" void fn_416f10(TScene *self, TGraphicSprite *arg)
{
    fn_412c60(self);
    fn_4160f4(self);
    fn_4077c4(arg, 0, 500, fn_4128d4);
}

extern "C" void fn_416f40(TScene *self, TGraphicSprite *arg)
{
    fn_4077c4(arg, 0, 1500, fn_416f10);
}

extern "C" void fn_416f5c(TScene *self, TGraphicSprite *arg)
{
    fn_414570(self);
    fn_412c80(self);
    fn_40cb34(self->stage->sound, self, arg, "hit.wav", 0x32, 0);
    fn_407fdc(arg, 1, fn_416f40);
}

extern "C" void fn_416fac(TScene *self, TGraphicSprite *arg)
{
    fn_416ab0(self);
    fn_416a60(self);
    TPoint pt = Classes::Point(arg->x / 1000, arg->y / 1000);
    arg = fn_40a598(self, "DeerBall");
    fn_408908(arg);
    fn_406a80(arg, pt);
    fn_40cbd0(self->stage->sound, self);
    fn_40cb34(self->stage->sound, self, arg, "BowlDrop.wav", 6, 0);
    fn_407fdc(arg, 0, fn_416f5c);
}

extern "C" void fn_41753c(TScene *self)
{
    g_gutterLeft = 0;
    g_gutterRight = 0;
    g_gutterBall = 0;
    g_ballRow = 4;
    g_ballOffset = fn_41237c(self);
    fn_41665c(self);
    TGraphicSprite *s = g_46056c->items[g_ballRow];
    fn_406c50(s, 0);
    fn_406a80(s, Classes::Point(160, 500));
    fn_40971c(g_46056c);
    fn_408908(s);
    TPoint v = Classes::Point(0, -16);
    s->clip = Classes::Rect(0, 0xd4, 0x140, 0x1f6);
    fn_407a44_pt(s, v, 20, 0, 0);
    s->onCell = fn_417074;
    s->onLeave = fn_416b00;
    fn_40cb34(self->stage->sound, self, s, "BowlDrop.wav", 8, 0);
    fn_40cb7c(self->stage->sound, self, "BowlBack.wav");
}

extern "C" void fn_41768c(TScene *self, TGraphicSprite *arg)
{
    fn_407fdc(arg, 1, 0);
}

extern "C" void fn_4176a0(TScene *self, TGraphicSprite *arg)
{
    fn_4077c4(arg, 0, 300, fn_41768c);
}

extern "C" void fn_4176bc(TScene *self)
{
    TGraphicSprite *s = fn_40a598(self, "Santa");
    int r = fn_412c30(self);
    if (r == 0) {
        fn_407fdc(s, 0, fn_4176a0);
        fn_40cb34(self->stage->sound, self, s, "HoHoHo.wav", 0x30, 0);
    } else
        fn_4176a0(self, s);
}

extern "C" void fn_417734(TScene *self, TGraphicSprite *arg)
{
    fn_406c50(arg, 0);
}

extern "C" void fn_417748(TScene *self, TGraphicSprite *arg)
{
    fn_4078a8(arg);
    fn_4077c4(arg, 0, 800, fn_417734);
}

extern "C" void fn_417770(TScene *self, TGraphicSprite *s)
{
    int st = s->frame;
    if (st == 0) {
        TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
        TRect r;
        r = s->clip;
        if (r.Bottom - 60 > pt.y) {
            fn_406c50(s, 1);
            fn_41753c(self);
        }
    }
}

extern "C" void fn_4177f0(TScene *self)
{
    g_45886d = 0;
    TGraphicSprite *s = fn_40a598(self, "Santa");
    TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
    TPoint v = Classes::Point(0, -10);
    s->clip = Classes::Rect(0, pt.y - 100, 320, pt.y + 2);
    fn_407a44_pt(s, v, 30, 0, 0);
    s->onCell = fn_417770;
    s->onLeave = fn_417748;
}

extern "C" void fn_4178b4(TScene *self, TGraphicSprite *arg)
{
    fn_4078a8(arg);
}

extern "C" void fn_4178c4(TScene *self)
{
    TGraphicSprite *s = fn_40a598(self, "Santa");
    fn_406c50(s, 0);
    fn_406c00(s);
    fn_408908(s);
    TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
    TPoint v = Classes::Point(0, -6);
    s->clip = Classes::Rect(0, pt.y - 0x48, 320, pt.y + 2);
    fn_407a44_pt(s, v, 60, 0, 0);
    s->onLeave = fn_4178b4;
}

extern "C" void fn_417994(TScene *self, TGraphicSprite *arg)
{
    fn_412850(self, arg);
}

extern "C" void fn_4179a8(TScene *self, TGraphicSprite *arg)
{
    fn_417994(self, arg);
}

extern "C" void fn_4179bc(TScene *self, TGraphicSprite *arg)
{
    fn_4077c4(arg, 0, 100, fn_4179a8);
}

extern "C" void fn_4179d8(TScene *self, TGraphicSprite *arg)
{
    fn_4078a8(arg);
    fn_408924(arg);
    fn_4179bc(self, arg);
}

extern "C" void fn_417a00(TScene *self, TGraphicSprite *s)
{
    g_460770 = (g_460770 + 1) % 2;
    TRect rr, r;
    fn_407a98(s, (RECT *)&rr);
    if (rr.Bottom >= 210) {
        r = ELF_AS(TRect, s->bounds);
        r.Left += 4;
        r.Right -= 4;
        fn_407a68(s, r);
    }
    for (int i = 0; i < 10; i++) {
        if (g_knocked[i]) {
            TGraphicSprite *e = g_46053c->items[i];
            TRect er;
            fn_407a98(e, (RECT *)&er);
            if (rr.Bottom + 10 < er.Bottom)
                fn_408740(e, 0, rr.Bottom + 10 - er.Bottom);
            if (rr.Bottom <= 210) {
                TRect b = Classes::Rect(0, 0, 640, 194);
                fn_407a68(e, b);
                TPoint pt = Classes::Point(e->x / 1000, e->y / 1000);
                pt.y = (210 - rr.Bottom) / 3 + 194;
                fn_406a80(e, pt);
            }
        }
    }
    s = fn_40a598(self, "LeftRake");
    if (rr.Bottom >= 210) {
        r = ELF_AS(TRect, s->bounds);
        r.Left++;
        r.Right--;
        fn_407a68(s, r);
    }
    TPoint pt2 = Classes::Point(s->x / 1000, s->y / 1000);
    pt2.y -= 2;
    fn_406a80(s, pt2);
    fn_407a98(s, (RECT *)&rr);
    for (int j = 0; j < 10; j++) {
        if (g_knocked[j]) {
            TGraphicSprite *e2 = g_460548->items[j];
            TRect er2;
            fn_407a98(e2, (RECT *)&er2);
            if (rr.Bottom + 2 < er2.Bottom)
                fn_408740(e2, 0, rr.Bottom + 2 - er2.Bottom);
            if (rr.Bottom <= 224)
                fn_408924(e2);
        }
    }
}

extern "C" void fn_417c74(TScene *self, TGraphicSprite *s)
{
    fn_4078a8(s);
    TPoint v = Classes::Point(0, -8);
    TPoint p = s->pos;
    s->clip = Classes::Rect(320, p.y - 1, 640, 0x98);
    fn_407a44_pt(s, v, 20, 0, 0);
    s->onCell = fn_417a00;
    s->onLeave = fn_4179d8;
}

extern "C" void fn_417d10(TScene *self, TGraphicSprite *arg)
{
    fn_4078a8(arg);
    fn_4077c4(arg, 0, 300, fn_417c74);
}

extern "C" void fn_417d38(TScene *self, TGraphicSprite *arg)
{
    arg = fn_40a598(self, "LeftRake");
    TPoint pt = Classes::Point(arg->x / 1000, arg->y / 1000);
    pt.y += 2;
    fn_406a80(arg, pt);
}

extern "C" void fn_417d98(TScene *self, TGraphicSprite *s)
{
    fn_406c00(s);
    fn_408908(s);
    g_460770 = 0;
    TGraphicSprite *lr = fn_40a598(self, "LeftRake");
    fn_406c00(lr);
    fn_408908(lr);
    TRect r;
    fn_407a98(s, (RECT *)&r);
    r.Top = 0;
    r.Bottom = 480;
    fn_407a68(s, r);
    fn_407a98(lr, (RECT *)&r);
    r.Top = 0;
    r.Bottom = 480;
    fn_407a68(lr, r);
    TPoint v = Classes::Point(0, 8);
    TPoint p = s->pos;
    s->clip = Classes::Rect(320, p.y - 1, 640, 0x98);
    fn_407a44_pt(s, v, 20, 0, 0);
    s->onCell = fn_417d38;
    s->onLeave = fn_417d10;
}

extern "C" void fn_417ed4(TScene *self)
{
    bool any = false;
    for (int i = 0; i < 10; i++)
        any |= g_knocked[i];
    TGraphicSprite *rake = fn_40a598(self, "Rake");
    if (any)
        fn_417d98(self, rake);
    else
        fn_417994(self, rake);
}

extern "C" void fn_417f34(TScene *self)
{
    TGraphicSprite *s = fn_40a598(self, "LeftRake");
    fn_408924(s);
    s = fn_40a598(self, "Rake");
    fn_408924(s);
}

extern "C" void fn_417f74(TScene *self)
{
    if (g_roll == 1) {
        g_frame++;
        g_roll = 0;
        return;
    }
    if (g_frames[g_frame].v[g_roll] == 10) {
        g_frame++;
        g_roll = 0;
        return;
    }
    g_roll++;
}

extern "C" void fn_417fc4(TScene *self)
{
    fn_417f74(self);
    if (g_roll == 0)
        fn_412bc4(self);
    fn_4120b4(self);
    fn_415170(self);
    fn_412818(self);
    g_45886d = 0;
}

extern "C" void fn_418008(TScene *self)
{
    g_45886d = 0;
    if (g_frame >= g_deerFrame)
        fn_412d90(self);
    fn_415d7c(self);
    fn_4178c4(self);
}

extern "C" void fn_41803c(TScene *self)
{
}

extern "C" void fn_418044(TScene *self)
{
    TGraphicSprite *s = fn_40a598(self, "Hint1a");
    fn_408924(s);
    fn_407834(s);
}

extern "C" void fn_418070(TScene *self, TGraphicSprite *unused)
{
    fn_418044(self);
}

extern "C" void fn_418080(TScene *self, TGraphicSprite *arg)
{
    fn_408908(arg);
    fn_40cb34(self->stage->sound, self, arg, "BepBeep.wav", 4, 0);
    fn_4077c4(arg, 0, 3500, fn_418070);
}

extern "C" void fn_4180cc(TScene *self, int delay)
{
    g_hintShown = 1;
    TGraphicSprite *s = fn_40a598(self, "Hint1a");
    fn_4077c4(s, 0, delay, fn_418080);
}

extern "C" void fn_418104(TScene *self)
{
    if (g_hintCount++ == 0)
        fn_4180cc(self, 3750);
    fn_418008(self);
}

extern "C" void fn_418134(TScene *self, TGraphicSprite *arg)
{
    fn_408908(arg);
    if (arg->frame == 0)
        fn_406c50(arg, 1);
    else
        fn_406c50(arg, 0);
    fn_4077c4(arg, 0, 350, fn_418134);
}

extern "C" void fn_418180(TScene *self)
{
    TGraphicSprite *s = fn_40a598(self, "LightsOn");
    fn_418134(self, s);
}

extern "C" void fn_4181a8(TScene *self)
{
    fn_41803c(self);
    g_aimPeriod = 20;
    g_score = 0;
    fn_412358();
    g_cheated = 0;
    g_gameKey = rand() % 999999 + 1;
    g_frame = -1;
    g_roll = 1;
    fn_414890(self);
    fn_4120cc(self);
    fn_4164d8(self);
    fn_412e08(self);
    fn_418180(self);
    fn_417f34(self);
    fn_412740(self);
    fn_4125f0(self);
    fn_417fc4(self);
    fn_418104(self);
    fn_412b44(self);
}

extern "C" void fn_418260(TScene *self)
{
    fn_412904(self, 0x20);
}
