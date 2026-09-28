// elf/bitmap.h - the engine's DIB wrapper (0x38 bytes).
//
// No RTTI name (it is never allocated on its own; it lives inside
// TGraphicCast and TStage). Helpers: fn_4029c8 create, fn_40290c free,
// fn_401fb4 header, fn_402b4c copy, fn_402bb8 scaled copy, fn_402ed4 blit.
#ifndef ELF_BITMAP_H
#define ELF_BITMAP_H

#include <elf/types.h>

// POD on purpose. ctors_4052dc.cpp declared `Bitmap() {}`, but its Cast
// ctors match without it too (checked); the ctors' `Bitmap *pb = &bmp`
// locals are what produce the two extra pointer stores.
struct Bitmap {
    int width;                  // 0x00
    int height;                 // 0x04
    int bpp;                    // 0x08 bits per pixel (8 or 24)
    RECT r0c;                   // 0x0c
    RECT r1c;                   // 0x1c
    HBITMAP handle;             // 0x2c DIB section
    char *bits;                 // 0x30 pixel data
    BITMAPINFOHEADER *info;     // 0x34
};

ELF_CHECK_OFS(Bitmap, r1c, 0x1c);
ELF_CHECK_OFS(Bitmap, handle, 0x2c);
ELF_CHECK_OFS(Bitmap, info, 0x34);
ELF_CHECK_SIZE(Bitmap, 0x38);

// Loaded bitmap file image: BITMAPINFOHEADER at +0x10 (fn_402a58).
struct DibFile {
    char _unk00[0x10];
    BITMAPINFOHEADER bih;       // 0x10
};

// 24-bit pixel.
struct RGB { unsigned char b, g, r; };

#endif
