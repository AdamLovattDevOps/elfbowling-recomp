// Bitmap / font / cast helpers 0x403540-0x40604c
#include <elf/funcs.h>
#ifdef PORT
#include "../port/src/hires.h"   // 3x art hooks (native only; no bytes under bcc32)
#endif

// fn_402e30 taking ERect (funcs.h has RECT *; see range_401d0c.cpp).
extern "C" void fn_402e30_t(SIZE *s, ERect *r);

extern "C" void fn_403540(Bitmap *b, int color)
{
    RECT r = Classes_Rect(0, 0, b->width, b->height);
    fn_40343c(b, &r, color);
}

extern "C" void fn_4039ac(Bitmap *b, void *dst, int y)
{
    int ok = 0;
    y = b->height - y - 1;
    if (y >= 0 && y < b->height) {
        rtl_memmove(dst, (b->width * b->bpp / 8 + 3) / 4 * 4 * y + b->bits, b->width * b->bpp / 8);
        ok = 1;
    }
    if (ok < 1)
        fn_401f0c("Error getting Scan Line of bitmap ");
}

extern "C" void fn_403a40(Bitmap *b, const void *src, int y)
{
    int ok = 0;
    y = b->height - y - 1;
    if (y >= 0 && y < b->height) {
        rtl_memmove((b->width * b->bpp / 8 + 3) / 4 * 4 * y + b->bits, src, b->width * b->bpp / 8);
        ok = 1;
    }
    if (ok < 1)
        fn_401f0c("Error getting Scan Line of bitmap ");
}

// Does any pixel in r equal the transparent colour?
extern "C" char fn_403ad4(Bitmap *b, RECT *r)
{
    char hit = 0;
    int w = r->right - r->left;
    int h = r->bottom - r->top;
    if (w > 0) {
        for (int i = 0; !hit && i < h; i++) {
            unsigned char *p = (unsigned char *)fn_403940(b, r->top + i) + r->left;
            for (int j = 0; !hit && j < w; j++) {
                hit |= *p == g_455530;
                p++;
            }
        }
    }
    return hit;
}

extern "C" int fn_4036c8(const char *face, int height, int weight)
{
    HFONT f;
    HDC dc;
    int r;
    LOGFONT lf;
    rtl_memset(&lf, 0, sizeof(lf));
    dc = CreateCompatibleDC(0);
    g_4601ec = 0;
    r = EnumFontFamiliesA(dc, face, (FONTENUMPROC)fn_4035f4, (LPARAM)&lf);
    if (g_4601ec) {
        lf.lfHeight = height;
        lf.lfWidth = 0;
        lf.lfWeight = weight;
        f = CreateFontIndirectA(&lf);
        if (f)
            g_4601cc[g_4555ac++] = f;
        else
            fn_401f30("Unable to Create font ", face);
    }
    DeleteDC(dc);
    return r;
}

extern "C" void fn_403768()
{
    for (int i = 0; i < g_4555ac; i++)
        DeleteObject(g_4601cc[i]);
}

extern "C" void fn_403794(HDC dc, int idx)
{
    if (idx < g_4555ac) {
        g_4601f0 = SelectObject(dc, g_4601cc[idx]);
        return;
    }
    fn_401f0c("Font Index out of range");
}

extern "C" void fn_4037c8(HDC dc)
{
    if (g_4555ac)
        SelectObject(dc, g_4601f0);
}

extern "C" char fn_403c34(Bitmap *b, RECT *a)
{
    return b->bpp == 8 ? fn_403ad4(b, a) : fn_403b6c(b, a);
}

extern "C" char fn_403e54(Bitmap *b, RECT *a, Bitmap *c, RECT *d)
{
    return b->bpp == 8 ? fn_403c60(b, a, c, d) : fn_403d38(b, a, c, d);
}

extern "C" HBITMAP fn_403574(Bitmap *b, int w, int h, int bpp, int which)
{
    fn_4029c8(b, w, h, bpp);
    int fill = which == 0 ? g_455530 : g_455531;
    unsigned bytes = b->width * b->bpp / 8;
    unsigned stride = (bytes + 3) / 4 * 4;
    rtl_memset(b->bits, fill, stride * b->height);
    return b->handle;
}

extern "C" char *fn_403940(Bitmap *b, int y)
{
    char *r = 0;
    y = b->height - y - 1;
    if (y >= 0 && y < b->height)
        r = (b->width * b->bpp / 8 + 3) / 4 * 4 * y + b->bits;
    else
        fn_401f0c("Error getting Scan Line of bitmap");
    return r;
}

extern "C" void fn_404f50(TGraphicCast *c, char f)
{
    if (c->bmp.bpp == 8) {
        fn_404974(c, f);
        return;
    }
    if (c->bmp.bpp == 0x18) {
        fn_404c54(c, f);
        return;
    }
    fn_401f0c("Unimplemented color depth for TGraphicCast");
}

extern "C" char fn_4052ac(TGraphicCast *c, void *pal, const char *name, char e, char f)
{
    fn_40d980(g_455524, c, c->name, name, pal, e, f);
    return 1;
}

extern "C" void fn_405954(TTextCast *p)
{
    fn_4058f0(p);
    fn_403540(&p->bmp, 0xffffff);
}

extern "C" void fn_405978(TTextCast *p, int v) { p->f24c = v; }
extern "C" void fn_40598c(TTextCast *p, int v) { p->f254 = v; }

extern "C" void fn_4059a0(TCastMgr *l, TGraphicCast *v)
{
    if (l->count < 0x200)
        l->items[l->count++] = v;
}

extern "C" char fn_403e8c(TGraphicCast *a, RECT *ax, TGraphicCast *b, RECT *bx)
{
    char r;
    if (!a->masked) {
        if (!b->masked)
            r = 1;
        else
            r = fn_403c34(&b->mask, bx);
    } else {
        if (!b->masked)
            r = fn_403c34(&a->mask, ax);
        else
            r = fn_403e54(&a->mask, ax, &b->mask, bx);
    }
    return r;
}

extern "C" void fn_404f94(TGraphicCast *c, unsigned char m)
{
    if (c->masked != m) {
        c->masked = m;
        if (c->bmp.handle) {
            if (!c->masked) {
                RECT t = Classes_Rect(0, 0, c->bmp.width, c->bmp.height);
                c->bounds = t;
            } else {
                fn_4048a0(c);
                fn_404f50(c, 1);
            }
        }
    }
}

extern "C" void fn_4058f0(TTextCast *t)
{
    t->fb8 = 0;
    t->f270 = t->f26c;
    t->bufPtr = t->bufStart;
    *t->bufPtr = 0;
    t->f25c.x = t->f27c.x;
    t->f25c.y = t->f27c.y;
}

extern "C" TGraphicCast *fn_4059c4(TCastMgr *l, const char *name)
{
    TGraphicCast *r = 0;
    for (int i = 0; i < l->count && !r; i++)
        if (!rtl_stricmp(l->items[i]->name, name))
            r = l->items[i];
    return r;
}

extern "C" void fn_40500c(TGraphicCast *c, TPoint pt)
{
    if (c->bmp.r0c.top == 0 && c->bmp.r0c.left == 0 && c->bmp.r0c.bottom == 0 && c->bmp.r0c.right == 0) {
        c->hot.x = pt.x;
        c->hot.y = pt.y;
    } else {
        c->hot.x = (c->bmp.r0c.right - c->bmp.r0c.left) / 2;
        c->hot.y = (c->bmp.r0c.bottom - c->bmp.r0c.top) / 2;
        c->hot.x -= c->bmp.r1c.left - c->bmp.r0c.left;
        c->hot.y -= c->bmp.r1c.top - c->bmp.r0c.top;
    }
}

// Does any pixel in r equal the transparent colour? (24-bit)
extern "C" char fn_403b6c(Bitmap *b, RECT *r)
{
    char hit = 0;
    int w = r->right - r->left;
    int h = r->bottom - r->top;
    if (w > 0) {
        for (int i = 0; !hit && i < h; i++) {
            unsigned char *p = (unsigned char *)fn_403940(b, r->top + i) + r->left * 3;
            for (int j = 0; !hit && j < w; j++) {
                hit |= p[0] == g_455530 && p[1] == g_455530 && p[2] == g_455530;
                p += 3;
            }
        }
    }
    return hit;
}

// EnumFontFamilies callback: pick the italic script face
extern "C" int CALLBACK fn_4035f4(const LOGFONT *lf, const TEXTMETRIC *tm, DWORD type, LPARAM lp)
{
    if (!rtl_strcmp((const char *)((const ENUMLOGFONT *)lf)->elfFullName, "Lucida Handwriting Italic")) {
        if (!rtl_strcmp((const char *)((const ENUMLOGFONT *)lf)->elfStyle, "Italic")) {
            rtl_memcpy((void *)lp, lf, sizeof(LOGFONT));
            g_4601ec = 1;
        }
    } else if (!rtl_strcmp((const char *)((const ENUMLOGFONT *)lf)->elfFullName, "Lucida Calligraphy Italic")) {
        if (!rtl_strcmp((const char *)((const ENUMLOGFONT *)lf)->elfStyle, "Italic")) {
            rtl_memcpy((void *)lp, lf, sizeof(LOGFONT));
            g_4601ec = 1;
        }
    } else if (!rtl_strcmp((const char *)((const ENUMLOGFONT *)lf)->elfStyle, "Regular")) {
        rtl_memcpy((void *)lp, lf, sizeof(LOGFONT));
        g_4601ec = 1;
    }
    return 1;
}

extern "C" void fn_4048a0(TGraphicCast *c)
{
    if (!c->mask.handle) {
        fn_402b4c(&c->mask, &c->bmp);
        if (!c->mask.handle)
            fn_401f30("Unable to create a mask for bitmap ", c->name);
    }
    if (c->masked == 1) {
        if (c->bmp.bpp == 8)
            fn_403f04(c);
        else if (c->bmp.bpp == 0x18)
            fn_404268(c);
        else
            fn_401f30("Unknown color depth for cast ", c->name);
    } else if (c->masked == 2) {
        if (c->bmp.bpp == 8)
            fn_4045f0(c);
        else if (c->bmp.bpp == 0x18)
            fn_404718(c);
        else
            fn_401f30("Unknown color depth for cast ", c->name);
    }
}

// Do two 8-bit masks both have a transparent pixel at the same place?
extern "C" char fn_403c60(Bitmap *a, RECT *ar, Bitmap *b, RECT *br)
{
    char hit = 0;
    int w = ar->right - ar->left;
    int h = ar->bottom - ar->top;
    if (w > 0) {
        for (int i = 0; !hit && i < h; i++) {
            unsigned char *p = (unsigned char *)fn_403940(a, ar->top + i) + ar->left;
            unsigned char *q = (unsigned char *)fn_403940(b, br->top + i) + br->left;
            for (int j = 0; !hit && j < w; j++) {
                hit |= *p == g_455530 && *q == g_455530;
                p++;
                q++;
            }
        }
    }
    return hit;
}

extern "C" char fn_405710(TTextCast *t, const char *text)
{
    fn_40343c(&t->bmp, &t->bounds, 0xffffff);
    int n = fn_4037e4(&t->bmp, text, t->f25c.x, t->f25c.y, t->f24c, t->f254, t->align);
    fn_402ed4(&t->mask, (TPoint *)&t->bounds, &t->bmp, &t->bounds, SRCCOPY);
    fn_403240(&t->bmp, &t->bounds);
    char ok = n >= 0;
    if (ok) {
        t->f25c.y += n + t->lineSpacing;
        t->lineHeight = n + t->lineSpacing;
    }
    return ok;
}

extern "C" char fn_4057f8(TTextCast *t, const char *text)
{
    fn_402ed4(&t->bmp, (TPoint *)&t->bounds, &t->mask, &t->bounds, SRCCOPY);
    int n = fn_4037e4(&t->bmp, text, t->f25c.x, t->f25c.y, t->f24c, t->f254, t->align);
    fn_402ed4(&t->mask, (TPoint *)&t->bounds, &t->bmp, &t->bounds, SRCCOPY);
    fn_403240(&t->bmp, &t->bounds);
    char ok = n >= 0;
    if (ok) {
        t->f25c.y += n + t->lineSpacing;
        t->lineHeight = n + t->lineSpacing;
    }
    return ok;
}

// Fill r (clipped to the bitmap) with the transparent (which==0) or background index
extern "C" void fn_40343c(Bitmap *b, RECT *r, int which)
{
    ERect t = *(ERect *)r;
    fn_402e30_t((SIZE *)b, &t);
    int w = t.right - t.left;
    int h = t.bottom - t.top;
    unsigned char fill = which == 0 ? g_455530 : g_455531;
    if (w > 0) {
        for (int i = 0; i < h; i++) {
            char *p = (b->height - (t.top + i) - 1) * ((b->width * b->bpp / 8 + 3) / 4 * 4) + b->bits + t.left * b->bpp / 8;
            int n = w * b->bpp / 8;
            rtl_memset(p, fill, n);
        }
    }
#ifdef PORT
    hires_touch(HIRES_BMP(b), t.left, t.top, t.right, t.bottom);
#endif
}

// Do two 24-bit masks both have a transparent pixel at the same place?
extern "C" char fn_403d38(Bitmap *a, RECT *ar, Bitmap *b, RECT *br)
{
    char hit = 0;
    int w = ar->right - ar->left;
    int h = ar->bottom - ar->top;
    if (w > 0) {
        for (int i = 0; !hit && i < h; i++) {
            unsigned char *p = (unsigned char *)fn_403940(a, ar->top + i) + ar->left * 3;
            unsigned char *q = (unsigned char *)fn_403940(b, br->top + i) + br->left * 3;
            for (int j = 0; !hit && j < w; j++) {
                hit |= p[0] == g_455530 && p[1] == g_455530 && p[2] == g_455530 &&
                       q[0] == g_455530 && q[1] == g_455530 && q[2] == g_455530;
                p += 3;
                q += 3;
            }
        }
    }
    return hit;
}

// Mask: background-index pixels stay background, everything else becomes transparent
extern "C" void fn_4045f0(TGraphicCast *c)
{
    char *line = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "MakeTransparentMask8Bit");
    char *mline = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "MakeTransparentMask8Bit");
    for (int y = 0; y < c->r20.bottom; y++) {
        fn_4039ac(&c->bmp, line, y);
        fn_4039ac(&c->mask, mline, y);
        unsigned char *p = (unsigned char *)line;
        unsigned char *q = (unsigned char *)mline;
        for (int x = 0; x < c->r20.right; x++) {
            if (p[x] == g_455531)
                q[x] = g_455531;
            else
                q[x] = g_455530;
        }
        fn_403a40(&c->mask, mline, y);
    }
    fn_402380(mline, "MakeTransparentMask8Bit");
    fn_402380(line, "MakeTransparentMask8Bit");
}

// Draw text into a bitmap with font idx; returns the DrawText height
extern "C" int fn_4037e4(Bitmap *b, const char *text, int x, int y, int font, int color, char align)
{
    x = tmin_if(x, b->width);
    y = tmin_rev(y, b->height);
    RECT r = Classes_TRect(x, y, b->width, b->height);
    HDC dc = CreateCompatibleDC(0);
    SelectObject(dc, b->handle);
    fn_403794(dc, font);
    SetBkMode(dc, OPAQUE);
    switch (color) {
    case 0xffffff: SetTextColor(dc, 0xf0f0f0); break;
    case 0xffff: SetTextColor(dc, 0xffff); break;
    case 0: SetTextColor(dc, 0); break;
    default: fn_401f0c("Unknown color for MyTextOut");
    }
    int flags = 0;
    switch (align) {
    case 0: flags = 0; break;
    case 2: flags = 2; break;
    case 1: flags = 1; break;
    }
    flags |= DT_WORDBREAK;
    int n = DrawTextA(dc, text, -1, &r, flags);
    fn_4037c8(dc);
    DeleteDC(dc);
    return n;
}
// Matte: flood-fill the background-index area connected to the border (8-bit)
extern "C" void fn_403f04(TGraphicCast *c)
{
    int size = c->r20.right * c->r20.bottom;
    unsigned char *matte = (unsigned char *)fn_402338(size, "MakeMatte8Bit");
    char *line = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "MakeMatte8Bit");
    char *mline = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "MakeMatte8Bit");
    int pass = 0;
    if (matte) {
        rtl_memset(matte, 0, size);
        bool done = false;
        while (!done) {
            done = 1;
            int dy = 1;
            int y0 = 0;
            int y1 = c->r20.bottom;
            int dx = 1;
            int x0 = 0;
            int x1 = c->r20.right;
            if (pass % 2) {
                dy = -1;
                y0 = c->r20.bottom - 1;
                y1 = -1;
            }
            if (pass % 4 > 1) {
                dx = -1;
                x0 = c->r20.right - 1;
                x1 = -1;
            }
            pass++;
            for (int y = y0; y != y1; y += dy) {
                fn_4039ac(&c->bmp, line, y);
                unsigned char *p = (unsigned char *)line;
                unsigned char *m = c->r20.right * y + matte;
                unsigned char *up = m - c->r20.right;
                unsigned char *dn = c->r20.right + m;
                for (int x = x0; x != x1; x += dx) {
                    if (!m[x] && p[x] == g_455531) {
                        if (y == 0 || c->r20.bottom - 1 == y || x == 0 || c->r20.right - 1 == x) {
                            m[x] = 1;
                            done = 0;
                        } else if (m[x - 1] == 1) {
                            m[x] = 1;
                            done = 0;
                        } else if (m[x + 1] == 1) {
                            m[x] = 1;
                            done = 0;
                        } else if (up[x] == 1) {
                            m[x] = 1;
                            done = 0;
                        } else if (dn[x] == 1) {
                            m[x] = 1;
                            done = 0;
                        }
                    }
                }
            }
        }
        for (int y = 0; y < c->r20.bottom; y++) {
            fn_4039ac(&c->bmp, line, y);
            fn_4039ac(&c->mask, mline, y);
            unsigned char *q = (unsigned char *)mline;
            unsigned char *m = c->r20.right * y + matte;
            for (int x = 0; x < c->r20.right; x++) {
                *q = *m ? g_455531 : g_455530;
                q++;
                m++;
            }
            fn_403a40(&c->mask, mline, y);
        }
    }
    fn_402380(mline, "MakeMatte8Bit");
    fn_402380(line, "MakeMatte8Bit");
    fn_402380(matte, "MakeMatte8Bit");
}

// Matte: flood-fill the background-index area connected to the border (24-bit)
extern "C" void fn_404268(TGraphicCast *c)
{
    int size = c->r20.right * c->r20.bottom;
    unsigned char *matte = (unsigned char *)fn_402338(size, "MakeMatte24Bit");
    char *line = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "MakeMatte24Bit");
    char *mline = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "MakeMatte24Bit");
    int pass = 0;
    if (matte) {
        rtl_memset(matte, 0, size);
        bool done = false;
        while (!done) {
            done = 1;
            int dy = 1;
            int y0 = 0;
            int y1 = c->r20.bottom;
            int dx = 1;
            int x0 = 0;
            int x1 = c->r20.right;
            if (pass % 2) {
                dy = -1;
                y0 = c->r20.bottom - 1;
                y1 = -1;
            }
            if (pass % 4 > 1) {
                dx = -1;
                x0 = c->r20.right - 1;
                x1 = -1;
            }
            pass++;
            for (int y = y0; y != y1; y += dy) {
                fn_4039ac(&c->bmp, line, y);
                RGB *p = (RGB *)line;
                unsigned char *m = c->r20.right * y + matte;
                unsigned char *up = m - c->r20.right;
                unsigned char *dn = c->r20.right + m;
                for (int x = x0; x != x1; x += dx) {
                    if (!m[x] && (p[x].b & p[x].g & p[x].r) == g_455531) {
                        if (y == 0 || c->r20.bottom - 1 == y || x == 0 || c->r20.right - 1 == x) {
                            m[x] = 1;
                            done = 0;
                        } else if (m[x - 1] == 1) {
                            m[x] = 1;
                            done = 0;
                        } else if (m[x + 1] == 1) {
                            m[x] = 1;
                            done = 0;
                        } else if (up[x] == 1) {
                            m[x] = 1;
                            done = 0;
                        } else if (dn[x] == 1) {
                            m[x] = 1;
                            done = 0;
                        }
                    }
                }
            }
        }
        for (int y = 0; y < c->r20.bottom; y++) {
            fn_4039ac(&c->bmp, line, y);
            fn_4039ac(&c->mask, mline, y);
            RGB *q = (RGB *)mline;
            unsigned char *m = c->r20.right * y + matte;
            for (int x = 0; x < c->r20.right; x++) {
                q->b = q->g = q->r = *m ? g_455531 : g_455530;
                q++;
                m++;
            }
            fn_403a40(&c->mask, mline, y);
        }
    }
    fn_402380(mline, "MakeMatte24Bit");
    fn_402380(line, "MakeMatte24Bit");
    fn_402380(matte, "MakeMatte24Bit");
}

// Mask: pixels whose channels AND to the background index stay background, others transparent (24-bit)
extern "C" void fn_404718(TGraphicCast *c)
{
    char *line = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "MakeTransparentMask24Bit");
    char *mline = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "MakeTransparentMask24Bit");
    for (int y = 0; y < c->r20.bottom; y++) {
        fn_4039ac(&c->bmp, line, y);
        fn_4039ac(&c->mask, mline, y);
        RGB *p = (RGB *)line;
        RGB *q = (RGB *)mline;
        for (int x = 0; x < c->r20.right; x++) {
            if ((p[x].b & p[x].g & p[x].r) == g_455531)
                q[x].b = q[x].g = q[x].r = g_455531;
            else
                q[x].b = q[x].g = q[x].r = g_455530;
        }
        fn_403a40(&c->mask, mline, y);
    }
    fn_402380(mline, "MakeTransparentMask8Bit");
    fn_402380(line, "MakeTransparentMask8Bit");
}

// Build the mask from the transparent index and crop bounds to the opaque area (8-bit)
extern "C" void fn_404974(TGraphicCast *c, char crop)
{
    char *cols = (char *)fn_402338(c->r20.right, "Crop8Bit");
    char *rows = (char *)fn_402338(c->r20.bottom, "Crop8Bit");
    char *line = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "Crop8Bit");
    char *mline = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "Crop8Bit");
    if (rows && cols) {
        rtl_memset(cols, 0, c->r20.right);
        rtl_memset(rows, 0, c->r20.bottom);
        char any = 0;
        for (int y = c->r20.top; y < c->r20.bottom; y++) {
            fn_4039ac(&c->bmp, line, y);
            fn_4039ac(&c->mask, mline, y);
            unsigned char *p = (unsigned char *)line;
            unsigned char *q = (unsigned char *)mline;
            char rowany = 0;
            for (int x = c->r20.left; x < c->r20.right; x++) {
                bool opaque = *q != g_455531;
                if (c->keepBg != 1 && !opaque)
                    *p = g_455530;
                rowany |= opaque;
                cols[x] |= opaque;
                q++;
                p++;
            }
            fn_403a40(&c->bmp, line, y);
            any |= rowany;
            rows[y] = rowany;
        }
        if (crop) {
            if (any) {
                int i;
                for (i = c->r20.left; i < c->r20.right && !cols[i]; i++)
                    ;
                c->bounds.left = i;
                for (i = c->r20.right - 1; i >= c->r20.left && !cols[i]; i--)
                    ;
                c->bounds.right = i + 1;
                for (i = c->r20.top; i < c->r20.bottom && !rows[i]; i++)
                    ;
                c->bounds.top = i;
                for (i = c->r20.bottom - 1; i >= c->r20.left && !rows[i]; i--)
                    ;
                c->bounds.bottom = i + 1;
            } else
                c->bounds = c->r20;
        } else
            c->bounds = c->r20;
    }
    fn_402380(mline, "Crop8Bit");
    fn_402380(line, "Crop8Bit");
    fn_402380(cols, "Crop8Bit");
    fn_402380(rows, "Crop8Bit");
}

// Build the mask from the transparent index and crop bounds to the opaque area (24-bit)
extern "C" void fn_404c54(TGraphicCast *c, char crop)
{
    char *cols = (char *)fn_402338(c->r20.right, "Crop24Bit");
    char *rows = (char *)fn_402338(c->r20.bottom, "Crop24Bit");
    char *line = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "Crop24Bit");
    char *mline = (char *)fn_402338(c->bmp.bpp * c->bmp.width / 8, "Crop24Bit");
    if (rows && cols) {
        rtl_memset(cols, 0, c->r20.right);
        rtl_memset(rows, 0, c->r20.bottom);
        char any = 0;
        for (int y = c->r20.top; y < c->r20.bottom; y++) {
            fn_4039ac(&c->bmp, line, y);
            fn_4039ac(&c->mask, mline, y);
            unsigned char *p = (unsigned char *)line;
            unsigned char *q = (unsigned char *)mline;
            char rowany = 0;
            for (int x = c->r20.left; x < c->r20.right; x++) {
                bool opaque = (q[0] & q[1] & q[2]) != g_455531;
                if (c->keepBg != 1 && !opaque)
                    p[0] = p[1] = p[2] = g_455530;
                rowany |= opaque;
                cols[x] |= opaque;
                q += 3;
                p += 3;
            }
            fn_403a40(&c->bmp, line, y);
            any |= rowany;
            rows[y] = rowany;
        }
        if (crop) {
            if (any) {
                int i;
                for (i = c->r20.left; i < c->r20.right && !cols[i]; i++)
                    ;
                c->bounds.left = i;
                for (i = c->r20.right - 1; i >= c->r20.left && !cols[i]; i--)
                    ;
                c->bounds.right = i + 1;
                for (i = c->r20.top; i < c->r20.bottom && !rows[i]; i++)
                    ;
                c->bounds.top = i;
                for (i = c->r20.bottom - 1; i >= c->r20.left && !rows[i]; i--)
                    ;
                c->bounds.bottom = i + 1;
            } else
                c->bounds = c->r20;
        } else
            c->bounds = c->r20;
    }
    fn_402380(mline, "Crop24Bit");
    fn_402380(line, "Crop24Bit");
    fn_402380(cols, "Crop24Bit");
    fn_402380(rows, "Crop24Bit");
}

// TGraphicCast init: register with the engine's cast list and load the named bitmap.
// Name prefixes: "#<digits>" (scaled copies), then '%' / '$' flags.
extern "C" void fn_4050b4(TGraphicCast *c, TStage *eng, const char *name, char m, TPoint pt, char e, char f)
{
    c->stage = eng;
    fn_40290c(&c->bmp);
    fn_40290c(&c->mask);
    c->masked = 3;
    c->keepBg = 0;
    fn_40500c(c, pt);
    c->used = 0;
    c->name[0] = 0;
    if (name) {
        const char *p = name;
        if (*p == '#') {
            p++;
            while (*p >= '0' && *p <= '9')
                p++;
        }
        while (*p == '%' || *p == '$')
            p++;
        if (e && f)
            fn_401d48(c->name, p, 'X', 'Y');
        else if (e)
            fn_401d48(c->name, p, 'X', 0);
        else if (f)
            fn_401d48(c->name, p, 'Y', 0);
        else
            fn_402410(c->name, p, 0x18);
    }
    fn_4059a0(c->stage->casts, c);
    if (name) {
        fn_4052ac(c, c->stage->pk(), name, e, f);
        RECT r = Classes_Rect(0, 0, c->bmp.width, c->bmp.height);
        c->bounds = c->r20 = r;
    } else {
        RECT r = Classes_Rect(0, 0, 0, 0);
        c->r20 = c->bounds = r;
    }
    fn_404f94(c, m);
}

// TextCast::Init
extern "C" void fn_40552c(TTextCast *t, int w, int h, const char *name)
{
    t->lineSpacing = 0;
    t->lineHeight = 0;
    t->align = 1;
    t->f259 = 0;
    t->f258 = 0;
    fn_405978(t, 0);
    fn_402410(t->name, name, 0x18);
    t->bufStart = (char *)fn_402338(5000, "TextCast::Init");
    t->f26c = 5000;
    fn_403574(&t->bmp, w, h, t->stage->bpp, 0xffffff);
    fn_403574(&t->mask, w, h, t->stage->bpp, 0xffffff);
    RECT r = Classes_Rect(0, 0, w, h);
    t->r20 = t->bounds = r;
    TPoint ctr = Classes_Point((t->r20.right - t->r20.left) / 2, (t->r20.bottom - t->r20.top) / 2);
    fn_40500c(t, ctr);
    t->f25c = t->f27c = Classes_Point(0, 0);
    fn_4058f0(t);
    fn_40598c(t, 0);
}

// LoadAllGraphicCasts: a TGraphicCast for every "bmp" entry in the packed resource.
// "#<n>" adds a copy scaled down by n; "%$"/"$%" loads all four flips, '%' X flips, '$' Y flips.
extern "C" char fn_405a1c(TCastMgr *o)
{
    char ok = 0;
    TGraphicCast *last;
    if (g_455524) {
        char buf[132];
        char found = fn_40dbe0(g_455524, "bmp", buf);
        if (found) do {
            char *p = buf;
            int n = 0;
            if (*p == '#') {
                p++;
                while (*p >= '0' && *p <= '9') {
                    n = *p + n * 10 - '0';
                    p++;
                }
            }
            if ((p[0] == '%' && p[1] == '$') || (p[0] == '$' && p[1] == '%')) {
                last = new TGraphicCast(o->stage, buf, 0, 0, 0);
                if (n)
                    new TGraphicCast(o->stage, last, n);
                last = new TGraphicCast(o->stage, buf, 0, 1, 0);
                if (n)
                    new TGraphicCast(o->stage, last, n);
                last = new TGraphicCast(o->stage, buf, 0, 0, 1);
                if (n)
                    new TGraphicCast(o->stage, last, n);
                last = new TGraphicCast(o->stage, buf, 0, 1, 1);
                if (n)
                    new TGraphicCast(o->stage, last, n);
            }
            if (*p == '%') {
                last = new TGraphicCast(o->stage, buf, 0, 0, 0);
                if (n)
                    new TGraphicCast(o->stage, last, n);
                last = new TGraphicCast(o->stage, buf, 0, 1, 0);
                if (n)
                    new TGraphicCast(o->stage, last, n);
            } else if (*p == '$') {
                last = new TGraphicCast(o->stage, buf, 0, 0, 0);
                if (n)
                    new TGraphicCast(o->stage, last, n);
                last = new TGraphicCast(o->stage, buf, 0, 0, 1);
                if (n)
                    new TGraphicCast(o->stage, last, n);
            } else {
                last = new TGraphicCast(o->stage, buf, 0);
                if (n)
                    new TGraphicCast(o->stage, last, n);
            }
            ok = 1;
            found = fn_40db00(g_455524, "bmp", buf);
        } while (found);
    } else
        fn_401f0c("Packed Resource not valid in LoadAllGraphicCasts");
    return ok;
}
