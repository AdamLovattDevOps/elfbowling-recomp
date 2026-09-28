// ScktComp stub: TClientSocket never connects (the tracker server is gone).
#include <vcl/scktcomp.hpp>

#include <cstdio>

namespace Scktcomp {

TSocketErrorProc __thread SocketErrorProc;

int TCustomWinSocket::SendBuf(void *, int) { return 0; }
bool TCustomWinSocket::SendText(const AnsiString &) { return false; }
int TCustomWinSocket::ReceiveLength() { return 0; }
int TCustomWinSocket::ReceiveBuf(void *, int) { return 0; }
AnsiString TCustomWinSocket::ReceiveText() { return AnsiString(); }

void TAbstractSocket::Open()
{
    std::fprintf(stderr, "[vcl] TClientSocket::Open(%s:%d): stub, no connection\n", FHost.c_str(), FPort);
}

} // namespace Scktcomp
