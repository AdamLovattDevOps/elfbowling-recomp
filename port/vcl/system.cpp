// System, SysUtils and Graphics subset: AnsiString, HInstance, the class
// registry, Exception helpers, colours.
#include <vcl/vcl.h>

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <strings.h>

namespace System {

static struct HINSTANCE__ s_instance;
HINSTANCE HInstance = &s_instance;
HINSTANCE MainInstance = &s_instance;

AnsiString::AnsiString(double v)
{
    char buf[64];
    std::snprintf(buf, sizeof buf, "%g", v);
    s_ = buf;
}

AnsiString AnsiString::SubString(int index, int count) const
{
    if (index < 1)
        index = 1;
    if (count <= 0 || index > (int)s_.size())
        return AnsiString();
    return AnsiString(s_.substr(index - 1, count));
}

int AnsiString::Pos(const AnsiString &sub) const
{
    std::string::size_type p = s_.find(sub.s_);
    return p == std::string::npos ? 0 : (int)p + 1;
}

AnsiString AnsiString::UpperCase() const
{
    std::string r = s_;
    for (char &c : r)
        c = (char)std::toupper((unsigned char)c);
    return AnsiString(r);
}

AnsiString AnsiString::LowerCase() const
{
    std::string r = s_;
    for (char &c : r)
        c = (char)std::tolower((unsigned char)c);
    return AnsiString(r);
}

AnsiString AnsiString::Trim() const
{
    std::string::size_type b = 0, e = s_.size();
    while (b < e && (unsigned char)s_[b] <= ' ')
        b++;
    while (e > b && (unsigned char)s_[e - 1] <= ' ')
        e--;
    return AnsiString(s_.substr(b, e - b));
}

int AnsiString::ToIntDef(int def) const
{
    const char *p = s_.c_str();
    char *end;
    long v = std::strtol(p, &end, 0);
    if (end == p || *end)
        return def;
    return (int)v;
}

int AnsiString::ToInt() const
{
    const char *p = s_.c_str();
    char *end;
    long v = std::strtol(p, &end, 0);
    if (end == p || *end)
        throw Sysutils::EConvertError("'" + *this + "' is not a valid integer value");
    return (int)v;
}

int AnsiString::AnsiCompareIC(const AnsiString &o) const
{
    return strcasecmp(s_.c_str(), o.s_.c_str());
}

static std::string vformat(const char *fmt, va_list ap)
{
    va_list ap2;
    va_copy(ap2, ap);
    int n = std::vsnprintf(nullptr, 0, fmt, ap2);
    va_end(ap2);
    if (n < 0)
        return std::string();
    std::string r((size_t)n, '\0');
    std::vsnprintf(&r[0], (size_t)n + 1, fmt, ap);
    return r;
}

int AnsiString::printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    s_ = vformat(fmt, ap);
    va_end(ap);
    return (int)s_.size();
}

AnsiString &AnsiString::sprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    s_ = vformat(fmt, ap);
    va_end(ap);
    return *this;
}

} // namespace System

namespace Sysutils {

Exception Exception::CreateFmt(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::string s = System::vformat(fmt, ap);
    va_end(ap);
    return Exception(System::AnsiString(s));
}

System::AnsiString IntToStr(int v) { return System::AnsiString(v); }
int StrToInt(const System::AnsiString &s) { return s.ToInt(); }

System::AnsiString Format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::string s = System::vformat(fmt, ap);
    va_end(ap);
    return System::AnsiString(s);
}

} // namespace Sysutils

namespace Graphics {

// Windows 98 "Windows Standard" scheme, COLOR_SCROLLBAR (0) .. COLOR_BTNHIGHLIGHT (20).
static const int k_sysColors[21] = {
    0xC0C0C0, 0x808000, 0x800000, 0x808080, 0xC0C0C0, 0xFFFFFF, 0x000000, 0x000000,
    0x000000, 0xFFFFFF, 0xC0C0C0, 0xC0C0C0, 0x808080, 0x800000, 0xFFFFFF, 0xC0C0C0,
    0x808080, 0x808080, 0x000000, 0xC0C0C0, 0xFFFFFF,
};

int ColorToRGB(TColor c)
{
    if ((unsigned)c & 0x80000000u) {
        unsigned i = (unsigned)c & 0xff;
        return i < 21 ? k_sysColors[i] : 0;
    }
    return c & 0xFFFFFF;
}

static const struct { const char *name; TColor value; } k_colorNames[] = {
    {"clBlack", clBlack}, {"clMaroon", clMaroon}, {"clGreen", clGreen}, {"clOlive", clOlive},
    {"clNavy", clNavy}, {"clPurple", clPurple}, {"clTeal", clTeal}, {"clGray", clGray},
    {"clSilver", clSilver}, {"clRed", clRed}, {"clLime", clLime}, {"clYellow", clYellow},
    {"clBlue", clBlue}, {"clFuchsia", clFuchsia}, {"clAqua", clAqua}, {"clLtGray", clLtGray},
    {"clDkGray", clDkGray}, {"clWhite", clWhite}, {"clNone", clNone}, {"clDefault", clDefault},
    {"clScrollBar", clScrollBar}, {"clBackground", clBackground}, {"clActiveCaption", clActiveCaption},
    {"clInactiveCaption", clInactiveCaption}, {"clMenu", clMenu}, {"clWindow", clWindow},
    {"clWindowFrame", clWindowFrame}, {"clMenuText", clMenuText}, {"clWindowText", clWindowText},
    {"clCaptionText", clCaptionText}, {"clActiveBorder", clActiveBorder},
    {"clInactiveBorder", clInactiveBorder}, {"clAppWorkSpace", clAppWorkSpace},
    {"clHighlight", clHighlight}, {"clHighlightText", clHighlightText}, {"clBtnFace", clBtnFace},
    {"clBtnShadow", clBtnShadow}, {"clGrayText", clGrayText}, {"clBtnText", clBtnText},
    {"clInactiveCaptionText", clInactiveCaptionText}, {"clBtnHighlight", clBtnHighlight},
};

bool IdentToColor(const char *ident, TColor *out)
{
    for (const auto &c : k_colorNames)
        if (!strcasecmp(c.name, ident)) {
            *out = c.value;
            return true;
        }
    return false;
}

} // namespace Graphics

namespace Vcl {

static std::map<std::string, System::TMetaClass *> &registry()
{
    static std::map<std::string, System::TMetaClass *> m;
    return m;
}

static std::string lower(const char *s)
{
    std::string r(s);
    for (char &c : r)
        c = (char)std::tolower((unsigned char)c);
    return r;
}

void RegisterClass(System::TMetaClass *cls) { registry()[lower(cls->ClassName)] = cls; }

System::TMetaClass *LookupClass(const char *name)
{
    auto it = registry().find(lower(name));
    return it == registry().end() ? nullptr : it->second;
}

System::TMetaClass *FindClass(const char *name)
{
    System::TMetaClass *c = LookupClass(name);
    if (!c) {
        std::fprintf(stderr, "[vcl] __classid(%s): class not registered (add VCL_REGISTER_CLASS(%s))\n", name, name);
        std::abort();
    }
    return c;
}

} // namespace Vcl
