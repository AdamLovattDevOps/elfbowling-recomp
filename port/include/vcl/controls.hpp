// vcl/controls.hpp - Controls unit subset: TControl, TWinControl and the
// mouse / key event types.
//
// There is one native window (the shim's screen), owned by the main form.
// Controls have no non-client area: Width == ClientWidth.
#ifndef ControlsHPP
#define ControlsHPP

#include <vcl/classes.hpp>
#include <vcl/graphics.hpp>

namespace Controls {

using Classes::TShiftState;
using System::AnsiString;
using System::TObject;
using System::Word;

enum TMouseButton { mbLeft, mbRight, mbMiddle };

ELF_CLOSURE(void, TMouseEvent, (System::TObject *Sender, TMouseButton Button, Classes::TShiftState Shift, int X, int Y));
ELF_CLOSURE(void, TMouseMoveEvent, (System::TObject *Sender, Classes::TShiftState Shift, int X, int Y));
ELF_CLOSURE(void, TKeyEvent, (System::TObject *Sender, System::Word &Key, Classes::TShiftState Shift));
ELF_CLOSURE(void, TKeyPressEvent, (System::TObject *Sender, char &Key));

class TControl : public Classes::TComponent {
public:
    explicit TControl(Classes::TComponent *AOwner);
    ~TControl() override;

    // Delphi 3 setters / getters (called by address from the game: see
    // port/vcl/entry.cpp and include/elf/funcs.h).
    void SetLeft(int v) { SetBounds(v, FTop, FWidth, FHeight); }
    void SetTop(int v) { SetBounds(FLeft, v, FWidth, FHeight); }
    void SetWidth(int v) { SetBounds(FLeft, FTop, v, FHeight); }
    void SetHeight(int v) { SetBounds(FLeft, FTop, FWidth, v); }
    virtual void SetBounds(int ALeft, int ATop, int AWidth, int AHeight);
    int GetLeft() const { return FLeft; }
    int GetTop() const { return FTop; }
    int GetWidth() const { return FWidth; }
    int GetHeight() const { return FHeight; }
    int GetClientWidth() { return FWidth; }
    int GetClientHeight() { return FHeight; }
    void SetClientWidth(int v) { SetWidth(v); }
    void SetClientHeight(int v) { SetHeight(v); }
    Graphics::TColor GetColor() const { return FColor; }
    void SetColor(Graphics::TColor c);
    bool GetVisible() const { return FVisible; }
    virtual void SetVisible(bool v);
    AnsiString GetCaption() const { return FCaption; }
    virtual void SetCaption(const AnsiString &s) { FCaption = s; }
    bool GetEnabled() const { return FEnabled; }
    void SetEnabled(bool v) { FEnabled = v; }

    void Show() { SetVisible(true); }
    void Hide() { SetVisible(false); }
    void BringToFront();
    virtual void Invalidate();
    void Repaint();
    virtual void Update();
    void Refresh() { Repaint(); }

    VCL_PROPERTY(TControl, int, Left, &TControl::GetLeft, &TControl::SetLeft);
    VCL_PROPERTY(TControl, int, Top, &TControl::GetTop, &TControl::SetTop);
    VCL_PROPERTY(TControl, int, Width, &TControl::GetWidth, &TControl::SetWidth);
    VCL_PROPERTY(TControl, int, Height, &TControl::GetHeight, &TControl::SetHeight);
    VCL_PROPERTY(TControl, int, ClientWidth, &TControl::GetClientWidth, &TControl::SetClientWidth);
    VCL_PROPERTY(TControl, int, ClientHeight, &TControl::GetClientHeight, &TControl::SetClientHeight);
    VCL_PROPERTY(TControl, Graphics::TColor, Color, &TControl::GetColor, &TControl::SetColor);
    VCL_PROPERTY(TControl, bool, Visible, &TControl::GetVisible, &TControl::SetVisible);
    VCL_PROPERTY(TControl, AnsiString, Caption, &TControl::GetCaption, &TControl::SetCaption);
    VCL_PROPERTY(TControl, bool, Enabled, &TControl::GetEnabled, &TControl::SetEnabled);
    // The field names include/elf/stage.h gives TStageForm (FWidth / FHeight
    // at 0x38 / 0x3c), so engine_40a844 reads `e->form->width` unchanged.
    VCL_PROPERTY(TControl, int, width, &TControl::GetWidth, nullptr);
    VCL_PROPERTY(TControl, int, height, &TControl::GetHeight, nullptr);

    // Events (protected in VCL; public here, as the game assigns them from
    // TStage). Fields, as {read=FOnX, write=FOnX}.
    Classes::TNotifyEvent OnClick;
    Classes::TNotifyEvent OnDblClick;
    TMouseEvent OnMouseDown;
    TMouseMoveEvent OnMouseMove;
    TMouseEvent OnMouseUp;

    // Dispatch (called by the runtime with VCL semantics).
    virtual void MouseDown(TMouseButton Button, TShiftState Shift, int X, int Y);
    virtual void MouseMove(TShiftState Shift, int X, int Y);
    virtual void MouseUp(TMouseButton Button, TShiftState Shift, int X, int Y);
    virtual void Click();
    virtual void DblClick();

    bool DfmSetProperty(const char *name, const Vcl::TDfmValue &v) override;
    void *DfmEvent(const char *name, Vcl::TEventKind *kind) override;

protected:
    virtual void BoundsChanged() {}
    int FLeft, FTop, FWidth, FHeight;
    Graphics::TColor FColor;
    bool FVisible;
    bool FEnabled;
    AnsiString FCaption;
};

class TWinControl : public TControl {
public:
    explicit TWinControl(Classes::TComponent *AOwner) : TControl(AOwner) {}
    ~TWinControl() override {}

    virtual HWND GetHandle();
    bool HandleAllocated() { return GetHandle() != nullptr; }
    VCL_PROPERTY(TWinControl, HWND, Handle, &TWinControl::GetHandle, nullptr);

    TKeyEvent OnKeyDown;
    TKeyPressEvent OnKeyPress;
    TKeyEvent OnKeyUp;

    virtual void KeyDown(Word &Key, TShiftState Shift);
    virtual void KeyUp(Word &Key, TShiftState Shift);
    virtual void KeyPress(char &Key);

    void *DfmEvent(const char *name, Vcl::TEventKind *kind) override;
};

} // namespace Controls

#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Controls;
#endif

#endif
