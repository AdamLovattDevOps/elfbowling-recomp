// TPackedResources: lazily load the saved-cast table, 0x40dc1c.
#include <elf/funcs.h>

extern "C" char fn_40dc1c(TPackedResources *pr, void *arg)
{
    char ok = 1;
    if (pr->casts == 0) {
        pr->casts = new TSaveCasts();
        ok = fn_40e354(pr->casts, arg);
        if (!ok) {
            delete pr->casts;
            pr->casts = 0;
        }
    }
    return ok;
}
