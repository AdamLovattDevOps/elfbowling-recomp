// Threads unit 0x40f604-0x40f7ec: TMyThread (TThread subclass), constructor.
// The class is in elf/thread.h (VCL: <vcl/classes.hpp> first).
// The implicit destructor @TMyThread@$bdtr$qqrv (0x40f734) is generated here too and is
// byte-identical, but funcs.tsv runs it into its EH table (see threads_40f734.cpp).
// MATCH 40f604 @TMyThread@$bctr$qqr4bool
#include <vcl/classes.hpp>
#include <elf/funcs.h>

__fastcall TMyThread::TMyThread(bool CreateSuspended) : Classes::TThread(CreateSuspended)
{
}

#ifndef __BORLANDC__
// Native build: the members are the matched free functions.
void __fastcall TMyThread::Execute() { fn_40f684(this); }
void __fastcall TMyThread::Update() { fn_40f674(this); }
#endif
