// Small helpers 0x401cec-0x402840
#include <elf/funcs.h>

extern "C" void fn_401cec() {}
extern "C" void fn_401cf4(const char *fmt, ...) {}
extern "C" void fn_401cfc() {}
extern "C" void fn_401d04(const char *fmt, ...) {}

extern "C" void fn_401df8()
{
    HWND h = FindWindowA("Shell_TrayWnd", 0);
    if (h)
        ShowWindow(h, SW_HIDE);
}

extern "C" void fn_401e20()
{
    HWND h = FindWindowA("Shell_TrayWnd", 0);
    if (h)
        ShowWindow(h, SW_RESTORE);
}

extern "C" void fn_401f0c(const char *msg)
{
    TApplication_MessageBox(*g_45fee4, msg, "Offical Application Error", 0);
    rtl_exit(1);
}

extern "C" void fn_4022fc(const char *a)
{
    fn_401d04("%s:\n", a);
    fn_402224();
}

extern "C" void fn_402318(const char *a, const char *b)
{
    fn_401d04("%s%s:\n", a, b);
    fn_402224();
}

extern "C" void fn_402380(void *p, const char *who)
{
    g_45554c -= 2;
    fn_402194();
    rtl_free(p);
}

extern "C" char *fn_402410(char *d, const char *s, int n)
{
    rtl_strncpy(d, s, n);
    d[n] = 0;
    return d;
}

extern "C" int fn_4024bc(int a)
{
    return GetTickCount() + a;
}

extern "C" int fn_4024cc(int a, int b)
{
    if (!a)
        a = fn_4024bc(0);
    return a + b;
}

extern "C" char fn_4027d0(ERect r, TPoint p)
{
    return p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom;
}

extern "C" char fn_402800(ERect r, TPoint p)
{
    return r.left + 1 <= p.x && r.right - 1 > p.x;
}

extern "C" char fn_402820(ERect r, TPoint p)
{
    return r.top + 1 <= p.y && r.bottom - 1 > p.y;
}
