// elf/game.h - game-wide globals: the engine objects, bowling state (frames,
// pins, score), cheats and the sprite groups.
//
// Every global is declared under its address name (g_XXXXXX), extern "C".
// A readable alias macro is given where the meaning is clear. Globals that
// some units declared with an address-plus-word name (g_frames_4605c8,
// g_present_460578, g_group_46053c, ...) are listed in docs/HEADERS.md;
// migrate them to the address name.
//
// Type conflicts between units are noted per global. bool vs char matters
// for stores (`g = c != 0` vs a plain byte store): cast at the use site,
// e.g. ELF_AS(char, g_4605a0).
#ifndef ELF_GAME_H
#define ELF_GAME_H

#include <elf/types.h>
#include <elf/stage.h>
#include <elf/scene.h>
#include <elf/sprite.h>
#include <elf/sound.h>
#include <elf/packres.h>

// ---- bowling data -------------------------------------------------------

// Score of one frame (16 bytes), 12 frames at 0x4605c8 (10 + 2 bonus).
// game_* units see the frames as int[12][4] (use .v[]); exit_411af4 clears
// both flags with one int store (f0c).
struct FrameScore {
    union {
        struct {
            int roll[2];        // 0x00 pins per ball, -1 = not bowled
            int total;          // 0x08 running total, -1 = not scored
            union {
                struct {
                    char shown0;    // 0x0c roll 0 drawn on the board (score_4120cc: strike)
                    char shown1;    // 0x0d roll 1 drawn on the board (score_4120cc: spare)
                };
                int f0c;        // 0x0c both flags (exit_411af4)
            };
        };
        int v[4];               // game_*: g_frames_4605c8[i][k]
    };
};
ELF_CHECK_OFS(FrameScore, total, 0x08);
ELF_CHECK_OFS(FrameScore, shown0, 0x0c);
ELF_CHECK_SIZE(FrameScore, 0x10);

// Per-pin state (8 bytes), 10 at 0x460240. Old: Pin (score_4120cc), Slot (game_414240).
struct PinState {
    unsigned char state;        // 0x00 6 = knocked down (compares need unsigned: cmp byte)
    char _pad1[3];
    int f4;                     // 0x04
};
ELF_CHECK_SIZE(PinState, 8);

extern "C" {

// ---- engine objects -----------------------------------------------------
extern TStage *g_4601c8;                // the stage (TStage ctor stores this; range_401d0c: Screen)
extern TStage *g_456c38;                // game engine (init_40f350 creates it; menu: void *)
extern TSoundMgr *g_460208;             // sound manager
extern TPackedResources *g_455524;      // NVDPACKFILE (void * in some units)
extern TSaveParms *g_455528;            // PARMS
extern TPackedResources *g_460218;      // init_40f350: PackedFile
extern TWebTrack *g_46020c;             // web tracker
extern void *g_4601c0;                  // play form (TForm *)
extern void *g_460214;                  // main form (TForm *)
extern void **g_45fee8;                 // &Forms::Screen
extern void **g_45fee4;                 // &Forms::Application (init_40f350; old: p_Application)
extern HINSTANCE *g_45fee0;             // &HInstance
#define g_stage     g_4601c8
#define g_game      g_456c38
#define g_soundMgr  g_460208
#define g_packRes   g_455524
#define g_saveParms g_455528
#define g_webTrack  g_46020c

// ---- GDI / bitmap helpers ----------------------------------------------
extern BITMAPINFOHEADER *g_45552c;      // shared BITMAPINFO (header + 256 RGBQUAD, fn_402030)
extern unsigned char g_455530;          // transparent colour index
extern unsigned char g_455531;          // background colour index
extern int g_455534;                    // bytes read (source), packres_40d6f0
extern int g_455538;                    // bytes kept (cropped), packres_40d6f0
extern char g_45553c;
extern int g_455540;                    // spare width
extern int g_455544;                    // spare height
extern bool g_455548;                   // form fits
extern int g_4555ac;                    // font count
extern int g_4555bc;
extern int g_4555c0;
extern HGDIOBJ g_4601cc[];              // fonts (HFONT)
extern char g_4601ec;                   // font family found
extern HGDIOBJ g_4601f0;                // previously selected font
extern HDC g_4601f4;                    // screen DC
extern HDC g_4601f8;                    // memory DC
extern HPALETTE g_4601fc;
extern int g_460200;
extern int g_460204;

// ---- memory statistics (fn_402194) -------------------------------------
extern int g_45554c;                    // allocation counter
extern unsigned g_455550;
extern int g_455554;                    // min memory load (-1 = unset)
extern unsigned g_455558;               // max memory load
extern unsigned g_45555c;               // total physical memory
extern unsigned g_455560;               // min available
extern unsigned g_455564;               // available physical memory
extern unsigned g_455568;               // max available

// ---- menu / about -------------------------------------------------------
extern char *g_460228;
extern int g_46022c;
extern int g_460230;
extern bool g_460234;                   // menu popup open
extern int g_460238;                    // last click time
extern const char *g_456ee8[];
extern const char *g_456f18[];
extern const char *g_456f3c[];
extern const char *g_456f5c[];
extern const char *g_456f78[];
extern const char *g_456f9c[];
extern const int g_456fb8[10];          // egg hotspot x
extern const int g_456fe0[10];          // egg hotspot y

// ---- sprite groups ------------------------------------------------------
extern TSpriteGroup *g_46023c;          // menu elves
extern TSpriteGroup *g_460538;          // elves (game_*: g_group_460538)
extern TSpriteGroup *g_46053c;          // lanes (score: LaneGroup)
extern TSpriteGroup *g_460540;
extern TSpriteGroup *g_460544;
extern TSpriteGroup *g_460548;
extern TSpriteGroup *g_46054c;
extern TSpriteGroup *g_460550;          // pin markers
extern TSpriteGroup *g_460554;          // aim marker
extern TSpriteGroup *g_460558;          // frame boxes / score board
extern TSpriteGroup *g_46055c;          // text: unit_411 totals, score_4120cc roll 1
extern TSpriteGroup *g_460560;          // text: unit_411 roll 1, score_4120cc roll 2
extern TSpriteGroup *g_460564;          // text: unit_411 roll 2, score_4120cc totals
extern TSpriteGroup *g_460568;
extern TSpriteGroup *g_46056c;
extern TSpriteGroup *g_460570;
extern TSpriteGroup *g_460574;
extern TSpriteGroup *g_460750;
extern TSpriteGroup *g_460778;
extern TSpriteGroup *g_46077c;

// ---- bowling state ------------------------------------------------------
extern PinState g_460240[10];           // per-pin state (game_414240: g_slots_460240)
extern bool g_460578[10];               // pin standing / present (game_*: char g_present_460578[])
extern bool g_460582[10];               // (game_*: char[])
extern bool g_46058c[10];               // pin knocked (game_*: char[] in some)
extern bool g_460596;
extern bool g_460597;
extern int g_460598;                    // current frame
extern int g_46059c;                    // score
extern bool g_4605a0;                   // cheated this game (game_*: char)
extern bool g_4605a1, g_4605a2, g_4605a3, g_4605a4;   // cheats Ctrl+X/D/S/G (Ctrl+N clears)
extern int g_4605a8;                    // key1
extern int g_4605ac;
extern int *g_4605b0;
extern bool g_4605b4;
extern int *g_4605b8;
extern int g_4605bc;                    // current roll
extern int g_4605c0;
extern int g_4605c4;                    // aim timer ms
extern FrameScore g_4605c8[12];         // frames (game_*: int g_frames_4605c8[][4];
                                        //  g_frame9_460658 = &g_4605c8[9] etc.)
extern int g_460688;                    // aim position 0..1400
extern int g_46068c;                    // aim direction
extern bool g_460690;                   // taunt scheduled
extern int g_460694;
extern bool g_460698[2];
extern int g_46069c;
extern int g_4606a0;
extern int g_4606a4;
extern bool g_4606a8[2];
extern int g_4606ac;
extern bool g_4606b0;                   // deer hit
extern bool g_4606b1;                   // deer up
extern int g_4606c8, g_4606cc, g_4606d0, g_4606d4, g_4606d8, g_4606dc, g_4606e0, g_4606e4, g_4606e8;
extern char g_4606ec;
extern int g_4606f0;
extern int g_4606f4;
extern char g_4606f8[10];               // (game_414240: g_flag_4606f8)
extern int g_460704[12];                // shuffle order (game_414240: g_order_460704)
extern int g_460734;
extern char g_460738;
extern int g_46073c, g_460740;
extern char g_460744;
extern int g_460748;
extern int g_46074c;
extern int g_460754;
extern bool g_460758, g_460759;         // (char in one unit)
extern int g_46075c[4];
extern int g_46076c;
extern int g_460770;
extern char *g_460780;
extern int g_460784, g_460788, g_46078c;
#define g_frames    g_4605c8
#define g_pins      g_460240
#define g_present   g_460578
#define g_knocked   g_46058c
#define g_frame     g_460598
#define g_score     g_46059c
#define g_roll      g_4605bc
#define g_cheated   g_4605a0
#define g_aimPos    g_460688
#define g_aimDir    g_46068c
// Names found while migrating batch D (score_4120cc, game_41*, opening_*, egg).
#define g_cheatX       g_4605a1     // Ctrl+X: aim cell 13 or 15
#define g_cheatD       g_4605a2     // Ctrl+D: aim cell 0
#define g_cheatS       g_4605a3     // Ctrl+S: cell 14 on ball 1
#define g_cheatG       g_4605a4     // Ctrl+G: random off-centre cell
#define g_aimCell      g_4605ac     // ball cell 0..28 (aim / 50, or the cheat); knock table row + 9
#define g_aimPeriod    g_4605c4     // aim marker step period in ms
#define g_aiming       g_460596     // aim marker running (space/enter throws)
#define g_pinsDown     g_4605c0     // pins knocked by this ball (fn_41665c)
#define g_gameKey      g_4605a8     // random 1..999999 per game, sent with the score (menu_41008c)
#define g_gutterBall   g_460597     // ball left the lane this roll
#define g_gutterLeft   g_460758     // ball in the left gutter
#define g_gutterRight  g_460759     // ball in the right gutter
#define g_ballOffset   g_460754     // ball x offset along the lane (fn_41237c)
#define g_ballRow      g_46076c     // ball depth row 0..4 (thresholds g_46075c)
#define g_ballRowY     g_46075c     // y thresholds of the ball rows
#define g_tauntPending g_460690     // a taunt line is scheduled
#define g_deerHit      g_4606b0     // the ball hit the deer
#define g_deerUp       g_4606b1     // the deer is on the lane
#define g_deerFrame    g_4606ac     // frame from which the deer may run (1..5)
#define g_hintShown    g_45886c     // "Hint1a" shown this game
#define g_hintCount    g_459124     // games started (the hint shows on the first)
#define g_eggText      g_460228     // ElfCrew easter-egg text buffer (0x4b0 bytes)
#define g_eggLineH     g_46022c     // its line height
#define g_eggLen       g_460230     // its scroll length in pixels
#define g_scrollText   g_460780     // rules scroll text buffer (0xb54 bytes)
#define g_scrollLineH  g_460784     // its line height
#define g_scrollLen    g_460788     // its scroll length in pixels

// ---- tables (.data) -----------------------------------------------------
extern int g_458840[];                  // (game_414240: g_map_458840)
extern int g_458868;
extern char g_45886c;
extern bool g_45886d;                   // (char in one unit)
extern int g_458870;
extern const char *g_458874[];
extern const char *g_45887c[];
extern int g_458884[][3][10];
extern int g_458b54[][3][10];
extern int g_458e24[][3][10];
extern int g_4590f4[];
extern int g_459124;
extern const char *g_45aaf4[];          // (game_*: g_lines_45aaf4)
extern const char *g_45ab68[];          // (game_*: g_lines_45ab68)

// ---- hooks --------------------------------------------------------------
extern void (*g_45c41c)(const char *msg);   // zlib error hook (packres_40dce0 sets fn_40dc04)

}   // extern "C"

#endif
