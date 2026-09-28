// TPackedResources: load a bitmap resource (BMP headers, optional palette, pixels), 0x40d980.
#include <elf/funcs.h>
#ifdef PORT
#include "../port/src/hires.h"   // 3x art hooks (native only; no bytes under bcc32)
#endif

extern "C" HBITMAP fn_40d980(TPackedResources *pr, TGraphicCast *b, const char *name, const char *res, void *pal, char swap, char flip)
{
    TStreamVmt *st = (TStreamVmt *)fn_40d4bc(g_455524, res);
    if (st) {
        CastEntry *ce = 0;
        if (pr->casts) {
            ce = fn_40e304(pr->casts, name);
            if (!ce)
                fn_401f30("No Cast Parm entry found for ", name);
        }
        if (!ce)
            g_45553c = 1;
        int got = st->vt[1](st, pr->inbuf, 0x36);
        if (got == 0x36) {
            BmpFileHdr *h = (BmpFileHdr *)(pr->inbuf - 2);
            if (h->bi.biBitCount == 8 && pal)
                got += fn_402030(st, (HPALETTE *)pal);
            DWORD off = h->offBits;
            DWORD skip = off - got;
            if (skip > 0) {
                char *tmp = pr->inbuf + 0x36;
                st->vt[1](st, tmp, skip);
            }
            if (!ce)
                fn_40d578(pr, st, b, h, name, swap, flip);
            else
                fn_40d6f0(pr, ce, st, b, h, name, swap, flip);
        }
        fn_40d568(g_455524, st);
#ifdef PORT
        hires_cast_loaded(HIRES_BMP(&b->bmp), res, swap, flip, b->bmp.r1c.left, b->bmp.r1c.top, b->bmp.r1c.right,
                          b->bmp.r1c.bottom, (const unsigned char *)(g_45552c + 1));
#endif
    } else {
        fn_401f30("Unable to locate bitmap ", name);
    }
    if (!b->bmp.handle)
        fn_401f30("Unable to crate a bitmap image for ", name);
    return b->bmp.handle;
}
