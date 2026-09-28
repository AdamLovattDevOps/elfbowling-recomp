// vcl/sysutils.hpp - SysUtils subset: Exception and the few descendants the
// VCL subset raises.
#ifndef SysUtilsHPP
#define SysUtilsHPP

#include <vcl/system.hpp>

namespace Sysutils {

class Exception : public System::TObject {
public:
    explicit Exception(const System::AnsiString &Msg) : Message(Msg), HelpContext(0) {}
    Exception(const Exception &o) : System::TObject(), Message(o.Message), HelpContext(o.HelpContext) {}
    virtual ~Exception() {}
    static Exception CreateFmt(const char *fmt, ...);

    System::AnsiString Message;     // __property Message = {read=FMessage, write=FMessage}
    int HelpContext;
};

#define VCL_EXCEPTION_CLASS(Name, Base)                                   \
    class Name : public Base {                                            \
    public:                                                               \
        explicit Name(const System::AnsiString &Msg) : Base(Msg) {}       \
    }

VCL_EXCEPTION_CLASS(EAbort, Exception);
VCL_EXCEPTION_CLASS(EOutOfMemory, Exception);
VCL_EXCEPTION_CLASS(EConvertError, Exception);

System::AnsiString IntToStr(int v);
int StrToInt(const System::AnsiString &s);
System::AnsiString Format(const char *fmt, ...);

} // namespace Sysutils

#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Sysutils;
#endif

#endif
