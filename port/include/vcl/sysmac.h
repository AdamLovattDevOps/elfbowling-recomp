// vcl/sysmac.h - the Borland C++Builder language extensions, for a native
// (clang, 64-bit) build. See docs/VCL_PORT.md.
//
//   __fastcall      empty (port/include/windows.h)
//   __published     public
//   __classid(T)    the registered metaclass of T (VCL_REGISTER_CLASS)
//   __declspec(..)  empty off Windows (delphiclass, delphireturn, package,
//                   pascalimplementation are only in the old local VCL copies)
//   __thread        native GNU __thread (same meaning, same position);
//                   thread_local on MSVC
//   USEFORM, USERES, USEUNIT, ...   declarations / nothing
//   PACKAGE, DELPHICLASS, HIDESBASE  empty; DYNAMIC -> virtual
//   __closure       cannot be a macro: use ELF_CLOSURE (vcl/closure.h)
//   __property      cannot be a macro: the classes in port/include/vcl
//                   declare properties as proxy members (vcl/property.h)
//   #pragma hdrstop / package / option / resource / link: build with
//                   -Wno-unknown-pragmas
#ifndef VCL_SYSMAC_H
#define VCL_SYSMAC_H

#include <windows.h>

#ifndef __BORLANDC__

#ifndef __published
#define __published public
#endif

#if !defined(_WIN32) && !defined(__declspec)
#define __declspec(...)
#endif

// clang and gcc accept GNU __thread after the type (`extern T __thread x;`)
// and it is Borland's meaning; only MSVC needs the C++11 spelling.
#if defined(_MSC_VER) && !defined(__clang__)
#define __thread thread_local
#endif

#define PACKAGE
#define DELPHICLASS
#define HIDESBASE
#define DYNAMIC virtual

namespace System { class TMetaClass; }
namespace Vcl { System::TMetaClass *FindClass(const char *name); }
#define __classid(T) (::Vcl::FindClass(#T))

// USEFORM declares the form class. SYSDEFS.H also declares `extern TForm1
// *Form1`; here the game supplies the variable itself, because Form1 is the
// game global g_4601c0 (see docs/VCL_PORT.md), and FormName would be macro-
// expanded in that declaration.
#define USEFORM(FileName, FormName) class T##FormName
#define USEFORMNS(FileName, UnitName, FormName) \
    namespace UnitName { class T##FormName; } using namespace UnitName
#define USEDATAMODULE(FileName, DataModuleName) \
    class T##DataModuleName;                    \
    extern T##DataModuleName *DataModuleName
// The rest reference nothing (SYSDEFS.H: extern DummyThatIsNeverReferenced).
#define VCL_USE_DUMMY_ extern int vcl_DummyThatIsNeverReferenced
#define USEUNIT(ModName) VCL_USE_DUMMY_
#define USEOBJ(FileName) VCL_USE_DUMMY_
#define USERC(FileName) VCL_USE_DUMMY_
#define USEASM(FileName) VCL_USE_DUMMY_
#define USEDEF(FileName) VCL_USE_DUMMY_
#define USERES(FileName) VCL_USE_DUMMY_
#define USETLB(FileName) VCL_USE_DUMMY_
#define USELIB(FileName) VCL_USE_DUMMY_
#define USEFILE(FileName) VCL_USE_DUMMY_

#endif /* !__BORLANDC__ */

#endif
