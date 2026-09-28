// TStage: add an off-stage update area, 0x40af80 (member function).
#include <elf/funcs.h>

// Aliases: this unit uses the ERect (copy-ctor) spelling where funcs.h has
// RECT / RectPod. See docs/HEADERS.md.
extern "C" char fn_40252c_E(ERect a, ERect b);
extern "C" ERect fn_4025e0_E(ERect a, ERect b);
extern "C" ERect fn_40a5d4_E(TStage *e, ERect &r);
extern "C" ERect fn_40a63c_E(TStage *e, ERect &r);

void TStage::fn_40af80(ERect r)
{
    char found = 0;
    ERect t = fn_40a5d4_E(this, r);
    char vis = fn_402654(&t, fn_43a8dc(0, 0, fn_42fdd4(*g_45fee8), fn_42fdcc(*g_45fee8)));
    r = fn_40a63c_E(this, t);
    if (vis && r.r > r.l && r.b > r.t) {
        for (int i = 0; !found && i < nareas2; i++) {
            if (fn_40252c_E(ELF_AS(ERect, offstage[i]), r)) {
                ERect u = fn_4025e0_E(ELF_AS(ERect, offstage[i]), r);
                if (!fn_40252c_E(ELF_AS(ERect, screen), u)) {
                    ELF_AS(ERect, offstage[i]) = u;
                    found = 1;
                }
            }
        }
        if (!found) {
            if (nareas2 < 40)
                ELF_AS(ERect, offstage[nareas2++]) = r;
            else
                fn_401f0c("Too many off stage update areas.");
        }
    }
}
// MATCH 40af80 @TStage@fn_40af80$q5ERect
