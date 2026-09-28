/* KERNEL32 / ADVAPI32 / SHELL32 subset, plus the shim's logging and SDL setup.
 * Real: GetTickCount, TLS, LocalAlloc/Free, lstr*. Approximate:
 * GlobalMemoryStatus. Stubs (logged): LoadLibrary/GetProcAddress/FreeLibrary,
 * registry, ShellExecuteA, WinExec. */
#include <SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shim.h"

void port_gdi_shutdown(void);
void port_winmm_shutdown(void);

/* ------------------------------------------------------------------ port */

void port_log(const char *fmt, ...)
{
    static int enabled = -1;
    if (enabled < 0) {
        const char *e = getenv("PORT_LOG");
        enabled = !(e && *e == '0');
    }
    if (!enabled)
        return;
    va_list ap;
    va_start(ap, fmt);
    fputs("[shim] ", stderr);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}

int port_sdl_init(unsigned flags)
{
    if ((SDL_WasInit(flags) & flags) == flags)
        return 0;
    if (SDL_InitSubSystem(flags)) {
        port_log("SDL_InitSubSystem(%#x): %s", flags, SDL_GetError());
        return -1;
    }
    return 0;
}

void port_shutdown(void)
{
    port_winmm_shutdown();
    port_gdi_shutdown();
    SDL_Quit();
}

/* ------------------------------------------------------------------ time / memory */

DWORD WINAPI GetTickCount(void)
{
    /* milliseconds since the shim first asked; wraps at 2^32 like Win32 */
    return (DWORD)SDL_GetTicks64();
}

void WINAPI GlobalMemoryStatus(LPMEMORYSTATUS ms)
{
    /* Win32 GlobalMemoryStatus saturates on large machines; report the host's
     * RAM clamped to 2 GB, with a fixed 3/4 available. */
    unsigned long long total = (unsigned long long)SDL_GetSystemRAM() * 1024 * 1024;
    if (total == 0 || total > 0x7fffffffULL)
        total = 0x7fffffffULL;
    if (!ms)
        return;
    ms->dwLength = sizeof *ms;
    ms->dwMemoryLoad = 25;
    ms->dwTotalPhys = (DWORD)total;
    ms->dwAvailPhys = (DWORD)(total / 4 * 3);
    ms->dwTotalPageFile = (DWORD)total;
    ms->dwAvailPageFile = (DWORD)(total / 4 * 3);
    ms->dwTotalVirtual = 0x7ffe0000u;
    ms->dwAvailVirtual = 0x7ff00000u;
}

/* ------------------------------------------------------------------ TLS */

/* SDL TLS ids are never released, so TlsFree only clears the value; freed
 * indices are reused by TlsAlloc. */
#define MAX_TLS 64
static SDL_TLSID s_tls[MAX_TLS];
static int s_tlsUsed[MAX_TLS];

DWORD WINAPI TlsAlloc(void)
{
    for (int i = 0; i < MAX_TLS; i++) {
        if (!s_tlsUsed[i]) {
            if (!s_tls[i])
                s_tls[i] = SDL_TLSCreate();
            if (!s_tls[i])
                return TLS_OUT_OF_INDEXES;
            s_tlsUsed[i] = 1;
            SDL_TLSSet(s_tls[i], NULL, NULL);
            return (DWORD)i;
        }
    }
    return TLS_OUT_OF_INDEXES;
}

BOOL WINAPI TlsFree(DWORD idx)
{
    if (idx >= MAX_TLS || !s_tlsUsed[idx])
        return FALSE;
    SDL_TLSSet(s_tls[idx], NULL, NULL);
    s_tlsUsed[idx] = 0;
    return TRUE;
}

LPVOID WINAPI TlsGetValue(DWORD idx)
{
    return idx < MAX_TLS && s_tlsUsed[idx] ? SDL_TLSGet(s_tls[idx]) : NULL;
}

BOOL WINAPI TlsSetValue(DWORD idx, LPVOID v)
{
    if (idx >= MAX_TLS || !s_tlsUsed[idx])
        return FALSE;
    SDL_TLSSet(s_tls[idx], v, NULL);   /* return convention differs under sdl2-compat */
    return SDL_TLSGet(s_tls[idx]) == v;
}

/* ------------------------------------------------------------------ LocalAlloc */

/* Every HLOCAL is the block pointer itself (LMEM_MOVEABLE is treated as fixed). */
HLOCAL WINAPI LocalAlloc(UINT flags, SIZE_T bytes)
{
    if (bytes == 0)
        bytes = 1;
    return (flags & LMEM_ZEROINIT) ? calloc(1, bytes) : malloc(bytes);
}

HLOCAL WINAPI LocalFree(HLOCAL h)
{
    free(h);
    return NULL;
}

/* ------------------------------------------------------------------ lstr* */

LPSTR WINAPI lstrcpyA(LPSTR d, LPCSTR s)
{
    if (!d || !s)
        return NULL;
    return strcpy(d, s);
}

LPSTR WINAPI lstrcatA(LPSTR d, LPCSTR s)
{
    if (!d || !s)
        return NULL;
    return strcat(d, s);
}

int WINAPI lstrlenA(LPCSTR s) { return s ? (int)strlen(s) : 0; }

/* ------------------------------------------------------------------ DLLs */

HMODULE WINAPI LoadLibraryA(LPCSTR name)
{
    PORT_STUB("LoadLibraryA", "(\"%s\") -> NULL", name ? name : "(null)");
    return NULL;
}

FARPROC WINAPI GetProcAddress(HMODULE mod, LPCSTR name)
{
    PORT_STUB("GetProcAddress", "(%p, \"%s\") -> NULL", (void *)mod,
              IS_INTRESOURCE(name) ? "#ordinal" : name);
    return NULL;
}

BOOL WINAPI FreeLibrary(HMODULE mod)
{
    PORT_STUB("FreeLibrary", "(%p)", (void *)mod);
    return TRUE;
}

/* ------------------------------------------------------------------ registry / shell */

LONG WINAPI RegOpenKeyExA(HKEY key, LPCSTR sub, DWORD opts, REGSAM sam, PHKEY out)
{
    (void)opts; (void)sam;
    PORT_STUB("RegOpenKeyExA", "(%p, \"%s\") -> ERROR_FILE_NOT_FOUND", (void *)key, sub ? sub : "");
    if (out)
        *out = NULL;
    return ERROR_FILE_NOT_FOUND;
}

LONG WINAPI RegQueryValueA(HKEY key, LPCSTR sub, LPSTR data, PLONG size)
{
    (void)data; (void)size;
    PORT_STUB("RegQueryValueA", "(%p, \"%s\") -> ERROR_FILE_NOT_FOUND", (void *)key, sub ? sub : "");
    return ERROR_FILE_NOT_FOUND;
}

LONG WINAPI RegCloseKey(HKEY key)
{
    (void)key;
    return ERROR_SUCCESS;
}

/* Opens http(s) URLs with the host browser only when PORT_OPEN_URLS=1;
 * otherwise logs and reports failure (SE_ERR_NOASSOC, 31). */
HINSTANCE WINAPI ShellExecuteA(HWND hwnd, LPCSTR op, LPCSTR file, LPCSTR params, LPCSTR dir, INT show)
{
    (void)hwnd; (void)params; (void)dir; (void)show;
    const char *allow = getenv("PORT_OPEN_URLS");
    int isUrl = file && (!strncmp(file, "http://", 7) || !strncmp(file, "https://", 8));
    if (allow && *allow == '1' && isUrl && (!op || !strcmp(op, "open")) && SDL_OpenURL(file) == 0) {
        port_log("ShellExecuteA: opened %s", file);
        return (HINSTANCE)(ULONG_PTR)42;
    }
    PORT_STUB("ShellExecuteA", "(\"%s\", \"%s\") not executed (set PORT_OPEN_URLS=1 for URLs)",
              op ? op : "", file ? file : "");
    return (HINSTANCE)(ULONG_PTR)31;
}

UINT WINAPI WinExec(LPCSTR cmd, UINT show)
{
    (void)show;
    PORT_STUB("WinExec", "(\"%s\") not executed", cmd ? cmd : "");
    return 2;   /* ERROR_FILE_NOT_FOUND */
}
