// About unit 0x40f7fc-0x40fa1c
#include <string.h>
#include <elf/funcs.h>

extern "C" void fn_40f7fc(TScene *s, void *sender, unsigned short &key, ShiftState shift)
{
    if (key == 0x1b) {
        fn_40bdec(s);
        return;
    }
    if (key == 0xd)
        fn_40bdec(s);
}

extern "C" void fn_40f828(TScene *s)
{
}

extern "C" void fn_40f830(TScene *s, TGraphicSprite *k)
{
    if (k->enabled && k->shown) {
        fn_409234((TButtonSprite *)k);
        fn_40bdec(s);
    }
}

extern "C" void fn_40f85c(TScene *s, char again)
{
    TSoundMgr *snd;
    TButtonSprite *b;
    char name[16];
    snd = s->stage->sound;
    fn_40cfb0(snd, "Click.wav", 0);
    if (!again) {
        strcpy(name, "NVDLight00");
        fn_409fa8(s, fn_40f7fc);
        fn_40a06c(s, "NVDMess2", "NVDMess2.bmp");
        fn_40a06c(s, "NVDMessHdr", "Top3Aggressive.bmp");
        fn_40a06c(s, "NVDAddress", "NVDAddress.bmp");
        fn_40a06c(s, "NVDLogo", "Top2Logo.bmp");
        b = fn_40a2c0(s, "ReturnButton", "OtherOK1.bmp", "OtherOK2.bmp");
        b->onRelease = fn_40f830;
    }
}

extern "C" void fn_40f934(TStage *game)
{
    TScene *s = new TScene("About", 30, fn_40f85c, fn_40f828);
    fn_40ae30(game, s);
    fn_409bb8(s, game);
}
