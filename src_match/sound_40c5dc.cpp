// Wave-out sound player helpers, 0x40c5dc-0x40c880.
#include <elf/funcs.h>

extern "C" void fn_40c5dc(TSound *s, int flags)
{
    if (s) {
        fn_402380(s->data, "TSound::~TSound");
        if (flags & 1)
            fn_448a2c(s);
    }
}

extern "C" void fn_40c60c(HWAVEOUT h)
{
    if (g_460208->enabled)
        waveOutReset(h);
}

extern "C" void fn_40c628(HWAVEOUT h, WAVEHDR *hdr, UINT sz)
{
    if (g_460208->enabled)
        waveOutUnprepareHeader(h, hdr, sz);
}

extern "C" void fn_40c64c(HWAVEOUT *h)
{
    if (g_460208->enabled)
        waveOutClose(*h);
    *h = 0;
}

extern "C" MMRESULT fn_40c670(HWAVEOUT h, WAVEHDR *hdr, UINT sz)
{
    MMRESULT r = 0;
    if (g_460208->enabled)
        r = waveOutWrite(h, hdr, sz);
    return r;
}

extern "C" MMRESULT fn_40c6a0(HWAVEOUT h, WAVEHDR *hdr, UINT sz)
{
    MMRESULT r = 0;
    if (g_460208->enabled)
        r = waveOutPrepareHeader(h, hdr, sz);
    return r;
}

extern "C" MMRESULT fn_40c6d0(HWAVEOUT *h, UINT dev, WAVEFORMATEX *fmt, DWORD_PTR cb, DWORD_PTR inst, DWORD flags)
{
    MMRESULT r = 0;
    *h = (HWAVEOUT)1;
    if (g_460208->enabled) {
        *h = 0;
        r = waveOutOpen(h, dev, fmt, cb, inst, flags);
    }
    return r;
}

extern "C" void CALLBACK fn_40c718(HWAVEOUT h, UINT msg, DWORD_PTR inst, DWORD_PTR p1, DWORD_PTR p2)
{
    if (msg == WOM_DONE)
        g_460208->done = g_460208->queue[0].seq;
}

extern "C" void fn_40c740()
{
    g_460208->done = g_460208->queue[0].seq;
}

extern "C" void fn_40c75c(TSoundMgr *p)
{
    if (p->playing) {
        if (p->done == 0)
            fn_40c60c(p->hwo);
        fn_40c628(p->hwo, &p->hdr, 0x20);
        fn_40c64c(&p->hwo);
        TGraphicSprite *s = p->queue[0].sprite;
        if (s && s->type == 1) {
            TGraphicSprite *t = s;
            fn_40779c(t);
        }
    }
    p->playing = 0;
    p->done = 0;
    p->endTime = 0;
}

extern "C" void fn_40c804(TSoundMgr *p, char flag)
{
    SoundDoneCb cb = 0;
    TGraphicSprite *a = 0;
    void *b = 0;
    fn_40c75c(p);
    if (!flag && p->queue[0].cb) {
        a = p->queue[0].sprite;
        b = p->queue[0].owner;
        cb = p->queue[0].cb;
    }
    fn_40c880(p, flag);
    if (cb)
        cb(b, a);
}
