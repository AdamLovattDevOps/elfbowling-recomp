// TPackedStream::Read (__fastcall): inflate into the output buffer and copy out, 0x40dee0.
#include <elf/funcs.h>
#include <string.h>

extern "C" int __fastcall fn_40dee0(TPackedStreamData *s, char *buf, int n)
{
    int want = n;
    int err = 0;
    while (err == 0 && n > 0) {
        int avail = s->outsize - s->z.avail_out - s->used;
        if (avail == 0) {
            s->z.next_out = s->out;
            s->z.avail_out = s->outsize;
            err = fn_42099c(&s->z, 1);
            s->used = 0;
        }
        avail = s->outsize - s->z.avail_out - s->used;
        if (err == 0 || err == 1) {
            int k = tmin(avail, n);
            memmove(buf, s->out + s->used, k);
            buf += k;
            s->used += k;
            n -= k;
        }
    }
    return want - n;
}
