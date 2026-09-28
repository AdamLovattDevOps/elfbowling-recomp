#!/bin/sh
# Build a debug-signed elfbowl.apk (make -C port pkg-android). Linux or WSL host with:
#   ANDROID_HOME (platforms;android-34, build-tools;34.0.0, ndk;26.3.11579264), JDK 17,
#   SDL2 2.30.x source (SDL2_SRC, default build/deps/src/SDL2-2.30.9) and
#   SDL2_ttf 2.22 source (SDL2_TTF_SRC, default build/deps/src/SDL2_ttf-2.22.0).
#   pkg/build_apk.sh          -> build/dist/elfbowl.apk (arm64-v8a + x86_64)
# SDL's own android-project template is copied and given the game (pkg/android/Android.mk), the
# ElfBowlActivity (BOWL button, first-run document picker) and the free fonts as assets. Nothing
# from the exe is in the APK: the first run asks for the user's copy (pkg/firstrun.c).
set -eu
cd "$(dirname "$0")/../.."
ROOT=$PWD
SDL=${SDL2_SRC:-$ROOT/build/deps/src/SDL2-2.30.9}
TTF=${SDL2_TTF_SRC:-$ROOT/build/deps/src/SDL2_ttf-2.22.0}
: "${ANDROID_HOME:?set ANDROID_HOME}"
export ANDROID_HOME ANDROID_SDK_ROOT=$ANDROID_HOME
P=$ROOT/build/pkg/android/project
rm -rf "$P"; mkdir -p "$(dirname "$P")"
cp -R "$SDL/android-project" "$P"
ln -s "$SDL" "$P/app/jni/SDL"; ln -s "$TTF" "$P/app/jni/SDL_ttf"
rm -f "$P/app/jni/src/"*; cp port/pkg/android/Android.mk "$P/app/jni/src/Android.mk"; ln -s "$ROOT" "$P/app/jni/src/elf"
cat > "$P/app/jni/Application.mk" <<MK
APP_STL := c++_static
APP_ABI := arm64-v8a x86_64
APP_PLATFORM := android-21
MK
mkdir -p "$P/app/src/main/java/com/mussyg/elfbowling" "$P/app/src/main/assets/fonts"
cp port/pkg/android/ElfBowlActivity.java "$P/app/src/main/java/com/mussyg/elfbowling/"
cp port/web/fonts/LiberationSans-Regular.ttf port/web/fonts/LiberationSans-Bold.ttf "$P/app/src/main/assets/fonts/"
cp port/web/fonts/texgyrechorus-mediumitalic.otf "$P/app/src/main/assets/fonts/Z003-MediumItalic.ttf"
G=$P/app/build.gradle
sed -i -e 's/namespace "org.libsdl.app"/namespace "com.mussyg.elfbowling"/' \
  -e 's/^\( *\)defaultConfig {/&\n        applicationId "com.mussyg.elfbowling"/' \
  -e 's/minSdkVersion 19/minSdkVersion 21/' -e 's/APP_PLATFORM=android-19/APP_PLATFORM=android-21", "SUPPORT_HARFBUZZ=false/' \
  -e "s/abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'/abiFilters 'arm64-v8a', 'x86_64'/" "$G"
sed -i -e 's/android:name="SDLActivity"/android:name="com.mussyg.elfbowling.ElfBowlActivity" android:screenOrientation="sensorLandscape"/' \
  "$P/app/src/main/AndroidManifest.xml"
sed -i 's/>Game</>Elf Bowling</' "$P/app/src/main/res/values/strings.xml"
echo "ndk.dir=$ANDROID_HOME/ndk/26.3.11579264" > "$P/local.properties"; echo "sdk.dir=$ANDROID_HOME" >> "$P/local.properties"
(cd "$P" && chmod +x gradlew && ./gradlew --no-daemon -q assembleDebug)
mkdir -p build/dist
cp "$P/app/build/outputs/apk/debug/app-debug.apk" build/dist/elfbowl.apk
ls -la build/dist/elfbowl.apk
