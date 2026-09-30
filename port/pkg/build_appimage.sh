#!/bin/sh
# Wrap a Linux x86_64 elfbowl binary as an AppImage (make -C port pkg-linux).
#   pkg/build_appimage.sh BINARY OUT.AppImage
# Bundles SDL2, SDL2_ttf and their non-system libraries (ldd, minus glibc and the graphics/audio
# stack every desktop has) and the free fonts (port/web/fonts). Nothing from the exe: at first run
# the game looks for the user's copy ($ELFBOWL_EXE, "Elf Bowling.exe" next to the AppImage or in
# the current directory, ~/.local/share/NStorm/Elf Bowling/) or asks with zenity/kdialog
# (pkg/firstrun.c). appimagetool is downloaded to build/deps if missing and run without FUSE.
set -eu
cd "$(dirname "$0")/.."
BIN=$1 OUT=$2
APPDIR=../build/pkg/linux/ElfBowling.AppDir
TOOL=../build/deps/appimagetool-x86_64.AppImage
rm -rf "$APPDIR"; mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib" "$APPDIR/usr/share/elfbowl/fonts" "$(dirname "$OUT")"
cp "$BIN" "$APPDIR/usr/bin/elfbowl"; strip "$APPDIR/usr/bin/elfbowl"
ldd "$BIN" | awk '/=> \// {print $3}' | while read -r lib; do
  case $(basename "$lib") in
    ld-linux*|libc.so*|libm.so*|libdl.so*|libpthread.so*|librt.so*|libstdc++*|libgcc_s*|libz.so*|\
    libGL*|libEGL*|libX*|libxcb*|libwayland*|libasound*|libpulse*|libdbus*|libdrm*|libgbm*|libudev*|libsystemd*|libcap*|libgcrypt*|liblzma*|liblz4*|libzstd*|libgpg-error*|libxkbcommon*|libdecor*|libffi*|libsndfile*|libFLAC*|libogg*|libvorbis*|libopus*|libmp3lame*|libmpg123*|libglib*|libapparmor*|libbsd*|libmd.so*|libpcre2*|libasyncns*|libexpat*) ;;
    *) cp -L "$lib" "$APPDIR/usr/lib/" ;;
  esac
done
cp web/fonts/LiberationSans-Regular.ttf web/fonts/LiberationSans-Bold.ttf web/fonts/LICENSE-* "$APPDIR/usr/share/elfbowl/fonts/"
cp web/fonts/texgyrechorus-mediumitalic.otf "$APPDIR/usr/share/elfbowl/fonts/Z003-MediumItalic.ttf"
cat > "$APPDIR/AppRun" <<'RUN'
#!/bin/sh
HERE=$(dirname "$(readlink -f "$0")")
export LD_LIBRARY_PATH="$HERE/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export PORT_FONT_DIR="${PORT_FONT_DIR:-$HERE/usr/share/elfbowl/fonts}"
export PORT_FULLSCREEN="${PORT_FULLSCREEN:-1}"     # PORT_FULLSCREEN=0 for a window; F11 toggles
if [ -z "${ELFBOWL_EXE:-}" ]; then
  for d in "$(dirname "${APPIMAGE:-$0}")" "$PWD" "$HOME"; do
    [ -f "$d/Elf Bowling.exe" ] && { export ELFBOWL_EXE="$d/Elf Bowling.exe"; break; }
  done
fi
exec "$HERE/usr/bin/elfbowl" "$@"
RUN
chmod +x "$APPDIR/AppRun"
cat > "$APPDIR/elfbowl.desktop" <<'DESK'
[Desktop Entry]
Type=Application
Name=Elf Bowling
Exec=elfbowl
Icon=elfbowl
Categories=Game;
DESK
python3 - "$APPDIR/elfbowl.png" <<'PY'
import struct, sys, zlib
n = 128  # a bowling ball: dark blue disc, three finger holes
def px(x, y):
    dx, dy = x - n / 2, y - n / 2
    if dx * dx + dy * dy > (n / 2 - 2) ** 2: return b'\0\0\0\0'
    for hx, hy in ((52, 40), (70, 36), (64, 58)):
        if (x - hx) ** 2 + (y - hy) ** 2 < 49: return b'\x10\x10\x18\xff'
    return b'\x20\x40\xa0\xff'
raw = b''.join(b'\0' + b''.join(px(x, y) for x in range(n)) for y in range(n))
chunk = lambda t, d: struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
open(sys.argv[1], 'wb').write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', n, n, 8, 6, 0, 0, 0))
                              + chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b''))
PY
[ -x "$TOOL" ] || { mkdir -p "$(dirname "$TOOL")"; wget -q -O "$TOOL" \
  https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage; chmod +x "$TOOL"; }
ARCH=x86_64 "$TOOL" --appimage-extract-and-run "$APPDIR" "$OUT" >/dev/null 2>&1 || ARCH=x86_64 "$TOOL" --appimage-extract-and-run "$APPDIR" "$OUT"
ls -la "$OUT"
