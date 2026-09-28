// vcl/property.h - Borland __property as proxy members.
//
//   __property AnsiString Title = {read=GetTitle, write=SetTitle};
// becomes
//   VCL_PROPERTY(TApplication, AnsiString, Title, &TApplication::GetTitle, &TApplication::SetTitle);
//
// The proxy holds the owner pointer (one pointer per property), converts to
// T by calling the getter, and assigns / compound-assigns through the setter,
// so `Application->Title = "..."` and `rs->Position -= 3` compile unchanged.
// Pass nullptr for a missing getter or setter (write-only / read-only).
// Field-backed properties ({read=FOnPaint, write=FOnPaint}) are plain public
// fields instead.
//
// A class with proxies cannot be copied (VCL objects never are). A class
// whose layout the game depends on (TStream) must not use VCL_PROPERTY; see
// VCL_EMBEDDED_PROPERTY in classes.hpp.
#ifndef VCL_PROPERTY_H
#define VCL_PROPERTY_H

#include <cstddef>
#include <type_traits>

namespace Vcl {

template <class O, class T, auto Get, auto Set> class Property {
    static constexpr bool kRead = !std::is_same_v<decltype(Get), std::nullptr_t>;
    static constexpr bool kWrite = !std::is_same_v<decltype(Set), std::nullptr_t>;

public:
    explicit Property(O *o) : o_(o) {}
    Property(const Property &) = delete;

    T get() const
    {
        static_assert(kRead, "write-only property");
        if constexpr (kRead)
            return (o_->*Get)();
        else
            return T();
    }
    void set(const T &v) const
    {
        static_assert(kWrite, "read-only property");
        if constexpr (kWrite)
            (o_->*Set)(v);
    }

    operator T() const { return get(); }
    T operator()() const { return get(); }
    T operator->() const { return get(); }     // pointer-typed properties: Prop->Member
    const Property &operator=(const T &v) const { set(v); return *this; }
    const Property &operator=(const Property &p) const { set(p.get()); return *this; }

    template <class U> const Property &operator+=(const U &v) const { set(get() + v); return *this; }
    template <class U> const Property &operator-=(const U &v) const { set(get() - v); return *this; }
    template <class U> const Property &operator*=(const U &v) const { set(get() * v); return *this; }
    template <class U> const Property &operator/=(const U &v) const { set(get() / v); return *this; }
    template <class U> const Property &operator|=(const U &v) const { set(get() | v); return *this; }
    template <class U> const Property &operator&=(const U &v) const { set(get() & v); return *this; }
    const Property &operator++() const { set(get() + 1); return *this; }
    const Property &operator--() const { set(get() - 1); return *this; }
    T operator++(int) const { T v = get(); set(v + 1); return v; }
    T operator--(int) const { T v = get(); set(v - 1); return v; }

private:
    O *o_;
};

} // namespace Vcl

#define VCL_PROPERTY(Owner, T, Name, Get, Set) \
    ::Vcl::Property<Owner, T, Get, Set> Name { this }

#endif
