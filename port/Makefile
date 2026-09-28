# Win32-subset shim on SDL2, and the VCL/RTL subset on top of it (vcl/).
# `make -C port test` builds and runs both headless tests.
CC ?= clang
CXX ?= clang++
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS := $(shell sdl2-config --libs) -lSDL2_ttf
CFLAGS ?= -O2 -g
CFLAGS += -std=c99 -Wall -Wextra -Wno-unused-parameter -Iinclude $(SDL_CFLAGS)
CXXFLAGS ?= -O2 -g
CXXFLAGS += -std=c++11 -Wall -Wextra -Iinclude $(SDL_CFLAGS)

B := ../build/port
EXE ?= ../orig/1999/Elf Bowling.exe
SRCS := src/gdi.c src/winmm.c src/res.c src/kernel.c
OBJS := $(SRCS:src/%.c=$(B)/%.o)
HDRS := include/windows.h include/mmsystem.h include/port.h src/shim.h

# VCL subset (docs/VCL_PORT.md): C++17, same headers path
VCLXXFLAGS ?= -O2 -g
VCLXXFLAGS += -std=c++17 -Wall -Wextra -Wno-unused-parameter -Iinclude $(SDL_CFLAGS)
VCL_SRCS := vcl/system.cpp vcl/classes.cpp vcl/thread.cpp vcl/controls.cpp vcl/forms.cpp \
            vcl/dfm.cpp vcl/scktcomp.cpp vcl/entry.cpp vcl/rtl.cpp
VCL_OBJS := $(VCL_SRCS:vcl/%.cpp=$(B)/vcl/%.o)
VCL_HDRS := include/vcl.h include/bcb_rtl.h $(wildcard include/vcl/*.h include/vcl/*.hpp) include/windows.h include/port.h

.PHONY: all test clean elf-glue native native-objs
all: $(B)/libshim.a $(B)/shim_test $(B)/header_cxx.o $(B)/libvcl.a $(B)/vcl_main.o $(B)/vcl_test

$(B)/%.o: src/%.c $(HDRS)
	@mkdir -p $(B)
	$(CC) $(CFLAGS) -c $< -o $@

$(B)/libshim.a: $(OBJS)
	rm -f $@
	ar rcs $@ $^

$(B)/shim_test: tests/shim_test.c $(B)/libshim.a $(HDRS)
	$(CC) $(CFLAGS) $< -o $@ $(B)/libshim.a $(SDL_LIBS) -lz

# the headers must also compile as C++ (the game is C++)
$(B)/header_cxx.o: tests/header_cxx.cpp $(HDRS)
	@mkdir -p $(B)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(B)/vcl/%.o: vcl/%.cpp $(VCL_HDRS)
	@mkdir -p $(B)/vcl
	$(CXX) $(VCLXXFLAGS) -c $< -o $@

$(B)/libvcl.a: $(VCL_OBJS)
	rm -f $@
	ar rcs $@ $^

# main() -> WinMain, for the game build (not in libvcl.a: tests have their own main)
$(B)/vcl_main.o: vcl/main.cpp $(VCL_HDRS)
	@mkdir -p $(B)
	$(CXX) $(VCLXXFLAGS) -c $< -o $@

$(B)/vcl_test: tests/vcl_test.cpp $(B)/libvcl.a $(B)/libshim.a $(VCL_HDRS)
	$(CXX) $(VCLXXFLAGS) $< -o $@ $(B)/libvcl.a $(B)/libshim.a $(SDL_LIBS)

# Compile check of the glue that needs include/elf (part of the game build;
# not in `test`, since include/elf is being migrated in parallel).
elf-glue: $(B)/vcl/elf_glue.o
$(B)/vcl/elf_glue.o: vcl/elf_glue.cpp $(VCL_HDRS) ../include/elf/types.h
	@mkdir -p $(B)/vcl
	$(CXX) $(VCLXXFLAGS) -I../include -Wno-return-type-c-linkage -Wno-deprecated-copy-with-user-provided-copy -c $< -o $@

test: all
	@test -f "$(EXE)" || { echo "need the original exe at $(EXE) (EXE=...)"; exit 2; }
	rm -f $(B)/audio.raw $(B)/mix.raw
	SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=disk \
	SDL_DISKAUDIOFILE=$(B)/audio.raw SDL_AUDIO_DISK_OUTPUT_FILE=$(B)/audio.raw \
	PORT_AUDIO_RATE=11025 PORT_AUDIO_DUMP=$(B)/mix.raw \
	$(B)/shim_test "$(EXE)" ../build/port_test.bmp $(B)/report.txt
	python3 tests/check_shim.py "$(EXE)" ../build/port_test.bmp $(B)/report.txt $(B)/mix.raw $(B)/audio.raw
	SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy PORT_LOG=0 $(B)/vcl_test "$(EXE)"

clean:
	rm -rf $(B) ../build/port_test.bmp

# ---- native game build (docs/NATIVE_TRIAGE.md) -----------------------------------------------
# Every ../src_match/*.cpp compiled separately with clang++ against port/include and include/,
# then linked with the VCL subset, the shim, the generated game data and SDL2.
# `make -k native` (the top-level `make native` passes -k) keeps going past errors.
NB := ../build/native
NATIVE_SRCS := $(wildcard ../src_match/*.cpp)
# overlay builds keep their objects apart, so a later plain build never links a stale overlay object
NOBJ = $(NB)/$(if $(NATIVE_OVERLAY),obj-overlay,obj)
NATIVE_OBJS = $(NATIVE_SRCS:../src_match/%.cpp=$(NOBJ)/%.o)
NATIVE_PORT_SRCS := vcl/elf_glue.cpp native/game_native.cpp
NATIVE_PORT_OBJS := $(NATIVE_PORT_SRCS:%.cpp=$(NB)/port/%.o) $(NB)/port/game_data.o
NATIVE_EXE ?= ../orig/1999/Elf Bowling.exe
# Only the noise that the matched source produces by design (-Wall -Wextra on src_match/ shows
# nothing else): extern "C" functions returning ERect/TPoint, ERect's user copy ctor, zlib's
# `register`. Everything else stays visible.
NATIVE_WNO := -Wno-unknown-pragmas -Wno-return-type-c-linkage -Wno-deprecated-copy-with-user-provided-copy \
    -Wno-mismatched-tags -Wno-deprecated-register -Wno-register
NATIVE_CXXFLAGS ?= -O1 -g
NATIVE_CXXFLAGS += -std=gnu++17 -DPORT -fms-extensions -fsigned-char -fno-strict-aliasing -fwrapv \
    -Iinclude -I../include $(SDL_CFLAGS) $(NATIVE_WNO) -ferror-limit=0
NATIVE_LIBS := $(SDL_LIBS) -lz

native: $(NB)/../elfbowl
native-objs: $(NATIVE_OBJS)

# NATIVE_OVERLAY=dir: compile dir/X.cpp in place of ../src_match/X.cpp where it exists
# (for trying source-side fixes before they land; never set by default).
NATIVE_OVERLAY ?=
native_src = $(if $(and $(NATIVE_OVERLAY),$(wildcard $(NATIVE_OVERLAY)/$(1).cpp)),$(NATIVE_OVERLAY)/$(1).cpp,../src_match/$(1).cpp)

.SECONDEXPANSION:
$(NOBJ)/%.o: $$(call native_src,$$*) $(VCL_HDRS) $(wildcard ../include/elf/*.h)
	@mkdir -p $(NOBJ)
	$(CXX) $(NATIVE_CXXFLAGS) $(if $(NATIVE_RTL_$*),,-include bcb_rtl.h) $(NATIVE_FLAGS_$*) -c $< -o $@ 2> $(NOBJ)/$*.log; rc=$$?; cat $(NOBJ)/$*.log >&2; exit $$rc

# NATIVE_RTL_<unit> := none builds that unit without the forced bcb_rtl.h (was needed for
# zlib_41e2cc's K&R memcpy/calloc redeclarations until the unit took the libc headers natively).

$(NB)/port/%.o: %.cpp $(VCL_HDRS) $(wildcard ../include/elf/*.h)
	@mkdir -p $(dir $@)
	$(CXX) $(NATIVE_CXXFLAGS) -include bcb_rtl.h -c $< -o $@

# native/game_data.cpp: every g_XXXXXX, plus the table (file offsets, no exe bytes) that loads
# their initial values from the user's exe at startup (tools/gen_game_data.py). It is kept in the
# tree, so the build needs no exe; `make game-data` regenerates it after game.h or src_match/ change.
.PHONY: game-data
game-data:
	python3 tools/gen_game_data.py --exe "$(NATIVE_EXE)" --header ../include/elf/game.h \
	    --src ../src_match --out native/game_data.cpp
$(NB)/port/game_data.o: native/game_data.cpp $(VCL_HDRS)
	@mkdir -p $(NB)/port
	$(CXX) $(NATIVE_CXXFLAGS) -include bcb_rtl.h -c $< -o $@

$(NB)/../elfbowl: $(NATIVE_OBJS) $(NATIVE_PORT_OBJS) $(B)/vcl_main.o $(B)/libvcl.a $(B)/libshim.a
	$(CXX) -o $@ $(NATIVE_OBJS) $(NATIVE_PORT_OBJS) $(B)/vcl_main.o $(B)/libvcl.a $(B)/libshim.a $(NATIVE_LIBS)

# WebAssembly build: `make -C port web` (web.mk, port/web/README.md)
include web.mk
# 3x hi-res art: hires.o in libshim.a (hires.mk, docs/HIRES.md)
include hires.mk
# Windows/Linux/iOS/Android packages (pkg.mk, pkg/README.md)
include pkg.mk
