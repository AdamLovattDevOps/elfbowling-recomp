// Easter egg ("ElfCrew") unit 0x40fa1c-0x40ffff. Functions here are not
// 4-byte aligned in the original.
#include <string.h>
#include <elf/funcs.h>

// Alias of fn_407a44: the velocity is passed as a TPoint by value (the
// register rotation differs from the two-int spelling in funcs.h).
extern "C" void fn_407a44_pt(TGraphicSprite *s, TPoint v, int a, int b, int c);
// -a1 after the headers: it packs the header structs too (their ELF_CHECKs fail).
#pragma option -a1

extern "C" void fn_40fa1c(TScene *s, TGraphicSprite *unused)
{
    fn_40fbfb(s);
}

extern "C" void fn_40fa2a(TScene *s, TGraphicSprite *spr)
{
    fn_4078a8(spr);
    fn_40fd94(s, g_456f9c, 7);
    fn_40cb34(s->stage->sound, s, spr, "light.wav", 5, 0);
    fn_4077c4(spr, 0, 9000, fn_40fa1c);
}

extern "C" void fn_40fa87(TScene *s, TGraphicSprite *spr)
{
    fn_4078a8(spr);
    fn_40fd94(s, g_456f78, 9);
    fn_40cb34(s->stage->sound, s, spr, "light.wav", 5, 0);
    fn_4077c4(spr, 0, 9000, fn_40fa2a);
}

extern "C" void fn_40fae4(TScene *s, TGraphicSprite *spr)
{
    fn_4078a8(spr);
    fn_40fd94(s, g_456f5c, 7);
    fn_40cb34(s->stage->sound, s, spr, "light.wav", 5, 0);
    fn_4077c4(spr, 0, 9000, fn_40fa87);
}

extern "C" void fn_40fb41(TScene *s, TGraphicSprite *spr)
{
    fn_4078a8(spr);
    fn_40fd94(s, g_456f3c, 8);
    fn_40cb34(s->stage->sound, s, spr, "light.wav", 5, 0);
    fn_4077c4(spr, 0, 9000, fn_40fae4);
}

extern "C" void fn_40fb9e(TScene *s, TGraphicSprite *spr)
{
    fn_4078a8(spr);
    fn_40fd94(s, g_456f18, 9);
    fn_40cb34(s->stage->sound, s, spr, "light.wav", 5, 0);
    fn_4077c4(spr, 0, 9000, fn_40fb41);
}

extern "C" void fn_40fbfb(TScene *s)
{
    fn_40feb0(s);
}

extern "C" void fn_40fc09(TScene *s)
{
    g_eggText = (char *)fn_402338(0x4b0, "InitText");
    fn_40a348(s, "ElfCrew", "EggText");
}

extern "C" void fn_40fc3c(TScene *s, int first, int unused, const char **lines, int n)
{
    TGraphicSprite *txt;
    char *buf;
    int i;
    char *end;
    bool hdr;
    const char *line;
    char *at;
    char *star;
    int style;
    txt = fn_40a598(s, "EggText");
    fn_405954((TTextCast *)txt->casts[txt->frame]);
    buf = g_eggText;
    for (i = first; i < n; i++) {
        end = buf;
        if (i >= 0) {
            line = lines[i];
            hdr = *line == '#';
            if (hdr)
                line++;
            strcpy(buf, line);
            at = strchr(buf, '@');
            if (at)
                *at = '&';
            star = strchr(buf, '*');
            while (star) {
                *star = '"';
                star = strchr(star, '*');
            }
            end += strlen(buf);
        }
        *end = 0;
        style = hdr ? 0 : 3;
        fn_405978((TTextCast *)txt->casts[txt->frame], style);
        if (i == first)
            fn_406a08(txt, buf);
        else
            fn_406a44(txt, buf);
        buf = end;
    }
}

extern "C" void fn_40fd94(TScene *s, const char **lines, int n)
{
    TGraphicSprite *txt;
    TGraphicSprite *crew;
    TPoint p;
    int h;
    TRect r1;
    TRect r2;
    txt = fn_40a598(s, "EggText");
    fn_405978((TTextCast *)txt->casts[txt->frame], 3);
    crew = fn_40a598(s, "ElfCrew");
    fn_408908(txt);
    fn_406c00(txt);
    p = Classes_Point(txt->x / 1000, txt->y / 1000);
    p.y -= 60;
    fn_406a80(txt, p);
    fn_407a98(crew, (RECT *)&r1);
    fn_407a98(txt, (RECT *)&r2);
    h = r1.bottom - r1.top;
    r1.top += r1.bottom - r1.top + 10;
    r1.bottom = r1.top + h - 54;
    r1.left = 0;
    r1.right = 640;
    fn_407a68(txt, r1);
    fn_40fc3c(s, 0, 0, lines, n);
}

extern "C" void fn_40feb0(TScene *s)
{
    TGraphicSprite *txt;
    TGraphicSprite *crew;
    TPoint p;
    int h;
    txt = fn_40a598(s, "EggText");
    fn_405978((TTextCast *)txt->casts[txt->frame], 3);
    crew = fn_40a598(s, "ElfCrew");
    fn_408908(txt);
    fn_406c00(txt);
    p = Classes_Point(txt->x / 1000, txt->y / 1000);
    fn_406a08(txt, " ");
    g_eggLineH = ((TTextCast *)txt->casts[txt->frame])->lineHeight;
    TRect r1;
    TRect r2;
    fn_407a98(crew, (RECT *)&r1);
    fn_407a98(txt, (RECT *)&r2);
    h = r1.bottom - r1.top;
    p.y += h - 100;
    fn_406a80(txt, p);
    r1.top += r1.bottom - r1.top + 10;
    r1.bottom = r1.top + h - 54;
    r1.left = 0;
    r1.right = 640;
    fn_407a68(txt, r1);
    g_eggLen = g_eggLineH * 12 + (r1.bottom - r1.top);
    fn_40fc3c(s, 0, 0, g_456ee8, 12);
    fn_407a44_pt(txt, Classes_Point(0, -1), 0x50, 0, 0);
    TRect r3 = Classes::Rect(p.x - 1, p.y - g_eggLen - g_eggLineH - 20, p.x + 2, p.y + 1000);
    txt->clip = r3;
    txt->onLeave = fn_40fb9e;
}
