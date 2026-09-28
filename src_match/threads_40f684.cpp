// Threads unit 0x40f604-0x40f7ec: TMyThread::Execute (elf/thread.h).
#include <vcl/classes.hpp>
#include <elf/funcs.h>

// TMyThread::Execute
extern "C" void __fastcall fn_40f684(TMyThread *self)
{
    while (!self->Terminated)
        TThread_Synchronize(self, ELF_METHOD(self, Update, fn_40f674));
}
