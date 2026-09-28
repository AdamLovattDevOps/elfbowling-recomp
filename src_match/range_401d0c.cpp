// Helpers 0x401d0c-0x403540
#include <elf/funcs.h>
#ifdef PORT
#include "../port/src/hires.h"   // 3x art hooks (native only; no bytes under bcc32)
#endif

extern "C" int fn_401d0c(int unused, int x, int y0, int x0, int y1, int x1)
{
    int t = (x - x0) * 100 / (x1 - x0);
    int r = (y1 - y0) * t / 100 + y0;
    return r;
}

extern "C" void fn_401f30(const char *a, const char *b)
{
    char buf[512];
    rtl_strcpy(buf, a);
    rtl_strcat(buf, b);
    fn_401f0c(buf);
}

extern "C" void fn_401f70(const char *a, const char *b, const char *c)
{
    char buf[512];
    rtl_strcpy(buf, a);
    rtl_strcat(buf, b);
    fn_401f30(buf, c);
}

extern "C" void *fn_402338(unsigned size, const char *what)
{
    void *p = rtl_malloc(size);
    g_45554c += 2;
    if (p)
        g_455550 += size;
    fn_402194();
    if (!p)
        fn_401f30("Unable to allocate memory for ", what);
    return p;
}

extern "C" void fn_4024e8(RECT *d, RECT *s, int n)
{
    d->top = s->top / n;
    d->left = s->left / n;
    d->bottom = s->bottom / n;
    d->right = s->right / n;
}

extern "C" RECT fn_402748(RECT a, RECT b)
{
    RECT r;
    r.left = a.left + b.left;
    r.top = a.top + b.top;
    r.right = a.right + b.right;
    r.bottom = a.bottom + b.bottom;
    return r;
}

extern "C" RECT fn_40278c(RECT a, RECT b)
{
    RECT r;
    r.left = a.left - b.left;
    r.top = a.top - b.top;
    r.right = a.right - b.right;
    r.bottom = a.bottom - b.bottom;
    return r;
}

extern "C" void fn_40298c(Bitmap *b, int w, int h, int bpp)
{
    fn_40290c(b);
    b->width = w;
    b->height = h;
    b->bpp = bpp;
    b->info = g_45552c;
    fn_401fb4(b);
}

extern "C" bool fn_40252c(RECT a, RECT b)
{
    bool h = true, v = true;
    if (b.left >= a.right)
        h = false;
    else if (b.right <= a.left)
        h = false;
    if (b.top >= a.bottom)
        v = false;
    else if (b.bottom <= a.top)
        v = false;
    return h & v;
}

extern "C" bool fn_402580(RECT a, RECT b)
{
    bool h = true, v = true;
    if (b.left - a.right > 20)
        h = false;
    else if (b.right - a.left < -20)
        h = false;
    if (b.top - a.bottom > 20)
        v = false;
    else if (b.bottom - a.top < -20)
        v = false;
    return h & v;
}

extern "C" TPoint fn_4028b8(RECT r, TPoint p)
{
    if (p.x < r.left)
        p.x = r.left;
    else if (p.x >= r.right)
        p.x = r.right - 1;
    if (p.y < r.top)
        p.y = r.top;
    else if (p.y >= r.bottom)
        p.y = r.bottom - 1;
    return p;
}

extern "C" BITMAPINFOHEADER *fn_401fb4(Bitmap *b)
{
    BITMAPINFOHEADER *bi = b->info;
    bi->biWidth = b->width;
    bi->biHeight = b->height;
    bi->biPlanes = 1;
    bi->biBitCount = b->bpp;
    bi->biCompression = 0;
    bi->biSizeImage = (b->width * b->bpp / 8 + 3) / 4 * 4 * b->height;
    return bi;
}

extern "C" void fn_40290c(Bitmap *b)
{
    b->width = b->height = 0;
    b->bpp = 24;
    RECT t1 = Classes_Rect(0, 0, 0, 0);
    b->r0c = t1;
    RECT t2 = Classes_Rect(0, 0, 0, 0);
    b->r1c = t2;
    b->bits = 0;
    b->handle = 0;
    b->info = 0;
}

extern "C" void fn_402b4c(Bitmap *d, Bitmap *s)
{
    fn_4029c8(d, s->width, s->height, s->bpp);
    int size = (s->width * s->bpp / 8 + 3) / 4 * 4 * s->height;
    rtl_memmove(d->bits, s->bits, size);
}

// Compare with '?' wildcards in a
extern "C" int fn_40239c(const char *a, const char *b)
{
    int n = rtl_strlen(a) + 1;
    for (int i = 0; i < n; i++) {
        if (a[i] != '?') {
            if (b[i] > a[i])
                return -1;
            if (b[i] < a[i])
                return 1;
        }
    }
    return 0;
}

// Increment the trailing digit of a name ("Foo" -> "Foo0", "Foo9" -> "Fo10" style)
extern "C" char *fn_402434(char *s)
{
    int n = rtl_strlen(s);
    if (n > 0 && s[n - 1] >= '0' && s[n - 1] <= '9')
        n--;
    else
        s[n] = '0';
    s[n]++;
    if (s[n] > '9') {
        s[n] = '0';
        if (n > 0)
            s[n - 1]++;
    }
    s[n + 1] = 0;
    return s;
}

extern "C" RECT fn_4025e0(RECT a, RECT b)
{
    RECT r;
    r.left = *(a.left < b.left ? &a.left : &b.left);
    r.right = *(a.right > b.right ? &a.right : &b.right);
    r.top = *(a.top < b.top ? &a.top : &b.top);
    r.bottom = *(a.bottom > b.bottom ? &a.bottom : &b.bottom);
    return r;
}

extern "C" TPoint fn_402840(RECT r, TPoint p)
{
    if (r.left * 1000 > p.x)
        p.x = r.left * 1000;
    else if (r.right * 1000 <= p.x)
        p.x = (r.right - 1) * 1000;
    if (r.top * 1000 > p.y)
        p.y = r.top * 1000;
    else if (r.bottom * 1000 <= p.y)
        p.y = (r.bottom - 1) * 1000;
    return p;
}

extern "C" void fn_402d48(unsigned char *d, unsigned char *s, int n)
{
    if (!g_455530) {
        for (int i = 0; i < n; i++) {
            *d &= *s;
            d++;
            s++;
        }
    } else {
        for (int j = 0; j < n; j++) {
            if (*s != g_455531)
                *d = *s;
            d++;
            s++;
        }
    }
}

extern "C" void fn_402dbc(unsigned char *d, unsigned char *s, int n)
{
    if (!g_455530) {
        for (int i = 0; i < n; i++) {
            *d |= *s;
            d++;
            s++;
        }
    } else {
        for (int j = 0; j < n; j++) {
            if (*s != g_455530)
                *d = *s;
            d++;
            s++;
        }
    }
}

extern "C" void fn_402194()
{
    MEMORYSTATUS ms;
    ms.dwLength = 32;
    GlobalMemoryStatus(&ms);
    g_45555c = ms.dwTotalPhys;
    g_455564 = ms.dwAvailPhys;
    if (!g_455560 || ms.dwAvailPhys < g_455560)
        g_455560 = ms.dwAvailPhys;
    if (ms.dwAvailPhys > g_455568)
        g_455568 = ms.dwAvailPhys;
    if (g_455554 == -1 || (int)ms.dwMemoryLoad < g_455554)
        g_455554 = ms.dwMemoryLoad;
    if (ms.dwMemoryLoad > g_455558)
        g_455558 = ms.dwMemoryLoad;
}

extern "C" void fn_4029c8(Bitmap *b, int w, int h, int bpp)
{
    HDC dc = g_4601c8->dc;
    if (!g_4601c8->dc)
        dc = CreateCompatibleDC(0);
    fn_40298c(b, w, h, bpp);
    b->handle = CreateDIBSection(dc, (BITMAPINFO *)b->info, 0, (void **)&b->bits, 0, 0);
    if (!b->handle)
        fn_401f0c("Unable to create bitmap");
    if (!g_4601c8->dc)
        DeleteDC(dc);
}

// Build "name" + "R" + a + b + ext from src
extern "C" void fn_401d48(char *dst, const char *src, char a, char b)
{
    char suffix[4];
    suffix[0] = 'R';
    suffix[1] = a;
    suffix[2] = b;
    suffix[3] = 0;
    char *dot = rtl_strrchr(src, '.');
    int len = dot - src;
    if (128 - rtl_strlen(suffix) <= rtl_strlen(src))
        len -= rtl_strlen(suffix);
    rtl_strncpy(dst, src, len);
    dst[len] = 0;
    rtl_strcat(dst, suffix);
    rtl_strcat(dst, dot);
}

extern "C" void fn_402e30(SIZE *s, RECT *r)
{
    r->left = tmax_if((LONG)0, r->left);
    r->top = tmax_if((LONG)0, r->top);
    r->right = tmin_if(s->cx, r->right);
    r->bottom = tmin_if(s->cy, r->bottom);
}

extern "C" void fn_402a58(Bitmap *b, DibFile *f)
{
    fn_4029c8(b, f->bih.biWidth, f->bih.biHeight, f->bih.biBitCount);
}

// Format v as a fixed number of decimal digits; skip leading zeros if strip
extern "C" void fn_401e48(char *d, int v, int digits, char strip)
{
    char started = 0;
    int div = 1;
    for (int i = 1; i < digits; i++)
        div *= 10;
    for (int k = 0; k < digits; k++) {
        int dig = tmin_if(9, v / div);
        v = v % div;
        if (!strip || started || dig > 0 || digits - 1 == k) {
            *d = dig + '0';
            d++;
            started = 1;
        }
        div = div / 10;
    }
    *d = 0;
}

// Frame primer for fn_402a7c (not game code; match.py ignores it). bcc32 5.3
// sizes the gap above fn_402a7c's 3-byte RGB local from state left by the
// function compiled just before it; a function with one int local leaves the
// state the original build had. See docs/HEADERS.md "Frame primer".
extern "C" int elf_frame_primer_402a7c(int a)
{
    int b = a;
    return b;
}

// Mirror a scan line in place (bpp is bytes per pixel, 1 or 3)
extern "C" void fn_402a7c(int bpp, char *row, int bytes)
{
    int n = bytes / bpp;
    if (bpp == 1) {
        char *p = row;
        char *q = p + n - 1;
        for (int i = 0; i < n / 2; i++) {
            char t = *p;
            *p = *q;
            p++;
            *q = t;
            q--;
        }
    } else {
        RGB *p = (RGB *)row;
        RGB *q = p + n - 1;
        for (int i = 0; i < n / 2; i++) {
            int unused1;        // 8 unexplained bytes in the original frame. Two
            int unused2;        // ints, not int[2]: the array form moved the frame (0x2c -> 0x80) with header changes
            RGB t = *p;
            *p = *q;
            p++;
            *q = t;
            q--;
        }
    }
}

extern "C" void fn_402224()
{
    fn_401cfc();
    fn_402194();
    fn_401d04("-------------- Memory Report ---------------\n");
    fn_401d04("Total Physical Memory: %dK\n", g_45555c / 1024);
    fn_401d04("Current Available Memory: %dK\n", g_455564 / 1024);
    fn_401d04("Lowest Memory Load: %d\n", g_455554);
    fn_401d04("Highest Memory Load: %d\n", g_455558);
    fn_401d04("Highest Available Physical Memory: %dK\n", g_455568 / 1024);
    fn_401d04("Lowest Available Physical Memory: %dK\n", g_455560 / 1024);
    fn_401d04("Difference from Highest to Lowest: %dK\n\n", (g_455568 - g_455560) / 1024);
    fn_401d04("Allocated by Mike: %d\n\n", g_455550);
    fn_401d04("-------------- End of Report ---------------\n");
}

// fn_40252c taking ERect (the call target is a fixup, so the name is free)
extern "C" bool fn_40252c_TRect(ERect a, ERect b);

// Clip *r to b; empty rect if they do not intersect
extern "C" char fn_402654(ERect *r, ERect b)
{
    char ok = fn_40252c_TRect(*r, b);
    if (ok) {
        ERect t;
        t.left = tmax_if(r->left, b.left);
        t.right = tmin_if(r->right, b.right);
        t.top = tmax_if(r->top, b.top);
        t.bottom = tmin_if(r->bottom, b.bottom);
        *r = t;
    } else
        *r = Classes_TRect(0, 0, 0, 0);
    return ok;
}

// Delphi virtual call: st->vt[1] (TStream::Read) compiles to mov ebx,[eax]; call [ebx+4].

// Read a 256-entry RGBQUAD palette: fill the shared BITMAPINFO colours and create an HPALETTE
extern "C" int fn_402030(TStreamVmt *st, HPALETTE *pal)
{
    int size = 0x400;
    int lsize = 0x408;
    BITMAPINFO *bi = (BITMAPINFO *)g_45552c;
    RGBQUAD *src = (RGBQUAD *)fn_402338(size, "GetPalette");
    int n = st->vt[1](st, src, size);
    if (n != size)
        fn_401f0c("Error reading palette");
    LOGPALETTE *lp = (LOGPALETTE *)fn_402338(lsize, "GetPalette");
    rtl_memset(lp, 0, lsize);
    lp->palVersion = 0x300;
    lp->palNumEntries = 0x100;
    for (int i = 0; i < 0x100; i++) {
        lp->palPalEntry[i].peRed = bi->bmiColors[i].rgbRed = src[i].rgbRed;
        lp->palPalEntry[i].peGreen = bi->bmiColors[i].rgbGreen = src[i].rgbGreen;
        lp->palPalEntry[i].peBlue = bi->bmiColors[i].rgbBlue = src[i].rgbBlue;
    }
    // The original tests entry 0's red three times
    if (src[0].rgbRed == 0xff && src[0].rgbRed == 0xff && src[0].rgbRed == 0xff) {
        g_455530 = 0xff;
        g_455531 = 0;
    }
    *pal = CreatePalette(lp);
    fn_402380(lp, "GetPalette");
    fn_402380(src, "GetPalette");
    if (!*pal)
        fn_401f0c("Error creating palette.");
    return n;
}

// Blit sr of src to dp in dst with a raster op (SRCAND / SRCPAINT / SRCCOPY)
extern "C" void fn_402ed4(Bitmap *dst, TPoint *dp, Bitmap *src, RECT *sr, DWORD rop)
{
    int w = sr->right - sr->left;
    int h = sr->bottom - sr->top;
#ifdef PORT
    int canvas = g_4601c8 && (dst == &g_4601c8->back || dst == &g_4601c8->front);
    hires_blit(0, HIRES_BMP(dst), canvas, dp->x, dp->y, HIRES_BMP(src), sr->left, sr->top, w, h, rop, g_455530, g_455531);
#endif
    if (w > 0) {
        for (int i = 0; i < h; i++) {
            unsigned char *d = (unsigned char *)dst->bits + (dst->height - (dp->y + i) - 1) * ((dst->width * dst->bpp / 8 + 3) / 4 * 4) + dp->x * dst->bpp / 8;
            unsigned char *s = (unsigned char *)src->bits + (src->height - (sr->top + i) - 1) * ((src->width * src->bpp / 8 + 3) / 4 * 4) + sr->left * src->bpp / 8;
            int n = w * src->bpp / 8;
            switch (rop) {
            case SRCAND: fn_402d48(d, s, n); break;
            case SRCPAINT: fn_402dbc(d, s, n); break;
            case SRCCOPY: rtl_memmove(d, s, n); break;
            }
        }
    }
#ifdef PORT
    hires_blit(1, HIRES_BMP(dst), canvas, dp->x, dp->y, HIRES_BMP(src), sr->left, sr->top, w, h, rop, g_455530, g_455531);
#endif
}

// Scaled-down copy of s (every n-th pixel of every n-th row)
extern "C" void fn_402bb8(Bitmap *d, Bitmap *s, int n)
{
    fn_4029c8(d, s->width / n, s->height / n, s->bpp);
    fn_4024e8(&d->r0c, &s->r0c, n);
    fn_4024e8(&d->r1c, &s->r1c, n);
    int ss = (s->width * s->bpp / 8 + 3) / 4 * 4;
    int ds = (d->width * d->bpp / 8 + 3) / 4 * 4;
    if (d->bpp == 8) {
        for (int i = 0; i < d->height; i++) {
            char *sp = s->bits + n * i * ss;
            char *dp = ds * i + d->bits;
            for (int j = 0; j < d->width; j++) {
                *dp = *sp;
                dp++;
                sp += n;
            }
        }
    } else {
        for (int i = 0; i < d->height; i++) {
            RGB *sp = (RGB *)(s->bits + n * i * ss);
            RGB *dp = (RGB *)(ds * i + d->bits);
            for (int j = 0; j < d->width; j++) {
                *dp = *sp;
                dp++;
                sp += n;
            }
        }
    }
#ifdef PORT
    hires_scaled(HIRES_BMP(d), HIRES_BMP(s), n);
#endif
}

// fn_402e30 taking ERect
extern "C" void fn_402e30_t(SIZE *s, ERect *r);

// Fill r (clipped) with the colour of src's first pixel
extern "C" void fn_403048(Bitmap *b, RECT *r, Bitmap *src)
{
    ERect t = *(ERect *)r;
    fn_402e30_t((SIZE *)b, &t);
    int w = t.right - t.left;
    int h = t.bottom - t.top;
    if (w > 0) {
        if (b->bpp == 8) {
            unsigned char c = *src->bits;
            for (int i = 0; i < h; i++) {
                char *p = (b->height - (t.top + i) - 1) * ((b->width * b->bpp / 8 + 3) / 4 * 4) + b->bits + t.left * b->bpp / 8;
                int n = w * b->bpp / 8;
                rtl_memset(p, c, n);
            }
        } else {
            char c0 = src->bits[0];
            char c1 = src->bits[1];
            char c2 = src->bits[2];
            for (int i = 0; i < h; i++) {
                char *p = (b->height - (t.top + i) - 1) * ((b->width * b->bpp / 8 + 3) / 4 * 4) + b->bits + t.left * b->bpp / 8;
                int n = w * b->bpp / 8;
                for (int k = 0; k < n; k += 3) {
                    *p = c0;
                    p++;
                    *p = c1;
                    p++;
                    *p = c2;
                    p++;
                }
            }
        }
    }
#ifdef PORT
    hires_touch(HIRES_BMP(b), t.left, t.top, t.right, t.bottom);
#endif
}

// In r: background index -> transparent index (8-bit); white -> black (24-bit)
extern "C" void fn_403240(Bitmap *b, RECT *r)
{
    ERect t = *(ERect *)r;
    fn_402e30_t((SIZE *)b, &t);
    int w = t.right - t.left;
    int h = t.bottom - t.top;
    if (w > 0) {
        if (b->bpp == 8) {
            for (int i = 0; i < h; i++) {
                unsigned char *p = (b->height - (t.top + i) - 1) * ((b->width * b->bpp / 8 + 3) / 4 * 4) + (unsigned char *)b->bits + t.left * b->bpp / 8;
                int n = w * b->bpp / 8;
                for (int k = 0; k < n; k++)
                    if (p[k] == g_455531)
                        p[k] = g_455530;
            }
        } else {
            for (int i = 0; i < h; i++) {
                unsigned char *p = (b->height - (t.top + i) - 1) * ((b->width * b->bpp / 8 + 3) / 4 * 4) + (unsigned char *)b->bits + t.left * b->bpp / 8;
                int n = w * b->bpp / 8;
                for (int k = 0; k < n; k += 3) {
                    if (p[0] == 0xff && p[1] == 0xff && p[2] == 0xff) {
                        p[0] = 0;
                        p[1] = 0;
                        p[2] = 0;
                    }
                    p += 3;
                }
            }
        }
    }
}
