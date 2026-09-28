/* Windows file picker for pkg/firstrun.c. Built with the real Windows headers (not the port's
 * include/windows.h), so it lives in its own unit (pkg.mk, pkg-windows). */
#include <windows.h>
#include <commdlg.h>
#include <string.h>
#include "firstrun.h"

int elfbowl_pick_file(char *out, size_t outsz)
{
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof ofn);
    out[0] = 0;
    ofn.lStructSize = sizeof ofn;
    ofn.lpstrFilter = "Elf Bowling.exe\0Elf Bowling.exe;*.exe\0All files\0*.*\0";
    ofn.lpstrFile = out;
    ofn.nMaxFile = (DWORD)outsz;
    ofn.lpstrTitle = "Choose the original Elf Bowling.exe (NStorm, 1999)";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    return GetOpenFileNameA(&ofn) ? 0 : -1;
}
