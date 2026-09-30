/* GDI/USER subset on SDL2: memory DCs, DIB sections, palettes, BitBlt, FillRect,
 * DrawTextA (SDL2_ttf) and an 8-bit palettized "screen" presented via SDL.
 * Semantics are documented in docs/SHIM.md. */
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shim.h"
#include "hires.h"

/* ------------------------------------------------------------------ objects */

enum { T_BITMAP = 0x4d42, T_PALETTE, T_FONT, T_BRUSH };
enum { DC_MEMORY = 1, DC_SCREEN, DC_INFO };

typedef struct Obj {
    int type;
    int stock;              /* stock objects are never freed */
    int selected;           /* number of DCs it is selected into */
} Obj;

/* A pixel buffer. y is always top-down here; row() handles bottom-up storage. */
typedef struct Surface {
    int w, h, bpp, stride, topdown;
    unsigned char *bits;
    int ncolors;
    RGBQUAD colors[256];    /* 8-bit colour table (a copy, as CreateDIBSection makes) */
} Surface;

typedef struct Bitmap { Obj o; Surface s; } Bitmap;
typedef struct Palette { Obj o; int n; PALETTEENTRY e[256]; } Palette;
typedef struct Font {
    Obj o; LOGFONTA lf; TTF_Font *ttf; int tried;
    TTF_Font *ttf3; int tried3, pt, style; char path[1024];    /* the same face at the canvas scale (hires.c) */
} Font;
typedef struct Brush { Obj o; COLORREF color; int hollow; } Brush;

typedef struct DC {
    int kind;
    Bitmap *bmp;            /* memory DC: selected bitmap */
    Palette *pal;
    int palBackground;
    Font *font;
    Brush *brush;
    COLORREF text, bk;
    int bkmode;
} DC;

/* The 20 static colours of the Windows default palette. */
static const PALETTEENTRY k_static[20] = {
    {0, 0, 0, 0}, {0x80, 0, 0, 0}, {0, 0x80, 0, 0}, {0x80, 0x80, 0, 0}, {0, 0, 0x80, 0},
    {0x80, 0, 0x80, 0}, {0, 0x80, 0x80, 0}, {0xc0, 0xc0, 0xc0, 0}, {0xc0, 0xdc, 0xc0, 0},
    {0xa6, 0xca, 0xf0, 0}, {0xff, 0xfb, 0xf0, 0}, {0xa0, 0xa0, 0xa4, 0}, {0x80, 0x80, 0x80, 0},
    {0xff, 0, 0, 0}, {0, 0xff, 0, 0}, {0xff, 0xff, 0, 0}, {0, 0, 0xff, 0}, {0xff, 0, 0xff, 0},
    {0, 0xff, 0xff, 0}, {0xff, 0xff, 0xff, 0},
};

static Bitmap s_stockBitmap = {{T_BITMAP, 1, 0}, {1, 1, 8, 4, 1, NULL, 0, {{0}}}};
static unsigned char s_stockBits[4];
static Palette s_defaultPalette = {{T_PALETTE, 1, 0}, 20, {{0}}};
static Font s_systemFont = {{T_FONT, 1, 0}, {16, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial"}, NULL, 0, NULL, 0, 0, 0, ""};
static Brush s_stockBrush[6] = {
    {{T_BRUSH, 1, 0}, 0xffffff, 0}, {{T_BRUSH, 1, 0}, 0xc0c0c0, 0}, {{T_BRUSH, 1, 0}, 0x808080, 0},
    {{T_BRUSH, 1, 0}, 0x404040, 0}, {{T_BRUSH, 1, 0}, 0x000000, 0}, {{T_BRUSH, 1, 0}, 0, 1},
};

/* ------------------------------------------------------------------ screen */

static Surface s_screen;                 /* 8-bit, top-down; colors = system palette */
static PALETTEENTRY s_syspal[256];
static int s_syspalInit;
static struct HWND__ s_mainWnd;
static SDL_Window *s_window;
static SDL_Renderer *s_renderer;
static SDL_Texture *s_texture;
static Uint32 *s_rgb;                    /* presented ARGB8888 frame */
static SDL_Texture *s_htex;              /* the 3x canvas (hires.c) */
static int s_hw, s_hh, s_hfull = 1;

static void syspal_init(void)
{
    if (s_syspalInit)
        return;
    s_syspalInit = 1;
    for (int i = 0; i < 10; i++) {
        s_syspal[i] = k_static[i];
        s_syspal[246 + i] = k_static[10 + i];
    }
    memcpy(s_defaultPalette.e, k_static, sizeof k_static);
    s_stockBitmap.s.bits = s_stockBits;
}

static void screen_sync_colors(void)
{
    for (int i = 0; i < 256; i++) {
        s_screen.colors[i].rgbRed = s_syspal[i].peRed;
        s_screen.colors[i].rgbGreen = s_syspal[i].peGreen;
        s_screen.colors[i].rgbBlue = s_syspal[i].peBlue;
        s_screen.colors[i].rgbReserved = 0;
    }
}

static int screen_alloc(int w, int h)
{
    syspal_init();
    if (s_screen.bits && s_screen.w == w && s_screen.h == h)
        return 0;
    unsigned char *bits = calloc((size_t)w * h, 1);
    Uint32 *rgb = calloc((size_t)w * h, 4);
    if (!bits || !rgb) {
        free(bits);
        free(rgb);
        return -1;
    }
    if (s_screen.bits)
        hires_gdi_forget(s_screen.bits);
    free(s_screen.bits);
    free(s_rgb);
    s_screen.bits = bits;
    s_rgb = rgb;
    s_screen.w = w;
    s_screen.h = h;
    s_screen.bpp = 8;
    s_screen.stride = w;
    s_screen.topdown = 1;
    s_screen.ncolors = 256;
    screen_sync_colors();
    hires_gdi_canvas(s_screen.bits, w, h, w, 1);
    return 0;
}

/* Window: resizable, letterboxed to the screen's aspect by the renderer's logical size
 * (which also maps mouse events back to screen coordinates), Alt+Enter / F11 full screen,
 * F9 the 3x art (hires.c) or the classic 1x frame. */
static volatile int s_reqToggle, s_reqFull;

static int SDLCALL key_watch(void *ud, SDL_Event *e)
{
    (void)ud;
    if (e->type == SDL_KEYDOWN && !e->key.repeat) {
        SDL_Keycode k = e->key.keysym.sym;
        if (k == SDLK_F9)
            s_reqToggle = 1;
        else if (k == SDLK_F11 || ((k == SDLK_RETURN || k == SDLK_KP_ENTER) && (e->key.keysym.mod & KMOD_ALT)))
            s_reqFull = 1;
    }
    return 1;
}

/* The game controller's pointer (vcl/forms.cpp registers its source): an arrow in screen
 * coordinates, drawn over the frame through the renderer's logical size. */
static int (*s_pointer)(int *x, int *y);

void port_set_pointer_source(int (*fn)(int *x, int *y)) { s_pointer = fn; }

static void draw_pointer(void)
{
    int x, y;
    if (!s_pointer || !s_pointer(&x, &y))
        return;
    static const SDL_Point outline[] = {{0, 0}, {0, 14}, {4, 10}, {10, 10}, {0, 0}};
    SDL_SetRenderDrawColor(s_renderer, 255, 255, 255, 255);
    for (int r = 1; r < 14; r++) {          /* between x = 0 and the arrow's right edge */
        int e = r <= 10 ? r - 1 : 10 - (r - 10) * 3 / 2 - 1;
        if (e >= 1)
            SDL_RenderDrawLine(s_renderer, x + 1, y + r, x + e, y + r);
    }
    SDL_Point p[5];
    for (int i = 0; i < 5; i++) {
        p[i].x = x + outline[i].x;
        p[i].y = y + outline[i].y;
    }
    SDL_SetRenderDrawColor(s_renderer, 0, 0, 0, 255);
    SDL_RenderDrawLines(s_renderer, p, 5);
}

/* $PORT_WINDOW_SIZE=WxH, else 1x, or up to 2x (85% of the display) when the 3x art shows. */
static void initial_window_size(int w, int h, int *ww, int *wh)
{
    *ww = w;
    *wh = h;
    const char *e = getenv("PORT_WINDOW_SIZE");
    int a, b;
    if (e && sscanf(e, "%dx%d", &a, &b) == 2 && a > 0 && b > 0) {
        *ww = a;
        *wh = b;
        return;
    }
    const char *drv = SDL_GetCurrentVideoDriver();
    SDL_Rect u;
    if (!hires_showing() || (drv && !strcmp(drv, "dummy")) || SDL_GetDisplayUsableBounds(0, &u))
        return;
    double k = 2.0, kx = 0.85 * u.w / w, ky = 0.85 * u.h / h;
    if (kx < k) k = kx;
    if (ky < k) k = ky;
    if (k > 1.0) {
        *ww = (int)(w * k);
        *wh = (int)(h * k);
    }
}

int port_init(int w, int h, const char *title)
{
    if (w <= 0 || h <= 0) {
        w = 640;
        h = 480;
    }
    int resized = !s_screen.bits || s_screen.w != w || s_screen.h != h;
    if (screen_alloc(w, h))
        return -1;
    if (port_sdl_init(SDL_INIT_VIDEO)) {
        port_log("port_init: no video (%s); running headless", SDL_GetError());
        return 0;
    }
    if (!s_window) {
        int ww, wh;
        initial_window_size(w, h, &ww, &wh);
        s_window = SDL_CreateWindow(title ? title : "Elf Bowling", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    ww, wh, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        if (!s_window) {
            port_log("port_init: SDL_CreateWindow failed (%s); running headless", SDL_GetError());
            return 0;
        }
        SDL_SetWindowMinimumSize(s_window, w / 2, h / 2);
        s_renderer = SDL_CreateRenderer(s_window, -1, 0);
        SDL_AddEventWatch(key_watch, NULL);
        const char *fs = getenv("PORT_FULLSCREEN");     /* =1: start full screen (the AppImage sets it) */
        if (fs && *fs == '1') {
            SDL_SetWindowFullscreen(s_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            port_log("full screen on");
        }
        resized = 1;
    }
    /* A later call (the form's bounds changed) keeps the window the player sized; the
     * picture follows through the logical size. */
    if (s_renderer && resized)
        SDL_RenderSetLogicalSize(s_renderer, w, h);
    if (s_texture && resized) {
        SDL_DestroyTexture(s_texture);
        s_texture = NULL;
    }
    if (s_renderer && !s_texture) {
        s_texture = SDL_CreateTexture(s_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
        if (s_texture)
            SDL_SetTextureScaleMode(s_texture, SDL_ScaleModeNearest);
    }
    return 0;
}

void port_gdi_shutdown(void)
{
    hires_stats_log();
    if (s_htex)
        SDL_DestroyTexture(s_htex);
    if (s_texture)
        SDL_DestroyTexture(s_texture);
    if (s_renderer)
        SDL_DestroyRenderer(s_renderer);
    if (s_window) {
        SDL_DelEventWatch(key_watch, NULL);
        SDL_DestroyWindow(s_window);
    }
    s_htex = NULL;
    s_texture = NULL;
    s_renderer = NULL;
    s_window = NULL;
    if (TTF_WasInit())
        TTF_Quit();
}

HWND port_main_hwnd(void) { return &s_mainWnd; }
void *port_sdl_window(void) { return s_window; }

unsigned char *port_screen_pixels(int *w, int *h)
{
    if (!s_screen.bits)
        screen_alloc(640, 480);
    if (w)
        *w = s_screen.w;
    if (h)
        *h = s_screen.h;
    return s_screen.bits;
}

const unsigned char *port_system_palette(void)
{
    syspal_init();
    return (const unsigned char *)s_syspal;
}

static void screen_convert(void)
{
    Uint32 lut[256];
    for (int i = 0; i < 256; i++)
        lut[i] = 0xff000000u | (Uint32)s_syspal[i].peRed << 16 | (Uint32)s_syspal[i].peGreen << 8 |
                 s_syspal[i].peBlue;
    size_t n = (size_t)s_screen.w * s_screen.h;
    for (size_t i = 0; i < n; i++)
        s_rgb[i] = lut[s_screen.bits[i]];
}

static void put_le(unsigned char *p, unsigned v, int n)
{
    for (int i = 0; i < n; i++)
        p[i] = (unsigned char)(v >> (8 * i));
}

/* 24-bit BMP from 32-bit pixels: fmt 0 = ARGB8888 words, 1 = bytes R,G,B,A. Colours are
 * scaled by mod/255. */
static int write_bmp(const char *path, const void *pixels, int w, int h, int pitch, int fmt, int mod)
{
    int stride = (w * 3 + 3) & ~3;
    unsigned char hdr[54] = {'B', 'M'};
    put_le(hdr + 2, 54 + stride * h, 4);
    put_le(hdr + 10, 54, 4);
    put_le(hdr + 14, 40, 4);
    put_le(hdr + 18, w, 4);
    put_le(hdr + 22, h, 4);
    put_le(hdr + 26, 1, 2);
    put_le(hdr + 28, 24, 2);
    put_le(hdr + 34, stride * h, 4);
    FILE *f = fopen(path, "wb");
    if (!f)
        return -1;
    fwrite(hdr, 1, sizeof hdr, f);
    unsigned char *line = calloc(stride, 1);
    for (int y = h - 1; line && y >= 0; y--) {
        const unsigned char *p = (const unsigned char *)pixels + (size_t)y * pitch;
        for (int x = 0; x < w; x++) {
            unsigned r, g, b;
            if (fmt == 0) {
                Uint32 c = ((const Uint32 *)p)[x];
                r = (c >> 16) & 0xff; g = (c >> 8) & 0xff; b = c & 0xff;
            } else {
                r = p[4 * x]; g = p[4 * x + 1]; b = p[4 * x + 2];
            }
            if (mod != 255) {
                r = r * mod / 255; g = g * mod / 255; b = b * mod / 255;
            }
            line[3 * x] = (unsigned char)b;
            line[3 * x + 1] = (unsigned char)g;
            line[3 * x + 2] = (unsigned char)r;
        }
        fwrite(line, 1, stride, f);
    }
    free(line);
    return fclose(f) == 0 ? 0 : -1;
}

/* $PORT_DUMP_FRAMES=N: every Nth presented frame is written to $PORT_DUMP_DIR/NNNN.bmp
 * (default build/frames; NNNN = the present count): what is presented, so 1920x1440 when
 * the 3x art shows. PORT_DUMP_WINDOW=1: the window's pixels instead (letterbox included). */
static int s_dumpWindow;

static const char *dump_path(void)
{
    static int every = -1, count;
    static char dir[512], path[600];
    if (every < 0) {
        const char *e = getenv("PORT_DUMP_FRAMES");
        every = e ? atoi(e) : 0;
        const char *d = getenv("PORT_DUMP_DIR");
        snprintf(dir, sizeof dir, "%s", d && *d ? d : "build/frames");
        const char *wd = getenv("PORT_DUMP_WINDOW");
        s_dumpWindow = wd && *wd == '1';
        if (every > 0) {
            for (char *q = dir; *q; q++) /* mkdir -p, one level at a time */
                if (*q == '/' && q != dir) {
                    *q = 0;
                    mkdir(dir, 0777);
                    *q = '/';
                }
            mkdir(dir, 0777);
            port_log("dumping every %d frames to %s/", every, dir);
        }
    }
    if (every <= 0)
        return NULL;
    int n = count++;
    if (n % every)
        return NULL;
    snprintf(path, sizeof path, "%s/%04d.bmp", dir, n);
    return path;
}

/* $PORT_WINPUT: scripted events in WINDOW coordinates, pushed through SDL's queue so the
 * renderer maps them as it maps real ones: "ms:click:x,y;ms:key:sym;ms:size:w,h", times from
 * the first present (tests of the letterbox mapping, F9 and F11). */
static void winput_step(void)
{
    typedef struct { Uint32 at; char kind; int a, b; } Ev;
    static Ev evs[64];
    static int n = -1, next;
    static Uint32 t0;
    if (n < 0) {
        n = 0;
        t0 = SDL_GetTicks();
        const char *p = getenv("PORT_WINPUT");
        while (p && *p && n < 64) {
            char kind[16] = {0};
            int used = 0;
            Ev ev = {0, 0, 0, 0};
            if (sscanf(p, "%u:%15[a-z]:%d,%d%n", &ev.at, kind, &ev.a, &ev.b, &used) < 4 || !used) {
                used = 0;
                if (sscanf(p, "%u:%15[a-z]:%d%n", &ev.at, kind, &ev.a, &used) < 3 || !used)
                    break;
            }
            ev.kind = kind[0];
            evs[n++] = ev;
            p += used;
            while (*p == ';' || *p == ' ')
                p++;
        }
        if (n)
            port_log("PORT_WINPUT: %d scripted window events", n);
    }
    while (next < n && SDL_GetTicks() - t0 >= evs[next].at) {
        Ev *ev = &evs[next++];
        SDL_Event e;
        Uint32 id = s_window ? SDL_GetWindowID(s_window) : 0;
        memset(&e, 0, sizeof e);
        if (ev->kind == 'c') {
            e.type = SDL_MOUSEMOTION;
            e.motion.windowID = id;
            e.motion.x = ev->a;
            e.motion.y = ev->b;
            SDL_PushEvent(&e);
            memset(&e, 0, sizeof e);
            e.type = SDL_MOUSEBUTTONDOWN;
            e.button.windowID = id;
            e.button.button = SDL_BUTTON_LEFT;
            e.button.state = SDL_PRESSED;
            e.button.clicks = 1;
            e.button.x = ev->a;
            e.button.y = ev->b;
            SDL_PushEvent(&e);
            e.type = SDL_MOUSEBUTTONUP;
            e.button.state = SDL_RELEASED;
            SDL_PushEvent(&e);
        } else if (ev->kind == 'k') {
            e.type = SDL_KEYDOWN;
            e.key.windowID = id;
            e.key.state = SDL_PRESSED;
            e.key.keysym.sym = ev->a;
            e.key.keysym.scancode = SDL_GetScancodeFromKey(ev->a);
            SDL_PushEvent(&e);
            e.type = SDL_KEYUP;
            e.key.state = SDL_RELEASED;
            SDL_PushEvent(&e);
        } else if (ev->kind == 's' && s_window) {
            SDL_SetWindowSize(s_window, ev->a, ev->b);
        }
        port_log("PORT_WINPUT: t=%u %c %d,%d", ev->at, ev->kind, ev->a, ev->b);
    }
}

/* The window (the web canvas's backing store) sized for the hi-res frame: scale x the screen, in
 * device pixels (the web page sets the canvas's CSS size itself). */
void port_hires_window(int scale, float dpr)
{
    if (!s_window || !s_screen.bits)
        return;
    if (dpr < 1)
        dpr = 1;
    int w = (int)(s_screen.w * scale / dpr), h = (int)(s_screen.h * scale / dpr);
    if (w < s_screen.w || h < s_screen.h) {
        w = s_screen.w;
        h = s_screen.h;
    }
    SDL_SetWindowSize(s_window, w, h);
    s_hfull = 1;
}

void port_present(void)
{
    if (!s_screen.bits)
        return;
    if (s_reqToggle) {
        s_reqToggle = 0;
        hires_toggle();
        s_hfull = 1;
    }
    if (s_reqFull) {
        s_reqFull = 0;
        if (s_window) {
            int fs = (SDL_GetWindowFlags(s_window) & SDL_WINDOW_FULLSCREEN) != 0;
            SDL_SetWindowFullscreen(s_window, fs ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
            port_log("full screen %s", fs ? "off" : "on");
        }
    }
    winput_step();
    {   /* ELFBOWL_HIRES_LATE=N[,scale]: the art starts at present N, as the web's pack does (tests) */
        static int late = -1, n;
        static char dir[600];
        if (late < 0) {
            const char *e = getenv("ELFBOWL_HIRES_LATE"), *d = getenv("ELFBOWL_HIRES");
            late = e ? atoi(e) : 0;
            snprintf(dir, sizeof dir, "%s", d && *d ? d : "build/hires/x3");
        }
        if (late > 0 && ++n == late) {
            const char *c = strchr(getenv("ELFBOWL_HIRES_LATE"), ',');
            hires_activate(dir, c ? atoi(c + 1) : 3);
        }
    }
    const unsigned int *hp = NULL;
    int w3 = 0, h3 = 0, y0 = 0, y1 = 0, mod = -1;
    if (hires_showing())
        hp = hires_screen((const unsigned char *)s_syspal, &w3, &h3, &y0, &y1, &mod);
    int hi = hp && mod >= 0;       /* else: no canvas, or a palette the art cannot follow */
    if (!hi) {
        screen_convert();
        s_hfull = 1;
    }
    const char *dump = dump_path();
    if (dump && !(s_dumpWindow && s_renderer)) {
        int r = hi ? write_bmp(dump, hp, w3, h3, w3 * 4, 1, mod)
                   : write_bmp(dump, s_rgb, s_screen.w, s_screen.h, s_screen.w * 4, 0, 255);
        if (r)
            port_log("frame dump %s failed", dump);
        const char *both = getenv("PORT_DUMP_1X");        /* the 1x frame too, for comparisons */
        if (hi && both && *both == '1') {
            char p1[640];
            snprintf(p1, sizeof p1, "%.*s_1x.bmp", (int)strlen(dump) - 4, dump);
            screen_convert();
            write_bmp(p1, s_rgb, s_screen.w, s_screen.h, s_screen.w * 4, 0, 255);
        }
    }
    if (!s_window)
        return;
    SDL_PumpEvents();
    if (s_renderer && s_texture) {
        SDL_SetRenderDrawColor(s_renderer, 0, 0, 0, 255);
        SDL_RenderClear(s_renderer);
        if (hi) {
            if (!s_htex || s_hw != w3 || s_hh != h3) {
                if (s_htex)
                    SDL_DestroyTexture(s_htex);
                s_htex = SDL_CreateTexture(s_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, w3, h3);
                if (s_htex)
                    SDL_SetTextureScaleMode(s_htex, SDL_ScaleModeLinear);
                s_hw = w3;
                s_hh = h3;
                s_hfull = 1;
            }
            if (s_hfull) {
                y0 = 0;
                y1 = h3;
                s_hfull = 0;
            }
            if (s_htex && y1 > y0) {
                SDL_Rect r = {0, y0, w3, y1 - y0};
                SDL_UpdateTexture(s_htex, &r, hp + (size_t)y0 * w3, w3 * 4);
            }
            if (s_htex) {
                SDL_SetTextureColorMod(s_htex, (Uint8)mod, (Uint8)mod, (Uint8)mod);
                SDL_RenderCopy(s_renderer, s_htex, NULL, NULL);
            }
        } else {
            SDL_UpdateTexture(s_texture, NULL, s_rgb, s_screen.w * 4);
            SDL_RenderCopy(s_renderer, s_texture, NULL, NULL);
        }
        if (dump && s_dumpWindow) {
            int ow = 0, oh = 0;
            SDL_GetRendererOutputSize(s_renderer, &ow, &oh);
            Uint32 *buf = ow > 0 && oh > 0 ? malloc((size_t)ow * oh * 4) : NULL;
            /* the read rectangle is relative to the viewport: read the whole output */
            int lw, lh;
            SDL_RenderGetLogicalSize(s_renderer, &lw, &lh);
            SDL_RenderSetLogicalSize(s_renderer, 0, 0);
            SDL_RenderSetScale(s_renderer, 1.0f, 1.0f);
            SDL_RenderSetViewport(s_renderer, NULL);
            if (buf && !SDL_RenderReadPixels(s_renderer, NULL, SDL_PIXELFORMAT_ARGB8888, buf, ow * 4))
                write_bmp(dump, buf, ow, oh, ow * 4, 0, 255);
            else
                port_log("window dump failed: %s", SDL_GetError());
            SDL_RenderSetLogicalSize(s_renderer, lw, lh);
            free(buf);
        }
        draw_pointer();
        SDL_RenderPresent(s_renderer);
    } else {
        if (hi)
            screen_convert();
        SDL_Surface *ws = SDL_GetWindowSurface(s_window);
        SDL_Surface *src = SDL_CreateRGBSurfaceWithFormatFrom(s_rgb, s_screen.w, s_screen.h, 32,
                                                              s_screen.w * 4, SDL_PIXELFORMAT_ARGB8888);
        if (ws && src) {
            SDL_BlitScaled(src, NULL, ws, NULL);
            SDL_UpdateWindowSurface(s_window);
        }
        SDL_FreeSurface(src);
    }
}

int port_screen_dump_bmp(const char *path)
{
    if (!s_screen.bits)
        return -1;
    screen_convert();
    return write_bmp(path, s_rgb, s_screen.w, s_screen.h, s_screen.w * 4, 0, 255);
}

/* ------------------------------------------------------------------ helpers */

static unsigned char *row(const Surface *s, int y)
{
    return s->bits + (size_t)(s->topdown ? y : s->h - 1 - y) * s->stride;
}

static Surface *dc_surface(DC *dc)
{
    if (!dc)
        return NULL;
    if (dc->kind == DC_SCREEN) {
        if (!s_screen.bits)
            screen_alloc(640, 480);
        return &s_screen;
    }
    if (dc->kind == DC_MEMORY && dc->bmp)
        return &dc->bmp->s;
    return NULL;
}

/* Nearest colour-table index: exact match first, else least squared distance,
 * ties to the lowest index. */
static int nearest(const Surface *s, int r, int g, int b)
{
    int best = 0, bestd = 0x7fffffff;
    for (int i = 0; i < s->ncolors; i++) {
        int dr = s->colors[i].rgbRed - r, dg = s->colors[i].rgbGreen - g, db = s->colors[i].rgbBlue - b;
        int d = dr * dr + dg * dg + db * db;
        if (d < bestd) {
            bestd = d;
            best = i;
            if (!d)
                break;
        }
    }
    return best;
}

/* Write one pixel of colour c (a COLORREF) at (x, y); caller clips. */
static void put_color(Surface *s, int x, int y, COLORREF c, int idx)
{
    unsigned char *p = row(s, y);
    switch (s->bpp) {
    case 8: p[x] = (unsigned char)idx; break;
    case 24: p += 3 * x; p[0] = GetBValue(c); p[1] = GetGValue(c); p[2] = GetRValue(c); break;
    case 32: p += 4 * x; p[0] = GetBValue(c); p[1] = GetGValue(c); p[2] = GetRValue(c); p[3] = 0; break;
    }
}

static int color_index(Surface *s, COLORREF c)
{
    return s->bpp == 8 ? nearest(s, GetRValue(c), GetGValue(c), GetBValue(c)) : 0;
}

static void fill(Surface *s, int l, int t, int r, int b, COLORREF c)
{
    if (l < 0) l = 0;
    if (t < 0) t = 0;
    if (r > s->w) r = s->w;
    if (b > s->h) b = s->h;
    int idx = color_index(s, c);
    for (int y = t; y < b; y++) {
        if (s->bpp == 8) {
            if (r > l)
                memset(row(s, y) + l, idx, r - l);
        } else {
            for (int x = l; x < r; x++)
                put_color(s, x, y, c, idx);
        }
    }
    if (s->bpp == 8)        /* mirrored exactly on a 3x canvas (hires.c) */
        hires_gdi_touch(s->bits, s->w, s->h, s->stride, s->topdown, l, t, r, b);
}

/* ------------------------------------------------------------------ DCs */

static DC *dc_new(int kind)
{
    syspal_init();
    DC *dc = calloc(1, sizeof *dc);
    if (!dc)
        return NULL;
    dc->kind = kind;
    dc->pal = &s_defaultPalette;
    dc->font = &s_systemFont;
    dc->brush = &s_stockBrush[WHITE_BRUSH];
    dc->text = 0x000000;
    dc->bk = 0xffffff;
    dc->bkmode = OPAQUE;
    if (kind == DC_MEMORY) {
        dc->bmp = &s_stockBitmap;
        s_stockBitmap.o.selected++;
    }
    return dc;
}

HDC WINAPI CreateCompatibleDC(HDC hdc)
{
    (void)hdc;
    return (HDC)dc_new(DC_MEMORY);
}

HDC WINAPI CreateICA(LPCSTR driver, LPCSTR device, LPCSTR port, const void *devmode)
{
    (void)device; (void)port; (void)devmode;
    if (driver && SDL_strcasecmp(driver, "DISPLAY"))
        port_log("CreateICA(\"%s\"): treated as DISPLAY", driver);
    return (HDC)dc_new(DC_INFO);
}

static void dc_release_objects(DC *dc)
{
    if (dc->bmp)
        dc->bmp->o.selected--;
    if (dc->font)
        dc->font->o.selected--;
    if (dc->brush)
        dc->brush->o.selected--;
    if (dc->pal)
        dc->pal->o.selected--;
}

BOOL WINAPI DeleteDC(HDC hdc)
{
    DC *dc = (DC *)hdc;
    if (!dc)
        return FALSE;
    dc_release_objects(dc);
    free(dc);
    return TRUE;
}

HDC WINAPI GetDC(HWND hwnd)
{
    (void)hwnd;     /* one window: every HWND (and NULL) maps to the screen */
    DC *dc = dc_new(DC_SCREEN);
    if (dc)
        dc_surface(dc);
    return (HDC)dc;
}

int WINAPI ReleaseDC(HWND hwnd, HDC hdc)
{
    (void)hwnd;
    DC *dc = (DC *)hdc;
    if (!dc || dc->kind != DC_SCREEN)
        return 0;
    return DeleteDC(hdc);
}

int WINAPI GetDeviceCaps(HDC hdc, int index)
{
    (void)hdc;
    int w = s_screen.bits ? s_screen.w : 640, h = s_screen.bits ? s_screen.h : 480;
    switch (index) {
    case BITSPIXEL: return 8;       /* the port's screen is palettized 8-bit */
    case PLANES: return 1;
    case HORZRES: return w;
    case VERTRES: return h;
    case NUMCOLORS: return 20;
    case SIZEPALETTE: return 256;
    case NUMRESERVED: return 20;
    case RASTERCAPS: return RC_BITBLT | RC_PALETTE | RC_DI_BITMAP;
    default:
        port_log("GetDeviceCaps(%d): unknown index, returning 0", index);
        return 0;
    }
}

/* ------------------------------------------------------------------ objects */

HGDIOBJ WINAPI GetStockObject(int i)
{
    syspal_init();
    if (i >= WHITE_BRUSH && i <= NULL_BRUSH)
        return &s_stockBrush[i];
    if (i == SYSTEM_FONT)
        return &s_systemFont;
    if (i == DEFAULT_PALETTE)
        return &s_defaultPalette;
    port_log("GetStockObject(%d): unsupported", i);
    return NULL;
}

static void swap_sel(Obj **slot, Obj *o)
{
    if (*slot)
        (*slot)->selected--;
    o->selected++;
    *slot = o;
}

HGDIOBJ WINAPI SelectObject(HDC hdc, HGDIOBJ h)
{
    DC *dc = (DC *)hdc;
    Obj *o = (Obj *)h;
    Obj *prev;
    if (!dc || !o)
        return NULL;
    switch (o->type) {
    case T_BITMAP:
        if (dc->kind != DC_MEMORY)
            return NULL;
        if (o->selected && (Obj *)dc->bmp != o && !o->stock)
            return NULL;    /* a bitmap can be selected into one DC at a time */
        prev = (Obj *)dc->bmp;
        swap_sel((Obj **)&dc->bmp, o);
        return prev;
    case T_FONT:
        prev = (Obj *)dc->font;
        swap_sel((Obj **)&dc->font, o);
        return prev;
    case T_BRUSH:
        prev = (Obj *)dc->brush;
        swap_sel((Obj **)&dc->brush, o);
        return prev;
    default:
        return NULL;        /* palettes go through SelectPalette, as on Windows */
    }
}

BOOL WINAPI DeleteObject(HGDIOBJ h)
{
    Obj *o = (Obj *)h;
    if (!o)
        return FALSE;
    if (o->stock)
        return TRUE;
    if (o->selected) {
        port_log("DeleteObject(%p): still selected into a DC; not deleted", h);
        return FALSE;
    }
    if (o->type == T_BITMAP) {
        hires_gdi_forget(((Bitmap *)o)->s.bits);
        free(((Bitmap *)o)->s.bits);
    }
    if (o->type == T_FONT && ((Font *)o)->ttf)
        TTF_CloseFont(((Font *)o)->ttf);
    if (o->type == T_FONT && ((Font *)o)->ttf3)
        TTF_CloseFont(((Font *)o)->ttf3);
    o->type = 0;
    free(o);
    return TRUE;
}

HBITMAP WINAPI CreateDIBSection(HDC hdc, const BITMAPINFO *bmi, UINT usage, void **bits,
                                HANDLE section, DWORD offset)
{
    (void)hdc; (void)offset;
    if (bits)
        *bits = NULL;
    if (!bmi)
        return NULL;
    const BITMAPINFOHEADER *bh = &bmi->bmiHeader;
    int w = bh->biWidth, h = bh->biHeight, bpp = bh->biBitCount;
    if (w <= 0 || h == 0 || (bpp != 8 && bpp != 24 && bpp != 32) || bh->biCompression != BI_RGB) {
        port_log("CreateDIBSection: unsupported %dx%d %d bpp compression %u", w, h, bpp,
                 (unsigned)bh->biCompression);
        return NULL;
    }
    if (section)
        port_log("CreateDIBSection: file-mapping section ignored");
    if (usage != DIB_RGB_COLORS)
        port_log("CreateDIBSection: DIB_PAL_COLORS unsupported, colour table treated as RGB");
    Bitmap *b = calloc(1, sizeof *b);
    if (!b)
        return NULL;
    b->o.type = T_BITMAP;
    b->s.w = w;
    b->s.h = h < 0 ? -h : h;
    b->s.topdown = h < 0;
    b->s.bpp = bpp;
    b->s.stride = ((w * bpp + 31) / 32) * 4;
    b->s.bits = calloc((size_t)b->s.stride * b->s.h, 1);
    if (!b->s.bits) {
        free(b);
        return NULL;
    }
    if (bpp == 8) {
        int n = bh->biClrUsed ? (int)bh->biClrUsed : 256;
        if (n > 256)
            n = 256;
        const RGBQUAD *ct = (const RGBQUAD *)((const unsigned char *)bmi + bh->biSize);
        memcpy(b->s.colors, ct, n * sizeof(RGBQUAD));
        b->s.ncolors = n;
    }
    if (bits)
        *bits = b->s.bits;
    return (HBITMAP)b;
}

/* ------------------------------------------------------------------ palettes */

HPALETTE WINAPI CreatePalette(const LOGPALETTE *lp)
{
    if (!lp || lp->palNumEntries == 0 || lp->palNumEntries > 256)
        return NULL;
    Palette *p = calloc(1, sizeof *p);
    if (!p)
        return NULL;
    p->o.type = T_PALETTE;
    p->n = lp->palNumEntries;
    memcpy(p->e, lp->palPalEntry, p->n * sizeof(PALETTEENTRY));
    return (HPALETTE)p;
}

HPALETTE WINAPI SelectPalette(HDC hdc, HPALETTE hpal, BOOL forceBackground)
{
    DC *dc = (DC *)hdc;
    Palette *p = (Palette *)hpal;
    if (!dc || !p || p->o.type != T_PALETTE)
        return NULL;
    Palette *prev = dc->pal;
    swap_sel((Obj **)&dc->pal, &p->o);
    dc->palBackground = forceBackground;
    return (HPALETTE)prev;
}

UINT WINAPI RealizePalette(HDC hdc)
{
    DC *dc = (DC *)hdc;
    if (!dc || !dc->pal)
        return 0;
    /* Foreground realization of a logical palette writes it into the system
     * palette 1:1 (no static-colour reservation). The stock palette and
     * background realizations leave the system palette alone. */
    if (dc->pal->o.stock || dc->palBackground)
        return 0;
    for (int i = 0; i < dc->pal->n; i++) {
        s_syspal[i] = dc->pal->e[i];
        s_syspal[i].peFlags = 0;
    }
    screen_sync_colors();
    return dc->pal->n;
}

/* ------------------------------------------------------------------ blits */

/* Convert one source row segment to the destination's pixel format. */
static void convert_row(const Surface *d, const Surface *s, const unsigned char *sp, int n, unsigned char *out)
{
    int db = d->bpp / 8, sb = s->bpp / 8;
    for (int i = 0; i < n; i++) {
        int r, g, b;
        if (sb == 1) {
            if (db == 1) {      /* index to index: identity (see docs/SHIM.md) */
                out[i] = sp[i];
                continue;
            }
            const RGBQUAD *q = &s->colors[sp[i]];
            r = q->rgbRed; g = q->rgbGreen; b = q->rgbBlue;
        } else {
            b = sp[i * sb]; g = sp[i * sb + 1]; r = sp[i * sb + 2];
        }
        if (db == 1) {
            out[i] = (unsigned char)nearest(d, r, g, b);
        } else {
            out[i * db] = (unsigned char)b;
            out[i * db + 1] = (unsigned char)g;
            out[i * db + 2] = (unsigned char)r;
            if (db == 4)
                out[i * db + 3] = 0;
        }
    }
}

BOOL WINAPI BitBlt(HDC hdst, int x, int y, int w, int h, HDC hsrc, int sx, int sy, DWORD rop)
{
    Surface *d = dc_surface((DC *)hdst);
    if (!d)
        return FALSE;
    int needSrc = rop != BLACKNESS && rop != WHITENESS;
    Surface *s = needSrc ? dc_surface((DC *)hsrc) : NULL;
    if (needSrc && (!s || s->bpp == 0))
        return FALSE;
    if (rop != SRCCOPY && rop != SRCAND && rop != SRCPAINT && rop != SRCINVERT && rop != NOTSRCCOPY &&
        needSrc) {
        port_log("BitBlt: unsupported rop %08x", (unsigned)rop);
        return FALSE;
    }
    /* clip to the destination, then to the source */
    if (x < 0) { sx -= x; w += x; x = 0; }
    if (y < 0) { sy -= y; h += y; y = 0; }
    if (x + w > d->w) w = d->w - x;
    if (y + h > d->h) h = d->h - y;
    if (s) {
        if (sx < 0) { x -= sx; w += sx; sx = 0; }
        if (sy < 0) { y -= sy; h += sy; sy = 0; }
        if (sx + w > s->w) w = s->w - sx;
        if (sy + h > s->h) h = s->h - sy;
    }
    if (w <= 0 || h <= 0)
        return TRUE;
    int db = d->bpp / 8, n = w * db;
    unsigned char *tmp = s ? malloc(n) : NULL;
    if (s && !tmp)
        return FALSE;
    int mirror = d->bpp == 8 && (!s || s->bpp == 8);   /* the 3x canvas follows (hires.c) */
    if (mirror)
        hires_gdi_blit(0, d->bits, d->w, d->h, d->stride, d->topdown, s ? s->bits : NULL, s ? s->w : 0,
                       s ? s->h : 0, s ? s->stride : 0, s ? s->topdown : 0, x, y, sx, sy, w, h, rop);
    for (int j = 0; j < h; j++) {
        unsigned char *dp = row(d, y + j) + x * db;
        if (!s) {
            memset(dp, rop == BLACKNESS ? 0x00 : 0xff, n);
            continue;
        }
        convert_row(d, s, row(s, sy + j) + sx * (s->bpp / 8), w, tmp);
        switch (rop) {
        case SRCCOPY: memmove(dp, tmp, n); break;
        case SRCAND: for (int i = 0; i < n; i++) dp[i] &= tmp[i]; break;
        case SRCPAINT: for (int i = 0; i < n; i++) dp[i] |= tmp[i]; break;
        case SRCINVERT: for (int i = 0; i < n; i++) dp[i] ^= tmp[i]; break;
        case NOTSRCCOPY: for (int i = 0; i < n; i++) dp[i] = (unsigned char)~tmp[i]; break;
        }
    }
    free(tmp);
    if (mirror)
        hires_gdi_blit(1, d->bits, d->w, d->h, d->stride, d->topdown, s ? s->bits : NULL, s ? s->w : 0,
                       s ? s->h : 0, s ? s->stride : 0, s ? s->topdown : 0, x, y, sx, sy, w, h, rop);
    return TRUE;
}

int WINAPI FillRect(HDC hdc, const RECT *r, HBRUSH hbr)
{
    Surface *s = dc_surface((DC *)hdc);
    Brush *b = (Brush *)hbr;
    if (!s || !r || !b)
        return 0;
    if ((uintptr_t)b <= 64) {           /* (HBRUSH)(COLOR_xxx + 1) system colour */
        port_log("FillRect: system-colour brush %d drawn as grey", (int)(uintptr_t)b);
        fill(s, r->left, r->top, r->right, r->bottom, 0xc0c0c0);
        return 1;
    }
    if (b->o.type != T_BRUSH)
        return 0;
    if (!b->hollow)
        fill(s, r->left, r->top, r->right, r->bottom, b->color);
    return 1;
}

/* ------------------------------------------------------------------ text */

COLORREF WINAPI SetTextColor(HDC hdc, COLORREF c)
{
    DC *dc = (DC *)hdc;
    if (!dc)
        return 0xffffffffu;     /* CLR_INVALID */
    COLORREF prev = dc->text;
    dc->text = c & 0xffffff;
    return prev;
}

int WINAPI SetBkMode(HDC hdc, int mode)
{
    DC *dc = (DC *)hdc;
    if (!dc || (mode != OPAQUE && mode != TRANSPARENT))
        return 0;
    int prev = dc->bkmode;
    dc->bkmode = mode;
    return prev;
}

/* Font files per face. The game asks for Arial (weights 200 and 700) and for
 * Lucida Handwriting / Lucida Calligraphy (italic-only faces, weight 600). */
typedef struct FaceInfo {
    const char *face;
    int italicOnly;
    const char *regular[6];
    const char *bold[6];
} FaceInfo;

static const FaceInfo k_faces[] = {
    {"Arial", 0,
     {"Arial.ttf", "arial.ttf", "LiberationSans-Regular.ttf", "DejaVuSans.ttf", NULL},
     {"Arial Bold.ttf", "arialbd.ttf", "LiberationSans-Bold.ttf", "DejaVuSans-Bold.ttf", NULL}},
    {"Lucida Handwriting", 1,
     {"LHANDW.TTF", "lhandw.ttf", "Apple Chancery.ttf", "URWChanceryL-MediItal.ttf", "Z003-MediumItalic.ttf", NULL},
     {NULL}},
    {"Lucida Calligraphy", 1,
     {"LCALLIG.TTF", "lcallig.ttf", "Apple Chancery.ttf", "URWChanceryL-MediItal.ttf", "Z003-MediumItalic.ttf", NULL},
     {NULL}},
};
#define NFACES ((int)(sizeof k_faces / sizeof k_faces[0]))

static const FaceInfo *face_info(const char *face)
{
    for (int i = 0; i < NFACES; i++)
        if (!SDL_strcasecmp(face, k_faces[i].face))
            return &k_faces[i];
    return NULL;
}

static int find_font_file(const char *name, char *out, size_t outsz)
{
    const char *home = getenv("HOME");
    char homefonts[512] = "";
    if (home)
        snprintf(homefonts, sizeof homefonts, "%s/Library/Fonts", home);
    const char *dirs[] = {getenv("PORT_FONT_DIR"), "/System/Library/Fonts/Supplemental", "/System/Library/Fonts",
                          "/Library/Fonts", homefonts, "/usr/share/fonts/truetype/dejavu",
                          "/usr/share/fonts/truetype/liberation", "/usr/share/fonts/TTF",
                          "/usr/share/fonts/dejavu", "/usr/share/fonts/urw-base35", "C:/Windows/Fonts"};
    for (size_t i = 0; i < sizeof dirs / sizeof dirs[0]; i++) {
        if (!dirs[i] || !*dirs[i])
            continue;
        snprintf(out, outsz, "%s/%s", dirs[i], name);
        FILE *f = fopen(out, "rb");
        if (f) {
            fclose(f);
            return 1;
        }
    }
    return 0;
}

static int find_in_list(const char *const *list, char *out, size_t outsz)
{
    for (int i = 0; list[i]; i++)
        if (find_font_file(list[i], out, outsz))
            return 1;
    return 0;
}

static TTF_Font *font_ttf(Font *f)
{
    if (f->ttf || f->tried)
        return f->ttf;
    f->tried = 1;
    if (!TTF_WasInit() && TTF_Init()) {
        port_log("TTF_Init failed: %s", TTF_GetError());
        return NULL;
    }
    const FaceInfo *fi = face_info(f->lf.lfFaceName);
    if (!fi)
        fi = &k_faces[0];
    int wantBold = f->lf.lfWeight >= FW_SEMIBOLD, synthBold = 0;
    char path[1024];
    int found = 0;
    if (wantBold && fi->bold[0])
        found = find_in_list(fi->bold, path, sizeof path);
    if (!found) {
        found = find_in_list(fi->regular, path, sizeof path);
        synthBold = wantBold;
    }
    if (!found && fi != &k_faces[0]) {
        found = find_in_list(k_faces[0].regular, path, sizeof path);
        synthBold = wantBold;
    }
    if (!found) {
        port_log("font \"%s\": no TrueType file found (set PORT_FONT_DIR); text will not draw",
                 f->lf.lfFaceName);
        return NULL;
    }
    int height = f->lf.lfHeight;
    int pt = height > 0 ? height : height < 0 ? -height : 16;
    f->ttf = TTF_OpenFont(path, pt);
    if (!f->ttf) {
        port_log("font \"%s\": TTF_OpenFont(%s) failed: %s", f->lf.lfFaceName, path, TTF_GetError());
        return NULL;
    }
    if (height > 0) {   /* positive lfHeight is the cell height (ascent + descent) */
        int h0 = TTF_FontHeight(f->ttf);
        if (h0 > 0 && h0 != height) {
            int pt2 = (height * height + h0 / 2) / h0;
            pt = pt2 > 0 ? pt2 : 1;
            TTF_SetFontSize(f->ttf, pt);
        }
    }
    int style = (synthBold ? TTF_STYLE_BOLD : 0) |
                (f->lf.lfItalic && !fi->italicOnly ? TTF_STYLE_ITALIC : 0) |
                (f->lf.lfUnderline ? TTF_STYLE_UNDERLINE : 0) | (f->lf.lfStrikeOut ? TTF_STYLE_STRIKETHROUGH : 0);
    TTF_SetFontStyle(f->ttf, style);
    f->pt = pt;
    f->style = style;
    snprintf(f->path, sizeof f->path, "%s", path);
    port_log("font \"%s\" h=%d w=%d -> %s (height %d%s)", f->lf.lfFaceName, (int)f->lf.lfHeight,
             (int)f->lf.lfWeight, path, TTF_FontHeight(f->ttf), synthBold ? ", synthetic bold" : "");
    return f->ttf;
}

/* The same face and style at the canvas scale (hires.c: 3, or 2). */
static TTF_Font *font_ttf3(Font *f)
{
    if (f->ttf3 || f->tried3 || !font_ttf(f))
        return f->ttf3;
    f->tried3 = 1;
    f->ttf3 = TTF_OpenFont(f->path, f->pt * hires_scale());
    if (f->ttf3)
        TTF_SetFontStyle(f->ttf3, f->style);
    return f->ttf3;
}

HFONT WINAPI CreateFontIndirectA(const LOGFONTA *lf)
{
    if (!lf)
        return NULL;
    Font *f = calloc(1, sizeof *f);
    if (!f)
        return NULL;
    f->o.type = T_FONT;
    f->lf = *lf;
    f->lf.lfFaceName[LF_FACESIZE - 1] = 0;
    return (HFONT)f;
}

int WINAPI EnumFontFamiliesA(HDC hdc, LPCSTR family, FONTENUMPROCA proc, LPARAM lp)
{
    (void)hdc;
    static const char *const arialStyles[] = {"Regular", "Bold", "Italic", "Bold Italic"};
    int r = 1;
    for (int i = 0; i < NFACES && r; i++) {
        const FaceInfo *fi = &k_faces[i];
        if (family && SDL_strcasecmp(family, fi->face))
            continue;
        int nstyles = fi->italicOnly ? 1 : 4;
        for (int s = 0; s < nstyles && r; s++) {
            const char *style = fi->italicOnly ? "Italic" : arialStyles[s];
            ENUMLOGFONTA elf;
            TEXTMETRICA tm;
            memset(&elf, 0, sizeof elf);
            memset(&tm, 0, sizeof tm);
            LOGFONTA *l = &elf.elfLogFont;
            l->lfHeight = 16;
            l->lfWeight = strstr(style, "Bold") ? FW_BOLD : FW_NORMAL;
            l->lfItalic = strstr(style, "Italic") != NULL;
            l->lfCharSet = ANSI_CHARSET;
            l->lfOutPrecision = 3;      /* OUT_STROKE_PRECIS */
            l->lfClipPrecision = 2;     /* CLIP_STROKE_PRECIS */
            l->lfQuality = 1;           /* DRAFT_QUALITY */
            l->lfPitchAndFamily = fi->italicOnly ? 0x42 : 0x22;   /* VARIABLE | FF_SCRIPT / FF_SWISS */
            SDL_strlcpy(l->lfFaceName, fi->face, LF_FACESIZE);
            if (!strcmp(style, "Regular"))
                SDL_strlcpy((char *)elf.elfFullName, fi->face, LF_FULLFACESIZE);
            else
                snprintf((char *)elf.elfFullName, LF_FULLFACESIZE, "%s %s", fi->face, style);
            SDL_strlcpy((char *)elf.elfStyle, style, LF_FACESIZE);
            tm.tmHeight = 16;
            tm.tmAscent = 13;
            tm.tmDescent = 3;
            tm.tmWeight = l->lfWeight;
            tm.tmItalic = l->lfItalic;
            tm.tmLastChar = 0xff;
            tm.tmBreakChar = ' ';
            tm.tmPitchAndFamily = l->lfPitchAndFamily | 4;   /* TMPF_TRUETYPE */
            r = proc((const LOGFONTA *)&elf, &tm, TRUETYPE_FONTTYPE, lp);
        }
    }
    return r;
}

/* Strip '&' prefixes as DrawText does without DT_NOPREFIX ("&&" -> "&"). */
static char *text_prep(const char *text, int n, UINT fmt)
{
    if (n < 0)
        n = (int)strlen(text);
    char *out = malloc(n + 1), *o = out;
    if (!out)
        return NULL;
    for (int i = 0; i < n && text[i]; i++) {
        if (text[i] == '&' && !(fmt & DT_NOPREFIX)) {
            if (i + 1 < n && text[i + 1] == '&')
                i++;
            else
                continue;
        }
        *o++ = text[i];
    }
    *o = 0;
    return out;
}

static int text_width(TTF_Font *f, const char *s, int len)
{
    char buf[1024];
    int w = 0;
    if (len <= 0)
        return 0;
    if (len >= (int)sizeof buf)
        len = sizeof buf - 1;
    memcpy(buf, s, len);
    buf[len] = 0;
    if (f)
        TTF_SizeText(f, buf, &w, NULL);
    return w;
}

typedef struct Line { const char *p; int len; } Line;

/* Break text into lines: hard breaks at \n (\r ignored), and word breaks at
 * spaces when DT_WORDBREAK is set. A word wider than the rectangle stays whole. */
static int layout(TTF_Font *f, const char *t, UINT fmt, int maxw, Line *lines, int maxlines)
{
    int n = 0;
    const char *p = t;
    while (*p && n < maxlines) {
        const char *e = p;
        while (*e && *e != '\n' && *e != '\r')
            e++;
        if (fmt & DT_SINGLELINE) {
            e = p + strlen(p);
        }
        /* p..e is one paragraph */
        if (!(fmt & DT_WORDBREAK) || (fmt & DT_SINGLELINE)) {
            lines[n].p = p;
            lines[n].len = (int)(e - p);
            n++;
        } else {
            const char *ls = p;
            while (ls < e && n < maxlines) {
                const char *best = NULL, *q = ls;
                for (;;) {
                    const char *we = q;
                    while (we < e && *we != ' ')
                        we++;
                    if (text_width(f, ls, (int)(we - ls)) > maxw && best)
                        break;
                    best = we;
                    if (we >= e)
                        break;
                    q = we + 1;
                }
                lines[n].p = ls;
                lines[n].len = (int)(best - ls);
                n++;
                ls = best;
                while (ls < e && *ls == ' ')
                    ls++;
            }
            if (p == e) {   /* empty paragraph */
                lines[n].p = p;
                lines[n].len = 0;
                n++;
            }
        }
        p = e;
        if (*p == '\r')
            p++;
        if (*p == '\n')
            p++;
    }
    return n;
}

/* The line just drawn at 1x (at x0, y, bw x bh, clipped to cl..cr x ct..cb), again with the font
 * at the canvas scale (3x), antialiased, as a coverage plane for the 3x canvas. The 1x layout is kept: each line
 * is placed where the 1x line is, centred on it. */
static void text3(DC *dc, Surface *s, const char *buf, int x0, int y, int bw, int bh, int w, int lh, int cl,
                  int ct, int cr, int cb, int ti)
{
    TTF_Font *f3 = font_ttf3(dc->font);
    int k = hires_scale(), m = 4;       /* m: the scaled line may run a few pixels wider than the 1x */
    int l = x0 - m > cl ? x0 - m : cl, t = y > ct ? y : ct;
    int r = x0 + bw + m < cr ? x0 + bw + m : cr, b = y + bh < cb ? y + bh : cb;
    if (!f3 || l >= r || t >= b)
        return;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *g = TTF_RenderText_Blended(f3, buf, white);
    if (!g)
        return;
    int W = (r - l) * k, Hh = (b - t) * k;
    unsigned char *cov = calloc((size_t)W * Hh, 1);
    if (cov && g->format->BytesPerPixel == 4) {
        int w3 = 0;
        TTF_SizeText(f3, buf, &w3, NULL);
        int X0 = k * x0 + (k * w - w3) / 2 - k * l, Y0 = k * y + (k * lh - TTF_FontHeight(f3)) / 2 - k * t;
        SDL_LockSurface(g);
        for (int gy = 0; gy < g->h; gy++) {
            int Y = Y0 + gy;
            if (Y < 0 || Y >= Hh)
                continue;
            const Uint32 *gp = (const Uint32 *)((const unsigned char *)g->pixels + gy * g->pitch);
            for (int gx = 0; gx < g->w; gx++) {
                int X = X0 + gx;
                if (X >= 0 && X < W)
                    cov[(size_t)Y * W + X] = (unsigned char)((gp[gx] & g->format->Amask) >> g->format->Ashift);
            }
        }
        SDL_UnlockSurface(g);
        hires_gdi_text(s->bits, s->w, s->h, s->stride, s->topdown, l, t, r, b, cov, W, (unsigned char)ti,
                       (unsigned char)color_index(s, dc->bk));
    }
    free(cov);
    SDL_FreeSurface(g);
}

int WINAPI DrawTextA(HDC hdc, LPCSTR text, int n, LPRECT rc, UINT fmt)
{
    DC *dc = (DC *)hdc;
    if (!dc || !text || !rc)
        return 0;
    char *t = text_prep(text, n, fmt);
    if (!t)
        return 0;
    TTF_Font *f = font_ttf(dc->font);
    int lh = f ? TTF_FontHeight(f) : (dc->font->lf.lfHeight > 0 ? dc->font->lf.lfHeight : 16);
    Line lines[256];
    int nl = *t ? layout(f, t, fmt, rc->right - rc->left, lines, 256) : 0;
    int height = nl * lh, maxw = 0;
    for (int i = 0; i < nl; i++) {
        int w = text_width(f, lines[i].p, lines[i].len);
        if (w > maxw)
            maxw = w;
    }
    if (fmt & DT_CALCRECT) {
        rc->right = rc->left + maxw;
        rc->bottom = rc->top + height;
        free(t);
        return height;
    }
    Surface *s = dc_surface(dc);
    if (!s || !f) {
        free(t);
        return height;
    }
    int y0 = rc->top;
    if (fmt & DT_SINGLELINE) {
        if (fmt & DT_BOTTOM)
            y0 = rc->bottom - lh;
        else if (fmt & DT_VCENTER)
            y0 = rc->top + (rc->bottom - rc->top - lh) / 2;
    }
    int cl = 0, ct = 0, cr = s->w, cb = s->h;
    if (!(fmt & DT_NOCLIP)) {
        if (rc->left > cl) cl = rc->left;
        if (rc->top > ct) ct = rc->top;
        if (rc->right < cr) cr = rc->right;
        if (rc->bottom < cb) cb = rc->bottom;
    }
    int ti = color_index(s, dc->text);
    SDL_Color white = {255, 255, 255, 255};
    for (int i = 0; i < nl; i++) {
        int w = text_width(f, lines[i].p, lines[i].len);
        int x0 = rc->left, y = y0 + i * lh;
        if (fmt & DT_CENTER)
            x0 = rc->left + (rc->right - rc->left - w) / 2;
        else if (fmt & DT_RIGHT)
            x0 = rc->right - w;
        if (dc->bkmode == OPAQUE && w > 0) {
            int l = x0 > cl ? x0 : cl, tt = y > ct ? y : ct;
            int r = x0 + w < cr ? x0 + w : cr, b = y + lh < cb ? y + lh : cb;
            if (l < r && tt < b)
                fill(s, l, tt, r, b, dc->bk);
        }
        if (!lines[i].len)
            continue;
        char buf[1024];
        int len = lines[i].len < (int)sizeof buf - 1 ? lines[i].len : (int)sizeof buf - 1;
        memcpy(buf, lines[i].p, len);
        buf[len] = 0;
        /* Solid (non-antialiased) rendering: Windows does not smooth text on 8-bit targets. */
        SDL_Surface *g = TTF_RenderText_Solid(f, buf, white);
        if (!g)
            continue;
        SDL_LockSurface(g);
        for (int gy = 0; gy < g->h; gy++) {
            int py = y + gy;
            if (py < ct || py >= cb)
                continue;
            const unsigned char *gp = (const unsigned char *)g->pixels + gy * g->pitch;
            for (int gx = 0; gx < g->w; gx++) {
                int px = x0 + gx;
                if (gp[gx] && px >= cl && px < cr)
                    put_color(s, px, py, dc->text, ti);
            }
        }
        SDL_UnlockSurface(g);
        if (s->bpp == 8 && dc->bkmode == OPAQUE && hires_text_wanted())
            text3(dc, s, buf, x0, y, w > g->w ? w : g->w, lh > g->h ? lh : g->h, w, lh, cl, ct, cr, cb, ti);
        SDL_FreeSurface(g);
    }
    free(t);
    return height;
}

/* ------------------------------------------------------------------ windows */

HWND WINAPI FindWindowA(LPCSTR cls, LPCSTR title)
{
    PORT_STUB("FindWindowA", "(\"%s\", \"%s\") -> NULL", cls ? cls : "(null)", title ? title : "(null)");
    return NULL;
}

BOOL WINAPI ShowWindow(HWND hwnd, int cmd)
{
    if (hwnd == &s_mainWnd && s_window) {
        if (cmd == SW_HIDE)
            SDL_HideWindow(s_window);
        else
            SDL_ShowWindow(s_window);
        return TRUE;
    }
    PORT_STUB("ShowWindow", "(%p, %d) ignored", (void *)hwnd, cmd);
    return FALSE;
}
