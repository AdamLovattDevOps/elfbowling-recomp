// TSpriteGroup members 0x40971c-0x409844, matched as real C++ members (this
// at [ebp+8], cdecl). As free functions the compiler orders the loads for
// this->arr[i] / i < this->n differently.
#include <elf/funcs.h>

void TSpriteGroup::fn_40971c()
{
    for (int i = 0; i < count; i++) {
        TGraphicSprite *s = items[i];
        fn_408924(s);
    }
}

void TSpriteGroup::fn_409754(int t)
{
    for (int i = 0; i < count; i++) {
        TGraphicSprite *s = items[i];
        fn_406c50(s, t);
    }
}

void TSpriteGroup::fn_409790(int start, int n)
{
    int j = start % count;
    for (int i = 0; i < n; i++) {
        TGraphicSprite *s = items[j];
        fn_408908(s);
        j = (j + 1) % count;
    }
}

void TSpriteGroup::fn_4097e8(int start, int n, int t)
{
    int j = start % count;
    for (int i = 0; i < n; i++) {
        TGraphicSprite *s = items[j];
        fn_406c50(s, t);
        j = (j + 1) % count;
    }
}
// MATCH 40971c @TSpriteGroup@fn_40971c$qv
// MATCH 409754 @TSpriteGroup@fn_409754$qi
// MATCH 409790 @TSpriteGroup@fn_409790$qii
// MATCH 4097e8 @TSpriteGroup@fn_4097e8$qiii
