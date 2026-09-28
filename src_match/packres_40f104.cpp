// Compiler-emitted destructors of the TPackedStream unit (0x40f104-0x40f2fc), code byte-identical:
//   @TPackedStream@$bdtr$qqrv 75 bytes (funcs.tsv 76: EH table from 0x40f150). Compared in
//     packres_40e0a8.cpp: TPackedStream has no user-declared dtor (elf/packres.h), and the unit
//     that defines its ctor is the one that emits the vtable and the implicit dtor.
//   @Classes@TStream@$bdtr$qqrv 87 bytes (funcs.tsv 88: EH table + TResourceStream typeinfo from 0x40f23c)
//   @Classes@TCustomMemoryStream@$bdtr$qqrv 88 bytes (compared in packres_40f2a4.cpp).
// Uses notes/func_ends.tsv 40f104->40f150 and 40f1e4->40f23c.
// TCustomMemoryStream's inline dtor is emitted because the unit news a Classes::TResourceStream
// (typeinfo "Classes::TResourceStream" at 0x40f16c). Real <vcl/classes.hpp>.
// MATCH 40f1e4 @Classes@TStream@$bdtr$qqrv
#include <vcl/classes.hpp>
#include <elf/funcs.h>

int __fastcall TPackedStream::Write(const void *Buffer, int Count) { return 0; }
void *dummy_new_40f104(int inst, int id)
{
    Classes::TResourceStream *rs = new Classes::TResourceStream(inst, id, (char *)10);
    delete rs;
    return 0;
}
