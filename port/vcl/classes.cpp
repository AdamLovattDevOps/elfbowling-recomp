// Classes subset: TComponent ownership, streams, Rect/Point.
#include <vcl/vcl.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Classes {

// ---- TComponent ----------------------------------------------------------------

TComponent::TComponent(TComponent *AOwner) : FOwner(nullptr), FDestroying(false)
{
    if (AOwner)
        AOwner->InsertComponent(this);
}

TComponent::~TComponent()
{
    FDestroying = true;
    // Owned components are destroyed last-in first-out, as TComponent.DestroyComponents does.
    while (!FComponents.empty()) {
        TComponent *c = FComponents.back();
        FComponents.pop_back();
        c->FOwner = nullptr;
        delete c;
    }
    if (FOwner)
        FOwner->RemoveComponent(this);
}

void TComponent::InsertComponent(TComponent *c)
{
    if (c->FOwner)
        c->FOwner->RemoveComponent(c);
    FComponents.push_back(c);
    c->FOwner = this;
}

void TComponent::RemoveComponent(TComponent *c)
{
    auto it = std::find(FComponents.begin(), FComponents.end(), c);
    if (it != FComponents.end())
        FComponents.erase(it);
    c->FOwner = nullptr;
}

bool TComponent::DfmSetProperty(const char *name, const Vcl::TDfmValue &v)
{
    if (!std::strcmp(name, "Name") && v.kind == Vcl::TDfmValue::String) {
        SetName(v.s.c_str());
        return true;
    }
    // Design-time only, harmless to ignore.
    return !std::strcmp(name, "Tag") || !std::strcmp(name, "DesignSize");
}

void *TComponent::DfmEvent(const char *, Vcl::TEventKind *kind)
{
    *kind = Vcl::ekNone;
    return nullptr;
}

// ---- TStream -------------------------------------------------------------------

// The proxies are empty members of TStream: recover the owner from their
// offset. TStream is not standard-layout (it has a vptr), so offsetof is
// "conditionally supported"; clang and gcc support it.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
TStream *TStreamPositionProp::owner() const
{
    return reinterpret_cast<TStream *>(const_cast<char *>(reinterpret_cast<const char *>(this)) - offsetof(TStream, Position));
}
TStream *TStreamSizeProp::owner() const
{
    return reinterpret_cast<TStream *>(const_cast<char *>(reinterpret_cast<const char *>(this)) - offsetof(TStream, Size));
}
#pragma clang diagnostic pop

TStreamPositionProp::operator int() const { return owner()->GetPosition(); }
const TStreamPositionProp &TStreamPositionProp::operator=(int v) const
{
    owner()->SetPosition(v);
    return *this;
}
TStreamSizeProp::operator int() const { return owner()->GetSize(); }
const TStreamSizeProp &TStreamSizeProp::operator=(int v) const
{
    owner()->SetSize(v);
    return *this;
}

static_assert(sizeof(TStream) == sizeof(void *), "TStream must be a vptr and nothing else (see classes.hpp)");

void TStream::SetSize(int) {}

int TStream::GetSize()
{
    int pos = Seek(0, soFromCurrent);
    int size = Seek(0, soFromEnd);
    Seek(pos, soFromBeginning);
    return size;
}

void TStream::ReadBuffer(void *Buffer, int Count)
{
    if (Count != 0 && Read(Buffer, Count) != Count)
        throw EReadError("Stream read error");
}

void TStream::WriteBuffer(const void *Buffer, int Count)
{
    if (Count != 0 && Write(Buffer, Count) != Count)
        throw EWriteError("Stream write error");
}

int TStream::CopyFrom(TStream *Source, int Count)
{
    if (Count == 0) {
        Source->SetPosition(0);
        Count = Source->GetSize();
    }
    int total = Count;
    char buf[4096];
    while (Count > 0) {
        int n = Count > (int)sizeof buf ? (int)sizeof buf : Count;
        Source->ReadBuffer(buf, n);
        WriteBuffer(buf, n);
        Count -= n;
    }
    return total;
}

// ---- memory streams --------------------------------------------------------------

int TCustomMemoryStream::Read(void *Buffer, int Count)
{
    if (FPosition >= 0 && Count >= 0) {
        int n = FSize - FPosition;
        if (n > 0) {
            if (n > Count)
                n = Count;
            std::memcpy(Buffer, static_cast<char *>(FMemory) + FPosition, n);
            FPosition += n;
            return n;
        }
    }
    return 0;
}

int TCustomMemoryStream::Seek(int Offset, Word Origin)
{
    switch (Origin) {
    case soFromBeginning: FPosition = Offset; break;
    case soFromCurrent: FPosition += Offset; break;
    case soFromEnd: FPosition = FSize + Offset; break;
    }
    return FPosition;
}

void TCustomMemoryStream::SaveToStream(TStream *Stream)
{
    if (FSize)
        Stream->WriteBuffer(FMemory, FSize);
}

TMemoryStream::~TMemoryStream() { Clear(); }

void TMemoryStream::Clear()
{
    std::free(FMemory);
    SetPointer(nullptr, 0);
    FCapacity = 0;
    FPosition = 0;
}

void TMemoryStream::SetCapacity(int NewCapacity)
{
    NewCapacity = (NewCapacity + 0x1FFF) & ~0x1FFF;     // Delphi's 8 KB granularity
    if (NewCapacity == FCapacity)
        return;
    void *p = NewCapacity ? std::realloc(FMemory, NewCapacity) : (std::free(FMemory), nullptr);
    if (NewCapacity && !p)
        throw Sysutils::EOutOfMemory("Out of memory while expanding memory stream");
    FMemory = p;
    FCapacity = NewCapacity;
}

void TMemoryStream::SetSize(int NewSize)
{
    int old = FPosition;
    SetCapacity(NewSize);
    FSize = NewSize;
    if (old > NewSize)
        Seek(0, soFromEnd);
}

int TMemoryStream::Write(const void *Buffer, int Count)
{
    if (FPosition < 0 || Count < 0)
        return 0;
    int end = FPosition + Count;
    if (end > FSize) {
        if (end > FCapacity)
            SetCapacity(end);
        FSize = end;
    }
    std::memcpy(static_cast<char *>(FMemory) + FPosition, Buffer, Count);
    FPosition = end;
    return Count;
}

void TMemoryStream::LoadFromStream(TStream *Stream)
{
    Stream->SetPosition(0);
    int n = Stream->GetSize();
    SetSize(n);
    if (n)
        Stream->ReadBuffer(FMemory, n);
}

// ---- TResourceStream ----------------------------------------------------------------

static HINSTANCE inst(System::THandle h) { return reinterpret_cast<HINSTANCE>(h); }

TResourceStream::TResourceStream(System::THandle Instance, const AnsiString &ResName, const char *ResType)
{
    Initialize(inst(Instance), ResName.c_str(), ResType);
}
TResourceStream::TResourceStream(System::THandle Instance, int ResID, const char *ResType)
{
    Initialize(inst(Instance), MAKEINTRESOURCEA(ResID), ResType);
}
TResourceStream::TResourceStream(HINSTANCE Instance, const AnsiString &ResName, const char *ResType)
{
    Initialize(Instance, ResName.c_str(), ResType);
}
TResourceStream::TResourceStream(HINSTANCE Instance, int ResID, const char *ResType)
{
    Initialize(Instance, MAKEINTRESOURCEA(ResID), ResType);
}

void TResourceStream::Initialize(HINSTANCE Instance, const char *Name, const char *ResType)
{
    HResInfo = FindResourceA(Instance, Name, ResType);
    if (!HResInfo) {
        char id[16];
        const char *shown = Name;
        if (IS_INTRESOURCE(Name)) {
            std::snprintf(id, sizeof id, "#%u", (unsigned)(uintptr_t)Name);
            shown = id;
        }
        throw EResNotFound(AnsiString("Resource ") + shown + " not found");
    }
    HGlobal = LoadResource(Instance, HResInfo);
    if (!HGlobal)
        throw EResNotFound(AnsiString("Resource ") + (IS_INTRESOURCE(Name) ? "#" : Name) + " not found");
    SetPointer(LockResource(HGlobal), (int)SizeofResource(Instance, HResInfo));
}

int TResourceStream::Write(const void *, int)
{
    throw EStreamError("Can't write to a read-only resource stream");
}

// ---- Rect / Point -------------------------------------------------------------------

Windows::TRect Rect(int ALeft, int ATop, int ARight, int ABottom) { return Windows::TRect(ALeft, ATop, ARight, ABottom); }
Windows::TRect Bounds(int ALeft, int ATop, int AWidth, int AHeight)
{
    return Windows::TRect(ALeft, ATop, ALeft + AWidth, ATop + AHeight);
}
Windows::TPoint Point(int AX, int AY)
{
    Windows::TPoint p;
    p.x = AX;
    p.y = AY;
    return p;
}

} // namespace Classes
