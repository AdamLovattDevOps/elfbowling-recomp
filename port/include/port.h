/* Port-only entry points of the Win32 shim (not Win32 API). See docs/SHIM.md. */
#ifndef PORT_PORT_H
#define PORT_PORT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HWND__;

/* Create the SDL window (or run headless if that fails) and the w x h 8-bit
 * screen surface. Safe to call more than once; later calls resize the screen.
 * Returns 0 on success. GetDC() works without it (headless 640x480). */
int port_init(int w, int h, const char *title);
void port_shutdown(void);

/* The HWND that GetDC/ShowWindow understand (the one game window). */
struct HWND__ *port_main_hwnd(void);

/* The SDL_Window behind that HWND, or NULL when headless (for the VCL layer:
 * borders, title, position). */
void *port_sdl_window(void);

/* Convert the screen's 8-bit pixels through the realized (system) palette
 * and present them in the window. No-op on the window side when headless. */
void port_present(void);
/* web: size the window (the canvas backing store) for the hi-res frame (gdi.c) */
void port_hires_window(int scale, float dpr);

/* Write the presented image (palette-converted, 24-bit) as a BMP. 0 = ok. */
int port_screen_dump_bmp(const char *path);

/* Raw screen access: 8-bit indices, top-down, pitch == w. */
unsigned char *port_screen_pixels(int *w, int *h);
/* The system palette as 256 x {r,g,b,flags}. */
const unsigned char *port_system_palette(void);

/* Path of the user's original "Elf Bowling.exe". Default: $ELFBOWL_EXE, then
 * "Elf Bowling.exe" in the working directory. Returns 0 if it loaded. */
int port_res_open(const char *path);
/* The whole exe as port_res_open read it (NULL before a successful open). The game's
 * .data initial values are loaded from it (native/game_data.cpp, elf_game_data_load). */
const unsigned char *port_res_image(size_t *size);

/* Logging to stderr with a "[shim]" prefix; silenced by PORT_LOG=0. */
void port_log(const char *fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 1, 2)))
#endif
    ;

#ifdef __cplusplus
}
#endif

#endif
