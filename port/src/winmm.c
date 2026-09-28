/* waveOut on SDL2 audio.
 *
 * One SDL output device (S16 stereo at $PORT_AUDIO_RATE, default 44100) is
 * opened on first use and shared. Each HWAVEOUT is a logical stream with an
 * SDL_AudioStream that converts its WAVEFORMATEX to the device format; the SDL
 * audio callback mixes all open streams.
 *
 * Completion: when the last converted byte of a header has been pulled into
 * the device buffer, the header gets WHDR_DONE (WHDR_INQUEUE cleared) and the
 * CALLBACK_FUNCTION receives WOM_DONE, on the SDL audio thread, as Windows
 * calls it on a driver thread. waveOutReset completes pending headers the
 * same way, synchronously on the caller's thread.
 *
 * $PORT_AUDIO_DUMP=<file> also writes the mixed output (raw S16 stereo at the
 * device rate) for tests. */
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shim.h"
#include "mmsystem.h"

#define MAX_STREAMS 16

typedef struct WaveOut {
    int used;
    WAVEFORMATEX fmt;
    DWORD_PTR cb, inst;
    DWORD flags;
    SDL_AudioStream *stream;
    WAVEHDR *head, *tail;
    int flushed;
} WaveOut;

static WaveOut s_streams[MAX_STREAMS];
static SDL_AudioDeviceID s_dev;
static SDL_AudioSpec s_spec;
static int s_devFailed;
static FILE *s_dump;            /* $PORT_AUDIO_DUMP: raw S16 stereo mixer output */

static void notify(WaveOut *h, UINT msg, WAVEHDR *hdr)
{
    if ((h->flags & CALLBACK_TYPEMASK) == CALLBACK_FUNCTION && h->cb)
        ((LPWAVECALLBACK)h->cb)((HWAVEOUT)h, msg, h->inst, (DWORD_PTR)hdr, 0);
}

/* Mark the head header done and advance. Caller holds the device lock. */
static void complete_head(WaveOut *h)
{
    WAVEHDR *hdr = h->head;
    h->head = hdr->lpNext;
    if (!h->head)
        h->tail = NULL;
    hdr->lpNext = NULL;
    hdr->reserved = 0;
    hdr->dwFlags = (hdr->dwFlags & ~(DWORD)WHDR_INQUEUE) | WHDR_DONE;
    h->flushed = 0;
    notify(h, WOM_DONE, hdr);
}

/* Pull up to len bytes of device-format audio from one stream. */
static int pull(WaveOut *h, Uint8 *out, int len)
{
    int got = 0;
    while (got < len) {
        int avail = SDL_AudioStreamAvailable(h->stream);
        if (avail > 0) {
            int r = SDL_AudioStreamGet(h->stream, out + got, len - got);
            if (r <= 0)
                break;
            got += r;
            continue;
        }
        WAVEHDR *hdr = h->head;
        if (!hdr)
            break;
        DWORD fed = (DWORD)hdr->reserved;
        if (fed < hdr->dwBufferLength) {
            DWORD chunk = hdr->dwBufferLength - fed;
            if (chunk > 4096)
                chunk = 4096;
            chunk -= chunk % (h->fmt.nBlockAlign ? h->fmt.nBlockAlign : 1);
            if (!chunk)
                chunk = hdr->dwBufferLength - fed;
            SDL_AudioStreamPut(h->stream, hdr->lpData + fed, (int)chunk);
            hdr->reserved = fed + chunk;
        } else if (!h->flushed && !hdr->lpNext) {
            /* last queued buffer: release what the resampler holds back */
            SDL_AudioStreamFlush(h->stream);
            h->flushed = 1;
        } else {
            complete_head(h);
        }
    }
    return got;
}

static void SDLCALL mix(void *ud, Uint8 *out, int len)
{
    (void)ud;
    static Uint8 *tmp;
    static int tmplen;
    memset(out, 0, len);
    if (tmplen < len) {
        free(tmp);
        tmp = malloc(len);
        tmplen = tmp ? len : 0;
        if (!tmp)
            return;
    }
    Sint16 *o = (Sint16 *)out, *t = (Sint16 *)tmp;
    for (int i = 0; i < MAX_STREAMS; i++) {
        WaveOut *h = &s_streams[i];
        if (!h->used || !h->stream)
            continue;
        int got = pull(h, tmp, len);
        for (int k = 0; k < got / 2; k++) {
            int v = o[k] + t[k];
            o[k] = (Sint16)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
        }
    }
    if (s_dump)
        fwrite(out, 1, len, s_dump);
}

static int device_open(void)
{
    if (s_dev)
        return 0;
    if (s_devFailed || port_sdl_init(SDL_INIT_AUDIO))
        return -1;
    SDL_AudioSpec want;
    SDL_zero(want);
    const char *rate = getenv("PORT_AUDIO_RATE");
    want.freq = rate && atoi(rate) > 0 ? atoi(rate) : 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = mix;
    s_dev = SDL_OpenAudioDevice(NULL, 0, &want, &s_spec, 0);
    if (!s_dev) {
        port_log("waveOut: SDL_OpenAudioDevice failed: %s", SDL_GetError());
        s_devFailed = 1;
        return -1;
    }
    port_log("waveOut: SDL audio '%s' %d Hz S16 stereo", SDL_GetCurrentAudioDriver(), s_spec.freq);
    const char *dump = getenv("PORT_AUDIO_DUMP");
    if (dump && *dump && !(s_dump = fopen(dump, "wb")))
        port_log("waveOut: cannot write PORT_AUDIO_DUMP %s", dump);
    SDL_PauseAudioDevice(s_dev, 0);
    return 0;
}

void port_winmm_shutdown(void)
{
    if (s_dev) {
        SDL_CloseAudioDevice(s_dev);
        s_dev = 0;
    }
    if (s_dump) {
        fclose(s_dump);
        s_dump = NULL;
    }
    for (int i = 0; i < MAX_STREAMS; i++) {
        if (s_streams[i].stream)
            SDL_FreeAudioStream(s_streams[i].stream);
        memset(&s_streams[i], 0, sizeof s_streams[i]);
    }
}

static WaveOut *valid(HWAVEOUT hw)
{
    WaveOut *h = (WaveOut *)hw;
    return h >= &s_streams[0] && h < &s_streams[MAX_STREAMS] && h->used ? h : NULL;
}

UINT WINAPI waveOutGetNumDevs(void)
{
    const char *ns = getenv("PORT_NOSOUND");
    if (ns && *ns && *ns != '0')
        return 0;
    return device_open() == 0 ? 1 : 0;
}

static int format_ok(const WAVEFORMATEX *f)
{
    return f && f->wFormatTag == WAVE_FORMAT_PCM && (f->nChannels == 1 || f->nChannels == 2) &&
           (f->wBitsPerSample == 8 || f->wBitsPerSample == 16) && f->nSamplesPerSec >= 1000 &&
           f->nSamplesPerSec <= 192000;
}

MMRESULT WINAPI waveOutOpen(LPHWAVEOUT phwo, UINT dev, LPCWAVEFORMATEX fmt, DWORD_PTR cb,
                            DWORD_PTR inst, DWORD flags)
{
    if (dev != WAVE_MAPPER && dev != 0)
        return MMSYSERR_BADDEVICEID;
    if (!format_ok(fmt))
        return WAVERR_BADFORMAT;
    if (device_open())
        return MMSYSERR_NODRIVER;
    if (flags & WAVE_FORMAT_QUERY)
        return MMSYSERR_NOERROR;
    DWORD ct = flags & CALLBACK_TYPEMASK;
    if (ct != CALLBACK_NULL && ct != CALLBACK_FUNCTION) {
        port_log("waveOutOpen: callback type %#x unsupported (only NULL/FUNCTION)", (unsigned)ct);
        return MMSYSERR_INVALFLAG;
    }
    if (!phwo)
        return MMSYSERR_INVALPARAM;
    SDL_LockAudioDevice(s_dev);
    WaveOut *h = NULL;
    for (int i = 0; i < MAX_STREAMS; i++) {
        if (!s_streams[i].used) {
            h = &s_streams[i];
            break;
        }
    }
    if (!h) {
        SDL_UnlockAudioDevice(s_dev);
        return MMSYSERR_NOMEM;
    }
    memset(h, 0, sizeof *h);
    h->fmt = *fmt;
    h->cb = cb;
    h->inst = inst;
    h->flags = flags;
    h->stream = SDL_NewAudioStream(fmt->wBitsPerSample == 8 ? AUDIO_U8 : AUDIO_S16LSB, (Uint8)fmt->nChannels,
                                   (int)fmt->nSamplesPerSec, s_spec.format, s_spec.channels, s_spec.freq);
    if (!h->stream) {
        SDL_UnlockAudioDevice(s_dev);
        port_log("waveOutOpen: SDL_NewAudioStream failed: %s", SDL_GetError());
        return MMSYSERR_NOMEM;
    }
    h->used = 1;
    SDL_UnlockAudioDevice(s_dev);
    *phwo = (HWAVEOUT)h;
    notify(h, WOM_OPEN, NULL);
    return MMSYSERR_NOERROR;
}

MMRESULT WINAPI waveOutPrepareHeader(HWAVEOUT hw, LPWAVEHDR hdr, UINT cbwh)
{
    WaveOut *h = valid(hw);
    (void)cbwh;
    if (!h)
        return MMSYSERR_INVALHANDLE;
    if (!hdr || !hdr->lpData)
        return MMSYSERR_INVALPARAM;
    hdr->dwFlags |= WHDR_PREPARED;
    return MMSYSERR_NOERROR;
}

MMRESULT WINAPI waveOutUnprepareHeader(HWAVEOUT hw, LPWAVEHDR hdr, UINT cbwh)
{
    WaveOut *h = valid(hw);
    (void)cbwh;
    if (!h)
        return MMSYSERR_INVALHANDLE;
    if (!hdr)
        return MMSYSERR_INVALPARAM;
    SDL_LockAudioDevice(s_dev);
    DWORD fl = hdr->dwFlags;
    SDL_UnlockAudioDevice(s_dev);
    if (fl & WHDR_INQUEUE)
        return WAVERR_STILLPLAYING;
    hdr->dwFlags &= ~(DWORD)WHDR_PREPARED;
    return MMSYSERR_NOERROR;
}

MMRESULT WINAPI waveOutWrite(HWAVEOUT hw, LPWAVEHDR hdr, UINT cbwh)
{
    WaveOut *h = valid(hw);
    (void)cbwh;
    if (!h)
        return MMSYSERR_INVALHANDLE;
    if (!hdr)
        return MMSYSERR_INVALPARAM;
    if (!(hdr->dwFlags & WHDR_PREPARED))
        return WAVERR_UNPREPARED;
    if (hdr->dwFlags & WHDR_INQUEUE)
        return WAVERR_STILLPLAYING;
    if (hdr->dwFlags & (WHDR_BEGINLOOP | WHDR_ENDLOOP))
        port_log("waveOutWrite: looping flags ignored");
    SDL_LockAudioDevice(s_dev);
    hdr->dwFlags = (hdr->dwFlags & ~(DWORD)WHDR_DONE) | WHDR_INQUEUE;
    hdr->lpNext = NULL;
    hdr->reserved = 0;
    if (h->tail)
        h->tail->lpNext = hdr;
    else
        h->head = hdr;
    h->tail = hdr;
    h->flushed = 0;
    SDL_UnlockAudioDevice(s_dev);
    return MMSYSERR_NOERROR;
}

MMRESULT WINAPI waveOutReset(HWAVEOUT hw)
{
    WaveOut *h = valid(hw);
    if (!h)
        return MMSYSERR_INVALHANDLE;
    SDL_LockAudioDevice(s_dev);
    SDL_AudioStreamClear(h->stream);
    while (h->head)
        complete_head(h);
    SDL_UnlockAudioDevice(s_dev);
    return MMSYSERR_NOERROR;
}

MMRESULT WINAPI waveOutClose(HWAVEOUT hw)
{
    WaveOut *h = valid(hw);
    if (!h)
        return MMSYSERR_INVALHANDLE;
    SDL_LockAudioDevice(s_dev);
    if (h->head) {
        SDL_UnlockAudioDevice(s_dev);
        return WAVERR_STILLPLAYING;
    }
    h->used = 0;
    SDL_UnlockAudioDevice(s_dev);
    notify(h, WOM_CLOSE, NULL);
    SDL_LockAudioDevice(s_dev);
    SDL_FreeAudioStream(h->stream);
    h->stream = NULL;
    SDL_UnlockAudioDevice(s_dev);
    return MMSYSERR_NOERROR;
}

FOURCC WINAPI mmioStringToFOURCCA(LPCSTR s, UINT flags)
{
    char c[4] = {' ', ' ', ' ', ' '};
    for (int i = 0; i < 4 && s && s[i]; i++) {
        c[i] = s[i];
        if ((flags & MMIO_TOUPPER) && c[i] >= 'a' && c[i] <= 'z')
            c[i] = (char)(c[i] - 'a' + 'A');
    }
    return mmioFOURCC(c[0], c[1], c[2], c[3]);
}
