// TStage scene list and dirty-area list helpers, 0x40ae30-0x40ae80.
#include <elf/funcs.h>

extern "C" void fn_40ae30(TStage *e, TScene *p)
{
    if (e->nscenes < 20)
        e->scenes[e->nscenes++] = p;
}

extern "C" void fn_40ae60(TStage *st)
{
    st->nareas = 0;
}

extern "C" void fn_40ae70(TStage *st)
{
    st->nareas2 = 0;
}
