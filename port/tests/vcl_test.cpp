// Headless test of the VCL subset (port/include/vcl, port/vcl).
//
//   vcl_test "path/to/Elf Bowling.exe"
// Run with SDL_VIDEODRIVER=dummy. It checks:
//   - closures bound to members and to game-style free functions, properties,
//     streams (layout and vtable slot the game relies on), TResourceStream;
//   - the real form resource (RCDATA/TFORM1): properties and event bindings;
//   - a synthetic DFM with the mouse events;
//   - Application->Run(): injected SDL key / mouse / text events reach the
//     form's closures with VCL values; OnPaint on invalidate; a TThread that
//     loops `while (!Terminated) Synchronize(Update)` like the game's TMyThread,
//     with Update running on the main thread;
//   - the extern "C" by-address entry points of include/elf/funcs.h;
//   - the Borland rand() sequence (bcb_rtl.h).
#include <bcb_rtl.h>            // the native game build force-includes it
#include <vcl.h>
#include <vcl/elfcompat.h>
#include <vcl/scktcomp.hpp>

#include <SDL.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int g_fail, g_checks;
#define CHECK(c)                                                                  \
    do {                                                                          \
        g_checks++;                                                               \
        if (!(c)) {                                                               \
            g_fail++;                                                             \
            std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c);    \
        }                                                                         \
    } while (0)

// The game's own 1-byte ShiftState (include/elf/types.h).
struct ShiftState {
    unsigned char data;
    bool Contains(int el) const { return (data & (1 << el)) != 0; }
};

// ---- the game's call shapes -------------------------------------------------------------

// funcs.h entry points, declared the way the game declares them.
extern "C" int __fastcall fn_42fdd4(void *screen);
extern "C" int __fastcall fn_42fdcc(void *screen);
extern "C" int __fastcall fn_434238(void *ctl);
extern "C" int __fastcall fn_43427c(void *ctl);
extern "C" void __fastcall fn_434198(void *c, int v);
extern "C" void __fastcall fn_4341b8(void *c, int v);
extern "C" HWND __fastcall fn_438384(void *ctl);
extern "C" TPoint __fastcall fn_43a8c4(int x, int y);

// A TPackedStream-shaped subclass and the game's non-VCL view of it.
class TTestPackedStream : public Classes::TStream {
public:
    char *out;
    int outsize;
    int used;
    int pos;
    TTestPackedStream() : out(nullptr), outsize(0), used(0), pos(0) {}
    int Read(void *Buffer, int Count) override
    {
        std::memset(Buffer, 0x5a, Count);
        pos += Count;
        return Count;
    }
    int Write(const void *, int) override { return 0; }
    int Seek(int Offset, Word Origin) override { return Origin == soFromCurrent ? pos + Offset : Offset; }
};
struct TestPackedStreamData {       // elf/packres.h TPackedStreamData shape
    void *vmt;
    char *out;
    int outsize;
    int used;
    int pos;
};
struct TStreamVmt;                  // elf/packres.h
typedef int (__fastcall *TStreamReadFn)(TStreamVmt *s, void *buf, int n);
struct TStreamVmt {
    TStreamReadFn *vt;
};

// ---- the form under test: shaped like the game's TForm1 (main_401508) -----------------

class TForm1;
static void *g_form1;               // stands in for g_4601c0 (Form1)
static bool g_refBeforeCtor;
static int g_paints, g_keyDowns, g_keyUps;
static unsigned short g_lastKeyDown, g_lastKeyUp;
static unsigned char g_lastKeyShift;
static std::vector<std::pair<unsigned short, unsigned char>> g_keyDownLog;
static std::string g_keyPresses;

// TForm1::Form1KeyDown in the matched source: a free function taking the form first.
extern "C" void __fastcall fn_test_keydown(void *form, void *sender, unsigned short &key, ShiftState shift)
{
    CHECK(form == g_form1 && sender == g_form1);
    g_keyDowns++;
    g_lastKeyDown = key;
    g_lastKeyShift = shift.data;
    g_keyDownLog.push_back(std::make_pair(key, shift.data));
}

class TForm1 : public TForm {
__published:
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall Form1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);     // bound AS fn_test_keydown
    void __fastcall Form1KeyUp2(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall FormKeyPress(TObject *Sender, char &Key);

public:
    __fastcall TForm1(TComponent *Owner) : TForm(Owner)
    {
        g_refBeforeCtor = (g_form1 == this);   // CreateForm assigns Form1 first
        OnKeyPress = ELF_METHOD(this, FormKeyPress, &TForm1::FormKeyPress);
    }
    VCL_PUBLISHED(TForm1,
                  VCL_METHOD(FormPaint),
                  VCL_METHOD_AS(Form1KeyDown, fn_test_keydown),
                  VCL_METHOD(Form1KeyUp2))
};
VCL_REGISTER_CLASS(TForm1);

void __fastcall TForm1::FormPaint(TObject *Sender)
{
    CHECK(Sender == this);
    g_paints++;
    // draw a black square at (0,0)-(8,8) over the gray erase
    HDC dc = GetDC(Handle);
    RECT r = {0, 0, 8, 8};
    FillRect(dc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
    ReleaseDC(Handle, dc);
}

void __fastcall TForm1::Form1KeyUp2(TObject *Sender, WORD &Key, TShiftState Shift)
{
    g_keyUps++;
    g_lastKeyUp = Key;
}

void __fastcall TForm1::FormKeyPress(TObject *Sender, char &Key) { g_keyPresses += Key; }

// ---- a TStage-shaped object that hooks the form's mouse events (engine_40a978) --------

struct Stage;
struct MouseRec {
    int kind;       // 0 down, 1 move, 2 up
    int button;
    unsigned char shift;
    int x, y;
};
static std::vector<MouseRec> g_mouse;
static int g_clicks;

// engine_40a6e4 shapes: fn_40a6e4(TStage *, void *sender, char shift, int x, int y) etc.
extern "C" void __fastcall fn_test_mousemove(Stage *s, void *sender, char shift, int x, int y)
{
    g_mouse.push_back({1, -1, (unsigned char)shift, x, y});
}
extern "C" void __fastcall fn_test_mousedown(Stage *s, void *sender, char button, char shift, int x, int y)
{
    CHECK(sender == g_form1);
    g_mouse.push_back({0, button, (unsigned char)shift, x, y});
}

struct Stage {
    TForm *form;
    void __fastcall MouseUp(TObject *Sender, char Button, char Shift, int X, int Y)
    {
        g_mouse.push_back({2, Button, (unsigned char)Shift, X, Y});
    }
    explicit Stage(TForm *f) : form(f)
    {
        form->OnMouseMove = ELF_METHOD(this, MouseMove, fn_test_mousemove);
        form->OnMouseDown = ELF_METHOD(this, MouseDown, fn_test_mousedown);
        form->OnMouseUp = ELF_METHOD(this, MouseUp, &Stage::MouseUp);
    }
};

static void on_click(TForm1 *self, TObject *) { g_clicks++; }
static int g_closes;
static void on_close(void *, TObject *, TCloseAction &action)
{
    CHECK(action == caHide);
    g_closes++;
}

// ---- a TThread shaped like the game's TMyThread (threads_40f604..40f734) -----------------

static SDL_threadID g_mainThread;
static int g_updates, g_updatesOffMain;
static bool g_inputDone;
static int g_terminateCalls;
static bool g_terminateOnMain;

class TMyThread : public Classes::TThread {
public:
    void __fastcall Execute() override;
    void __fastcall Update();
    __fastcall TMyThread(bool CreateSuspended) : Classes::TThread(CreateSuspended) {}
};

// fn_40f684: `while (!self->Terminated) TThread_Synchronize(self, self->Update);`
extern "C" void __fastcall fn_test_execute(TMyThread *self)
{
    while (!self->Terminated)
        TThread_Synchronize(self, ELF_METHOD(self, Update, &TMyThread::Update));
}
void __fastcall TMyThread::Execute() { fn_test_execute(this); }
void __fastcall TMyThread::Update()
{
    g_updates++;
    if (SDL_ThreadID() != g_mainThread)
        g_updatesOffMain++;
    if (g_inputDone && g_updates >= 50)
        Application->Terminate();
}
static void on_thread_terminate(void *, TObject *sender)
{
    g_terminateCalls++;
    g_terminateOnMain = SDL_ThreadID() == g_mainThread;
}

// ---- helpers ----------------------------------------------------------------------------------

static void push_key(Uint32 type, SDL_Keycode sym, SDL_Scancode sc, Uint16 mod)
{
    SDL_Event e;
    std::memset(&e, 0, sizeof e);
    e.type = type;
    e.key.state = type == SDL_KEYDOWN ? SDL_PRESSED : SDL_RELEASED;
    e.key.keysym.sym = sym;
    e.key.keysym.scancode = sc;
    e.key.keysym.mod = mod;
    SDL_PushEvent(&e);
}
static void push_button(Uint32 type, Uint8 button, int x, int y, Uint8 clicks)
{
    SDL_Event e;
    std::memset(&e, 0, sizeof e);
    e.type = type;
    e.button.button = button;
    e.button.state = type == SDL_MOUSEBUTTONDOWN ? SDL_PRESSED : SDL_RELEASED;
    e.button.clicks = clicks;
    e.button.x = x;
    e.button.y = y;
    SDL_PushEvent(&e);
}
static void push_motion(int x, int y)
{
    SDL_Event e;
    std::memset(&e, 0, sizeof e);
    e.type = SDL_MOUSEMOTION;
    e.motion.x = x;
    e.motion.y = y;
    SDL_PushEvent(&e);
}
static void push_text(const char *s)
{
    SDL_Event e;
    std::memset(&e, 0, sizeof e);
    e.type = SDL_TEXTINPUT;
    std::snprintf(e.text.text, sizeof e.text.text, "%s", s);
    Vcl::DispatchEvent(&e);     // SDL_PushEvent(SDL_TEXTINPUT) crashes in sdl2-compat
}

static unsigned char sh(std::initializer_list<TShiftStateElem> l)
{
    unsigned char b = 0;
    for (auto e : l)
        b |= (unsigned char)(1u << e);
    return b;
}

// ---- unit parts --------------------------------------------------------------------------------

static void test_closures_properties_streams()
{
    // Closure: member and free-function binding, null, equality.
    struct Obj {
        int n = 0;
        void Add(TObject *, int k) { n += k; }
    } o;
    ELF_CLOSURE(void, TAddEvent, (TObject *Sender, int K));
    TAddEvent a;
    CHECK(!a && a == nullptr);
    a = ELF_METHOD(&o, Add, &Obj::Add);
    CHECK(a && a != nullptr);
    a(nullptr, 3);
    CHECK(o.n == 3);
    TAddEvent b = a;
    CHECK(a == b);
    b = nullptr;
    CHECK(!b && a != b);

    // TShiftState -> the game's ShiftState struct and -> char
    TShiftState s;
    s << ssShift << ssLeft;
    ShiftState g = s;
    CHECK(g.data == sh({ssShift, ssLeft}) && g.Contains(ssShift) && !g.Contains(ssCtrl));
    char c = (unsigned char)s;
    CHECK((unsigned char)c == sh({ssShift, ssLeft}));
    CHECK(s.Contains(ssLeft) && !s.Contains(ssRight));

    // Properties: Application->Title, Screen->Width
    Application->Title = "Elf Bowling";
    CHECK(Application->Title == "Elf Bowling");
    CHECK(AnsiString(Application->Title).Length() == 11);
    CHECK(Screen->Width == 640 && Screen->Height == 480);
    CHECK(fn_42fdd4(*g_45fee8) == 640 && fn_42fdcc(*g_45fee8) == 480);

    // TMemoryStream, Position arithmetic
    TMemoryStream ms;
    ms.Write("abcdefgh", 8);
    CHECK(ms.Position == 8 && ms.Size == 8);
    ms.Position -= 3;
    char buf[8] = {0};
    CHECK(ms.Read(buf, 8) == 3 && !std::memcmp(buf, "fgh", 3));
    ms.Position = 0;
    CHECK(ms.Read(buf, 2) == 2 && buf[0] == 'a');

    // TStream layout: vptr + nothing, Read at vtable slot 1 (TStreamVmt).
    CHECK(sizeof(Classes::TStream) == sizeof(void *));
    CHECK(sizeof(TTestPackedStream) == sizeof(TestPackedStreamData));
    TTestPackedStream ps;
    ps.outsize = 0x1000;
    ps.pos = 7;
    TestPackedStreamData *view = reinterpret_cast<TestPackedStreamData *>(&ps);
    CHECK(view->outsize == 0x1000 && view->pos == 7);
    TStreamVmt *raw = reinterpret_cast<TStreamVmt *>(&ps);
    unsigned char rb[4] = {0};
    CHECK(raw->vt[1](raw, rb, 4) == 4 && rb[3] == 0x5a && ps.pos == 11);

    // TResourceStream over the real exe: sound_40c1f0's "rs->Position -= 3" scan.
    Classes::TResourceStream *rs = new Classes::TResourceStream((System::THandle)HInstance, "TFORM1", (const char *)10);
    int sig = 0;
    CHECK(rs->Read(&sig, 4) == 4 && !std::memcmp(&sig, "TPF0", 4));
    rs->Position -= 3;
    CHECK(rs->Position == 1);
    CHECK(rs->Size == 354);
    delete rs;
    bool threw = false;
    try {
        Classes::TResourceStream missing(HInstance, "NOSUCHRES", "Wave");
    } catch (Classes::EResNotFound &e) {
        threw = e.Message.Pos("NOSUCHRES") > 0;
    }
    CHECK(threw);

    // Point / Rect
    TPoint p = Classes::Point(3, 4);
    CHECK(p.x == 3 && p.y == 4 && fn_43a8c4(5, 6).y == 6);
    TRect r = Classes::Rect(1, 2, 3, 4);
    CHECK(r.Left == 1 && r.bottom == 4 && r.BottomRight.x == 3);

    // TClientSocket stub (never connects)
    Scktcomp::TClientSocket *cs = new Scktcomp::TClientSocket(Application);
    cs->ClientType = Scktcomp::ctNonBlocking;
    cs->Host = "www.nstorm.com";
    cs->Port = 80;
    cs->Name = "ClientSocket";
    cs->Open();
    CHECK(cs->Socket->ReceiveLength() == 0 && AnsiString(cs->Host) == "www.nstorm.com" && cs->Port == 80);
    delete cs;

    // Borland RTL: srand(1) -> 346, 130, 10982, 1090, 11656 (Turbo C / BCB)
    srand(1);
    int r1 = rand(), r2 = rand(), r3 = rand(), r4 = rand(), r5 = rand();
    CHECK(r1 == 346 && r2 == 130 && r3 == 10982 && r4 == 1090 && r5 == 11656);
    CHECK(random(10) >= 0 && random(10) < 10 && random(0) == 0 && min(3, 4) == 3 && __abs__(-5) == 5);
    CHECK(stricmp("PackedFile", "PACKEDFILE") == 0);

    // Exceptions
    try {
        throw Exception("boom");
    } catch (Exception &e) {
        CHECK(e.Message == "boom");
    }
}

// A synthetic DFM with the mouse events (the game's form has none in its DFM).
class TMouseForm : public TForm {
public:
    int downs = 0, moves = 0, ups = 0, clicks = 0;
    void __fastcall FormMouseDown(TObject *, TMouseButton b, TShiftState s, int x, int y) { downs++; }
    void __fastcall FormMouseMove(TObject *, TShiftState s, int x, int y) { moves++; }
    void __fastcall FormMouseUp(TObject *, TMouseButton b, TShiftState s, int x, int y) { ups++; }
    void __fastcall FormClick(TObject *) { clicks++; }
    __fastcall TMouseForm(TComponent *Owner) : TForm(Owner) {}
    VCL_PUBLISHED(TMouseForm, VCL_METHOD(FormMouseDown), VCL_METHOD(FormMouseMove), VCL_METHOD(FormMouseUp),
                  VCL_METHOD(FormClick))
};
VCL_REGISTER_CLASS(TMouseForm);

static void put_s(std::string &d, const char *s)
{
    d += (char)std::strlen(s);
    d += s;
}

static void test_synthetic_dfm()
{
    std::string d = "TPF0";
    put_s(d, "TMouseForm");
    put_s(d, "MouseForm");
    put_s(d, "ClientWidth"); d += '\x03'; d += '\x40'; d += '\x01';        // vaInt16 320
    put_s(d, "Height"); d += '\x03'; d += (char)0xc8; d += '\x00';           // vaInt16 200
    put_s(d, "Caption"); d += '\x06'; put_s(d, "Mouse");
    put_s(d, "Color"); d += '\x07'; put_s(d, "clBlack");
    put_s(d, "Font.Style"); d += '\x0b'; put_s(d, "fsBold"); d += '\x00';     // vaSet [fsBold]
    put_s(d, "OnMouseDown"); d += '\x07'; put_s(d, "FormMouseDown");
    put_s(d, "OnMouseMove"); d += '\x07'; put_s(d, "FormMouseMove");
    put_s(d, "OnMouseUp"); d += '\x07'; put_s(d, "FormMouseUp");
    put_s(d, "OnClick"); d += '\x07'; put_s(d, "FormClick");
    put_s(d, "OnDblClick"); d += '\x07'; put_s(d, "FormMouseDown");          // wrong type: left unbound
    d += '\0';                                                               // end of properties
    d += '\0';                                                               // no children
    System::TMetaClass *cls = __classid(TMouseForm);
    TMouseForm *f = new TMouseForm(nullptr);                                 // not through CreateForm: no DFM
    CHECK(f->Width == 640 && !f->OnMouseDown);
    CHECK(Vcl::ReadDfm(f, d.data(), d.size(), cls, f));
    CHECK(AnsiString(f->Name) == "MouseForm");
    CHECK(f->ClientWidth == 320 && f->Height == 200 && f->Caption == "Mouse" && f->Color == clBlack);
    CHECK(f->OnMouseDown && f->OnMouseMove && f->OnMouseUp && f->OnClick && !f->OnDblClick);
    f->MouseDown(mbLeft, TShiftState(), 1, 2);
    f->MouseMove(TShiftState(), 1, 2);
    f->MouseUp(mbLeft, TShiftState(), 1, 2);
    f->Click();
    CHECK(f->downs == 1 && f->moves == 1 && f->ups == 1 && f->clicks == 1);
    delete f;
}

// ---- the runtime ------------------------------------------------------------------------------

static void test_runtime()
{
    g_mainThread = SDL_ThreadID();
    Application->Initialize();
    Application->Title = "Elf Bowling";
    Application->CreateForm(__classid(TForm1), &g_form1);
    TForm1 *form = static_cast<TForm1 *>(g_form1);
    CHECK(form && g_refBeforeCtor);
    CHECK(Application->MainForm == form);

    // DFM: RCDATA/TFORM1
    CHECK(AnsiString(form->Name) == "Form1");
    CHECK(form->Left == 326 && form->Top == 233);
    CHECK(form->ClientWidth == 640 && form->ClientHeight == 480);
    CHECK(form->Width == 640 && form->Height == 480 && form->width == 640);
    CHECK(form->BorderStyle == bsNone && form->Position == poScreenCenter && form->KeyPreview);
    CHECK(form->Caption == "Elf Bowl" && form->Color == clGray);
    CHECK(form->OnPaint && form->OnKeyDown && form->OnKeyUp && form->OnKeyPress);
    CHECK(!form->Visible);

    // entry points as the game calls them
    CHECK(fn_434238(g_form1) == 640 && fn_43427c(g_form1) == 480);
    CHECK(fn_438384(g_form1) == port_main_hwnd());
    CHECK(TControl_GetClientWidth(g_form1) == 640);

    // TStage-style mouse hookup, and an OnClick bound to a free function
    Stage stage(form);
    form->OnClick = ELF_METHOD(form, Click, on_click);

    TMyThread *thread = new TMyThread(true);
    thread->OnTerminate = Vcl::Bind<on_thread_terminate>((void *)nullptr);
    thread->Resume();

    // queue the input (Run shows the form, then pumps)
    push_key(SDL_KEYDOWN, SDLK_a, SDL_SCANCODE_A, KMOD_LSHIFT);
    push_key(SDL_KEYUP, SDLK_a, SDL_SCANCODE_A, KMOD_LSHIFT);
    push_key(SDL_KEYDOWN, SDLK_RETURN, SDL_SCANCODE_RETURN, KMOD_NONE);
    push_key(SDL_KEYDOWN, SDLK_LEFT, SDL_SCANCODE_LEFT, KMOD_LCTRL);
    push_key(SDL_KEYUP, SDLK_ESCAPE, SDL_SCANCODE_ESCAPE, KMOD_NONE);
    push_key(SDL_KEYDOWN, SDLK_LSHIFT, SDL_SCANCODE_LSHIFT, KMOD_LSHIFT);
    push_motion(100, 200);
    push_key(SDL_KEYDOWN, SDLK_F11, SDL_SCANCODE_F11, KMOD_LSHIFT);   // Watch: text input goes here
    push_button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 10, 20, 1);
    push_motion(12, 22);
    push_button(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 12, 22, 1);
    push_key(SDL_KEYUP, SDLK_LSHIFT, SDL_SCANCODE_LSHIFT, KMOD_NONE);
    push_button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT, 630, 470, 1);
    push_button(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT, 630, 470, 1);
    push_button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 5, 6, 2);   // double click
    push_button(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 5, 6, 2);
    push_key(SDL_KEYDOWN, SDLK_F12, SDL_SCANCODE_F12, KMOD_NONE);

    int keyDownsBefore = g_keyDowns;
    // g_inputDone is set by the F12 key-down (last event); the thread's Update
    // then terminates the application after 50 updates.
    struct Watch {
        static void __fastcall KeyDown(void *form, void *sender, unsigned short &key, ShiftState shift)
        {
            if (key == VK_F11) {        // in queue order: deliver "xé" -> 'x', 0xE9
                push_text("x\xc3\xa9");
                return;
            }
            fn_test_keydown(form, sender, key, shift);
            if (key == VK_F12)
                g_inputDone = true;
        }
    };
    form->OnKeyDown = Vcl::Bind<&Watch::KeyDown>((void *)form);

    Uint32 t0 = SDL_GetTicks();
    Application->Run();
    Uint32 dt = SDL_GetTicks() - t0;
    std::printf("Run() returned after %u ms: %d updates, %d paints, %d key downs, %d key ups, %zu mouse events\n",
                dt, g_updates, g_paints, g_keyDowns - keyDownsBefore, g_keyUps, g_mouse.size());

    CHECK(form->Visible);
    CHECK(g_updates >= 50 && g_updatesOffMain == 0);
    CHECK(g_paints >= 1);

    // keys: VK codes and TShiftState
    CHECK(g_keyDowns - keyDownsBefore == 5);         // A, Return, Left, Shift, F12
    CHECK(g_lastKeyDown == VK_F12);
    std::vector<std::pair<unsigned short, unsigned char>> wantKeys = {
        {'A', sh({ssShift})}, {VK_RETURN, 0}, {VK_LEFT, sh({ssCtrl})}, {VK_SHIFT, sh({ssShift})}, {VK_F12, 0}};
    CHECK(g_keyDownLog == wantKeys);
    CHECK(g_keyUps == 3 && g_lastKeyUp == VK_SHIFT);  // A, Esc, Shift
    CHECK(g_keyPresses == std::string("\rx\xe9"));

    // mouse: client coordinates, buttons, Shift (VCL: the pressed button is in
    // Shift on down, the released one is not on up; ssDouble on a double click)
    std::vector<MouseRec> want = {
        {1, -1, sh({ssShift}), 100, 200},
        {0, mbLeft, sh({ssShift, ssLeft}), 10, 20},
        {1, -1, sh({ssShift, ssLeft}), 12, 22},
        {2, mbLeft, sh({ssShift}), 12, 22},
        {0, mbRight, sh({ssRight}), 630, 470},
        {2, mbRight, 0, 630, 470},
        {0, mbLeft, sh({ssLeft, ssDouble}), 5, 6},
        {2, mbLeft, 0, 5, 6},
    };
    CHECK(g_mouse.size() == want.size());
    for (size_t i = 0; i < want.size() && i < g_mouse.size(); i++) {
        const MouseRec &a = g_mouse[i], &w = want[i];
        bool ok = a.kind == w.kind && a.button == w.button && a.shift == w.shift && a.x == w.x && a.y == w.y;
        if (!ok)
            std::fprintf(stderr, "mouse[%zu]: got kind %d button %d shift %#x (%d,%d)\n", i, a.kind, a.button,
                         a.shift, a.x, a.y);
        CHECK(ok);
    }
    CHECK(g_clicks == 2);

    // paint: gray erase, then OnPaint's black square; present happened
    form->Invalidate();
    int before = g_paints;
    Vcl::PumpOnce(false);
    CHECK(g_paints == before + 1);
    int w = 0, h = 0;
    unsigned char *px = port_screen_pixels(&w, &h);
    const unsigned char *pal = port_system_palette();
    CHECK(w == 640 && h == 480);
    const unsigned char *c0 = pal + 4 * px[0], *c1 = pal + 4 * px[100 * w + 100];
    CHECK(c0[0] == 0 && c0[1] == 0 && c0[2] == 0);
    CHECK(c1[0] == 0x80 && c1[1] == 0x80 && c1[2] == 0x80);

    // Synchronize throughput (informational): the game's frame pump rate
    {
        int u0 = g_updates;
        Uint32 b0 = SDL_GetTicks();
        while (g_updates - u0 < 2000 && SDL_GetTicks() - b0 < 2000)
            Vcl::PumpOnce(true);
        Uint32 bt = SDL_GetTicks() - b0;
        std::printf("Synchronize round trips: %d in %u ms\n", g_updates - u0, bt);
        CHECK(g_updates - u0 >= 100);
    }

    // Synchronize from the main thread runs directly
    int u = g_updates;
    thread->Synchronize(Classes::TThreadMethod::Make<&TMyThread::Update>(thread));
    CHECK(g_updates == u + 1);

    // resize through the game's entry points
    fn_434198(g_form1, 320);
    fn_4341b8(g_form1, 240);
    CHECK(form->ClientWidth == 320 && port_screen_pixels(&w, &h) && w == 320 && h == 240);
    fn_434198(g_form1, 640);
    fn_4341b8(g_form1, 480);

    // thread shutdown: Terminate + WaitFor (the loop pumps Synchronize meanwhile)
    thread->Terminate();
    thread->WaitFor();
    CHECK(g_terminateCalls == 1 && g_terminateOnMain);
    delete thread;

    // Close on the main form (fn_40f3c0: TCustomForm_Close) runs OnClose and
    // terminates the application
    Application->CreateForm(__classid(TMouseForm), nullptr);        // second form: not main, no DFM
    CHECK(Application->MainForm == form);
    form->OnClose = Vcl::Bind<on_close>((void *)nullptr);
    TCustomForm_Close(g_form1);
    CHECK(g_closes == 1 && Application->Terminated);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::fprintf(stderr, "usage: vcl_test \"Elf Bowling.exe\"\n");
        return 2;
    }
    if (port_res_open(argv[1]) != 0)
        return 2;
    test_closures_properties_streams();
    test_synthetic_dfm();
    test_runtime();

    // Shutdown with a thread still running (the game never stops TMyThread):
    // TThread(false) starts on the next pump; Vcl::Shutdown terminates it and
    // no Synchronize'd Update runs afterwards.
    {
        int u = g_updates;
        TMyThread *t2 = new TMyThread(false);
        CHECK(!t2->Finished);
        Uint32 t0 = SDL_GetTicks();
        while (g_updates == u && SDL_GetTicks() - t0 < 2000)
            Vcl::PumpOnce(true);
        CHECK(g_updates > u);
        Vcl::Shutdown();
        int after = g_updates;
        SDL_Delay(20);
        CHECK(t2->Finished && g_updates == after && !Application);
        delete t2;
    }
    Vcl::Shutdown();    // idempotent
    port_shutdown();
    std::printf("vcl_test: %d checks, %d failed\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
