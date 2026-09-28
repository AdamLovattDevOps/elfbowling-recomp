// elf/stage.h - TStage (0x8d4 bytes), the engine object: back/front
// buffers, dirty areas, the scene list, mouse state, sound and cast
// managers. RTTI name: TStage. Global: g_4601c8 (see game.h).
//
// Old local names -> TStage: Engine, Stage, App, GameCore, MenuApp, GameObj,
// Sprites (game_*/opening_*: the object at scene+0x13c), Screen
// (range_401d0c only), Game (init_40f350 only).
//
// ctor 0x40a978 (TStage.cpp keeps it as a real member: MATCH);
// mouse handlers 0x40a6e4/0x40a764/0x40a7e4; add dirty area 0x40ae80
// (member); scene switch 0x40bdec; frame 0x40b8fc.
#ifndef ELF_STAGE_H
#define ELF_STAGE_H

#include <elf/types.h>
#include <elf/bitmap.h>

struct TScene;
struct TGraphicSprite;
struct TGraphicCast;
struct TSoundMgr;
struct TCastMgr;
struct TWebTrack;
class TMyThread;

typedef void (*StageCb)();      // old: EngineCb

// Dirty-area rectangle. It has an OUT-OF-LINE default ctor (0x40adc4), which
// makes the TStage ctor build the arrays with _vector_new_ldtc_. Everywhere
// else the areas are handled as ERect (copy ctor): ELF_AS(ERect, e->dirty[i])
// or (ERect *)e->dirty. Old: Area (engine_40a978), Empty (engine_40adc4),
// ERect areas[80] (engine_40ae80/40af80/40ba68, stage_40ae30).
struct Area {
    union {
        struct { int l, t, r, b; };
        struct { int left, top, right, bottom; };
    };
    Area();                     // 0x40adc4
};

// The main form as TStage sees it (old: TForm in engine_40a978, Control in
// engine_40a844). The BCB3 CONTROLS.HPP in the toolchain puts
// FOnMouseDown/Move/Up 0xc bytes later than the VCL the game linked, so this
// stand-in with the game's offsets is used instead of Forms::TForm
// (notes/round1_407.md). With <vcl/classes.hpp> included first, the event
// fields are real __closure types and the TStage mouse handlers take a
// TObject * Sender, so engine_40a978 can assign `form->OnMouseMove =
// ELF_METHOD(this, MouseMove, fn_40a6e4)`. The three member functions are
// the VCL methods (their calls are fixups; the stand-in is never
// constructed). The native build uses the port's real Forms::TForm (its
// `width`/`height` are read-only proxies on TControl).
#if defined(ClassesHPP)
#define ELF_SENDER TObject
ELF_CLOSURE(void, TStageMouseEvent, (TObject *Sender, char Button, char Shift, int X, int Y));
ELF_CLOSURE(void, TStageMouseMoveEvent, (TObject *Sender, char Shift, int X, int Y));
#endif
#ifndef ELF_SENDER
#define ELF_SENDER void
#endif
#ifdef __BORLANDC__
struct TStageForm {
    char _unk00[0x38];
    int width;                  // 0x38 TControl::FWidth  (engine_40a844)
    int height;                 // 0x3c TControl::FHeight
    char _unk40[0x6c - 0x40];
#if defined(ClassesHPP)
    TStageMouseEvent OnMouseDown;       // 0x6c
    TStageMouseMoveEvent OnMouseMove;   // 0x74
    TStageMouseEvent OnMouseUp;         // 0x7c
#else
    void *OnMouseDown[2];       // 0x6c __closure (code, this)
    void *OnMouseMove[2];       // 0x74
    void *OnMouseUp[2];         // 0x7c
#endif
    void __fastcall SetColor(int c);        // Controls::TControl::SetColor
    int __fastcall GetClientWidth();
    int __fastcall GetClientHeight();
};
#else
typedef Forms::TForm TStageForm;
#endif

struct TStage {
    TWebTrack *webtrack;        // 0x000 (old: wt)
    int nareas;                 // 0x004 dirty[] entries in use
    int nareas2;                // 0x008 offstage[] entries in use
    char running;               // 0x00c (old: f0c)
    char paused;                // 0x00d (old: f0d)
    char f0e;                   // 0x00e
    char fullscreen;            // 0x00f hid the taskbar (old: f0f, hidTaskbar)
    TGraphicCast *bgimage;      // 0x010 (old: f10, Image *)
    int bgcolor;                // 0x014 (old: color)
    Area dirty[80];             // 0x018 update areas
    Area offstage[40];          // 0x518 off-stage update areas (old: areas2)
    char dirtyFlag;             // 0x798 (old: dirty, f798)
    char dirtyFlag2;            // 0x799 (old: dirty2)
    char _pad79a[2];
    StageCb onExit;             // 0x79c (old: onStop; void * in engine_40a978). Called by fn_40c130
    TScene *next;               // 0x7a0 pending scene (old: f7a0)
    TScene *nextRet;            // 0x7a4 its return scene (old: f7a4)
    Bitmap back;                // 0x7a8 back buffer (handle at 0x7d4)
    Bitmap front;               // 0x7e0 front buffer (handle at 0x80c)
    HDC dc;                     // 0x818 memory DC (old: f818)
    HPALETTE palette;           // 0x81c (old: pal). HPALETTE: engine_40b158 passes it to
                                //       SelectPalette, the ctor stores 0. range_403540 (int f81c +
                                //       pk()): use TStage::pk() below.
    TStageForm *form;           // 0x820
    Windows::TRect screen;      // 0x824 visible area. engine_40af80/40b158 use ERect,
                                //       range_4060d4 RECT, scene_409fc8/40a0d8 four ints.
    TPoint offset;              // 0x834 screen offset (old: f834, ox/oy)
    char active;                // 0x83c (old: f83c)
    char dragging;              // 0x83d (old: f83d)
    char mousedown;             // 0x83e (old: f83e)
    char _pad83f;
    TGraphicSprite *drag;       // 0x840 sprite being dragged (old: f840)
    int nscenes;                // 0x844 (old: nitems)
    TScene *scene;              // 0x848 current scene
    TScene *scenes[20];         // 0x84c (old: items)
    TPoint dragStart;           // 0x89c
    TPoint dragDelta;           // 0x8a4
    int bpp;                    // 0x8ac screen colour depth (old: f8ac)
    TPoint mouse;               // 0x8b0 (scene_4099dc: f8b8.. as ints)
    TPoint down;                // 0x8b8 mouse-down position (sprites_4092bc: lclick)
    TPoint up;                  // 0x8c0 mouse-up position (sprites_4092bc: rclick; menu: mouse)
    TSoundMgr *sound;           // 0x8c8 (old: snd, sounds, f8c8)
    TCastMgr *casts;            // 0x8cc (old: Obj804 *, List200 *, init_40f350 music)
    TMyThread *thread;          // 0x8d0

    TStage(TStageForm *f, int bpp, int color, char flag, StageCb onExit, TWebTrack *wt);  // 0x40a978
    ~TStage();
    void __fastcall MouseMove(ELF_SENDER *Sender, char Shift, int X, int Y);                 // 0x40a6e4
    void __fastcall MouseDown(ELF_SENDER *Sender, char Button, char Shift, int X, int Y);    // 0x40a764
    void __fastcall MouseUp(ELF_SENDER *Sender, char Button, char Shift, int X, int Y);      // 0x40a7e4

    // matched as real members (engine_40ae80.cpp, engine_40af80.cpp, engine_40ba68.cpp)
    void fn_40ae80(ERect r);
    void fn_40af80(ERect r);
    void fn_40b9b8();
    void fn_40ba68();
    TScene *fn_40bf2c(const char *name);
    TScene *fn_40bf84(const char *name);
    int fn_40c160(const char *name);
    // inline helpers some units need (they change codegen: use only where
    // the unit was matched with them)
    void SetDrag(TGraphicSprite *p) { drag = p; }                       // sprites_408940
    // range_403540 (fn_4052ac caller): the old `int f81c` + this inline.
    // Checked: the HPALETTE field with this body matches range_403540 in full.
    int *pk() { return palette == 0 ? (int *)&palette : 0; }
};

ELF_CHECK_OFS(TStage, nareas, 0x004);
ELF_CHECK_OFS(TStage, running, 0x00c);
ELF_CHECK_OFS(TStage, fullscreen, 0x00f);
ELF_CHECK_OFS(TStage, bgimage, 0x010);
ELF_CHECK_OFS(TStage, bgcolor, 0x014);
ELF_CHECK_OFS(TStage, dirty, 0x018);
ELF_CHECK_OFS(TStage, offstage, 0x518);
ELF_CHECK_OFS(TStage, dirtyFlag, 0x798);
ELF_CHECK_OFS(TStage, onExit, 0x79c);
ELF_CHECK_OFS(TStage, next, 0x7a0);
ELF_CHECK_OFS(TStage, back, 0x7a8);
ELF_CHECK_OFS(TStage, front, 0x7e0);
ELF_CHECK_OFS(TStage, dc, 0x818);
ELF_CHECK_OFS(TStage, palette, 0x81c);
ELF_CHECK_OFS(TStage, form, 0x820);
ELF_CHECK_OFS(TStage, screen, 0x824);
ELF_CHECK_OFS(TStage, offset, 0x834);
ELF_CHECK_OFS(TStage, active, 0x83c);
ELF_CHECK_OFS(TStage, drag, 0x840);
ELF_CHECK_OFS(TStage, nscenes, 0x844);
ELF_CHECK_OFS(TStage, scene, 0x848);
ELF_CHECK_OFS(TStage, scenes, 0x84c);
ELF_CHECK_OFS(TStage, dragStart, 0x89c);
ELF_CHECK_OFS(TStage, bpp, 0x8ac);
ELF_CHECK_OFS(TStage, mouse, 0x8b0);
ELF_CHECK_OFS(TStage, down, 0x8b8);
ELF_CHECK_OFS(TStage, up, 0x8c0);
ELF_CHECK_OFS(TStage, sound, 0x8c8);
ELF_CHECK_OFS(TStage, casts, 0x8cc);
ELF_CHECK_OFS(TStage, thread, 0x8d0);
ELF_CHECK_SIZE(TStage, 0x8d4);

// TWebTrack (0x18 bytes): the "phone home" HTTP tracker, ctor 0x40e888,
// dtor 0x40ebd0 (fn_40ebd0). The real class (with TClientSocket and the
// __closure handlers) is in webtrack_40e5ec.cpp; this is the layout other
// units see, with the same ctor. That unit defines ELF_NO_TWEBTRACK before
// including elf headers and keeps its own.
#ifndef ELF_NO_TWEBTRACK
struct TWebTrack {
    char *buf;                  // 0x00 receive buffer (0x1000 bytes)
    char *request;              // 0x04 HTTP request text
    int len;                    // 0x08 bytes received
    bool f0c;                   // 0x0c
    char _pad0d[3];
    int state;                  // 0x10 1 connected 2 sent 3 reading 4 done 5 reported 6 error
    void *client;               // 0x14 Scktcomp::TClientSocket *

    TWebTrack(const char *host, const char *path, bool f);     // 0x40e888 (init_40f350 passes 1)
};

ELF_CHECK_OFS(TWebTrack, state, 0x10);
ELF_CHECK_SIZE(TWebTrack, 0x18);
#endif

#endif
