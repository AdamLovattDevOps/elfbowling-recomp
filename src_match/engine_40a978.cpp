// TStage constructor, 0x40a978. VCL unit: <vcl/classes.hpp> comes before the
// elf headers. The form is TStageForm (elf/stage.h), a stand-in with the
// game's event offsets: the BCB3 CONTROLS.HPP layout puts
// FOnMouseDown/Move/Up 0xc bytes later than the VCL the game linked
// (0x6c/0x74/0x7c). See notes/round1_407.md.
// Bitmap *pb/*pf are the unused member-pointer locals seen in the Cast ctors.
// MATCH 40a978 @TStage@$bctr$qp10TStageFormiicpqv$vp9TWebTrack
#include <vcl/classes.hpp>
#include <elf/funcs.h>

// TMyThread is in elf/thread.h (needs <vcl/classes.hpp> first).

TStage::TStage(TStageForm *f, int abpp, int acolor, char flag, StageCb aonExit, TWebTrack *awt)
{
    Bitmap *pb = &back;
    Bitmap *pf = &front;
    fn_4022fc("Beginning of Stage Creation");
    fn_4036c8("Arial", 0x10, 700);
    fn_4036c8("Arial", 0x2c, 700);
    fn_4036c8("Arial", 0xe, 700);
    fn_4036c8("Arial", 0x10, 200);
    fn_4036c8("Lucida Handwriting", 0x13, 600);
    fn_4036c8("Lucida Calligraphy", 0x13, 600);
    fn_4036c8("Arial", 0x14, 700);
    form = f;
    f->SetColor(acolor);
    g_4601c8 = this;
    fn_401cec();
    webtrack = awt;
    running = 1;
    active = 0;
    paused = 0;
    bgimage = 0;
    fullscreen = flag;
    bgcolor = acolor;
    bpp = abpp;
    nscenes = 0;
    palette = 0;
    scene = 0;
    sound = new TSoundMgr;
    casts = new TCastMgr(this);
    onExit = aonExit;
    dragging = 0;
    mousedown = 0;
    drag = 0;
    dragStart = Point(0, 0);
    dragDelta = Point(0, 0);
    next = 0;
    nextRet = 0;
    dc = CreateCompatibleDC(0);
    mouse.x = -1;
    mouse.y = -1;
    down.x = -1;
    down.y = -1;
    up.x = -1;
    up.y = -1;
    form->OnMouseMove = ELF_METHOD(this, MouseMove, fn_40a6e4);
    form->OnMouseDown = ELF_METHOD(this, MouseDown, fn_40a764);
    form->OnMouseUp = ELF_METHOD(this, MouseUp, fn_40a7e4);
    screen = Rect(0, 0, form->GetClientWidth(), form->GetClientHeight());
    offset = Point(0, 0);
    int size = sizeof(BITMAPINFOHEADER);
    if (bpp == 8)
        size += 256 * sizeof(RGBQUAD);   // unsigned addend: load/add/store, not add [mem],imm
    g_45552c = (BITMAPINFOHEADER *)fn_402338(size, "TStage");
    memset(g_45552c, 0, size);
    g_45552c->biSize = sizeof(BITMAPINFOHEADER);
    if (fullscreen) {
        fn_401df8();
        fn_40a844(this, acolor);
    }
    fn_40ae60(this);
    fn_40ae70(this);
    thread = new TMyThread(true);
    thread->Resume();
}
