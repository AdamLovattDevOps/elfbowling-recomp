# iOS / iPadOS app: an Xcode project for signing and sideloading (pkg/build_ipa.sh), as in
# jungle-recomp's ios/. SDL2 is the static library built from SDL's own Xcode project; SDL2_ttf and
# its FreeType are compiled straight into the app with the free fonts. Nothing from the exe is in
# the app: the first run takes the user's copy from Documents (Files app / Finder) or a document
# picker, checks its SHA-256 and keeps it (pkg/pkg_main.cpp, pkg/firstrun.c).
#   cmake -S port/pkg/ios -B build/pkg/ios-xcode -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DTEAM=<team id>
cmake_minimum_required(VERSION 3.20)
project(ElfBowling C CXX OBJC)
set(CMAKE_OSX_DEPLOYMENT_TARGET 15.0)
get_filename_component(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. ABSOLUTE)
set(P ${ROOT}/port)
set(SDL ${ROOT}/../jungle-recomp/build/deps/SDL2-2.30.9-src CACHE PATH "SDL2 source")
set(SDL_LIB_DIR ${ROOT}/../jungle-recomp/build/deps/sdl2-ios-build CACHE PATH "SDL2 static library build")
set(TTF ${ROOT}/build/deps/src/SDL2_ttf-2.22.0 CACHE PATH "SDL2_ttf source")
set(BUNDLE_ID "com.mussyg.elfbowling" CACHE STRING "")

set(SHIM ${P}/src/gdi.c ${P}/src/winmm.c ${P}/src/res.c ${P}/src/kernel.c ${P}/src/hires.c)
set(VCL ${P}/vcl/system.cpp ${P}/vcl/classes.cpp ${P}/vcl/thread.cpp ${P}/vcl/controls.cpp ${P}/vcl/forms.cpp
        ${P}/vcl/dfm.cpp ${P}/vcl/scktcomp.cpp ${P}/vcl/entry.cpp ${P}/vcl/rtl.cpp)
file(GLOB GAME ${ROOT}/src_match/*.cpp)
list(APPEND GAME ${P}/vcl/elf_glue.cpp ${P}/native/game_native.cpp ${P}/native/game_data.cpp)
set(FT ${TTF}/external/freetype/src)
set(FREETYPE ${FT}/autofit/autofit.c ${FT}/base/ftbase.c ${FT}/base/ftbbox.c ${FT}/base/ftbdf.c
    ${FT}/base/ftbitmap.c ${FT}/base/ftcid.c ${FT}/base/ftdebug.c ${FT}/base/ftfstype.c ${FT}/base/ftgasp.c
    ${FT}/base/ftglyph.c ${FT}/base/ftgxval.c ${FT}/base/ftinit.c ${FT}/base/ftmm.c ${FT}/base/ftotval.c
    ${FT}/base/ftpatent.c ${FT}/base/ftpfr.c ${FT}/base/ftstroke.c ${FT}/base/ftsynth.c ${FT}/base/ftsystem.c
    ${FT}/base/fttype1.c ${FT}/base/ftwinfnt.c ${FT}/bdf/bdf.c ${FT}/cache/ftcache.c ${FT}/cff/cff.c
    ${FT}/cid/type1cid.c ${FT}/gzip/ftgzip.c ${FT}/lzw/ftlzw.c ${FT}/pcf/pcf.c ${FT}/pfr/pfr.c
    ${FT}/psaux/psaux.c ${FT}/pshinter/pshinter.c ${FT}/psnames/psmodule.c ${FT}/raster/raster.c
    ${FT}/sdf/sdf.c ${FT}/sfnt/sfnt.c ${FT}/svg/svg.c ${FT}/smooth/smooth.c ${FT}/truetype/truetype.c
    ${FT}/type1/type1.c ${FT}/type42/type42.c ${FT}/winfonts/winfnt.c)
set(WNO -Wno-unknown-pragmas -Wno-return-type-c-linkage -Wno-deprecated-copy-with-user-provided-copy
    -Wno-mismatched-tags -Wno-deprecated-register -Wno-register -Wno-unused-parameter)
set_source_files_properties(${SHIM} PROPERTIES COMPILE_OPTIONS "-std=c99;-O1")
set_source_files_properties(${VCL} PROPERTIES COMPILE_OPTIONS "-std=c++17;-O1;${WNO}")
set_source_files_properties(${P}/vcl/main.cpp PROPERTIES COMPILE_OPTIONS
    "-std=c++17;-O1;-DSDL_MAIN_HANDLED;-Dmain=elfbowl_main;${WNO}")
set_source_files_properties(${GAME} PROPERTIES COMPILE_OPTIONS
    "-std=gnu++17;-O1;-DPORT;-fms-extensions;-fsigned-char;-fno-strict-aliasing;-fwrapv;-include;${P}/include/bcb_rtl.h;${WNO}")
set_source_files_properties(${FREETYPE} ${TTF}/SDL_ttf.c PROPERTIES COMPILE_OPTIONS
    "-DFT2_BUILD_LIBRARY;-DFT_PUBLIC_FUNCTION_ATTRIBUTE=;-Os;-w")
set_source_files_properties(${P}/pkg/pkg_main.cpp PROPERTIES COMPILE_OPTIONS "-std=c++17")
set_source_files_properties(${P}/pkg/firstrun.c PROPERTIES COMPILE_OPTIONS "-std=gnu99")

set(FONT_SRC ${P}/web/fonts/LiberationSans-Regular.ttf ${P}/web/fonts/LiberationSans-Bold.ttf)
set(FONT_Z ${CMAKE_CURRENT_BINARY_DIR}/fonts/Z003-MediumItalic.ttf)
configure_file(${P}/web/fonts/texgyrechorus-mediumitalic.otf ${FONT_Z} COPYONLY)
set_source_files_properties(${FONT_SRC} ${FONT_Z} PROPERTIES MACOSX_PACKAGE_LOCATION fonts)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/LaunchScreen.storyboard
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

add_executable(ElfBowling MACOSX_BUNDLE
  ${SHIM} ${VCL} ${P}/vcl/main.cpp ${GAME} ${FREETYPE} ${TTF}/SDL_ttf.c
  ${P}/pkg/pkg_main.cpp ${P}/pkg/firstrun.c ${CMAKE_CURRENT_SOURCE_DIR}/bowl_button.m ${SDL}/src/main/uikit/SDL_uikit_main.c
  ${FONT_SRC} ${FONT_Z} ${CMAKE_CURRENT_SOURCE_DIR}/LaunchScreen.storyboard)
target_include_directories(ElfBowling PRIVATE ${P}/include ${ROOT}/include ${SDL}/include ${TTF}
  ${TTF}/external/freetype/include)
target_link_libraries(ElfBowling PRIVATE
  ${SDL_LIB_DIR}/Release-$(PLATFORM_NAME)/libSDL2.a m iconv z
  "-framework UIKit" "-framework Foundation" "-framework CoreGraphics" "-framework QuartzCore"
  "-framework OpenGLES" "-framework Metal" "-framework AVFoundation" "-framework AudioToolbox"
  "-framework CoreAudio" "-framework CoreMotion" "-framework GameController" "-framework CoreHaptics"
  "-framework CoreBluetooth" "-framework CoreFoundation" "-framework UniformTypeIdentifiers")
set_target_properties(ElfBowling PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
  MACOSX_BUNDLE_GUI_IDENTIFIER ${BUNDLE_ID}
  XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${BUNDLE_ID}
  XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${TEAM}"
  XCODE_ATTRIBUTE_CODE_SIGN_STYLE Automatic
  XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
  XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
  XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 15.0
  XCODE_ATTRIBUTE_ENABLE_BITCODE NO
  XCODE_ATTRIBUTE_SKIP_INSTALL NO
  XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
  XCODE_ATTRIBUTE_GCC_WARN_INHIBIT_ALL_WARNINGS NO
  XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT dwarf
  XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
  )
