// Main menu unit 0x41008c-...
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <elf/funcs.h>

extern "C" void fn_41008c(TScene *s, int a, unsigned short *key)
{
    if (*key == 0x1b) {
        if (g_460234) {
            fn_411194(s);
            return;
        }
        fn_40c130(g_456c38);
        return;
    }
    if (*key == 0xd) {
        if (g_460234)
            fn_411194(s);
        fn_40c130(g_456c38);
    }
}

extern "C" LONG fn_4100e4(HKEY root, const char *sub, char *out)
{
    LONG r;
    HKEY key;
    LONG size;
    char buf[260];
    r = RegOpenKeyExA(root, sub, 0, KEY_QUERY_VALUE, &key);
    if (r == 0) {
        size = 260;
        RegQueryValueA(key, 0, buf, &size);
        lstrcpyA(out, buf);
        RegCloseKey(key);
    }
    return r;
}

extern "C" bool fn_410144(const char *url)
{
    int ok;
    int show;
    char *p;
    char cmd[520];
    ok = 0;
    show = SW_SHOW;
    if ((intptr_t)ShellExecuteA(0, "open", url, 0, 0, show) <= 32) {
        if (fn_4100e4(HKEY_CLASSES_ROOT, ".htm", cmd) == 0) {
            lstrcatA(cmd, "\\shell\\open\\command");
            if (fn_4100e4(HKEY_CLASSES_ROOT, cmd, cmd) == 0) {
                p = strstr(cmd, "\"%1\"");
                if (p == 0) {
                    p = strstr(cmd, "%1");
                    if (p == 0)
                        p = cmd + lstrlenA(cmd) - 1;
                    else
                        *p = 0;
                } else {
                    *p = 0;
                }
                lstrcatA(p, " ");
                lstrcatA(p, url);
                if (WinExec(cmd, show) > 31)
                    ok = 1;
            }
        }
    } else {
        ok = 1;
    }
    return ok != 0;
}

extern "C" void fn_410278(TScene *s)
{
    TGraphicSprite *e;
    if (!g_460234) {
        if (fn_4024bc(0) >= g_460238 + 3000) {
            fn_410a40(s);
            e = fn_40a598(s, "Elfx02");
            fn_40d100(s->stage->sound, s, e, "ClickMe.wav", 3, 100, 0);
        }
    }
}

extern "C" void fn_4102e4(TScene *s)
{
    TGraphicSprite *e;
    if (!g_460234) {
        if (fn_4024bc(0) >= g_460238 + 3000) {
            fn_410a40(s);
            e = fn_40a598(s, "Elfx00");
            fn_40d100(s->stage->sound, s, e, "ClickMe.wav", 3, 100, 0);
        }
    }
}

extern "C" void fn_410350(TScene *s)
{
    TGraphicSprite *e;
    if (!g_460234) {
        if (fn_4024bc(0) >= g_460238 + 3000) {
            fn_410a40(s);
            e = fn_40a598(s, "Elfx01");
            fn_40d100(s->stage->sound, s, e, "ClickMe.wav", 3, 100, 0);
        }
    }
}

extern "C" void fn_4103bc(TScene *s, TGraphicSprite *k)
{
    if (!g_460234) {
        if (k->enabled && k->shown)
            fn_40bf2c(s->stage, "Game");
    } else {
        fn_411194(s);
    }
}

extern "C" void fn_410400(TScene *s, TGraphicSprite *k)
{
    if (!g_460234) {
        if (k->enabled && k->shown)
            fn_40bf84(g_456c38, "About");
    } else {
        fn_411194(s);
    }
}

extern "C" void fn_410440(TScene *s, TGraphicSprite *k)
{
    if (!g_460234) {
        if (k->enabled && k->shown)
            fn_40c130(g_456c38);
    } else {
        fn_411194(s);
    }
}

extern "C" void fn_41047c(TScene *s, TGraphicSprite *k)
{
    int key;
    char url[300];
    if (!g_460234) {
        if (k->enabled && k->shown) {
            key = g_46059c * g_46059c * 3 + g_46059c * 17 + g_4605a8 * 73;
            key = key % 1000000;
            strcpy(url, "http://www.nstorm.com/scores/setscore.cfm");
            strcat(url, "?game=");
            strcat(url, "elfbowl");
            strcat(url, "&score=");
            fn_401e48(url + strlen(url), g_46059c, 9, 1);
            strcat(url, "&key1=");
            fn_401e48(url + strlen(url), g_4605a8, 9, 1);
            strcat(url, "&key2=");
            if (g_4605a0)
                strcat(url, "-1");
            else
                fn_401e48(url + strlen(url), key, 9, 1);
            fn_410144(url);
        }
    } else {
        fn_411194(s);
    }
}

extern "C" void fn_410610(TScene *s, TGraphicSprite *k)
{
    if (!g_460234) {
        if (k->enabled && k->shown)
            fn_410144("http://www.nstorm.com");
    } else {
        fn_411194(s);
    }
}

extern "C" void fn_410648(TScene *s, TGraphicSprite *k)
{
    if (!g_460234) {
        if (k->enabled && k->shown)
            fn_410144("http://www.theplanet.com");
    } else {
        fn_411194(s);
    }
}

extern "C" void fn_410680(TScene *s)
{
    fn_4111dc(s);
}

extern "C" void fn_410690(TScene *s, TGraphicSprite *k)
{
    if (!g_460234) {
        if (k->enabled && k->shown) {
            fn_40cb34(s->stage->sound, s, k, "Click.wav", 5, 0);
            fn_4077c4(k, 0, 200, (SpriteCb)fn_410680);
        }
    } else {
        fn_411194(s);
    }
}

extern "C" void fn_4106fc(TScene *s)
{
    fn_411194(s);
}

extern "C" void fn_41070c(TScene *s)
{
    TGraphicSprite *e;
    e = fn_40a598(s, "ElfCrew");
    fn_40cb34(s->stage->sound, s, e, "Click.wav", 5, 0);
    fn_4077c4(e, 0, 200, (SpriteCb)fn_4106fc);
}

// ---- ElfCrew easter egg / elves on the menu ----

extern "C" void fn_410764(TScene *s)
{
    TGraphicSprite *crew;
    TPoint pt;
    bool hit;
    int i;
    crew = fn_40a598(s, "ElfCrew");
    pt = crew->stage->up;
    TRect r;
    fn_407a98(crew, (RECT *)&r);
    pt.x -= r.left;
    pt.y -= r.top;
    hit = false;
    for (i = 0; i < 10; i++) {
        int dx = abs(pt.x - g_456fb8[i]);
        int dy = abs(pt.y - g_456fe0[i]);
        if (dx <= 15 && dy <= 20) {
            TGraphicSprite *txt = fn_40a598(s, "EggText");
            int which = i / 2;
            switch (which) {
            case 0: fn_40fb9e(s, txt); break;
            case 1: fn_40fb41(s, txt); break;
            case 2: fn_40fae4(s, txt); break;
            case 3: fn_40fa87(s, txt); break;
            case 4: fn_40fa2a(s, txt); break;
            }
            hit = true;
        }
    }
    if (!hit)
        fn_41070c(s);
}

extern "C" void fn_4108bc(TScene *s, TGraphicSprite *spr)
{
    int f;
    f = spr->frame;
    f = (f + 1) % 2;
    fn_406c50(spr, f);
    fn_4077c4(spr, 0, rand() % 200 + 400, (SpriteCb)fn_4108bc);
}

extern "C" void fn_410914(TScene *s)
{
    int i;
    TGraphicSprite *e;
    for (i = 0; i < g_46023c->count; i++) {
        e = g_46023c->items[i];
        fn_4077c4(e, 0, rand() % 600, (SpriteCb)fn_4108bc);
    }
}

extern "C" void fn_410968(TScene *s, TGraphicSprite *spr)
{
    fn_40779c(spr);
}

extern "C" void fn_410978(TScene *s, TGraphicSprite *spr)
{
    spr = fn_40a598(s, "Elfx00");
    fn_4077c4(spr, 0, 8000, (SpriteCb)fn_410978);
    fn_40d100(s->stage->sound, s, spr, "ElfBaby.wav", 4, 100, 0);
    spr = fn_40a598(s, "Elfx01");
    fn_407774(spr, 0);
    fn_4077c4(spr, 0, 1000, (SpriteCb)fn_410968);
    spr = fn_40a598(s, "Elfx02");
    fn_407774(spr, 0);
    fn_4077c4(spr, 0, 1000, (SpriteCb)fn_410968);
}

extern "C" void fn_410a40(TScene *s)
{
    TGraphicSprite *e;
    e = fn_40a598(s, "Elfx00");
    fn_4077c4(e, 0, 8000, (SpriteCb)fn_410978);
}

extern "C" void fn_410a74(TScene *s)
{
    TGraphicSprite *e;
    e = fn_40a598(s, "Elfx00");
    fn_4077c4(e, 0, 1400, (SpriteCb)fn_410978);
    e = fn_40a598(s, "Elfx01 Body");
    fn_407858(e, 300, 0, 4);
    e = fn_40a598(s, "Elfx01 Arms");
    fn_407858(e, 300, 0, 2);
    e = fn_40a598(s, "Elfx00 Body");
    fn_407858(e, 300, 0, 4);
    e = fn_40a598(s, "Elfx00 Arms");
    fn_407858(e, 300, 0, 2);
    e = fn_40a598(s, "Elfx02 Body");
    fn_407858(e, 300, 0, 4);
    e = fn_40a598(s, "Elfx02 Arms");
    fn_407858(e, 300, 0, 2);
}

extern "C" void fn_410b90(TScene *s)
{
    TGraphicSprite *e;
    e = fn_40a598(s, "ElfCrew");
    fn_408924(e);
    e = fn_40a598(s, "EggText");
    fn_408924(e);
}

extern "C" void fn_410bd0(TScene *s)
{
    TGraphicSprite *e;
    e = fn_40a598(s, "ElfCrew");
    fn_408908(e);
    e = fn_40a598(s, "EggText");
    fn_408908(e);
    fn_40fbfb(s);
}

// Hide the main-menu sprites.
extern "C" void fn_410c18(TScene *s)
{
    TGraphicSprite *e;
    TGraphicSprite *b;
    e = fn_40a598(s, "BLogo2");
    fn_408924(e);
    b = fn_40a598(s, "ExitPlay");
    fn_408924(b);
    b = fn_40a598(s, "ExitRules");
    fn_408924(b);
    b = fn_40a598(s, "ExitQuit");
    fn_408924(b);
    e = fn_40a598(s, "xMas");
    fn_408924(e);
    b = fn_40a598(s, "NStormCom");
    fn_408924(b);
    b = fn_40a598(s, "Planet");
    fn_408924(b);
    e = fn_40a598(s, "MouseElf1");
    fn_408924(e);
    e = fn_40a598(s, "MouseElf2");
    fn_408924(e);
    e = fn_40a598(s, "MouseElf3");
    fn_408924(e);
    e = fn_40a598(s, "Elfx00");
    fn_408924(e);
    e = fn_40a598(s, "Elfx00 Body");
    fn_408924(e);
    e = fn_40a598(s, "Elfx00 Arms");
    fn_408924(e);
    e = fn_40a598(s, "Elfx01");
    fn_408924(e);
    e = fn_40a598(s, "Elfx01 Body");
    fn_408924(e);
    e = fn_40a598(s, "Elfx01 Arms");
    fn_408924(e);
    e = fn_40a598(s, "Elfx02");
    fn_408924(e);
    e = fn_40a598(s, "Elfx02 Body");
    fn_408924(e);
    e = fn_40a598(s, "Elfx02 Arms");
    fn_408924(e);
    e = fn_40a598(s, "BigRedButton");
    fn_408924(e);
    e = fn_40a598(s, "DaScore");
    fn_408924(e);
    e = fn_40a598(s, "ScorePadTextr2");
    fn_408924(e);
}

// Show the main-menu sprites; the three buttons flash with a sound.
extern "C" void fn_410e8c(TScene *s)
{
    TGraphicSprite *e;
    TGraphicSprite *b;
    e = fn_40a598(s, "BLogo2");
    fn_408908(e);
    b = fn_40a598(s, "ExitPlay");
    fn_408908(b);
    b = fn_40a598(s, "ExitRules");
    fn_408908(b);
    b = fn_40a598(s, "ExitQuit");
    fn_408908(b);
    e = fn_40a598(s, "xMas");
    fn_408908(e);
    e = fn_40a598(s, "MouseElf1");
    fn_408908(e);
    e = fn_40a598(s, "MouseElf2");
    fn_408908(e);
    e = fn_40a598(s, "MouseElf3");
    fn_408908(e);
    e = fn_40a598(s, "Elfx00");
    fn_408908(e);
    e = fn_40a598(s, "Elfx00 Body");
    fn_408908(e);
    e = fn_40a598(s, "Elfx00 Arms");
    fn_408908(e);
    e = fn_40a598(s, "Elfx01");
    fn_408908(e);
    e = fn_40a598(s, "Elfx01 Body");
    fn_408908(e);
    e = fn_40a598(s, "Elfx01 Arms");
    fn_408908(e);
    e = fn_40a598(s, "Elfx02");
    fn_408908(e);
    e = fn_40a598(s, "Elfx02 Body");
    fn_408908(e);
    e = fn_40a598(s, "Elfx02 Arms");
    fn_408908(e);
    e = fn_40a598(s, "DaScore");
    fn_408908(e);
    e = fn_40a598(s, "ScorePadTextr2");
    fn_408908(e);
    fn_410a74(s);
    b = fn_40a598(s, "BigRedButton");
    fn_408908(b);
    fn_409000((TButtonSprite *)b, 3, 1500, 300, 2500);
    fn_4090b4((TButtonSprite *)b, "light.wav", 1);
    b = fn_40a598(s, "NStormCom");
    fn_408908(b);
    fn_409000((TButtonSprite *)b, 3, 1500, 300, 2500);
    fn_4090b4((TButtonSprite *)b, "light.wav", 1);
    b = fn_40a598(s, "Planet");
    fn_408908(b);
    fn_409000((TButtonSprite *)b, 3, 1500, 300, 2500);
    fn_4090b4((TButtonSprite *)b, "light.wav", 1);
}

extern "C" void fn_411194(TScene *s)
{
    s->f1c = 0;
    g_460234 = false;
    fn_4099dc(s);
    fn_410b90(s);
    fn_410e8c(s);
    fn_410914(s);
    g_460238 = fn_4024bc(0);
}

extern "C" void fn_4111dc(TScene *s)
{
    s->f1c = (SceneCb)fn_410764;
    g_460234 = true;
    fn_4099dc(s);
    fn_410c18(s);
    fn_410bd0(s);
    fn_410914(s);
}

extern "C" void fn_411218(TScene *s)
{
    TGraphicSprite *t;
    char buf[20];
    TRect r;
    fn_401e48(buf, g_46059c, 3, 0);
    t = fn_40a598(s, "ScorePadTextr2");
    fn_405954((TTextCast *)t->casts[t->frame]);
    fn_406a08(t, buf);
    fn_40fc09(s);
    fn_406c00(t);
    fn_407a98(t, (RECT *)&r);
    fn_407a68(t, r);
    fn_411194(s);
}

// Build the main-menu scene (once).
extern "C" void fn_4112b8(TScene *s, char loaded)
{
    TSoundMgr *snd;
    TGraphicSprite *spr;
    TGraphicSprite *txt;
    TButtonSprite *btn;
    snd = s->stage->sound;
    fn_40cfb0(snd, "light.wav", 0);
    fn_40cfb0(snd, "ElfBaby.wav", 0);
    fn_40cfb0(snd, "ClickMe.wav", 0);
    if (!loaded) {
        fn_409fa8(s, (SceneKeyCb)fn_41008c);
        fn_40a06c(s, "Base00", "MountainBase.bmp");
        fn_40a06c(s, "Base01", "MountainBase.bmp");
        fn_40a06c(s, "Base02", "MountainBase.bmp");
        fn_40a06c(s, "Base03", "MountainBase.bmp");
        fn_40a06c(s, "Base04", "MountainBase.bmp");
        fn_40a06c(s, "Base05", "MountainBase.bmp");
        fn_40a06c(s, "Base06", "MountainBase.bmp");
        fn_40a06c(s, "Base07", "MountainBase.bmp");
        fn_40a06c(s, "Base08", "MountainBase.bmp");
        fn_40a06c(s, "Base09", "MountainBase.bmp");
        fn_40a06c(s, "Base10", "MountainBase.bmp");
        fn_40a06c(s, "Base11", "MountainBase.bmp");
        fn_40a06c(s, "Base12", "MountainBase.bmp");
        fn_40a06c(s, "Base13", "MountainBase.bmp");
        fn_40a06c(s, "Base14", "MountainBase.bmp");
        fn_40a06c(s, "Base15", "MountainBase.bmp");
        fn_40a06c(s, "Base16", "MountainBase.bmp");
        fn_40a06c(s, "Base17", "MountainBase.bmp");
        fn_40a06c(s, "Base18", "MountainBase.bmp");
        fn_40a06c(s, "Base19", "MountainBase.bmp");
        fn_40a06c(s, "MountainsR", "Mountains.bmp");
        fn_40a06c(s, "MountainsC", "Mountains.bmp");
        fn_40a06c(s, "MountainsL", "MountainsRx.bmp");
        btn = fn_40a2c0(s, "MouseElf1", "ElfCover.bmp", "ElfCover.bmp");
        btn->onEnter = (SpriteCb)fn_410278;
        fn_409068(btn, 0, 1);
        btn->onRelease = (SpriteCb)fn_410690;
        btn = fn_40a2c0(s, "MouseElf2", "ElfCover.bmp", "ElfCover.bmp");
        btn->onEnter = (SpriteCb)fn_4102e4;
        fn_409068(btn, 0, 1);
        btn->onRelease = (SpriteCb)fn_410690;
        btn = fn_40a2c0(s, "MouseElf3", "ElfCover.bmp", "ElfCover.bmp");
        btn->onEnter = (SpriteCb)fn_410350;
        fn_409068(btn, 0, 1);
        btn->onRelease = (SpriteCb)fn_410690;
        fn_40a06c(s, "NStorm", "NStormLogo.bmp");
        fn_40a06c(s, "BLogo2", "BowlingLogo.bmp");
        btn = fn_40a2c0(s, "ExitPlay", "PlayOn.bmp", "PlayOff.bmp");
        btn->onRelease = (SpriteCb)fn_4103bc;
        btn = fn_40a2c0(s, "ExitRules", "Who1.bmp", "Who2.bmp");
        btn->onRelease = (SpriteCb)fn_410400;
        btn = fn_40a2c0(s, "ExitQuit", "IntroQuitOff.bmp", "IntroQuitOn.bmp");
        btn->onRelease = (SpriteCb)fn_410440;
        fn_40a06c(s, "xMas", "MerryChristmas.bmp");
        btn = fn_40a2c0(s, "nstormcom", "nstormOff.bmp", "nstormOn.bmp");
        btn->onRelease = (SpriteCb)fn_410610;
        btn = fn_40a2c0(s, "planet", "ThePlanetOff.bmp", "ThePlanetOn.bmp");
        btn->onRelease = (SpriteCb)fn_410648;
        spr = fn_40a06c(s, "Elfx01 Body", "Elf Body0.bmp");
        fn_406724(spr, "ElfBodySideStep.bmp");
        fn_406724(spr, "Elf Body0.bmp");
        fn_406724(spr, "ElfBodySideStepRx.bmp");
        fn_407858(spr, 300, 0, 4);
        spr = fn_40a06c(s, "Elfx01", "Elf0.bmp");
        fn_4067e0(spr, "Elf0 Talk.bmp", 0);
        spr = fn_40a06c(s, "Elfx01 Arms", "ArmsDance1.bmp");
        fn_406724(spr, "ArmsDance1Rx.bmp");
        spr = fn_40a06c(s, "Elfx00 Body", "Elf Body0.bmp");
        fn_406724(spr, "ElfBodySideStep.bmp");
        fn_406724(spr, "Elf Body0.bmp");
        fn_406724(spr, "ElfBodySideStepRx.bmp");
        fn_407858(spr, 300, 0, 4);
        spr = fn_40a06c(s, "Elfx00", "Elf0.bmp");
        fn_4067e0(spr, "Elf0 Talk.bmp", 0);
        spr = fn_40a06c(s, "Elfx00 Arms", "ArmsDance1.bmp");
        fn_406724(spr, "ArmsDance1Rx.bmp");
        spr = fn_40a06c(s, "Elfx02 Body", "Elf Body0.bmp");
        fn_406724(spr, "ElfBodySideStep.bmp");
        fn_406724(spr, "Elf Body0.bmp");
        fn_406724(spr, "ElfBodySideStepRx.bmp");
        fn_407858(spr, 300, 0, 4);
        spr = fn_40a06c(s, "Elfx02", "Elf0.bmp");
        fn_4067e0(spr, "Elf0 Talk.bmp", 0);
        spr = fn_40a06c(s, "Elfx02 Arms", "ArmsDance1.bmp");
        fn_406724(spr, "ArmsDance1Rx.bmp");
        btn = fn_40a2c0(s, "BigRedButton", "ClickHere1.bmp", "ClickHere2.bmp");
        btn->onRelease = (SpriteCb)fn_41047c;
        spr = fn_40a06c(s, "DaScore", "Score.bmp");
        txt = fn_40a3a4(s, "ScorePadTextr2", spr, 2, 3, 3, 20, -175);
        fn_40598c((TTextCast *)txt->casts[txt->frame], 0xffff);
        fn_405978((TTextCast *)txt->casts[txt->frame], 1);
        spr = fn_40a06c(s, "ElfCrew", "ElfCrew75.bmp");
        txt = fn_40a3a4(s, "EggText", spr, 2, -84, -88, 220, -600);
        txt->keepParms = 1;
        fn_40598c((TTextCast *)txt->casts[txt->frame], 0);
        spr = fn_40a06c(s, "Lights00", "LightsOff.bmp");
        fn_406724(spr, "LightsOn.bmp");
        spr = fn_40a06c(s, "Lights02", "LightsOff90.bmp");
        fn_406724(spr, "LightsOn90.bmp");
        spr = fn_40a06c(s, "Lights03", "LightsOff90.bmp");
        fn_406724(spr, "LightsOn90.bmp");
        g_46023c = new TSpriteGroup("ExitLightGroup", s, "Lights??");
    }
    s->stage->bgimage = 0;
}
