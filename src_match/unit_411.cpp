// Exit key and score board helpers, 0x411bac-0x4120cc.
#include <stdlib.h>
#include <elf/funcs.h>

extern "C" void fn_411bac(TScene *g, TGraphicSprite *k)
{
    if (k->enabled && k->shown)
        fn_40bf2c(g->stage, "Exit");
}

// ---- Exit / score screen helpers 0x411af4-0x4120cc ----

extern "C" void fn_411bdc(TScene *s)
{
    fn_409754(g_460558, 0);
    fn_4097e8(g_460558, min(g_460598, 9), 1, 1);
}

extern "C" void fn_411c24(TScene *s, int frame)
{
    g_460598 = frame;
    fn_411bdc(s);
}

extern "C" void fn_411c3c(TScene *s, int score)
{
    g_46059c = score;
}

extern "C" void fn_411c4c(TScene *s)
{
    int last;
    int i;
    FrameScore *f;
    last = 0;
    for (i = 0; i < 12; i++) {
        f = &g_4605c8[i];
        if (f->total == -1)
            break;
        last = f->total;
    }
    fn_411c3c(s, last);
}

// Redraw the score board: rolls not yet drawn, then any frame total that can now be scored.
extern "C" void fn_411c98(TScene *s)
{
    int total;
    int i;
    FrameScore *f;
    TGraphicSprite *spr;
    char buf[8];
    FrameScore *n1;
    FrameScore *n2;
    total = 0;
    for (i = 0; i < 12; i++) {
        f = &g_4605c8[i];
        if (f->roll[0] >= 0 && !f->shown0) {
            if (i < 10)
                spr = g_460560->items[i];
            else if (i == 10 && g_4605c8[i - 1].roll[0] == 10)
                spr = g_460564->items[i - 1];
            else
                spr = fn_40a598(s, "ScoreMarkC09");
            if (f->roll[0] == 10) {
                if (i < 9)
                    spr = g_460564->items[i];
                buf[0] = 'X';
            } else if (f->roll[0] == 0)
                buf[0] = '-';
            else
                buf[0] = f->roll[0] + '0';
            buf[1] = 0;
            fn_405954((TTextCast *)spr->casts[spr->frame]);
            fn_406a08(spr, buf);
            f->shown0 = 1;
        }
        if (f->roll[1] >= 0 && !f->shown1) {
            if (i < 10)
                spr = g_460564->items[i];
            else
                spr = fn_40a598(s, "ScoreMarkC09");
            if (f->roll[0] + f->roll[1] == 10)
                buf[0] = i < 10 ? '/' : f->roll[1] + '0';
            else if (f->roll[1] == 0)
                buf[0] = '-';
            else
                buf[0] = f->roll[1] + '0';
            buf[1] = 0;
            fn_405954((TTextCast *)spr->casts[spr->frame]);
            fn_406a08(spr, buf);
            f->shown1 = 1;
        }
        if (i < 10 && f->total == -1) {
            total = i == 0 ? 0 : g_4605c8[i - 1].total;
            spr = g_46055c->items[i];
            n1 = &g_4605c8[i + 1];
            n2 = &g_4605c8[i + 2];
            if (f->roll[0] == 10) {
                if (n1->roll[0] == 10 && n2->roll[0] >= 0) {
                    total += f->roll[0];
                    total = n1->roll[0] + total + n2->roll[0];
                    fn_401e48(buf, total, 3, 1);
                    fn_405954((TTextCast *)spr->casts[spr->frame]);
                    fn_406a08(spr, buf);
                    f->total = total;
                } else if (n1->roll[0] >= 0 && n1->roll[1] >= 0) {
                    total += f->roll[0];
                    total = n1->roll[0] + total + n1->roll[1];
                    fn_401e48(buf, total, 3, 1);
                    fn_405954((TTextCast *)spr->casts[spr->frame]);
                    fn_406a08(spr, buf);
                    f->total = total;
                }
            } else if (f->roll[0] + f->roll[1] == 10) {
                if (n1->roll[0] >= 0) {
                    total = total + f->roll[0] + f->roll[1];
                    total += n1->roll[0];
                    fn_401e48(buf, total, 3, 1);
                    fn_405954((TTextCast *)spr->casts[spr->frame]);
                    fn_406a08(spr, buf);
                    f->total = total;
                }
            } else if (f->roll[0] >= 0 && f->roll[1] >= 0) {
                total = total + f->roll[0] + f->roll[1];
                fn_401e48(buf, total, 3, 1);
                fn_405954((TTextCast *)spr->casts[spr->frame]);
                fn_406a08(spr, buf);
                f->total = total;
            }
        }
    }
    fn_411c4c(s);
}

extern "C" void fn_41208c(TScene *s, int pins)
{
    ((int *)&g_4605c8[g_460598])[g_4605bc] = pins;
    fn_411c98(s);
}

extern "C" void fn_4120b4(TScene *s)
{
    fn_411c24(s, g_460598);
}
