// Threads unit 0x40f604-0x40f7ec: the implicit TMyThread destructor
// @TMyThread@$bdtr$qqrv (0x40f734, 75 bytes; notes/func_ends.tsv 40f734->40f780).
// bcc32 emits it in the unit that defines the ctor, so this file repeats the
// ctor of threads_40f604.cpp (elf/thread.h has the class). The native build
// takes the ctor from threads_40f604.cpp only.
// MATCH 40f734 @TMyThread@$bdtr$qqrv
#include <vcl/classes.hpp>
#include <elf/funcs.h>

#ifdef __BORLANDC__
__fastcall TMyThread::TMyThread(bool CreateSuspended) : Classes::TThread(CreateSuspended)
{
}
#endif
