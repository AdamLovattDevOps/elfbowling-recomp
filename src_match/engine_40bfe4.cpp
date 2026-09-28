// TStage: activate (load parms, size buffers, start first scene), 0x40bfe4.
#include <elf/funcs.h>

extern "C" TScene *fn_40bfe4(TStage *e, const char *first)
{
    TScene *r = 0;
    TSaveParms *sp = new TSaveParms();
    char ok = fn_40e55c(sp, e);
    if (ok)
        g_455528 = sp;
    if (!ok)
        fn_401f0c("Unable to read stage parameters.\n");
    fn_403574(&e->back, fn_434238(e->form), fn_43427c(e->form), e->bpp, e->bgcolor);
    fn_403574(&e->front, 0x78, 0x78, e->bpp, e->bgcolor);
    DeleteDC(e->dc);
    e->dc = 0;
    if (first)
        r = fn_40bf2c(e, first);
    fn_4022fc("Activation of Stage");
    e->active = 1;
    return r;
}
