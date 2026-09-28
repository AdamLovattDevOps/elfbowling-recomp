// TPackedResources: find the next entry with a given extension, 0x40db00.
#include <elf/funcs.h>
#include <string.h>

extern "C" char fn_40db00(TPackedResources *pr, const char *ext, char *out)
{
    char e[8];
    char found;
    if (*ext == '.') {
        fn_402410(e, ext, 4);
    } else {
        e[0] = '.';
        fn_402410(e + 1, ext, 3);
    }
    found = 0;
    *out = 0;
    while (pr->iter < pr->count && !found) {
        PackEntry *en = &pr->entries[pr->iter];
        char *dot = strrchr(en->name, '.');
        if (dot)
            found = rtl_stricmp(e, dot) == 0;
        pr->iter++;
    }
    if (found)
        strcpy(out, pr->entries[pr->iter - 1].name);
    return found;
}
