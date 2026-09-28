// Game unit: pin knock-down simulation (0x41665c).
#include <stdlib.h>
#include <string.h>
#include <elf/funcs.h>

extern "C" int fn_41665c(TScene *self)
{
    bool flip;
    int row;
    int i;
    int r;
    bool knocked;
    int *tbl;
    int pass;
    int before;
    int pin;
    int mask;
    int bit;
    int j;
    bool blocked;
    int bit2;
    int k;

    g_pinsDown = 0;
    flip = false;
    for (i = 0; i < 10; i++)
        g_knocked[i] = 0;
    if (g_aimCell > 14) {
        row = 28 - g_aimCell - 9;
        flip = true;
    } else
        row = g_aimCell - 9;
    r = rand() % 3;
    memmove(g_460582, g_present, 10);
    fn_416570((char *)g_460582);
    if (row >= 0) {
        tbl = g_458884[row][r];
        g_4605b8 = g_458e24[row][r];
        g_4605b0 = g_458b54[row][r];
        g_4605b4 = flip;
        for (pass = 0; pass < 3; pass++) {
            fn_401cf4("Pass %d:\n", pass);
            before = g_pinsDown;
            for (pin = 9; pin >= 0; pin--) {
                if (g_460582[flip ? g_458840[pin] : pin]) {
                    mask = tbl[pin];
                    knocked = false;
                    if (mask & 0x400) {
                        fn_401cf4("  Pin %d knocked down by ball\n", flip ? g_458840[pin] : pin);
                        knocked = true;
                    } else {
                        for (bit = 1, j = 0; j < 10 && !knocked; bit <<= 1, j++) {
                            if (g_present[flip ? g_458840[j] : j] && !g_460582[flip ? g_458840[j] : j] && (mask & bit)) {
                                blocked = false;
                                for (bit2 = 0x10000, k = 0; k < 10 && !knocked; bit2 <<= 1, k++) {
                                    if (g_460582[flip ? g_458840[k] : k] && !blocked && (mask & bit2)) {
                                        blocked = true;
                                        fn_401cf4("  Pin %d blocked from Pin %d by Pin %d\n",
                                                  flip ? g_458840[pin] : pin, flip ? g_458840[j] : j,
                                                  flip ? g_458840[k] : k);
                                    }
                                }
                                if (!blocked) {
                                    knocked = true;
                                    fn_401cf4("  Pin %d knocked down by Pin %d\n",
                                              flip ? g_458840[pin] : pin, flip ? g_458840[j] : j);
                                }
                            }
                        }
                    }
                    if (knocked) {
                        g_460582[flip ? g_458840[pin] : pin] = 0;
                        g_pinsDown++;
                    }
                }
            }
            if (g_pinsDown == before)
                break;
        }
    }
    fn_401cf4("%d Pins were knocked down\n", g_pinsDown);
    fn_416570((char *)g_460582);
    return g_pinsDown;
}
