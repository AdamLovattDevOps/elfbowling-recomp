/* Force-included in the MinGW build (pkg.mk, pkg-windows): the POSIX spellings the port uses
 * that MinGW's CRT declares differently. Nothing here changes behaviour on other platforms. */
#ifndef ELFBOWL_WIN_COMPAT_H
#define ELFBOWL_WIN_COMPAT_H
#include <string.h>
#include <strings.h>
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
/* POSIX mkdir(path, mode) -> MinGW mkdir(path) */
#define mkdir(p, m) _mkdir(p)
/* bcb_rtl.h defines these static inline; MinGW's <string.h> already declares them extern */
#define stricmp port_bcb_stricmp
#define strcmpi port_bcb_strcmpi
#define strnicmp port_bcb_strnicmp
/* kernel.c's shims of these clash with the kernel32 import-library thunks that libstdc++ and
 * winpthread pull in; the port's own callers get the shim under another name. */
#define TlsGetValue port_TlsGetValue
#define TlsSetValue port_TlsSetValue
#define TlsAlloc port_TlsAlloc
#define TlsFree port_TlsFree
#define LocalAlloc port_LocalAlloc
#define LocalFree port_LocalFree
#endif
