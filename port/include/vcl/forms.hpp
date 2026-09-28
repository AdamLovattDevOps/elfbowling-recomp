// vcl/forms.hpp - Forms unit subset: TCustomForm, TForm, TApplication,
// TScreen, and the native stand-ins for published methods and metaclasses.
//
// Runtime (port/vcl/forms.cpp): Application->Run() shows the main form (one
// SDL window through the shim, client size from the DFM) and runs an SDL
// event loop. It maps SDL key and mouse events to OnKeyDown / OnKeyUp /
// OnKeyPress and OnMouseDown / OnMouseMove / OnMouseUp with VCL semantics
// (virtual-key codes, TShiftState, client coordinates), calls OnPaint when
// the form is invalidated, runs TThread::Synchronize calls, and presents the
// shim's screen.
#ifndef FormsHPP
#define FormsHPP

#include <vcl/classes.hpp>
#include <vcl/controls.hpp>
#include <vcl/graphics.hpp>

#include <cstring>
#include <new>
#include <type_traits>

namespace Forms {

using System::AnsiString;
using System::TObject;

enum TFormBorderStyle { bsNone, bsSingle, bsSizeable, bsDialog, bsToolWindow, bsSizeToolWin };
typedef TFormBorderStyle TBorderStyle;
enum TPosition { poDesigned, poDefault, poDefaultPosOnly, poDefaultSizeOnly, poScreenCenter };
enum TCloseAction { caNone, caHide, caFree, caMinimize };
enum TFormStyle { fsNormal, fsMDIChild, fsMDIForm, fsStayOnTop };
enum TWindowState { wsNormal, wsMinimized, wsMaximized };

ELF_CLOSURE(void, TCloseEvent, (System::TObject *Sender, TCloseAction &Action));

class TCustomForm : public Controls::TWinControl {
public:
    // Reads the form's DFM (RCDATA resource named after the class, e.g.
    // TFORM1) when constructed through CreateForm / Vcl::CreateComponent.
    explicit TCustomForm(Classes::TComponent *AOwner);
    ~TCustomForm() override;

    void Close();
    void Show() { SetVisible(true); BringToFront(); }
    void Hide() { SetVisible(false); }
    void SetClientWidth(int v) { SetWidth(v); }
    void SetClientHeight(int v) { SetHeight(v); }
    HWND GetHandle() override;
    void Invalidate() override;
    void SetVisible(bool v) override;
    void SetCaption(const AnsiString &s) override;

    TFormBorderStyle GetBorderStyle() const { return FBorderStyle; }
    void SetBorderStyle(TFormBorderStyle v);
    TPosition GetPosition() const { return FPosition; }
    void SetPosition(TPosition v) { FPosition = v; }
    bool GetKeyPreview() const { return FKeyPreview; }
    void SetKeyPreview(bool v) { FKeyPreview = v; }

    VCL_PROPERTY(TCustomForm, TFormBorderStyle, BorderStyle, &TCustomForm::GetBorderStyle, &TCustomForm::SetBorderStyle);
    VCL_PROPERTY(TCustomForm, TPosition, Position, &TCustomForm::GetPosition, &TCustomForm::SetPosition);
    VCL_PROPERTY(TCustomForm, bool, KeyPreview, &TCustomForm::GetKeyPreview, &TCustomForm::SetKeyPreview);
    // ClientWidth/ClientHeight write through TCustomForm::SetClientWidth (VCL 3).
    VCL_PROPERTY(TCustomForm, int, ClientWidth, &TCustomForm::GetClientWidth, &TCustomForm::SetClientWidth);
    VCL_PROPERTY(TCustomForm, int, ClientHeight, &TCustomForm::GetClientHeight, &TCustomForm::SetClientHeight);

    Classes::TNotifyEvent OnPaint;
    Classes::TNotifyEvent OnCreate;
    Classes::TNotifyEvent OnDestroy;
    Classes::TNotifyEvent OnShow;
    Classes::TNotifyEvent OnHide;
    Classes::TNotifyEvent OnActivate;
    Classes::TNotifyEvent OnResize;
    TCloseEvent OnClose;

    // Runtime: erase with Color, then OnPaint. Clears the invalid flag.
    virtual void Paint();
    bool NeedsPaint() const { return FInvalid && FVisible; }

    bool DfmSetProperty(const char *name, const Vcl::TDfmValue &v) override;
    void *DfmEvent(const char *name, Vcl::TEventKind *kind) override;

protected:
    void BoundsChanged() override;
    TFormBorderStyle FBorderStyle;
    TPosition FPosition;
    bool FKeyPreview;
    bool FInvalid;
    bool FWindowOwner;      // this form owns the shim window
    bool FLoading;          // reading the DFM
};

class TForm : public TCustomForm {
public:
    explicit TForm(Classes::TComponent *AOwner) : TCustomForm(AOwner) {}
    ~TForm() override {}
};

class TScreen : public Classes::TComponent {
public:
    explicit TScreen(Classes::TComponent *AOwner) : Classes::TComponent(AOwner) {}
    // The "desktop" the game sees: $PORT_SCREEN_W x $PORT_SCREEN_H, default
    // 640 x 480, so the game's cover-the-desktop code keeps a 640x480 window.
    int GetWidth();
    int GetHeight();
    VCL_PROPERTY(TScreen, int, Width, &TScreen::GetWidth, nullptr);
    VCL_PROPERTY(TScreen, int, Height, &TScreen::GetHeight, nullptr);
};

class TApplication : public Classes::TComponent {
public:
    explicit TApplication(Classes::TComponent *AOwner);
    ~TApplication() override;

    void Initialize();
    void CreateForm(System::TMetaClass *InstanceClass, void *Reference);
    void Run();
    void Terminate() { FTerminated = true; }
    void ProcessMessages();
    void HandleMessage();
    int MessageBox(const char *Text, const char *Caption, int Flags = MB_OK);
    void ShowException(Sysutils::Exception *E);

    AnsiString GetTitle() const { return FTitle; }
    void SetTitle(const AnsiString &v);
    bool GetTerminated() const { return FTerminated; }
    TForm *GetMainForm() const { return FMainForm; }
    HWND GetHandle() const { return nullptr; }

    VCL_PROPERTY(TApplication, AnsiString, Title, &TApplication::GetTitle, &TApplication::SetTitle);
    VCL_PROPERTY(TApplication, bool, Terminated, &TApplication::GetTerminated, nullptr);
    VCL_PROPERTY(TApplication, TForm *, MainForm, &TApplication::GetMainForm, nullptr);
    bool ShowMainForm;

private:
    friend struct TApplicationAccess;
    AnsiString FTitle;
    bool FTerminated;
    TForm *FMainForm;
};

extern TApplication *Application;
extern TScreen *Screen;

} // namespace Forms

// ---- published methods and metaclasses (native stand-ins) -------------------
//
// Borland binds DFM event names ("OnPaint = FormPaint") to __published
// methods through RTTI. Natively a form class lists them:
//
//   class TForm1 : public TForm {
//   __published:
//       void __fastcall FormPaint(TObject *Sender);
//       ...
//       VCL_PUBLISHED(TForm1,
//           VCL_METHOD(FormPaint),                  // a member with a body
//           VCL_METHOD_AS(Form1KeyDown, fn_401610)) // or a free function taking the object first
//   };
//   VCL_REGISTER_CLASS(TForm1);                     // makes __classid(TForm1) work
namespace Vcl {

template <auto F> struct MethodEntry {
    const char *name;
};

namespace detail {
template <class Self, auto F, class C> bool BindAs(void *obj, void *out)
{
    if constexpr (Fits<C, F, Self>::value) {
        *static_cast<C *>(out) = C::template Make<F>(static_cast<Self *>(obj));
        return true;
    } else {
        return false;
    }
}
bool ReportMismatch(const char *method, int kind);
template <class Self, auto F> bool TryMethod(const MethodEntry<F> &e, void *obj, const char *name, int kind, void *out)
{
    if (std::strcmp(e.name, name) != 0)
        return false;
    bool ok = false;
    switch (kind) {
    case ekNotify: ok = BindAs<Self, F, Classes::TNotifyEvent>(obj, out); break;
    case ekKey: ok = BindAs<Self, F, Controls::TKeyEvent>(obj, out); break;
    case ekKeyPress: ok = BindAs<Self, F, Controls::TKeyPressEvent>(obj, out); break;
    case ekMouse: ok = BindAs<Self, F, Controls::TMouseEvent>(obj, out); break;
    case ekMouseMove: ok = BindAs<Self, F, Controls::TMouseMoveEvent>(obj, out); break;
    case ekClose: ok = BindAs<Self, F, Forms::TCloseEvent>(obj, out); break;
    default: break;
    }
    return ok || ReportMismatch(name, kind);
}
template <class Self, class... E> bool FindMethodIn(void *obj, const char *name, int kind, void *out, const E &...e)
{
    return (TryMethod<Self>(e, obj, name, kind, out) || ...);
}

template <class T, class = void> struct HasFindMethod : std::false_type {};
template <class T> struct HasFindMethod<T, std::void_t<decltype(&T::VclFindMethod)>> : std::true_type {};

template <class T> Classes::TComponent *Construct(void *mem, Classes::TComponent *owner) { return new (mem) T(owner); }
template <class T> void Destroy(void *obj) { delete static_cast<T *>(obj); }
} // namespace detail

template <class T> System::TMetaClass *MetaClassOf(const char *name)
{
    static System::TMetaClass m = {name, sizeof(T), &detail::Construct<T>, &detail::Destroy<T>, nullptr};
    if constexpr (detail::HasFindMethod<T>::value)
        m.FindMethod = &T::VclFindMethod;
    return &m;
}
template <class T> bool RegisterClassT(const char *name)
{
    RegisterClass(MetaClassOf<T>(name));
    return true;
}

// new T(owner) with DFM loading (what CreateForm does, without the global).
Classes::TComponent *CreateComponent(System::TMetaClass *cls, Classes::TComponent *owner, void **reference = nullptr);

// Construction context: set while a registered class is being constructed,
// so TCustomForm's constructor can find the DFM and the published methods.
struct TConstructing {
    System::TMetaClass *cls;
    void *obj;
};
TConstructing *Constructing();

// Read a binary DFM (TPF0) into a component: properties through
// DfmSetProperty, events through DfmEvent + the class's published methods.
// Returns false on a malformed stream. Used by TCustomForm; public for tests.
bool ReadDfm(Classes::TComponent *root, const void *data, std::size_t size, System::TMetaClass *cls, void *obj);
// The DFM for a class name from the user's exe (RCDATA/<NAME>), or NULL.
const void *FindDfm(const char *className, std::size_t *size);

// Run the application loop for one iteration: handle pending SDL events,
// Synchronize calls and paints; when `wait` and nothing happened, wait up to
// `waitMs` for an event. Returns true if anything was handled.
bool PumpOnce(bool wait, unsigned waitMs = 10);
// Dispatch one SDL_Event (passed as const SDL_Event *) as the loop would.
// For bots and tests; SDL_PushEvent cannot inject SDL_TEXTINPUT under
// sdl2-compat (it crashes).
void DispatchEvent(const void *sdlEvent);
// Map an SDL keysym (SDL_Keycode, SDL_Keymod) to a VCL virtual-key code, 0 if none.
unsigned short VirtualKeyFromSDL(int sdlKeycode, int sdlScancode);
Classes::TShiftState ShiftFromSDLMod(int sdlMod);
// Delete Application (and its forms), join threads. Idempotent.
void Shutdown();
} // namespace Vcl

#define VCL_PUBLISHED(Self, ...)                                                                  \
public:                                                                                           \
    static bool VclFindMethod(void *vcl_obj, const char *vcl_name, int vcl_kind, void *vcl_out)   \
    {                                                                                             \
        typedef Self VclSelf;                                                                     \
        return ::Vcl::detail::FindMethodIn<VclSelf>(vcl_obj, vcl_name, vcl_kind, vcl_out, __VA_ARGS__); \
    }
#define VCL_METHOD(Name) ::Vcl::MethodEntry<&VclSelf::Name>{#Name}
#define VCL_METHOD_AS(Name, Impl) ::Vcl::MethodEntry<Impl>{#Name}
#define VCL_REGISTER_CLASS(T) \
    static const bool vcl_registered_##T = ::Vcl::RegisterClassT<T>(#T)

#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Forms;
#endif

#endif
