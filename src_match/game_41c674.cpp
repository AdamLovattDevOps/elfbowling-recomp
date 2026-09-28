// Game unit tail (scene factory, scroll text) at 0x41c674. Layouts shared with game_414240.cpp.
#include <stdlib.h>
#include <string.h>
#include <elf/funcs.h>

// Alias of fn_407a44: the velocity is passed as a TPoint by value (see
// game_416f10.cpp).
extern "C" void fn_407a44_pt(TGraphicSprite *s, TPoint v, int a, int b, int c);

extern "C" void fn_41c674(TStage *self)
{
    TScene *s = new TScene("Game", 100, fn_418274, fn_4181a8);
    fn_40ae30(self, s);
    fn_409bb8(s, self);
}

extern "C" void fn_41c714(TScene *self, TGraphicSprite *arg)
{
    fn_4077c4(arg, 0, 1200, fn_41ca30);
}

extern "C" void fn_41c730(TScene *self)
{
    TGraphicSprite *s = fn_40a598(self, "ScrollText");
    fn_405978((TTextCast *)s->casts[s->frame], 4);
    TGraphicSprite *sc = fn_40a598(self, "Scroll");
    fn_408908(s);
    fn_406c00(s);
    TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
    fn_406a08(s, " ");
    g_scrollLineH = ((TTextCast *)s->casts[s->frame])->lineHeight;
    TRect r1;
    fn_407a98(sc, (RECT *)&r1);
    TRect r2;
    fn_407a98(s, (RECT *)&r2);
    g_46078c = 1 - (r1.Bottom - r1.Top) / g_scrollLineH;
    pt.y += (r2.Bottom - r2.Top) / 2 + (r1.Bottom - r1.Top) / 2;
    fn_406a80(s, pt);
    r1.Top += 10;
    r1.Bottom -= 18;
    fn_407a68(s, r1);
    g_scrollLen = g_scrollLineH * 29 + (r1.Bottom - r1.Top);
    fn_41ccb4(self, 0, 0);
    fn_407a44_pt(s, Classes::Point(0, -1), 70, 0, 0);
    s->clip = Classes::Rect(pt.x - 1, pt.y - g_scrollLen - g_scrollLineH, pt.x + 2, pt.y + 1000);
    s->onLeave = fn_41c714;
}

extern "C" void fn_41c906(TScene *self)
{
    g_scrollText = (char *)fn_402338(0xb54, "InitText");
    fn_40a348(self, "Scroll", "ScrollText");
    fn_41c730(self);
}

extern "C" void fn_41c940(TScene *self, void *sender, unsigned short &key, ShiftState shift)
{
    if (key == 27) {
        fn_40c130(g_game);
        return;
    }
    if (key == 13) {
        TGraphicSprite *s = fn_40a598(self, "NStorm");
        fn_41ca40(self, s);
    }
}

extern "C" void fn_41c98c(TScene *self, TGraphicSprite *unused)
{
    fn_40c130(self->stage);
}

extern "C" void fn_41c9a0(TScene *self, TGraphicSprite *unused)
{
    fn_40bf2c(self->stage, "Game");
}

extern "C" void fn_41c9bc(TScene *self, TGraphicSprite *unused)
{
    fn_41cf48(self);
}

extern "C" void fn_41c9cc(TScene *self, TGraphicSprite *unused)
{
    fn_40bf2c(self->stage, "Game");
}

extern "C" void fn_41c9e8(TScene *self, TGraphicSprite *arg)
{
    for (int i = 0; i < self->nsprites; i++)
        fn_408924(self->sprites[i]);
    fn_4077c4(arg, 0, 500, fn_41c9cc);
}

extern "C" void fn_41ca30(TScene *self, TGraphicSprite *s)
{
    fn_41c730(self);
}

extern "C" void fn_41ca40(TScene *self, TGraphicSprite *s)
{
    fn_407834(s);
    fn_40cb34(self->stage->sound, self, s, "Click.wav", 3, 0);
    fn_41c9e8(self, s);
}

extern "C" void fn_41ca84(TScene *self, TGraphicSprite *s)
{
    TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
    TRect r;
    r = s->clip;
    if (pt.y >= r.Bottom) {
        fn_4060d4(s, Classes::Point(rand() % 640, -(rand() % 480)));
        fn_406c00(s);
    }
    if (pt.x >= 588)
        fn_407fdc(s, rand() % 2 + 2, fn_41ca84);
    else if (pt.x <= 52)
        fn_407fdc(s, rand() % 2, fn_41ca84);
    else
        fn_407fdc(s, rand() % 4, fn_41ca84);
}

extern "C" void fn_41cb9c(TScene *self)
{
    for (int i = 0; i < g_460778->count; i++) {
        TGraphicSprite *s = g_460778->items[i];
        fn_406c00(s);
        fn_4077c4(s, 0, rand() % 100, fn_41ca84);
    }
}

extern "C" void fn_41cbf8(TScene *self, int start, int unused)
{
    char *p = g_scrollText;
    for (int i = start; i < 26; i++) {
        if (i >= 0) {
            strcpy(p, g_45ab68[i]);
            char *q = strchr(p, '*');
            if (q)
                *q = '"';
            p += strlen(p);
        }
        if (i < 25)
            *p++ = '\r';
    }
    *p = 0;
    TGraphicSprite *s = fn_40a598(self, "ScrollText");
    fn_405954((TTextCast *)s->casts[s->frame]);
    fn_406a08(s, g_scrollText);
}

extern "C" void fn_41ccb4(TScene *self, int start, int unused)
{
    char *p = g_scrollText;
    for (int i = start; i < 29; i++) {
        if (i >= 0) {
            strcpy(p, g_45aaf4[i]);
            char *q = strchr(p, '*');
            if (q)
                *q = '"';
            p += strlen(p);
        }
        if (i < 28)
            *p++ = '\r';
    }
    *p = 0;
    TGraphicSprite *s = fn_40a598(self, "ScrollText");
    fn_405954((TTextCast *)s->casts[s->frame]);
    fn_406a08(s, g_scrollText);
}

extern "C" void fn_41cd70(TScene *self)
{
    TGraphicSprite *s = fn_40a598(self, "ScrollText");
    fn_405978((TTextCast *)s->casts[s->frame], 2);
    TGraphicSprite *sc = fn_40a598(self, "Scroll");
    fn_408908(s);
    fn_406c00(s);
    TPoint pt = Classes::Point(s->x / 1000, s->y / 1000);
    fn_406a08(s, " ");
    g_scrollLineH = ((TTextCast *)s->casts[s->frame])->lineHeight;
    TRect r1;
    fn_407a98(sc, (RECT *)&r1);
    TRect r2;
    fn_407a98(s, (RECT *)&r2);
    g_46078c = 1 - (r1.Bottom - r1.Top) / g_scrollLineH;
    pt.y += (r2.Bottom - r2.Top) / 2 + (r1.Bottom - r1.Top) / 2;
    fn_406a80(s, pt);
    r1.Top += 10;
    r1.Bottom -= 18;
    fn_407a68(s, r1);
    g_scrollLen = g_scrollLineH * 26 + (r1.Bottom - r1.Top);
    fn_41cbf8(self, 0, 0);
    fn_407a44_pt(s, Classes::Point(0, -1), 120, 0, 0);
    s->clip = Classes::Rect(pt.x - 1, pt.y - g_scrollLen - g_scrollLineH, pt.x + 2, pt.y + 1000);
    s->onLeave = fn_41c714;
}

extern "C" void fn_41cf48(TScene *self)
{
    fn_41cd70(self);
}

extern "C" void fn_41cf58(TScene *self, TGraphicSprite *s)
{
    int f = s->frame;
    f = (f + 1) % 2;
    fn_406c50(s, f);
    fn_4077c4(s, 0, rand() % 200 + 400, fn_41cf58);
}

extern "C" void fn_41cfb0(TScene *self)
{
    for (int i = 0; i < g_46077c->count; i++) {
        TGraphicSprite *s = g_46077c->items[i];
        fn_4077c4(s, 0, rand() % 600, fn_41cf58);
    }
}

extern "C" void fn_41d004(TScene *self, TGraphicSprite *arg)
{
    fn_406c50(arg, 0);
}

extern "C" void fn_41d018(TScene *self, TGraphicSprite *arg)
{
    fn_40779c(arg);
    fn_4077c4(arg, 0, 200, fn_41d004);
}

extern "C" void fn_41d040(TScene *self, TGraphicSprite *arg)
{
    fn_406c50(arg, 1);
    fn_4077c4(arg, 0, 300, fn_41d018);
}

extern "C" void fn_41d06c(TScene *self, TGraphicSprite *arg)
{
    fn_407fdc(arg, 0, fn_41d06c);
    arg = fn_40a598(self, "Elf00");
    fn_407774(arg, 2000);
    fn_4077c4(arg, 0, 200, fn_41d040);
}

extern "C" void fn_41d0c0(TScene *self)
{
    fn_41cb9c(self);
    fn_41c906(self);
    fn_41cfb0(self);
    TGraphicSprite *s = fn_40a598(self, "Elf01 Arms");
    fn_407858(s, 450, 0, 2);
    s = fn_40a598(self, "Circle");
    TRect r;
    fn_407a98(s, (RECT *)&r);
    s = fn_40a598(self, "SantaWalkBack");
    fn_407a68(s, r);
    s = fn_40a598(self, "Elf00 Arms");
    fn_407fdc(s, 0, fn_41d06c);
}
