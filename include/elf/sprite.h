// elf/sprite.h - sprites: TGraphicSprite (with its talking casts), TButtonSprite,
// and TSpriteGroup. RTTI names: TGraphicSprite, TButtonSprite.
//
// Old local names -> canonical:
//   Sprite, Spr, Sprite2, Actor (range_4060d4, ctors_4052dc, sprites_409100),
//   TextSprite, MenuText, LaneSprite, EggSprite, Key, S24, Spr160
//                                              -> TGraphicSprite
//   Button (sprites_4092bc, about_40f7fc), MenuButton, Btn, TalkActor,
//   Actor (scene_409fc8, scene_40a1fc, sprites_408f84)     -> TButtonSprite
//   Group, SpriteGroup, TextGroup, LaneGroup, Group160     -> TSpriteGroup
//   Seq / Frame (sprites_407ffc/408940: the thing at 0x168) -> TSeq
//   Step                                                   -> TSeqStep
#ifndef ELF_SPRITE_H
#define ELF_SPRITE_H

#include <elf/types.h>

struct TStage;
struct TScene;
struct TGraphicCast;
struct TGraphicSprite;

// Timer / event callback: (scene, sprite). The engine passes the current
// scene (stage->scene) as the first argument, and the game's handlers are
// written as fn(TScene *self, TGraphicSprite *s). Old: SprFn, TimerFn,
// SpriteCb, SeqDone, BtnCb (all with void * or Scene * first).
typedef void (*SpriteCb)(TScene *scene, TGraphicSprite *s);
// Old spelling of onMoveDone (range_4060d4 called it as (int arg, sprite)).
// The arg is the scene that fn_4087c8 passes to fn_406ac4, so onMoveDone is
// an ordinary SpriteCb; nothing uses this typedef any more. Kept for
// source compatibility. Old: MoveDone.
typedef void (*MoveDoneCb)(int arg, TGraphicSprite *s);

// One step of an animation sequence (12 bytes).
struct TSeqStep {
    int x, y;                   // 0x00 move target
    unsigned char cast;         // 0x08 cast index to show (unsigned: see round1_401 round 2)
    char _pad9;
    unsigned short delay;       // 0x0a ms until the next step
};

// Animation sequence (0x18 bytes). Old: Seq; sprites_407ffc/408940 "Frame" (f10).
struct TSeq {
    int repeat;                 // 0x00 repeat count (old: f0)
    int used;                   // 0x04 steps used (old: f4)
    int n;                      // 0x08 step capacity
    int fc;                     // 0x0c
    SpriteCb done;              // 0x10 called when the sequence ends (old: SeqDone(void *scene, Actor*), f10)
    TSeqStep *steps;            // 0x14
};

// Sprite base class, ctor 0x40609c (sets type = 0, shown = 1, index = 0).
// No RTTI name was found for it; "TSprite" is ours.
struct TSprite {
    char shown;                 // 0x00 (old: visible, f0, Key::down). sprites_407ffc declared
                                //      it bool (fn_408908/408924 compare with `== true`):
                                //      ELF_AS(bool, s->shown) there.
    char _pad01[3];
    int index;                  // 0x04 index in the scene (old: f4)
    char name[0x19];            // 0x08 (strncpy 0x18)
    unsigned char type;         // 0x21 3 = TButtonSprite, 1 = set by init 0x406378 (old: f21).
                                //      Only stored by the sprite units (range_4060d4 declared
                                //      it char; stores are the same bytes).

    TSprite();                  // 0x40609c
};

ELF_CHECK_OFS(TSprite, index, 0x04);
ELF_CHECK_OFS(TSprite, name, 0x08);
ELF_CHECK_OFS(TSprite, type, 0x21);

// TGraphicSprite (0x384 bytes). ctors 0x4064d0 (name, stage) and 0x406520
// (name, stage, pos); init 0x406378; full reset 0x40610c; per-frame update
// 0x4081b8 (move) / 0x408940 (mouse).
struct TGraphicSprite : TSprite {
    int frame;                  // 0x024 current cast index (old: cur, f24)
    char animating;             // 0x028 has an overlay (old: f28)
    char _pad029[3];
    int nextTick;               // 0x02c tick of next frame (old: f2c)
    int f30;                    // 0x030
    int nextMove;               // 0x034 next move time (old: f34)
    int nextAnim;               // 0x038 next anim time (old: f38)
    int phase;                  // 0x03c toggles 0/1 each frame (old: f3c)
    int queueLen;               // 0x040 entries in queue[] (old: nqueue, f40)
    int due[4];                 // 0x044 timer ticks (see timerCb)
    int step;                   // 0x054 current step in the sequence
    int f58;                    // 0x058
    int stepTick;               // 0x05c
    int timer;                  // 0x060 (old: f60)
    char f64;                   // 0x064
    char f65;                   // 0x065
    char _pad066[2];
    TPoint moveFrom;            // 0x068 (old: f68)
    TPoint moveTo;              // 0x070 (old: f70)
    int moveProgress;           // 0x078 0..10000 (old: f78)
    int moveSpeed;              // 0x07c (old: f7c)
    int f80;                    // 0x080
    int f84;                    // 0x084
    SpriteCb onMoveDone;        // 0x088 set by 0x406970, called by 0x406ac4(s, scene) as
                                //       (scene, s); game units pass scene handlers (old: f88)
    SpriteCb onCell;            // 0x08c called when pos/1000 changes (old: f8c)
    TPoint grab;                // 0x090 position when grabbed, x = 999999999: none (old: f90)
    TPoint rclick;              // 0x098 (old: f98)
    union {                     // 0x0a0 velocity (range_4060d4: POINT fa0)
        struct { int vx, vy; };
        TPoint vel;
    };
    int maxvy;                  // 0x0a8 999999999 = unset (old: fa8)
    int ay;                     // 0x0ac accel y
    int dvy;                    // 0x0b0
    int ax;                     // 0x0b4 accel x (friction)
    int dvx;                    // 0x0b8
    TPoint fling;               // 0x0bc (old: fbc)
    int dragTime;               // 0x0c4 (old: fc4)
    char bounceX;               // 0x0c8
    char bounceY;               // 0x0c9
    char outX;                  // 0x0ca
    char outY;                  // 0x0cb
    TGraphicSprite *child;      // 0x0cc linked sprite (old: fcc; game_418274 Link())
    RECT bounds;                // 0x0d0 set from stage->screen (old: fd0; Windows::TRect in game_416f10)
    TRect clip;                 // 0x0e0 set from stage->screen. Declared as Windows::TRect
                                //       (game/egg/score units, Classes::Rect results).
                                //       sprites_* use ERect (copy ctor), range_4060d4 RECT:
                                //       ELF_AS(ERect, s->clip) / ELF_AS(RECT, s->clip).
    TPoint f0f0;                // 0x0f0 x = 999999999: unset (old: ff0, scene_409fc8 fpos)
    TPoint pos;                 // 0x0f8 position in pixels (old: Pt pos)
    union {                     // 0x100 position * 1000 (old: f100/f104, fpos; sprites_*: pos)
        struct { int x, y; };
        TPoint fpos;
    };
    SpriteCb timerCb[4];        // 0x108 timer callbacks (old: cb[4], int data[4])
    int queue[12];              // 0x118 queued sequence indices
    char enabled;               // 0x148 (old: f148; set to 1 on reset)
    char _pad149[3];
    SpriteCb onClick;           // 0x14c (old: f14c)
    SpriteCb onRClick;          // 0x150 (opening_41e01c: onclick)
    SpriteCb onGrab;            // 0x154
    SpriteCb onDrag;            // 0x158
    SpriteCb onDrop;            // 0x15c
    SpriteCb onLeave;           // 0x160 (old: f160, egg onDone)
    int nseq;                   // 0x164 (old: nframes)
    TSeq *seqs[24];             // 0x168 24 is right: fn_407ad8 refuses a 25th sequence, and
                                //       sprites_407ffc only indexes below nseq. (sprites_407ffc
                                //       and 408940 declared Frame *frames[26], which ran into
                                //       lane/period; 408940 never touched it.)
    int lane;                   // 0x1c8 (old: f1c8; score: lane index)
    int period;                 // 0x1cc frame period in ms; sound code stores the sound
                                //       duration here (old: f1cc)
    char f1d0;                  // 0x1d0
    char _pad1d1[3];
    TStage *stage;              // 0x1d4 (old: engine, game)
    char keepParms;             // 0x1d8 keep default parms (old: f1d8)
    char f1d9;                  // 0x1d9
    char animate;               // 0x1da (old: f1da; score declared bool)
    char move;                  // 0x1db (old: f1db)
    char f1dc;                  // 0x1dc
    char draggable;             // 0x1dd (old: f1dd)
    char clickable;             // 0x1de (old: f1de)
    char _pad1df;
    int movePeriod;             // 0x1e0 (old: f1e0)
    int animPeriod;             // 0x1e4 (old: f1e4)
    int animFirst;              // 0x1e8 first anim frame (old: f1e8)
    int animCount;              // 0x1ec anim frame count (old: f1ec)
    int ncasts;                 // 0x1f0 (old: count)
    TGraphicCast *casts[50];    // 0x1f4 (old: img, frames, f1f4; char * where the code
                                //        does byte arithmetic: cast at the use site)
    TGraphicCast *talkCasts[50];// 0x2bc talking / overlay casts (old: casts2, ovl)

    TGraphicSprite(const char *name, TStage *stage);              // 0x4064d0
    TGraphicSprite(const char *name, TStage *stage, TPoint pos);  // 0x406520

    // 0x4067e0 was matched as a real member (the index loads first).
    int AddTalkingIndex(const char *name, int idx);

    // Inline helpers the original calls (an inline member call has its own
    // codegen: a `this` temp and a copied argument, so use them only where
    // the unit was matched with them).
    void Link(TGraphicSprite *p) { child = p; }     // game_418274
    void SetClip(TRect r) { clip = r; }             // score_4120cc (fn_413168)
    TPoint GetPos();                                // score_4120cc (fn_413244): position in pixels
};
// Out of class, as score_4120cc was matched.
inline TPoint TGraphicSprite::GetPos() { return Classes_Point(x / 1000, y / 1000); }

ELF_CHECK_OFS(TGraphicSprite, frame, 0x024);
ELF_CHECK_OFS(TGraphicSprite, queueLen, 0x040);
ELF_CHECK_OFS(TGraphicSprite, moveFrom, 0x068);
ELF_CHECK_OFS(TGraphicSprite, onMoveDone, 0x088);
ELF_CHECK_OFS(TGraphicSprite, onCell, 0x08c);
ELF_CHECK_OFS(TGraphicSprite, vx, 0x0a0);
ELF_CHECK_OFS(TGraphicSprite, fling, 0x0bc);
ELF_CHECK_OFS(TGraphicSprite, bounceX, 0x0c8);
ELF_CHECK_OFS(TGraphicSprite, child, 0x0cc);
ELF_CHECK_OFS(TGraphicSprite, bounds, 0x0d0);
ELF_CHECK_OFS(TGraphicSprite, clip, 0x0e0);
ELF_CHECK_OFS(TGraphicSprite, pos, 0x0f8);
ELF_CHECK_OFS(TGraphicSprite, x, 0x100);
ELF_CHECK_OFS(TGraphicSprite, timerCb, 0x108);
ELF_CHECK_OFS(TGraphicSprite, queue, 0x118);
ELF_CHECK_OFS(TGraphicSprite, enabled, 0x148);
ELF_CHECK_OFS(TGraphicSprite, onClick, 0x14c);
ELF_CHECK_OFS(TGraphicSprite, onLeave, 0x160);
ELF_CHECK_OFS(TGraphicSprite, seqs, 0x168);
ELF_CHECK_OFS(TGraphicSprite, lane, 0x1c8);
ELF_CHECK_OFS(TGraphicSprite, f1d0, 0x1d0);
ELF_CHECK_OFS(TGraphicSprite, stage, 0x1d4);
ELF_CHECK_OFS(TGraphicSprite, keepParms, 0x1d8);
ELF_CHECK_OFS(TGraphicSprite, clickable, 0x1de);
ELF_CHECK_OFS(TGraphicSprite, movePeriod, 0x1e0);
ELF_CHECK_OFS(TGraphicSprite, ncasts, 0x1f0);
ELF_CHECK_OFS(TGraphicSprite, casts, 0x1f4);
ELF_CHECK_OFS(TGraphicSprite, talkCasts, 0x2bc);
ELF_CHECK_SIZE(TGraphicSprite, 0x384);

// Button (0x3cc bytes). ctor 0x409100 (stage, name, up cast, down cast, pos),
// reset 0x408f84, blink setup 0x409000, sounds 0x409068/0x4090b4, play the
// hover/click sound 0x4091ac/0x409234, per-frame update 0x4092bc.
// Scene helper 0x40a2c0 / TScene::fn_40a1fc create one. TSprite::type == 3.
//
// Earlier notes called the ctor's class "TalkActor"/"the talking actor"
// (sprites_409100, scene_409fc8, scene_40a1fc, sprites_408f84): it is this
// class. The fields 0x394-0x3c8 that sprites_408f84 sets (snd1..3, f3ac..)
// are the same ones 4092bc reads as the hover/click/blink state.
// Offsets 0x384-0x390 are named from the caller in 4092bc: 0x384 fires on
// stage->down, 0x388 on stage->up. menu_41008c/about_40f7fc/game_418274
// store their "click" handler at 0x388 and a "down" handler at 0x38c.
struct TButtonSprite : TGraphicSprite {
    SpriteCb onPress;           // 0x384 fires on stage->down (4092bc: onClick)
    SpriteCb onRelease;         // 0x388 fires on stage->up (4092bc: onRClick; menu/about/game: onClick)
    SpriteCb onEnter;           // 0x38c hover enter (menu: onDown)
    SpriteCb onExit;            // 0x390 hover leave (4092bc: onLeave)
    int hoverSnd, hoverSndArg;  // 0x394 played on hover enter (408f84: snd1; -2 = none)
    int clickSnd, clickSndArg;  // 0x39c played on click (408f84: snd2)
    int blinkSnd, blinkSndArg;  // 0x3a4 played when lit (408f84: snd3)
    char blink;                 // 0x3ac (408f84: f3ac)
    char _pad3ad[3];
    int onTime;                 // 0x3b0 (f3b0)
    int offTime;                // 0x3b4 (f3b4)
    int blinkCount;             // 0x3b8 (f3b8)
    int blinkN;                 // 0x3bc (f3bc)
    int lit;                    // 0x3c0 (f3c0)
    int nextTime;               // 0x3c4 (f3c4)
    int hover;                  // 0x3c8 (f3c8)

    TButtonSprite(TStage *stage, const char *name, const char *up, const char *down, TPoint pos);  // 0x409100
};

ELF_CHECK_OFS(TButtonSprite, onPress, 0x384);
ELF_CHECK_OFS(TButtonSprite, onRelease, 0x388);
ELF_CHECK_OFS(TButtonSprite, hoverSnd, 0x394);
ELF_CHECK_OFS(TButtonSprite, clickSnd, 0x39c);
ELF_CHECK_OFS(TButtonSprite, blinkSnd, 0x3a4);
ELF_CHECK_OFS(TButtonSprite, blink, 0x3ac);
ELF_CHECK_OFS(TButtonSprite, hover, 0x3c8);
ELF_CHECK_SIZE(TButtonSprite, 0x3cc);

// A named set of sprites whose names match a pattern (0x12c bytes).
// ctor 0x409844 (name, scene, pattern); members 0x40971c.. (show/hide/frames).
struct TSpriteGroup {
    char name[0x1c];            // 0x00
    int count;                  // 0x1c
    int f20;                    // 0x20
    int f24;                    // 0x24
    TScene *scene;              // 0x28
    TGraphicSprite *items[64];  // 0x2c

    TSpriteGroup(const char *name, TScene *scene, const char *pattern);  // 0x409844
    // matched as real members in sprites_40971c.cpp
    void fn_40971c();
    void fn_409754(int t);
    void fn_409790(int start, int n);
    void fn_4097e8(int start, int n, int t);
};

ELF_CHECK_OFS(TSpriteGroup, count, 0x1c);
ELF_CHECK_OFS(TSpriteGroup, scene, 0x28);
ELF_CHECK_OFS(TSpriteGroup, items, 0x2c);
ELF_CHECK_SIZE(TSpriteGroup, 0x12c);

#endif
