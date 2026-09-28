// Web build only: entry points the page (port/web/loader.js) calls.
//
//   web_key(sym, down, mod)   an SDL keycode pressed or released (the on-screen Bowl / Enter / Esc
//                             buttons). It goes through the SDL queue, so the game sees it in the
//                             next frame exactly like a hardware key (forms.cpp dispatch).
//   web_pack_ready(name)      a manifest pack is in the FS. "hires": the ESRGAN art at /hires/x3;
//                             hires.c starts using it (at $ELFBOWL_HIRES_SCALE, 3 or 2 on phones) and
//                             the canvas's backing store grows to match. Returns 1 when it is on.
//   web_hd(on)                the HD toggle: the hi-res frame (1) or the classic 1x one (0).
//
// Touch needs no glue: SDL2's Emscripten backend turns touches on the canvas into left-button mouse
// events in window coordinates (SDL_HINT_TOUCH_MOUSE_EVENTS), scaled by the canvas's CSS size, and
// the window is the game's 640x480 client area.
#include <SDL.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdlib.h>
#include <string.h>

#include "port.h"
#include "../src/hires.h"

extern "C" {

EMSCRIPTEN_KEEPALIVE void web_key(int sym, int down, int mod)
{
    if (!(SDL_WasInit(SDL_INIT_EVENTS) & SDL_INIT_EVENTS))
        return;
    SDL_Event e;
    SDL_zero(e);
    e.type = down ? SDL_KEYDOWN : SDL_KEYUP;
    e.key.state = down ? SDL_PRESSED : SDL_RELEASED;
    e.key.keysym.sym = sym;
    e.key.keysym.scancode = SDL_GetScancodeFromKey(sym);
    e.key.keysym.mod = (Uint16)mod;
    SDL_PushEvent(&e);
}

EMSCRIPTEN_KEEPALIVE int web_pack_ready(const char *name)
{
    if (!name || strcmp(name, "hires"))
        return 0;
    const char *e = getenv("ELFBOWL_HIRES_SCALE");
    int scale = e && *e == '2' ? 2 : 3;
    if (!hires_activate("/hires/x3", scale))
        return 0;
    port_hires_window(scale, (float)emscripten_get_device_pixel_ratio());
    return 1;
}

EMSCRIPTEN_KEEPALIVE void web_hd(int on)
{
    hires_set_showing(on);
}

} // extern "C"
