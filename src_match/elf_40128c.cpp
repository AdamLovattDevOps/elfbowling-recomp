//---------------------------------------------------------------------------
// Project unit (Elf.cpp, IDE-generated). 0x40128c WinMain + its catch handler
// (0x40131a-0x40134a, ends in ret 0x10); the Sysutils::Exception dtor at 0x40142c
// is the inline instantiated by the catch clause.
//
// WinMain itself is byte-identical (190 bytes = 0x40128c..0x40134a) but cannot be
// recorded yet: tools/funcs.py keeps 0x40131a as a separate start (it is reached
// only from the EH table), so funcs.tsv gives WinMain 142 bytes. notes/func_ends.tsv
// has 40128c->40134c; once funcs.py lets that entry swallow inner starts, add:
//   a MATCH directive for 40128c WinMain
//
// 0x401284 (sub [0x4601b8],1; ret) is NOT from this file: it is the dcc32 unit
// initialization stub of SysInit.pas (pairs with the "Finalization" at 0x401254).
// No #pragma package(smart_init) here: the original has no @Elf@C3_x counters.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <elf/funcs.h>
USERES("Elf.res");
USEFORM("Main.cpp", Form1);
#ifndef __BORLANDC__
// Natively Form1 is the game's own form pointer (0x45fedc -> g_4601c0);
// USEFORM only declares the class there.
#define Form1 g_4601c0
#endif
//---------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        Application->Initialize();
        Application->Title = "Elf Bowling";
        Application->CreateForm(__classid(TForm1), &Form1);
        Application->Run();
    }
    catch (Exception &exception)
    {
        Application->ShowException(&exception);
    }
    return 0;
}
//---------------------------------------------------------------------------
// MATCH 40142c @Sysutils@Exception@$bdtr$qqrv
// MATCH 40128c WinMain
