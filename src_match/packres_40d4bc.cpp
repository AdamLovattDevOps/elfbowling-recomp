// TPackedResources::OpenStream: new TPackedStream over a packed entry, 0x40d4bc.
// The new result goes through a named temp (t) before the return local.
#include <vcl/classes.hpp>
#include <elf/funcs.h>

extern "C" TPackedStream *fn_40d4bc(TPackedResources *pr, const char *name)
{
    TPackedStream *s = 0;
    int idx = fn_40d3f8(pr, name);
    if (idx >= 0) {
        pr->loaded[idx] = 1;
        char *p = pr->entries[idx].dataOffset + (char *)g_455524->hdr;
        TPackedStream *t = new TPackedStream(p, pr->entries[idx].csize, pr->outbuf, 0x1000, pr->entries[idx].size);
        s = t;
    }
    return s;
}
