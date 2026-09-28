// elf/types.h - basic value types shared by every unit.
//
// The matching build (bcc32 5.3 -Od) is sensitive to *how* a rectangle type
// is declared, not just its layout, so several layout-identical rect types
// exist on purpose. Pick the one the original code needs:
//
//   RECT            Win32 POD, LONG fields left/top/right/bottom. By-value
//                   arguments are pushed as four dwords.
//   RectPod         POD, int fields (l,t,r,b or left,top,right,bottom).
//                   Same codegen as RECT but with int fields (so tmin/tmax
//                   deduce against int), and the short names.
//   ERect           has an empty default ctor and a USER COPY CTOR. By-value
//                   arguments are copied with rep movsd, `ERect r = *p` copies
//                   through a stored temp pointer, returns build in place.
//                   (notes/round1_401.md round 2, round1_407.md "Quirks").
//                   This is the engine's own rect (TStage areas, sprite clip,
//                   cast bounds) and the "copy-ctor TRect" of the 0x401-0x407
//                   units: both field-name sets are available.
//   Windows::TRect  the BCB3 windows.hpp TRect: empty default ctor, no copy
//                   ctor, anonymous union. What Classes::Rect() returns. Also
//                   exported as plain `TRect`. Lower-case aliases (left..,
//                   topLeft/bottomRight) are provided in addition to the VCL
//                   names (Left.., TopLeft/BottomRight) when the real
//                   windows.hpp is not included.
//
// Where a field is declared as one of these and a unit needs another, cast
// at the use site: ELF_AS(ERect, s->clip). A reference cast of an lvalue does
// not change the generated code (verified on the pilot files).
//
// TPoint is tagPOINT (as in windows.hpp: `typedef tagPOINT TPoint`), so it
// mangles as 8tagPOINT in MATCH symbols.
//
// ERect is not called Rect because VCL units see Classes::Rect() (a
// function) through `using namespace Classes`.
//
// Include order: VCL headers (<vcl.h>, <vcl/classes.hpp>) BEFORE any elf/
// header, so that the real Windows::TRect / Classes::Rect are seen first.
//
// Native build (anything but bcc32): <vcl/vcl.h> from port/include comes
// first, so the port's Windows::TRect, Classes::Rect/Point and closures are
// the ones every unit sees (docs/VCL_PORT.md).
#ifndef ELF_TYPES_H
#define ELF_TYPES_H

#ifndef __BORLANDC__
#include <vcl/vcl.h>
#endif
#include <windows.h>

// Pointer-sized integer types for the 64-bit port. The BCB3 headers have
// neither <basetsd.h> nor <stdint.h>; on bcc32 both are 32-bit, so the
// casts generate the same code as the old (DWORD)/(int) ones.
#if defined(__BORLANDC__) && !defined(ELF_PTR_TYPES)
#define ELF_PTR_TYPES
typedef unsigned long DWORD_PTR;
typedef int intptr_t;
typedef unsigned int uintptr_t;
#endif

// Borland closures (__closure) and method binding, spelled so that the
// native build can replace them (docs/VCL_PORT.md "Closures"):
//   ELF_CLOSURE(void, TFooEvent, (TObject *Sender, int X));
//       Borland: typedef void __fastcall (__closure *TFooEvent)(TObject *Sender, int X)
//   ELF_METHOD(obj, Member, Impl)
//       Borland: (obj)->Member (the bound method); native: Vcl::Bind<Impl>(obj),
//       where Impl is the matched free function (fn_XXXXXX) or &Class::Member.
// port/include/vcl/closure.h defines the native forms.
#ifdef __BORLANDC__
#define ELF_CLOSURE(R, Name, Params) typedef R __fastcall (__closure *Name) Params
#define ELF_METHOD(Obj, Member, Impl) (Obj)->Member
#endif

#define ELF_AS(T, lv) (*(T *)&(lv))

#ifdef __BORLANDC__
#include <stddef.h>
// Compile-time layout check (bcc32 5.3 has no static_assert).
#define ELF_CHECK_OFS(T, f, ofs) typedef char chk_##T##_##f[(offsetof(T, f) == (ofs)) ? 1 : -1]
#define ELF_CHECK_SIZE(T, sz)    typedef char chk_size_##T[(sizeof(T) == (sz)) ? 1 : -1]
#else
#define ELF_CHECK_OFS(T, f, ofs)
#define ELF_CHECK_SIZE(T, sz)
#endif

#ifndef WindowsHPP
typedef tagPOINT TPoint;      // (windows.hpp has Windows::TPoint, the same type)
#endif

// POD int rectangle (see above).
struct RectPod {
    union {
        struct { int l, t, r, b; };
        struct { int left, top, right, bottom; };
    };
};

// Engine rectangle with a user copy constructor (see above).
struct ERect {
    union {
        struct { int l, t, r, b; };
        struct { int left, top, right, bottom; };
    };
    ERect() {}
    ERect(const ERect &o) { *this = o; }
    // Inline TRect -> RECT conversion (fn_4037e4 copies field by field into
    // a temp, then rep movsd into the RECT local).
    operator RECT() const { RECT q; q.left = left; q.top = top; q.right = right; q.bottom = bottom; return q; }
};

#ifndef WindowsHPP
namespace Windows {
struct TRect {
    TRect() {}
    TRect(RECT &r) { Left = r.left; Top = r.top; Right = r.right; Bottom = r.bottom; }
    operator RECT() { RECT r; r.left = Left; r.top = Top; r.right = Right; r.bottom = Bottom; return r; }
    union {
        struct { POINT TopLeft; POINT BottomRight; };
        struct { int Left; int Top; int Right; int Bottom; };
        struct { POINT topLeft; POINT bottomRight; };
        struct { int left; int top; int right; int bottom; };
    };
};
typedef tagPOINT TPoint;
}
#endif
using Windows::TRect;

#ifndef ClassesHPP
namespace Classes {
extern Windows::TRect __fastcall Rect(int ALeft, int ATop, int ARight, int ABottom);
extern tagPOINT __fastcall Point(int AX, int AY);
}
#endif
// Classes::Rect / Classes::Point under other return types. The call target is
// a fixup, so the C++ name does not matter to the match; the return type
// decides where the result is built.
extern RECT __fastcall Classes_Rect(int l, int t, int r, int b);
extern ERect __fastcall Classes_TRect(int l, int t, int r, int b);
extern TPoint __fastcall Classes_Point(int x, int y);

// TShiftState (a Delphi set of 1 byte).
struct ShiftState {
    unsigned char data;
    bool Contains(int el) const { return (data & (1 << el)) != 0; }
};

// min/max. bcc32 -Od emits different code for each spelling, so each
// spelling the original needs has its own name. All take const refs (the
// `lea reg,[a]` pattern). stdlib.h's min/max are the tmin_if/tmax_if forms.
template <class T> inline const T &tmin(const T &a, const T &b) { return a < b ? a : b; }
template <class T> inline const T &tmax(const T &a, const T &b) { return a > b ? a : b; }
template <class T> inline const T &tmax_lt(const T &a, const T &b) { return a < b ? b : a; }
template <class T> inline const T &tmin_if(const T &t1, const T &t2) { if (t1 < t2) return t1; else return t2; }
template <class T> inline const T &tmax_if(const T &t1, const T &t2) { if (t1 > t2) return t1; else return t2; }
// reversed compares: load the second operand first
template <class T> inline const T &tmin_rev(const T &t1, const T &t2) { if (t2 > t1) return t1; else return t2; }
template <class T> inline const T &tmax_rev(const T &t1, const T &t2) { if (t2 < t1) return t1; else return t2; }

#endif
