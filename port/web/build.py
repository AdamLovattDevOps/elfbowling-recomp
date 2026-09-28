#!/usr/bin/env python3
"""Build the WebAssembly version of the native port into build/web/dist.

    python3 port/web/build.py [--exe "orig/1999/Elf Bowling.exe"] [--jobs N] [--clean]

Needs Emscripten (em++ on PATH, e.g. after `emsdk_env`). Works the same on macOS, Linux and
Windows (emsdk's own python is enough), which is why this is a script and not make rules:
port/web.mk's `make -C port web` just runs it.

The same sources as `make native` (port/Makefile): every src_match/*.cpp, the port glue, the VCL
subset and the Win32 shim, plus port/web/web_glue.cpp, compiled with em++ -fwasm-exceptions. See
port/web/README.md for the design (no ASYNCIFY, no pthreads: a cooperative TThread).

--exe is needed to generate game_data.cpp (the .data initial values, as `make native` does) and is
copied into dist/data/ with the fonts: the player's browser loads it from there (loader.js). Leave
it out and dist/ has no game data (then put your own exe at dist/data/elfbowl.exe).

Output (build/web/dist):
    index.html loader.js elfbowl.js elfbowl.wasm
    data/manifest.json data/elfbowl.exe data/fonts/*        (see loader.js for the manifest)
    data/hires/x3/*         the "hires" pack, from --hires (default build/hires/x3) when it is finished
Objects go to build/web/obj; a unit is rebuilt when it, a header, or the flags changed.
"""
import argparse
import concurrent.futures as cf
import hashlib
import json
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

PORT = Path(__file__).resolve().parent.parent          # port/
ROOT = PORT.parent                                     # repo root
OUT = ROOT / 'build' / 'web'
OBJ = OUT / 'obj'
DIST = OUT / 'dist'
WEB = PORT / 'web'

OPT = ['-O2']
EH = ['-fwasm-exceptions']          # the game and the VCL throw (Exception, EResNotFound, CoopYield)
SDL = ['-sUSE_SDL=2', '-sUSE_SDL_TTF=2']
# as port/Makefile's NATIVE_CXXFLAGS / NATIVE_WNO (the matched source's by-design noise)
NATIVE_WNO = ['-Wno-unknown-pragmas', '-Wno-return-type-c-linkage', '-Wno-deprecated-copy-with-user-provided-copy',
              '-Wno-mismatched-tags', '-Wno-deprecated-register', '-Wno-register']
GAME = ['-std=gnu++17', '-DPORT', '-fms-extensions', '-fsigned-char', '-fno-strict-aliasing', '-fwrapv',
        '-I' + str(PORT / 'include'), '-I' + str(ROOT / 'include')] + NATIVE_WNO + ['-ferror-limit=0']
VCL = ['-std=c++17', '-Wall', '-Wextra', '-Wno-unused-parameter', '-I' + str(PORT / 'include')]
SHIM = ['-std=c99', '-Wall', '-Wextra', '-Wno-unused-parameter', '-I' + str(PORT / 'include')]
VCL_SRCS = ['system', 'classes', 'thread', 'controls', 'forms', 'dfm', 'scktcomp', 'entry', 'rtl', 'main']

LINK = [
    # Win32 never maps the first 64 KB, and the game relies on it: IS_INTRESOURCE(p) is
    # (p >> 16) == 0, so FindResource("TForm1") and ("PackedFile") would read as integer ids if a
    # string literal sat below 0x10000, as wasm's static data does by default (from 1024). Static
    # data, and the stack and heap after it, start at 64 KB instead.
    '-sGLOBAL_BASE=65536',
    # The matched source casts callbacks to wider types, e.g. (SpriteCb)fn_410680 where fn_410680
    # takes fewer arguments, or a void function stored where an int one is expected. Harmless under
    # x86 cdecl (the caller pops the arguments), but a wasm call_indirect whose type differs from
    # the target traps ("function signature mismatch"). The casts are part of the byte-matched
    # source, so the web build emulates them (Binaryen --fpcast-emu: every indirect call goes
    # through an i64 thunk; small cost for this game) instead of changing src_match/.
    '-sEMULATE_FUNCTION_POINTER_CASTS=1',
    '-sALLOW_MEMORY_GROWTH=1', '-sINITIAL_MEMORY=67108864', '-sSTACK_SIZE=1048576',
    '-sINVOKE_RUN=0',                   # loader.js calls main after the data is in the FS and the player tapped
    '-sFORCE_FILESYSTEM=1', '-lidbfs.js',
    '-sEXPORTED_RUNTIME_METHODS=callMain,FS,IDBFS,ENV,cwrap,ccall',
    '-sEXPORTED_FUNCTIONS=_main,_web_key,_web_pack_ready,_web_hd',
    '-sENVIRONMENT=web',
    '-sMIN_SAFARI_VERSION=150200',      # wasm exceptions (legacy) and BigInt: iOS 15.2+
    '--profiling-funcs',                # function names in stack traces (a real-device report is readable)
]

# The files dist/data/ serves. `name` is the published file, `path` where it goes in the Emscripten
# FS. The fonts are free substitutes for the Windows faces the game asks for (README.md, "Fonts"):
# the shim looks them up by these file names in $PORT_FONT_DIR (gdi.c, k_faces).
FONTS = [
    ('LiberationSans-Regular.ttf', '/fonts/LiberationSans-Regular.ttf'),
    ('LiberationSans-Bold.ttf', '/fonts/LiberationSans-Bold.ttf'),
    # TeX Gyre Chorus is a Zapf Chancery clone, like URW Chancery L / Z003, the shim's Linux
    # stand-in for Lucida Handwriting and Lucida Calligraphy; it is filed under that name.
    ('texgyrechorus-mediumitalic.otf', '/fonts/Z003-MediumItalic.ttf'),
]


def find_emxx():
    """[python, em++.py] when emsdk's script is next to em++ (fast on Windows), else [em++]."""
    exe = shutil.which('em++')
    if not exe:
        sys.exit('em++ not found: run emsdk_env first')
    py = Path(exe).with_name('em++.py')
    return [sys.executable, str(py)] if py.exists() else [exe]


def header_stamp():
    newest = 0.0
    for d in (PORT / 'include', ROOT / 'include'):
        for p in d.rglob('*'):
            if p.is_file():
                newest = max(newest, p.stat().st_mtime)
    return newest


def units(game_data):
    """(source, flags, object) for every translation unit."""
    rtl = ['-include', 'bcb_rtl.h']
    out = []
    for src in sorted((ROOT / 'src_match').glob('*.cpp')):
        out.append((src, GAME + rtl, OBJ / 'game' / (src.stem + '.o')))
    for rel in ('vcl/elf_glue.cpp', 'native/game_native.cpp'):
        src = PORT / rel
        out.append((src, GAME + rtl, OBJ / 'port' / (src.stem + '.o')))
    out.append((game_data, GAME + rtl, OBJ / 'port' / 'game_data.o'))
    for n in VCL_SRCS:
        src = PORT / 'vcl' / (n + '.cpp')
        out.append((src, VCL, OBJ / 'vcl' / (n + '.o')))
    for src in sorted((PORT / 'src').glob('*.c')):
        out.append((src, SHIM, OBJ / 'shim' / (src.stem + '.o')))
    out.append((WEB / 'web_glue.cpp', VCL, OBJ / 'web' / 'web_glue.o'))
    return out


def compile_one(emxx, src, flags, obj, stamp):
    cmd = emxx + ([] if src.suffix == '.cpp' else ['-x', 'c']) + OPT + EH + SDL + flags + ['-c', str(src), '-o', str(obj)]
    sig = hashlib.sha1(' '.join(cmd).encode()).hexdigest()
    sigf = obj.with_suffix('.sig')
    if obj.exists() and sigf.exists() and sigf.read_text() == sig and obj.stat().st_mtime >= max(src.stat().st_mtime, stamp):
        return src, None, False
    obj.parent.mkdir(parents=True, exist_ok=True)
    obj.unlink(missing_ok=True)
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode:
        return src, (r.stdout + r.stderr), True
    sigf.write_text(sig)
    return src, (r.stderr or None), True


def gen_game_data(exe):
    out = OUT / 'game_data.cpp'
    OUT.mkdir(parents=True, exist_ok=True)
    subprocess.check_call([sys.executable, str(PORT / 'tools' / 'gen_game_data.py'), '--exe', str(exe),
                           '--header', str(ROOT / 'include' / 'elf' / 'game.h'), '--src', str(ROOT / 'src_match'),
                           '--out', str(out)])
    return out


def hires_pack(src):
    """The "hires" pack: tools/upscale.py's cache (src, build/hires/x3) under data/hires/x3/,
    read by hires.c at /hires/x3. None when there is no finished cache."""
    if not src or not (src / '.done').exists() or not (src / 'manifest.tsv').exists():
        return None
    dst = DIST / 'data' / 'hires' / 'x3'
    if dst.exists():
        shutil.rmtree(dst)
    dst.mkdir(parents=True)
    files, h = [], hashlib.sha1()
    for p in sorted(src.iterdir()):
        if p.suffix not in ('.png', '.idx', '.tsv'):
            continue
        shutil.copy2(p, dst / p.name)
        h.update(p.name.encode() + b'\0' + p.read_bytes())
        files.append({'name': 'hires/x3/' + p.name, 'path': '/hires/x3/' + p.name, 'size': p.stat().st_size})
    return {'version': h.hexdigest()[:16], 'files': files}


def dist(exe, hires=None):
    DIST.mkdir(parents=True, exist_ok=True)
    (DIST / 'data' / 'fonts').mkdir(parents=True, exist_ok=True)
    build = str(int(time.time()))
    for n in ('elfbowl.js', 'elfbowl.wasm'):
        shutil.copy2(OUT / n, DIST / n)
    # every URL the page loads carries the build id, so a new build is never mixed with a cached old one
    for n in ('index.html', 'loader.js', 'manifest.webmanifest'):
        (DIST / n).write_text((WEB / n).read_text(encoding='utf-8').replace('@BUILD@', build), encoding='utf-8')
    files = []
    for name, fspath in FONTS:
        shutil.copy2(WEB / 'fonts' / name, DIST / 'data' / 'fonts' / name)
        files.append({'name': 'fonts/' + name, 'path': fspath, 'size': (WEB / 'fonts' / name).stat().st_size})
    for lic in (WEB / 'fonts').glob('LICENSE-*'):
        shutil.copy2(lic, DIST / 'data' / 'fonts' / lic.name)
    if exe:
        shutil.copy2(exe, DIST / 'data' / 'elfbowl.exe')
    exe_d = DIST / 'data' / 'elfbowl.exe'
    if exe_d.exists():
        files.append({'name': 'elfbowl.exe', 'path': '/data/Elf Bowling.exe', 'size': exe_d.stat().st_size})
    h = hashlib.sha1()
    for f in files:
        h.update((DIST / 'data' / f['name']).read_bytes())
    # "packs": optional data loaded after the game has started and cached the same way:
    # "hires", the ESRGAN art ({"version", "files": [...]}), when --hires has a finished cache.
    packs = {}
    hp = hires_pack(hires)
    if hp:
        packs['hires'] = hp
    manifest = {'version': h.hexdigest()[:16], 'build': build, 'files': files, 'packs': packs}
    (DIST / 'data' / 'manifest.json').write_text(json.dumps(manifest, indent=1))
    return manifest


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--exe', help='the original Elf Bowling.exe (game_data.cpp, and dist/data)')
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    ap.add_argument('--clean', action='store_true')
    ap.add_argument('--hires', default=str(ROOT / 'build' / 'hires' / 'x3'),
                    help='the art cache to publish as the "hires" pack (default build/hires/x3; "" for none)')
    a = ap.parse_args()
    if a.clean and OUT.exists():
        shutil.rmtree(OUT)
    exe = Path(a.exe).resolve() if a.exe else None
    if exe:
        game_data = gen_game_data(exe)
    else:
        game_data = OUT / 'game_data.cpp'
        if not game_data.exists():
            sys.exit('need --exe once, to generate game_data.cpp')
    emxx = find_emxx()
    stamp = header_stamp()
    todo = units(game_data)
    built = 0
    # Build the ports into the Emscripten cache first, one step at a time: parallel em++ runs race
    # for the cache, and on Windows an em++ that builds SDL2_ttf trips over its own cache lock
    # while building freetype for it ("attempt to lock the cache while a parent process is holding
    # the lock"). embuilder, dependencies first, works.
    for libs in (['sdl2', 'freetype', 'harfbuzz'], ['sdl2_ttf']):
        subprocess.check_call(emxx[:-1] + [str(Path(emxx[-1]).with_name('embuilder.py'))] + ['build'] + libs
                              if emxx[-1].endswith('.py') else [shutil.which('embuilder'), 'build'] + libs)
    first = [compile_one(emxx, todo[0][0], todo[0][1], todo[0][2], stamp)]
    with cf.ThreadPoolExecutor(a.jobs) as pool:
        rest = pool.map(lambda u: compile_one(emxx, u[0], u[1], u[2], stamp), todo[1:])
        for src, log, did in first + list(rest):
            built += did
            if log:
                print(f'--- {src.relative_to(ROOT)}\n{log}', file=sys.stderr)
    failed = [u[0] for u in todo if not u[2].exists()]
    if failed:
        sys.exit('failed: ' + ' '.join(str(f.relative_to(ROOT)) for f in failed))
    print(f'compiled {built} of {len(todo)} units')
    objs = [str(u[2]) for u in todo]
    link = emxx + OPT + EH + SDL + LINK + objs + ['-o', str(OUT / 'elfbowl.js')]
    subprocess.check_call(link)
    m = dist(exe, Path(a.hires) if a.hires else None)
    hp = m['packs'].get('hires')
    print(f'{DIST}: build {m["build"]}, data version {m["version"]}, {len(m["files"])} data files'
          + (f', hires pack {len(hp["files"])} files' if hp else ', no hires pack'))


if __name__ == '__main__':
    main()
