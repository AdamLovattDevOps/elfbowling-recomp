// TScene: update which buttons are covered by sprites, 0x409c50.
#include <elf/funcs.h>

// Alias: ERect (copy-ctor) by value where funcs.h has RECT. See docs/HEADERS.md.
extern "C" char fn_40252c_E(ERect a, ERect b);

extern "C" void fn_409c50(TScene *sc)
{
    for (int i = 0; i < sc->nbuttons; i++) {
        TSceneButton *b = &sc->buttons[i];
        if (b->visible) {
            char hit = 0;
            for (int j = b->first; j < sc->nsprites; j++) {
                TGraphicSprite *s = sc->sprites[j];
                if (s->shown) {
                    ERect *p = (ERect *)&s->casts[s->frame]->bounds;
                    ERect r;
                    r = *p;
                    fn_40717c(s, &r);
                    if (fn_40252c_E(ELF_AS(ERect, b->rect), r)) {
                        fn_434b6c(b->ctl);
                        fn_40ae80(sc->stage, ELF_AS(ERect, b->rect));
                        hit = 1;
                        break;
                    }
                }
            }
            if (!hit) {
                fn_434b74(b->ctl);
                TSceneButton *bb = b;
                bb->ctl->f120 = 0;
#ifdef __BORLANDC__
                // Delphi TControl VMT slot 0x7c: 32-bit slot numbering with no
                // native meaning. Dead code (TScene::nbuttons is never raised
                // above 0), so the native build leaves it out.
                bb->ctl->vt[0x7c / 4](bb->ctl);
#endif
            }
        }
    }
}
