// Compile-only check: the shim headers work from C++ with the call shapes the game uses.
#include <windows.h>
#include <mmsystem.h>

static_assert(sizeof(WAVEFORMATEX) == 18, "WAVEFORMATEX is packed");
static_assert(sizeof(BITMAPINFOHEADER) == 40, "BITMAPINFOHEADER");
static_assert(sizeof(BITMAPFILEHEADER) == 14, "BITMAPFILEHEADER");
static_assert(sizeof(RGBQUAD) == 4 && sizeof(PALETTEENTRY) == 4, "colour structs");
static_assert(sizeof(LOGFONTA) == 60, "LOGFONTA");
static_assert(sizeof(RECT) == 16 && sizeof(POINT) == 8, "RECT/POINT use 32-bit LONG");
static_assert(sizeof(MEMORYSTATUS) == 32, "MEMORYSTATUS keeps the Win32 layout");
static_assert(sizeof(LPARAM) == sizeof(void *), "LPARAM is pointer-sized");

extern "C" int CALLBACK enum_cb(const LOGFONT *lf, const TEXTMETRIC *tm, DWORD type, LPARAM lp);
extern "C" void CALLBACK wave_cb(HWAVEOUT h, UINT msg, DWORD_PTR inst, DWORD_PTR p1, DWORD_PTR p2);

void header_cxx(HDC dc, HBITMAP bmp, HPALETTE pal, BITMAPINFO *bi, WAVEFORMATEX *fmt, WAVEHDR *hdr)
{
    HGDIOBJ br = GetStockObject(BLACK_BRUSH);
    SelectObject(dc, br);
    RECT r = {0, 0, 1, 1};
    FillRect(dc, &r, (HBRUSH)br);
    SelectObject(dc, bmp);
    BitBlt(dc, 0, 0, 1, 1, dc, 0, 0, SRCAND);
    HPALETTE old = SelectPalette(dc, pal, 0);
    RealizePalette(dc);
    SelectPalette(dc, old, 1);
    char *bits;
    CreateDIBSection(dc, bi, 0, (void **)&bits, 0, 0);
    LOGFONT lf;
    EnumFontFamiliesA(dc, "Arial", (FONTENUMPROC)enum_cb, (LPARAM)&lf);
    HFONT f = CreateFontIndirectA(&lf);
    DeleteObject(f);
    SetBkMode(dc, OPAQUE);
    SetTextColor(dc, 0xffff);
    DrawTextA(dc, "x", -1, &r, DT_WORDBREAK | 1);
    HWAVEOUT h = (HWAVEOUT)1;
    waveOutOpen(&h, WAVE_MAPPER, fmt, (DWORD_PTR)wave_cb, 0, CALLBACK_FUNCTION);
    waveOutPrepareHeader(h, hdr, 0x20);
    waveOutWrite(h, hdr, 0x20);
    int tag = mmioStringToFOURCC("fmt", 0);
    (void)tag;
    HRSRC rs = FindResourceA(0, "PackedFile", "NVDPACKFILE");
    LockResource(LoadResource(0, rs));
    HKEY key;
    RegOpenKeyExA(HKEY_CLASSES_ROOT, ".htm", 0, KEY_QUERY_VALUE, &key);
    MEMORYSTATUS ms;
    ms.dwLength = 32;
    GlobalMemoryStatus(&ms);
}
