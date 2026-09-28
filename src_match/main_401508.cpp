// Main unit 0x401508-0x401cec: startup helper, TForm1 constructor / implicit destructor
// and event handlers. The real VCL headers compile via the INCLUDE\VCL.H stub; the TForm /
// TComponent / DelphiInterface inlines they generate are matched too.
#pragma option -k
#include <vcl.h>
#include <elf/funcs.h>

extern "C" void fn_401508()
{
    rtl_srand(rtl_time(0));
    fn_40f460(g_4601c0);
}

// TForm1 event handlers (Delphi register convention = __fastcall).
// TShiftState is a 1-byte VCL Set<> passed by value on the stack (ShiftState).

// TForm1::FormPaint
extern "C" void __fastcall fn_4015f8(void *form, void *sender)
{
    fn_40ec20();
}

// TForm1::Form1KeyDown
extern "C" void __fastcall fn_401610(void *form, void *sender, unsigned short &key, ShiftState shift)
{
    if (g_456c38)
        fn_40bd3c(g_456c38, sender, key, shift);
}

// TForm1::Form1KeyUp2
extern "C" void __fastcall fn_401644(void *form, void *sender, unsigned short &key, ShiftState shift)
{
    if (g_456c38)
        fn_40bda4(g_456c38, sender, key, shift);
}

class TForm1 : public TForm
{
__published:
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall Form1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall Form1KeyUp2(TObject *Sender, WORD &Key, TShiftState Shift);
public:
    __fastcall TForm1(TComponent *Owner);
#ifndef __BORLANDC__
    // No RTTI for __published methods natively: the DFM's OnPaint/OnKeyDown/
    // OnKeyUp names bind to the matched handlers (docs/VCL_PORT.md).
    VCL_PUBLISHED(TForm1,
                  VCL_METHOD_AS(FormPaint, fn_4015f8),
                  VCL_METHOD_AS(Form1KeyDown, fn_401610),
                  VCL_METHOD_AS(Form1KeyUp2, fn_401644))
#endif
};
#ifndef __BORLANDC__
VCL_REGISTER_CLASS(TForm1);     // __classid(TForm1) in elf_40128c
#endif

__fastcall TForm1::TForm1(TComponent *Owner) : TForm(Owner)
{
    fn_401508();
}
// MATCH 401528 @TForm1@$bctr$qqrp18Classes@TComponent
// MATCH 40176c @TForm1@$bdtr$qqrv
// MATCH 401588 @Forms@TForm@$bctr$qqrp18Classes@TComponent
// MATCH 4017b8 @Classes@TComponent@UpdateRegistry$qqr4boolx17System@AnsiStringxt2
// MATCH 401c8c @System@%DelphiInterface$t8IUnknown%@$bdtr$qqrv
// Also byte-identical, but funcs.tsv runs them into the RTTI / EH data that follows
// (func_ends 401884->4018dc, 401a2c->401a6c): @Forms@TForm@$bdtr$qqrv (0x401884),
// @System@%DelphiInterface$t14Forms@IOleForm%@$bdtr$qqrv (0x401a2c).
// MATCH 401884 @Forms@TForm@$bdtr$qqrv
// MATCH 401a2c @System@%DelphiInterface$t14Forms@IOleForm%@$bdtr$qqrv
