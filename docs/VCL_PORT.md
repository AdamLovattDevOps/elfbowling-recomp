# VCL/RTL subset (`port/vcl/`, `port/include/vcl/`)

This is Phase B step 2 of docs/PORT.md. It is the part of Borland C++Builder 3's VCL 3.5 and RTL that the game uses, written as portable C++17 (clang, 64-bit) on top of the Win32 shim (docs/SHIM.md). It contains only the members the game or the runtime needs.

| Path | Contents |
|---|---|
| `port/include/vcl.h`, `vcl/vcl.h` | what `<vcl.h>` gives the game |
| `port/include/vcl/sysmac.h` | the Borland keywords (see "Language extensions") |
| `port/include/vcl/closure.h` | `Vcl::Closure<R(Args...)>`, `Vcl::Bind`, and the native `ELF_CLOSURE` / `ELF_METHOD` |
| `port/include/vcl/property.h` | `Vcl::Property` proxies, `VCL_PROPERTY` |
| `port/include/vcl/system.hpp` | `TObject`, `AnsiString`, `Set<>`, `TMetaClass`, `HInstance`, `Word`/`THandle`/... |
| `port/include/vcl/windows.hpp` | `Windows::TRect`, `TPoint` (the same definition as include/elf/types.h) |
| `port/include/vcl/sysutils.hpp` | `Exception`, `EAbort`, `EConvertError`, `IntToStr`, `Format` |
| `port/include/vcl/classes.hpp` | `TShiftState`, `TNotifyEvent`, `TThreadMethod`, `TPersistent`, `TComponent`, `TStream`, `TCustomMemoryStream`, `TMemoryStream`, `TResourceStream`, `TThread`, `Rect`/`Point`/`Bounds` |
| `port/include/vcl/graphics.hpp` | `TColor` (an int) and the `cl*` constants |
| `port/include/vcl/controls.hpp` | `TControl`, `TWinControl`, `TMouseButton`, `TMouseEvent`, `TMouseMoveEvent`, `TKeyEvent`, `TKeyPressEvent` |
| `port/include/vcl/forms.hpp` | `TCustomForm`, `TForm`, `TScreen`, `TApplication`, `Application`, `Screen`, `VCL_PUBLISHED`, `VCL_REGISTER_CLASS`, runtime hooks |
| `port/include/vcl/scktcomp.hpp` | `TClientSocket` as a no-op stub |
| `port/include/vcl/elfcompat.h` | the plain-function spellings of VCL calls that the matched source declares for itself, and the VCL indirection cells |
| `port/include/bcb_rtl.h` | Borland RTL names that the host libc lacks: `rand`/`srand` (Borland's generator), `random`, `min`/`max`, `__abs__`, `stricmp` |
| `port/vcl/*.cpp` | the runtime: `system`, `classes`, `thread`, `controls`, `forms` (the event loop), `dfm`, `scktcomp`, `entry` (the by-address entry points), `rtl`; `main.cpp` (main → WinMain); `elf_glue.cpp` |
| `port/tests/vcl_test.cpp` | the headless test |

Build and test:

```
make -C port test          # the shim test, then vcl_test (SDL_VIDEODRIVER=dummy)
make -C port elf-glue      # compile-check of vcl/elf_glue.cpp against include/elf
```

`make` builds `build/port/libvcl.a` and `build/port/vcl_main.o`. `vcl_main.o` holds `main()`, which loads the exe, calls `WinMain`, then runs `Vcl::Shutdown` and `port_shutdown`.

The native game build (Phase C) needs these flags: `-std=c++17 -include bcb_rtl.h -Wno-unknown-pragmas -Iport/include -Iinclude`. It links `vcl_main.o libvcl.a libshim.a`, `vcl/elf_glue.cpp`, and the SDL2 libraries.

## What the game uses

The inputs were:
- the `lib` rows of notes/port_deps.tsv;
- the `library / VCL / RTL entry points called by address` section of include/elf/funcs.h;
- a grep of src_match/ and include/elf/;
- the form resource.

| Area | Used by | Provided |
|---|---|---|
| `TApplication` | elf_40128c: `Initialize`, `Title =`, `CreateForm(__classid(TForm1), &Form1)`, `Run`, `ShowException`. init_40f350: `Terminate`. range_401cec: `MessageBox` | yes, plus `ProcessMessages`, `HandleMessage`, `Terminated`, `MainForm` |
| `TForm`/`TCustomForm` | main_401508: `TForm1 : TForm` and its ctor/dtor. init_40f350: `Close`. webtrack: `SetClientWidth/Height` | yes, plus DFM loading and `OnPaint`/`OnClose`/`OnShow`/... |
| `TControl` | fn_434158..4341b8 `SetLeft/Top/Width/Height`, fn_434238/43427c `GetClientWidth/Height`, fn_4348e8 `BringToFront`, fn_434b6c/74 `Hide`/`Show`, `SetColor`, the `FWidth`/`FHeight` reads (engine_40a844), `OnMouseDown/Move/Up` (engine_40a978) | yes |
| `TWinControl` | fn_438384 `GetHandle` | the shim's one HWND |
| `TScreen` | fn_42fdcc/42fdd4 `GetHeight/GetWidth` through `*g_45fee8` | a virtual 640×480 desktop (see below) |
| `TThread` | TMyThread: ctor(true), `Resume`, `Execute`, `Synchronize`, `Terminated` | SDL_Thread (see "Threads") |
| `TStream` / `TResourceStream` / `TCustomMemoryStream` | TPackedStream (packres), sound_40c1f0: `Read`, `Position -= 3`, `new TResourceStream(HInstance, name, "Wave")`, dtors | yes; `TMemoryStream` too |
| `AnsiString` | `Title = "..."`, `TComponent::Name`, `Host` | yes |
| `Exception` | elf_40128c `catch (Exception &)` | yes |
| `TClientSocket` | webtrack_40e5ec | a stub (never reached: `LoadLibraryA` is NULL, so fn_40e800 reports "no Internet connection") |
| `Classes::Rect` / `Point` | 53 + 125 calls (fn_43a8dc/43a8c4, `Classes::`, `Classes_*`) | yes |
| RTL | `new`/`delete`, `new[]` (fn_448a94), `rtl_*`, `rand`/`srand`/`random`/`min`/`__abs__`/`stricmp`/`time` | entry.cpp, bcb_rtl.h |

`@Dsgnintf@TSetElementProperty@$bdtr$qqrv` in port_deps.tsv is a library-matcher false positive. The design-time packages are not linked into the game.

## The form (RCDATA/TFORM1)

The resource is 354 bytes in binary DFM format (`TPF0`). `python3 tools/rsrc.py --extract build/rsrc` extracts it, and `scratch/ph_b2/dfm.py` (a local script; scratch/ is git-ignored) decodes it:

```
object Form1: TForm1
  Left = 326
  Top = 233
  BorderStyle = bsNone
  Caption = 'Elf Bowl'
  ClientHeight = 480
  ClientWidth = 640
  Color = clGray
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  Position = poScreenCenter
  OnKeyDown = Form1KeyDown
  OnKeyUp = Form1KeyUp2
  OnPaint = FormPaint
  PixelsPerInch = 96
  TextHeight = 13
end
```

- The DFM has no child components and no mouse events. TStage's constructor assigns `OnMouseDown`/`OnMouseMove`/`OnMouseUp` at run time.
- `FormPaint` is fn_4015f8, which calls fn_40ec20 and then fn_40b1d4 (the stage repaint).
- `Form1KeyDown` is fn_401610, which calls fn_40bd3c. `Form1KeyUp2` is fn_401644, which calls fn_40bda4.

When `CreateForm` or `Vcl::CreateComponent` builds a registered class, `TCustomForm`'s constructor reads `RCDATA/<ClassName>` from the user's exe through the shim's `FindResourceA`, as VCL's `InitInheritedComponent` does. It applies the properties with VCL semantics:

| Property | How the port applies it |
|---|---|
| `Left`, `Top`, `Width`, `Height`, `ClientWidth`, `ClientHeight` | stored in `FLeft`..`FHeight` |
| `Color` | the identifier is mapped through `IdentToColor` |
| `Caption`, `Visible`, `Enabled` | stored. `Visible = True` shows the form after loading. |
| `BorderStyle`, `Position`, `KeyPreview` | stored |
| `Font.*`, `PixelsPerInch`, `TextHeight`, `Scaled`, `OldCreateOrder`, ... | accepted, no effect |

`On*` identifiers are bound through the class's published-method table. An unknown property, or a method that is not published or does not fit the event, is logged and the loader continues. Borland would raise `EReadError` instead.

VCL assigns `*Reference` before it runs the constructor. The game relies on this: TForm1's constructor calls fn_401508, then fn_40f460(g_4601c0), and g_4601c0 is Form1. `CreateComponent` keeps that order.

## Language extensions

| Borland | Native |
|---|---|
| `__fastcall` | empty. It is defined in port/include/windows.h, so non-VCL units (funcs.h) see it too. |
| `__published:` | `public:` |
| `__classid(T)` | `Vcl::FindClass("T")`: the metaclass that `VCL_REGISTER_CLASS(T)` registered. It aborts with a message if T is not registered. |
| `__declspec(delphiclass, package)` and the like | empty off Windows. These appear only in the local VCL copies that the migration removes. |
| `__thread` | GNU `__thread`, which has the same meaning and the same position (`extern T __thread x;`); `thread_local` on MSVC |
| `PACKAGE`, `DELPHICLASS`, `HIDESBASE` | empty. `DYNAMIC` becomes `virtual`. |
| `USEFORM(file, Form1)` | `class TForm1;`. The game supplies the `Form1` variable (see "Game source changes"). |
| `USERES`, `USEUNIT`, `USEOBJ`, ... | a dummy `extern` declaration, as SYSDEFS.H has |
| `#pragma hdrstop` / `package(smart_init)` / `option -k` / `resource` / `link` | build with `-Wno-unknown-pragmas` |
| `__property` | cannot be a macro. The port's classes declare proxy members (below). The game's own units only use `__property` in the local VCL copies. |
| `__closure` | cannot be a macro: `ELF_CLOSURE` (below) |

### Properties

`VCL_PROPERTY(Owner, T, Name, &Owner::GetX, &Owner::SetX)` declares a proxy member.
- It holds the owner pointer.
- It converts to `T` through the getter.
- It assigns through the setter, including compound assignment, `++`/`--` and `->`.
- A missing getter or setter is `nullptr`.

So `Application->Title = "Elf Bowling"`, `client->Port = 80` and `if (form->KeyPreview)` compile unchanged.

Field-backed properties (`{read=FOnPaint, write=FOnPaint}`) are plain public fields, which covers all the event properties.

`TStream` is the exception. Its `Position` and `Size` are empty `[[no_unique_address]]` proxies that recover their owner from their own offset, because `TStream` must stay "vtable pointer + nothing" (see "Layout"). `rs->Position -= 3` (sound_40c1f0) therefore also compiles unchanged.

Limitation: a proxy is not the value itself. Where a real `T` is needed, convert first, for example `AnsiString(form->Caption).c_str()`. `Application->Title == "x"` works, because AnsiString's comparison operators are found through the proxy.

### Closures: the macro proposal

A Borland closure is a pair (code, this). `Vcl::Closure<R(Args...)>` stores `void *obj` and a thunk pointer. The thunk is instantiated for one callee, so the closure is two pointers, trivially copyable, comparable, and testable with `if (closure)`. `Vcl::Bind<F>(obj)` binds F, which can be:
- a member function: `Vcl::Bind<&TStage::MouseMove>(stage)`;
- a free function whose first parameter takes the object. The matched source defines its handlers this way: fn_40a6e4(TStage *, void *sender, char shift, int x, int y).

The callee's parameters only have to be convertible from the closure's:
- `TShiftState` converts to `char` and to any 1-byte trivially copyable struct, such as the game's `ShiftState`;
- `TMouseButton` converts to `char`;
- `TObject *` converts to `void *`.

So the matched handlers bind as they are spelled, with no casts.

Declarator syntax (`typedef void __fastcall (__closure *T)(...)`) and the implicit `this` in `form->OnMouseMove = MouseMove` cannot be macro-replaced. Two source-side macros cover them. For the Borland side, add these to include/elf/types.h:

```c++
#ifdef __BORLANDC__
#define ELF_CLOSURE(R, Name, Params) typedef R __fastcall (__closure *Name) Params
#define ELF_METHOD(Obj, Member, Impl) (Obj)->Member
#endif
```

For every other compiler, port/include/vcl/closure.h defines them (types.h pulls it in through `<vcl/vcl.h>`):

```c++
#define ELF_CLOSURE(R, Name, Params) typedef ::Vcl::Closure<R Params> Name
#define ELF_METHOD(Obj, Member, Impl) (::Vcl::Bind<Impl>(Obj))
```

Usage:

```c++
ELF_CLOSURE(void, TStageMouseEvent, (TObject *Sender, char Button, char Shift, int X, int Y));
form->OnMouseMove = ELF_METHOD(this, MouseMove, fn_40a6e4);   // Borland: (this)->MouseMove
TThread_Synchronize(self, ELF_METHOD(self, Update, fn_40f674));  // Borland: (self)->Update
```

- `Member` is what Borland binds.
- `Impl` is what the native build calls. It is the matched free function (`fn_XXXXXX`), or `&Class::Member` when the member has a body.
- **Verified:** engine_40a978 with the three `ELF_METHOD` assignments still matches 1098/1098 bytes under bcc32 (`tools/match.py --no-record` on a scratch copy). ELF_CLOSURE expands to the original declarator text.

### Published methods and metaclasses

There is no RTTI for `__published` methods, so a form class that has a DFM lists them itself:

```c++
class TForm1 : public TForm {
__published:
    void __fastcall FormPaint(TObject *Sender);
    ...
public:
    __fastcall TForm1(TComponent *Owner);
#ifndef __BORLANDC__
    VCL_PUBLISHED(TForm1,
                  VCL_METHOD_AS(FormPaint, fn_4015f8),        // DFM name -> the matched handler
                  VCL_METHOD_AS(Form1KeyDown, fn_401610),
                  VCL_METHOD_AS(Form1KeyUp2, fn_401644))
#endif
};
#ifndef __BORLANDC__
VCL_REGISTER_CLASS(TForm1);       // __classid(TForm1)
#endif
```

`VCL_METHOD(Name)` publishes a member that has a body. The event kind comes from the property: TNotifyEvent, TKeyEvent, TKeyPressEvent, TMouseEvent, TMouseMoveEvent or TCloseEvent. A method is bound only if it can be called with that event's parameters.

## Runtime

**Window.** There is one native window: the shim's.
- The first form shown owns it. `Show` calls `port_init(ClientWidth, ClientHeight, title)`, so the window and screen size come from the DFM (640×480).
- The title is `Application->Title` ("Elf Bowling", as on the Windows taskbar), or the Caption if the Title is empty.
- `BorderStyle = bsNone` makes the window borderless (`port_sdl_window()`, new in port.h).
- The window is always centred on the real display. `Left`/`Top` are stored but not applied, because the game moves its form to 0,0 of the virtual desktop.
- A later `SetWidth`/`SetHeight` resizes the window and the screen.

**Screen.** `TScreen::Width`/`Height` are `$PORT_SCREEN_W`/`$PORT_SCREEN_H`, default 640×480.
- The game makes its form cover the desktop (fn_40a844: offset = (Screen − form)/2, then form size = Screen) and shrinks the form to the desktop (fn_40ec34).
- With a 640×480 virtual desktop, the window stays 640×480 and the offset is 0.
- A larger value gives a desktop-sized window with the game centred in it, which is what the original did.

**The loop.** `Application->Run()` shows the main form, then repeats `Vcl::PumpOnce(true)` until `Terminate()`. Each pass does the following:
1. It handles up to 256 pending SDL events (see below).
2. It runs the queued `TThread::Synchronize` calls, and starts any threads created with `CreateSuspended = false`.
3. If the form is invalid, it erases the form with `Color` and calls `OnPaint(Sender = form)`. The erase is WM_ERASEBKGND and uses stock brushes only: black, white, the grays. `Invalidate()`, show, expose, resize and restore make the form invalid.
4. It calls `port_present()` when something drew, at most every 8 ms. A pending frame is still presented within 8 ms when the loop goes idle.
5. When nothing happened, it waits:
   - while a TThread runs, on the Synchronize queue's condition variable in 1 ms slices (a Synchronize round trip takes about 5 µs here);
   - otherwise in `SDL_WaitEventTimeout`.

`ProcessMessages` is one non-waiting pass. `Vcl::DispatchEvent(&sdl_event)` feeds one event directly, for bots and tests.

**Keys.**

| SDL event | VCL call |
|---|---|
| `SDL_KEYDOWN` | `OnKeyDown(Sender, Word &Key, Shift)`. Auto-repeat is included, as WM_KEYDOWN does. |
| `SDL_KEYUP` | `OnKeyUp` |
| `SDL_TEXTINPUT` | `OnKeyPress(Sender, char &Key)`, one call per Latin-1 character |
| Return / Esc / Backspace / Tab / Ctrl+letter key-down | `OnKeyPress` with 13 / 27 / 8 / 9 / 1..26 (WM_CHAR) |

- `Key` is the Windows virtual-key code: 'A'..'Z', '0'..'9', VK_F1.., VK_RETURN, VK_ESCAPE, the arrows, VK_SHIFT/CONTROL/MENU, the numpad and the OEM keys. port/include/windows.h now defines `VK_*`.
- `Shift` holds ssShift/ssAlt/ssCtrl from the key event's modifiers.
- Letters on non-US layouts fall back to the scancode's US position.
- The game tests 13, 27, 'D','G','N','S','X', and Shift+Return (fn_40bd3c ignores it).

**Mouse.**

| SDL event | VCL call |
|---|---|
| `SDL_MOUSEBUTTONDOWN` | `OnMouseDown(Sender, Button, Shift, X, Y)` |
| `SDL_MOUSEMOTION` | `OnMouseMove(Sender, Shift, X, Y)` |
| `SDL_MOUSEBUTTONUP` | `OnMouseUp(Sender, Button, Shift, X, Y)` |

- X and Y are client coordinates. The window has no non-client area, and SDL scales the coordinates when the renderer's logical size differs.
- `Button` is mbLeft/mbRight/mbMiddle.
- `Shift` is the modifiers plus the buttons held, with Windows' MK_ semantics: on a button-down, `Shift` includes that button; on a button-up, it no longer does. A double click adds ssDouble and calls `OnDblClick` first. A left button-up inside the client area also calls `OnClick`.
- The modifiers of a mouse event are those of the last key event OR'd with `SDL_GetModState()`.

**Close.** `SDL_QUIT` calls `MainForm->Close()`. `Close` runs `OnClose` (default caHide), and on the main form it calls `Application->Terminate()`. The game's exit callback fn_40f3c0 does `TCustomForm_Close` and then `TApplication_Terminate`.

**MessageBox.** It always logs to stderr. It shows `SDL_ShowSimpleMessageBox`, except under the dummy or offscreen video drivers or with `$PORT_NO_MSGBOX`. It returns IDOK, or IDYES for MB_YESNO. `ShowException` shows `E->Message`.

## Threads

The game's only thread is TMyThread (threads_40f604..40f734). TStage's constructor does `new TMyThread(true)` and then `Resume()`, and the thread's Execute (fn_40f684) is

```c++
while (!Terminated) Synchronize(Update);    // Update = fn_40f674 -> fn_40c1b4, the frame tick
```

All game logic therefore runs on the main thread, interleaved with input and paint. The thread only paces it.

A cooperative model would have to run Execute's infinite loop on the main thread. So the port keeps a real thread, which is simple and safe:
- `TThread` runs `Execute` on an SDL_Thread.
- `Synchronize` from the worker queues the call, signals the main loop, and blocks on a condition variable until the main thread has run it. It rethrows any exception in the worker, as VCL does.
- `Synchronize` called on the main thread runs the method directly.
- `WaitFor` on the main thread keeps running Synchronize calls, so it cannot deadlock with a worker that is waiting in Synchronize.
- `OnTerminate` runs through Synchronize.
- `FreeOnTerminate` deletes the thread on its own thread, detached.

Differences from VCL 3:
- `TThread(false)` starts on the next main-loop pump (or on `Resume`), not inside the base constructor, where `Execute` would still be pure virtual.
- `Suspend` is advisory. `Resume` starts a thread that was created suspended.
- `Terminated`, `Synchronize` and `Execute` are public, because the matched source calls them from free functions. `Finished` is added.
- `Vcl::Shutdown()`, which `main()` calls after WinMain, terminates every thread, makes pending and later `Synchronize` calls return without running, and waits up to 1 s. No game frame runs after `Run()` returns. It then deletes Application, which deletes the forms (`OnDestroy`).

## Layout the game depends on

- **`TObject` has no virtual functions and no data.** `TStream` is therefore exactly a vtable pointer (a `static_assert` in classes.cpp checks this), so:
  - a game subclass (TPackedStream in elf/packres.h) has the same layout as the non-VCL view of it (`TPackedStreamData`, whose first field is `void *vmt`);
  - `TStream::Read` is **vtable slot 1** (Delphi order: SetSize, Read, Write, Seek, then the destructor), which is where the game's raw VMT calls look (`TStreamVmt`: `st->vt[1](st, buf, n)`, in packres_40d578/40d6f0/40d980 and range_401d0c).
  - Calling a member function through a plain function pointer that takes `this` first works on SysV x86-64, AArch64 and Win64. vcl_test checks it.
- The polymorphic roots (`TPersistent`/`TComponent`, `TStream`, `TThread`, `Exception`) declare their own virtual destructors.
- Components are single-inheritance with the `TComponent` base at offset 0, so the form pointer the game keeps as `void *` (g_4601c0) is also the `TForm *` and the `TComponent *`.

## Entry points and globals the port defines

In entry.cpp and elf_glue.cpp. Every one is taken from funcs.h or elfcompat.h:

| Kind | Names |
|---|---|
| `extern "C"` by address | `fn_42fdcc`/`fn_42fdd4` (Screen height/width), `fn_434158`/`fn_434178`/`fn_434198`/`fn_4341b8` (SetLeft/Top/Width/Height), `fn_434238`/`fn_43427c` (client size), `fn_4348e8` (BringToFront), `fn_434b6c`/`fn_434b74` (Hide/Show), `fn_438384` (GetHandle), `fn_43a8c4` (Point), `fn_43a8dc` (Rect, returns ERect: elf_glue.cpp), `fn_448a2c` (delete), `fn_448a94` (new[]) |
| `rtl_*` | libc. `rtl_srand` seeds the Borland generator. |
| C++ spellings (elfcompat.h) | `TCustomForm_Close`, `TCustomForm_SetClientWidth/Height`, `TApplication_Terminate`, `TApplication_MessageBox`, `TScreen_GetWidth/Height`, `TControl_GetClientWidth/Height`, `TCustomWinSocket_SendBuf/ReceiveLength/ReceiveBuf`, `TThread_Synchronize(void *, TThreadMethod)`, `Classes_Point`, `Classes_Rect`, `Classes_TRect` (elf_glue.cpp) |
| indirection cells, `extern "C"` | `g_45fee4` → `&Application`, `g_45fee8` → `&Screen`, `g_45fee0` → `&HInstance` (the cells at 0x45fee0..0x45fee8). The cell at 0x45fedc points at Form1, which is the game's own `g_4601c0`. **Exclude g_45fee0/4/8 from any generated game-data definitions.** |

`bcb_rtl.h` makes `rand()` Borland's LCG:
- `seed = seed * 0x015A4E35 + 1`, the result is bits 16..30 of the seed, and RAND_MAX is 0x7FFF. `srand(1)` gives 346, 130, 10982, 1090, 11656, the same as the original.
- `random(n)` is `_lrand() % n`. `_lrand`'s assembly multiplies the 64-bit seed by 0x0000015A00004E35.

## Test (`port/tests/vcl_test.cpp`)

The test runs headless (`SDL_VIDEODRIVER=dummy`) with the real exe. It currently makes 92 checks, all passing:

- **Closures:** bound to members and to game-style free functions, null tests and equality. `TShiftState` converts to the game's `ShiftState` and to `char`.
- **Properties:** `Application->Title`, `Screen->Width`, `TMemoryStream::Position -= 3`.
- **Streams:**
  - `sizeof(TStream)` is one pointer;
  - a TPackedStream-shaped subclass has TPackedStreamData's layout;
  - the raw `vt[1]` call reaches Read;
  - `TResourceStream` over RCDATA/TFORM1 reads the `TPF0` signature, `Position -= 3`, and `Size` is 354;
  - a missing resource raises EResNotFound.
- **Other units:** `Rect`/`Point`/`fn_43a8c4`, the TClientSocket stub, Exception, and the Borland `rand` sequence, `random`, `min`, `__abs__` and `stricmp`.
- **DFM:** `CreateForm(__classid(TForm1), &g_form1)` loads the real TFORM1: every property above, OnPaint/OnKeyDown/OnKeyUp bound, and Form1 assigned before the constructor runs. A synthetic DFM with OnMouseDown/Move/Up/OnClick binds members and rejects a mismatched method.
- **Runtime:**
  - A Stage object binds the form's mouse events with `ELF_METHOD` (free functions with `char` parameters and a member), as engine_40a978 does.
  - A TMyThread loops `while (!Terminated) TThread_Synchronize(self, ELF_METHOD(self, Update, ...))`.
  - The test injects SDL key and mouse events, then `Application->Run()`. The thread's Update terminates Run after the input and 50 updates.
  - Keys are checked as (VK, Shift): ('A', ssShift), (VK_RETURN, none), (VK_LEFT, ssCtrl), (VK_SHIFT, ssShift), (VK_F12, none). KeyUp gets A, Esc and Shift. KeyPress gets "\r", 'x', 0xE9.
  - Eight mouse events are checked for kind, button, Shift and client position, including ssLeft on down but not on up, ssRight, ssDouble, and two OnClicks.
  - Every Update runs on the main thread.
  - OnPaint fires on show and on Invalidate. The screen shows the clGray erase and OnPaint's black square.
- **Timing and shutdown:**
  - The test times 2000 Synchronize round trips. They take about 10 ms.
  - A resize through fn_434198/fn_4341b8 resizes the shim screen.
  - Terminate + WaitFor runs OnTerminate on the main thread.
  - `TCustomForm_Close` runs OnClose and terminates the application.
  - `TThread(false)` starts on the next pump, and `Vcl::Shutdown` stops it with no Update after it.

SDL_PushEvent crashes on `SDL_TEXTINPUT` under sdl2-compat (the SDL2 on this Mac is sdl2-compat 2.32 on SDL3), so the test delivers the text event with `Vcl::DispatchEvent`, in queue order.

## Game source changes for a native compile

These are for the migration agents to apply. The port does not edit include/ or src_match/. Every item below was checked with `clang++ -std=c++17 -fsyntax-only -include bcb_rtl.h` against scratch copies of the headers (scratch/ph_b2/inc). Items 1–7 were also checked with bcc32 where they touch Borland code.

1. **include/elf/types.h**
   - Natively, `#include <vcl/vcl.h>` before `<windows.h>` (`#ifndef __BORLANDC__`). The port's `Windows::TRect`/`Classes::Rect`/closures are then the ones every unit sees. types.h already skips its own copies when `WindowsHPP`/`ClassesHPP` are set.
   - Add the Borland `ELF_CLOSURE`/`ELF_METHOD` definitions above.
   - `ShiftState` stays as it is: `TShiftState` converts to it.
2. **include/elf/stage.h**
   - Write the two `TStageMouse*Event` typedefs as `ELF_CLOSURE(...)`.
   - Natively, `typedef Forms::TForm TStageForm;` in place of the offset stand-in (`#ifdef __BORLANDC__` keeps the struct). Its `width`/`height` are read-only proxies on `TControl`, so engine_40a844 is unchanged.
   - With (1) and (2), the VCL units compile natively as they are: packres_40d4bc, packres_40e0a8, packres_40f104, packres_40f2a4, sound_40c1f0 (its `(intptr_t)HInstance` and `rs->Position -= 3` are fine).
3. **engine_40a978.cpp**: use `form->OnMouseMove = ELF_METHOD(this, MouseMove, fn_40a6e4);`, and the same for MouseDown with fn_40a764 and MouseUp with fn_40a7e4. Checked with bcc32: 1098/1098 bytes.
4. **main_401508.cpp**: add the native `VCL_PUBLISHED(TForm1, VCL_METHOD_AS(FormPaint, fn_4015f8), VCL_METHOD_AS(Form1KeyDown, fn_401610), VCL_METHOD_AS(Form1KeyUp2, fn_401644))` and `VCL_REGISTER_CLASS(TForm1)` in `#ifndef __BORLANDC__` blocks. Checked with bcc32: 11/11 OK, 826 bytes.
5. **elf_40128c.cpp**
   - `WINAPI WinMain(...)` relies on implicit int: write `int WINAPI WinMain(...)`.
   - Natively, Form1 is g_4601c0: `#define Form1 g_4601c0`, after game.h's `extern "C" void *g_4601c0`. `&Form1` then goes to `CreateForm(TMetaClass *, void *Reference)`.
   - Checked with bcc32: WinMain and the Exception destructor still match, 296/296 bytes.
6. **threads_40f604 / 40f734**
   - Replace the local `TObject`/`TThread` copies with `<vcl/classes.hpp>`, as engine_40a978 already does.
   - Natively, add bodies: `void __fastcall TMyThread::Execute() { fn_40f684(this); }` and `void __fastcall TMyThread::Update() { fn_40f674(this); }`.
7. **threads_40f674 / 40f684**
   - Drop the local `TThreadMethod` closure typedef (use `Classes::TThreadMethod`) or write it with `ELF_CLOSURE(void, TThreadMethod, ())`.
   - Write `self->Update` as `ELF_METHOD(self, Update, fn_40f674)`.
   - The struct view `self->terminated` (+0x0c) has the 32-bit layout. Natively it must be `self->Terminated`, for example through a macro in the future threads header.
   - `TThread_Synchronize(void *, TThreadMethod)` is provided.
8. **webtrack_40e5ec.cpp** (define `ELF_NO_TWEBTRACK`)
   - Replace the local VCL copies with `<vcl/scktcomp.hpp>`. ScktComp is not in `<vcl.h>`.
   - Bind the seven handlers with `ELF_METHOD(this, OnXXX, fn_40eXXX)`: On5ac→fn_40e5ac, On5cc→fn_40e5cc, On5ec→fn_40e5ec, On624→fn_40e624, On664→fn_40e664, On684→fn_40e684, On6ac→fn_40e6ac. fn_40e5ac and fn_40e5cc are defined in packres_40dbe0.
   - Natively, `self->client->socket` (+0x80) becomes `client->Socket`, and `new TClientSocket(*p_Application)` becomes `(Classes::TComponent *)*g_45fee4`. The code is never reached in the port, but it must compile.
9. **p_Application → g_45fee4** (range_401cec, init_40f350, webtrack), through game.h's `extern "C"` declaration. The unmigrated units declare `p_Application` with C++ linkage. The C++ free-function spellings (`TApplication_MessageBox`, `TCustomForm_Close`, ...) can stay.
10. **fn_40e19c** is not library code. funcs.h's `is_open` label is a libmap false match. Its body is `push s+0x10; call fn_4207e6` (inflateEnd on `&TPackedStreamData::z`). It needs a native definition written against the struct, because +0x10 is the 32-bit offset. No unit defines it yet.
11. **scene_409c50** calls `ctl->vt[0x7c / 4](ctl)` on a TControl VMT. The code is dead (`TScene::nbuttons` is never raised above 0), but it has no native meaning. Guard it, or map it to the VCL method once its slot is identified.
12. **Not VCL, found by the same compile** (Phase C):
    - menu_41008c `(int)ShellExecuteA(...)` → `(INT_PTR)`;
    - range_401d0c `tmax_if(0L, ...)` (docs/SHIM.md);
    - zlib_41e2cc redeclares `memcpy`/`calloc` K&R-style and uses `register`: build it as C, or with `-Wno-register` and without the redeclarations.
    - `bcb_rtl.h` resolves `stricmp` (scene_40a520), `min` (unit_411), `random` and `__abs__` (game_414240).
