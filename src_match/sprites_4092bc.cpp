// TButtonSprite update (0x4092bc): clicks, blinking, hover.
#include <elf/funcs.h>

extern "C" void fn_4092bc(TButtonSprite *b, TStage *e, int now, char *clickL, char *clickR, char *unused)
{
    TGraphicCast *c = b->casts[b->frame];
    ERect r = ELF_AS(ERect, c->bounds);
    if (fn_407040(b, &r)) {
        if (*clickL && fn_4027d0(r, e->down)) {
            e->down.x = -1;
            e->down.y = -1;
            if (b->enabled && b->onPress) {
                fn_409234(b);
                *clickL = 0;
                b->onPress(e->scene, b);
            }
        }
        if (*clickR && fn_4027d0(r, e->up)) {
            e->up.x = -1;
            e->up.y = -1;
            if (b->enabled && b->onRelease) {
                fn_409234(b);
                *clickR = 0;
                b->onRelease(e->scene, b);
            }
        }
        if (b->blink && now > b->nextTime) {
            b->lit = (b->lit + 1) % 2;
            if (b->lit == 1) {
                TSoundMgr *snd = e->sound;
                if (b->blinkSnd >= 0)
                    fn_40cae0(snd, 0, b, b->blinkSnd, b->blinkSndArg, 0);
            }
            int d = b->onTime;
            if (b->lit == 0) {
                if (++b->blinkN >= b->blinkCount) {
                    d = b->offTime;
                    b->blinkN = 0;
                }
            }
            b->nextTime = fn_4024cc(b->nextTime, d);
            fn_406c50(b, b->lit | b->hover);
        }
        if (b->hover == 0) {
            if (b->enabled && fn_4027d0(r, e->mouse)) {
                fn_4091ac(b);
                fn_40ae80(e, r);
                b->hover = 1;
                fn_406c50(b, b->lit | b->hover);
                fn_40ae80(e, r);
                if (b->onEnter)
                    b->onEnter(e->scene, b);
            }
        } else if (b->hover == 1) {
            if (!fn_4027d0(r, e->mouse)) {
                fn_40ae80(e, r);
                b->hover = 0;
                fn_406c50(b, b->lit | b->hover);
                fn_40ae80(e, r);
                if (b->onExit)
                    b->onExit(e->scene, b);
            }
        }
    }
}
