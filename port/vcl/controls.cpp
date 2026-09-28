// Controls subset: TControl, TWinControl (bounds, colour, visibility, event
// dispatch, DFM properties).
#include <vcl/vcl.h>

#include <cstring>

namespace Controls {

TControl::TControl(Classes::TComponent *AOwner)
    : Classes::TComponent(AOwner), FLeft(0), FTop(0), FWidth(0), FHeight(0), FColor(Graphics::clBtnFace),
      FVisible(false), FEnabled(true)
{
}

TControl::~TControl() {}

void TControl::SetBounds(int ALeft, int ATop, int AWidth, int AHeight)
{
    if (ALeft == FLeft && ATop == FTop && AWidth == FWidth && AHeight == FHeight)
        return;
    FLeft = ALeft;
    FTop = ATop;
    FWidth = AWidth;
    FHeight = AHeight;
    BoundsChanged();
}

void TControl::SetColor(Graphics::TColor c)
{
    if (c != FColor) {
        FColor = c;
        Invalidate();
    }
}

void TControl::SetVisible(bool v) { FVisible = v; }
void TControl::BringToFront() {}
void TControl::Invalidate() {}
void TControl::Update() {}

void TControl::Repaint()
{
    Invalidate();
    Update();
}

void TControl::MouseDown(TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (OnMouseDown)
        OnMouseDown(this, Button, Shift, X, Y);
}

void TControl::MouseMove(TShiftState Shift, int X, int Y)
{
    if (OnMouseMove)
        OnMouseMove(this, Shift, X, Y);
}

void TControl::MouseUp(TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (OnMouseUp)
        OnMouseUp(this, Button, Shift, X, Y);
}

void TControl::Click()
{
    if (OnClick)
        OnClick(this);
}

void TControl::DblClick()
{
    if (OnDblClick)
        OnDblClick(this);
}

static bool int_of(const Vcl::TDfmValue &v, int *out)
{
    if (v.kind != Vcl::TDfmValue::Int)
        return false;
    *out = (int)v.i;
    return true;
}

bool TControl::DfmSetProperty(const char *name, const Vcl::TDfmValue &v)
{
    int i;
    if (!std::strcmp(name, "Left") && int_of(v, &i)) { SetLeft(i); return true; }
    if (!std::strcmp(name, "Top") && int_of(v, &i)) { SetTop(i); return true; }
    if (!std::strcmp(name, "Width") && int_of(v, &i)) { SetWidth(i); return true; }
    if (!std::strcmp(name, "Height") && int_of(v, &i)) { SetHeight(i); return true; }
    if (!std::strcmp(name, "ClientWidth") && int_of(v, &i)) { SetClientWidth(i); return true; }
    if (!std::strcmp(name, "ClientHeight") && int_of(v, &i)) { SetClientHeight(i); return true; }
    if (!std::strcmp(name, "Color")) {
        Graphics::TColor c;
        if (v.kind == Vcl::TDfmValue::Ident && Graphics::IdentToColor(v.s.c_str(), &c)) { SetColor(c); return true; }
        if (int_of(v, &i)) { SetColor(i); return true; }
        return false;
    }
    if (!std::strcmp(name, "Visible") && v.kind == Vcl::TDfmValue::Bool) { SetVisible(v.b); return true; }
    if (!std::strcmp(name, "Enabled") && v.kind == Vcl::TDfmValue::Bool) { SetEnabled(v.b); return true; }
    if ((!std::strcmp(name, "Caption") || !std::strcmp(name, "Text")) && v.kind == Vcl::TDfmValue::String) {
        SetCaption(v.s.c_str());
        return true;
    }
    // Accepted, no effect in the port: fonts (the game draws its own text
    // with GDI), hints, cursors, parent flags.
    if (!std::strncmp(name, "Font.", 5) || !std::strcmp(name, "ParentFont") || !std::strcmp(name, "ParentColor") ||
        !std::strcmp(name, "Hint") || !std::strcmp(name, "ShowHint") || !std::strcmp(name, "ParentShowHint") ||
        !std::strcmp(name, "Cursor") || !std::strcmp(name, "TabOrder") || !std::strcmp(name, "TabStop"))
        return true;
    return Classes::TComponent::DfmSetProperty(name, v);
}

void *TControl::DfmEvent(const char *name, Vcl::TEventKind *kind)
{
    struct { const char *n; Vcl::TEventKind k; void *slot; } t[] = {
        {"OnClick", Vcl::ekNotify, &OnClick},         {"OnDblClick", Vcl::ekNotify, &OnDblClick},
        {"OnMouseDown", Vcl::ekMouse, &OnMouseDown},  {"OnMouseMove", Vcl::ekMouseMove, &OnMouseMove},
        {"OnMouseUp", Vcl::ekMouse, &OnMouseUp},
    };
    for (auto &e : t)
        if (!std::strcmp(e.n, name)) {
            *kind = e.k;
            return e.slot;
        }
    return Classes::TComponent::DfmEvent(name, kind);
}

// ---- TWinControl -------------------------------------------------------------------

HWND TWinControl::GetHandle() { return nullptr; }

void TWinControl::KeyDown(Word &Key, TShiftState Shift)
{
    if (OnKeyDown)
        OnKeyDown(this, Key, Shift);
}

void TWinControl::KeyUp(Word &Key, TShiftState Shift)
{
    if (OnKeyUp)
        OnKeyUp(this, Key, Shift);
}

void TWinControl::KeyPress(char &Key)
{
    if (OnKeyPress)
        OnKeyPress(this, Key);
}

void *TWinControl::DfmEvent(const char *name, Vcl::TEventKind *kind)
{
    struct { const char *n; Vcl::TEventKind k; void *slot; } t[] = {
        {"OnKeyDown", Vcl::ekKey, &OnKeyDown},
        {"OnKeyUp", Vcl::ekKey, &OnKeyUp},
        {"OnKeyPress", Vcl::ekKeyPress, &OnKeyPress},
    };
    for (auto &e : t)
        if (!std::strcmp(e.n, name)) {
            *kind = e.k;
            return e.slot;
        }
    return TControl::DfmEvent(name, kind);
}

} // namespace Controls
