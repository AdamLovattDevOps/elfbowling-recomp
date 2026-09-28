// vcl/system.hpp - System unit subset: TObject, AnsiString, Set<>, TMetaClass,
// HInstance and the Delphi scalar names. See docs/VCL_PORT.md.
#ifndef SystemHPP
#define SystemHPP

#include <vcl/sysmac.h>
#include <vcl/closure.h>
#include <vcl/property.h>

#include <cstddef>
#include <cstring>
#include <string>
#include <type_traits>

namespace Classes { class TComponent; }

namespace System {

typedef unsigned char Byte;
typedef unsigned short Word;
typedef unsigned int Cardinal;
typedef int Integer;
typedef int Longint;
typedef bool Boolean;
typedef char Char;
typedef long double Extended;
// THandle is Integer in Delphi 3. Pointer-sized here so that handles fit.
typedef intptr_t THandle;

// TObject has no virtual functions and no data. That keeps Classes::TStream
// at "vtable pointer + nothing", so a game subclass (TPackedStream) has the
// same layout as the game's non-VCL view of it (TPackedStreamData: void *vmt
// first), and TStream::Read is vtable slot 1 as the game's raw VMT calls
// (st->vt[1]) expect. Polymorphic roots (TStream, TComponent, TThread,
// Exception) declare their own virtual destructors.
class TObject {
public:
    TObject() {}
    ~TObject() {}
};

// Delphi `set of T` over [Lo..Hi] (sysset.h). Bit e is bit (e % 8) of byte
// (e / 8 - Lo / 8), as in BCB. Converts to any 1-byte trivially copyable
// class (the game's ShiftState) and to unsigned char, for handlers that take
// the set as `char`.
template <class T, unsigned char Lo, unsigned char Hi> class Set {
public:
    enum { kBytes = Hi / 8 - Lo / 8 + 1 };
    Set() { std::memset(Data, 0, sizeof Data); }
    Set &operator<<(T e) { if (in(e)) Data[idx(e)] |= bit(e); return *this; }
    Set &operator>>(T e) { if (in(e)) Data[idx(e)] &= (unsigned char)~bit(e); return *this; }
    bool Contains(T e) const { return in(e) && (Data[idx(e)] & bit(e)) != 0; }
    bool Contains(int e) const { return Contains((T)e); }
    Set &Clear() { std::memset(Data, 0, sizeof Data); return *this; }
    bool Empty() const { for (int i = 0; i < kBytes; i++) if (Data[i]) return false; return true; }
    Set operator+(const Set &o) const { Set r; for (int i = 0; i < kBytes; i++) r.Data[i] = Data[i] | o.Data[i]; return r; }
    Set operator*(const Set &o) const { Set r; for (int i = 0; i < kBytes; i++) r.Data[i] = Data[i] & o.Data[i]; return r; }
    Set operator-(const Set &o) const { Set r; for (int i = 0; i < kBytes; i++) r.Data[i] = Data[i] & (unsigned char)~o.Data[i]; return r; }
    bool operator==(const Set &o) const { return std::memcmp(Data, o.Data, sizeof Data) == 0; }
    bool operator!=(const Set &o) const { return !(*this == o); }

    operator unsigned char() const { return Data[0]; }
    template <class S, class = std::enable_if_t<std::is_class_v<S> && std::is_trivially_copyable_v<S> &&
                                                 sizeof(S) == kBytes && !std::is_same_v<S, Set>>>
    operator S() const
    {
        S s;
        std::memcpy(&s, Data, sizeof s);
        return s;
    }
    static Set FromByte(unsigned char b) { Set s; s.Data[0] = b; return s; }

    unsigned char Data[kBytes];

private:
    static bool in(T e) { return (int)e >= Lo && (int)e <= Hi; }
    static int idx(T e) { return (int)e / 8 - Lo / 8; }
    static unsigned char bit(T e) { return (unsigned char)(1u << ((int)e % 8)); }
};

// AnsiString (dstring.h subset). Value semantics; c_str() is never NULL.
// Indexing is 1-based, as in BCB.
class AnsiString {
public:
    AnsiString() {}
    AnsiString(const char *s) : s_(s ? s : "") {}
    AnsiString(const char *s, int n) : s_(s ? std::string(s, n) : std::string()) {}
    AnsiString(const std::string &s) : s_(s) {}
    AnsiString(char c) : s_(1, c) {}
    AnsiString(int v) : s_(std::to_string(v)) {}
    AnsiString(unsigned v) : s_(std::to_string(v)) {}
    AnsiString(long v) : s_(std::to_string(v)) {}
    AnsiString(double v);

    const char *c_str() const { return s_.c_str(); }
    const char *data() const { return s_.c_str(); }
    int Length() const { return (int)s_.size(); }
    bool IsEmpty() const { return s_.empty(); }
    char &operator[](int i) { return s_[i - 1]; }
    char operator[](int i) const { return s_[i - 1]; }

    AnsiString &operator+=(const AnsiString &o) { s_ += o.s_; return *this; }
    friend AnsiString operator+(const AnsiString &a, const AnsiString &b) { return AnsiString(a.s_ + b.s_); }
    friend bool operator==(const AnsiString &a, const AnsiString &b) { return a.s_ == b.s_; }
    friend bool operator!=(const AnsiString &a, const AnsiString &b) { return a.s_ != b.s_; }
    friend bool operator<(const AnsiString &a, const AnsiString &b) { return a.s_ < b.s_; }
    friend bool operator>(const AnsiString &a, const AnsiString &b) { return a.s_ > b.s_; }

    AnsiString SubString(int index, int count) const;
    int Pos(const AnsiString &sub) const;
    AnsiString UpperCase() const;
    AnsiString LowerCase() const;
    AnsiString Trim() const;
    int ToInt() const;
    int ToIntDef(int def) const;
    AnsiString &SetLength(int n) { s_.resize(n); return *this; }
    AnsiString &Insert(const AnsiString &str, int index) { s_.insert(index - 1, str.s_); return *this; }
    AnsiString &Delete(int index, int count) { s_.erase(index - 1, count); return *this; }
    int AnsiCompareIC(const AnsiString &o) const;
    static AnsiString StringOfChar(char c, int n) { return AnsiString(std::string(n, c)); }
    int printf(const char *fmt, ...);
    AnsiString &sprintf(const char *fmt, ...);

    const std::string &str() const { return s_; }

private:
    std::string s_;
};

// Metaclass (`class of TComponent`): what __classid(T) yields and
// TApplication::CreateForm takes. Filled in by VCL_REGISTER_CLASS(T).
class TMetaClass {
public:
    const char *ClassName;                                      // "TForm1"; also the DFM resource name
    std::size_t InstanceSize;
    Classes::TComponent *(*Construct)(void *mem, Classes::TComponent *owner);   // placement-new T(owner)
    void (*Destroy)(void *obj);                                 // delete (T *)obj
    // Published-method lookup for DFM event binding (VCL_PUBLISHED); may be NULL.
    bool (*FindMethod)(void *obj, const char *name, int kind, void *closure);
};

extern HINSTANCE HInstance;     // SysInit::HInstance (non-NULL; the shim ignores module handles)
extern HINSTANCE MainInstance;

} // namespace System

namespace Vcl {
// Class registry behind __classid / VCL_REGISTER_CLASS.
void RegisterClass(System::TMetaClass *cls);
System::TMetaClass *FindClass(const char *name);    // aborts with a message if unknown
System::TMetaClass *LookupClass(const char *name);  // NULL if unknown
}

#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace System;
#endif

#endif
