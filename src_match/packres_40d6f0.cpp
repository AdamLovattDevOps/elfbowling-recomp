// TPackedResources: read a bitmap from a stream, cropped to the cast entry's keep
// rect (falls back to fn_40d578), 0x40d6f0.
#include <elf/funcs.h>
#include <string.h>

extern "C" void fn_40d6f0(TPackedResources *pr, CastEntry *c, TStreamVmt *st, TGraphicCast *b, BmpFileHdr *h, const char *name, char swap, char flip)
{
    int w = c->keep.right - c->keep.left;
    int ht = c->keep.bottom - c->keep.top;
    int sw = h->bi.biWidth;
    int sh = h->bi.biHeight;
    int srcsize = (h->bi.biBitCount * sw / 8 + 3) / 4 * 4 * sh;
    int dstsize = (h->bi.biBitCount * w / 8 + 3) / 4 * 4 * ht;
    g_455534 += srcsize;
    g_455538 += dstsize;
    if (w == sw && ht == sh) {
        fn_40d578(pr, st, b, h, name, swap, flip);
    } else {
        h->bi.biWidth = w;
        h->bi.biHeight = ht;
        fn_402a58(&b->bmp, (DibFile *)h);
        b->bmp.r0c = c->rect;
        b->bmp.r1c = c->keep;
        int sstride = (b->bmp.bpp * sw / 8 + 3) / 4 * 4;
        int dstride = (b->bmp.bpp * w / 8 + 3) / 4 * 4;
        char *dst = flip ? b->bmp.bits + (ht - 1) * dstride : b->bmp.bits;
        int off = b->bmp.bpp * c->keep.left / 8;
        int step = flip ? -dstride : dstride;
        for (int y = 0; y < sh; y++) {
            int row = flip ? y : sh - y - 1;
            int got = st->vt[1](st, pr->inbuf, sstride);
            if (got != sstride)
                fn_401f30("Error reading bitmap image for ", name);
            if (row >= c->keep.top && row < c->keep.bottom) {
                if (swap)
                    fn_402a7c(b->bmp.bpp / 8, pr->inbuf, b->bmp.bpp * sw / 8);
                memmove(dst, pr->inbuf + off, dstride);
                dst += step;
            }
        }
    }
}
