// TStage members 0x40ae80, 0x40b9b8, 0x40bf2c, 0x40bf84, 0x40c160 (this at
// [ebp+8], cdecl). As free functions the compiler orders the loads for
// this->arr[i] / i < this->n differently.
#include <elf/funcs.h>

// Aliases: this unit passes the rects as ERect (rep movsd copies), where
// funcs.h has the RECT (four-dword push) spelling. See docs/HEADERS.md.
extern "C" char fn_402580_E(ERect a, ERect b);      // rects touch/overlap
extern "C" ERect fn_4025e0_E(ERect a, ERect b);     // union

void TStage::fn_40ae80(ERect r)
{
    char found = 0;
    for (int i = 0; !found && i < nareas; i++) {
        if (fn_402580_E(ELF_AS(ERect, dirty[i]), r)) {
            ELF_AS(ERect, dirty[i]) = fn_4025e0_E(ELF_AS(ERect, dirty[i]), r);
            found = 1;
        }
    }
    if (!found) {
        if (nareas < 80)
            ELF_AS(ERect, dirty[nareas++]) = r;
        else
            fn_401f0c("Too many update areas on Stage.");
    }
}

void TStage::fn_40b9b8()
{
    if (scene)
        fn_409c50(scene);
    for (int i = 0; i < nareas; i++) {
        ERect r = ELF_AS(ERect, dirty[i]);
        fn_40b708(this, r);
        if (dirtyFlag) {
            fn_40b8fc(r, back.handle, *(TPoint *)&r);
            dirtyFlag = 0;
        }
    }
    nareas = 0;
}

TScene *TStage::fn_40bf2c(const char *name)
{
    TScene *r = 0;
    int i = fn_40c160(name);
    if (i >= 0)
        r = fn_40bef4(this, scenes[i], 0);
    else
        fn_401f30("Unable to find and start scene ", name);
    return r;
}

TScene *TStage::fn_40bf84(const char *name)
{
    TScene *r = 0;
    int i = fn_40c160(name);
    if (i >= 0)
        r = fn_40bef4(this, scenes[i], scene);
    else
        fn_401f30("Unable to find and start scene ", name);
    return r;
}

int TStage::fn_40c160(const char *name)
{
    int idx = -1;
    for (int i = 0; i < nscenes; i++) {
        if (rtl_stricmp(scenes[i]->name, name) == 0)
            idx = i;
    }
    return idx;
}
// MATCH 40ae80 @TStage@fn_40ae80$q5ERect
// MATCH 40b9b8 @TStage@fn_40b9b8$qv
// MATCH 40bf2c @TStage@fn_40bf2c$qpxc
// MATCH 40bf84 @TStage@fn_40bf84$qpxc
// MATCH 40c160 @TStage@fn_40c160$qpxc
