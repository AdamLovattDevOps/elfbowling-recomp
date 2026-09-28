// Threads unit 0x40f604-0x40f7ec: TMyThread::Update (elf/thread.h).
#include <vcl/classes.hpp>
#include <elf/funcs.h>

// TMyThread::Update: one frame tick, run on the main thread by Synchronize.
extern "C" void __fastcall fn_40f674(TMyThread *self)
{
    fn_40c1b4();
}
