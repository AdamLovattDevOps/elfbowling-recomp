// Compiler-generated destructors of the TPackedStream unit (0x40f104-0x40f2fc).
// Only @Classes@TCustomMemoryStream@$bdtr$qqrv is compared here; the TPackedStream and TStream
// dtors are compared in packres_40e0a8.cpp and packres_40f104.cpp. The inline dtor is emitted
// because the unit news a Classes::TResourceStream. Real <vcl/classes.hpp>.
// MATCH 40f2a4 @Classes@TCustomMemoryStream@$bdtr$qqrv
#include <vcl/classes.hpp>
#include <elf/funcs.h>

void *dummy_new_40f2a4(int inst, int id)
{
    Classes::TResourceStream *rs = new Classes::TResourceStream(inst, id, (char *)10);
    delete rs;
    return 0;
}
