# ndk-build modules for the game (pkg/build_apk.sh copies this to app/jni/src/Android.mk and links
# app/jni/src/elf to the repository root). Mirrors pkg.mk's pkg-game flag groups.
LOCAL_PATH := $(call my-dir)
E := elf
EP := $(LOCAL_PATH)/$(E)
ELF_INC := $(EP)/port/include $(EP)/include $(LOCAL_PATH)/../SDL/include $(LOCAL_PATH)/../SDL_ttf
ELF_WNO := -Wno-unknown-pragmas -Wno-return-type-c-linkage -Wno-deprecated-copy-with-user-provided-copy \
    -Wno-mismatched-tags -Wno-deprecated-register -Wno-register -Wno-unused-parameter -Wno-unknown-warning-option

include $(CLEAR_VARS)
LOCAL_MODULE := elfshim
LOCAL_SRC_FILES := $(E)/port/src/gdi.c $(E)/port/src/winmm.c $(E)/port/src/res.c $(E)/port/src/kernel.c $(E)/port/src/hires.c
LOCAL_C_INCLUDES := $(ELF_INC)
LOCAL_CFLAGS := -O1 -std=c99 $(ELF_WNO)
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := elfvcl
LOCAL_SRC_FILES := $(addprefix $(E)/port/vcl/,system.cpp classes.cpp thread.cpp controls.cpp forms.cpp dfm.cpp scktcomp.cpp entry.cpp rtl.cpp)
LOCAL_C_INCLUDES := $(ELF_INC)
LOCAL_CPPFLAGS := -O1 -std=c++17 $(ELF_WNO)
LOCAL_CPP_FEATURES := exceptions rtti
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := elfentry
LOCAL_SRC_FILES := $(E)/port/vcl/main.cpp
LOCAL_C_INCLUDES := $(ELF_INC)
LOCAL_CPPFLAGS := -O1 -std=c++17 -DSDL_MAIN_HANDLED -Dmain=elfbowl_main $(ELF_WNO)   # pkg/pkg_main.cpp calls it
LOCAL_CPP_FEATURES := exceptions rtti
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := elfgame
LOCAL_SRC_FILES := $(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(EP)/src_match/*.cpp)) \
    $(E)/port/vcl/elf_glue.cpp $(E)/port/native/game_native.cpp $(E)/port/native/game_data.cpp
LOCAL_C_INCLUDES := $(ELF_INC)
LOCAL_CPPFLAGS := -O1 -std=gnu++17 -DPORT -fms-extensions -fsigned-char -fno-strict-aliasing -fwrapv \
    -include $(EP)/port/pkg/cxx_pre.h -include bcb_rtl.h $(ELF_WNO)
LOCAL_CPP_FEATURES := exceptions rtti
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := main
LOCAL_SRC_FILES := $(E)/port/pkg/pkg_main.cpp $(E)/port/pkg/firstrun.c
LOCAL_C_INCLUDES := $(ELF_INC)
LOCAL_CFLAGS := -O1
LOCAL_CPPFLAGS := -std=c++17
LOCAL_WHOLE_STATIC_LIBRARIES := elfentry elfgame elfvcl elfshim
LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lOpenSLES -llog -landroid -lz
include $(BUILD_SHARED_LIBRARY)
