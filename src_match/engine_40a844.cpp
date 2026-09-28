// TStage: centre the play area and make the form cover the screen, 0x40a844.
// The ctor passes its colour as a second argument, which this function
// ignores (cdecl, so the unused parameter is free).
#include <elf/funcs.h>

extern "C" void fn_40a844(TStage *e, int color)
{
    int ox = tmax(0, (fn_42fdd4(*g_45fee8) - e->form->width) / 2);
    int oy = tmax(0, (fn_42fdcc(*g_45fee8) - e->form->height) / 2);
    e->offset.x = ox;
    e->offset.y = oy;
    fn_4341b8(e->form, fn_42fdcc(*g_45fee8));
    fn_434198(e->form, fn_42fdd4(*g_45fee8));
    fn_434158(e->form, 0);
    fn_434178(e->form, 0);
    fn_4348e8(e->form);
}
