/* Portable WinMM subset: waveOut and mmioStringToFOURCC. Implemented in port/src/winmm.c. */
#ifndef PORT_MMSYSTEM_H
#define PORT_MMSYSTEM_H

#include "windows.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef UINT MMRESULT;
typedef DWORD FOURCC;
DECLARE_HANDLE(HWAVEOUT);
typedef HWAVEOUT *LPHWAVEOUT;

#define MMSYSERR_NOERROR 0
#define MMSYSERR_ERROR 1
#define MMSYSERR_BADDEVICEID 2
#define MMSYSERR_NOTENABLED 3
#define MMSYSERR_INVALHANDLE 5
#define MMSYSERR_NODRIVER 6
#define MMSYSERR_NOMEM 7
#define MMSYSERR_NOTSUPPORTED 8
#define MMSYSERR_INVALFLAG 10
#define MMSYSERR_INVALPARAM 11
#define WAVERR_BADFORMAT 32
#define WAVERR_STILLPLAYING 33
#define WAVERR_UNPREPARED 34

#define WAVE_MAPPER ((UINT)-1)
#define WAVE_FORMAT_QUERY 0x0001
#define WAVE_FORMAT_PCM 1
#define CALLBACK_TYPEMASK 0x00070000u
#define CALLBACK_NULL 0x00000000u
#define CALLBACK_WINDOW 0x00010000u
#define CALLBACK_THREAD 0x00020000u
#define CALLBACK_FUNCTION 0x00030000u
#define CALLBACK_EVENT 0x00050000u

#define WOM_OPEN 0x3BB
#define WOM_CLOSE 0x3BC
#define WOM_DONE 0x3BD

#define WHDR_DONE 0x00000001
#define WHDR_PREPARED 0x00000002
#define WHDR_BEGINLOOP 0x00000004
#define WHDR_ENDLOOP 0x00000008
#define WHDR_INQUEUE 0x00000010

/* WAVEFORMATEX is byte-packed in the Windows headers (18 bytes); the game relies on it. */
#pragma pack(push, 1)
typedef struct tWAVEFORMATEX {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;
#pragma pack(pop)
typedef const WAVEFORMATEX *LPCWAVEFORMATEX;

typedef struct wavehdr_tag {
    LPSTR lpData;
    DWORD dwBufferLength;
    DWORD dwBytesRecorded;
    DWORD_PTR dwUser;
    DWORD dwFlags;
    DWORD dwLoops;
    struct wavehdr_tag *lpNext;     /* driver-reserved: the shim's queue link */
    DWORD_PTR reserved;             /* driver-reserved: bytes already fed */
} WAVEHDR, *PWAVEHDR, *LPWAVEHDR;

typedef void (CALLBACK *LPWAVECALLBACK)(HWAVEOUT h, UINT msg, DWORD_PTR inst, DWORD_PTR p1, DWORD_PTR p2);

/* The cbwh size arguments are accepted and ignored: the game hard-codes the
 * 32-bit sizeof(WAVEHDR) (0x20), which differs from the 64-bit one. */
UINT WINAPI waveOutGetNumDevs(void);
MMRESULT WINAPI waveOutOpen(LPHWAVEOUT phwo, UINT dev, LPCWAVEFORMATEX fmt, DWORD_PTR cb,
                            DWORD_PTR inst, DWORD flags);
MMRESULT WINAPI waveOutClose(HWAVEOUT h);
MMRESULT WINAPI waveOutPrepareHeader(HWAVEOUT h, LPWAVEHDR hdr, UINT cbwh);
MMRESULT WINAPI waveOutUnprepareHeader(HWAVEOUT h, LPWAVEHDR hdr, UINT cbwh);
MMRESULT WINAPI waveOutWrite(HWAVEOUT h, LPWAVEHDR hdr, UINT cbwh);
MMRESULT WINAPI waveOutReset(HWAVEOUT h);

#define MMIO_TOUPPER 0x0010
#define mmioFOURCC(a, b, c, d) \
    ((DWORD)(BYTE)(a) | ((DWORD)(BYTE)(b) << 8) | ((DWORD)(BYTE)(c) << 16) | ((DWORD)(BYTE)(d) << 24))
FOURCC WINAPI mmioStringToFOURCCA(LPCSTR s, UINT flags);
#define mmioStringToFOURCC mmioStringToFOURCCA

#ifdef __cplusplus
}
#endif

#endif /* PORT_MMSYSTEM_H */
