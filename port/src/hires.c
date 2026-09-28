/* hires.c - 3x high-resolution shadow surfaces (see hires.h and docs/HIRES.md).
 *
 * Every 8-bit DIB the game touches can have an HSurf, found by its bits pointer:
 *
 *  - a canvas (the screen, TStage's back and front buffers) keeps px, a 3x RGBA copy,
 *    and ref, the 1x indices px was made from. Mirrored draws update both. Any pixel
 *    whose 1x index no longer equals ref was changed by something not mirrored: sync()
 *    redraws its 3x3 block from the index (nearest neighbour). So the canvas is always
 *    a faithful 3x rendering of the 1x surface, pixel by pixel.
 *  - a source (a cast's bitmap) has a registration (Reg): which upscaled asset it was
 *    loaded from, flipped and cropped how, or subsampled from which cast. src_image()
 *    turns it and its mask into a 3x RGBA image with the art's smooth alpha, checking it
 *    against the 1x pixels and falling back to 3x nearest where they disagree.
 *
 * Draws: SRCCOPY copies 3x blocks; SRCAND (mask) then SRCPAINT (sprite) at the same
 * place is an alpha composite, accepted per 1x pixel only where the 1x result is what
 * the composite means (sprite pixel where the mask is opaque, the old pixel elsewhere);
 * fills are mirrored exactly; anything else refreshes nearest. The 1x path is never
 * changed: the classic frame is always correct, and the 3x one follows it.
 *
 * Colours: canvases hold RGB through the art's palette (the game's one palette). At
 * present, a realized palette that is a uniform scale of it (a fade) becomes a colour
 * modulation; any other difference shows the classic frame. */
#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hires.h"
#include "port.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "third_party/stb_image.h"
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

static int g_scale = 3;         /* the canvas scale: 3 (the art's), or 2 (phones on the web, to save memory) */
#define S3 g_scale
#define ASSET_BG 255            /* background index the art's masks were made with (tools/upscale.py) */
#define ROP_SRCCOPY 0x00CC0020u
#define ROP_SRCAND 0x008800C6u
#define ROP_SRCPAINT 0x00EE0086u

typedef uint32_t px_t;          /* bytes R, G, B, A (little-endian: A in the top byte) */
#define PX_A(c) ((c) >> 24)
#define PX(r, g, b, a) ((px_t)(r) | (px_t)(g) << 8 | (px_t)(b) << 16 | (px_t)(a) << 24)

/* ------------------------------------------------------------------ state */

typedef struct Asset {
    char name[40];
    int w, h, nvar;             /* masked variants: 1 (m1) or 2 (m1, m2) */
    int has_op;                 /* NAME.op.png: the opaque picture, for unmasked use */
    uint32_t crc;
    uint8_t *idx;               /* w*h, top-down (NAME.idx) */
    uint8_t *tr[2];             /* variant masks, 1 = transparent */
    px_t *img[3];               /* 3w x 3h: m1, m2, op */
    char tried_idx, tried_img[3];
} Asset;

typedef struct Reg {
    Asset *a;
    int flipX, flipY;
    int kl, kt;                 /* crop origin, in the flipped asset */
    int n, r;                   /* subsample step (1: none) and row phase (base height % n) */
} Reg;

typedef struct HSurf {
    uint8_t *bits;
    int w, h, stride, topdown;
    struct HSurf *next;
    /* canvas */
    int canvas;
    px_t *px;
    uint8_t *ref;
    int dy0, dy1;               /* rows of px changed since the last hires_screen */
    /* source */
    int reg_ok;
    Reg reg;
    uint8_t *expect;            /* w*h: the 1x pixels the art stands for */
    px_t *spx;                  /* 3x image for the current use */
    uint8_t *sref, *smref;      /* 1x bits / mask when spx was made */
    const uint8_t *smask;       /* mask bits it was made with (NULL: unmasked) */
    int sbuilt, svar;
    /* text drawn at 3x (DrawTextA, hires_gdi_text): per 1x pixel, tset = a 3x block is in tpx
     * (the text colour, alpha = the 3x glyph coverage), tref = the 1x index it stands for,
     * tbk = the background index it goes over */
    px_t *tpx;
    uint8_t *tset, *tref, *tbk;
} HSurf;

static struct {
    int init, have_cache, enabled, showing;
    char dir[600];
    Asset *assets;
    int nassets;
    px_t pal[256];
    int palsrc;                 /* 0 none, 1 system palette, 2 the game's */
    uint8_t T, B;               /* g_455530 / g_455531 */
    HSurf *hash[1024];
    HSurf *screen;
    /* the SRCAND half of a mask + sprite pair */
    struct { int valid; HSurf *D, *M; int dx, dy, sx, sy, w, h; uint8_t *old; size_t cap; } pend;
    /* web: casts loaded before the art arrives, registered when it does (hires_web_activate) */
    int defer, replay;
    struct Pend { uint8_t *bits, *sbits; int w, h, sw, sh, n, flipX, flipY, kl, kt, kr, kb; char res[48]; } *pending;
    int npending, cappending;
    /* counters */
    long casts, verified, art_blits, nn_src, composites, rejected_px, nn_px, copies, texts, texts_1x;
} H;

/* ------------------------------------------------------------------ helpers */

static uint8_t *row1(const HSurf *s, int y) { return s->bits + (size_t)(s->topdown ? y : s->h - 1 - y) * s->stride; }

static px_t over(px_t s, px_t d)
{
    unsigned a = PX_A(s);
    if (a == 255)
        return s;
    if (a == 0)
        return d | 0xff000000u;
    unsigned ia = 255 - a;
    unsigned r = ((s & 0xff) * a + (d & 0xff) * ia + 127) / 255;
    unsigned g = (((s >> 8) & 0xff) * a + ((d >> 8) & 0xff) * ia + 127) / 255;
    unsigned b = (((s >> 16) & 0xff) * a + ((d >> 16) & 0xff) * ia + 127) / 255;
    return PX(r, g, b, 255);
}

static void fill_block(px_t *p, int stride3, px_t c)
{
    for (int j = 0; j < S3; j++, p += stride3)
        for (int i = 0; i < S3; i++)
            p[i] = c;
}

static void mark_rows(HSurf *s, int y0, int y1)
{
    if (s->dy0 >= s->dy1) {
        s->dy0 = y0;
        s->dy1 = y1;
    } else {
        if (y0 < s->dy0) s->dy0 = y0;
        if (y1 > s->dy1) s->dy1 = y1;
    }
}

static unsigned hash_ptr(const void *p) { return (unsigned)(((uintptr_t)p >> 4) * 2654435761u) >> 22; }

static HSurf *surf_find(const uint8_t *bits)
{
    for (HSurf *s = H.hash[hash_ptr(bits)]; s; s = s->next)
        if (s->bits == bits)
            return s;
    return NULL;
}

static void surf_free_data(HSurf *s)
{
    free(s->px); free(s->ref); free(s->expect); free(s->spx); free(s->sref); free(s->smref);
    free(s->tpx); free(s->tset); free(s->tref); free(s->tbk);
    s->px = NULL; s->ref = s->expect = s->sref = s->smref = NULL; s->spx = NULL;
    s->tpx = NULL; s->tset = s->tref = s->tbk = NULL;
    s->canvas = s->reg_ok = s->sbuilt = 0;
}

/* The HSurf for a DIB; created, or reset when the geometry changed (a reused pointer). */
static HSurf *surf_get(uint8_t *bits, int w, int h, int stride, int topdown)
{
    if (!bits || w <= 0 || h <= 0)
        return NULL;
    HSurf *s = surf_find(bits);
    if (s && (s->w != w || s->h != h || s->stride != stride || s->topdown != topdown)) {
        surf_free_data(s);
        s->w = w; s->h = h; s->stride = stride; s->topdown = topdown;
    }
    if (!s) {
        s = calloc(1, sizeof *s);
        if (!s)
            return NULL;
        s->bits = bits; s->w = w; s->h = h; s->stride = stride; s->topdown = topdown;
        unsigned k = hash_ptr(bits);
        s->next = H.hash[k];
        H.hash[k] = s;
    }
    return s;
}

static int dib_stride(int w, int bpp) { return (w * bpp / 8 + 3) / 4 * 4; }

/* ------------------------------------------------------------------ assets */

static int file_exists(const char *p)
{
    SDL_RWops *rw = SDL_RWFromFile(p, "rb");
    if (!rw)
        return 0;
    SDL_RWclose(rw);
    return 1;
}

static void *read_file(const char *path, size_t *len)
{
    SDL_RWops *rw = SDL_RWFromFile(path, "rb");
    if (!rw)
        return NULL;
    Sint64 n = SDL_RWsize(rw);
    void *buf = n > 0 ? malloc((size_t)n + 1) : NULL;
    if (buf && SDL_RWread(rw, buf, 1, (size_t)n) != (size_t)n) {
        free(buf);
        buf = NULL;
    }
    SDL_RWclose(rw);
    if (buf) {
        ((char *)buf)[n] = 0;
        *len = (size_t)n;
    }
    return buf;
}

static int find_cache(void)
{
    const char *e = getenv("ELFBOWL_HIRES");
    char probe[700];
    if (e && *e) {
        snprintf(H.dir, sizeof H.dir, "%s", e);
        snprintf(probe, sizeof probe, "%s/manifest.tsv", H.dir);
        if (file_exists(probe))
            return 1;
        snprintf(H.dir, sizeof H.dir, "%s/x3", e);
        snprintf(probe, sizeof probe, "%s/manifest.tsv", H.dir);
        return file_exists(probe);
    }
    const char *cands[3] = {"build/hires/x3", NULL, NULL};
    char a[600] = "", b[600] = "";
    char *base = SDL_GetBasePath();
    if (base) {
        snprintf(a, sizeof a, "%shires/x3", base);          /* build/elfbowl -> build/hires/x3 */
        snprintf(b, sizeof b, "%s../build/hires/x3", base);
        SDL_free(base);
        cands[1] = a;
        cands[2] = b;
    }
    for (int i = 0; i < 3; i++) {
        if (!cands[i])
            continue;
        snprintf(probe, sizeof probe, "%s/manifest.tsv", cands[i]);
        if (file_exists(probe)) {
            snprintf(H.dir, sizeof H.dir, "%s", cands[i]);
            return 1;
        }
    }
    return 0;
}

static void load_manifest(void)
{
    char path[700];
    snprintf(path, sizeof path, "%s/manifest.tsv", H.dir);
    size_t len = 0;
    char *text = read_file(path, &len);
    if (!text)
        return;
    int lines = 0;
    for (size_t i = 0; i < len; i++)
        lines += text[i] == '\n';
    H.assets = calloc((size_t)lines + 1, sizeof *H.assets);
    for (char *line = text, *nl; H.assets && line && *line; line = nl) {
        nl = strchr(line, '\n');
        if (nl)
            *nl++ = 0;
        Asset *a = &H.assets[H.nassets];
        char vars[64] = "";
        unsigned crc = 0;
        if (sscanf(line, "%39[^\t]\t%d\t%d\t%x\t%63s", a->name, &a->w, &a->h, &crc, vars) >= 4 && a->w > 0 &&
            a->h > 0) {
            a->crc = crc;
            a->nvar = strstr(vars, "m2") ? 2 : 1;
            a->has_op = strstr(vars, "op") != NULL;
            H.nassets++;
        }
    }
    free(text);
}

static Asset *asset_find(const char *name)
{
    for (int i = 0; i < H.nassets; i++)
        if (!SDL_strcasecmp(H.assets[i].name, name))
            return &H.assets[i];
    return NULL;
}

static uint32_t crc32_buf(const uint8_t *p, size_t n)
{
    uint32_t c = 0xffffffffu;
    for (size_t i = 0; i < n; i++) {
        c ^= p[i];
        for (int k = 0; k < 8; k++)
            c = (c >> 1) ^ (0xedb88320u & (0u - (c & 1)));
    }
    return ~c;
}

/* fn_403f04's matte on the original: background pixels 4-connected to the border. */
static void matte1(const uint8_t *idx, int w, int h, uint8_t *tr)
{
    memset(tr, 0, (size_t)w * h);
    int *q = malloc(sizeof(int) * (size_t)w * h);
    if (!q)
        return;
    int qn = 0;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            if ((y == 0 || x == 0 || y == h - 1 || x == w - 1) && idx[y * w + x] == ASSET_BG && !tr[y * w + x]) {
                tr[y * w + x] = 1;
                q[qn++] = y * w + x;
            }
    for (int qi = 0; qi < qn; qi++) {
        int p = q[qi], x = p % w, y = p / w;
        int nb[4] = {x > 0 ? p - 1 : -1, x < w - 1 ? p + 1 : -1, y > 0 ? p - w : -1, y < h - 1 ? p + w : -1};
        for (int k = 0; k < 4; k++)
            if (nb[k] >= 0 && !tr[nb[k]] && idx[nb[k]] == ASSET_BG) {
                tr[nb[k]] = 1;
                q[qn++] = nb[k];
            }
    }
    free(q);
}

static int asset_idx(Asset *a)
{
    if (a->idx || a->tried_idx)
        return a->idx != NULL;
    a->tried_idx = 1;
    char path[700];
    snprintf(path, sizeof path, "%s/%.*s.idx", H.dir, (int)strlen(a->name) - 4, a->name);
    size_t len = 0;
    uint8_t *f = read_file(path, &len);
    size_t n = (size_t)a->w * a->h;
    if (!f || len != 12 + n || memcmp(f, "EIDX", 4) || crc32_buf(f + 12, n) != a->crc) {
        port_log("hires: %s: bad or missing %s", a->name, path);
        free(f);
        return 0;
    }
    a->idx = malloc(n);
    a->tr[0] = malloc(n);
    a->tr[1] = a->nvar > 1 ? malloc(n) : NULL;
    if (!a->idx || !a->tr[0] || (a->nvar > 1 && !a->tr[1])) {
        free(f);
        return 0;
    }
    memcpy(a->idx, f + 12, n);
    free(f);
    matte1(a->idx, a->w, a->h, a->tr[0]);
    if (a->tr[1])
        for (size_t i = 0; i < n; i++)
            a->tr[1][i] = a->idx[i] == ASSET_BG;
    return 1;
}

/* Area resample of the 3x art to the canvas scale (alpha-weighted colour). */
static px_t *resample(const px_t *src, int sw, int sh, int dw, int dh)
{
    px_t *out = malloc((size_t)dw * dh * sizeof(px_t));
    if (!out)
        return NULL;
    double kx = (double)sw / dw, ky = (double)sh / dh;
    for (int Y = 0; Y < dh; Y++) {
        double y0 = Y * ky, y1 = y0 + ky;
        for (int X = 0; X < dw; X++) {
            double x0 = X * kx, x1 = x0 + kx, r = 0, g = 0, b = 0, al = 0, wt = 0;
            for (int y = (int)y0; y < sh && y < y1; y++) {
                double wy = (y + 1 < y1 ? y + 1 : y1) - (y > y0 ? y : y0);
                for (int x = (int)x0; x < sw && x < x1; x++) {
                    double w = wy * ((x + 1 < x1 ? x + 1 : x1) - (x > x0 ? x : x0));
                    px_t c = src[(size_t)y * sw + x];
                    double ca = PX_A(c) * w;
                    r += (c & 0xff) * ca; g += ((c >> 8) & 0xff) * ca; b += ((c >> 16) & 0xff) * ca;
                    al += ca; wt += w;
                }
            }
            out[(size_t)Y * dw + X] = al > 0 ? PX((unsigned)(r / al + 0.5), (unsigned)(g / al + 0.5),
                                                  (unsigned)(b / al + 0.5), (unsigned)(al / wt + 0.5)) : 0;
        }
    }
    return out;
}

static px_t *asset_img(Asset *a, int v)
{
    if (v == 2 ? !a->has_op : v >= a->nvar)
        v = 0;
    if (a->img[v] || a->tried_img[v])
        return a->img[v];
    a->tried_img[v] = 1;
    char path[700];
    snprintf(path, sizeof path, "%s/%.*s%s.png", H.dir, (int)strlen(a->name) - 4, a->name, v == 2 ? ".op" : v ? ".m2" : "");
    int w, h, n;
    unsigned char *p = stbi_load(path, &w, &h, &n, 4);
    if (!p) {
        port_log("hires: cannot load %s", path);
        return NULL;
    }
    if (w != a->w * 3 || h != a->h * 3) {
        port_log("hires: %s is %dx%d, expected %dx%d", path, w, h, a->w * 3, a->h * 3);
        stbi_image_free(p);
        return NULL;
    }
    if (S3 != 3) {
        a->img[v] = resample((const px_t *)p, w, h, a->w * S3, a->h * S3);
        stbi_image_free(p);
        return a->img[v];
    }
    a->img[v] = (px_t *)p;          /* stbi's bytes are R, G, B, A: px_t order */
    return a->img[v];
}

/* ------------------------------------------------------------------ registrations */

/* 1x plane value of the asset under cast pixel (x, y). */
static int reg_at(const Reg *g, int x, int y, int *ax, int *ay)
{
    int bx = g->n * x, by = g->n == 1 ? y : g->n * y + g->n - 1 + g->r;
    int fx = g->kl + bx, fy = g->kt + by;
    int ox = g->flipX ? g->a->w - 1 - fx : fx, oy = g->flipY ? g->a->h - 1 - fy : fy;
    if (ox < 0 || oy < 0 || ox >= g->a->w || oy >= g->a->h)
        return 0;
    *ax = ox;
    *ay = oy;
    return 1;
}

static void reg_plane(const Reg *g, const uint8_t *plane, int w, int h, uint8_t *out)
{
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int ax, ay;
            out[y * w + x] = reg_at(g, x, y, &ax, &ay) ? plane[ay * g->a->w + ax] : 0;
        }
}

/* 3x image of the cast from the asset's 3x art (box-filtered for subsampled copies). */
static void reg_image(const Reg *g, const px_t *img, int w, int h, px_t *out)
{
    int W3 = g->a->w * S3, H3 = g->a->h * S3, n = g->n;
    for (int Y = 0; Y < h * S3; Y++)
        for (int X = 0; X < w * S3; X++) {
            unsigned r = 0, gg = 0, b = 0, a = 0, cnt = 0;
            for (int j = 0; j < n; j++)
                for (int i = 0; i < n; i++) {
                    int bx = n * X + i, by = n == 1 ? Y : n * Y + S3 * g->r + j;
                    int fx = g->kl * S3 + bx, fy = g->kt * S3 + by;
                    int ox = g->flipX ? W3 - 1 - fx : fx, oy = g->flipY ? H3 - 1 - fy : fy;
                    if (ox < 0 || oy < 0 || ox >= W3 || oy >= H3)
                        continue;
                    px_t c = img[(size_t)oy * W3 + ox];
                    unsigned ca = PX_A(c);
                    r += (c & 0xff) * ca; gg += ((c >> 8) & 0xff) * ca; b += ((c >> 16) & 0xff) * ca;
                    a += ca;
                    cnt++;
                }
            px_t o = 0;
            if (a)
                o = PX((r + a / 2) / a, (gg + a / 2) / a, (b + a / 2) / a, cnt ? (a + cnt / 2) / cnt : 0);
            out[(size_t)Y * w * S3 + X] = o;
        }
}

static int register_cast(HSurf *S, const Reg *g)
{
    size_t n = (size_t)S->w * S->h;
    uint8_t *ex = malloc(n);
    if (!ex)
        return 0;
    reg_plane(g, g->a->idx, S->w, S->h, ex);
    int ok = 1;
    for (int y = 0; y < S->h && ok; y++) {
        const uint8_t *r = row1(S, y), *e = ex + (size_t)y * S->w;
        ok = !memcmp(r, e, S->w);
        /* registered late (hires_activate): a sprite's background may already be set to the
         * transparent index, as the game does once it has made the mask */
        for (int x = 0; !ok && H.replay && x < S->w && (r[x] == e[x] || (e[x] == H.B && r[x] == H.T)); x++)
            ok = x == S->w - 1;
    }
    free(S->expect);
    S->expect = ex;
    S->reg = *g;
    S->reg_ok = ok;
    S->sbuilt = 0;
    H.casts++;
    H.verified += ok;
    if (!ok)
        port_log("hires: %s: loaded pixels differ from the art's; using its own pixels", g->a->name);
    return ok;
}

/* ------------------------------------------------------------------ init / palette */

static void hires_init(void)
{
    if (H.init)
        return;
    H.init = 1;
    H.T = 0;
    H.B = 255;
    const char *c = getenv("ELFBOWL_CLASSIC");
    int classic = c && *c && *c != '0';
    H.have_cache = find_cache();
    if (H.have_cache)
        load_manifest();
#ifdef __EMSCRIPTEN__
    H.defer = !H.have_cache && !classic;    /* the art comes later, as a pack (port/web/loader.js) */
#else
    const char *late = getenv("ELFBOWL_HIRES_LATE");   /* test the web's late start: hires_activate */
    if (late && *late && H.have_cache && !classic) {
        H.have_cache = H.enabled = H.showing = 0;
        free(H.assets);
        H.assets = NULL;
        H.nassets = 0;
        H.defer = 1;
    }
#endif
    H.enabled = H.have_cache && H.nassets > 0 && !classic;
    H.showing = H.enabled;
    port_log("hires: %s%s (%d assets)%s", H.have_cache ? "art cache " : "no art cache (make hires)",
             H.have_cache ? H.dir : "", H.nassets, classic ? "; ELFBOWL_CLASSIC: 1x" : "");
}

static void invalidate_all(void)
{
    for (int k = 0; k < 1024; k++)
        for (HSurf *s = H.hash[k]; s; s = s->next) {
            s->sbuilt = 0;
            if (s->canvas && s->ref)
                for (int y = 0; y < s->h; y++) {       /* every pixel stale: refreshed on the next sync */
                    const uint8_t *r = row1(s, y);
                    uint8_t *f = s->ref + (size_t)y * s->w;
                    for (int x = 0; x < s->w; x++)
                        f[x] = (uint8_t)~r[x];
                }
        }
}

static void set_palette_rgbquad(const unsigned char *q, int src)
{
    px_t p[256];
    for (int i = 0; i < 256; i++)
        p[i] = PX(q[4 * i + 2], q[4 * i + 1], q[4 * i], 255);
    if (H.palsrc == src && !memcmp(p, H.pal, sizeof p))
        return;
    int changed = memcmp(p, H.pal, sizeof p) != 0;
    memcpy(H.pal, p, sizeof p);
    H.palsrc = src;
    if (changed)
        invalidate_all();
}

/* ------------------------------------------------------------------ canvases */

static int make_canvas(HSurf *s)
{
    if (s->canvas)
        return 1;
    size_t n = (size_t)s->w * s->h;
    s->px = malloc(n * S3 * S3 * sizeof(px_t));
    s->ref = malloc(n);
    if (!s->px || !s->ref) {
        free(s->px); free(s->ref);
        s->px = NULL; s->ref = NULL;
        return 0;
    }
    for (int y = 0; y < s->h; y++) {                  /* all stale: the first sync draws it */
        const uint8_t *r = row1(s, y);
        for (int x = 0; x < s->w; x++)
            s->ref[(size_t)y * s->w + x] = (uint8_t)~r[x];
    }
    s->canvas = 1;
    s->dy0 = 0;
    s->dy1 = s->h;
    return 1;
}

/* Nearest-neighbour refresh of every pixel in the rectangle whose index changed. */
static void sync(HSurf *D, int x0, int y0, int x1, int y1)
{
    int W3 = D->w * S3;
    for (int y = y0; y < y1; y++) {
        const uint8_t *r = row1(D, y);
        uint8_t *f = D->ref + (size_t)y * D->w;
        int any = 0;
        for (int x = x0; x < x1; x++)
            if (r[x] != f[x]) {
                f[x] = r[x];
                fill_block(D->px + (size_t)y * S3 * W3 + x * S3, W3, H.pal[r[x]]);
                any = 1;
                H.nn_px++;
            }
        if (any)
            mark_rows(D, y, y + 1);
    }
}

/* Unconditional nearest refresh (a fill, or an operation not mirrored otherwise). */
static void nn_rect(HSurf *D, int x0, int y0, int x1, int y1)
{
    int W3 = D->w * S3;
    for (int y = y0; y < y1; y++) {
        const uint8_t *r = row1(D, y);
        uint8_t *f = D->ref + (size_t)y * D->w;
        for (int x = x0; x < x1; x++) {
            f[x] = r[x];
            fill_block(D->px + (size_t)y * S3 * W3 + x * S3, W3, H.pal[r[x]]);
        }
    }
    if (y1 > y0)
        mark_rows(D, y0, y1);
}

/* ------------------------------------------------------------------ 3x text */

static int opaque_at(const HSurf *M, int x, int y);

/* Forget the 3x text over a rectangle (a fill or a blit replaced those pixels). */
static void text_clear(HSurf *S, int l, int t, int r, int b)
{
    if (!S->tset)
        return;
    for (int y = t; y < b; y++)
        memset(S->tset + (size_t)y * S->w + l, 0, r - l);
}

/* The 3x text block of source pixel (x, y) into d, when the 1x pixel is still what the text
 * left there. Masked use over a background that is the mask's key: the glyphs alone, with
 * their coverage as alpha (smooth edges over the scene); otherwise over the background. */
static int text_block(HSurf *S, HSurf *M, int x, int y, px_t *d, int dstride)
{
    size_t i = (size_t)y * S->w + x;
    if (!S->tset || !S->tset[i])
        return 0;
    int clear = M && S->tbk[i] == H.B;
    if (M && !clear && !opaque_at(M, x, y))
        return 0;
    /* the text's background under a mask that hides it: its index does not matter (a sprite's
     * background is set to the transparent index once its mask is made) */
    if (row1(S, y)[x] != S->tref[i] && !(clear && S->tref[i] == S->tbk[i] && !opaque_at(M, x, y)))
        return 0;
    int W3 = S->w * S3;
    const px_t *t = S->tpx + (size_t)y * S3 * W3 + x * S3;
    px_t bg = H.pal[S->tbk[i]];
    for (int j = 0; j < S3; j++)
        for (int k = 0; k < S3; k++)
            d[j * dstride + k] = clear ? t[j * W3 + k] : over(t[j * W3 + k], bg);
    return 1;
}

/* ------------------------------------------------------------------ sources */

static int opaque_at(const HSurf *M, int x, int y) { return !M || row1(M, y)[x] != H.B; }

/* The source's 3x RGBA for use with mask M (NULL: unmasked, opaque), valid over the given
 * rectangle of its 1x pixels. */
static px_t *src_image(HSurf *S, HSurf *M, int rx0, int ry0, int rx1, int ry1)
{
    int w = S->w, h = S->h, W3 = w * S3;
    const uint8_t *mb = M ? M->bits : NULL;
    int rebuild = !S->sbuilt || S->smask != mb;
    if (!rebuild && M)
        for (int y = ry0; y < ry1 && !rebuild; y++)
            rebuild = memcmp(row1(M, y) + rx0, S->smref + (size_t)y * w + rx0, rx1 - rx0) != 0;
    if (!rebuild) {
        /* pixels the code changed since: 3x nearest where they show */
        for (int y = ry0; y < ry1; y++) {
            const uint8_t *r = row1(S, y);
            uint8_t *f = S->sref + (size_t)y * w;
            for (int x = rx0; x < rx1; x++)
                if (r[x] != f[x]) {
                    f[x] = r[x];
                    px_t *d = S->spx + (size_t)y * S3 * W3 + x * S3;
                    if (!text_block(S, M, x, y, d, W3) && opaque_at(M, x, y))
                        fill_block(d, W3, H.pal[r[x]]);
                }
        }
        return S->spx;
    }
    size_t n = (size_t)w * h;
    if (!S->spx) {
        S->spx = malloc(n * S3 * S3 * sizeof(px_t));
        S->sref = malloc(n);
    }
    if (M && !S->smref)
        S->smref = malloc(n);
    if (!S->spx || !S->sref || (M && !S->smref))
        return NULL;
    for (int y = 0; y < h; y++) {
        memcpy(S->sref + (size_t)y * w, row1(S, y), w);
        if (M)
            memcpy(S->smref + (size_t)y * w, row1(M, y), w);
    }
    S->smask = mb;
    S->sbuilt = 1;
    px_t *art = NULL;
    int v = 0;
    uint8_t *tv = NULL;
    if (S->reg_ok && asset_idx(S->reg.a)) {
        tv = malloc(n);
        if (tv) {
            if (M) {                /* the variant whose mask matches this one best */
                long best = -1;
                uint8_t *t = malloc(n);
                for (int k = 0; t && k < S->reg.a->nvar; k++) {
                    reg_plane(&S->reg, S->reg.a->tr[k], w, h, t);
                    long bad = 0;
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            bad += opaque_at(M, x, y) == t[y * w + x];
                    if (best < 0 || bad < best) {
                        best = bad;
                        v = k;
                        memcpy(tv, t, n);
                    }
                }
                free(t);
            } else {
                memset(tv, 0, n);
                if (S->reg.a->has_op)
                    v = 2;
            }
            art = asset_img(S->reg.a, v);
        }
    }
    if (art) {
        reg_image(&S->reg, art, w, h, S->spx);
        px_t bg = H.pal[ASSET_BG];
        if (!M)
            for (size_t i = 0; i < n * S3 * S3; i++)
                S->spx[i] = over(S->spx[i], bg);
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++) {
                int op = opaque_at(M, x, y);
                int mismatch = M && op == tv[y * w + x];
                int changed = op && S->sref[(size_t)y * w + x] != S->expect[(size_t)y * w + x];
                if (mismatch || changed)
                    fill_block(S->spx + (size_t)y * S3 * W3 + x * S3, W3,
                               op ? H.pal[S->sref[(size_t)y * w + x]] : 0);
            }
        H.art_blits++;
        S->svar = v;
    } else {
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                fill_block(S->spx + (size_t)y * S3 * W3 + x * S3, W3,
                           opaque_at(M, x, y) ? H.pal[S->sref[(size_t)y * w + x]] : 0);
        H.nn_src++;
    }
    free(tv);
    if (S->tset)
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                if (S->tset[(size_t)y * w + x])
                    text_block(S, M, x, y, S->spx + (size_t)y * S3 * W3 + x * S3, W3);
    return S->spx;
}

/* ------------------------------------------------------------------ blits */

static int clip(HSurf *D, HSurf *S, int *dx, int *dy, int *sx, int *sy, int *w, int *h)
{
    if (*dx < 0) { *sx -= *dx; *w += *dx; *dx = 0; }
    if (*dy < 0) { *sy -= *dy; *h += *dy; *dy = 0; }
    if (*sx < 0) { *dx -= *sx; *w += *sx; *sx = 0; }
    if (*sy < 0) { *dy -= *sy; *h += *sy; *sy = 0; }
    if (*dx + *w > D->w) *w = D->w - *dx;
    if (*dy + *h > D->h) *h = D->h - *dy;
    if (S) {
        if (*sx + *w > S->w) *w = S->w - *sx;
        if (*sy + *h > S->h) *h = S->h - *sy;
    }
    return *w > 0 && *h > 0;
}

static void copy_blocks(HSurf *D, HSurf *S, int dx, int dy, int sx, int sy, int w, int h)
{
    const px_t *src;
    int SW3 = S->w * S3, DW3 = D->w * S3;
    if (S->canvas) {
        sync(S, sx, sy, sx + w, sy + h);
        src = S->px;
    } else {
        src = src_image(S, NULL, sx, sy, sx + w, sy + h);
    }
    if (!src) {
        nn_rect(D, dx, dy, dx + w, dy + h);
        return;
    }
    for (int Y = 0; Y < h * S3; Y++) {
        px_t *d = D->px + (size_t)(dy * S3 + Y) * DW3 + dx * S3;
        const px_t *s = src + (size_t)(sy * S3 + Y) * SW3 + sx * S3;
        for (int X = 0; X < w * S3; X++)
            d[X] = s[X] | 0xff000000u;
    }
    for (int y = 0; y < h; y++)
        memcpy(D->ref + (size_t)(dy + y) * D->w + dx, row1(D, dy + y) + dx, w);
    mark_rows(D, dy, dy + h);
    H.copies++;
}

static void composite(HSurf *D, HSurf *S, HSurf *M, int dx, int dy, int sx, int sy, int w, int h, const uint8_t *old)
{
    px_t *src = src_image(S, M, sx, sy, sx + w, sy + h);
    if (!src) {
        nn_rect(D, dx, dy, dx + w, dy + h);
        return;
    }
    int SW3 = S->w * S3, DW3 = D->w * S3;
    for (int y = 0; y < h; y++) {
        const uint8_t *fin = row1(D, dy + y) + dx;
        const uint8_t *s1 = row1(S, sy + y) + sx;
        const uint8_t *m1 = row1(M, sy + y) + sx;
        uint8_t *f = D->ref + (size_t)(dy + y) * D->w + dx;
        for (int x = 0; x < w; x++) {
            int op = m1[x] != H.B;
            uint8_t pred = op ? s1[x] : old[y * w + x];
            px_t *d = D->px + (size_t)(dy + y) * S3 * DW3 + (dx + x) * S3;
            f[x] = fin[x];
            if (fin[x] != pred) {           /* index maths that no composite means: follow the 1x */
                fill_block(d, DW3, H.pal[fin[x]]);
                H.rejected_px++;
                continue;
            }
            const px_t *s = src + (size_t)(sy + y) * S3 * SW3 + (sx + x) * S3;
            for (int j = 0; j < S3; j++)
                for (int i = 0; i < S3; i++)
                    d[j * DW3 + i] = over(s[j * SW3 + i], d[j * DW3 + i]);
        }
    }
    mark_rows(D, dy, dy + h);
    H.composites++;
}

static void blit(int end, HSurf *D, HSurf *S, int dx, int dy, int sx, int sy, int w, int h, unsigned rop)
{
    if (!D || !D->canvas) {
        if (!end && rop != ROP_SRCPAINT)
            H.pend.valid = 0;
        if (end && D && D->tset && clip(D, S, &dx, &dy, &sx, &sy, &w, &h))
            text_clear(D, dx, dy, dx + w, dy + h);
        return;
    }
    if (!clip(D, S, &dx, &dy, &sx, &sy, &w, &h)) {
        if (!end && rop != ROP_SRCPAINT)
            H.pend.valid = 0;
        return;
    }
    if (!end) {
        if (rop == ROP_SRCAND && S) {
            sync(D, dx, dy, dx + w, dy + h);
            size_t need = (size_t)w * h;
            if (need > H.pend.cap) {
                free(H.pend.old);
                H.pend.old = malloc(need);
                H.pend.cap = H.pend.old ? need : 0;
            }
            H.pend.valid = H.pend.old != NULL;
            for (int y = 0; H.pend.valid && y < h; y++)
                memcpy(H.pend.old + (size_t)y * w, row1(D, dy + y) + dx, w);
            H.pend.D = D; H.pend.M = S;
            H.pend.dx = dx; H.pend.dy = dy; H.pend.sx = sx; H.pend.sy = sy; H.pend.w = w; H.pend.h = h;
        } else if (rop != ROP_SRCPAINT) {
            H.pend.valid = 0;
        }
        return;
    }
    if (rop == ROP_SRCAND)
        return;                             /* the sprite half follows */
    if (rop == ROP_SRCPAINT && S && H.pend.valid && H.pend.D == D && H.pend.dx == dx && H.pend.dy == dy &&
        H.pend.sx == sx && H.pend.sy == sy && H.pend.w == w && H.pend.h == h && H.pend.M->w == S->w &&
        H.pend.M->h == S->h) {
        composite(D, S, H.pend.M, dx, dy, sx, sy, w, h, H.pend.old);
        H.pend.valid = 0;
        return;
    }
    H.pend.valid = 0;
    if (rop == ROP_SRCCOPY && S) {
        copy_blocks(D, S, dx, dy, sx, sy, w, h);
        return;
    }
    nn_rect(D, dx, dy, dx + w, dy + h);
}

/* ------------------------------------------------------------------ game hooks */

static void pend_add(const struct Pend *e)
{
    if (!H.defer)
        return;
    if (H.npending == H.cappending) {
        int cap = H.cappending ? 2 * H.cappending : 256;
        struct Pend *p = realloc(H.pending, (size_t)cap * sizeof *p);
        if (!p)
            return;
        H.pending = p;
        H.cappending = cap;
    }
    H.pending[H.npending++] = *e;
}

void hires_cast_loaded(unsigned char *bits, int w, int h, int bpp, const char *res, int flipX, int flipY,
                       int kl, int kt, int kr, int kb, const unsigned char *pal)
{
    hires_init();
    if (bpp != 8 || !bits || !res)
        return;
    if (pal && (H.have_cache || H.defer))
        set_palette_rgbquad(pal, 2);
    if (!H.have_cache) {
        struct Pend e = {bits, NULL, w, h, 0, 0, 1, flipX, flipY, kl, kt, kr, kb, ""};
        snprintf(e.res, sizeof e.res, "%s", res);
        pend_add(&e);
        return;
    }
    const char *p = res;
    if (*p == '#')
        for (p++; *p >= '0' && *p <= '9'; p++)
            ;
    while (*p == '%' || *p == '$')
        p++;
    Asset *a = asset_find(p);
    if (!a || !asset_idx(a))
        return;
    HSurf *S = surf_get(bits, w, h, dib_stride(w, 8), 0);
    if (!S)
        return;
    Reg g = {a, flipX != 0, flipY != 0, 0, 0, 1, 0};
    if (kr > kl && kb > kt) {
        g.kl = kl;
        g.kt = kt;
    }
    register_cast(S, &g);
}

void hires_scaled(unsigned char *dbits, int dw, int dh, int dbpp, unsigned char *sbits, int sw, int sh, int sbpp,
                  int n)
{
    hires_init();
    if (dbpp != 8 || sbpp != 8 || n < 1)
        return;
    if (!H.have_cache) {
        struct Pend e = {dbits, sbits, dw, dh, sw, sh, n, 0, 0, 0, 0, 0, 0, ""};
        pend_add(&e);
        return;
    }
    HSurf *S = surf_find(sbits);
    if (!S || !S->reg_ok || S->reg.n != 1 || S->w != sw || S->h != sh)
        return;
    HSurf *D = surf_get(dbits, dw, dh, dib_stride(dw, 8), 0);
    if (!D)
        return;
    Reg g = S->reg;
    g.n = n;
    g.r = sh % n;
    register_cast(D, &g);
}

void hires_blit(int end, unsigned char *dbits, int dw, int dh, int dbpp, int canvas, int dx, int dy,
                unsigned char *sbits, int sw, int sh, int sbpp, int sx, int sy, int w, int h, unsigned rop,
                unsigned char T, unsigned char B)
{
    hires_init();
    if (!H.enabled || dbpp != 8 || sbpp != 8)
        return;
    H.T = T;
    H.B = B;
    HSurf *D = canvas ? surf_get(dbits, dw, dh, dib_stride(dw, 8), 0) : surf_find(dbits);
    if (D && canvas && !make_canvas(D))
        return;
    HSurf *S = surf_get(sbits, sw, sh, dib_stride(sw, 8), 0);
    blit(end, D, S, dx, dy, sx, sy, w, h, rop);
}

void hires_touch(unsigned char *bits, int w, int h, int bpp, int l, int t, int r, int b)
{
    if (!H.enabled || bpp != 8)
        return;
    HSurf *D = surf_find(bits);
    if (!D)
        return;
    if (l < 0) l = 0;
    if (t < 0) t = 0;
    if (r > D->w) r = D->w;
    if (b > D->h) b = D->h;
    if (r > l && b > t) {
        text_clear(D, l, t, r, b);
        if (D->canvas)
            nn_rect(D, l, t, r, b);
    }
}

/* ------------------------------------------------------------------ shim side */

void hires_gdi_canvas(unsigned char *bits, int w, int h, int stride, int topdown)
{
    hires_init();
    if (H.screen && H.screen->bits != bits) {
        hires_gdi_forget(H.screen->bits);
        H.screen = NULL;
    }
    H.screen = surf_get(bits, w, h, stride, topdown);
}

void hires_gdi_forget(unsigned char *bits)
{
    for (int i = 0; i < H.npending; i++)
        if (H.pending[i].bits == bits || H.pending[i].sbits == bits)
            H.pending[i].bits = NULL;
    unsigned k = hash_ptr(bits);
    for (HSurf **pp = &H.hash[k]; *pp; pp = &(*pp)->next)
        if ((*pp)->bits == bits) {
            HSurf *s = *pp;
            *pp = s->next;
            if (H.pend.D == s || H.pend.M == s)
                H.pend.valid = 0;
            if (H.screen == s)
                H.screen = NULL;
            surf_free_data(s);
            free(s);
            return;
        }
}

void hires_gdi_blit(int end, unsigned char *dbits, int dw, int dh, int dstride, int dtop, unsigned char *sbits,
                    int sw, int sh, int sstride, int stop, int dx, int dy, int sx, int sy, int w, int h,
                    unsigned rop)
{
    if (!H.enabled)
        return;
    HSurf *D = surf_find(dbits);
    if (D && D == H.screen && !make_canvas(D))
        return;
    HSurf *S = sbits ? surf_get(sbits, sw, sh, sstride, stop) : NULL;
    (void)dw; (void)dh; (void)dstride; (void)dtop;
    blit(end, D, S, dx, dy, sx, sy, w, h, rop);
}

void hires_gdi_touch(unsigned char *bits, int w, int h, int stride, int topdown, int l, int t, int r, int b)
{
    (void)stride; (void)topdown;
    hires_touch(bits, w, h, 8, l, t, r, b);
}

int hires_text_wanted(void)
{
    hires_init();
    return H.enabled;
}

void hires_gdi_text(unsigned char *bits, int w, int h, int stride, int topdown, int x0, int y0, int x1, int y1,
                    const unsigned char *cov, int cstride, unsigned char ti, unsigned char bi)
{
    if (!H.enabled)
        return;
    HSurf *S = surf_get(bits, w, h, stride, topdown);
    if (!S || x0 < 0 || y0 < 0 || x1 > w || y1 > h || x1 <= x0 || y1 <= y0)
        return;
    /* 1x is authoritative. The two renderings are hinted differently, so their pixels never agree
     * exactly; the 3x ink must span what the 1x ink spans (within 2 pixels, 2% more across), or
     * the line stays 3x nearest of the 1x. */
    int e1[4] = {w, h, -1, -1}, e3[4] = {w, h, -1, -1};
    for (int y = y0; y < y1; y++) {
        const uint8_t *r = row1(S, y);
        for (int x = x0; x < x1; x++) {
            const unsigned char *c = cov + (size_t)(y - y0) * S3 * cstride + (x - x0) * S3;
            unsigned sum = 0;
            for (int j = 0; j < S3; j++)
                for (int k = 0; k < S3; k++)
                    sum += c[j * cstride + k];
            int *e = NULL;
            if (r[x] == ti && ti != bi) {
                e = e1;
                if (x < e[0]) e[0] = x;
                if (y < e[1]) e[1] = y;
                if (x > e[2]) e[2] = x;
                if (y > e[3]) e[3] = y;
            }
            if (sum * 3 >= S3 * S3 * 255u) {
                e = e3;
                if (x < e[0]) e[0] = x;
                if (y < e[1]) e[1] = y;
                if (x > e[2]) e[2] = x;
                if (y > e[3]) e[3] = y;
            }
        }
    }
    for (int k = 0; k < 4; k++)
        if (abs(e1[k] - e3[k]) > (k & 1 ? 2 : 2 + (x1 - x0) / 50)) {
            H.texts_1x++;
            return;
        }
    size_t n = (size_t)w * h;
    if (!S->tset) {
        S->tset = calloc(n, 1);
        S->tref = malloc(n);
        S->tbk = malloc(n);
        S->tpx = malloc(n * S3 * S3 * sizeof(px_t));
        if (!S->tset || !S->tref || !S->tbk || !S->tpx) {
            free(S->tset); free(S->tref); free(S->tbk); free(S->tpx);
            S->tset = S->tref = S->tbk = NULL; S->tpx = NULL;
            return;
        }
    }
    int W3 = w * S3;
    px_t tc = H.pal[ti] & 0x00ffffffu;
    for (int y = y0; y < y1; y++) {
        const uint8_t *r = row1(S, y);
        for (int x = x0; x < x1; x++) {
            size_t i = (size_t)y * w + x;
            if (r[x] != ti && r[x] != bi) {      /* not what DrawTextA leaves: not ours */
                S->tset[i] = 0;
                continue;
            }
            S->tset[i] = 1;
            S->tref[i] = r[x];
            S->tbk[i] = bi;
            const unsigned char *c = cov + (size_t)(y - y0) * S3 * cstride + (x - x0) * S3;
            px_t *t = S->tpx + (size_t)y * S3 * W3 + x * S3;
            for (int j = 0; j < S3; j++)
                for (int k = 0; k < S3; k++)
                    t[j * W3 + k] = tc | (px_t)c[j * cstride + k] << 24;
        }
    }
    if (S->canvas) {                        /* drawn straight on a canvas */
        for (int y = y0; y < y1; y++)
            for (int x = x0; x < x1; x++)
                if (text_block(S, NULL, x, y, S->px + (size_t)y * S3 * W3 + x * S3, W3))
                    S->ref[(size_t)y * w + x] = row1(S, y)[x];
        mark_rows(S, y0, y1);
    } else {
        S->sbuilt = 0;                      /* its 3x image is rebuilt with the text */
    }
    H.texts++;
}

int hires_scale(void)
{
    return S3;
}

int hires_activate(const char *dir, int scale)
{
    hires_init();
    if (H.have_cache)
        return H.enabled;
    snprintf(H.dir, sizeof H.dir, "%s", dir);
    char probe[700];
    snprintf(probe, sizeof probe, "%s/manifest.tsv", H.dir);
    if (!file_exists(probe)) {
        port_log("hires: no art in %s", dir);
        return 0;
    }
    g_scale = scale == 2 ? 2 : 3;
    H.have_cache = 1;
    load_manifest();
    H.enabled = H.showing = H.nassets > 0;
    /* the casts loaded so far, in order (register_cast checks their pixels against the art) */
    H.replay = 1;
    for (int i = 0; i < H.npending; i++) {
        struct Pend *e = &H.pending[i];
        if (!e->bits)
            continue;
        if (e->sbits)
            hires_scaled(e->bits, e->w, e->h, 8, e->sbits, e->sw, e->sh, 8, e->n);
        else
            hires_cast_loaded(e->bits, e->w, e->h, 8, e->res, e->flipX, e->flipY, e->kl, e->kt, e->kr, e->kb, NULL);
    }
    H.replay = 0;
    free(H.pending);
    H.pending = NULL;
    H.npending = H.cappending = 0;
    H.defer = 0;
    port_log("hires: art %s (%d assets) at %dx, %ld of %ld casts match", H.dir, H.nassets, S3, H.verified, H.casts);
    return H.enabled;
}

void hires_set_showing(int on)
{
    hires_init();
    if (H.enabled)
        H.showing = on != 0;
}

int hires_showing(void)
{
    hires_init();
    return H.showing;
}

void hires_toggle(void)
{
    hires_init();
    if (!H.enabled)
        H.enabled = 1;          /* started classic: canvases build from the 1x as they are next drawn */
    H.showing = !H.showing;
    port_log("hires: %s", H.showing ? "3x art" : "classic 1x");
}

const unsigned int *hires_screen(const unsigned char *syspal, int *w3, int *h3, int *y0, int *y1, int *mod)
{
    HSurf *s = H.screen;
    *mod = -1;
    if (!s || !H.enabled)
        return NULL;
    if (H.palsrc != 2) {                    /* no game palette yet: follow the system's */
        unsigned char q[1024];
        for (int i = 0; i < 256; i++) {
            q[4 * i] = syspal[4 * i + 2]; q[4 * i + 1] = syspal[4 * i + 1]; q[4 * i + 2] = syspal[4 * i];
            q[4 * i + 3] = 0;
        }
        set_palette_rgbquad(q, 1);
    }
    if (!make_canvas(s))
        return NULL;
    sync(s, 0, 0, s->w, s->h);
    /* the realized palette against the art's: equal, a uniform scale (a fade), or other */
    long sa = 0, sb = 0;
    int same = 1;
    for (int i = 0; i < 256; i++) {
        px_t c = H.pal[i];
        int r = c & 0xff, g = (c >> 8) & 0xff, b = (c >> 16) & 0xff;
        same &= syspal[4 * i] == r && syspal[4 * i + 1] == g && syspal[4 * i + 2] == b;
        sa += syspal[4 * i] + syspal[4 * i + 1] + syspal[4 * i + 2];
        sb += r + g + b;
    }
    if (same) {
        *mod = 255;
    } else if (sb > 0) {
        int k = (int)((sa * 255 + sb / 2) / sb);
        int ok = k <= 255;
        for (int i = 0; ok && i < 256; i++) {
            px_t c = H.pal[i];
            int ch[3] = {(int)(c & 0xff), (int)((c >> 8) & 0xff), (int)((c >> 16) & 0xff)};
            for (int j = 0; j < 3; j++)
                if (abs(syspal[4 * i + j] - (ch[j] * k + 127) / 255) > 2)
                    ok = 0;
        }
        *mod = ok ? k : -1;
    }
    *w3 = s->w * S3;
    *h3 = s->h * S3;
    *y0 = s->dy0 * S3;
    *y1 = s->dy1 * S3;
    s->dy0 = s->dy1 = 0;
    return s->px;
}

void hires_stats_log(void)
{
    if (!H.init || !H.have_cache)
        return;
    port_log("hires: %ld casts registered, %ld match the art; %ld art images, %ld nearest; %ld composites "
             "(%ld px followed the 1x), %ld copies, %ld px refreshed nearest; text lines %ld at 3x, %ld 1x",
             H.casts, H.verified, H.art_blits, H.nn_src, H.composites, H.rejected_px, H.copies, H.nn_px,
             H.texts, H.texts_1x);
}
