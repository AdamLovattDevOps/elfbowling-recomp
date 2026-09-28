// The VCL and RTL as the matched source reaches them without the VCL
// headers:
//   - the "library / VCL / RTL entry points called by address" of
//     include/elf/funcs.h (extern "C" fn_XXXXXX, __fastcall with `this` first);
//   - the rtl_* aliases (libc);
//   - the plain-function spellings in vcl/elfcompat.h;
//   - the VCL globals' indirection cells (g_45fee4, g_45fee8, g_45fee0).
// Parameter types are void * here; extern "C" names do not encode them, and
// every one is a pointer or an int in both declarations.
// fn_43a8dc and Classes_TRect return the game's ERect and live in
// elf_glue.cpp, which needs include/elf/types.h.
#include <vcl/vcl.h>
#include <vcl/scktcomp.hpp>
#include <vcl/elfcompat.h>
#include <bcb_rtl.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <new>
#include <strings.h>

using Controls::TControl;
using Forms::TCustomForm;

static TControl *ctl(void *p) { return static_cast<TControl *>(static_cast<Classes::TComponent *>(p)); }

// ---- globals --------------------------------------------------------------------------------

extern "C" {
void **g_45fee4 = reinterpret_cast<void **>(&Forms::Application);
void **g_45fee8 = reinterpret_cast<void **>(&Forms::Screen);
HINSTANCE *g_45fee0 = &System::HInstance;
}

// ---- extern "C" entry points (funcs.h) ------------------------------------------------------

extern "C" {

// 0x42fdcc / 0x42fdd4: TScreen::GetHeight / GetWidth (GetSystemMetrics)
int fn_42fdcc(void *screen) { return static_cast<Forms::TScreen *>(screen)->GetHeight(); }
int fn_42fdd4(void *screen) { return static_cast<Forms::TScreen *>(screen)->GetWidth(); }

// 0x434158.. TControl::SetLeft / SetTop / SetWidth / SetHeight
void fn_434158(void *c, int v) { ctl(c)->SetLeft(v); }
void fn_434178(void *c, int v) { ctl(c)->SetTop(v); }
void fn_434198(void *c, int v) { ctl(c)->SetWidth(v); }
void fn_4341b8(void *c, int v) { ctl(c)->SetHeight(v); }

// 0x434238 / 0x43427c: TControl::GetClientWidth / GetClientHeight
int fn_434238(void *c) { return ctl(c)->GetClientWidth(); }
int fn_43427c(void *c) { return ctl(c)->GetClientHeight(); }

// 0x4348e8 TControl::BringToFront; 0x434b6c Hide; 0x434b74 Show
void fn_4348e8(void *c) { ctl(c)->BringToFront(); }
void fn_434b6c(void *c) { ctl(c)->Hide(); }
void fn_434b74(void *c) { ctl(c)->Show(); }

// 0x438384 TWinControl::GetHandle
HWND fn_438384(void *c) { return static_cast<Controls::TWinControl *>(ctl(c))->GetHandle(); }

// 0x43a8c4 Classes::Point
TPoint fn_43a8c4(int x, int y) { return Classes::Point(x, y); }

// 0x448a2c operator delete; 0x448a94 operator new[] (libmap mislabels it)
void fn_448a2c(void *p) { ::operator delete(p); }
void *fn_448a94(unsigned n) { return ::operator new[](n); }

// ---- rtl_* (funcs.h "RTL entry points declared by alias") ----
void *rtl_malloc(unsigned n) { return std::malloc(n); }
void rtl_free(void *p) { std::free(p); }
void rtl_delete(void *p) { ::operator delete(p); }
void *rtl_memcpy(void *d, const void *s, unsigned n) { return std::memcpy(d, s, n); }
void *rtl_memmove(void *d, const void *s, unsigned n) { return std::memmove(d, s, n); }
void *rtl_memset(void *d, int c, unsigned n) { return std::memset(d, c, n); }
unsigned rtl_strlen(const char *s) { return (unsigned)std::strlen(s); }
char *rtl_strcpy(char *d, const char *s) { return std::strcpy(d, s); }
char *rtl_strcat(char *d, const char *s) { return std::strcat(d, s); }
char *rtl_strncpy(char *d, const char *s, unsigned n) { return std::strncpy(d, s, n); }
char *rtl_strrchr(const char *s, int c) { return const_cast<char *>(std::strrchr(s, c)); }
int rtl_strcmp(const char *a, const char *b) { return std::strcmp(a, b); }
int rtl_stricmp(const char *a, const char *b) { return strcasecmp(a, b); }
long rtl_time(long *t)
{
    long v = (long)std::time(nullptr);
    if (t)
        *t = v;
    return v;
}
void rtl_srand(unsigned seed) { bcb_srand(seed); }     // the Borland generator (bcb_rtl.h)
void rtl_exit(int code) { std::exit(code); }

} // extern "C"

// ---- vcl/elfcompat.h ------------------------------------------------------------------------

void TCustomForm_Close(void *form) { static_cast<TCustomForm *>(ctl(form))->Close(); }
void TCustomForm_SetClientWidth(void *form, int v) { static_cast<TCustomForm *>(ctl(form))->SetClientWidth(v); }
void TCustomForm_SetClientHeight(void *form, int v) { static_cast<TCustomForm *>(ctl(form))->SetClientHeight(v); }
void TApplication_Terminate(void *app) { static_cast<Forms::TApplication *>(app)->Terminate(); }
int TApplication_MessageBox(void *app, const char *text, const char *caption, int flags)
{
    return static_cast<Forms::TApplication *>(app)->MessageBox(text, caption, flags);
}
int TScreen_GetWidth(void *scr) { return static_cast<Forms::TScreen *>(scr)->GetWidth(); }
int TScreen_GetHeight(void *scr) { return static_cast<Forms::TScreen *>(scr)->GetHeight(); }
int TControl_GetClientWidth(void *c) { return ctl(c)->GetClientWidth(); }
int TControl_GetClientHeight(void *c) { return ctl(c)->GetClientHeight(); }
void TCustomWinSocket_SendBuf(void *sock, void *buf, int count)
{
    static_cast<Scktcomp::TCustomWinSocket *>(sock)->SendBuf(buf, count);
}
int TCustomWinSocket_ReceiveLength(void *sock) { return static_cast<Scktcomp::TCustomWinSocket *>(sock)->ReceiveLength(); }
int TCustomWinSocket_ReceiveBuf(void *sock, void *buf, int count)
{
    return static_cast<Scktcomp::TCustomWinSocket *>(sock)->ReceiveBuf(buf, count);
}
void TThread_Synchronize(void *self, Classes::TThreadMethod method)
{
    static_cast<Classes::TThread *>(self)->Synchronize(method);
}

TPoint Classes_Point(int x, int y) { return Classes::Point(x, y); }
RECT Classes_Rect(int l, int t, int r, int b)
{
    RECT q;
    q.left = l;
    q.top = t;
    q.right = r;
    q.bottom = b;
    return q;
}
