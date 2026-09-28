# Packages for other platforms (port/pkg/README.md). Included by port/Makefile.
#   make -C port pkg-windows     -> build/dist/elfbowl-windows-x64.zip  (MinGW-w64 cross build, from macOS/Linux)
#   make -C port pkg-linux       -> build/dist/ElfBowling-x86_64.AppImage (run on x86_64 Linux / WSL)
#   make -C port pkg-ios         -> build/dist/Elf Bowling.ipa (Xcode, automatic signing; pkg/build_ipa.sh)
#   make -C port pkg-android     -> build/dist/elfbowl.apk (SDK + NDK; pkg/build_apk.sh)
# The game code is the same set of units as `make native`; pkg-game builds it for any gcc/clang
# toolchain given PKG_CC/PKG_CXX/PKG_SDL_CFLAGS/PKG_LIBS. No build needs the original exe, and no
# package contains it or anything from it: pkg/pkg_main.cpp + pkg/firstrun.c find the user's copy
# at first run (SHA-256 checked), and native/game_data.cpp loads the .data values from it.
DIST := ../build/dist
PKG ?= host
PKG_CC ?= $(CC)
PKG_CXX ?= $(CXX)
PKG_SDL_CFLAGS ?= $(SDL_CFLAGS)
PKG_LIBS ?= $(NATIVE_LIBS)
PKG_OPT ?= -O1
PKG_OUT ?= ../build/pkg/$(PKG)/elfbowl
PB := ../build/pkg/$(PKG)/obj
PKG_WNO := $(NATIVE_WNO) -Wno-unknown-warning-option -Wno-unused-parameter
PKG_CFLAGS = $(PKG_OPT) -std=c99 -Iinclude $(PKG_SDL_CFLAGS)
PKG_VCLFLAGS = $(PKG_OPT) -std=c++17 -Iinclude $(PKG_SDL_CFLAGS)
PKG_GAMEFLAGS = $(PKG_OPT) -std=gnu++17 -DPORT -fms-extensions -fsigned-char -fno-strict-aliasing -fwrapv \
    -Iinclude -I../include $(PKG_SDL_CFLAGS) $(PKG_WNO)
PKG_C_OBJS := $(SRCS:src/%.c=$(PB)/src/%.o) $(PB)/src/hires.o
PKG_VCL_OBJS := $(VCL_SRCS:vcl/%.cpp=$(PB)/vcl/%.o) $(PB)/vcl/main.o $(PB)/pkg/pkg_main.o $(PB)/pkg/firstrun.o
PKG_GAME_OBJS := $(NATIVE_SRCS:../src_match/%.cpp=$(PB)/game/%.o) $(PB)/port/elf_glue.o $(PB)/port/game_native.o \
    $(PB)/port/game_data.o $(PKG_EXTRA_OBJS)

.PHONY: pkg-game pkg-windows pkg-linux pkg-ios pkg-android
pkg-game: $(PKG_OUT)
$(PB)/src/%.o: src/%.c $(HDRS)
	@mkdir -p $(dir $@)
	$(PKG_CC) $(PKG_CFLAGS) -c $< -o $@
$(PB)/vcl/%.o: vcl/%.cpp $(VCL_HDRS)
	@mkdir -p $(dir $@)
	$(PKG_CXX) $(PKG_VCLFLAGS) -c $< -o $@
# the game's main() becomes elfbowl_main(), called by pkg/pkg_main.cpp after the first-run check
$(PB)/vcl/main.o: vcl/main.cpp $(VCL_HDRS)
	@mkdir -p $(dir $@)
	$(PKG_CXX) $(PKG_VCLFLAGS) -DSDL_MAIN_HANDLED -Dmain=elfbowl_main -c $< -o $@
$(PB)/pkg/pkg_main.o: pkg/pkg_main.cpp pkg/firstrun.h
	@mkdir -p $(dir $@)
	$(PKG_CXX) $(PKG_VCLFLAGS) -c $< -o $@
$(PB)/pkg/firstrun.o: pkg/firstrun.c pkg/firstrun.h
	@mkdir -p $(dir $@)
	$(PKG_CC) $(PKG_OPT) -std=gnu99 $(PKG_SDL_CFLAGS) -c $< -o $@
$(PB)/game/%.o: ../src_match/%.cpp $(VCL_HDRS)
	@mkdir -p $(dir $@)
	$(PKG_CXX) $(PKG_GAMEFLAGS) -include bcb_rtl.h -c $< -o $@
$(PB)/port/elf_glue.o: vcl/elf_glue.cpp $(VCL_HDRS)
	@mkdir -p $(dir $@)
	$(PKG_CXX) $(PKG_GAMEFLAGS) -include bcb_rtl.h -c $< -o $@
$(PB)/port/game_native.o: native/game_native.cpp $(VCL_HDRS)
	@mkdir -p $(dir $@)
	$(PKG_CXX) $(PKG_GAMEFLAGS) -include bcb_rtl.h -c $< -o $@
$(PB)/port/game_data.o: native/game_data.cpp $(VCL_HDRS)
	@mkdir -p $(dir $@)
	$(PKG_CXX) $(PKG_GAMEFLAGS) -include bcb_rtl.h -c $< -o $@
# pick_win.c: the real Windows headers, so neither -Iinclude nor win_compat.h
$(PB)/pkg/pick_win.o: pkg/pick_win.c pkg/firstrun.h
	@mkdir -p $(dir $@)
	$(PKG_CC) $(PKG_OPT) -c $< -o $@
$(PKG_OUT): $(PKG_C_OBJS) $(PKG_VCL_OBJS) $(PKG_GAME_OBJS)
	@mkdir -p $(dir $@)
	$(PKG_CXX) -o $@ $^ $(PKG_LIBS)

# ---- Windows x64 (MinGW-w64; brew install mingw-w64) ------------------------------------------
# SDL2 and SDL2_ttf "devel-mingw" packages, unpacked (pkg/README.md). zlib is the game's own
# (src_match/zlib_41e2cc.cpp); SDL2_ttf.dll carries its FreeType. libgcc/libstdc++ link statically.
WIN_SDL ?= ../../jungle-recomp/build/deps/SDL2-2.30.9/x86_64-w64-mingw32
WIN_TTF ?= ../build/deps/SDL2_ttf-2.22.0/x86_64-w64-mingw32
WIN_PREFIX ?= x86_64-w64-mingw32-
WIN_PTHREAD ?= $(shell $(WIN_PREFIX)gcc -print-file-name=../../../../x86_64-w64-mingw32/bin/libwinpthread-1.dll)
pkg-windows:
	$(MAKE) pkg-game PKG=win64 PKG_EXTRA_OBJS=../build/pkg/win64/obj/pkg/pick_win.o PKG_CC=$(WIN_PREFIX)gcc PKG_CXX=$(WIN_PREFIX)g++ PKG_OPT="-O1 -g0" \
	  PKG_SDL_CFLAGS="-I$(WIN_SDL)/include/SDL2 -I$(WIN_TTF)/include/SDL2 -DSDL_MAIN_HANDLED -include pkg/win_compat.h" \
	  PKG_LIBS="-L$(WIN_SDL)/lib -L$(WIN_TTF)/lib -lSDL2_ttf -lSDL2 -lcomdlg32 -static-libgcc -static-libstdc++ -mwindows" \
	  PKG_OUT=../build/pkg/win64/elfbowl.exe
	rm -rf ../build/pkg/win64/ElfBowling && mkdir -p ../build/pkg/win64/ElfBowling $(DIST)
	cp ../build/pkg/win64/elfbowl.exe $(WIN_SDL)/bin/SDL2.dll $(WIN_TTF)/bin/SDL2_ttf.dll $(WIN_PTHREAD) pkg/README-windows.txt \
	  ../build/pkg/win64/ElfBowling/
	$(WIN_PREFIX)strip ../build/pkg/win64/ElfBowling/*.exe ../build/pkg/win64/ElfBowling/*.dll
	cp ../build/pkg/win64/ElfBowling/elfbowl.exe ../build/pkg/win64/ElfBowling/*.dll $(DIST)/
	cd ../build/pkg/win64 && rm -f ../../dist/elfbowl-windows-x64.zip && zip -qr ../../dist/elfbowl-windows-x64.zip ElfBowling

# ---- Linux AppImage (x86_64 Linux with libsdl2-dev, libsdl2-ttf-dev; pkg/build_appimage.sh) -----
pkg-linux:
	$(MAKE) pkg-game PKG=linux PKG_CC=clang PKG_CXX=clang++ PKG_OPT="-O1 -g0" PKG_SDL_CFLAGS="$(SDL_CFLAGS) -include pkg/cxx_pre.h" PKG_LIBS="$(SDL_LIBS) -static-libstdc++ -static-libgcc" PKG_OUT=../build/pkg/linux/elfbowl
	sh pkg/build_appimage.sh ../build/pkg/linux/elfbowl $(DIST)/ElfBowling-x86_64.AppImage

# ---- iOS / Android ----------------------------------------------------------------------------
pkg-ios:
	sh pkg/build_ipa.sh
pkg-android:
	sh pkg/build_apk.sh
