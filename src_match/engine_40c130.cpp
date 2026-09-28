// TStage stop and frame tick, 0x40c130-0x40c1f0.
#include <elf/funcs.h>

extern "C" void fn_40c130(TStage *e)
{
    e->running = 0;
    e->active = 0;
    fn_401e20();
    if (e->onExit)
        e->onExit();
}

extern "C" void fn_40c1b4()
{
    int t = fn_4024bc(0);
    TScene *sc = g_4601c8->scene;
    if (sc) {
    }
    g_4555bc = t;
    fn_40bcb8(g_4601c8);
}
