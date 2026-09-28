// elf/packres.h - packed resources: TPackedResources (the NVDPACKFILE
// archive, 0xb8 bytes), TPackedStream (a zlib-inflating TStream, 0x50
// bytes), TSaveParms (PARMS) and TSaveCasts (CASTS).
// RTTI names: TPackedResources, TPackedStream, TSaveParms, TSaveCasts.
// Globals: g_455524 packed resources, g_455528 TSaveParms (see game.h).
//
// Old local names: PackRes, PackedFile -> TPackedResources; PStream ->
// TPackedStream (as seen by non-VCL units); SaveParms -> TSaveParms;
// SaveCasts -> TSaveCasts; ZStream -> z_stream (zlib 1.0 layout).
#ifndef ELF_PACKRES_H
#define ELF_PACKRES_H

#include <elf/types.h>

struct TSaveCasts;

// Directory entry (40 bytes). Layout confirmed by tools/unpack.py and by
// fn_40d4bc, which opens the stream at hdr + dataOffset with csize input
// bytes and size output bytes. (The first header draft had the names of
// 0x18 and 0x20 swapped; packres_40d4bc called them offset/packed.)
struct PackEntry {
    char name[0x18];            // 0x00
    int csize;                  // 0x18 compressed (zlib) size (old: offset)
    int size;                   // 0x1c unpacked size
    int dataOffset;             // 0x20 offset of the data from the archive start (old: packed)
    int f24;                    // 0x24 always 0
};
ELF_CHECK_OFS(PackEntry, csize, 0x18);
ELF_CHECK_OFS(PackEntry, dataOffset, 0x20);
ELF_CHECK_SIZE(PackEntry, 0x28);

// Archive header as loaded (entries follow at 0x18).
struct PackHdr {
    char _unk00[0x10];
    int count;                  // 0x10
    int dataStart;              // 0x14 = 0x18 + 40 * count
    PackEntry entries[1];       // 0x18
};
ELF_CHECK_OFS(PackHdr, entries, 0x18);

struct TPackedResources {
    HGLOBAL hres;               // 0x00
    PackHdr *hdr;               // 0x04 (packres_40d4bc: char *data)
    PackEntry *entries;         // 0x08
    char *loaded;               // 0x0c per-entry "used" flags
    int count;                  // 0x10
    DWORD size;                 // 0x14
    int outsize;                // 0x18
    char *outbuf;               // 0x1c (packres_40dce0: void *f1c)
    int insize;                 // 0x20
    char *inbuf;                // 0x24 also the bitmap scan-line buffer (packres_40d578/40d6f0/40d980: line)
    char _unk28[4];
    char name[0x84];            // 0x2c
    int iter;                   // 0xb0 enumeration cursor (old: fb0)
    TSaveCasts *casts;          // 0xb4

    TPackedResources(void *owner, const char *name);   // 0x40dce0 (owner unused)
    ~TPackedResources();                               // 0x40dd90
    int fn_40d250();            // matched as a real member (packres_40d250.cpp)
};

ELF_CHECK_OFS(TPackedResources, hdr, 0x04);
ELF_CHECK_OFS(TPackedResources, count, 0x10);
ELF_CHECK_OFS(TPackedResources, outbuf, 0x1c);
ELF_CHECK_OFS(TPackedResources, inbuf, 0x24);
ELF_CHECK_OFS(TPackedResources, name, 0x2c);
ELF_CHECK_OFS(TPackedResources, iter, 0xb0);
ELF_CHECK_OFS(TPackedResources, casts, 0xb4);
ELF_CHECK_SIZE(TPackedResources, 0xb8);

// zlib 1.0 z_stream (0x38 bytes), as the TPackedStream units declare it.
// zlib_41e2cc.cpp has the real zlib typedefs; do not include both.
#ifndef ELF_NO_ZSTREAM
struct ZStream {
    char *next_in;              // 0x00
    unsigned avail_in;          // 0x04
    unsigned total_in;          // 0x08
    char *next_out;             // 0x0c
    unsigned avail_out;         // 0x10 (packres_40dee0: int)
    unsigned total_out;         // 0x14
    char *msg;                  // 0x18
    void *state;                // 0x1c
    void *(*zalloc)(void *, unsigned, unsigned);    // 0x20
    void (*zfree)(void *, void *);                  // 0x24
    void *opaque;               // 0x28
    int data_type;              // 0x2c
    unsigned adler;             // 0x30
    unsigned reserved;          // 0x34
};
ELF_CHECK_OFS(ZStream, zalloc, 0x20);
ELF_CHECK_SIZE(ZStream, 0x38);
#endif

// TPackedStream (0x50 bytes) is a Classes::TStream subclass: its real
// declaration needs <vcl/classes.hpp> (included first). Units without the
// VCL see its fields through TPackedStreamData.
struct TPackedStreamData {
    void *vmt;                  // 0x00 Delphi VMT
    char *out;                  // 0x04 inflate output buffer
    int outsize;                // 0x08
    int used;                   // 0x0c bytes of out already returned
    ZStream z;                  // 0x10 (next_out at 0x1c, avail_out at 0x20)
    int size;                   // 0x48
    int pos;                    // 0x4c
};
ELF_CHECK_OFS(TPackedStreamData, z, 0x10);
ELF_CHECK_OFS(TPackedStreamData, size, 0x48);
ELF_CHECK_SIZE(TPackedStreamData, 0x50);

#if defined(ClassesHPP)
class TPackedStream : public Classes::TStream
{
public:
    char *out;                  // 0x04
    int outsize;                // 0x08
    int used;                   // 0x0c
    ZStream z;                  // 0x10
    int size;                   // 0x48
    int pos;                    // 0x4c
    TPackedStream(char *in, int inlen, char *o, int olen, int sz);    // 0x40e0a8 (cdecl)
    // No user-declared dtor: the real one (0x40f104) is compiler-generated.
    virtual int __fastcall Read(void *Buffer, int Count);             // 0x40dee0
    virtual int __fastcall Write(const void *Buffer, int Count);
    virtual int __fastcall Seek(int Offset, Word Origin);
#ifndef __BORLANDC__
    // VMT slot 0 of the original is fn_40dfd0 (`outsize = v`). Declared only
    // natively: on bcc32 a new virtual would change the unit's vtable, and the
    // matched code never calls it. Body in packres_40e0a8.cpp.
    virtual void SetSize(int NewSize);
#endif
};
#endif

// Delphi TStream as seen by non-VCL units that only call Read through the
// VMT: st->vt[1](st, buf, n) compiles to mov ebx,[eax]; call [ebx+4].
struct TStreamVmt;
typedef int (__fastcall *TStreamReadFn)(TStreamVmt *s, void *buf, int n);   // old: ReadFn
struct TStreamVmt {
    TStreamReadFn *vt;          // [1] = Read
};

// PARMS resource: per-sprite default parameters (entries 0x58 bytes).
struct ParmEntry {
    char name[0x1c];            // 0x00
    int f1c, f20;               // 0x1c
    char f24, f25;              // 0x24
    char _unk26[0x58 - 0x26];
};

struct TSaveParms {
    char magic[0x28];           // 0x00 "NV us, you strange little monkey!"
    int count;                  // 0x28
    int hdrsize;                // 0x2c
    ParmEntry *entries;         // 0x30
    char *data;                 // 0x34
    TSaveParms();               // 0x40e37c
};
ELF_CHECK_OFS(TSaveParms, count, 0x28);
ELF_CHECK_OFS(TSaveParms, data, 0x34);

// CASTS resource: per-cast parameters (entries 0x74 bytes).
// packres_40d6f0 crops the bitmap to `keep` and stores both rects in the
// cast's Bitmap (bmp.r0c = rect, bmp.r1c = keep). Old: Clip (r1c, src).
struct CastEntry {
    char name[0x1c];            // 0x00 cast name (the old unit spelled it name[0x74])
    RECT rect;                  // 0x1c (old: r1c)
    RECT keep;                  // 0x2c area to keep (old: src)
    char _unk3c[0x74 - 0x3c];
};
ELF_CHECK_OFS(CastEntry, rect, 0x1c);
ELF_CHECK_OFS(CastEntry, keep, 0x2c);
ELF_CHECK_SIZE(CastEntry, 0x74);

struct TSaveCasts {
    char magic[0x20];           // 0x00 "The NStorm Cannon Rules!"
    int count;                  // 0x20
    int hdrsize;                // 0x24
    CastEntry *entries;         // 0x28
    char *data;                 // 0x2c
    TSaveCasts();               // 0x40e1b0
    ~TSaveCasts();              // 0x40e210
};
ELF_CHECK_OFS(TSaveCasts, count, 0x20);
ELF_CHECK_OFS(TSaveCasts, data, 0x2c);

// A bitmap file header with natural alignment, read 2 bytes early
// (packres_40d980).
struct BmpFileHdr {
    WORD type;
    DWORD size;
    WORD r1, r2;
    DWORD offBits;              // 0x0c
    BITMAPINFOHEADER bi;        // 0x10
};

#endif
