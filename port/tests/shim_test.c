/* Headless end-to-end test of the Win32 shim.
 *
 *  1. Resources: find PACKEDFILE/PARMS/CASTS/TFORM1/STRING in the user's exe.
 *  2. GDI: inflate BowlingLogo.bmp from the pack (system zlib), draw it with
 *     CreateDIBSection + BitBlt (SRCCOPY, SRCAND over white, SRCPAINT over
 *     black), draw text with an enumerated Arial font, blit a 32-bit top-down
 *     DIB, present, and dump the screen to a BMP.
 *  3. Audio: play one WAV through waveOut (to the SDL disk driver), waiting
 *     for WOM_DONE like the game's sound manager does, then test waveOutReset.
 *
 * Facts for the Python check are written as key=value lines to the report.
 * Usage: shim_test <exe> <screen.bmp> <report.txt>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <SDL.h>

#include "windows.h"
#include "mmsystem.h"

static FILE *rep;
static int failures;

#define CHECK(c, ...) do { if (!(c)) { failures++; fprintf(stderr, "FAIL %s:%d: ", __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)

static unsigned rd32(const unsigned char *p) { return p[0] | p[1] << 8 | p[2] << 16 | (unsigned)p[3] << 24; }

static const unsigned char *res(const char *name, LPCSTR type, DWORD *size)
{
    HRSRC r = FindResourceA(NULL, name, type);
    if (!r)
        return NULL;
    HGLOBAL h = LoadResource(NULL, r);
    *size = SizeofResource(NULL, r);
    return h ? LockResource(h) : NULL;
}

/* Inflate one "NVD Rules!" pack entry (layout per tools/unpack.py). */
static unsigned char *pack_get(const unsigned char *pk, DWORD pksize, const char *want, unsigned *outsz)
{
    if (memcmp(pk, "NVD Rules!", 10))
        return NULL;
    unsigned n = rd32(pk + 0x10);
    for (unsigned i = 0; i < n; i++) {
        const unsigned char *e = pk + 0x18 + 40 * i;
        char name[25];
        memcpy(name, e, 24);
        name[24] = 0;
        const char *clean = name + strspn(name, "#4%");
        if (strcmp(clean, want))
            continue;
        unsigned cs = rd32(e + 24), us = rd32(e + 28), off = rd32(e + 32);
        if (off + cs > pksize)
            return NULL;
        unsigned char *out = malloc(us);
        uLongf len = us;
        if (uncompress(out, &len, pk + off, cs) != Z_OK || len != us) {
            free(out);
            return NULL;
        }
        *outsz = us;
        return out;
    }
    return NULL;
}

/* ---- font enumeration exactly as fn_4035f4 / fn_4036c8 do it ---- */
static int g_found;
static int CALLBACK enum_cb(const LOGFONTA *lf, const TEXTMETRICA *tm, DWORD type, LPARAM lp)
{
    (void)tm; (void)type;
    const ENUMLOGFONTA *e = (const ENUMLOGFONTA *)lf;
    if (!strcmp((const char *)e->elfFullName, "Lucida Handwriting Italic") ||
        !strcmp((const char *)e->elfFullName, "Lucida Calligraphy Italic")) {
        if (!strcmp((const char *)e->elfStyle, "Italic")) {
            memcpy((void *)lp, lf, sizeof(LOGFONTA));
            g_found = 1;
        }
    } else if (!strcmp((const char *)e->elfStyle, "Regular")) {
        memcpy((void *)lp, lf, sizeof(LOGFONTA));
        g_found = 1;
    }
    return 1;
}

static HFONT make_font(const char *face, int height, int weight)
{
    LOGFONTA lf;
    memset(&lf, 0, sizeof lf);
    HDC dc = CreateCompatibleDC(0);
    g_found = 0;
    EnumFontFamiliesA(dc, face, enum_cb, (LPARAM)&lf);
    DeleteDC(dc);
    if (!g_found)
        return NULL;
    lf.lfHeight = height;
    lf.lfWidth = 0;
    lf.lfWeight = weight;
    return CreateFontIndirectA(&lf);
}

/* ---- audio ---- */
static volatile int g_msgs[3];          /* WOM_OPEN, WOM_CLOSE, WOM_DONE counts */
static volatile int g_done;
static volatile WAVEHDR *g_doneHdr;

static void CALLBACK wave_cb(HWAVEOUT h, UINT msg, DWORD_PTR inst, DWORD_PTR p1, DWORD_PTR p2)
{
    (void)h; (void)inst; (void)p2;
    if (msg >= WOM_OPEN && msg <= WOM_DONE)
        g_msgs[msg - WOM_OPEN]++;
    if (msg == WOM_DONE) {
        g_doneHdr = (WAVEHDR *)p1;
        g_done = 1;
    }
}

static void test_audio(const unsigned char *pk, DWORD pksize, const char *wavname)
{
    unsigned wsz;
    unsigned char *wav = pack_get(pk, pksize, wavname, &wsz);
    CHECK(wav, "cannot unpack %s", wavname);
    if (!wav)
        return;
    /* chunk scan as TSound::LoadResource does (mmioStringToFOURCC("fmt", 0)) */
    FOURCC fmtcc = mmioStringToFOURCC("fmt", 0), datacc = mmioStringToFOURCC("data", 0);
    CHECK(fmtcc == mmioFOURCC('f', 'm', 't', ' '), "mmioStringToFOURCC(\"fmt\") = %08x", (unsigned)fmtcc);
    CHECK(mmioStringToFOURCCA("wave", MMIO_TOUPPER) == mmioFOURCC('W', 'A', 'V', 'E'), "MMIO_TOUPPER");
    WAVEFORMATEX fmt;
    memset(&fmt, 0, sizeof fmt);
    unsigned char *data = NULL;
    unsigned datasize = 0;
    for (unsigned o = 12; o + 8 <= wsz;) {
        unsigned id = rd32(wav + o), len = rd32(wav + o + 4);
        if (id == fmtcc)
            memcpy(&fmt, wav + o + 8, len < sizeof fmt ? len : sizeof fmt);
        else if (id == datacc) {
            data = wav + o + 8;
            datasize = len;
        }
        o += 8 + len + (len & 1);
    }
    CHECK(data && fmt.wFormatTag == WAVE_FORMAT_PCM, "bad WAV");
    fprintf(rep, "wav=%s\nwav_rate=%u\nwav_bits=%u\nwav_channels=%u\nwav_bytes=%u\n", wavname,
            (unsigned)fmt.nSamplesPerSec, fmt.wBitsPerSample, fmt.nChannels, datasize);

    UINT ndev = waveOutGetNumDevs();
    CHECK(ndev > 0, "waveOutGetNumDevs = 0");
    HWAVEOUT hwo = NULL;
    MMRESULT r = waveOutOpen(&hwo, WAVE_MAPPER, &fmt, 0, 0, WAVE_FORMAT_QUERY);
    CHECK(r == MMSYSERR_NOERROR && hwo == NULL, "WAVE_FORMAT_QUERY r=%u", r);
    r = waveOutOpen(&hwo, WAVE_MAPPER, &fmt, (DWORD_PTR)wave_cb, 0, CALLBACK_FUNCTION);
    CHECK(r == MMSYSERR_NOERROR && hwo, "waveOutOpen r=%u", r);
    CHECK(g_msgs[0] == 1, "WOM_OPEN not delivered");
    WAVEHDR hdr;
    memset(&hdr, 0, sizeof hdr);
    hdr.lpData = (LPSTR)data;
    hdr.dwBufferLength = datasize;
    CHECK(waveOutWrite(hwo, &hdr, 0x20) == WAVERR_UNPREPARED, "write of unprepared header accepted");
    CHECK(waveOutPrepareHeader(hwo, &hdr, 0x20) == MMSYSERR_NOERROR, "prepare");
    Uint64 t0 = SDL_GetTicks64();
    CHECK(waveOutWrite(hwo, &hdr, 0x20) == MMSYSERR_NOERROR, "write");
    CHECK(waveOutUnprepareHeader(hwo, &hdr, 0x20) == WAVERR_STILLPLAYING, "unprepare while playing");
    while (!g_done && SDL_GetTicks64() - t0 < 10000)
        SDL_Delay(2);                   /* the game polls its done flag each tick */
    Uint64 ms = SDL_GetTicks64() - t0;
    unsigned expect = datasize * 1000 / fmt.nAvgBytesPerSec;
    CHECK(g_done, "no WOM_DONE within 10 s");
    CHECK(g_doneHdr == &hdr && (hdr.dwFlags & WHDR_DONE) && !(hdr.dwFlags & WHDR_INQUEUE), "header flags %x",
          (unsigned)hdr.dwFlags);
    CHECK(ms + 150 >= expect && ms <= expect + 1000, "WOM_DONE after %llu ms, sound is %u ms",
          (unsigned long long)ms, expect);
    fprintf(rep, "wom_done_ms=%llu\nwav_ms=%u\n", (unsigned long long)ms, expect);
    CHECK(waveOutUnprepareHeader(hwo, &hdr, 0x20) == MMSYSERR_NOERROR, "unprepare");
    CHECK(waveOutClose(hwo) == MMSYSERR_NOERROR, "close");
    CHECK(g_msgs[1] == 1, "WOM_CLOSE not delivered");

    /* waveOutReset completes a queued header at once (the game's stop path) */
    g_done = 0;
    r = waveOutOpen(&hwo, WAVE_MAPPER, &fmt, (DWORD_PTR)wave_cb, 0, CALLBACK_FUNCTION);
    memset(&hdr, 0, sizeof hdr);
    hdr.lpData = (LPSTR)data;
    hdr.dwBufferLength = datasize;
    waveOutPrepareHeader(hwo, &hdr, 0x20);
    waveOutWrite(hwo, &hdr, 0x20);
    CHECK(waveOutClose(hwo) == WAVERR_STILLPLAYING, "close while playing accepted");
    waveOutReset(hwo);
    CHECK(g_done && (hdr.dwFlags & WHDR_DONE), "waveOutReset did not complete the header");
    waveOutUnprepareHeader(hwo, &hdr, 0x20);
    CHECK(waveOutClose(hwo) == MMSYSERR_NOERROR, "close after reset");
    fprintf(rep, "wom_open=%d\nwom_close=%d\nwom_done=%d\n", g_msgs[0], g_msgs[1], g_msgs[2]);
    SDL_Delay(200);                     /* let the disk driver write trailing silence */
    free(wav);
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr, "usage: %s <Elf Bowling.exe> <screen.bmp> <report.txt>\n", argv[0]);
        return 2;
    }
    rep = fopen(argv[3], "w");
    if (!rep)
        return 2;
    if (port_res_open(argv[1])) {
        fprintf(stderr, "cannot read resources from %s\n", argv[1]);
        return 2;
    }

    /* ---- 1. resources ---- */
    DWORD pksize = 0, sz = 0;
    const unsigned char *pk = res("PackedFile", "NVDPACKFILE", &pksize);   /* mixed case, as the game asks */
    CHECK(pk && !memcmp(pk, "NVD Rules!", 10), "PACKEDFILE missing");
    fprintf(rep, "res_PACKEDFILE=%u\n", (unsigned)pksize);
    const unsigned char *p = res("PARMS", "NVDPARMFILE", &sz);
    CHECK(p, "PARMS missing");
    fprintf(rep, "res_PARMS=%u\n", (unsigned)sz);
    p = res("CASTS", "NVDCASTFILE", &sz);
    CHECK(p, "CASTS missing");
    fprintf(rep, "res_CASTS=%u\n", (unsigned)sz);
    p = res("TFORM1", RT_RCDATA, &sz);
    CHECK(p && !memcmp(p, "TPF0", 4), "RCDATA TFORM1 missing");
    fprintf(rep, "res_TFORM1=%u\n", (unsigned)sz);
    p = res("#4087", RT_STRING, &sz);
    CHECK(p, "STRING #4087 missing");
    fprintf(rep, "res_STRING_4087=%u\n", (unsigned)sz);
    CHECK(res(MAKEINTRESOURCE(4087), RT_STRING, &sz) == p, "MAKEINTRESOURCE lookup differs");
    CHECK(!FindResourceA(NULL, "NOPE", "NVDPACKFILE"), "found a missing resource");
    if (!pk)
        return 1;

    /* ---- 2. GDI ---- */
    port_init(640, 480, "shim_test");
    char bpp[16];
    HDC ic = CreateICA("DISPLAY", 0, 0, 0);
    snprintf(bpp, sizeof bpp, "%d", GetDeviceCaps(ic, BITSPIXEL) * GetDeviceCaps(ic, PLANES));
    DeleteDC(ic);
    CHECK(!strcmp(bpp, "8"), "display bpp %s", bpp);

    unsigned bsz;
    unsigned char *bmp = pack_get(pk, pksize, "BowlingLogo.bmp", &bsz);
    CHECK(bmp && bmp[0] == 'B' && bmp[1] == 'M', "BowlingLogo.bmp not unpacked");
    if (!bmp)
        return 1;
    const BITMAPINFO *bi = (const BITMAPINFO *)(bmp + 14);
    const BITMAPINFOHEADER *bh = &bi->bmiHeader;
    int w = bh->biWidth, h = bh->biHeight;
    CHECK(bh->biBitCount == 8 && h > 0, "logo is not 8-bit bottom-up");
    int ncol = bh->biClrUsed ? (int)bh->biClrUsed : 256;
    const RGBQUAD *ct = bi->bmiColors;

    LOGPALETTE *lp = calloc(1, sizeof(LOGPALETTE) + 256 * sizeof(PALETTEENTRY));
    lp->palVersion = 0x300;
    lp->palNumEntries = 256;
    for (int i = 0; i < ncol; i++) {
        lp->palPalEntry[i].peRed = ct[i].rgbRed;
        lp->palPalEntry[i].peGreen = ct[i].rgbGreen;
        lp->palPalEntry[i].peBlue = ct[i].rgbBlue;
    }
    HPALETTE pal = CreatePalette(lp);
    CHECK(pal, "CreatePalette");

    void *bits = NULL;
    HBITMAP hb = CreateDIBSection(0, bi, DIB_RGB_COLORS, &bits, 0, 0);
    CHECK(hb && bits, "CreateDIBSection");
    int stride = (w * 8 + 31) / 32 * 4;
    memcpy(bits, bmp + rd32(bmp + 10), (size_t)stride * h);

    HDC scr = GetDC(port_main_hwnd());
    HPALETTE oldpal = SelectPalette(scr, pal, FALSE);
    CHECK(RealizePalette(scr) == 256, "RealizePalette");
    RECT all = {0, 0, 640, 480};
    CHECK(FillRect(scr, &all, (HBRUSH)GetStockObject(BLACK_BRUSH)), "FillRect black");
    HDC mem = CreateCompatibleDC(scr);
    HGDIOBJ oldbmp = SelectObject(mem, hb);
    CHECK(oldbmp != NULL, "SelectObject(bitmap) returned no previous bitmap");
    CHECK(BitBlt(scr, 8, 8, w, h, mem, 0, 0, SRCCOPY), "BitBlt SRCCOPY");
    RECT wr = {8, 160, 8 + w, 160 + h};
    FillRect(scr, &wr, (HBRUSH)GetStockObject(WHITE_BRUSH));
    CHECK(BitBlt(scr, 8, 160, w, h, mem, 0, 0, SRCAND), "BitBlt SRCAND");
    CHECK(BitBlt(scr, 8, 312, w, h, mem, 0, 0, SRCPAINT), "BitBlt SRCPAINT");
    /* clipped blit off the right edge must not crash or wrap */
    CHECK(BitBlt(scr, 600, 460, w, h, mem, 0, 0, SRCCOPY), "clipped BitBlt");
    fprintf(rep, "logo_w=%d\nlogo_h=%d\nblit_copy=8,8\nblit_and=8,160\nblit_paint=8,312\nclip_blit=600,460\n", w, h);

    /* text into an 8-bit DIB, as fn_4037e4 does */
    HFONT font = make_font("Arial", 0x14, 700);
    CHECK(font, "Arial not enumerated");
    CHECK(make_font("Lucida Handwriting", 0x13, 600) != NULL, "Lucida Handwriting not enumerated");
    CHECK(make_font("Lucida Calligraphy", 0x13, 600) != NULL, "Lucida Calligraphy not enumerated");
    BITMAPINFO *tbi = calloc(1, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
    tbi->bmiHeader = *bh;
    tbi->bmiHeader.biWidth = 200;
    tbi->bmiHeader.biHeight = 60;
    tbi->bmiHeader.biSizeImage = 0;
    memcpy(tbi->bmiColors, ct, ncol * sizeof(RGBQUAD));
    HBITMAP tb = CreateDIBSection(0, tbi, DIB_RGB_COLORS, &bits, 0, 0);
    HDC tdc = CreateCompatibleDC(0);
    SelectObject(tdc, tb);
    HGDIOBJ oldfont = SelectObject(tdc, font);
    SetBkMode(tdc, OPAQUE);
    SetTextColor(tdc, 0xffff);
    RECT tr = {0, 0, 200, 60};
    int th = DrawTextA(tdc, "Elf Bowling &Shim test", -1, &tr, DT_CENTER | DT_WORDBREAK);
    CHECK(th > 0, "DrawTextA height %d", th);
    SelectObject(tdc, oldfont);
    CHECK(BitBlt(scr, 430, 8, 200, 60, tdc, 0, 0, SRCCOPY), "text blit");
    fprintf(rep, "text_rect=430,8,200,60\ntext_color=ffff00\ntext_height=%d\n", th);
    DeleteDC(tdc);

    /* 32-bit top-down DIB holding exact palette colours: nearest() must hit them */
    BITMAPINFO rgb;
    memset(&rgb, 0, sizeof rgb);
    rgb.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    rgb.bmiHeader.biWidth = 64;
    rgb.bmiHeader.biHeight = -64;
    rgb.bmiHeader.biPlanes = 1;
    rgb.bmiHeader.biBitCount = 32;
    HBITMAP rb = CreateDIBSection(0, &rgb, DIB_RGB_COLORS, &bits, 0, 0);
    CHECK(rb, "32-bit DIB");
    for (int y = 0; y < 64; y++)
        for (int x = 0; x < 64; x++) {
            const RGBQUAD *c = &ct[((y / 8) * 8 + x / 8) * 4];
            unsigned char *px = (unsigned char *)bits + (y * 64 + x) * 4;
            px[0] = c->rgbBlue; px[1] = c->rgbGreen; px[2] = c->rgbRed; px[3] = 0;
        }
    SelectObject(mem, rb);
    CHECK(BitBlt(scr, 430, 100, 64, 64, mem, 0, 0, SRCCOPY), "32-bit blit");
    fprintf(rep, "rgb_blit=430,100\n");

    SelectObject(mem, oldbmp);
    CHECK(DeleteObject(hb), "DeleteObject(bitmap)");
    DeleteDC(mem);
    SelectPalette(scr, oldpal, TRUE);
    ReleaseDC(port_main_hwnd(), scr);
    port_present();
    CHECK(port_screen_dump_bmp(argv[2]) == 0, "dump %s", argv[2]);

    /* ---- 3. audio ---- */
    test_audio(pk, pksize, "bounce.wav");
    const char *rate = getenv("PORT_AUDIO_RATE");
    fprintf(rep, "device_rate=%s\n", rate ? rate : "44100");

    /* ---- kernel odds and ends ---- */
    DWORD t = TlsAlloc();
    CHECK(t != TLS_OUT_OF_INDEXES && TlsSetValue(t, &t) && TlsGetValue(t) == &t && TlsFree(t), "TLS");
    char s[32];
    lstrcpyA(s, "abc");
    lstrcatA(s, "def");
    CHECK(lstrlenA(s) == 6 && !strcmp(s, "abcdef"), "lstr*");
    HLOCAL l = LocalAlloc(LPTR, 16);
    CHECK(l && ((char *)l)[15] == 0 && LocalFree(l) == NULL, "LocalAlloc");
    MEMORYSTATUS ms;
    ms.dwLength = 32;
    GlobalMemoryStatus(&ms);
    CHECK(sizeof ms == 32 && ms.dwTotalPhys > 0 && ms.dwAvailPhys <= ms.dwTotalPhys, "GlobalMemoryStatus");
    CHECK(LoadLibraryA("Wininet.dll") == NULL, "LoadLibraryA");
    CHECK((INT_PTR)ShellExecuteA(0, "open", "http://www.nstorm.com", 0, 0, SW_SHOW) <= 32, "ShellExecuteA");
    CHECK(FindWindowA("Shell_TrayWnd", 0) == NULL, "FindWindowA");

    port_shutdown();
    fprintf(rep, "c_failures=%d\n", failures);
    fclose(rep);
    free(bmp);
    printf("shim_test: %d failure(s)\n", failures);
    return failures ? 1 : 0;
}
