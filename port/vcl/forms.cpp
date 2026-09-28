// Forms subset and the application loop.
//
// One native window: the shim's (port_init / port_present). The form that
// is shown first owns it; its client size is the window and screen size.
// Application->Run() shows the main form, then pumps until Terminate():
//   1. SDL events -> OnKeyDown / OnKeyUp / OnKeyPress, OnMouseDown /
//      OnMouseMove / OnMouseUp, OnClick / OnDblClick, close, expose;
//   2. queued TThread::Synchronize calls;
//   3. OnPaint for an invalidated form (after erasing it with Color);
//   4. port_present() when something drew, at most every 8 ms (a pending
//      frame is presented within 8 ms even when the loop goes idle).
#include <vector>
#include <cstdio>
#include <cstring>
#include <vcl/vcl.h>

#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <typeinfo>

namespace Forms {

TApplication *Application;
TScreen *Screen;

namespace {
struct Globals {
    Globals()
    {
        Application = new TApplication(nullptr);
        Screen = new TScreen(nullptr);
    }
} s_globals;

TCustomForm *s_windowForm;          // the form that owns the shim window
Uint32 s_wakeEvent = (Uint32)-1;
Uint32 s_lastPresent;
bool s_dirty;                       // drawn since the last present
unsigned s_buttons;                 // SDL_BUTTON_LMASK.. of the buttons held
int s_mods;                         // modifiers of the last key event (injected events do not update SDL's)
} // namespace

} // namespace Forms

namespace Vcl {

void EnsureEvents()
{
    if (!(SDL_WasInit(SDL_INIT_EVENTS) & SDL_INIT_EVENTS))
        SDL_InitSubSystem(SDL_INIT_EVENTS);
    if (Forms::s_wakeEvent == (Uint32)-1)
        Forms::s_wakeEvent = SDL_RegisterEvents(1);
}

void WakeMainThread()
{
    if (Forms::s_wakeEvent == (Uint32)-1 || !(SDL_WasInit(SDL_INIT_EVENTS) & SDL_INIT_EVENTS))
        return;
    SDL_Event e;
    std::memset(&e, 0, sizeof e);
    e.type = Forms::s_wakeEvent;
    SDL_PushEvent(&e);
}

TConstructing *Constructing()
{
    static thread_local TConstructing c = {nullptr, nullptr};
    return &c;
}

Classes::TComponent *CreateComponent(System::TMetaClass *cls, Classes::TComponent *owner, void **reference)
{
    void *mem = ::operator new(cls->InstanceSize);
    // VCL assigns the reference before the constructor runs (the game's
    // TForm1 constructor already reads Form1, as g_4601c0).
    if (reference)
        *reference = mem;
    TConstructing *ctx = Constructing();
    TConstructing saved = *ctx;
    ctx->cls = cls;
    ctx->obj = mem;
    Classes::TComponent *c;
    try {
        c = cls->Construct(mem, owner);
    } catch (...) {
        *ctx = saved;
        if (reference)
            *reference = nullptr;
        ::operator delete(mem);
        throw;
    }
    *ctx = saved;
    if ((void *)c != mem)
        std::fprintf(stderr, "[vcl] %s: TComponent base is not at offset 0\n", cls->ClassName);
    if (Forms::TCustomForm *f = dynamic_cast<Forms::TCustomForm *>(c))
        if (f->OnCreate)
            f->OnCreate(f);
    return c;
}

// ---- keys ----------------------------------------------------------------------------

unsigned short VirtualKeyFromSDL(int sym, int scancode)
{
    if (sym >= 'a' && sym <= 'z')
        return (unsigned short)(sym - 'a' + 'A');
    if (sym >= '0' && sym <= '9')
        return (unsigned short)sym;
    if (sym >= SDLK_F1 && sym <= SDLK_F12)
        return (unsigned short)(VK_F1 + (sym - SDLK_F1));
    if (sym >= SDLK_F13 && sym <= SDLK_F24)
        return (unsigned short)(VK_F1 + 12 + (sym - SDLK_F13));
    switch (sym) {
    case SDLK_BACKSPACE: return VK_BACK;
    case SDLK_TAB: return VK_TAB;
    case SDLK_CLEAR: return VK_CLEAR;
    case SDLK_RETURN: case SDLK_RETURN2: case SDLK_KP_ENTER: return VK_RETURN;
    case SDLK_LSHIFT: case SDLK_RSHIFT: return VK_SHIFT;
    case SDLK_LCTRL: case SDLK_RCTRL: return VK_CONTROL;
    case SDLK_LALT: case SDLK_RALT: return VK_MENU;
    case SDLK_PAUSE: return VK_PAUSE;
    case SDLK_CAPSLOCK: return VK_CAPITAL;
    case SDLK_ESCAPE: return VK_ESCAPE;
    case SDLK_SPACE: return VK_SPACE;
    case SDLK_PAGEUP: return VK_PRIOR;
    case SDLK_PAGEDOWN: return VK_NEXT;
    case SDLK_END: return VK_END;
    case SDLK_HOME: return VK_HOME;
    case SDLK_LEFT: return VK_LEFT;
    case SDLK_UP: return VK_UP;
    case SDLK_RIGHT: return VK_RIGHT;
    case SDLK_DOWN: return VK_DOWN;
    case SDLK_PRINTSCREEN: return VK_SNAPSHOT;
    case SDLK_INSERT: return VK_INSERT;
    case SDLK_DELETE: return VK_DELETE;
    case SDLK_HELP: return VK_HELP;
    case SDLK_LGUI: return VK_LWIN;
    case SDLK_RGUI: return VK_RWIN;
    case SDLK_APPLICATION: return VK_APPS;
    case SDLK_KP_0: return VK_NUMPAD0;
    case SDLK_KP_1: case SDLK_KP_2: case SDLK_KP_3: case SDLK_KP_4: case SDLK_KP_5:
    case SDLK_KP_6: case SDLK_KP_7: case SDLK_KP_8: case SDLK_KP_9:
        return (unsigned short)(VK_NUMPAD0 + 1 + (sym - SDLK_KP_1));
    case SDLK_KP_MULTIPLY: return VK_MULTIPLY;
    case SDLK_KP_PLUS: return VK_ADD;
    case SDLK_KP_MINUS: return VK_SUBTRACT;
    case SDLK_KP_PERIOD: return VK_DECIMAL;
    case SDLK_KP_DIVIDE: return VK_DIVIDE;
    case SDLK_NUMLOCKCLEAR: return VK_NUMLOCK;
    case SDLK_SCROLLLOCK: return VK_SCROLL;
    case SDLK_SEMICOLON: return VK_OEM_1;
    case SDLK_EQUALS: case SDLK_PLUS: return VK_OEM_PLUS;
    case SDLK_COMMA: return VK_OEM_COMMA;
    case SDLK_MINUS: return VK_OEM_MINUS;
    case SDLK_PERIOD: return VK_OEM_PERIOD;
    case SDLK_SLASH: return VK_OEM_2;
    case SDLK_BACKQUOTE: return VK_OEM_3;
    case SDLK_LEFTBRACKET: return VK_OEM_4;
    case SDLK_BACKSLASH: return VK_OEM_5;
    case SDLK_RIGHTBRACKET: return VK_OEM_6;
    case SDLK_QUOTE: return VK_OEM_7;
    default: break;
    }
    // Non-US layouts: fall back to the US position of the key.
    if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z)
        return (unsigned short)('A' + (scancode - SDL_SCANCODE_A));
    return 0;
}

Classes::TShiftState ShiftFromSDLMod(int mod)
{
    Classes::TShiftState s;
    if (mod & KMOD_SHIFT)
        s << Classes::ssShift;
    if (mod & KMOD_ALT)
        s << Classes::ssAlt;
    if (mod & KMOD_CTRL)
        s << Classes::ssCtrl;
    return s;
}

static Classes::TShiftState mouseShift(unsigned buttons, int mods)
{
    Classes::TShiftState s = ShiftFromSDLMod(mods);
    if (buttons & SDL_BUTTON_LMASK)
        s << Classes::ssLeft;
    if (buttons & SDL_BUTTON_RMASK)
        s << Classes::ssRight;
    if (buttons & SDL_BUTTON_MMASK)
        s << Classes::ssMiddle;
    return s;
}

static Forms::TCustomForm *inputForm()
{
    if (Forms::s_windowForm)
        return Forms::s_windowForm;
    return Forms::Application ? Forms::Application->GetMainForm() : nullptr;
}

static void keyPress(Forms::TCustomForm *f, char c)
{
    char k = c;
    f->KeyPress(k);
}

static void dispatch(const SDL_Event &e)
{
    Forms::TCustomForm *f = inputForm();
    switch (e.type) {
    case SDL_QUIT:
        if (f)
            f->Close();
        else if (Forms::Application)
            Forms::Application->Terminate();
        break;
    case SDL_WINDOWEVENT:
        switch (e.window.event) {
        case SDL_WINDOWEVENT_EXPOSED:
        case SDL_WINDOWEVENT_SHOWN:
        case SDL_WINDOWEVENT_RESTORED:
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            if (f)
                f->Invalidate();
            break;
        default:
            break;
        }
        break;
    case SDL_KEYDOWN:
    case SDL_KEYUP: {
        Forms::s_mods = e.key.keysym.mod;
        if (!f)
            break;
        System::Word key = VirtualKeyFromSDL(e.key.keysym.sym, e.key.keysym.scancode);
        if (!key)
            break;
        Classes::TShiftState shift = ShiftFromSDLMod(e.key.keysym.mod);
        if (e.type == SDL_KEYDOWN) {
            f->KeyDown(key, shift);
            // WM_CHAR for the control keys (text arrives as SDL_TEXTINPUT).
            switch (e.key.keysym.sym) {
            case SDLK_RETURN: case SDLK_KP_ENTER: keyPress(f, '\r'); break;
            case SDLK_ESCAPE: keyPress(f, 27); break;
            case SDLK_BACKSPACE: keyPress(f, 8); break;
            case SDLK_TAB: keyPress(f, 9); break;
            default:
                if ((e.key.keysym.mod & KMOD_CTRL) && e.key.keysym.sym >= 'a' && e.key.keysym.sym <= 'z')
                    keyPress(f, (char)(e.key.keysym.sym - 'a' + 1));
                break;
            }
        } else {
            f->KeyUp(key, shift);
        }
        break;
    }
    case SDL_TEXTINPUT:
        if (f) {
            // UTF-8 -> Latin-1 (the game's strings are Windows-1252)
            const unsigned char *s = (const unsigned char *)e.text.text;
            while (*s) {
                unsigned c = *s++;
                if (c >= 0xC0 && c < 0xE0 && (*s & 0xC0) == 0x80)
                    c = ((c & 0x1F) << 6) | (*s++ & 0x3F);
                else if (c >= 0x80)
                    continue;
                if (c < 256)
                    keyPress(f, (char)c);
            }
        }
        break;
    case SDL_MOUSEMOTION:
        if (f)
            f->MouseMove(mouseShift(Forms::s_buttons, Forms::s_mods | SDL_GetModState()), e.motion.x, e.motion.y);
        break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
        Controls::TMouseButton b;
        switch (e.button.button) {
        case SDL_BUTTON_LEFT: b = Controls::mbLeft; break;
        case SDL_BUTTON_RIGHT: b = Controls::mbRight; break;
        case SDL_BUTTON_MIDDLE: b = Controls::mbMiddle; break;
        default: return;
        }
        unsigned mask = SDL_BUTTON(e.button.button);
        int mods = Forms::s_mods | SDL_GetModState();
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            Forms::s_buttons |= mask;      // Windows sets MK_xBUTTON for the button going down
            if (!f)
                break;
            Classes::TShiftState shift = mouseShift(Forms::s_buttons, mods);
            if (e.button.clicks >= 2) {
                shift << Classes::ssDouble;
                if (b == Controls::mbLeft)
                    f->DblClick();
            }
            f->MouseDown(b, shift, e.button.x, e.button.y);
        } else {
            Forms::s_buttons &= ~mask;     // ... and clears it for the button going up
            if (!f)
                break;
            f->MouseUp(b, mouseShift(Forms::s_buttons, mods), e.button.x, e.button.y);
            if (b == Controls::mbLeft && e.button.x >= 0 && e.button.y >= 0 && e.button.x < f->GetClientWidth() &&
                e.button.y < f->GetClientHeight())
                f->Click();
        }
        break;
    }
    default:
        break;
    }
}

void DispatchEvent(const void *sdlEvent) { dispatch(*static_cast<const SDL_Event *>(sdlEvent)); }

static const Uint32 kPresentMs = 8;     // at most ~120 presents a second

static void present(bool force)
{
    Uint32 now = SDL_GetTicks();
    if (Forms::s_dirty && (force || now - Forms::s_lastPresent >= kPresentMs)) {
        port_present();
        Forms::s_lastPresent = now;
        Forms::s_dirty = false;
    }
}

// $PORT_INPUT: scripted input for headless runs, "ms:click:x,y;ms:key:sym;..." where ms counts
// from the first pump, `click` is a left press+release at client (x, y) and `key` a press+release
// of an SDL keycode (13 = Return, 27 = Esc, 32 = space, or a lower-case letter's code).
static bool scriptStep()
{
    struct Ev { Uint32 at; char kind; int a, b; };
    static std::vector<Ev> evs;
    static size_t next;
    static Uint32 t0;
    static bool init;
    if (!init) {
        init = true;
        t0 = SDL_GetTicks();
        if (const char *p = getenv("PORT_INPUT")) {
            while (*p) {
                Ev ev{};
                char kind[16] = {0};
                int n = 0;
                if (sscanf(p, "%u:%15[a-z]:%d,%d%n", &ev.at, kind, &ev.a, &ev.b, &n) >= 3 && n == 0)
                    sscanf(p, "%u:%15[a-z]:%d%n", &ev.at, kind, &ev.a, &n);
                if (n == 0)
                    break;
                ev.kind = kind[0];
                evs.push_back(ev);
                p += n;
                while (*p == ';' || *p == ' ')
                    p++;
            }
            port_log("PORT_INPUT: %zu scripted events", evs.size());
        }
    }
    bool did = false;
    while (next < evs.size() && SDL_GetTicks() - t0 >= evs[next].at) {
        const Ev &ev = evs[next++];
        SDL_Event e;
        memset(&e, 0, sizeof e);
        if (ev.kind == 'c') {
            e.type = SDL_MOUSEMOTION;
            e.motion.x = ev.a;
            e.motion.y = ev.b;
            dispatch(e);
            memset(&e, 0, sizeof e);
            e.type = SDL_MOUSEBUTTONDOWN;
            e.button.button = SDL_BUTTON_LEFT;
            e.button.clicks = 1;
            e.button.x = ev.a;
            e.button.y = ev.b;
            dispatch(e);
            e.type = SDL_MOUSEBUTTONUP;
            dispatch(e);
        } else if (ev.kind == 'k') {
            e.type = SDL_KEYDOWN;
            e.key.keysym.sym = ev.a;
            e.key.keysym.scancode = SDL_GetScancodeFromKey(ev.a);
            dispatch(e);
            e.type = SDL_KEYUP;
            dispatch(e);
        }
        port_log("PORT_INPUT: t=%u %c %d,%d", ev.at, ev.kind, ev.a, ev.b);
        did = true;
    }
    return did;
}

bool PumpOnce(bool wait, unsigned waitMs)
{
    EnsureEvents();
    bool did = scriptStep();
    SDL_Event e;
    // Bounded, so a flood of motion events cannot starve Synchronize calls.
    for (int n = 0; n < 256 && SDL_PollEvent(&e); n++) {
        dispatch(e);
        did = true;
    }
    if (CheckSynchronize() > 0) {
        did = true;
        Forms::s_dirty = true;      // the game draws from its Synchronize'd Update
    }
    Forms::TCustomForm *f = inputForm();
    bool painted = false;
    if (f && f->NeedsPaint()) {
        f->Paint();
        painted = did = true;
        Forms::s_dirty = true;
    }
    present(painted);
    if (!did && wait) {
        // Idle: wait for an event (a Synchronize wakes us too), but not past
        // the moment a pending frame is due.
        unsigned ms = waitMs;
        if (Forms::s_dirty) {
            Uint32 since = SDL_GetTicks() - Forms::s_lastPresent;
            ms = since >= kPresentMs ? 0 : (kPresentMs - since < ms ? kPresentMs - since : ms);
        }
        // With a running TThread (the game's frame pump), wait on the
        // Synchronize queue in 1 ms slices and poll SDL in between; otherwise
        // sleep in SDL until an event arrives.
        if (WaitForSynchronize(ms < 1 ? 0 : 1)) {
            did = true;
        } else if (SDL_WaitEventTimeout(&e, (int)ms)) {
            dispatch(e);
            did = true;
        }
        present(false);
    } else if (!did) {
        present(true);
    }
    return did;
}

#ifdef __EMSCRIPTEN__
// ---- web build: one browser frame --------------------------------------------------------------
// The browser main thread must return to the event loop every frame, so Application->Run() cannot
// loop. It registers WebFrame with emscripten_set_main_loop (requestAnimationFrame) and returns;
// WinMain then returns and main() leaves the runtime running (port/vcl/main.cpp). No ASYNCIFY and
// no pthreads: the game's TThread is cooperative (thread.cpp), one slice (= one Update) per frame.
int RunThreadSlices();      // thread.cpp

static void WebFrame()
{
    EnsureEvents();
    scriptStep();
    SDL_Event e;
    for (int n = 0; n < 256 && SDL_PollEvent(&e); n++)
        dispatch(e);
    CheckSynchronize();                 // starts threads created with CreateSuspended = false
    if (RunThreadSlices() > 0)
        Forms::s_dirty = true;          // the game draws from its Update
    Forms::TCustomForm *f = inputForm();
    if (f && f->NeedsPaint()) {
        f->Paint();
        Forms::s_dirty = true;
    }
    present(true);                      // at most once per animation frame
    if (!Forms::Application || Forms::Application->GetTerminated()) {
        emscripten_cancel_main_loop();
        ShutdownThreads(0);
        // the page shows a "play again" screen (port/web/loader.js)
        EM_ASM({ if (Module['onGameExit']) Module['onGameExit'](); });
    }
}

void WebRun() { emscripten_set_main_loop(WebFrame, 0, 0); }
#endif

void Shutdown()
{
    ShutdownThreads(1000);
    Forms::TApplication *app = Forms::Application;
    Forms::Application = nullptr;
    delete app;
    delete Forms::Screen;
    Forms::Screen = nullptr;
    Forms::s_windowForm = nullptr;
}

} // namespace Vcl

namespace Forms {

struct TApplicationAccess {
    static void ClearMainForm(TApplication *a) { a->FMainForm = nullptr; }
};

// ---- TCustomForm ------------------------------------------------------------------------

TCustomForm::TCustomForm(Classes::TComponent *AOwner)
    : Controls::TWinControl(AOwner), FBorderStyle(bsSizeable), FPosition(poDesigned), FKeyPreview(false),
      FInvalid(true), FWindowOwner(false), FLoading(false)
{
    FLeft = 0;
    FTop = 0;
    FWidth = 640;
    FHeight = 480;
    Vcl::TConstructing *c = Vcl::Constructing();
    if (c->cls && c->obj == static_cast<void *>(this)) {
        std::size_t n = 0;
        const void *dfm = Vcl::FindDfm(c->cls->ClassName, &n);
        if (dfm) {
            // A Visible = True in the DFM shows the form after loading, as
            // TCustomForm.Loaded / Create do.
            FLoading = true;
            Vcl::ReadDfm(this, dfm, n, c->cls, c->obj);
            FLoading = false;
            bool wantVisible = FVisible;
            FVisible = false;
            if (wantVisible)
                SetVisible(true);
        } else {
            std::fprintf(stderr, "[vcl] %s: no DFM resource (RCDATA/%s); using defaults\n", c->cls->ClassName,
                         c->cls->ClassName);
        }
    }
}

TCustomForm::~TCustomForm()
{
    if (OnDestroy)
        OnDestroy(this);
    if (s_windowForm == this)
        s_windowForm = nullptr;
    if (Application && Application->GetMainForm() == this)
        TApplicationAccess::ClearMainForm(Application);
}

HWND TCustomForm::GetHandle() { return port_main_hwnd(); }

void TCustomForm::Invalidate() { FInvalid = true; }

static void apply_window(TCustomForm *f)
{
    SDL_Window *w = static_cast<SDL_Window *>(port_sdl_window());
    if (!w)
        return;
    SDL_SetWindowBordered(w, f->GetBorderStyle() == bsNone ? SDL_FALSE : SDL_TRUE);
    AnsiString title = Application ? Application->GetTitle() : AnsiString();
    SDL_SetWindowTitle(w, title.IsEmpty() ? f->GetCaption().c_str() : title.c_str());
    // Left/Top are not applied: the "desktop" the game sees is TScreen's
    // virtual 640x480 (the game moves the form to 0,0 of it). The window
    // stays centred on the real display.
    SDL_SetWindowPosition(w, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void TCustomForm::SetVisible(bool v)
{
    if (FLoading) {         // DFM Visible = True: shown after loading
        FVisible = v;
        return;
    }
    if (v == FVisible)
        return;
    FVisible = v;
    if (v) {
        if (!s_windowForm || s_windowForm == this) {
            AnsiString title = Application ? Application->GetTitle() : AnsiString();
            port_init(FWidth, FHeight, title.IsEmpty() ? FCaption.c_str() : title.c_str());
            s_windowForm = this;
            FWindowOwner = true;
            apply_window(this);
            ShowWindow(port_main_hwnd(), SW_SHOW);
        }
        Vcl::EnsureEvents();
        if (OnShow)
            OnShow(this);
        FInvalid = true;
    } else {
        if (OnHide)
            OnHide(this);
        if (FWindowOwner)
            ShowWindow(port_main_hwnd(), SW_HIDE);
    }
}

void TCustomForm::SetCaption(const AnsiString &s)
{
    FCaption = s;
    if (FWindowOwner)
        apply_window(this);
}

void TCustomForm::SetBorderStyle(TFormBorderStyle v)
{
    FBorderStyle = v;
    if (FWindowOwner)
        apply_window(this);
}

void TCustomForm::BoundsChanged()
{
    if (FWindowOwner && FVisible && FWidth > 0 && FHeight > 0) {
        AnsiString title = Application ? Application->GetTitle() : AnsiString();
        port_init(FWidth, FHeight, title.IsEmpty() ? FCaption.c_str() : title.c_str());
        apply_window(this);
    }
    FInvalid = true;
    if (OnResize)
        OnResize(this);
}

void TCustomForm::Close()
{
    TCloseAction action = caHide;
    if (OnClose)
        OnClose(this, action);
    if (action == caNone)
        return;
    if (Application && Application->GetMainForm() == this) {
        Application->Terminate();
        return;
    }
    if (action == caHide || action == caMinimize)
        Hide();
    // caFree: VCL releases the form after the current message; forms here
    // are owned by Application and freed at shutdown.
    else if (action == caFree)
        Hide();
}

void TCustomForm::Paint()
{
    FInvalid = false;
    // WM_ERASEBKGND: fill with Color (stock brushes only: the shim has no
    // CreateSolidBrush). Then WM_PAINT -> OnPaint.
    int rgb = Graphics::ColorToRGB(FColor);
    int stock = -1;
    switch (rgb) {
    case 0xFFFFFF: stock = WHITE_BRUSH; break;
    case 0xC0C0C0: stock = LTGRAY_BRUSH; break;
    case 0x808080: stock = GRAY_BRUSH; break;
    case 0x404040: stock = DKGRAY_BRUSH; break;
    case 0x000000: stock = BLACK_BRUSH; break;
    default: break;
    }
    if (stock >= 0 && FWindowOwner) {
        HDC dc = GetDC(port_main_hwnd());
        RECT r = {0, 0, FWidth, FHeight};
        FillRect(dc, &r, (HBRUSH)GetStockObject(stock));
        ReleaseDC(port_main_hwnd(), dc);
    }
    if (OnPaint)
        OnPaint(this);
}

bool TCustomForm::DfmSetProperty(const char *name, const Vcl::TDfmValue &v)
{
    using K = Vcl::TDfmValue;
    if (!std::strcmp(name, "BorderStyle") && v.kind == K::Ident) {
        static const char *const names[] = {"bsNone", "bsSingle", "bsSizeable", "bsDialog", "bsToolWindow", "bsSizeToolWin"};
        for (int i = 0; i < 6; i++)
            if (v.s == names[i]) {
                SetBorderStyle((TFormBorderStyle)i);
                return true;
            }
        return false;
    }
    if (!std::strcmp(name, "Position") && v.kind == K::Ident) {
        static const char *const names[] = {"poDesigned", "poDefault", "poDefaultPosOnly", "poDefaultSizeOnly", "poScreenCenter"};
        for (int i = 0; i < 5; i++)
            if (v.s == names[i]) {
                SetPosition((TPosition)i);
                return true;
            }
        return false;
    }
    if (!std::strcmp(name, "KeyPreview") && v.kind == K::Bool) {
        SetKeyPreview(v.b);
        return true;
    }
    // Accepted, no effect in the port.
    if (!std::strcmp(name, "PixelsPerInch") || !std::strcmp(name, "TextHeight") || !std::strcmp(name, "Scaled") ||
        !std::strcmp(name, "OldCreateOrder") || !std::strcmp(name, "WindowState") || !std::strcmp(name, "FormStyle") ||
        !std::strcmp(name, "Icon.Data") || !std::strcmp(name, "BorderIcons") || !std::strcmp(name, "AutoScroll"))
        return true;
    return Controls::TWinControl::DfmSetProperty(name, v);
}

void *TCustomForm::DfmEvent(const char *name, Vcl::TEventKind *kind)
{
    struct { const char *n; Vcl::TEventKind k; void *slot; } t[] = {
        {"OnPaint", Vcl::ekNotify, &OnPaint},     {"OnCreate", Vcl::ekNotify, &OnCreate},
        {"OnDestroy", Vcl::ekNotify, &OnDestroy}, {"OnShow", Vcl::ekNotify, &OnShow},
        {"OnHide", Vcl::ekNotify, &OnHide},       {"OnActivate", Vcl::ekNotify, &OnActivate},
        {"OnResize", Vcl::ekNotify, &OnResize},   {"OnClose", Vcl::ekClose, &OnClose},
    };
    for (auto &e : t)
        if (!std::strcmp(e.n, name)) {
            *kind = e.k;
            return e.slot;
        }
    return Controls::TWinControl::DfmEvent(name, kind);
}

// ---- TScreen ---------------------------------------------------------------------------------

static int env_int(const char *name, int def)
{
    const char *s = std::getenv(name);
    int v = s ? std::atoi(s) : 0;
    return v > 0 ? v : def;
}

int TScreen::GetWidth() { return env_int("PORT_SCREEN_W", 640); }
int TScreen::GetHeight() { return env_int("PORT_SCREEN_H", 480); }

// ---- TApplication ------------------------------------------------------------------------------

TApplication::TApplication(Classes::TComponent *AOwner)
    : Classes::TComponent(AOwner), ShowMainForm(true), FTerminated(false), FMainForm(nullptr)
{
}

TApplication::~TApplication()
{
    // Forms are owned components: TComponent's destructor frees them.
}

void TApplication::Initialize() { Vcl::EnsureEvents(); }

void TApplication::SetTitle(const AnsiString &v)
{
    FTitle = v;
    if (s_windowForm)
        apply_window(s_windowForm);
}

void TApplication::CreateForm(System::TMetaClass *InstanceClass, void *Reference)
{
    Classes::TComponent *c = Vcl::CreateComponent(InstanceClass, this, static_cast<void **>(Reference));
    if (!FMainForm)
        if (TForm *f = dynamic_cast<TForm *>(c))
            FMainForm = f;
}

void TApplication::Run()
{
    Vcl::EnsureEvents();
    if (FMainForm && ShowMainForm)
        FMainForm->Show();
#ifdef __EMSCRIPTEN__
    Vcl::WebRun();          // returns at once; the browser drives the loop
    return;
#endif
    while (!FTerminated)
        Vcl::PumpOnce(true);
}

void TApplication::ProcessMessages() { Vcl::PumpOnce(false); }
void TApplication::HandleMessage() { Vcl::PumpOnce(true); }

int TApplication::MessageBox(const char *Text, const char *Caption, int Flags)
{
    const char *drv = SDL_GetCurrentVideoDriver();
    bool headless = !drv || !std::strcmp(drv, "dummy") || !std::strcmp(drv, "offscreen") || std::getenv("PORT_NO_MSGBOX");
    std::fprintf(stderr, "[vcl] MessageBox \"%s\": %s\n", Caption ? Caption : "", Text ? Text : "");
    if (!headless) {
        Uint32 icon = (Flags & 0xF0) == MB_ICONHAND ? SDL_MESSAGEBOX_ERROR
                      : (Flags & 0xF0) == MB_ICONEXCLAMATION ? SDL_MESSAGEBOX_WARNING
                                                            : SDL_MESSAGEBOX_INFORMATION;
        SDL_ShowSimpleMessageBox(icon, Caption ? Caption : "", Text ? Text : "",
                                 static_cast<SDL_Window *>(port_sdl_window()));
    }
    return (Flags & 0x0F) == MB_YESNO ? IDYES : IDOK;
}

void TApplication::ShowException(Sysutils::Exception *E)
{
    AnsiString msg = E ? E->Message : AnsiString("Unknown exception");
    MessageBox(msg.c_str(), FTitle.c_str(), MB_OK | MB_ICONSTOP);
}

} // namespace Forms
