// Score board / game-play helpers 0x4120cc-...
#include <stdlib.h>
#include <elf/funcs.h>

// fn_413168/fn_413244 call the inline TGraphicSprite::SetClip (a TRect by
// value into clip) and GetPos (the position in pixels).

extern "C" void fn_4120cc(TScene *s)
{
    int i;
    int j;
    FrameScore *f;
    for (i = 0; i < g_460558->count; i++) {
        fn_40a348(s, g_460558->items[i]->name, g_46055c->items[i]->name);
        fn_40a348(s, g_460558->items[i]->name, g_460560->items[i]->name);
        fn_40a348(s, g_460558->items[i]->name, g_460564->items[i]->name);
        fn_406a08(g_46055c->items[i], " ");
        fn_406a08(g_460560->items[i], " ");
        fn_406a08(g_460564->items[i], " ");
        if (g_460558->count - 1 == i) {
            fn_40a348(s, g_460558->items[i]->name, "ScoreMarkC09");
            fn_406a08(fn_40a598(s, "ScoreMarkC09"), " ");
        }
    }
    for (j = 0; j < 12; j++) {
        f = &g_frames[j];
        f->roll[0] = -1;
        f->roll[1] = -1;
        f->total = -1;
        f->shown0 = 0;
        f->shown1 = 0;
    }
}

extern "C" void fn_412260(TScene *s)
{
    int i;
    TGraphicSprite *spr;
    int d;
    for (i = 0; i < 15; i++) {
        spr = g_460554->items[i];
        d = abs(g_aimPos - i * 100);
        if (d == 0)
            fn_406c50(spr, 2);
        else if (d <= 50)
            fn_406c50(spr, 1);
        else if (d <= 100)
            fn_406c50(spr, 0);
    }
}

extern "C" void fn_4122e0(TScene *s, TGraphicSprite *spr)
{
    g_aimPos += g_aimDir * 50;
    if (g_aimPos > 1400) {
        g_aimDir = -1;
        g_aimPos = 1400;
    } else if (g_aimPos < 0) {
        g_aimDir = 1;
        g_aimPos = 0;
    }
    fn_412260(s);
    fn_4077c4(spr, 0, g_aimPeriod, fn_4122e0);
    g_aiming = true;
}

extern "C" void fn_412358()
{
    g_cheatX = false;
    g_cheatD = false;
    g_cheatS = false;
    g_cheatG = false;
}

extern "C" int fn_41237c(TScene *self)
{
    int r;
    if (g_cheatX)
        g_aimCell = rand() % 2 * 2 + 13;
    else if (g_cheatD)
        g_aimCell = 0;
    else if (g_cheatS)
        g_aimCell = g_roll == 0 ? 14 : rand() % 2 * 10 + 9;
    else if (g_cheatG)
        g_aimCell = rand() % 8 + rand() % 2 * 20 + 1;
    else
        g_aimCell = g_aimPos / 50;
    if (g_cheatX || g_cheatD || g_cheatS || g_cheatG)
        r = g_aimCell * 50 / 14 - 50;
    else
        r = g_aimPos / 14 - 50;
    return r;
}

extern "C" void fn_4124a4(TScene *s, TGraphicSprite *spr)
{
    spr = g_460538->items[g_46069c];
    fn_40d100(s->stage->sound, s, spr, g_458874[g_4606a0], 30, g_4606a0 == 1 ? 0 : 100, 0);
}

extern "C" int fn_4124f8(TScene *s, bool left)
{
    int delay;
    int side;
    int i;
    PinState *p;
    TGraphicSprite *line;
    int t;
    delay = 0;
    if (!g_tauntPending && g_roll == 1) {
        side = left ? 1 : 0;
        if (!g_460698[side]) {
            for (i = 9; i >= 0; i--) {
                p = &g_pins[i];
                if (g_present[i] && p->state != 6) {
                    if (side == 0)
                        g_460698[side] = true;
                    g_460694++;
                    g_46069c = i;
                    g_4606a0 = side;
                    line = fn_40a598(s, "BlackLine");
                    t = 1200 - side * 400;
                    delay = t + 1500;
                    fn_4077c4(line, 2, t, fn_4124a4);
                    g_tauntPending = true;
                    break;
                }
            }
        }
    }
    return delay;
}

extern "C" void fn_4125f0(TScene *s)
{
    int i;
    g_460694 = 0;
    for (i = 0; i < 2; i++)
        g_460698[i] = false;
}

extern "C" void fn_412618(TScene *s)
{
    int i;
    PinState *p;
    int k;
    TGraphicSprite *spr;
    if (g_4606a4 < 2) {
        for (i = 9; i >= 0; i--) {
            p = &g_pins[i];
            if (g_present[i] && p->state != 6) {
                k = rand() % 2;
                while (g_4606a8[k])
                    k = (k + 1) % 2;
                g_4606a8[k] = true;
                g_4606a4++;
                spr = g_460538->items[i];
                fn_40d100(s->stage->sound, s, spr, g_45887c[k], 30, 100, 0);
                g_tauntPending = true;
                break;
            }
        }
    }
}

extern "C" void fn_4126f8(TScene *s, TGraphicSprite *spr)
{
    if (g_45886d) {
        if (fn_4024bc(0) - g_458870 >= 5000) {
            fn_412618(s);
            return;
        }
        fn_4077c4(spr, 1, 1000, fn_4126f8);
    }
}

extern "C" void fn_412740(TScene *s)
{
    int i;
    g_4606a4 = 0;
    for (i = 0; i < 2; i++)
        g_4606a8[i] = false;
}

extern "C" void fn_412768(TScene *s)
{
    TGraphicSprite *line;
    g_tauntPending = false;
    g_45886d = true;
    g_458870 = fn_4024bc(0);
    if (g_roll == 1) {
        line = fn_40a598(s, "BlackLine");
        fn_4077c4(line, 1, 1000, fn_4126f8);
    }
}

extern "C" void fn_4127c0(TScene *s)
{
    fn_412768(s);
    fn_409754(g_460554, 0);
    g_aimPos = 0;
    g_aimDir = 1;
    fn_412260(s);
    fn_4077c4(g_460554->items[0], 0, g_aimPeriod, fn_4122e0);
}

extern "C" void fn_412818(TScene *s)
{
    g_aiming = false;
    fn_407834(g_460554->items[0]);
}

extern "C" void fn_412834(TScene *s, TGraphicSprite *unused)
{
    fn_40bf2c(s->stage, "Exit");
}

extern "C" void fn_412850(TScene *s, TGraphicSprite *spr)
{
    if (fn_4169a0(s)) {
        fn_40cb34(s->stage->sound, s, spr, "gameovr.wav", 0x3a, 0);
        fn_4077c4(spr, 3, 3000, fn_412834);
        return;
    }
    fn_417fc4(s);
    fn_418008(s);
}

extern "C" void fn_4128b4(TScene *s, TGraphicSprite *unused)
{
    fn_417ed4(s);
}

extern "C" void fn_4128c4(TScene *s, TGraphicSprite *unused)
{
    fn_415600(s);
}

extern "C" void fn_4128d4(TScene *s, TGraphicSprite *spr)
{
    fn_412cfc(s);
    fn_4176bc(s);
    fn_4077c4(spr, 3, 1000, fn_4128c4);
}

extern "C" void fn_412904(TScene *s, unsigned short key)
{
    fn_418044(s);
    if (g_458868) {
        g_458868 = 0;
        return;
    }
    if (key == 0x1b) {
        fn_40bf2c(s->stage, "Exit");
        return;
    }
    if ((key == ' ' || key == 0xd) && g_aiming) {
        fn_412818(s);
        fn_4177f0(s);
    }
}

extern "C" void fn_41296c(TScene *s, void *sender, unsigned short &key, ShiftState shift)
{
    if (shift.Contains(2)) {      // ssCtrl
        if (key == 'X') {
            g_cheatX = true;
            g_cheatD = false;
            g_cheatS = false;
            g_cheatG = false;
            g_cheated = true;
            return;
        }
        if (key == 'D') {
            g_cheatX = false;
            g_cheatD = true;
            g_cheatG = false;
            g_cheatS = false;
            g_cheated = true;
            return;
        }
        if (key == 'S') {
            g_cheatX = false;
            g_cheatD = false;
            g_cheatG = false;
            g_cheatS = true;
            g_cheated = true;
            return;
        }
        if (key == 'G') {
            g_cheatX = false;
            g_cheatD = false;
            g_cheatS = false;
            g_cheatG = true;
            g_cheated = true;
            return;
        }
        if (key == 'N') {
            g_cheatX = false;
            g_cheatD = false;
            g_cheatS = false;
            g_cheatG = false;
            return;
        }
    } else {
        fn_412904(s, key);
    }
}

extern "C" void fn_412a74(TScene *s, unsigned short key)
{
}

extern "C" void fn_412a7c(TScene *s, void *sender, unsigned short &key, ShiftState shift)
{
    fn_412a74(s, key);
}

extern "C" void fn_412a94(TScene *s)
{
    TGraphicSprite *m;
    int i;
    TGraphicSprite *pin;
    m = fn_40a598(s, "BallMarker0");
    fn_408908(m);
    fn_406c50(m, 1);
    m = fn_40a598(s, "BallMarker1");
    fn_408908(m);
    fn_406c50(m, g_roll);
    for (i = 0; i < 10; i++) {
        pin = g_460550->items[i];
        fn_408908(pin);
        if (g_present[i])
            fn_406c50(pin, 1);
        else
            fn_406c50(pin, 0);
    }
}

extern "C" void fn_412b44(TScene *s)
{
    TGraphicSprite *m;
    m = fn_40a598(s, "BallMarker0");
    fn_408908(m);
    fn_406c50(m, 0);
    m = fn_40a598(s, "BallMarker1");
    fn_408908(m);
    fn_406c50(m, 0);
    fn_409790(g_460550, 0, 10);
    fn_4097e8(g_460550, 0, 10, 0);
}

extern "C" void fn_412bc4(TScene *self)
{
    int i;
    int j;
    if (rand() % 2 < 2) {
        for (i = 0; i < 10; i++)
            g_present[i] = true;
    } else {
        for (j = 0; j < 10; j++)
            g_present[j] = rand() % 2 ? true : false;
    }
}

extern "C" int fn_412c30(TScene *self)
{
    int n;
    int i;
    n = 0;
    for (i = 0; i < 10; i++)
        if (g_present[i])
            n++;
    return n;
}

extern "C" void fn_412c60(TScene *s)
{
    fn_41208c(s, g_pinsDown);
    fn_412a94(s);
}

// ---- Reindeer head + lane sprites ----

extern "C" void fn_412c80(TScene *s)
{
    TGraphicSprite *e;
    e = fn_40a598(s, "DeerHead");
    fn_408924(e);
    e = fn_40a598(s, "Deer");
    fn_407fdc(e, 1, 0);
}

extern "C" bool fn_412cc8(TScene *self)
{
    bool hit;
    hit = false;
    if (g_deerUp && !g_deerHit && g_aimCell == 0) {
        g_deerHit = true;
        hit = true;
    }
    return hit;
}

extern "C" void fn_412cfc(TScene *s)
{
    TGraphicSprite *e;
    if (g_deerUp) {
        e = fn_40a598(s, "DeerHead");
        fn_406c50(e, 1);
    }
}

extern "C" void fn_412d2c(TScene *s)
{
    TGraphicSprite *e;
    if (g_deerUp) {
        e = fn_40a598(s, "DeerHead");
        fn_406c50(e, 0);
    }
}

extern "C" void fn_412d5c(TScene *s, TGraphicSprite *unused)
{
    g_deerUp = true;
    fn_412cfc(s);
}

extern "C" void fn_412d74(TScene *s, TGraphicSprite *spr)
{
    fn_4077c4(spr, 0, 400, fn_412d5c);
}

extern "C" void fn_412d90(TScene *s)
{
    TGraphicSprite *e;
    if (!g_deerUp) {
        e = fn_40a598(s, "Deer");
        fn_406c00(e);
        fn_408908(e);
        fn_407fdc(e, 0, fn_412d74);
        e = fn_40a598(s, "DeerHead");
        fn_406c50(e, 0);
        fn_408908(e);
        g_deerHit = false;
    }
}

extern "C" void fn_412e08(TScene *s)
{
    TGraphicSprite *e;
    e = fn_40a598(s, "Deer");
    fn_406c00(e);
    e = fn_40a598(s, "DeerHead");
    fn_406c50(e, 0);
    g_deerFrame = rand() % 5 + 1;
    g_deerHit = false;
    g_deerUp = false;
}

extern "C" void fn_412e70(int i, int frame)
{
    TGraphicSprite *spr;
    spr = g_460540->items[i];
    fn_406c50(spr, frame);
    spr->animate = false;
    spr = g_46054c->items[i];
    fn_406c50(spr, frame);
    spr->animate = false;
}

extern "C" void fn_412ec8(int i, int a, int b, int c)
{
    TGraphicSprite *spr;
    spr = g_460538->items[i];
    fn_407858(spr, a, b, c);
    spr = g_460544->items[i];
    fn_407858(spr, a, b, c);
}

extern "C" void fn_412f18(int i, int a, int b, int c)
{
    TGraphicSprite *spr;
    spr = g_460540->items[i];
    fn_407858(spr, a, b, c);
    spr = g_46054c->items[i];
    fn_407858(spr, a, b, c);
}

extern "C" void fn_412f68(int i, int a, int b, int c)
{
    TGraphicSprite *spr;
    spr = g_46053c->items[i];
    fn_407a68(spr, Classes::Rect(0, 0, 640, 480));
    fn_407858(spr, a, b, c);
    spr = g_460548->items[i];
    fn_407858(spr, a, b, c);
}

extern "C" void fn_412fdc(int i)
{
    TGraphicSprite *spr;
    spr = g_46053c->items[i];
    fn_407a68(spr, Classes::Rect(0, 0, 640, 480));
    spr->animate = false;
    spr = g_460538->items[i];
    spr->animate = false;
    spr = g_460540->items[i];
    spr->animate = false;
    spr = g_460548->items[i];
    spr->animate = false;
    spr = g_460544->items[i];
    spr->animate = false;
    spr = g_46054c->items[i];
    spr->animate = false;
}

extern "C" void fn_4130a4(int i)
{
    TGraphicSprite *spr;
    spr = g_46053c->items[i];
    fn_407a68(spr, Classes::Rect(0, 0, 640, 480));
    fn_4078a8(spr);
    spr = g_460538->items[i];
    fn_4078a8(spr);
    spr = g_460540->items[i];
    fn_4078a8(spr);
    spr = g_460548->items[i];
    fn_4078a8(spr);
    spr = g_460544->items[i];
    fn_4078a8(spr);
    spr = g_46054c->items[i];
    fn_4078a8(spr);
}

extern "C" void fn_413168(int i)
{
    TGraphicSprite *spr;
    TPoint p;
    TRect r;
    spr = g_46053c->items[i];
    r = spr->clip;
    p = spr->GetPos();
    p.y = r.bottom - 1;
    fn_406a80(spr, p);
    spr = g_460548->items[i];
    r = spr->clip;
    p = spr->GetPos();
    p.y = r.bottom - 1;
    fn_406a80(spr, p);
}

extern "C" void fn_413244(int i, TPoint d)
{
    TGraphicSprite *spr;
    TPoint p;
    spr = g_46053c->items[i];
    fn_407a68(spr, Classes::Rect(0, 0, 640, 480));
    fn_407924(spr, d, 30, 0, 0, 2000);
    p = spr->GetPos();
    TRect r = Classes::Rect(-1000, -1000, 1000, p.y + 1);
    spr->SetClip(r);
    spr = g_460548->items[i];
    d.x /= 4;
    d.y /= 4;
    fn_407924(spr, d, 30, 0, 0, 500);
    p = spr->GetPos();
    r = Classes::Rect(-1000, -1000, 1000, p.y + 1);
    spr->SetClip(r);
}

extern "C" void fn_4133cc(int i)
{
    TGraphicSprite *spr;
    spr = g_46053c->items[i];
    fn_407a68(spr, Classes::Rect(0, 0, 640, 480));
    fn_406c00(spr);
    spr = g_460538->items[i];
    fn_406c00(spr);
    spr = g_460540->items[i];
    fn_406c00(spr);
    spr = g_460548->items[i];
    fn_406c00(spr);
    spr = g_460544->items[i];
    fn_406c00(spr);
    spr = g_46054c->items[i];
    fn_406c00(spr);
}

extern "C" void fn_413490(int i, int frame)
{
    TGraphicSprite *spr;
    spr = g_460538->items[i];
    fn_406c50(spr, frame);
    spr = g_460544->items[i];
    fn_406c50(spr, frame);
}

extern "C" void fn_4134d4(int i, int frame)
{
    TGraphicSprite *spr;
    spr = g_46053c->items[i];
    fn_407a68(spr, Classes::Rect(0, 0, 640, 480));
    fn_406c50(spr, frame);
    spr = g_460548->items[i];
    fn_406c50(spr, frame);
}

extern "C" void fn_41353c(int i, int a, SpriteCb done)
{
    TGraphicSprite *spr;
    spr = g_46053c->items[i];
    fn_407a68(spr, Classes::Rect(0, 0, 640, 480));
    fn_407fdc(spr, a, done);
    spr = g_460548->items[i];
    fn_407fdc(spr, a, 0);
}

extern "C" void fn_4135ac(int i, int a, SpriteCb done)
{
    TGraphicSprite *spr;
    spr = g_460540->items[i];
    fn_407fdc(spr, a, done);
    spr = g_46054c->items[i];
    fn_407fdc(spr, a, 0);
}

extern "C" void fn_4135f4(int i)
{
    TGraphicSprite *spr;
    spr = g_460538->items[i];
    fn_40806c(spr);
    spr = g_460544->items[i];
    fn_40806c(spr);
}

extern "C" void fn_41362c(int i)
{
    TGraphicSprite *spr;
    spr = g_46053c->items[i];
    fn_407a68(spr, Classes::Rect(0, 0, 640, 480));
    fn_40806c(spr);
    spr = g_460548->items[i];
    fn_40806c(spr);
}

extern "C" void fn_41368c(int i)
{
    TGraphicSprite *spr;
    spr = g_460540->items[i];
    fn_40806c(spr);
    spr = g_46054c->items[i];
    fn_40806c(spr);
}

extern "C" void fn_4136c4(TScene *s, TGraphicSprite *e)
{
    fn_4077c4(e, 0, rand() % 2000 + 1000, fn_413788);
    fn_413490(e->lane, 0);
    e = g_460538->items[e->lane];
    fn_40779c(e);
}

extern "C" void fn_413720(TScene *s, TGraphicSprite *e)
{
    fn_412e70(e->lane, 2);
    fn_4077c4(e, 0, 300, fn_4136c4);
    fn_413490(e->lane, 1);
    e = g_460538->items[e->lane];
    fn_407774(e, 250);
}

extern "C" void fn_413788(TScene *s, TGraphicSprite *e)
{
    fn_412e70(e->lane, 3);
    fn_4077c4(e, 0, 1500, fn_413720);
    e = g_460538->items[e->lane];
}

extern "C" void fn_4137d0(TScene *s, TGraphicSprite *e)
{
    fn_412f18(e->lane, 400, g_4606cc * 2 + 8, 2);
}

extern "C" void fn_4137f8(TScene *s, TGraphicSprite *e)
{
    fn_40d100(s->stage->sound, s, g_460538->items[e->lane], "ElvesLaugh.wav", 0x2f, 100, 0);
}

extern "C" void fn_413834(TScene *s)
{
    if (g_4606d4 != -2 && g_roll == 0 && !g_460582[g_4606d4]) {
        if (rand() % 2 == 0) {
            g_460582[g_4606d4] = true;
            g_pinsDown--;
            fn_41353c(g_4606d4, 4, fn_4137f8);
        }
        g_4606d4 = -2;
    }
}

extern "C" void fn_4138a0(TScene *s, TGraphicSprite *e)
{
    fn_412e70(e->lane, 0);
    fn_4134d4(e->lane, 3);
    fn_4077c4(e, 0, rand() % 600 + 1000, fn_413a48);
    g_4606f0++;
}

extern "C" void fn_4138f8(TScene *s, TGraphicSprite *e)
{
    if (g_4606f0 == 0 && e->lane == 9)
        fn_40d100(s->stage->sound, s, g_460538->items[e->lane], "ElvesLaugh.wav", 20, 100, 0);
    fn_412e70(e->lane, 14);
    fn_4134d4(e->lane, 3);
    if (fn_4024bc(0) >= g_4606f4) {
        fn_40cb34(s->stage->sound, s, e, "WhosyrDaddy.wav", 30, 0);
        g_4606f4 = fn_4024bc(rand() % 1500 + 3000);
    } else {
        fn_40cb34(s->stage->sound, s, e, "SlapAss.wav", 5, 0);
    }
    fn_4077c4(e, 0, 150, fn_4138a0);
}

extern "C" void fn_413a04(TScene *s, TGraphicSprite *e)
{
    fn_412e70(e->lane, 0);
    fn_4134d4(e->lane, 3);
    fn_4077c4(e, 0, 150, fn_4138f8);
}

extern "C" void fn_413a48(TScene *s, TGraphicSprite *e)
{
    fn_412e70(e->lane, 14);
    fn_4134d4(e->lane, 3);
    fn_4077c4(e, 0, 150, fn_413a04);
}

extern "C" void fn_413a8c(TScene *s, TGraphicSprite *e)
{
    g_4606f4 = fn_4024bc(rand() % 1500 + 3000);
    g_4606f0 = 0;
    fn_412e70(e->lane, 14);
    fn_4134d4(e->lane, 2);
    fn_4077c4(e, 0, 700, fn_413a48);
}

extern "C" void fn_413af4(TScene *s, TGraphicSprite *e)
{
    fn_412e70(e->lane, 0);
    fn_413490(e->lane, 3);
    fn_4134d4(e->lane, 2);
    fn_4077c4(e, 0, 700, fn_413a8c);
}

extern "C" void fn_413b4c(TScene *s, TGraphicSprite *e)
{
    if (e->lane == 9)
        fn_40d100(s->stage->sound, s, g_460538->items[e->lane], "HeySanta.wav", 30, 100, 0);
    fn_4077c4(e, 0, 1000, fn_413af4);
}

extern "C" void fn_413bac(TScene *s, TGraphicSprite *e)
{
    if (e->lane != g_4606dc)
        fn_412e70(e->lane, 15);
}

extern "C" void fn_413bd8(TScene *s, TGraphicSprite *e)
{
    fn_40779c(g_460538->items[e->lane]);
    fn_4077c4(e, 0, rand() % 1200 + 250, fn_413bac);
}

extern "C" void fn_413c1c(TScene *s, TGraphicSprite *e)
{
    if (e->lane == g_4606e0)
        fn_40d100(s->stage->sound, s, g_460538->items[e->lane], "ElvesLaugh.wav", 20, 100, 0);
    else
        fn_407774(g_460538->items[e->lane], 100);
    fn_4077c4(e, 0, 560, fn_413bd8);
}

extern "C" void fn_413ca0(TScene *s, TGraphicSprite *e)
{
    if (e->lane == g_4606e0)
        fn_40d100(s->stage->sound, s, g_460538->items[e->lane], "ElliotFarted.wav", 30, 100, 0);
    fn_4077c4(e, 0, 1000, fn_413c1c);
}

extern "C" void fn_413d08(TScene *s, TGraphicSprite *e)
{
    fn_413490(e->lane, 6);
}

extern "C" void fn_413d20(TScene *s, TGraphicSprite *e)
{
    fn_413490(e->lane, 2);
    fn_41353c(e->lane, 6, 0);
    fn_4077c4(e, 0, 1000, fn_413d08);
}

extern "C" void fn_413d64(TScene *s, TGraphicSprite *e)
{
    if (e->lane == g_4606dc) {
        fn_40cb34(s->stage->sound, s, e, "fart.wav", 0x20, 0);
        fn_413490(e->lane, 7);
        fn_4077c4(e, 0, 450, fn_413d20);
        return;
    }
    fn_4077c4(e, 0, 1200, fn_413ca0);
}

extern "C" void fn_413de4(TScene *s, TGraphicSprite *e)
{
    fn_41353c(e->lane, 5, 0);
    fn_4135ac(e->lane, 0, fn_413e40);
}

extern "C" void fn_413e18(TScene *s, TGraphicSprite *e)
{
    TGraphicSprite *p;
    p = g_460538->items[e->lane];
    fn_40779c(p);
}

extern "C" void fn_413e40(TScene *s, TGraphicSprite *e)
{
    TGraphicSprite *p;
    p = g_460538->items[e->lane];
    if (p->lane == 9) {
        fn_40d100(s->stage->sound, s, p, "ElfBaby.wav", 30, 100, 0);
    } else {
        fn_407774(p, 0);
        fn_4077c4(e, 0, 1000, fn_413e18);
    }
    fn_41353c(e->lane, 5, 0);
    fn_4135ac(e->lane, 0, fn_413de4);
}

extern "C" void fn_413ee4(TScene *s, TGraphicSprite *e)
{
    TGraphicSprite *p;
    p = g_460538->items[e->lane];
    fn_40779c(p);
    fn_4077c4(e, 0, 0x79e, fn_413ee4);
}

extern "C" void fn_413f20(TScene *s, TGraphicSprite *e)
{
    TGraphicSprite *p;
    int i;
    p = g_460538->items[e->lane];
    if (p->lane == 9) {
        fn_40d100(s->stage->sound, s, p, "Fewer.wav", 30, 100, 0);
        fn_4077c4(e, 0, 0x8fc, fn_413f20);
        for (i = 0; i < 9; i++) {
            e = g_460540->items[i];
            if (g_pins[e->lane].state != 2) {
                p = g_460538->items[e->lane];
                fn_407774(p, 0);
                fn_4077c4(e, 0, 0x73a, fn_413ee4);
            }
        }
    }
}

extern "C" void fn_413ffc(int i)
{
    fn_412e70(i, 0);
    fn_412ec8(i, 170, 4, 2);
}

extern "C" void fn_414024(TScene *s, int i)
{
    PinState *p;
    if (g_roll == 0) {
        p = &g_pins[i];
        if (p->state == 0) {
            fn_412e70(i, 0);
        } else if (p->state == 1) {
            fn_412e70(i, 2);
            fn_4077c4(g_460540->items[i], 0, rand() % 2000 + 1000, fn_413788);
        } else if (p->state == 2) {
            fn_412e70(i, 0);
            fn_4077c4(g_460540->items[i], 0, 400, fn_4137d0);
        } else if (p->state == 3) {
            fn_412e70(i, 0);
            fn_4077c4(g_460540->items[i], 0, 600, fn_413b4c);
        } else if (p->state == 4) {
            fn_412e70(i, 0);
            fn_4077c4(g_460540->items[i], 0, 600, fn_413e40);
        } else if (p->state == 5) {
            fn_412e70(i, 0);
            fn_4077c4(g_460540->items[i], 0, 600, fn_413f20);
        } else if (p->state == 6) {
        } else if (p->state == 7) {
            fn_412e70(i, 0);
            fn_4077c4(g_460540->items[i], 0, 600, fn_413d64);
        } else if (p->state == 8) {
        }
    }
}

extern "C" void fn_4141d0(TScene *s, int i)
{
    TGraphicSprite *p;
    fn_4135f4(i);
    fn_41362c(i);
    fn_41368c(i);
    fn_412e70(i, 0);
    p = g_460540->items[i];
    fn_407820(p, 0);
    p = g_460538->items[i];
    fn_407820(p, 0);
    fn_40779c(p);
}
