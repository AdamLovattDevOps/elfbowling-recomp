// vcl/classes.hpp - Classes unit subset: TShiftState, TNotifyEvent,
// TPersistent, TComponent, the streams, TThread, Rect/Point.
// Only the members the game (src_match/) and the port runtime use.
#ifndef ClassesHPP
#define ClassesHPP

#include <vcl/system.hpp>
#include <vcl/windows.hpp>
#include <vcl/sysutils.hpp>

#include <atomic>
#include <string>
#include <vector>

namespace Classes {

using System::AnsiString;
using System::TObject;
using System::Word;

enum TShiftStateElem { ssShift, ssAlt, ssCtrl, ssLeft, ssRight, ssMiddle, ssDouble };
typedef System::Set<TShiftStateElem, ssShift, ssDouble> TShiftState;

ELF_CLOSURE(void, TNotifyEvent, (System::TObject *Sender));
ELF_CLOSURE(void, TThreadMethod, ());

enum TOperation { opInsert, opRemove };

VCL_EXCEPTION_CLASS(EStreamError, Sysutils::Exception);
VCL_EXCEPTION_CLASS(EReadError, EStreamError);
VCL_EXCEPTION_CLASS(EWriteError, EStreamError);
VCL_EXCEPTION_CLASS(EResNotFound, Sysutils::Exception);
VCL_EXCEPTION_CLASS(EThread, Sysutils::Exception);

} // namespace Classes

namespace Vcl {
// A decoded DFM property value (see port/vcl/dfm.cpp).
struct TDfmValue {
    enum Kind { Int, Float, String, Ident, Bool, Set, List, Binary, Nil, Unsupported } kind = Nil;
    long long i = 0;
    double f = 0;
    std::string s;                  // String and Ident
    std::vector<std::string> set;   // Set
    bool b = false;
};
// Event kinds for DFM binding (TComponent::DfmEvent / VCL_PUBLISHED).
enum TEventKind { ekNone, ekNotify, ekKey, ekKeyPress, ekMouse, ekMouseMove, ekClose };
}

namespace Classes {

class TPersistent : public System::TObject {
public:
    TPersistent() {}
    virtual ~TPersistent() {}
};

class TComponent : public TPersistent {
public:
    explicit TComponent(TComponent *AOwner);
    virtual ~TComponent();

    TComponent *GetOwner() const { return FOwner; }
    AnsiString GetName() const { return FName; }
    virtual void SetName(const AnsiString &NewName) { FName = NewName; }
    int GetComponentCount() const { return (int)FComponents.size(); }
    TComponent *GetComponent(int i) const { return FComponents[i]; }
    void InsertComponent(TComponent *c);
    void RemoveComponent(TComponent *c);
    virtual void Loaded() {}

    VCL_PROPERTY(TComponent, AnsiString, Name, &TComponent::GetName, &TComponent::SetName);
    VCL_PROPERTY(TComponent, TComponent *, Owner, &TComponent::GetOwner, nullptr);
    VCL_PROPERTY(TComponent, int, ComponentCount, &TComponent::GetComponentCount, nullptr);

    // DFM streaming (port/vcl/dfm.cpp). DfmSetProperty returns false for an
    // unknown property; DfmEvent returns the address of the closure field
    // for an event property and its kind, or NULL.
    virtual bool DfmSetProperty(const char *name, const Vcl::TDfmValue &v);
    virtual void *DfmEvent(const char *name, Vcl::TEventKind *kind);

protected:
    TComponent *FOwner;
    AnsiString FName;
    std::vector<TComponent *> FComponents;
    bool FDestroying;
};

// ---- streams ---------------------------------------------------------------

enum { soFromBeginning = 0, soFromCurrent = 1, soFromEnd = 2 };

// Zero-size property proxies for TStream (it must stay "vptr + nothing").
class TStream;
struct TStreamPositionProp {
    operator int() const;
    const TStreamPositionProp &operator=(int v) const;
    const TStreamPositionProp &operator=(const TStreamPositionProp &p) const { return *this = (int)p; }
    const TStreamPositionProp &operator+=(int v) const { return *this = (int)*this + v; }
    const TStreamPositionProp &operator-=(int v) const { return *this = (int)*this - v; }
    TStream *owner() const;
};
struct TStreamSizeProp {
    operator int() const;
    const TStreamSizeProp &operator=(int v) const;
    TStream *owner() const;
};

// Virtual order is the Delphi 3 VMT order: SetSize, Read, Write, Seek. With
// no virtuals in TObject, Read is vtable slot 1, which is where the game's
// raw VMT calls (elf/packres.h TStreamVmt: st->vt[1](st, buf, n)) look.
class TStream : public System::TObject {
protected:
    virtual void SetSize(int NewSize);

public:
    virtual int Read(void *Buffer, int Count) = 0;
    virtual int Write(const void *Buffer, int Count) = 0;
    virtual int Seek(int Offset, Word Origin) = 0;
    TStream() {}
    virtual ~TStream() {}

    int GetPosition() { return Seek(0, soFromCurrent); }
    void SetPosition(int Pos) { Seek(Pos, soFromBeginning); }
    int GetSize();
    void ReadBuffer(void *Buffer, int Count);
    void WriteBuffer(const void *Buffer, int Count);
    int CopyFrom(TStream *Source, int Count);

    [[no_unique_address]] TStreamPositionProp Position;
    [[no_unique_address]] TStreamSizeProp Size;

    friend struct TStreamPositionProp;
    friend struct TStreamSizeProp;
};

class TCustomMemoryStream : public TStream {
public:
    TCustomMemoryStream() : FMemory(nullptr), FSize(0), FPosition(0) {}
    virtual ~TCustomMemoryStream() {}
    int Read(void *Buffer, int Count) override;
    int Seek(int Offset, Word Origin) override;
    void *GetMemory() const { return FMemory; }
    void SaveToStream(TStream *Stream);
    VCL_PROPERTY(TCustomMemoryStream, void *, Memory, &TCustomMemoryStream::GetMemory, nullptr);

protected:
    void SetPointer(void *Ptr, int Size) { FMemory = Ptr; FSize = Size; }
    void *FMemory;
    int FSize;
    int FPosition;
};

class TMemoryStream : public TCustomMemoryStream {
public:
    TMemoryStream() : FCapacity(0) {}
    ~TMemoryStream() override;
    int Write(const void *Buffer, int Count) override;
    void SetSize(int NewSize) override;
    void Clear();
    void LoadFromStream(TStream *Stream);

private:
    void SetCapacity(int NewCapacity);
    int FCapacity;
};

// Read-only stream over a resource of the user's Elf Bowling.exe (the shim's
// FindResourceA). Raises EResNotFound when the resource is missing.
class TResourceStream : public TCustomMemoryStream {
public:
    TResourceStream(System::THandle Instance, const AnsiString &ResName, const char *ResType);
    TResourceStream(System::THandle Instance, int ResID, const char *ResType);
    TResourceStream(HINSTANCE Instance, const AnsiString &ResName, const char *ResType);
    TResourceStream(HINSTANCE Instance, int ResID, const char *ResType);
    ~TResourceStream() override {}
    int Write(const void *Buffer, int Count) override;

private:
    void Initialize(HINSTANCE Instance, const char *Name, const char *ResType);
    HRSRC HResInfo;
    void *HGlobal;
};

// ---- TThread -----------------------------------------------------------------
//
// A real thread (SDL_Thread). Synchronize(Method) runs Method on the main
// thread: it queues the call, wakes the main loop and blocks until
// Application's loop (or WaitFor / ProcessMessages) has run it, as VCL 3
// does with CM_EXECPROC. Called on the main thread it runs Method directly.
//
// Differences from VCL 3 (docs/VCL_PORT.md):
//   - TThread(false) starts the thread on the next main-loop pump (or Resume),
//     never inside the base constructor, where Execute would still be pure.
//   - Suspend is advisory: a running thread is not stopped; Resume only
//     starts a thread that was created suspended.
//   - Terminated, Synchronize and Execute are public, because the matched
//     source calls them from free functions (fn_40f684).
class TThread : public System::TObject {
public:
    explicit TThread(bool CreateSuspended);
    virtual ~TThread();

    virtual void Execute() = 0;
    void Resume();
    void Suspend();
    void Terminate() { FTerminated = true; }
    int WaitFor();
    void Synchronize(TThreadMethod Method);

    bool GetTerminated() const { return FTerminated; }
    bool GetSuspended() const { return FSuspended; }
    bool GetFreeOnTerminate() const { return FFreeOnTerminate; }
    void SetFreeOnTerminate(bool v) { FFreeOnTerminate = v; }
    int GetReturnValue() const { return FReturnValue; }
    void SetReturnValue(int v) { FReturnValue = v; }
    unsigned long GetThreadID() const { return FThreadID; }
    bool GetFinished() const { return FFinished; }

    VCL_PROPERTY(TThread, bool, Terminated, &TThread::GetTerminated, nullptr);
    VCL_PROPERTY(TThread, bool, Suspended, &TThread::GetSuspended, nullptr);
    VCL_PROPERTY(TThread, bool, FreeOnTerminate, &TThread::GetFreeOnTerminate, &TThread::SetFreeOnTerminate);
    VCL_PROPERTY(TThread, int, ReturnValue, &TThread::GetReturnValue, &TThread::SetReturnValue);
    VCL_PROPERTY(TThread, unsigned long, ThreadID, &TThread::GetThreadID, nullptr);
    VCL_PROPERTY(TThread, bool, Finished, &TThread::GetFinished, nullptr);   // (Delphi 2009+)
    TNotifyEvent OnTerminate;

protected:
    virtual void DoTerminate();

private:
    friend struct TThreadAccess;
    void Start();
    void CallOnTerminate();
    void *FHandle;              // SDL_Thread *
    unsigned long FThreadID;
    std::atomic<bool> FTerminated;
    bool FSuspended;
    bool FFreeOnTerminate;
    std::atomic<bool> FFinished;
    std::atomic<bool> FStarted;
    int FReturnValue;
};

// ---- Rect / Point ----------------------------------------------------------
Windows::TRect Rect(int ALeft, int ATop, int ARight, int ABottom);
Windows::TRect Bounds(int ALeft, int ATop, int AWidth, int AHeight);
Windows::TPoint Point(int AX, int AY);

} // namespace Classes

// ---- helpers the runtime and tests use ---------------------------------------
namespace Vcl {
// Run queued TThread::Synchronize calls (and start deferred threads) on the
// main thread. Returns the number of calls run.
int CheckSynchronize();
// Main thread, idle: wait up to `ms` for a Synchronize request when a thread
// is running (a condition variable: no SDL event round trip). Returns false
// at once when no thread runs.
bool WaitForSynchronize(unsigned ms);
// True on the thread that started the program (the VCL main thread).
bool IsMainThread();
// Make the calling thread the main one (main() does; static init may run elsewhere, as on Android).
void SetMainThread();
// Terminate, wake and join every live TThread (called at shutdown).
void ShutdownThreads(unsigned timeoutMs);
}

#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Classes;
#endif

#endif
