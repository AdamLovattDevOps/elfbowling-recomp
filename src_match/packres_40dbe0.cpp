// Packed stream / saved cast+parm resources, 0x40dbe0-0x40e5ec.
#include <elf/funcs.h>
#include <string.h>

extern "C" char fn_40dbe0(TPackedResources *pr, const char *ext, char *out)
{
    pr->iter = 0;
    return fn_40db00(pr, ext, out);
}

extern "C" void fn_40dc04(const char *msg)
{
    fn_401f30("Decompression Error: ", msg);
}

extern "C" void __fastcall fn_40dfd0(TPackedStreamData *s, int v)
{
    s->outsize = v;
}

extern "C" int __fastcall fn_40dfec(TPackedStreamData *s, const void *buf, int n)
{
    return 0;
}

extern "C" int __fastcall fn_40e004(TPackedStreamData *s, int off, unsigned short origin)
{
    if (origin == 2)
        s->pos = s->size + off;
    else if (origin == 1)
        s->pos += off;
    else
        s->pos = off;
    s->pos = tmax(0, s->pos);
    s->pos = tmin(s->size, s->pos);
    return s->pos;
}

extern "C" void fn_40e210(TSaveCasts *c, int flags)
{
    if (c) {
        fn_401cf4("Cast Parms reports: Original bitmap memory = %d, New bitmap memory = %d, Savings = %d\n",
                  g_455534, g_455538, g_455534 - g_455538);
        if (flags & 1)
            fn_448a2c(c);
    }
}

extern "C" char fn_40e250(TSaveCasts *c)
{
    char ok = 1;
    memmove(c, c->data, 0x28);
    if (strncmp(c->magic, "The NStorm Cannon Rules!", 0x1e) == 0)
        c->entries = (CastEntry *)(c->data + c->hdrsize);
    else
        ok = 0;
    return ok;
}

extern "C" char fn_40e2a0(TSaveCasts *c, const char *name)
{
    char ok = 0;
    HRSRC r = FindResourceA(*g_45fee0, name, "NVDCASTFILE");
    if (r) {
        HGLOBAL h = LoadResource(*g_45fee0, r);
        if (h) {
            c->data = (char *)LockResource(h);
            ok = fn_40e250(c);
        }
    }
    return ok;
}

extern "C" CastEntry *fn_40e304(TSaveCasts *c, const char *name)
{
    CastEntry *r = 0;
    for (int i = 0; i < c->count; i++) {
        if (rtl_stricmp(c->entries[i].name, name) == 0)
            r = &c->entries[i];
    }
    return r;
}

extern "C" char fn_40e354(TSaveCasts *c, void *unused)
{
    char ok = 0;
    if (!ok)
        ok = fn_40e2a0(c, "Casts");
    return ok;
}

extern "C" char fn_40e3d4(TSaveParms *p)
{
    char ok = 1;
    memmove(p, p->data, 0x30);
    if (strncmp(p->magic, "NV us, you strange little monkey!", 0x27) == 0)
        p->entries = (ParmEntry *)(p->data + p->hdrsize);
    else
        ok = 0;
    return ok;
}

extern "C" char fn_40e424(TSaveParms *p, const char *name)
{
    char ok = 0;
    HRSRC r = FindResourceA(*g_45fee0, name, "NVDPARMFILE");
    if (r) {
        HGLOBAL h = LoadResource(*g_45fee0, r);
        if (h) {
            p->data = (char *)LockResource(h);
            ok = fn_40e3d4(p);
        }
    }
    return ok;
}

extern "C" ParmEntry *fn_40e488(TSaveParms *p, const char *name)
{
    ParmEntry *r = 0;
    for (int i = 0; i < p->count; i++) {
        if (rtl_stricmp(p->entries[i].name, name) == 0)
            r = &p->entries[i];
    }
    return r;
}

extern "C" char fn_40e4d8(TSaveParms *sp, TScene *sc)
{
    char ok = 1;
    for (int i = 0; i < sc->nsprites; i++) {
        TGraphicSprite *s = sc->sprites[i];
        if (!s->keepParms) {
            ParmEntry *p = fn_40e488(sp, s->name);
            if (p)
                fn_40659c(s, p->f1c, p->f20, p->f24, p->f25);
        }
    }
    return ok;
}

extern "C" char fn_40e55c(TSaveParms *p, TStage *unused)
{
    char ok = 0;
    if (!ok)
        ok = fn_40e424(p, "Parms");
    return ok;
}

extern "C" void __fastcall fn_40e584(int err)
{
    g_46020c->state = 6;
    fn_401cf4("Socket Error %d.\n", err);
}

extern "C" void __fastcall fn_40e5ac(void *self, void *sender, void *socket)
{
    fn_401cf4("Connection established.\n");
}

extern "C" void __fastcall fn_40e5cc(void *self, void *sender, void *socket)
{
    fn_401cf4("Looking Up Host.\n");
}
