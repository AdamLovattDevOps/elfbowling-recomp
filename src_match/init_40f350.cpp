// Init unit 0x40f350-0x40f5f4
#include <stdlib.h>
#include <time.h>
#include <elf/funcs.h>

extern "C" void fn_40f350(TStage *g)
{
    fn_405a1c(g->casts);
}

extern "C" void fn_40f364(TStage *g)
{
    TSoundMgr *snd;
    snd = g->sound;
    fn_40cfb0(snd, "roll6.wav", 1);
    fn_40cfb0(snd, "Click.wav", 1);
    fn_40d058(snd, "roll6.wav", 2);
    fn_40d080(snd, "Click.wav", 8);
}

extern "C" void fn_40f3c0()
{
    TCustomForm_Close(g_460214);
    TApplication_Terminate(*g_45fee4);
}

extern "C" void fn_40f3dc()
{
    HDC dc;
    int bpp;
    int planes;
    int bits;
    dc = CreateICA("DISPLAY", 0, 0, 0);
    bpp = GetDeviceCaps(dc, BITSPIXEL);
    planes = GetDeviceCaps(dc, PLANES);
    DeleteDC(dc);
    bits = bpp * planes;
    if (bits < 8)
        fn_401f0c("Your Desktop must be set to 256 (8 bpp)\n or more colors to enjoy this program.");
    fn_40ec34(g_4601c0, fn_42fdd4(*g_45fee8), fn_42fdcc(*g_45fee8));
}

extern "C" void fn_40f460(void *mainForm)
{
    g_460214 = mainForm;
    fn_40f3dc();
    fn_402224();
    srand(time(0));
    TWebTrack *wt = new TWebTrack("http://www.nstorm.com", "gamehits/elfbowl/elfmain.html", 1);
    g_460218 = new TPackedResources(g_4601c0, "PackedFile");
    g_game = new TStage((TStageForm *)g_4601c0, 8, 0, 1, fn_40f3c0, wt);
    fn_40f350(g_game);
    fn_40f364(g_game);
    fn_41e22c(g_game);
    fn_41e01c(g_game);
    fn_41c674(g_game);
    fn_40f934(g_game);
    fn_411af4(g_game);
    g_frame = 0;
    g_cheated = false;
    fn_402224();
    fn_40bfe4(g_game, "PreIntro");
}
