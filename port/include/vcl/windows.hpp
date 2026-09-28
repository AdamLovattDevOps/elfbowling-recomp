// vcl/windows.hpp - the Windows unit's TRect/TPoint as BCB3 WINDOWS.HPP
// declares them, plus the lower-case aliases that include/elf/types.h adds
// when it defines its own copy (it skips that copy when WindowsHPP is set).
#ifndef WindowsHPP
#define WindowsHPP

#include <vcl/sysmac.h>

namespace Windows {
typedef tagPOINT TPoint;
struct TRect {
    TRect() {}
    TRect(RECT &r) { Left = r.left; Top = r.top; Right = r.right; Bottom = r.bottom; }
    TRect(const TPoint &tl, const TPoint &br) { TopLeft = tl; BottomRight = br; }
    TRect(int l, int t, int r, int b) { Left = l; Top = t; Right = r; Bottom = b; }
    operator RECT() { RECT r; r.left = Left; r.top = Top; r.right = Right; r.bottom = Bottom; return r; }
    int Width() const { return Right - Left; }
    int Height() const { return Bottom - Top; }
    union {
        struct { POINT TopLeft; POINT BottomRight; };
        struct { int Left; int Top; int Right; int Bottom; };
        struct { POINT topLeft; POINT bottomRight; };
        struct { int left; int top; int right; int bottom; };
    };
};
}

#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Windows;
#endif

#endif
