// TPackedResources: find, load and lock the NVDPACKFILE resource, 0x40d2c8.
#include <elf/funcs.h>
#include <string.h>

extern "C" char fn_40d2c8(TPackedResources *pr)
{
    char ok = 0;
    HRSRC r = FindResourceA(*g_45fee0, pr->name, "NVDPACKFILE");
    if (r) {
        pr->size = SizeofResource(*g_45fee0, r);
        pr->hres = LoadResource(*g_45fee0, r);
        if (pr->hres) {
            pr->hdr = (PackHdr *)LockResource(pr->hres);
            fn_40d250(pr);
            pr->insize = 0x2000;
            pr->inbuf = (char *)fn_402338(pr->insize, "LoadResources");
            if (pr->inbuf) {
                pr->outsize = 0x1000;
                pr->outbuf = (char *)fn_402338(pr->outsize + 1, "LoadResources");
                if (pr->outbuf) {
                    pr->loaded = (char *)fn_402338(pr->count, "LoadResources");
                    if (pr->loaded) {
                        memset(pr->loaded, 0, pr->count);
                        ok = 1;
                    }
                }
            }
        }
    }
    if (!ok)
        fn_401f0c("Error Loading Packed Resources");
    return ok;
}
