// Packed resource file (TPackedResources) and zlib alloc hooks, 0x40d3f8-0x40d578
// (fn_40d250 is a member: packres_40d250.cpp).
#include <elf/funcs.h>
#include <stdlib.h>

extern "C" int fn_40d3f8(TPackedResources *pr, const char *name)
{
    int idx = -1;
    for (int i = 0; idx == -1 && i < pr->count; i++) {
        if (rtl_stricmp(pr->entries[i].name, name) == 0)
            idx = i;
    }
    if (idx == -1)
        fn_401f30("Could not find resource ", name);
    return idx;
}

extern "C" void *fn_40d464(void *opaque, unsigned items, unsigned size)
{
    void *p = malloc(items * size);
    return p == 0 ? 0 : p;
}

extern "C" void fn_40d488(void *opaque, void *p)
{
    free(p);
}

extern "C" void fn_40d498(ZStream *z)
{
    z->zalloc = fn_40d464;
    z->zfree = fn_40d488;
    z->opaque = 0;
}

extern "C" int fn_40d568(TPackedResources *a, void *s)
{
    return fn_40e19c(s);
}
