// TPackedResources: read a bitmap's pixels from a stream (optionally flipped / byte-swapped), 0x40d578.
// Delphi virtual call: st->vt[1] (TStream::Read) compiles to mov ebx,[eax]; call [ebx+4].
#include <elf/funcs.h>
#include <string.h>

extern "C" void fn_40d578(TPackedResources *pr, TStreamVmt *st, TGraphicCast *b, BmpFileHdr *pal, const char *name, char swap, char flip)
{
    fn_402a58(&b->bmp, (DibFile *)pal);
    if (!swap && !flip) {
        int bytes = b->bmp.width * b->bmp.bpp / 8;
        int stride = (unsigned)(bytes + 3) / 4 * 4;
        int total = stride * b->bmp.height;
        int got = st->vt[1](st, b->bmp.bits, total);
        if (got != total)
            fn_401f30("Error reading bitmap image for ", name);
    } else {
        int bytes2 = b->bmp.width * b->bmp.bpp / 8;
        int stride2 = (unsigned)(bytes2 + 3) / 4 * 4;
        int step = flip ? -stride2 : stride2;
        char *dst = flip ? b->bmp.bits + (b->bmp.height - 1) * stride2 : b->bmp.bits;
        for (int y = 0; y < b->bmp.height; y++) {
            int got2 = st->vt[1](st, pr->inbuf, stride2);
            if (got2 != stride2)
                fn_401f30("Error reading bitmap image for ", name);
            if (swap)
                fn_402a7c(b->bmp.bpp / 8, pr->inbuf, bytes2);
            memmove(dst, pr->inbuf, stride2);
            dst += step;
        }
    }
}
