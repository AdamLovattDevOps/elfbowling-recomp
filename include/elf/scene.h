// elf/scene.h - TScene (0x770 bytes): a screen of the game with its sprites
// and VCL buttons. No RTTI name was found for the scene; the stage calls it
// a scene ("Start of Scene "), so TScene is ours.
//
// Old local names -> TScene: Scene, Screen (menu/egg/about/score), Game
// (game_*/opening_*/exit_411af4/unit_411), ScreenObj, MenuScreen.
// (range_401d0c's Screen and init_40f350's Game are the TStage.)
//
// ctor 0x4098e4 (name, fps, start, start2); start 0x409a6c; stop 0x409b44;
// add sprite 0x409bcc; find sprite 0x40a520 (index) / 0x40a598 (pointer).
#ifndef ELF_SCENE_H
#define ELF_SCENE_H

#include <elf/types.h>

struct TStage;
struct TScene;
struct TGraphicSprite;
struct TButtonSprite;

typedef void (*SceneCb)(TScene *sc);                     // old: SceneCb, SceneFn (void *)
typedef void (*SceneStartCb)(TScene *sc, char again);    // old: SceneCb2 (the loaders take char: a bool changes 0x409a6c's call)
// Key handler at 0x24/0x28 as the stage calls it (engine_40bd3c).
typedef void (*SceneKeyCb)(TScene *sc, void *sender, unsigned short &key, ShiftState shift);

// The VCL control a TSceneButton wraps (a Controls::TControl), as far as
// scene_409c50 touches it: a Delphi VMT whose slot 0x7c it calls after
// clearing the byte at 0x120. Stand-in, like TStageForm.
struct TSceneCtl;
typedef void (__fastcall *TSceneCtlFn)(TSceneCtl *c);
struct TSceneCtl {
    TSceneCtlFn *vt;            // 0x000 Delphi VMT
    char _unk004[0x120 - 4];
    char f120;                  // 0x120
};

// A VCL control shown on top of the scene (0x1c bytes). ctor 0x4099d4
// (8 bytes; funcs.tsv labels the identical-code lib entry).
struct TSceneButton {
    TSceneCtl *ctl;             // 0x00 Controls::TControl * (scene_409c50 calls through its VMT)
    int first;                  // 0x04 first sprite index to test (old: f4)
    char visible;               // 0x08
    char _pad09[3];
    RectPod rect;               // 0x0c (scene_409c50 uses it as ERect: ELF_AS(ERect, b->rect))

    TSceneButton();             // 0x4099d4
};

ELF_CHECK_OFS(TSceneButton, visible, 0x08);
ELF_CHECK_OFS(TSceneButton, rect, 0x0c);
ELF_CHECK_SIZE(TSceneButton, 0x1c);

struct TScene {
    char active;                // 0x000
    char _pad001[3];
    int startTime;              // 0x004
    int stopTime;               // 0x008
    int ticks;                  // 0x00c (old: f0c)
    int f10;                    // 0x010
    TScene *retScene;           // 0x014 scene to return to (old: f14)
    // 0x18/0x1c are the scene's own mouse handlers: fn_409dac calls them
    // once per tick when stage->down / stage->up is set and no button
    // sprite took the click. `load`/`f1c` are the older names (kept).
    union {
        SceneCb load;           // 0x018 (old: f18; game_418274: f18)
        SceneCb onDown;         // 0x018 mouse down on the background
    };
    union {
        SceneCb f1c;            // 0x01c (old: MenuScreen::onClick)
        SceneCb onUp;           // 0x01c mouse up on the background
    };
    char started;               // 0x020 started before (old: f20)
    char _pad021[3];
    SceneKeyCb onKeyDown;       // 0x024 (old: f24)
    SceneKeyCb onKeyUp;         // 0x028 (old: f28)
    SceneStartCb onStart;       // 0x02c
    SceneCb onStart2;           // 0x030
    int period;                 // 0x034 tick period (old: f34)
    int nsprites;               // 0x038
    char name[0x100];           // 0x03c
    TStage *stage;              // 0x13c (old: engine, app, sprites)
    TGraphicSprite *sprites[255];   // 0x140
    int nbuttons;               // 0x53c
    TSceneButton buttons[20];   // 0x540

    // 0x4098e4. `start` is stored to onStart (0x2c) and `start2` to
    // onStart2 (0x30).
    TScene(const char *name, int fps, SceneStartCb start, SceneCb start2);
    // matched as real members (scene_40a520.cpp, scene_40a1fc.cpp)
    int fn_40a520(const char *name);
    TGraphicSprite *fn_40a598(const char *name);
    TButtonSprite *fn_40a1fc(const char *name, const char *cast1, const char *cast2, TPoint pos);
};

ELF_CHECK_OFS(TScene, ticks, 0x00c);
ELF_CHECK_OFS(TScene, retScene, 0x014);
ELF_CHECK_OFS(TScene, onDown, 0x018);
ELF_CHECK_OFS(TScene, onUp, 0x01c);
ELF_CHECK_OFS(TScene, started, 0x020);
ELF_CHECK_OFS(TScene, onKeyDown, 0x024);
ELF_CHECK_OFS(TScene, onStart, 0x02c);
ELF_CHECK_OFS(TScene, period, 0x034);
ELF_CHECK_OFS(TScene, nsprites, 0x038);
ELF_CHECK_OFS(TScene, name, 0x03c);
ELF_CHECK_OFS(TScene, stage, 0x13c);
ELF_CHECK_OFS(TScene, sprites, 0x140);
ELF_CHECK_OFS(TScene, nbuttons, 0x53c);
ELF_CHECK_OFS(TScene, buttons, 0x540);
ELF_CHECK_SIZE(TScene, 0x770);

#endif
