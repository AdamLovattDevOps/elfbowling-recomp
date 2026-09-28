// TSound::LoadResource (0x40c1f0): read a WAV "fmt "/"data" chunk from the packed resources
// (TPackedStream) or, without them, from a "Wave" TResourceStream. Real <vcl/classes.hpp>.
// `tag = tag / 256` gives the sar/jns form; `tag /= 256` gives idiv.
#include <vcl/classes.hpp>
#include <elf/funcs.h>
#include <string.h>

extern "C" bool fn_40c1f0(TSound *snd, const char *name)
{
    bool ok = false;
    if (g_455524) {
        Classes::TStream *s = fn_40d4bc(g_455524, name);
        if (s) {
            int tag;
            int fmt = mmioStringToFOURCC("fmt", 0);
            s->Read(&tag, 4);
            for (int i = 0; i < 100; i++) {
                if (tag == fmt)
                    break;
                tag = tag / 256;
                s->Read((char *)&tag + 3, 1);
            }
            int len;
            s->Read(&len, 4);
            memset(&snd->fmt, 0, 0x12);
            s->Read(&snd->fmt, len);
            fmt = mmioStringToFOURCC("data", 0);
            s->Read(&tag, 4);
            for (int j = 0; j < 100; j++) {
                if (tag == fmt)
                    break;
                tag = tag / 256;
                s->Read((char *)&tag + 3, 1);
            }
            s->Read(&snd->datasize, 4);
            snd->data = fn_402338(snd->datasize, "TSound::LoadResource");
            s->Read(snd->data, snd->datasize);
            ok = true;
            fn_40d568(g_455524, s);
        }
    } else {
        Classes::TResourceStream *rs = new Classes::TResourceStream((intptr_t)HInstance, name, "Wave");
        int tag2;
        int fmt2 = mmioStringToFOURCC("fmt", 0);
        for (int k = 0; k < 100; k++) {
            rs->Read(&tag2, 4);
            if (tag2 == fmt2)
                break;
            rs->Position -= 3;
        }
        int len2;
        rs->Read(&len2, 4);
        memset(&snd->fmt, 0, 0x12);
        rs->Read(&snd->fmt, len2);
        fmt2 = mmioStringToFOURCC("data", 0);
        for (int m = 0; m < 100; m++) {
            rs->Read(&tag2, 4);
            if (tag2 == fmt2)
                break;
            rs->Position -= 3;
        }
        rs->Read(&snd->datasize, 4);
        snd->data = fn_402338(snd->datasize, "TSound::LoadResource");
        rs->Read(snd->data, snd->datasize);
        delete rs;
        ok = true;
    }
    return ok;
}
