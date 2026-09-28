// TStage deleting destructor, 0x40a948 (written as a free function).
#include <elf/funcs.h>

extern "C" void fn_40a948(TStage *e, int flags)
{
    if (e) {
        if (e->fullscreen)
            fn_401e20();
        fn_403768();
        if (flags & 1)
            fn_448a2c(e);
    }
}
