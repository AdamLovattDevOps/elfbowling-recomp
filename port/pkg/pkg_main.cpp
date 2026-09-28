// Entry point of every package (Windows, Linux AppImage, iOS, Android; pkg/README.md).
// port/vcl/main.cpp is compiled with -DSDL_MAIN_HANDLED -Dmain=elfbowl_main (a C++ function then);
// this runs the first-run check (pkg/firstrun.c: find, SHA-256 verify and store the user's
// original Elf Bowling.exe; no package contains it), points the shim at the bundled free fonts,
// and starts the game with the stored exe.
#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "firstrun.h"

int elfbowl_main(int argc, char **argv);
#ifdef __APPLE__
extern "C" void elfbowl_ios_setup(void); // ios/bowl_button.m: BOWL button
#endif

#ifdef __ANDROID__
static const char *const k_fonts[] = {"LiberationSans-Regular.ttf", "LiberationSans-Bold.ttf",
                                      "Z003-MediumItalic.ttf", NULL};
// assets/<name> -> dst (assets are not plain files; the shim fopen()s its fonts)
static void copy_asset(const char *name, const char *dst)
{
    SDL_RWops *in = SDL_RWFromFile(name, "rb");
    if (!in)
        return;
    struct stat st;
    if (stat(dst, &st) == 0 && st.st_size == SDL_RWsize(in)) {
        SDL_RWclose(in);
        return;
    }
    if (FILE *out = fopen(dst, "wb")) {
        char buf[65536];
        size_t n;
        while ((n = SDL_RWread(in, buf, 1, sizeof buf)) > 0)
            fwrite(buf, 1, n, out);
        fclose(out);
    }
    SDL_RWclose(in);
}
#endif

static void set_env(const char *k, const char *v)
{
#ifdef _WIN32
    SDL_setenv(k, v, 1);
    _putenv_s(k, v);
#else
    setenv(k, v, 1);
#endif
}

#if defined(__APPLE__) || defined(__ANDROID__)
extern "C" int SDL_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    static char exe[1024], fonts[1024], user[1024];
    const char *user_dir = NULL, *given = NULL;
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    char *base = SDL_GetBasePath();
    char *pref = SDL_GetPrefPath("NStorm", "Elf Bowling");
#if defined(__APPLE__)
    const char *home = getenv("HOME");
    snprintf(user, sizeof user, "%s/Documents", home ? home : ".");
    mkdir(user, 0755);
    user_dir = user;
    snprintf(fonts, sizeof fonts, "%sfonts", base ? base : "");
    elfbowl_ios_setup();
#elif defined(__ANDROID__)
    {   // stderr (the shim's port_log) goes nowhere on Android: keep it in files/elfbowl.log
        char log[1100];
        snprintf(log, sizeof log, "%selfbowl.log", pref ? pref : "");
        freopen(log, "w", stderr);
        setvbuf(stderr, NULL, _IONBF, 0);
    }
    if (const char *ext = SDL_AndroidGetExternalStoragePath()) {
        snprintf(user, sizeof user, "%s", ext);
        user_dir = user;
    }
    snprintf(fonts, sizeof fonts, "%sfonts", pref ? pref : "");
    mkdir(fonts, 0755);
    for (int i = 0; k_fonts[i]; i++) {
        char src[256], dst[1200];
        snprintf(src, sizeof src, "fonts/%s", k_fonts[i]);
        snprintf(dst, sizeof dst, "%s/%s", fonts, k_fonts[i]);
        copy_asset(src, dst);
    }
#else
    given = argc > 1 ? argv[1] : NULL;
    fonts[0] = 0;               // Windows: C:/Windows/Fonts; the AppImage's AppRun sets PORT_FONT_DIR
#endif
    if (fonts[0] && !getenv("PORT_FONT_DIR"))
        set_env("PORT_FONT_DIR", fonts);
    SDL_free(base);
    if (elfbowl_first_run(given, user_dir, exe, sizeof exe) != 0) {
        SDL_free(pref);
        return 1;
    }
    set_env("ELFBOWL_EXE", exe);
    if (pref && chdir(pref) != 0) { // anything the game writes lands in the app's own storage
    }
    SDL_free(pref);
    std::vector<char *> args;
    args.push_back(argv && argc > 0 ? argv[0] : (char *)"elfbowl");
    args.push_back(exe);
    for (int i = 2; i < argc; i++)   // desktop: the game's own arguments after the exe path
        args.push_back(argv[i]);
    args.push_back(NULL);
    return elfbowl_main((int)args.size() - 1, args.data());
}
