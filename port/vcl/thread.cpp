// TThread on SDL_Thread, and TThread::Synchronize.
//
// Synchronize from a worker queues the call, pushes a wake event into the
// SDL queue and blocks on a condition until the main thread has run it
// (Vcl::CheckSynchronize, called by the Application loop, ProcessMessages
// and WaitFor). This is VCL 3's CM_EXECPROC behaviour without a window.
//
// The game's only thread (TMyThread, threads_40f604..40f734) does
//     while (!Terminated) Synchronize(Update);
// so all game logic (Update -> fn_40c1b4, the frame tick) runs on the main
// thread, interleaved with input and paint, as on Windows.
#include <vcl/vcl.h>

#include <SDL.h>

#include <algorithm>
#include <cstdio>
#include <deque>
#include <exception>
#include <vector>

#ifndef __EMSCRIPTEN__      // the web build's cooperative TThread is at the end of this file

namespace Vcl {

namespace {

struct SyncRequest {
    Classes::TThreadMethod method;
    bool done;
    std::exception_ptr error;
};

struct ThreadState {
    SDL_mutex *lock = nullptr;
    SDL_cond *cond = nullptr;          // signalled when a request completes / at shutdown
    SDL_cond *queued = nullptr;        // signalled when a request is queued
    std::deque<SyncRequest *> queue;
    std::vector<Classes::TThread *> live;
    std::vector<Classes::TThread *> pendingStart;
    bool shutdown = false;
    SDL_threadID mainThread = 0;
};

ThreadState &state()
{
    static ThreadState s;
    if (!s.lock) {
        s.lock = SDL_CreateMutex();
        s.cond = SDL_CreateCond();
        s.queued = SDL_CreateCond();
    }
    return s;
}

// The main thread is the one that runs static initialisation.
struct MainThreadMark {
    MainThreadMark() { state().mainThread = SDL_ThreadID(); }
} s_mainMark;

} // namespace

bool IsMainThread() { return SDL_ThreadID() == state().mainThread; }
void SetMainThread() { state().mainThread = SDL_ThreadID(); }

// forms.cpp: the SDL user event that wakes the main loop.
void WakeMainThread();

bool WaitForSynchronize(unsigned ms)
{
    ThreadState &s = state();
    SDL_LockMutex(s.lock);
    bool running = false;
    for (Classes::TThread *t : s.live)
        running = running || !t->Suspended;
    if (running && s.queue.empty() && !s.shutdown)
        SDL_CondWaitTimeout(s.queued, s.lock, ms);
    SDL_UnlockMutex(s.lock);
    return running;
}

int CheckSynchronize()
{
    ThreadState &s = state();
    if (!IsMainThread())
        return 0;
    std::vector<Classes::TThread *> starts;
    std::deque<SyncRequest *> work;
    SDL_LockMutex(s.lock);
    starts.swap(s.pendingStart);
    work.swap(s.queue);
    SDL_UnlockMutex(s.lock);
    for (Classes::TThread *t : starts)
        t->Resume();
    int n = 0;
    for (SyncRequest *r : work) {
        try {
            r->method();
        } catch (...) {
            r->error = std::current_exception();
        }
        n++;
        SDL_LockMutex(s.lock);
        r->done = true;
        SDL_UnlockMutex(s.lock);
    }
    if (!work.empty())
        SDL_CondBroadcast(s.cond);
    return n;
}

} // namespace Vcl

namespace Classes {

struct TThreadAccess {
    static int SDLCALL Proc(void *p)
    {
        TThread *t = static_cast<TThread *>(p);
        try {
            if (!t->FTerminated)
                t->Execute();
        } catch (Sysutils::Exception &e) {
            std::fprintf(stderr, "[vcl] exception in thread: %s\n", e.Message.c_str());
        } catch (...) {
            std::fprintf(stderr, "[vcl] exception in thread\n");
        }
        bool freeIt = t->FFreeOnTerminate;
        int rv = t->FReturnValue;
        t->DoTerminate();
        t->FFinished = true;
        if (freeIt)
            delete t;
        return rv;
    }
    static bool Finished(TThread *t) { return t->FFinished; }
    static bool Running(TThread *t) { return t->FStarted && !t->FFinished; }
    static void MarkTerminated(TThread *t) { t->FTerminated = true; }
};

TThread::TThread(bool CreateSuspended)
    : FHandle(nullptr), FThreadID(0), FTerminated(false), FSuspended(true), FFreeOnTerminate(false),
      FFinished(false), FStarted(false), FReturnValue(0)
{
    Vcl::ThreadState &s = Vcl::state();
    SDL_LockMutex(s.lock);
    s.live.push_back(this);
    // Borland starts the thread here when !CreateSuspended, before the derived
    // constructor has run. Natively Execute is still pure at this point, so
    // the start is deferred to the next main-loop pump.
    if (!CreateSuspended)
        s.pendingStart.push_back(this);
    SDL_UnlockMutex(s.lock);
    if (!CreateSuspended)
        Vcl::WakeMainThread();
}

TThread::~TThread()
{
    if (FStarted && !FFinished && SDL_ThreadID() != FThreadID) {
        Terminate();
        WaitFor();
    }
    Vcl::ThreadState &s = Vcl::state();
    SDL_LockMutex(s.lock);
    s.live.erase(std::remove(s.live.begin(), s.live.end(), this), s.live.end());
    s.pendingStart.erase(std::remove(s.pendingStart.begin(), s.pendingStart.end(), this), s.pendingStart.end());
    SDL_UnlockMutex(s.lock);
    if (FHandle) {
        if (SDL_ThreadID() == FThreadID)
            SDL_DetachThread(static_cast<SDL_Thread *>(FHandle));   // FreeOnTerminate: deleting ourselves
        else
            SDL_WaitThread(static_cast<SDL_Thread *>(FHandle), nullptr);
    }
}

void TThread::Start()
{
    if (FStarted)
        return;
    FStarted = true;
    FSuspended = false;
    SDL_Thread *h = SDL_CreateThread(&TThreadAccess::Proc, "TThread", this);
    if (!h) {
        FStarted = false;
        FSuspended = true;
        throw EThread(AnsiString("Thread creation error: ") + SDL_GetError());
    }
    FHandle = h;
    FThreadID = (unsigned long)SDL_GetThreadID(h);
}

void TThread::Resume()
{
    Vcl::ThreadState &s = Vcl::state();
    SDL_LockMutex(s.lock);
    s.pendingStart.erase(std::remove(s.pendingStart.begin(), s.pendingStart.end(), this), s.pendingStart.end());
    SDL_UnlockMutex(s.lock);
    if (!FStarted)
        Start();
    FSuspended = false;
}

void TThread::Suspend()
{
    // Advisory only (see classes.hpp): SDL cannot suspend a thread.
    FSuspended = true;
}

int TThread::WaitFor()
{
    if (!FStarted)
        return FReturnValue;
    // On the main thread keep running Synchronize calls, or a thread blocked
    // in Synchronize could never finish.
    while (!TThreadAccess::Finished(this)) {
        if (Vcl::IsMainThread())
            Vcl::CheckSynchronize();
        SDL_Delay(1);
    }
    return FReturnValue;
}

void TThread::Synchronize(TThreadMethod Method)
{
    if (!Method)
        return;
    if (Vcl::IsMainThread()) {
        Method();
        return;
    }
    Vcl::ThreadState &s = Vcl::state();
    Vcl::SyncRequest req{Method, false, nullptr};
    SDL_LockMutex(s.lock);
    if (s.shutdown) {
        SDL_UnlockMutex(s.lock);
        return;
    }
    s.queue.push_back(&req);
    SDL_CondSignal(s.queued);
    SDL_UnlockMutex(s.lock);
    Vcl::WakeMainThread();
    SDL_LockMutex(s.lock);
    while (!req.done && !s.shutdown)
        SDL_CondWaitTimeout(s.cond, s.lock, 100);
    if (!req.done)
        s.queue.erase(std::remove(s.queue.begin(), s.queue.end(), &req), s.queue.end());
    SDL_UnlockMutex(s.lock);
    if (req.error)
        std::rethrow_exception(req.error);
}

void TThread::CallOnTerminate()
{
    if (OnTerminate)
        OnTerminate(this);
}

void TThread::DoTerminate()
{
    if (OnTerminate)
        Synchronize(TThreadMethod::Make<&TThread::CallOnTerminate>(this));
}

} // namespace Classes

namespace Vcl {

void ShutdownThreads(unsigned timeoutMs)
{
    ThreadState &s = state();
    // Terminate every thread and stop running Synchronize calls: a worker
    // blocked in (or entering) Synchronize returns at once, sees Terminated
    // and ends, so no game frame runs after Application->Run() has returned.
    // The lock keeps every pointer in s.live valid (the TThread destructor
    // unregisters under it, including FreeOnTerminate threads).
    SDL_LockMutex(s.lock);
    for (Classes::TThread *t : s.live)
        Classes::TThreadAccess::MarkTerminated(t);
    s.pendingStart.clear();
    s.shutdown = true;
    SDL_UnlockMutex(s.lock);
    SDL_CondBroadcast(s.cond);
    Uint32 until = SDL_GetTicks() + timeoutMs;
    for (;;) {
        SDL_LockMutex(s.lock);
        bool running = false;
        for (Classes::TThread *t : s.live)
            running = running || Classes::TThreadAccess::Running(t);
        SDL_UnlockMutex(s.lock);
        if (!running || SDL_TICKS_PASSED(SDL_GetTicks(), until))
            break;
        SDL_Delay(1);
    }
}

} // namespace Vcl

#else // __EMSCRIPTEN__

// ---- web build: a cooperative TThread on the one browser thread ----------------------------------
//
// The browser main thread cannot block, and real threads (pthreads) need SharedArrayBuffer, which
// means COOP/COEP headers and a main loop that still cannot wait on a condition variable. So on the
// web a TThread has no thread of its own:
//
//   - Start/Resume only marks it runnable.
//   - Every browser frame (forms.cpp, Vcl::WebFrame) calls RunThreadSlices(), which enters
//     Execute() on the main thread.
//   - Synchronize(Method) called from inside that Execute runs Method at once (it is already on the
//     main thread) and then throws CoopYield, which unwinds Execute back to RunThreadSlices. The next
//     frame enters Execute again.
//
// That is exact for the game's only thread, whose Execute (fn_40f684) is stateless:
//     while (!Terminated) Synchronize(Update);
// so each frame runs one Update (fn_40f674 -> fn_40c1b4, the frame tick). The tick is time based
// (fn_409dac catches up to 3 scene periods per call), so one call per requestAnimationFrame keeps
// the original speed. An Execute that keeps state in locals across Synchronize calls, or that
// catches (...) around Synchronize, would not survive this; the game has neither.
namespace Vcl {

namespace {

struct CoopYield {};

struct CoopState {
    std::vector<Classes::TThread *> live;
    std::vector<Classes::TThread *> pendingStart;
    Classes::TThread *current = nullptr;    // the thread whose Execute is on the stack
    bool shutdown = false;
};

CoopState &cstate()
{
    static CoopState s;
    return s;
}

} // namespace

bool IsMainThread() { return true; }
void SetMainThread() {}
void WakeMainThread();
bool WaitForSynchronize(unsigned) { return false; }

int CheckSynchronize()
{
    CoopState &s = cstate();
    std::vector<Classes::TThread *> starts;
    starts.swap(s.pendingStart);
    for (Classes::TThread *t : starts)
        t->Resume();
    return 0;
}

int RunThreadSlices();

} // namespace Vcl

namespace Classes {

struct TThreadAccess {
    // One slice of t's Execute. Returns true if Execute was entered.
    static bool Slice(TThread *t)
    {
        Vcl::CoopState &s = Vcl::cstate();
        if (!t->FStarted || t->FFinished || t->FSuspended || s.current)
            return false;
        bool finished = false;
        s.current = t;
        try {
            if (!t->FTerminated)
                t->Execute();
            finished = true;
        } catch (Vcl::CoopYield &) {
        } catch (Sysutils::Exception &e) {
            std::fprintf(stderr, "[vcl] exception in thread: %s\n", e.Message.c_str());
            finished = true;
        } catch (...) {
            std::fprintf(stderr, "[vcl] exception in thread\n");
            finished = true;
        }
        s.current = nullptr;
        if (finished) {
            bool freeIt = t->FFreeOnTerminate;
            t->DoTerminate();
            t->FFinished = true;
            if (freeIt)
                delete t;
        }
        return true;
    }
    static bool Runnable(TThread *t) { return t->FStarted && !t->FFinished && !t->FSuspended; }
    static void MarkTerminated(TThread *t) { t->FTerminated = true; }
};

TThread::TThread(bool CreateSuspended)
    : FHandle(nullptr), FThreadID(0), FTerminated(false), FSuspended(true), FFreeOnTerminate(false),
      FFinished(false), FStarted(false), FReturnValue(0)
{
    Vcl::CoopState &s = Vcl::cstate();
    s.live.push_back(this);
    if (!CreateSuspended)           // as natively: started on the next pump, once Execute is not pure
        s.pendingStart.push_back(this);
}

TThread::~TThread()
{
    if (FStarted && !FFinished && Vcl::cstate().current != this) {
        Terminate();
        WaitFor();
    }
    Vcl::CoopState &s = Vcl::cstate();
    s.live.erase(std::remove(s.live.begin(), s.live.end(), this), s.live.end());
    s.pendingStart.erase(std::remove(s.pendingStart.begin(), s.pendingStart.end(), this), s.pendingStart.end());
}

void TThread::Start()
{
    if (FStarted)
        return;
    FStarted = true;
    FSuspended = false;
    FThreadID = 1;
    FHandle = nullptr;          // no OS thread on the web
}

void TThread::Resume()
{
    Vcl::CoopState &s = Vcl::cstate();
    s.pendingStart.erase(std::remove(s.pendingStart.begin(), s.pendingStart.end(), this), s.pendingStart.end());
    if (!FStarted)
        Start();
    FSuspended = false;
}

void TThread::Suspend() { FSuspended = true; }

int TThread::WaitFor()
{
    // Run the thread's slices until its Execute returns (it sees Terminated). A thread that is
    // suspended, or whose Execute is on the stack (WaitFor from inside itself), cannot finish here.
    while (FStarted && !FFinished) {
        if (!TThreadAccess::Slice(this))
            break;
    }
    return FReturnValue;
}

void TThread::Synchronize(TThreadMethod Method)
{
    Vcl::CoopState &s = Vcl::cstate();
    if (s.current != this) {        // the main thread's own code: run directly, as VCL does
        if (Method && !s.shutdown)
            Method();
        return;
    }
    // From this thread's Execute: run the method (a nested Synchronize inside it is main-thread
    // code), then end this slice.
    s.current = nullptr;
    if (Method && !s.shutdown)
        Method();
    throw Vcl::CoopYield();
}

void TThread::CallOnTerminate()
{
    if (OnTerminate)
        OnTerminate(this);
}

void TThread::DoTerminate()
{
    if (OnTerminate)
        CallOnTerminate();
}

} // namespace Classes

namespace Vcl {

int RunThreadSlices()
{
    CoopState &s = cstate();
    std::vector<Classes::TThread *> run;
    for (Classes::TThread *t : s.live)
        if (Classes::TThreadAccess::Runnable(t))
            run.push_back(t);
    int n = 0;
    for (Classes::TThread *t : run) {
        // a slice may delete a FreeOnTerminate thread, or another thread: re-check membership
        if (std::find(s.live.begin(), s.live.end(), t) == s.live.end())
            continue;
        n += Classes::TThreadAccess::Slice(t) ? 1 : 0;
    }
    return n;
}

void ShutdownThreads(unsigned)
{
    CoopState &s = cstate();
    for (Classes::TThread *t : s.live)
        Classes::TThreadAccess::MarkTerminated(t);
    s.pendingStart.clear();
    s.shutdown = true;
}

} // namespace Vcl

#endif // __EMSCRIPTEN__
