// Opening unit at 0x41e01c. Layouts shared with game_*.cpp.
#include <stdlib.h>
#include <elf/funcs.h>

extern "C" void fn_41e01c(TStage *self)
{
    TScene *s = new TScene("Intro", 100, fn_41d184, fn_41d0c0);
    fn_40ae30(self, s);
    fn_409bb8(s, self);
}

extern "C" void fn_41e0bc(TScene *self, void *sender, unsigned short &key, ShiftState shift)
{
    if (key == 27) {
        fn_40c130(g_game);
        return;
    }
    if (key == 13) {
        TGraphicSprite *s = fn_40a598(self, "xNVDLogo");
        fn_41e124(self, s);
    }
}

extern "C" void fn_41e108(TScene *self, TGraphicSprite *unused)
{
    fn_40bf2c(self->stage, "Intro");
}

extern "C" void fn_41e124(TScene *self, TGraphicSprite *s)
{
    fn_407834(s);
    fn_40cb34(self->stage->sound, self, s, "Click.wav", 3, 0);
    fn_4077c4(s, 0, 150, fn_41e108);
}

extern "C" void fn_41e170(TScene *self)
{
    TGraphicSprite *s = fn_40a598(self, "xNVDLogo");
    fn_4077c4(s, 0, 2500, fn_41e108);
}

extern "C" void fn_41e1a4(TScene *self, char loaded)
{
    TSoundMgr *snd = self->stage->sound;
    fn_40cfb0(snd, "Click.wav", 0);
    if (!loaded) {
        fn_409fa8(self, fn_41e0bc);
        fn_40a06c(self, "xNVDMessHdr", "Top3Aggressive.bmp");
        TGraphicSprite *s = fn_40a06c(self, "xNVDLogo", "Top2Logo.bmp");
        s->onRClick = fn_41e124;
        s->clickable = 1;
    }
}

// Preopening unit

extern "C" void fn_41e22c(TStage *self)
{
    TScene *s = new TScene("PreIntro", 100, fn_41e1a4, fn_41e170);
    fn_40ae30(self, s);
    fn_409bb8(s, self);
}
