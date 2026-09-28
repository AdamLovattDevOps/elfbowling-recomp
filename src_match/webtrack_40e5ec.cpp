// TWebTrack (HTTP ping over TClientSocket) 0x40e5ec-0x40ec34
// Event handlers are __fastcall (Borland register convention) and
// show up as @fn_XXXXXX in the object file.
//
// This unit defines the real TWebTrack class (with the TClientSocket and the
// __closure handlers), so it sets ELF_NO_TWEBTRACK: elf/stage.h then only
// forward-declares TWebTrack. The layout is the same as stage.h's.
// ScktComp is not part of <vcl.h>: <vcl/scktcomp.hpp> (BCB3 SCKTCOMP.HPP, or
// the port's stub natively) comes before the elf headers.
#define ELF_NO_TWEBTRACK
#include <vcl/scktcomp.hpp>
#include <stdlib.h>
#include <string.h>
#include <elf/funcs.h>

class TWebTrack {
public:
    char *buf;              // 0x00 receive buffer (0x1000 bytes)
    char *request;          // 0x04 HTTP request text
    int len;                // 0x08 bytes received
    bool f0c;               // 0x0c
    int state;              // 0x10 1=connected 2=sent 3=reading 4=done 5=reported 6=error
    Scktcomp::TClientSocket *client;   // 0x14
    TWebTrack(const char *host, const char *path, bool f);      // 0x40e888
    ~TWebTrack();                                               // 0x40ebd0 (fn_40ebd0)
    // The socket event handlers. They are matched as the free functions
    // fn_40e5ac..fn_40e6ac (`this` first); ELF_METHOD binds them.
    void __fastcall On5ac(System::TObject *Sender, Scktcomp::TCustomWinSocket *Socket);
    void __fastcall On5cc(System::TObject *Sender, Scktcomp::TCustomWinSocket *Socket);
    void __fastcall On624(System::TObject *Sender, Scktcomp::TCustomWinSocket *Socket);
    void __fastcall On664(System::TObject *Sender, Scktcomp::TCustomWinSocket *Socket);
    void __fastcall On684(System::TObject *Sender, Scktcomp::TCustomWinSocket *Socket);
    void __fastcall On6ac(System::TObject *Sender, Scktcomp::TCustomWinSocket *Socket);
    void __fastcall On5ec(System::TObject *Sender, Scktcomp::TCustomWinSocket *Socket, Scktcomp::TErrorEvent ErrorEvent, int &ErrorCode);
};

extern "C" void __fastcall fn_40e5ec(TWebTrack *self, void *sender, void *socket, int ev, int &code)
{
    self->state = 6;
    fn_401cf4("x %d", code);
    code = 0;
}

extern "C" void __fastcall fn_40e624(TWebTrack *self, void *sender, void *socket)
{
    if (self->state == 3)
        self->state = 4;
    else
        self->state = 6;
    fn_401cf4("x");
}

extern "C" void __fastcall fn_40e664(TWebTrack *self, void *sender, void *socket)
{
    fn_401cf4("x");
}

extern "C" void __fastcall fn_40e684(TWebTrack *self, void *sender, void *socket)
{
    fn_401cf4("x");
    self->state = 3;
}

extern "C" void __fastcall fn_40e6ac(TWebTrack *self, void *sender, void *socket)
{
    fn_401cf4("x");
    self->state = 1;
}

extern "C" void fn_40e6d4(TWebTrack *self)
{
    TCustomWinSocket_SendBuf(self->client->Socket, self->request, strlen(self->request));
    fn_401cf4("Requesting: %s", self->request);
}

extern "C" void fn_40e714(TWebTrack *self)
{
    int n;
    int avail;
    int room;
    avail = TCustomWinSocket_ReceiveLength(self->client->Socket);
    if (avail > 0) {
        room = 0x1000 - self->len;
        n = *(avail < room ? &avail : &room);
        self->len += TCustomWinSocket_ReceiveBuf(self->client->Socket, self->buf + self->len, n);
        self->buf[self->len] = 0;
    }
    if (n < avail)
        self->len -= 0x100;
}

extern "C" void fn_40e7a4(TWebTrack *self)
{
    if (self->state == 1) {
        fn_40e6d4(self);
        self->state = 2;
        return;
    }
    if (self->state == 3) {
        fn_40e714(self);
        return;
    }
    if (self->state == 4) {
        fn_401cf4("%s", self->buf);
        self->state = 5;
    }
}

typedef int (__cdecl *InetGetConnFn)(unsigned long *flags, unsigned long reserved);

extern "C" bool fn_40e800()
{
    bool ok;
    HMODULE lib;
    InetGetConnFn fn;
    unsigned long flags;
    ok = false;
    lib = LoadLibraryA("Wininet.dll");
    if (lib) {
        fn = (InetGetConnFn)GetProcAddress(lib, "InternetGetConnectedState");
        if (fn) {
            ok = fn(&flags, 0) != 0;
            fn_401cf4("DLL InternetGetConnectedState = %d (flags = %04x)\n", ok, flags);
        } else {
            fn_401cf4("DLL InternetGetConnectedState function not found.  No Internet connection.\n");
        }
        FreeLibrary(lib);
    } else {
        fn_401cf4("WININET DLL not found.  No Internet connection.\n");
    }
    return ok;
}

// ---- TWebTrack constructor 0x40e888 ----
// MATCH 40e888 @TWebTrack@$bctr$qpxct14bool
TWebTrack::TWebTrack(const char *host, const char *path, bool f)
{
    fn_401cec();
    g_46020c = this;
    f0c = f;
    len = 0;
    if (strncmp(host, "http://", 7) == 0)
        host += 7;
    request = 0;
    buf = 0;
    state = 7;
    int online = fn_40e800();
    if (online) {
        Scktcomp::SocketErrorProc = fn_40e584;
        client = new Scktcomp::TClientSocket((Classes::TComponent *)*g_45fee4);
        client->ClientType = Scktcomp::ctNonBlocking;
        request = (char *)fn_402338(0x201, "TWebTrack");
        buf = (char *)fn_402338(0x1001, "TWebTrack");
        if (request && buf) {
            strcpy(request, "GET http://");
            strcat(request, host);
            if (path[0] != '/')
                strcat(request, "/");
            strcat(request, path);
            strcat(request, "?");
            fn_401e48(request + strlen(request), rand() % 100000, 6, 1);
            strcat(request, "\r\n");
            state = 0;
            client->Host = host;
            client->Port = 80;
            client->Name = "ClientSocket";
            client->OnConnect = ELF_METHOD(this, On5ac, fn_40e5ac);
            client->OnConnecting = ELF_METHOD(this, On664, fn_40e664);
            client->OnDisconnect = ELF_METHOD(this, On624, fn_40e624);
            client->OnError = ELF_METHOD(this, On5ec, fn_40e5ec);
            client->OnLookup = ELF_METHOD(this, On5cc, fn_40e5cc);
            client->OnRead = ELF_METHOD(this, On684, fn_40e684);
            client->OnWrite = ELF_METHOD(this, On6ac, fn_40e6ac);
            client->Open();
        } else
            state = 6;
    }
}

extern "C" void fn_40ebd0(TWebTrack *self, int flags)
{
    if (self) {
        if (self->request)
            fn_402380(self->request, "~TWebTrack");
        if (self->buf)
            fn_402380(self->buf, "~TWebTrack");
        if (flags & 1)
            rtl_delete(self);
    }
}

extern "C" void fn_40ec20()
{
    fn_40b1d4(g_4601c8);
}

extern "C" bool fn_40ec34(void *form, int w, int h)
{
    int dw;
    int zw;
    int dh;
    int zh;
    dw = TControl_GetClientWidth(form) - w;
    zw = 0;
    g_455540 = *(zw > dw ? &zw : &dw);
    dh = TControl_GetClientHeight(form) - h;
    zh = 0;
    g_455544 = *(zh > dh ? &zh : &dh);
    g_455548 = g_455540 > 0 || g_455544 > 0;
    if (g_455548) {
        TCustomForm_SetClientWidth(form, w);
        TCustomForm_SetClientHeight(form, h);
    }
    return g_455548;
}
