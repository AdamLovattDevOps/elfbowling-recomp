#!/bin/sh
# Build a signed "Elf Bowling.ipa" for sideloading onto your own iPhone/iPad (make -C port pkg-ios),
# the same way as jungle-recomp's tools/build_ipa.sh.
#   pkg/build_ipa.sh [TEAM_ID]   -> build/dist/Elf Bowling.ipa
#   IOS_SIM=1 pkg/build_ipa.sh   -> build/pkg/ios-xcode/Release-iphonesimulator/ElfBowling.app (unsigned)
# Needs Xcode signed in to an Apple ID (Settings > Accounts) whose team has the device registered,
# SDL2's source (SDL, default ../jungle-recomp/build/deps/SDL2-2.30.9-src; its static libraries are
# built into ../jungle-recomp/build/deps/sdl2-ios-build if missing) and SDL2_ttf's source at
# build/deps/src/SDL2_ttf-2.22.0. Nothing from the exe is in the app: the first run asks for the
# user's copy (the app's Documents folder via Files/Finder, or a document picker; pkg/firstrun.c).
set -eu
cd "$(dirname "$0")/../.."
TEAM=${1:-${IOS_TEAM:?usage: build_ipa.sh <Apple team ID> (or set IOS_TEAM)}}
SDL=${SDL:-$PWD/../jungle-recomp/build/deps/SDL2-2.30.9-src}
SDLB=$PWD/../jungle-recomp/build/deps/sdl2-ios-build
for sdk in iphoneos iphonesimulator; do
  [ -f "$SDLB/Release-$sdk/libSDL2.a" ] || \
    xcodebuild -project "$SDL/Xcode/SDL/SDL.xcodeproj" -target "Static Library-iOS" -configuration Release \
      -sdk $sdk -arch arm64 ONLY_ACTIVE_ARCH=NO SYMROOT="$SDLB" >/dev/null
done
X=build/pkg/ios-xcode
cmake -S port/pkg/ios -B $X -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DTEAM="$TEAM" -DSDL="$SDL" \
  -DSDL_LIB_DIR="$SDLB" >/dev/null
if [ "${IOS_SIM:-}" = 1 ]; then
  xcodebuild -project $X/ElfBowling.xcodeproj -scheme ElfBowling -configuration Release -sdk iphonesimulator \
    -destination 'generic/platform=iOS Simulator' -derivedDataPath build/pkg/ios-sim ARCHS=arm64 \
    CODE_SIGNING_ALLOWED=NO -quiet build
  ls -d $X/Release-iphonesimulator/ElfBowling.app
  exit 0
fi
rm -rf $X/ElfBowling.xcarchive $X/ipa
xcodebuild -project $X/ElfBowling.xcodeproj -scheme ElfBowling -configuration Release \
  -sdk iphoneos -destination generic/platform=iOS -allowProvisioningUpdates \
  -archivePath $X/ElfBowling.xcarchive archive -quiet
cat > $X/export.plist <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
  <key>method</key><string>development</string>
  <key>teamID</key><string>$TEAM</string>
  <key>signingStyle</key><string>automatic</string>
  <key>compileBitcode</key><false/>
</dict></plist>
PLIST
xcodebuild -exportArchive -archivePath $X/ElfBowling.xcarchive -exportPath $X/ipa \
  -exportOptionsPlist $X/export.plist -allowProvisioningUpdates -quiet
mkdir -p build/dist
cp "$X/ipa/Elf Bowling.ipa" "build/dist/Elf Bowling.ipa" 2>/dev/null || cp $X/ipa/*.ipa "build/dist/Elf Bowling.ipa"
ls -la "build/dist/Elf Bowling.ipa"
