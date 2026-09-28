// TStage scene switching, 0x40bdec-0x40bf2c.
#include <elf/funcs.h>

extern "C" char fn_40bdec(TScene *sc)
{
    char r = 0;
    if (sc->retScene) {
        fn_40bf14(sc->stage, sc->retScene);
        r = 1;
    } else {
        fn_401f30("Unknown return scene from ", sc->name);
    }
    return r;
}

extern "C" TScene *fn_40be34(TStage *e)
{
    if (e->scene) {
        fn_409b44(e->scene);
        fn_402318("End of Scene ", e->scene->name);
        e->scene = 0;
    }
    e->scene = e->next;
    TScene *ret = e->nextRet;
    e->scene->retScene = ret;
    e->next = 0;
    e->nextRet = 0;
    fn_40b1d4(e);
    fn_409a6c(e->scene);
    fn_40b9b8(e);
    e->mousedown = 0;
    return e->scene;
}

extern "C" TScene *fn_40bef4(TStage *e, TScene *next, TScene *ret)
{
    e->next = next;
    e->nextRet = ret;
    return next;
}

extern "C" TScene *fn_40bf14(TStage *e, TScene *next)
{
    return fn_40bef4(e, next, 0);
}
