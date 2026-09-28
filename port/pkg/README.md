# Packages (port/pkg.mk)

| Target | Output (build/dist/) | Host | Needs |
|---|---|---|---|
| `make -C port pkg-windows` | `elfbowl-windows-x64.zip` (and loose `elfbowl.exe` + DLLs) | macOS/Linux | `brew install mingw-w64`; SDL2 and SDL2_ttf `devel-mingw` packages (`WIN_SDL`, `WIN_TTF`) |
| `make -C port pkg-linux` | `ElfBowling-x86_64.AppImage` | x86_64 Linux / WSL | clang, `libsdl2-dev`, `libsdl2-ttf-dev`; appimagetool (downloaded) |
| `make -C port pkg-ios` | `Elf Bowling.ipa` | macOS | Xcode with an Apple ID; SDL2 source (jungle-recomp's `build/deps`), SDL2_ttf source in `build/deps/src` |
| `make -C port pkg-android` | `elfbowl.apk` (debug-signed, arm64-v8a + x86_64) | Linux / WSL | `ANDROID_HOME` with platform 34, build-tools 34, NDK 26.3; JDK 17; SDL2 and SDL2_ttf sources in `build/deps/src` |

## No game data in any package

No package contains `Elf Bowling.exe` or anything taken from it, and no build reads it.

- `port/native/game_data.cpp` defines the game's globals. It also holds a table of file offsets that fills in their initial values from the user's exe at startup (`elf_game_data_load`, called in `vcl/main.cpp`). The table holds structure only, no bytes. Regenerate it with `make -C port game-data` after `include/elf/game.h` or `src_match/` changes. This needs the exe, but only on the maintainer's machine.
- First run (`pkg_main.cpp`, `firstrun.c`) looks for the exe in this order:
  1. the command-line path
  2. `$ELFBOWL_EXE`
  3. the app's own copy (`SDL_GetPrefPath("NStorm", "Elf Bowling")`)
  4. next to the app
  5. the working directory
  6. iOS `Documents` (Files app, Finder file sharing) or Android `Android/data/com.mussyg.elfbowling/files/`
- If none of these is the right file, the platform picker asks for it: `GetOpenFileName`, zenity/kdialog, `UIDocumentPickerViewController` or the Android SAF picker. SDL2 has no `SDL_ShowOpenFileDialog`.
- The file must have SHA-256 `dceb5b89544b20744091f55c8ec49d2baaf59bf530ce50a4c2a14d12b069de0c` (Elf Bowling, NStorm 1999). A mismatch shows an error that names the expected file. When the file checks out, it is copied into app storage.

## Mobile

Touches are mouse clicks. The on-screen BOWL button sends Space (`ios/bowl_button.m`, `android/ElfBowlActivity.java`). The fonts are the free substitutes from `port/web/fonts`.
