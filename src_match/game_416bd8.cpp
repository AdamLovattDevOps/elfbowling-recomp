// Game unit: ball position -> lane marker (0x416bd8).
#include <elf/funcs.h>

// Alias of fn_401d0c: this unit passes three TPoints by value (the register
// rotation differs from the six-int spelling in funcs.h).
extern "C" int fn_401d0c_pt(TPoint p, TPoint a, TPoint b);

extern "C" bool fn_416bd8(TScene *self, TPoint p)
{
    int ret = 0;
    int t = (p.y - 266) * 100 / -54;
    int left = fn_401d0c_pt(p, Classes::Point(24, 480), Classes::Point(130, 220));
    int right = fn_401d0c_pt(p, Classes::Point(296, 480), Classes::Point(190, 220));
    int pct = (p.x - left) * 100 / (right - left);
    TPoint q;
    q.y = t * -362 / 100 + 530;
    int top = fn_401d0c_pt(q, Classes::Point(232, 480), Classes::Point(384, 195));
    int bottom = fn_401d0c_pt(q, Classes::Point(728, 480), Classes::Point(576, 195));
    if (g_gutterLeft)
        q.x = fn_401d0c_pt(q, Classes::Point(170, 480), Classes::Point(356, 195));
    else if (g_gutterRight)
        q.x = fn_401d0c_pt(q, Classes::Point(790, 480), Classes::Point(604, 195));
    else
        q.x = (bottom - top) * pct / 100 + top;
    int idx = 4;
    for (int i = 0; i < 4; i++) {
        if (g_ballRowY[i] >= q.y) {
            idx = i;
            break;
        }
    }
    if (idx != g_ballRow) {
        ret = 1;
        fn_414e64(self, idx);
        g_ballRow = idx;
    }
    int frame = 0;
    if (t >= 75)
        frame = 3;
    else if (t >= 50)
        frame = 2;
    else if (t >= 25)
        frame = 1;
    fn_40971c(g_460568);
    TGraphicSprite *s = g_460568->items[idx];
    fn_408908(s);
    fn_406c50(s, frame);
    fn_406a80(s, q);
    return ret != 0;
}
