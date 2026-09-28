// vcl/elfcompat.h - the plain-function spellings of VCL calls that the
// matched source declares for itself (the Borland linker resolves them to the
// VCL methods). port/vcl/entry.cpp defines them; this header only exists so
// that the definitions are checked against one declaration.
//
// The extern "C" fn_XXXXXX library entry points of include/elf/funcs.h
// ("library / VCL / RTL entry points called by address") and the rtl_*
// aliases are defined in port/vcl/entry.cpp too, and the three that return
// the game's ERect/TRect types in port/vcl/elf_glue.cpp.
#ifndef PORT_VCL_ELFCOMPAT_H
#define PORT_VCL_ELFCOMPAT_H

#include <vcl/classes.hpp>

// C++ linkage, as the units declare them (init_40f350, range_401cec,
// webtrack_40e5ec, threads_40f674/684).
void __fastcall TCustomForm_Close(void *form);
void __fastcall TCustomForm_SetClientWidth(void *form, int v);
void __fastcall TCustomForm_SetClientHeight(void *form, int v);
void __fastcall TApplication_Terminate(void *app);
int __fastcall TApplication_MessageBox(void *app, const char *text, const char *caption, int flags);
int __fastcall TScreen_GetWidth(void *scr);
int __fastcall TScreen_GetHeight(void *scr);
int __fastcall TControl_GetClientWidth(void *ctl);
int __fastcall TControl_GetClientHeight(void *ctl);
void __fastcall TCustomWinSocket_SendBuf(void *sock, void *buf, int count);
int __fastcall TCustomWinSocket_ReceiveLength(void *sock);
int __fastcall TCustomWinSocket_ReceiveBuf(void *sock, void *buf, int count);
void __fastcall TThread_Synchronize(void *self, Classes::TThreadMethod method);

// Classes::Point / Classes::Rect under the return types types.h also
// declares (Classes_TRect returns ERect: elf_glue.cpp).
TPoint __fastcall Classes_Point(int x, int y);
RECT __fastcall Classes_Rect(int l, int t, int r, int b);

// The VCL globals as the game reaches them: the indirection cells the
// Borland linker emits for Delphi unit variables (include/elf/game.h;
// unmigrated units call 0x45fee4 p_Application). 0x45fedc -> Form1 is the
// game's own g_4601c0, not defined here.
extern "C" {
extern void **g_45fee4;         // 0x45fee4 -> &Forms::Application
extern void **g_45fee8;         // 0x45fee8 -> &Forms::Screen
extern HINSTANCE *g_45fee0;     // 0x45fee0 -> &System::HInstance
}

#endif
