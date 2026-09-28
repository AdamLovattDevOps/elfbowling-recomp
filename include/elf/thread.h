// elf/thread.h - TMyThread, the game's only TThread (threads_40f604..40f734).
// VCL only: include <vcl/classes.hpp> (or <vcl.h>) before the elf headers.
// Without it only the forward declaration is visible (stage.h, funcs.h).
//
// ctor 0x40f604 (CreateSuspended), Update 0x40f674 (-> fn_40c1b4, the frame
// tick), Execute 0x40f684 (`while (!Terminated) Synchronize(Update)`),
// implicit dtor 0x40f734. TStage's ctor does `new TMyThread(true)` and
// `Resume()` (engine_40a978).
//
// Update and Execute are matched as the free functions fn_40f674/fn_40f684
// (extern "C" __fastcall, `this` in eax), so on bcc32 the members have no
// body in this build. The native build gives them one-line bodies
// (threads_40f604.cpp).
//
// Terminated is protected in BCB3's TThread; the matched Execute is a free
// function, so TMyThread republishes the property (`__property Terminated;`,
// a read of FTerminated at +0x0c: same code as the old struct view). The
// port's TThread already has it public.
#ifndef ELF_THREAD_H
#define ELF_THREAD_H

class TMyThread;

#if defined(ClassesHPP)
class TMyThread : public Classes::TThread
{
protected:
    void __fastcall Execute();              // 0x40f684 (fn_40f684)
public:
    void __fastcall Update();               // 0x40f674 (fn_40f674)
    __fastcall TMyThread(bool CreateSuspended);     // 0x40f604
#ifdef __BORLANDC__
    __property Terminated;
#endif
};

// Classes::TThread::Synchronize (protected in BCB3) under a plain-function
// spelling; the linker resolves it to the VCL method.
extern void __fastcall TThread_Synchronize(void *self, Classes::TThreadMethod method);
#endif

#endif
