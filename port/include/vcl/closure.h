// vcl/closure.h - Borland __closure as a portable C++17 delegate.
//
// A Borland closure is (code, this): a method pointer bound to an object.
// Vcl::Closure<R(Args...)> stores the object as void * plus a thunk that is
// instantiated for one callee, so it is two pointers wide, trivially
// copyable, and needs no member-function-pointer tricks.
//
// Binding: Vcl::Bind<F>(obj) where F is either
//   - a member function:    Vcl::Bind<&TStage::MouseMove>(stage)
//   - a free function whose first parameter takes the object pointer (the
//     way the matched source defines handlers, e.g. fn_40a6e4(TStage *, ...)):
//                           Vcl::Bind<fn_40a6e4>(stage)
// The callee's parameters only need to be convertible from the closure's
// (a TShiftState converts to the game's 1-byte ShiftState or to char, a
// TObject * to void *), exactly as the matched handlers are spelled.
//
// Source-side macros (the Borland forms are in docs/VCL_PORT.md; include/elf
// defines them for __BORLANDC__, this header for everything else):
//   ELF_CLOSURE(R, Name, (params))   typedef of a closure type
//   ELF_METHOD(obj, member, impl)    closure value: Borland `obj->member`,
//                                    native Vcl::Bind<impl>(obj)
#ifndef VCL_CLOSURE_H
#define VCL_CLOSURE_H

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace Vcl {

template <auto F, class T> struct Bound {
    T *obj;
};

template <auto F, class T> inline Bound<F, T> Bind(T *obj) { return Bound<F, T>{obj}; }

template <class Sig> class Closure;

namespace detail {
template <class R, class Fn, class... Args> constexpr bool InvocableAs()
{
    if constexpr (!std::is_invocable_v<Fn, Args...>)
        return false;
    else if constexpr (std::is_void_v<R>)
        return true;
    else
        return std::is_convertible_v<std::invoke_result_t<Fn, Args...>, R>;
}
} // namespace detail

// Fits<ClosureType, F, T>: can Bind<F>(T *) be stored in ClosureType?
template <class C, auto F, class T> struct Fits;
template <class R, class... A, auto F, class T> struct Fits<Closure<R(A...)>, F, T> {
    static constexpr bool value = detail::InvocableAs<R, decltype(F), T *, A...>();
};

template <class R, class... A> class Closure<R(A...)> {
public:
    typedef R (*Thunk)(void *, A...);

    Closure() : obj_(nullptr), thunk_(nullptr) {}
    Closure(std::nullptr_t) : obj_(nullptr), thunk_(nullptr) {}
    template <auto F, class T> Closure(Bound<F, T> b) : obj_(const_cast<void *>(static_cast<const void *>(b.obj))), thunk_(&Call<F, T>)
    {
        static_assert(Fits<Closure, F, T>::value, "the bound method does not fit this closure type");
    }
    template <auto F, class T> static Closure Make(T *obj) { return Closure(Bound<F, T>{obj}); }

    R operator()(A... a) const { return thunk_(obj_, std::forward<A>(a)...); }

    explicit operator bool() const { return thunk_ != nullptr; }
    bool operator!() const { return thunk_ == nullptr; }
    friend bool operator==(const Closure &x, const Closure &y) { return x.obj_ == y.obj_ && x.thunk_ == y.thunk_; }
    friend bool operator!=(const Closure &x, const Closure &y) { return !(x == y); }
    friend bool operator==(const Closure &x, std::nullptr_t) { return !x.thunk_; }
    friend bool operator!=(const Closure &x, std::nullptr_t) { return x.thunk_ != nullptr; }

    // TMethod.Data / TMethod.Code equivalents.
    void *Data() const { return obj_; }
    Thunk Code() const { return thunk_; }

private:
    template <auto F, class T> static R Call(void *o, A... a)
    {
        if constexpr (std::is_void_v<R>)
            std::invoke(F, static_cast<T *>(o), std::forward<A>(a)...);
        else
            return static_cast<R>(std::invoke(F, static_cast<T *>(o), std::forward<A>(a)...));
    }

    void *obj_;
    Thunk thunk_;
};

} // namespace Vcl

#if !defined(__BORLANDC__)
#ifndef ELF_CLOSURE
#define ELF_CLOSURE(R, Name, Params) typedef ::Vcl::Closure<R Params> Name
#endif
#ifndef ELF_METHOD
#define ELF_METHOD(Obj, Member, Impl) (::Vcl::Bind<Impl>(Obj))
#endif
#endif

#endif
