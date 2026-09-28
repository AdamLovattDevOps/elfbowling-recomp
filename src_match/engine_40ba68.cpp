// TStage: redraw off-stage update areas directly or via the front buffer, 0x40ba68 (member).
#include <elf/funcs.h>

// Alias: ERect (copy-ctor) return where funcs.h has RectPod. See docs/HEADERS.md.
extern "C" ERect fn_40a5d4_E(TStage *e, ERect &r);

void TStage::fn_40ba68()
{
    ERect s;
    if (fullscreen) {
        for (int i = 0; i < nareas2; i++) {
            ERect r = ELF_AS(ERect, offstage[i]);
            int w = r.r - r.l;
            int h = r.b - r.t;
            char big = w > 120 || h > 120;
            s = fn_40a5d4_E(this, r);
            fn_40b5bc(this, big, &s);
            TScene *sc = scene;
            for (int j = 0; j < sc->nsprites; j++) {
                TGraphicSprite *sp = sc->sprites[j];
                if ((sp->type == 1 || sp->type == 3 || sp->type == 2) && sp->shown && sp->f1d0) {
                    s = ELF_AS(ERect, sp->casts[sp->frame]->bounds);
                    if (fn_407040(sp, &s) && fn_402654(&s, r)) {
                        ERect t;
                        t = s;
                        fn_407690(sp, &t);
                        s = fn_40a5d4_E(this, s);
                        fn_40b3b0(this, big, (RectPod *)&s, sp->casts[sp->frame], (TPoint *)&t);
                    }
                }
                if (dirtyFlag2) {
                    s = r;
                    int w2 = s.r - s.l;
                    int h2 = s.b - s.t;
                    ERect q = fn_43a8dc(0, 0, w2, h2);
                    fn_40b8fc(s, front.handle, ELF_AS(TPoint, q));
                    dirtyFlag2 = 0;
                }
            }
        }
        nareas2 = 0;
    }
}
// MATCH 40ba68 @TStage@fn_40ba68$qv
