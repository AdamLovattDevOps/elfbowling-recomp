// Game unit: rolling-ball tick (0x417074).
#include <elf/funcs.h>

// Alias of fn_401d0c: three TPoints by value (see game_416bd8.cpp).
extern "C" int fn_401d0c_pt(TPoint p, TPoint a, TPoint b);

extern "C" void fn_417074(TScene *self, TGraphicSprite *s)
{
    bool done = false;
    TPoint pos = Classes::Point(s->x / 1000, s->y / 1000);
    int frame = s->frame;
    int pct = (500 - pos.y) * 100 / 288;
    if (g_gutterLeft) {
        pos.x = fn_401d0c_pt(pos, Classes::Point(0, 480), Classes::Point(126, 220));
        fn_406a80(s, pos);
    } else if (g_gutterRight) {
        pos.x = fn_401d0c_pt(pos, Classes::Point(320, 480), Classes::Point(194, 220));
        fn_406a80(s, pos);
    } else {
        int off = g_ballOffset * pct / 60;
        pos.x = off + 160;
        if (pct <= 96) {
            int lim = fn_401d0c_pt(pos, Classes::Point(24, 480), Classes::Point(130, 220));
            if (pos.x <= lim) {
                g_gutterLeft = 1;
                g_gutterBall = 1;
                if (fn_412cc8(self)) {
                    fn_416fac(self, s);
                    done = true;
                } else {
                    pos.x = fn_401d0c_pt(pos, Classes::Point(0, 480), Classes::Point(126, 220));
                    fn_40cb34(self->stage->sound, self, s, "BowlDrop.wav", 6, 0);
                }
            } else {
                lim = fn_401d0c_pt(pos, Classes::Point(296, 480), Classes::Point(190, 220));
                if (pos.x >= lim) {
                    g_gutterRight = 1;
                    g_gutterBall = 1;
                    pos.x = fn_401d0c_pt(pos, Classes::Point(320, 480), Classes::Point(194, 220));
                    fn_40cb34(self->stage->sound, self, s, "BowlDrop.wav", 6, 0);
                }
            }
        }
        if (!done)
            fn_406a80(s, pos);
    }
    if (!done) {
        int speed = 20;
        if (pos.y <= 330) {
            fn_412d2c(self);
            fn_413834(self);
            fn_4146b8(self);
        }
        if (pos.y <= 266) {
            if (fn_416bd8(self, pos)) {
            }
        }
        TPoint v = Classes::Point(s->vx / 1000, s->vy / 1000);
        TGraphicSprite *t = g_46056c->items[g_ballRow];
        if (t != s) {
            fn_408924(s);
            fn_4078a8(s);
            s = t;
            fn_406c50(s, frame);
            fn_408908(s);
            fn_406a80(s, pos);
        }
        int i;
        for (i = frame; g_4590f4[i] <= pct; i++)
            ;
        if (i != frame)
            fn_406c50(s, i);
        v.y = -(16 - i);
        s->clip = Classes::Rect(0, 212, 320, 502);
        fn_407a44(s, v, speed, 0, 0);
        s->onCell = fn_417074;
        s->onLeave = fn_416b00;
        fn_416288(self, s);
    }
}
