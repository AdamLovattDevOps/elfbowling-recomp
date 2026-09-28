// elf/sound.h - TSoundMgr (0x5a8 bytes) and TSound (0x38 bytes).
// RTTI names: TSoundMgr, TSound. Global: g_460208 (see game.h); also
// reached as stage->sound.
//
// Old local names: Player, Sounds -> TSoundMgr; Sound -> TSound;
// SndEntry -> TSoundSlot; QEntry -> TSoundQueueEntry.
#ifndef ELF_SOUND_H
#define ELF_SOUND_H

#include <elf/types.h>
#include <mmsystem.h>

// DWORD_PTR / intptr_t / uintptr_t on bcc32: elf/types.h.

struct TGraphicSprite;

// Called when a queued sound finishes: (owner, sprite). Old: DoneCb.
// The owner is an opaque cookie: the game passes the scene (TScene *) or 0,
// fn_40ccd0 compares it and fn_40c804 hands it back as the first argument.
// It and the callback are pointers (the old sound units had `int owner`,
// `int cb`); every caller passes a pointer or 0.
typedef void (*SoundDoneCb)(void *arg, TGraphicSprite *s);

// A loaded WAV (0x38 bytes). ctor 0x40c544 (name), dtor 0x40c5dc,
// load 0x40c1f0 (packed resource or "Wave" resource).
struct TSound {
    char used;                  // 0x00 (old: f0)
    char name[0x19];            // 0x01 (sound_40ca94/40cbf0 declared name[0x33] up to 0x34)
    WAVEFORMATEX fmt;           // 0x1a
    int msec;                   // 0x2c length in ms
    int datasize;               // 0x30
    void *data;                 // 0x34 (sound_40c960: char *)

    TSound(const char *name);   // 0x40c544
    ~TSound();                  // 0x40c5dc (deleting dtor matched as a free fn)
};

ELF_CHECK_OFS(TSound, name, 0x01);
ELF_CHECK_OFS(TSound, fmt, 0x1a);
ELF_CHECK_OFS(TSound, msec, 0x2c);
ELF_CHECK_OFS(TSound, data, 0x34);
ELF_CHECK_SIZE(TSound, 0x38);

// One loaded sound (8 bytes). Old: SndEntry.
struct TSoundSlot {
    TSound *snd;                // +0
    char keep;                  // +4 survives the per-scene purge
    char _pad5[3];
};

// One queued play request (0x1c bytes). Old: QEntry.
struct TSoundQueueEntry {
    void *owner;                // +0x00 opaque owner cookie (old: int owner; sound_40c5dc: qarg)
    TSound *snd;                // +0x04
    int id;                     // +0x08
    int seq;                    // +0x0c
    int prio;                   // +0x10
    TGraphicSprite *sprite;     // +0x14 (sound_40c5dc: qsprite)
    SoundDoneCb cb;             // +0x18 (old: int cb)
};

// The wave-out player. ctor 0x40d1ac; play 0x40cb34 (by name), 0x40cae0 (by
// id); stop 0x40c75c; queue step 0x40c880 (member); load 0x40cfb0.
struct TSoundMgr {
    int nsounds;                // 0x000
    TSoundSlot sounds[100];     // 0x004
    char playing;               // 0x324
    char _pad325[3];
    int seq;                    // 0x328
    int endTime;                // 0x32c (old: f32c)
    void *loopOwner;            // 0x330 owner cookie of the looping sound (old: int)
    int loopId;                 // 0x334
    char enabled;               // 0x338 wave output available
    char _pad339[3];
    HWAVEOUT hwo;               // 0x33c
    WAVEHDR hdr;                // 0x340
    int nqueue;                 // 0x360
    int done;                   // 0x364 set from queue[0].seq by the WOM_DONE callback
    TSoundQueueEntry queue[20]; // 0x368 (sound_40c5dc: qarg/q1/q2/f374/q4/qsprite/qcb = queue[0])
    int defA, defAarg;          // 0x598 default sound A (id, arg)
    int defB, defBarg;          // 0x5a0 default sound B (id, arg)

    TSoundMgr();                // 0x40d1ac (old: Player())
    char fn_40c880(char flag);  // matched as a real member (sound_40c880.cpp)
};

ELF_CHECK_SIZE(TSoundQueueEntry, 0x1c);
ELF_CHECK_SIZE(TSoundSlot, 8);

ELF_CHECK_OFS(TSoundMgr, sounds, 0x004);
ELF_CHECK_OFS(TSoundMgr, playing, 0x324);
ELF_CHECK_OFS(TSoundMgr, endTime, 0x32c);
ELF_CHECK_OFS(TSoundMgr, loopOwner, 0x330);
ELF_CHECK_OFS(TSoundMgr, enabled, 0x338);
ELF_CHECK_OFS(TSoundMgr, hwo, 0x33c);
ELF_CHECK_OFS(TSoundMgr, hdr, 0x340);
ELF_CHECK_OFS(TSoundMgr, nqueue, 0x360);
ELF_CHECK_OFS(TSoundMgr, done, 0x364);
ELF_CHECK_OFS(TSoundMgr, queue, 0x368);
ELF_CHECK_OFS(TSoundMgr, defA, 0x598);
ELF_CHECK_SIZE(TSoundMgr, 0x5a8);

#endif
