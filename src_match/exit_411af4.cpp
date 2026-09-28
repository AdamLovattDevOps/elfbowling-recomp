// NON-MATCHING: fn_411af4: code bytes IDENTICAL (126 bytes); funcs.tsv end 0x411b80 includes exception table at 0x411b74. Move back once funcs.tsv ends it at 0x411b74.

// ---- Exit / score screen helpers 0x411af4-0x4120cc ----
#include <stdlib.h>
#include <elf/funcs.h>

extern "C" void fn_411af4(TStage *g)
{
    TScene *s = new TScene("Exit", 100, fn_4112b8, fn_411218);
    fn_40ae30(g, s);
    fn_409bb8(s, g);
}
