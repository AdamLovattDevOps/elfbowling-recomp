// TPackedStream constructor (cdecl C++ ctor on a VCL TStream subclass), 0x40e0a8,
// plus the inline Classes::TStream ctor it emits (0x40de8c) and the compiler-generated
// TPackedStream dtor (0x40f104; this unit defines the ctor, so it emits the vtable and
// the implicit dtor). Real <vcl/classes.hpp>; the class is declared in elf/packres.h.
// MATCH 40e0a8 @TPackedStream@$bctr$qpcit1ii
// MATCH 40de8c @Classes@TStream@$bctr$qqrv
// MATCH 40f104 @TPackedStream@$bdtr$qqrv
#include <vcl/classes.hpp>
#include <elf/funcs.h>

TPackedStream::TPackedStream(char *in, int inlen, char *o, int olen, int sz)
{
    out = o;
    outsize = olen;
    used = 0;
    fn_40d498(&z);
    fn_420984(&z, "1.0", sizeof(ZStream));     // 0x38 on bcc32
    z.next_in = in;
    z.avail_in = inlen;
    z.next_out = out;
    z.avail_out = outsize;
    size = sz;
    used = 0;
    pos = 0;
}

#ifndef __BORLANDC__
// Native build: TStream::SetSize is VMT slot 0; the original's entry is
// fn_40dfd0 (packres_40dbe0.cpp).
void TPackedStream::SetSize(int NewSize)
{
    fn_40dfd0((TPackedStreamData *)this, NewSize);
}
#endif
