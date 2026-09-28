/* FindResourceA / LoadResource / LockResource / SizeofResource, served from the
 * user's original "Elf Bowling.exe" (never from the repo). The PE resource tree
 * (type / name / language) is read once into a flat table, as tools/rsrc.py
 * walks it. The module handle argument is ignored: every HMODULE is the exe. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "shim.h"

typedef struct ResKey {
    int id;                 /* >= 0: numeric id; -1: named */
    char name[64];          /* upper-case ASCII of the UTF-16 name */
} ResKey;

typedef struct ResEntry {
    ResKey type, name;
    int lang;
    unsigned char *data;
    DWORD size;
} ResEntry;

static unsigned char *s_img;
static size_t s_imgSize;
static ResEntry *s_res;
static int s_nres;
static int s_tried;

static unsigned rd16(size_t o) { return o + 2 <= s_imgSize ? s_img[o] | s_img[o + 1] << 8 : 0; }
static unsigned rd32(size_t o) { return o + 4 <= s_imgSize ? rd16(o) | rd16(o + 2) << 16 : 0; }

/* section table for RVA -> file offset */
static size_t s_secOff;
static int s_nsec;

static long rva2off(unsigned rva)
{
    for (int i = 0; i < s_nsec; i++) {
        size_t s = s_secOff + 40 * (size_t)i;
        unsigned vs = rd32(s + 8), va = rd32(s + 12), rs = rd32(s + 16), ro = rd32(s + 20);
        unsigned span = vs > rs ? vs : rs;
        if (rva >= va && rva < va + span)
            return (long)(ro + (rva - va));
    }
    return -1;
}

static void read_key(size_t base, unsigned e, ResKey *k)
{
    if (e & 0x80000000u) {
        size_t o = base + (e & 0x7fffffff);
        unsigned n = rd16(o);
        k->id = -1;
        if (n >= sizeof k->name)
            n = sizeof k->name - 1;
        for (unsigned i = 0; i < n; i++) {
            unsigned c = rd16(o + 2 + 2 * i);
            k->name[i] = (char)toupper(c < 128 ? (int)c : '?');
        }
        k->name[n] = 0;
    } else {
        k->id = (int)(e & 0xffff);
        k->name[0] = 0;
    }
}

static int add(const ResKey *t, const ResKey *n, int lang, unsigned drva, unsigned size)
{
    long off = rva2off(drva);
    if (off < 0 || (size_t)off + size > s_imgSize)
        return -1;
    ResEntry *r = realloc(s_res, (s_nres + 1) * sizeof *r);
    if (!r)
        return -1;
    s_res = r;
    r += s_nres++;
    r->type = *t;
    r->name = *n;
    r->lang = lang;
    r->data = s_img + off;
    r->size = size;
    return 0;
}

/* Walk the three-level directory: type -> name -> language -> data entry. */
static void walk(size_t base, size_t dir, int level, ResKey *keys)
{
    unsigned nnamed = rd16(base + dir + 12), nid = rd16(base + dir + 14);
    for (unsigned i = 0; i < nnamed + nid; i++) {
        size_t e = base + dir + 16 + 8 * i;
        unsigned nm = rd32(e), tgt = rd32(e + 4);
        if (level < 2)
            read_key(base, nm, &keys[level]);
        if (tgt & 0x80000000u) {
            if (level < 2)
                walk(base, tgt & 0x7fffffff, level + 1, keys);
        } else if (level == 2) {
            add(&keys[0], &keys[1], (int)(nm & 0xffff), rd32(base + tgt), rd32(base + tgt + 4));
        }
    }
}

const unsigned char *port_res_image(size_t *size)
{
    if (size)
        *size = s_imgSize;
    return s_img;
}

int port_res_open(const char *path)
{
    s_tried = 1;
    if (!path)
        path = getenv("ELFBOWL_EXE");
    if (!path)
        path = "Elf Bowling.exe";
    FILE *f = fopen(path, "rb");
    if (!f) {
        port_log("resources: cannot open \"%s\" (set ELFBOWL_EXE to the original Elf Bowling.exe)", path);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *img = sz > 0 ? malloc(sz) : NULL;
    if (!img || fread(img, 1, sz, f) != (size_t)sz) {
        fclose(f);
        free(img);
        return -1;
    }
    fclose(f);
    free(s_img);
    free(s_res);
    s_img = img;
    s_imgSize = (size_t)sz;
    s_res = NULL;
    s_nres = 0;

    size_t pe = rd32(0x3c);
    if (rd16(0) != 0x5a4d || rd32(pe) != 0x4550 || rd16(pe + 24) != 0x10b) {
        port_log("resources: \"%s\" is not a PE32 image", path);
        return -1;
    }
    size_t opt = pe + 24;
    s_nsec = (int)rd16(pe + 6);
    s_secOff = opt + rd16(pe + 20);
    if (rd32(opt + 92) < 3) {
        port_log("resources: \"%s\" has no resource directory", path);
        return -1;
    }
    long base = rva2off(rd32(opt + 96 + 8 * 2));
    if (base < 0)
        return -1;
    ResKey keys[2];
    walk((size_t)base, 0, 0, keys);
    port_log("resources: %d entries from \"%s\"", s_nres, path);
    return s_nres > 0 ? 0 : -1;
}

static int ci_equal(const char *a, const char *b)
{
    while (*a && toupper((unsigned char)*a) == toupper((unsigned char)*b))
        a++, b++;
    return toupper((unsigned char)*a) == toupper((unsigned char)*b);
}

/* Match a Win32 name/type argument: MAKEINTRESOURCE id, "#123", or a string
 * compared case-insensitively (resource compilers store names upper-case). */
static int key_match(const ResKey *k, LPCSTR s)
{
    if (IS_INTRESOURCE(s))
        return k->id == (int)(ULONG_PTR)s;
    if (s[0] == '#')
        return k->id == atoi(s + 1);
    return k->id < 0 && ci_equal(k->name, s);
}

HRSRC WINAPI FindResourceA(HMODULE mod, LPCSTR name, LPCSTR type)
{
    (void)mod;
    if (!s_tried)
        port_res_open(NULL);
    if (!name || !type)
        return NULL;
    for (int i = 0; i < s_nres; i++)
        if (key_match(&s_res[i].type, type) && key_match(&s_res[i].name, name))
            return (HRSRC)&s_res[i];
    return NULL;
}

HGLOBAL WINAPI LoadResource(HMODULE mod, HRSRC res)
{
    (void)mod;
    return res ? (HGLOBAL)((ResEntry *)res)->data : NULL;
}

LPVOID WINAPI LockResource(HGLOBAL h) { return h; }

DWORD WINAPI SizeofResource(HMODULE mod, HRSRC res)
{
    (void)mod;
    return res ? ((ResEntry *)res)->size : 0;
}
