// TSoundMgr: open wave-out and start playing a sound, 0x40c960.
// A function address stored through a DWORD_PTR cast goes via a register (mov edx,imm).
#include <elf/funcs.h>
#include <string.h>

extern "C" char fn_40c960(TSoundMgr *p, TSound *s)
{
    char ok = 0;
    s->used = 1;
    DWORD_PTR proc = (DWORD_PTR)fn_40c718;
    g_460208 = p;
    fn_40c75c(p);
    MMRESULT r = fn_40c6d0(&p->hwo, WAVE_MAPPER, &s->fmt, 0, 0, WAVE_FORMAT_QUERY);
    if (r == 0) {
        r = fn_40c6d0(&p->hwo, WAVE_MAPPER, &s->fmt, proc, 0, CALLBACK_FUNCTION);
        if (r == 0) {
            memset(&p->hdr, 0, sizeof(WAVEHDR));      // 0x20 on bcc32
            p->hdr.lpData = (char *)s->data;
            p->hdr.dwBufferLength = s->datasize;
            r = fn_40c6a0(p->hwo, &p->hdr, sizeof(WAVEHDR));
            if (r == 0) {
                r = fn_40c670(p->hwo, &p->hdr, sizeof(WAVEHDR));
                if (r == 0) {
                    p->playing = 1;
                    ok = 1;
                    p->endTime = fn_4024bc(s->msec);
                }
            }
        }
    }
    return ok;
}
