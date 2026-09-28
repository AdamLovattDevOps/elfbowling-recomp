// vcl/graphics.hpp - Graphics unit subset: TColor and the colour constants.
// TColor is an int (BCB: an enum spanning the int range), so the game's
// `form->SetColor(int)` calls compile unchanged.
#ifndef GraphicsHPP
#define GraphicsHPP

#include <vcl/classes.hpp>

namespace Graphics {

typedef int TColor;

// Delphi 3 values: RGB as 0x00BBGGRR; system colours as 0x80000000 | COLOR_x.
const TColor clBlack = 0x000000, clMaroon = 0x000080, clGreen = 0x008000, clOlive = 0x008080,
             clNavy = 0x800000, clPurple = 0x800080, clTeal = 0x808000, clGray = 0x808080,
             clSilver = 0xC0C0C0, clRed = 0x0000FF, clLime = 0x00FF00, clYellow = 0x00FFFF,
             clBlue = 0xFF0000, clFuchsia = 0xFF00FF, clAqua = 0xFFFF00, clLtGray = 0xC0C0C0,
             clDkGray = 0x808080, clWhite = 0xFFFFFF, clNone = 0x1FFFFFFF, clDefault = 0x20000000;
const TColor clScrollBar = (TColor)0x80000000, clBackground = (TColor)0x80000001,
             clActiveCaption = (TColor)0x80000002, clInactiveCaption = (TColor)0x80000003,
             clMenu = (TColor)0x80000004, clWindow = (TColor)0x80000005,
             clWindowFrame = (TColor)0x80000006, clMenuText = (TColor)0x80000007,
             clWindowText = (TColor)0x80000008, clCaptionText = (TColor)0x80000009,
             clActiveBorder = (TColor)0x8000000A, clInactiveBorder = (TColor)0x8000000B,
             clAppWorkSpace = (TColor)0x8000000C, clHighlight = (TColor)0x8000000D,
             clHighlightText = (TColor)0x8000000E, clBtnFace = (TColor)0x8000000F,
             clBtnShadow = (TColor)0x80000010, clGrayText = (TColor)0x80000011,
             clBtnText = (TColor)0x80000012, clInactiveCaptionText = (TColor)0x80000013,
             clBtnHighlight = (TColor)0x80000014;

// Resolve a TColor to 0x00BBGGRR (system colours use the Windows 98
// "Windows Standard" scheme). Unknown identifiers return false.
int ColorToRGB(TColor c);
bool IdentToColor(const char *ident, TColor *out);

} // namespace Graphics

#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Graphics;
#endif

#endif
