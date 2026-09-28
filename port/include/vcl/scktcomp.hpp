// vcl/scktcomp.hpp - ScktComp subset: TClientSocket as a no-op stub.
//
// The game's TWebTrack pinged www.nstorm.com through a non-blocking
// TClientSocket. That server is long gone, and the port's LoadLibraryA
// returns NULL, so fn_40e800 reports "no Internet connection" and TWebTrack
// never creates the socket. The stub keeps the declarations the unit needs:
// Open() logs and does nothing, so no event ever fires; the socket calls
// return 0 bytes.
#ifndef ScktCompHPP
#define ScktCompHPP

#include <vcl/classes.hpp>

namespace Scktcomp {

using System::AnsiString;

enum TClientType { ctNonBlocking, ctBlocking };
enum TErrorEvent { eeGeneral, eeSend, eeReceive, eeConnect, eeDisconnect, eeAccept };

class TCustomWinSocket : public System::TObject {
public:
    TCustomWinSocket() {}
    virtual ~TCustomWinSocket() {}
    int SendBuf(void *Buf, int Count);
    bool SendText(const AnsiString &s);
    int ReceiveLength();
    int ReceiveBuf(void *Buf, int Count);
    AnsiString ReceiveText();
    bool GetConnected() const { return false; }
    VCL_PROPERTY(TCustomWinSocket, bool, Connected, &TCustomWinSocket::GetConnected, nullptr);
};

class TClientWinSocket : public TCustomWinSocket {};

ELF_CLOSURE(void, TSocketNotifyEvent, (System::TObject *Sender, TCustomWinSocket *Socket));
ELF_CLOSURE(void, TSocketErrorEvent, (System::TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode));
typedef void (*TSocketErrorProc)(int ErrorCode);
extern TSocketErrorProc __thread SocketErrorProc;

class TAbstractSocket : public Classes::TComponent {
public:
    explicit TAbstractSocket(Classes::TComponent *AOwner) : Classes::TComponent(AOwner), FActive(false), FPort(0) {}
    ~TAbstractSocket() override {}
    void Open();
    void Close() { FActive = false; }

    AnsiString GetHost() const { return FHost; }
    void SetHost(const AnsiString &v) { FHost = v; }
    AnsiString GetAddress() const { return FAddress; }
    void SetAddress(const AnsiString &v) { FAddress = v; }
    AnsiString GetService() const { return FService; }
    void SetService(const AnsiString &v) { FService = v; }
    int GetPort() const { return FPort; }
    void SetPort(int v) { FPort = v; }
    bool GetActive() const { return FActive; }
    void SetActive(bool v) { if (v) Open(); else Close(); }

    VCL_PROPERTY(TAbstractSocket, AnsiString, Host, &TAbstractSocket::GetHost, &TAbstractSocket::SetHost);
    VCL_PROPERTY(TAbstractSocket, AnsiString, Address, &TAbstractSocket::GetAddress, &TAbstractSocket::SetAddress);
    VCL_PROPERTY(TAbstractSocket, AnsiString, Service, &TAbstractSocket::GetService, &TAbstractSocket::SetService);
    VCL_PROPERTY(TAbstractSocket, int, Port, &TAbstractSocket::GetPort, &TAbstractSocket::SetPort);
    VCL_PROPERTY(TAbstractSocket, bool, Active, &TAbstractSocket::GetActive, &TAbstractSocket::SetActive);

protected:
    bool FActive;
    int FPort;
    AnsiString FAddress, FHost, FService;
};

class TCustomSocket : public TAbstractSocket {
public:
    explicit TCustomSocket(Classes::TComponent *AOwner) : TAbstractSocket(AOwner) {}
    TSocketNotifyEvent OnLookup;
    TSocketNotifyEvent OnConnecting;
    TSocketNotifyEvent OnConnect;
    TSocketNotifyEvent OnDisconnect;
    TSocketNotifyEvent OnRead;
    TSocketNotifyEvent OnWrite;
    TSocketErrorEvent OnError;
};

class TClientSocket : public TCustomSocket {
public:
    explicit TClientSocket(Classes::TComponent *AOwner) : TCustomSocket(AOwner), FClientType(ctNonBlocking) {}
    ~TClientSocket() override {}
    TClientType GetClientType() const { return FClientType; }
    void SetClientType(TClientType v) { FClientType = v; }
    TClientWinSocket *GetSocket() { return &FSocket; }
    VCL_PROPERTY(TClientSocket, TClientType, ClientType, &TClientSocket::GetClientType, &TClientSocket::SetClientType);
    VCL_PROPERTY(TClientSocket, TClientWinSocket *, Socket, &TClientSocket::GetSocket, nullptr);

private:
    TClientType FClientType;
    TClientWinSocket FSocket;
};

} // namespace Scktcomp

#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Scktcomp;
#endif

#endif
