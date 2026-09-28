// TStage (TForm) key event handlers, 0x40bd3c-0x40bdec (__fastcall).
// TShiftState is a 1-byte VCL Set<>; Contains() is inline.
#include <elf/funcs.h>

extern "C" void __fastcall fn_40bd3c(TStage *e, void *sender, unsigned short &key, ShiftState shift)
{
    if (key == 13 && shift.Contains(0))
        return;
    if (!e->paused) {
        TScene *sc = e->scene;
        if (sc && sc->onKeyDown)
            sc->onKeyDown(sc, sender, key, shift);
    }
}

extern "C" void __fastcall fn_40bda4(TStage *e, void *sender, unsigned short &key, ShiftState shift)
{
    TScene *sc = e->scene;
    if (sc && sc->onKeyUp)
        sc->onKeyUp(sc, sender, key, shift);
}
