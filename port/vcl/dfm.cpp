// Binary DFM ("TPF0") reader: what TReader.ReadRootComponent does for a
// form, limited to the root object's properties and events.
//
// The game's form, RCDATA/TFORM1 in Elf Bowling.exe, decodes to:
//   object Form1: TForm1
//     Left = 326  Top = 233  BorderStyle = bsNone  Caption = 'Elf Bowl'
//     ClientHeight = 480  ClientWidth = 640  Color = clGray
//     Font.Charset = DEFAULT_CHARSET  Font.Color = clWindowText
//     Font.Height = -11  Font.Name = 'MS Sans Serif'  Font.Style = []
//     KeyPreview = True  Position = poScreenCenter
//     OnKeyDown = Form1KeyDown  OnKeyUp = Form1KeyUp2  OnPaint = FormPaint
//     PixelsPerInch = 96  TextHeight = 13
//   end
#include <vcl/vcl.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

namespace Vcl {

namespace {

enum {
    vaNull, vaList, vaInt8, vaInt16, vaInt32, vaExtended, vaString, vaIdent, vaFalse, vaTrue, vaBinary,
    vaSet, vaLString, vaNil, vaCollection, vaSingle, vaCurrency, vaDate, vaWString, vaInt64, vaUTF8String
};

struct Reader {
    const unsigned char *p, *end;
    bool ok = true;

    bool need(std::size_t n)
    {
        if ((std::size_t)(end - p) < n)
            ok = false;
        return ok;
    }
    unsigned u8() { return need(1) ? *p++ : 0; }
    int32_t i32()
    {
        if (!need(4))
            return 0;
        int32_t v = (int32_t)((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24);
        p += 4;
        return v;
    }
    void skip(std::size_t n)
    {
        if (need(n))
            p += n;
    }
    std::string sstr()
    {
        unsigned n = u8();
        if (!need(n))
            return std::string();
        std::string s((const char *)p, n);
        p += n;
        return s;
    }
    unsigned peek() { return need(1) ? *p : 0; }

    TDfmValue value()
    {
        TDfmValue v;
        unsigned t = u8();
        switch (t) {
        case vaInt8: v.kind = TDfmValue::Int; v.i = (int8_t)u8(); break;
        case vaInt16: {
            v.kind = TDfmValue::Int;
            unsigned lo = u8(), hi = u8();
            v.i = (int16_t)(lo | hi << 8);
            break;
        }
        case vaInt32: v.kind = TDfmValue::Int; v.i = i32(); break;
        case vaInt64: {
            v.kind = TDfmValue::Int;
            uint32_t lo = (uint32_t)i32(), hi = (uint32_t)i32();
            v.i = (long long)((uint64_t)hi << 32 | lo);
            break;
        }
        case vaString: v.kind = TDfmValue::String; v.s = sstr(); break;
        case vaLString:
        case vaUTF8String: {
            v.kind = TDfmValue::String;
            int32_t n = i32();
            if (n >= 0 && need((std::size_t)n)) {
                v.s.assign((const char *)p, n);
                p += n;
            }
            break;
        }
        case vaWString: {
            v.kind = TDfmValue::String;
            int32_t n = i32();
            for (int32_t k = 0; k < n && ok; k++) {
                unsigned lo = u8(), hi = u8();
                v.s += (char)(hi ? '?' : lo);      // Latin-1 subset
            }
            break;
        }
        case vaIdent: v.kind = TDfmValue::Ident; v.s = sstr(); break;
        case vaFalse: v.kind = TDfmValue::Bool; v.b = false; break;
        case vaTrue: v.kind = TDfmValue::Bool; v.b = true; break;
        case vaNil: v.kind = TDfmValue::Nil; break;
        case vaNull: v.kind = TDfmValue::Nil; break;
        case vaSet:
            v.kind = TDfmValue::Set;
            for (;;) {
                std::string e = sstr();
                if (!ok || e.empty())
                    break;
                v.set.push_back(e);
            }
            break;
        case vaList:
            v.kind = TDfmValue::List;
            while (ok && peek() != vaNull)
                value();
            u8();
            break;
        case vaBinary: v.kind = TDfmValue::Binary; skip((std::size_t)i32()); break;
        case vaExtended: v.kind = TDfmValue::Float; skip(10); break;
        case vaSingle: {
            v.kind = TDfmValue::Float;
            if (need(4)) {
                float f;
                std::memcpy(&f, p, 4);
                v.f = f;
                p += 4;
            }
            break;
        }
        case vaCurrency:
        case vaDate: v.kind = TDfmValue::Float; skip(8); break;
        case vaCollection:
            v.kind = TDfmValue::Unsupported;
            while (ok && peek() != vaNull) {
                unsigned t2 = peek();
                if (t2 == vaInt8 || t2 == vaInt16 || t2 == vaInt32)
                    value();            // item index
                if (u8() != vaList) {   // each item is a property list
                    ok = false;
                    break;
                }
                while (ok && peek() != vaNull) {
                    sstr();
                    value();
                }
                u8();
            }
            u8();
            break;
        default:
            std::fprintf(stderr, "[vcl] DFM: unknown value type %u\n", t);
            ok = false;
            break;
        }
        return v;
    }

    // Skip one object (a child component): header, properties, children.
    void skipObject()
    {
        if ((peek() & 0xf0) == 0xf0) {
            unsigned flags = u8();
            if (flags & 2)
                value();                // child position
        }
        std::string cls = sstr(), name = sstr();
        std::fprintf(stderr, "[vcl] DFM: child object %s: %s ignored\n", name.c_str(), cls.c_str());
        while (ok && peek() != vaNull) {
            sstr();
            value();
        }
        u8();
        while (ok && peek() != vaNull)
            skipObject();
        u8();
    }
};

} // namespace

bool detail::ReportMismatch(const char *method, int kind)
{
    std::fprintf(stderr, "[vcl] DFM: published method %s does not fit event kind %d\n", method, kind);
    return false;
}

bool ReadDfm(Classes::TComponent *root, const void *data, std::size_t size, System::TMetaClass *cls, void *obj)
{
    Reader r{static_cast<const unsigned char *>(data), static_cast<const unsigned char *>(data) + size};
    if (size < 4 || std::memcmp(data, "TPF0", 4) != 0) {
        std::fprintf(stderr, "[vcl] DFM: no TPF0 signature\n");
        return false;
    }
    r.p += 4;
    if ((r.peek() & 0xf0) == 0xf0) {
        unsigned flags = r.u8();
        if (flags & 2)
            r.value();
    }
    std::string className = r.sstr();
    std::string objName = r.sstr();
    if (!objName.empty())
        root->SetName(objName.c_str());
    while (r.ok && r.peek() != vaNull) {
        std::string prop = r.sstr();
        TDfmValue v = r.value();
        if (!r.ok)
            break;
        TEventKind kind = ekNone;
        void *slot = prop.compare(0, 2, "On") == 0 ? root->DfmEvent(prop.c_str(), &kind) : nullptr;
        if (slot && kind != ekNone) {
            if (v.kind != TDfmValue::Ident) {
                std::fprintf(stderr, "[vcl] DFM: %s: event value is not a method name\n", prop.c_str());
                continue;
            }
            // Borland raises EReadError ("Method '%s' not found"); the port
            // logs, leaves the event unassigned and continues.
            if (!cls || !cls->FindMethod || !cls->FindMethod(obj, v.s.c_str(), kind, slot))
                std::fprintf(stderr, "[vcl] DFM: %s = %s: method not published by %s\n", prop.c_str(), v.s.c_str(),
                             className.c_str());
            continue;
        }
        if (!root->DfmSetProperty(prop.c_str(), v))
            std::fprintf(stderr, "[vcl] DFM: %s.%s: property not supported, ignored\n", className.c_str(),
                         prop.c_str());
    }
    r.u8();
    while (r.ok && r.peek() != vaNull)
        r.skipObject();
    if (r.ok)
        r.u8();
    if (!r.ok)
        std::fprintf(stderr, "[vcl] DFM: truncated or malformed stream (%s)\n", className.c_str());
    root->Loaded();
    return r.ok;
}

const void *FindDfm(const char *className, std::size_t *size)
{
    HRSRC h = FindResourceA(System::HInstance, className, RT_RCDATA);
    if (!h)
        return nullptr;
    HGLOBAL g = LoadResource(System::HInstance, h);
    if (!g)
        return nullptr;
    *size = SizeofResource(System::HInstance, h);
    return LockResource(g);
}

} // namespace Vcl
