/* hires.h - 3x high-resolution shadow surfaces for the native port (docs/HIRES.md).
 *
 * The game composites in software on 8-bit DIB bits. The native build keeps, next to
 * the DIBs that reach the screen (the screen itself and TStage's back and front
 * buffers), a 3x RGBA "canvas" and mirrors each draw into it: casts are drawn from the
 * Real-ESRGAN art of tools/upscale.py (or from their own pixels, 3x nearest, when there
 * is no art or the code changed them), mask + sprite pairs are alpha-composited, fills
 * are mirrored, and anything else is caught by a per-pixel check of the canvas against
 * the 1x pixels it was made from (nearest-neighbour refresh). The 1x path is untouched:
 * it stays authoritative, and the classic view is always one key away.
 *
 * The src_match hooks are `#ifdef PORT` only (no bytes under bcc32). */
#ifndef PORT_HIRES_H
#define PORT_HIRES_H

#ifdef __cplusplus
extern "C" {
#endif

/* A game Bitmap's DIB (bottom-up, rows padded to 4 bytes), as the hooks pass it. */
#define HIRES_BMP(b) (unsigned char *)(b)->bits, (b)->width, (b)->height, (b)->bpp

/* ---- hooks from the game (src_match, #ifdef PORT) ---- */

/* fn_40d980: a cast's bitmap was just read from the packed resource `res` ("#4%Name.bmp"),
 * flipped (fn_401d48's X/Y) and cropped to keep (all zero: not cropped). pal is the
 * shared BITMAPINFO's colour table (RGBQUAD[256]). */
void hires_cast_loaded(unsigned char *bits, int w, int h, int bpp, const char *res, int flipX, int flipY,
                       int kl, int kt, int kr, int kb, const unsigned char *pal);
/* fn_402bb8: d is every n-th pixel of s (the "#<n>" scaled cast copies). */
void hires_scaled(unsigned char *dbits, int dw, int dh, int dbpp, unsigned char *sbits, int sw, int sh, int sbpp,
                  int n);
/* fn_402ed4, before (end = 0) and after (end = 1) the raster op. canvas: dst is TStage's
 * back or front buffer. T/B: the transparent and background indices (g_455530/31). */
void hires_blit(int end, unsigned char *dbits, int dw, int dh, int dbpp, int canvas, int dx, int dy,
                unsigned char *sbits, int sw, int sh, int sbpp, int sx, int sy, int w, int h, unsigned rop,
                unsigned char T, unsigned char B);
/* fn_40343c / fn_403048: game code rewrote r of a bitmap (a fill): mirrored on a canvas. */
void hires_touch(unsigned char *bits, int w, int h, int bpp, int l, int t, int r, int b);

/* ---- shim side (gdi.c) ---- */

/* A DIB or the screen: rows top-down when topdown, else bottom-up (GDI order). */
void hires_gdi_canvas(unsigned char *bits, int w, int h, int stride, int topdown);   /* the screen */
void hires_gdi_forget(unsigned char *bits);                                           /* DeleteObject */
void hires_gdi_blit(int end, unsigned char *dbits, int dw, int dh, int dstride, int dtop, unsigned char *sbits,
                    int sw, int sh, int sstride, int stop, int dx, int dy, int sx, int sy, int w, int h,
                    unsigned rop);
void hires_gdi_touch(unsigned char *bits, int w, int h, int stride, int topdown, int l, int t, int r, int b);
/* DrawTextA at 3x. hires_text_wanted(): the canvases are kept (render the 3x glyphs at all).
 * hires_gdi_text: one line was drawn over the 1x rectangle x0,y0-x1,y1 (clipped) in index ti
 * over background bi (OPAQUE); cov is its 3x coverage (3(x1-x0) x 3(y1-y0), 0-255, rows of
 * cstride bytes) from the same font at 3x size. The 1x pixels stay authoritative: a line
 * whose 3x glyphs disagree with the 1x ones is left to the nearest refresh, and each 3x
 * block stands only while its 1x pixel keeps the index the text left. */
int hires_text_wanted(void);
void hires_gdi_text(unsigned char *bits, int w, int h, int stride, int topdown, int x0, int y0, int x1, int y1,
                    const unsigned char *cov, int cstride, unsigned char ti, unsigned char bi);

/* Present support. hires_showing(): draw the 3x canvas (F9 / ELFBOWL_CLASSIC=1 toggle it).
 * hires_screen() brings the screen canvas up to date and returns its 3x RGBA pixels (bytes
 * R,G,B,A; SDL_PIXELFORMAT_RGBA32), its size, and the rows changed since the last call
 * (*y0 >= *y1: none). syspal is the realized palette (PALETTEENTRY[256]); *mod gets the
 * colour scale that turns the art's palette into it (255 = none, a fade scales it), or -1
 * when they differ in some other way: then show the classic frame. */
int hires_showing(void);
/* The canvas scale (3; 2 when activated so). hires_activate: start using the art cache in dir
 * now (the web build: when the loader's pack has arrived); the casts loaded before are
 * registered then. hires_set_showing: the 3x frame (1) or the classic one (0). */
int hires_scale(void);
int hires_activate(const char *dir, int scale);
void hires_set_showing(int on);
void hires_toggle(void);
const unsigned int *hires_screen(const unsigned char *syspal, int *w3, int *h3, int *y0, int *y1, int *mod);
void hires_stats_log(void);

#ifdef __cplusplus
}
#endif

#endif
