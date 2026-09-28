// TStage (TForm) mouse event handlers, 0x40a6e4-0x40a844.
// These are VCL __fastcall handlers: eax=this, edx=Sender, ecx=Button/Shift.
#include <elf/funcs.h>

extern "C" void __fastcall fn_40a6e4(TStage *e, void *sender, char shift, int x, int y)
{
    TPoint pt = fn_43a8c4(x, y);
    e->mouse = fn_40a6a4(e, pt);
    if (e->dragging) {
        e->dragDelta.x = e->mouse.x - e->dragStart.x;
        e->dragDelta.y = e->mouse.y - e->dragStart.y;
    }
}

extern "C" void __fastcall fn_40a764(TStage *e, void *sender, char button, char shift, int x, int y)
{
    e->mousedown = 1;
    TPoint pt = fn_43a8c4(x, y);
    e->down = fn_40a6a4(e, pt);
    e->dragStart = e->down;
    e->dragDelta = fn_43a8c4(0, 0);
    e->dragging = 1;
}

extern "C" void __fastcall fn_40a7e4(TStage *e, void *sender, char button, char shift, int x, int y)
{
    if (e->mousedown) {
        TPoint pt = fn_43a8c4(x, y);
        e->up = fn_40a6a4(e, pt);
        e->dragging = 0;
        e->drag = 0;
    }
}
