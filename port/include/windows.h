/* Portable Win32 subset for the Elf Bowling native port.
 *
 * Only the types, constants and functions the game code (src_match/) uses.
 * 64-bit clean: handles are pointers, LONG/DWORD are 32-bit as on Win32,
 * and LPARAM/WPARAM/DWORD_PTR are pointer-sized.
 * Implementations: port/src/{gdi,kernel,res}.c. See docs/SHIM.md.
 */
#ifndef PORT_WINDOWS_H
#define PORT_WINDOWS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- calling conventions (meaningless off x86-32) ---- */
#define WINAPI
#define CALLBACK
#define APIENTRY
#define WINAPIV
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif
/* Borland register convention (VCL methods, event handlers, funcs.h library
 * entry points). The native build only needs self-consistency. */
#if !defined(__BORLANDC__) && !defined(_MSC_VER) && !defined(__fastcall)
#define __fastcall
#endif

/* ---- scalar types (Win32 widths) ---- */
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef int32_t INT;
typedef uint32_t UINT;
typedef int BOOL;
typedef char CHAR;
typedef int16_t SHORT;
typedef uint16_t USHORT;
typedef float FLOAT;
typedef LONG *PLONG, *LPLONG;
typedef DWORD *LPDWORD;
typedef WORD *LPWORD;
typedef BYTE *LPBYTE;
typedef BOOL *LPBOOL;
typedef void *LPVOID, *PVOID;
typedef const void *LPCVOID;
typedef char *LPSTR, *PSTR;
typedef const char *LPCSTR, *PCSTR;
typedef intptr_t INT_PTR, LONG_PTR;
typedef uintptr_t UINT_PTR, ULONG_PTR, DWORD_PTR;
typedef size_t SIZE_T;
typedef LONG_PTR LPARAM;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LRESULT;
typedef LONG HRESULT;
typedef DWORD COLORREF;
typedef WORD ATOM;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif
#define MAX_PATH 260

/* ---- handles: distinct pointer types, like STRICT ---- */
#define DECLARE_HANDLE(n) struct n##__ { int unused; }; typedef struct n##__ *n
typedef void *HANDLE;
typedef void *HGDIOBJ;
DECLARE_HANDLE(HWND);
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HBITMAP);
DECLARE_HANDLE(HPALETTE);
DECLARE_HANDLE(HFONT);
DECLARE_HANDLE(HBRUSH);
DECLARE_HANDLE(HPEN);
DECLARE_HANDLE(HINSTANCE);
DECLARE_HANDLE(HRSRC);
DECLARE_HANDLE(HKEY);
typedef HINSTANCE HMODULE;
typedef HANDLE HGLOBAL;
typedef HANDLE HLOCAL;
typedef HKEY *PHKEY;
typedef INT_PTR (WINAPI *FARPROC)(void);

#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b) ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l) ((WORD)(((DWORD_PTR)(l) >> 16) & 0xffff))
#define LOBYTE(w) ((BYTE)((DWORD_PTR)(w) & 0xff))

/* ---- geometry ---- */
typedef struct tagRECT { LONG left, top, right, bottom; } RECT, *PRECT, *LPRECT;
typedef const RECT *LPCRECT;
typedef struct tagPOINT { LONG x, y; } POINT, *PPOINT, *LPPOINT;
typedef struct tagSIZE { LONG cx, cy; } SIZE, *PSIZE, *LPSIZE;

/* ---- colours ---- */
#define RGB(r, g, b) ((COLORREF)(((BYTE)(r)) | ((WORD)((BYTE)(g)) << 8) | (((DWORD)(BYTE)(b)) << 16)))
#define GetRValue(c) ((BYTE)(c))
#define GetGValue(c) ((BYTE)((c) >> 8))
#define GetBValue(c) ((BYTE)((c) >> 16))

typedef struct tagRGBQUAD { BYTE rgbBlue, rgbGreen, rgbRed, rgbReserved; } RGBQUAD, *LPRGBQUAD;
typedef struct tagPALETTEENTRY { BYTE peRed, peGreen, peBlue, peFlags; } PALETTEENTRY, *LPPALETTEENTRY;
typedef struct tagLOGPALETTE {
    WORD palVersion;
    WORD palNumEntries;
    PALETTEENTRY palPalEntry[1];
} LOGPALETTE, *PLOGPALETTE, *LPLOGPALETTE;

/* ---- bitmaps (file-format structs: exact Win32 layout) ---- */
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;          /* > 0 bottom-up, < 0 top-down */
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER, *LPBITMAPINFOHEADER;
typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[1];
} BITMAPINFO, *PBITMAPINFO, *LPBITMAPINFO;
#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1, bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pack(pop)
#define BI_RGB 0
#define DIB_RGB_COLORS 0
#define DIB_PAL_COLORS 1

/* ---- fonts ---- */
#define LF_FACESIZE 32
#define LF_FULLFACESIZE 64
typedef struct tagLOGFONTA {
    LONG lfHeight, lfWidth, lfEscapement, lfOrientation, lfWeight;
    BYTE lfItalic, lfUnderline, lfStrikeOut, lfCharSet;
    BYTE lfOutPrecision, lfClipPrecision, lfQuality, lfPitchAndFamily;
    CHAR lfFaceName[LF_FACESIZE];
} LOGFONTA, *PLOGFONTA, *LPLOGFONTA;
typedef struct tagENUMLOGFONTA {
    LOGFONTA elfLogFont;
    BYTE elfFullName[LF_FULLFACESIZE];
    BYTE elfStyle[LF_FACESIZE];
} ENUMLOGFONTA, *LPENUMLOGFONTA;
typedef struct tagTEXTMETRICA {
    LONG tmHeight, tmAscent, tmDescent, tmInternalLeading, tmExternalLeading;
    LONG tmAveCharWidth, tmMaxCharWidth, tmWeight, tmOverhang;
    LONG tmDigitizedAspectX, tmDigitizedAspectY;
    BYTE tmFirstChar, tmLastChar, tmDefaultChar, tmBreakChar;
    BYTE tmItalic, tmUnderlined, tmStruckOut, tmPitchAndFamily, tmCharSet;
} TEXTMETRICA, *LPTEXTMETRICA;
typedef int (CALLBACK *FONTENUMPROCA)(const LOGFONTA *, const TEXTMETRICA *, DWORD, LPARAM);
typedef LOGFONTA LOGFONT;
typedef ENUMLOGFONTA ENUMLOGFONT;
typedef TEXTMETRICA TEXTMETRIC;
typedef FONTENUMPROCA FONTENUMPROC;

#define FW_DONTCARE 0
#define FW_THIN 100
#define FW_LIGHT 300
#define FW_NORMAL 400
#define FW_SEMIBOLD 600
#define FW_BOLD 700
#define ANSI_CHARSET 0
#define DEFAULT_CHARSET 1
#define RASTER_FONTTYPE 1
#define DEVICE_FONTTYPE 2
#define TRUETYPE_FONTTYPE 4

/* ---- GDI constants ---- */
#define SRCCOPY     0x00CC0020u
#define SRCPAINT    0x00EE0086u
#define SRCAND      0x008800C6u
#define SRCINVERT   0x00660046u
#define NOTSRCCOPY  0x00330008u
#define BLACKNESS   0x00000042u
#define WHITENESS   0x00FF0062u

#define WHITE_BRUSH 0
#define LTGRAY_BRUSH 1
#define GRAY_BRUSH 2
#define DKGRAY_BRUSH 3
#define BLACK_BRUSH 4
#define NULL_BRUSH 5
#define SYSTEM_FONT 13
#define DEFAULT_PALETTE 15

#define HORZRES 8
#define VERTRES 10
#define BITSPIXEL 12
#define PLANES 14
#define NUMCOLORS 24
#define RASTERCAPS 38
#define SIZEPALETTE 104
#define NUMRESERVED 106
#define RC_BITBLT 1
#define RC_PALETTE 0x0100
#define RC_DI_BITMAP 0x0080

#define TRANSPARENT 1
#define OPAQUE 2

#define DT_TOP 0x0000
#define DT_LEFT 0x0000
#define DT_CENTER 0x0001
#define DT_RIGHT 0x0002
#define DT_VCENTER 0x0004
#define DT_BOTTOM 0x0008
#define DT_WORDBREAK 0x0010
#define DT_SINGLELINE 0x0020
#define DT_EXPANDTABS 0x0040
#define DT_NOCLIP 0x0100
#define DT_CALCRECT 0x0400
#define DT_NOPREFIX 0x0800

/* ---- GDI ---- */
HDC WINAPI CreateCompatibleDC(HDC hdc);
HDC WINAPI CreateICA(LPCSTR driver, LPCSTR device, LPCSTR port, const void *devmode);
BOOL WINAPI DeleteDC(HDC hdc);
HGDIOBJ WINAPI SelectObject(HDC hdc, HGDIOBJ obj);
BOOL WINAPI DeleteObject(HGDIOBJ obj);
HGDIOBJ WINAPI GetStockObject(int i);
HBITMAP WINAPI CreateDIBSection(HDC hdc, const BITMAPINFO *bmi, UINT usage, void **bits,
                                HANDLE section, DWORD offset);
HPALETTE WINAPI CreatePalette(const LOGPALETTE *lp);
HPALETTE WINAPI SelectPalette(HDC hdc, HPALETTE pal, BOOL forceBackground);
UINT WINAPI RealizePalette(HDC hdc);
BOOL WINAPI BitBlt(HDC dst, int x, int y, int w, int h, HDC src, int sx, int sy, DWORD rop);
int WINAPI GetDeviceCaps(HDC hdc, int index);
COLORREF WINAPI SetTextColor(HDC hdc, COLORREF c);
int WINAPI SetBkMode(HDC hdc, int mode);
HFONT WINAPI CreateFontIndirectA(const LOGFONTA *lf);
int WINAPI EnumFontFamiliesA(HDC hdc, LPCSTR family, FONTENUMPROCA proc, LPARAM lp);

/* ---- USER ---- */
HDC WINAPI GetDC(HWND hwnd);
int WINAPI ReleaseDC(HWND hwnd, HDC hdc);
int WINAPI FillRect(HDC hdc, const RECT *r, HBRUSH br);
int WINAPI DrawTextA(HDC hdc, LPCSTR text, int n, LPRECT r, UINT format);
HWND WINAPI FindWindowA(LPCSTR cls, LPCSTR title);
BOOL WINAPI ShowWindow(HWND hwnd, int cmd);

#define SW_HIDE 0
#define SW_SHOWNORMAL 1
#define SW_NORMAL 1
#define SW_SHOWMINIMIZED 2
#define SW_SHOWMAXIMIZED 3
#define SW_MAXIMIZE 3
#define SW_SHOWNOACTIVATE 4
#define SW_SHOW 5
#define SW_MINIMIZE 6
#define SW_RESTORE 9

/* MessageBox flags and results (used by the VCL layer, port/vcl/) */
#define MB_OK 0x0000
#define MB_OKCANCEL 0x0001
#define MB_YESNO 0x0004
#define MB_ICONHAND 0x0010
#define MB_ICONSTOP MB_ICONHAND
#define MB_ICONERROR MB_ICONHAND
#define MB_ICONQUESTION 0x0020
#define MB_ICONEXCLAMATION 0x0030
#define MB_ICONWARNING MB_ICONEXCLAMATION
#define MB_ICONASTERISK 0x0040
#define MB_ICONINFORMATION MB_ICONASTERISK
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

/* Virtual-key codes (what the VCL passes as OnKeyDown/OnKeyUp Key) */
#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_MBUTTON 0x04
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_CLEAR 0x0C
#define VK_RETURN 0x0D
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_PAUSE 0x13
#define VK_CAPITAL 0x14
#define VK_ESCAPE 0x1B
#define VK_SPACE 0x20
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define VK_SNAPSHOT 0x2C
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_HELP 0x2F
#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D
#define VK_NUMPAD0 0x60
#define VK_MULTIPLY 0x6A
#define VK_ADD 0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE 0x6F
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_F13 0x7C
#define VK_F14 0x7D
#define VK_F15 0x7E
#define VK_F16 0x7F
#define VK_F17 0x80
#define VK_F18 0x81
#define VK_F19 0x82
#define VK_F20 0x83
#define VK_F21 0x84
#define VK_F22 0x85
#define VK_F23 0x86
#define VK_F24 0x87
#define VK_NUMLOCK 0x90
#define VK_SCROLL 0x91
#define VK_OEM_1 0xBA
#define VK_OEM_PLUS 0xBB
#define VK_OEM_COMMA 0xBC
#define VK_OEM_MINUS 0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2 0xBF
#define VK_OEM_3 0xC0
#define VK_OEM_4 0xDB
#define VK_OEM_5 0xDC
#define VK_OEM_6 0xDD
#define VK_OEM_7 0xDE

/* ---- KERNEL ---- */
typedef struct _MEMORYSTATUS {     /* Win32 layout: 32 bytes (the game sets dwLength = 32) */
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORD dwTotalPhys;
    DWORD dwAvailPhys;
    DWORD dwTotalPageFile;
    DWORD dwAvailPageFile;
    DWORD dwTotalVirtual;
    DWORD dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

DWORD WINAPI GetTickCount(void);
void WINAPI GlobalMemoryStatus(LPMEMORYSTATUS ms);

#define TLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)
DWORD WINAPI TlsAlloc(void);
BOOL WINAPI TlsFree(DWORD idx);
LPVOID WINAPI TlsGetValue(DWORD idx);
BOOL WINAPI TlsSetValue(DWORD idx, LPVOID value);

#define LMEM_FIXED 0x0000
#define LMEM_MOVEABLE 0x0002
#define LMEM_ZEROINIT 0x0040
#define LPTR (LMEM_FIXED | LMEM_ZEROINIT)
#define LHND (LMEM_MOVEABLE | LMEM_ZEROINIT)
HLOCAL WINAPI LocalAlloc(UINT flags, SIZE_T bytes);
HLOCAL WINAPI LocalFree(HLOCAL h);

LPSTR WINAPI lstrcpyA(LPSTR d, LPCSTR s);
LPSTR WINAPI lstrcatA(LPSTR d, LPCSTR s);
int WINAPI lstrlenA(LPCSTR s);

HMODULE WINAPI LoadLibraryA(LPCSTR name);
FARPROC WINAPI GetProcAddress(HMODULE mod, LPCSTR name);
BOOL WINAPI FreeLibrary(HMODULE mod);
UINT WINAPI WinExec(LPCSTR cmd, UINT show);

/* ---- resources ---- */
#define MAKEINTRESOURCEA(i) ((LPSTR)(ULONG_PTR)((WORD)(i)))
#define MAKEINTRESOURCE MAKEINTRESOURCEA
#define IS_INTRESOURCE(p) ((((ULONG_PTR)(p)) >> 16) == 0)
#define RT_CURSOR MAKEINTRESOURCEA(1)
#define RT_BITMAP MAKEINTRESOURCEA(2)
#define RT_ICON MAKEINTRESOURCEA(3)
#define RT_STRING MAKEINTRESOURCEA(6)
#define RT_RCDATA MAKEINTRESOURCEA(10)
#define RT_VERSION MAKEINTRESOURCEA(16)

HRSRC WINAPI FindResourceA(HMODULE mod, LPCSTR name, LPCSTR type);
HGLOBAL WINAPI LoadResource(HMODULE mod, HRSRC res);
LPVOID WINAPI LockResource(HGLOBAL h);
DWORD WINAPI SizeofResource(HMODULE mod, HRSRC res);

/* ---- registry / shell (stubs) ---- */
#define HKEY_CLASSES_ROOT ((HKEY)(ULONG_PTR)0x80000000u)
#define HKEY_CURRENT_USER ((HKEY)(ULONG_PTR)0x80000001u)
#define HKEY_LOCAL_MACHINE ((HKEY)(ULONG_PTR)0x80000002u)
#define KEY_QUERY_VALUE 0x0001
#define KEY_READ 0x20019
#define ERROR_SUCCESS 0L
#define ERROR_FILE_NOT_FOUND 2L
typedef DWORD REGSAM;
LONG WINAPI RegOpenKeyExA(HKEY key, LPCSTR sub, DWORD opts, REGSAM sam, PHKEY out);
LONG WINAPI RegQueryValueA(HKEY key, LPCSTR sub, LPSTR data, PLONG size);
LONG WINAPI RegCloseKey(HKEY key);
HINSTANCE WINAPI ShellExecuteA(HWND hwnd, LPCSTR op, LPCSTR file, LPCSTR params, LPCSTR dir, INT show);

#ifdef __cplusplus
}
#endif

#include "port.h"

#endif /* PORT_WINDOWS_H */
