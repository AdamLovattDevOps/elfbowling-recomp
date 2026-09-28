#!/usr/bin/env python3
"""Build the high-resolution art cache for the native port from your own exe.

Every bitmap in the NVDPACKFILE archive (the logic of tools/unpack.py) is turned
into an RGBA image with the game's own transparency, upscaled 4x with
Real-ESRGAN's anime model (realesrgan-ncnn-vulkan, GPU through Vulkan/MoltenVK,
the binary jungle-recomp uses), then resampled to 3x with Lanczos. SMALL_TEXT
bitmaps (type a few pixels tall) skip the model: plain Lanczos 3x. The native
build draws these in place of the 8-bit casts (port/src/hires.c, docs/HIRES.md).

Usage:  tools/upscale.py [--exe PATH] [--out DIR] [--esrgan DIR] [--only NAME]

Output, under OUT/x3/ (default build/hires/x3):
  NAME.png      RGBA, 3x. Alpha is the mode-1 matte of the game (the white
                background connected to the border is transparent, what
                fn_403f04 computes); colour is bled into the transparent area
                first, so the upscaler never mixes the white background into
                sprite edges (no fringes).
  NAME.m2.png   only when it differs: the mode-2 mask (every white pixel
                transparent, fn_4045f0).
  NAME.op.png   only when the bitmap has white: the plain opaque picture, for
                casts drawn without a mask.
  NAME.idx      the original 1x pixels: "EIDX", u32 w, u32 h, w*h indices,
                top-down. The game checks its loaded cast against these before
                it uses the upscaled art, and makes the variant masks from them.
  manifest.tsv  asset, w, h, crc32 of the 1x indices, variants.

The 4x intermediates live in a temporary directory and are deleted after each
batch, so the peak extra disk use is one batch. Nothing here is committed: the
art is the exe's. Interrupted runs resume: finished assets are skipped.
"""
import argparse
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import time
import zlib
from concurrent.futures import ProcessPoolExecutor

import numpy as np
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from pe import PE          # noqa: E402
import rsrc                # noqa: E402
from unpack import entries  # noqa: E402

BG = 255        # g_455531 with the game's palette (entry 0 is black): white background
BATCH = 24      # assets per ESRGAN run (bounds the 4x scratch space)
ANIME = 'realesrgan-x4plus-anime'
# Bitmaps that are mostly small type: the anime model redraws letters as brush strokes,
# the anime video model keeps them legible.
TEXT = {'copyright.bmp', 'clickhere1.bmp', 'clickhere2.bmp', 'nvdaddress.bmp', 'nvdmess2.bmp', 'hint1a.bmp',
        'merrychristmas.bmp', 'theplanetoff.bmp', 'theplanetonbmp', 'theplaneton.bmp', 'top3aggressive.bmp',
        'elfcrew75.bmp', 'nstormoff.bmp', 'nstormon.bmp', 'scoreboard.bmp'}
# Bitmaps whose type is only a few pixels tall (the copyright line under the title logo, the
# "(c) 1999 NVISION DESIGN" line of the NStorm buttons): both models turn some of those letters
# into other letters ("Distribute 1reely"), so these skip the model and use plain Lanczos 3x,
# which never invents a glyph.
SMALL_TEXT = {'copyright.bmp', 'nstormoff.bmp', 'nstormon.bmp'}


def pack_bitmaps(exe):
    res = {tuple(p): data for p, data in rsrc.walk(PE(exe))}
    d = next(v for k, v in res.items() if k[0] == 'NVDPACKFILE')
    for name, cs, us, off in entries(d):
        clean = name.lstrip('#0123456789%$')
        if not clean.lower().endswith('.bmp'):
            continue
        raw = zlib.decompress(d[off:off + cs])
        yield clean, raw


def decode_bmp(raw):
    """8-bit BI_RGB BMP -> (indices top-down HxW uint8, palette 256x3 uint8)."""
    off, = struct.unpack_from('<I', raw, 10)
    hs, w, h, planes, bpp, comp = struct.unpack_from('<IiiHHI', raw, 14)
    if bpp != 8 or comp != 0:
        raise ValueError('not 8-bit BI_RGB')
    ncol = struct.unpack_from('<I', raw, 46)[0] or 256
    pal = np.frombuffer(raw, np.uint8, ncol * 4, 14 + hs).reshape(ncol, 4)[:, 2::-1]
    pal = np.vstack([pal, np.zeros((256 - ncol, 3), np.uint8)])
    stride = (w + 3) & ~3
    px = np.frombuffer(raw, np.uint8, stride * abs(h), off).reshape(abs(h), stride)[:, :w]
    if h > 0:
        px = px[::-1]
    return np.ascontiguousarray(px), pal


def matte1(white):
    """fn_403f04: background pixels 4-connected to the border (True = transparent)."""
    h, w = white.shape
    m = np.zeros_like(white)
    m[0, :] = white[0, :]; m[-1, :] = white[-1, :]; m[:, 0] = white[:, 0]; m[:, -1] = white[:, -1]
    while True:
        g = m.copy()
        g[1:, :] |= m[:-1, :]; g[:-1, :] |= m[1:, :]; g[:, 1:] |= m[:, :-1]; g[:, :-1] |= m[:, 1:]
        g &= white
        if (g == m).all():
            return m
        m = g


def bleed(rgb, transparent):
    """Fill transparent pixels with the mean of their opaque neighbours, growing outwards."""
    rgb = rgb.astype(np.float32)
    known = ~transparent
    if not known.any():
        return rgb.astype(np.uint8)
    h, w = known.shape
    for _ in range(64):
        if known.all():
            break
        acc = np.zeros_like(rgb); cnt = np.zeros((h, w), np.float32)
        for dy, dx in ((-1, 0), (1, 0), (0, -1), (0, 1), (-1, -1), (-1, 1), (1, -1), (1, 1)):
            k = np.zeros((h, w), bool); c = np.zeros_like(rgb)
            ys = slice(max(dy, 0), h + min(dy, 0)); yd = slice(max(-dy, 0), h + min(-dy, 0))
            xs = slice(max(dx, 0), w + min(dx, 0)); xd = slice(max(-dx, 0), w + min(-dx, 0))
            k[yd, xd] = known[ys, xs]; c[yd, xd] = rgb[ys, xs]
            acc += c * k[..., None]; cnt += k
        grow = ~known & (cnt > 0)
        rgb[grow] = acc[grow] / cnt[grow][:, None]
        known = known | grow
    rgb[~known] = rgb[known].mean(axis=0)
    return np.clip(rgb + 0.5, 0, 255).astype(np.uint8)


def rgba_for(px, pal, transparent):
    rgb = bleed(pal[px], transparent)
    a = np.where(transparent, 0, 255).astype(np.uint8)
    return np.dstack([rgb, a])


def blur(a, sigma):
    """Separable Gaussian blur of an HxWxC float array (edges clamped)."""
    r = int(sigma * 3 + 0.5)
    k = np.exp(-0.5 * (np.arange(-r, r + 1) / sigma) ** 2)
    k /= k.sum()
    for axis in (0, 1):
        pad = [(0, 0)] * a.ndim
        pad[axis] = (r, r)
        p = np.pad(a, pad, mode='edge')
        n = a.shape[axis]
        a = sum(k[i] * np.take(p, range(i, i + n), axis=axis) for i in range(2 * r + 1))
    return a


def finish(job):
    """4x ESRGAN colour + 4x ESRGAN mask -> Lanczos 3x RGBA; alpha tightened to a ~1-pixel ramp.
    The colour's low frequencies are pinned to the original's (the upscaler tints flat areas by a
    few levels, which shows as boxes where a cast's background meets the scene's)."""
    src, srca, dst, w, h, ref = job
    im = Image.open(src).convert('RGBA').resize((w * 3, h * 3), Image.LANCZOS)
    rgb = np.asarray(im)[..., :3].astype(np.float32)
    nn = np.repeat(np.repeat(ref.astype(np.float32), 3, 0), 3, 1)
    rgb = np.clip(rgb + blur(nn - rgb, 2.5) + 0.5, 0, 255).astype(np.uint8)
    if srca:
        am = Image.open(srca).convert('L').resize((w * 3, h * 3), Image.LANCZOS)
        a = np.asarray(am).astype(np.float32) / 255.0
        os.remove(srca)
    else:
        a = np.asarray(im)[..., 3].astype(np.float32) / 255.0
    t = np.clip((a - 0.2) / 0.6, 0, 1)
    a = (t * t * (3 - 2 * t) * 255 + 0.5).astype(np.uint8)       # smoothstep(0.2, 0.8)
    arr = np.dstack([rgb, a])
    tmp = dst + '.tmp'
    Image.fromarray(arr, 'RGBA').save(tmp, 'PNG', compress_level=6)
    os.replace(tmp, dst)
    os.remove(src)
    return dst


def lanczos3(job):
    """SMALL_TEXT: Lanczos 3x of the bled colour and of the mask; no model."""
    rgba, tr, dst = job
    h, w = tr.shape
    im = Image.fromarray(rgba[..., :3], 'RGB').resize((w * 3, h * 3), Image.LANCZOS)
    rgb = np.asarray(im)
    am = Image.fromarray(np.where(tr, 0, 255).astype(np.uint8), 'L').resize((w * 3, h * 3), Image.LANCZOS)
    t = np.clip((np.asarray(am).astype(np.float32) / 255.0 - 0.2) / 0.6, 0, 1)
    a = (t * t * (3 - 2 * t) * 255 + 0.5).astype(np.uint8)       # smoothstep(0.2, 0.8), as finish()
    tmp = dst + '.tmp'
    Image.fromarray(np.dstack([rgb, a]), 'RGBA').save(tmp, 'PNG', compress_level=6)
    os.replace(tmp, dst)
    return dst


ESRGAN_URL = ('https://github.com/xinntao/Real-ESRGAN/releases/download/v0.2.5.0/'
              'realesrgan-ncnn-vulkan-20220424-%s.zip')


def fetch_esrgan(dest):
    """Download the official realesrgan-ncnn-vulkan build for this OS into dest."""
    import io, platform, urllib.request, zipfile
    osname = {'Darwin': 'macos', 'Linux': 'ubuntu', 'Windows': 'windows'}[platform.system()]
    print('fetching Real-ESRGAN (%s) ...' % osname, flush=True)
    data = urllib.request.urlopen(ESRGAN_URL % osname).read()
    os.makedirs(dest, exist_ok=True)
    zipfile.ZipFile(io.BytesIO(data)).extractall(dest)
    for f in os.listdir(dest):
        if f.startswith('realesrgan-ncnn-vulkan'):
            os.chmod(os.path.join(dest, f), 0o755)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe', default=os.environ.get('ELFBOWL_EXE', os.path.join(ROOT, 'orig/1999/Elf Bowling.exe')))
    ap.add_argument('--out', default=os.path.join(ROOT, 'build/hires'))
    ap.add_argument('--esrgan', default=os.path.join(ROOT, 'build/deps/esrgan'))
    ap.add_argument('--only', help='one asset, e.g. Santa1.bmp')
    a = ap.parse_args()
    exe = os.path.join(a.esrgan, 'realesrgan-ncnn-vulkan')
    if not os.path.exists(exe) and not os.path.exists(exe + '.exe'):
        fetch_esrgan(a.esrgan)
    if os.name == 'nt':
        exe += '.exe'
    if not os.path.exists(exe) and not (a.only and a.only.lower() in SMALL_TEXT):
        sys.exit('realesrgan-ncnn-vulkan not found in %s (see docs/HIRES.md)' % a.esrgan)
    out = os.path.join(a.out, 'x3')
    os.makedirs(out, exist_ok=True)

    todo, manifest = [], {}
    for name, raw in pack_bitmaps(a.exe):
        if a.only and name.lower() != a.only.lower():
            continue
        try:
            px, pal = decode_bmp(raw)
        except ValueError:
            print('skip %s (not 8-bit)' % name)
            continue
        h, w = px.shape
        stem = os.path.join(out, name[:-4])
        with open(stem + '.idx.tmp', 'wb') as f:
            f.write(b'EIDX' + struct.pack('<II', w, h) + px.tobytes())
        os.replace(stem + '.idx.tmp', stem + '.idx')
        white = px == BG
        m1 = matte1(white)
        variants = [('', m1)]
        if (white != m1).any():
            variants.append(('.m2', white))
        if white.any():                 # unmasked use: the plain picture (dithered whites stay dithered)
            variants.append(('.op', np.zeros_like(white)))
        manifest[name] = (w, h, zlib.crc32(px.tobytes()) & 0xffffffff, ','.join(v or 'm1' for v, _ in variants))
        for suffix, tr in variants:
            dst = stem + suffix + '.png'
            if not os.path.exists(dst):
                todo.append((name, suffix, px, pal, tr, dst))
    small = [t for t in todo if t[0].lower() in SMALL_TEXT]
    todo = [t for t in todo if t[0].lower() not in SMALL_TEXT]
    for name, suffix, px, pal, tr, dst in small:
        lanczos3((rgba_for(px, pal, tr), tr, dst))
        print('  lanczos 3x  %s%s' % (name[:-4], suffix), flush=True)
    todo.sort(key=lambda t: t[0].lower() in TEXT)

    print('%d assets, %d images to upscale' % (len(manifest), len(todo)), flush=True)
    t0 = time.time()
    batches, cur = [], []
    for t in todo:                      # one model per batch
        if cur and (len(cur) == BATCH or (cur[-1][0].lower() in TEXT) != (t[0].lower() in TEXT)):
            batches.append(cur); cur = []
        cur.append(t)
    if cur:
        batches.append(cur)
    done = 0
    for batch in batches:
        model = 'realesr-animevideov3-x4' if batch[0][0].lower() in TEXT else ANIME
        with tempfile.TemporaryDirectory(prefix='elfbowl-hires-', dir=os.path.join(ROOT, 'build')) as tmp:
            src, up = os.path.join(tmp, 'src'), os.path.join(tmp, 'up')
            os.makedirs(src); os.makedirs(up)
            jobs = []
            for i, (name, suffix, px, pal, tr, dst) in enumerate(batch):
                f = '%03d.png' % i
                rgba = rgba_for(px, pal, tr)
                Image.fromarray(rgba, 'RGBA').save(os.path.join(src, f))
                fa = None
                if tr.any():        # the mask itself through the model: smooth contours, not bicubic steps
                    fa = '%03d_a.png' % i
                    Image.fromarray(np.where(tr, 0, 255).astype(np.uint8), 'L').convert('RGB').save(os.path.join(src, fa))
                jobs.append((os.path.join(up, f), fa and os.path.join(up, fa), dst, px.shape[1], px.shape[0], rgba[..., :3]))
            subprocess.run([exe, '-i', src, '-o', up, '-n', model, '-s', '4', '-f', 'png',
                            '-m', os.path.join(a.esrgan, 'models')], check=True,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            shutil.rmtree(src)
            with ProcessPoolExecutor() as ex:
                list(ex.map(finish, jobs))
        done += len(batch)
        print('  %3d/%d  %5.0f s  %s' % (done, len(todo), time.time() - t0, model), flush=True)

    if not a.only:
        with open(os.path.join(out, 'manifest.tsv.tmp'), 'w') as f:
            for name in sorted(manifest):
                f.write('%s\t%d\t%d\t%08x\t%s\n' % ((name,) + manifest[name]))
        os.replace(os.path.join(out, 'manifest.tsv.tmp'), os.path.join(out, 'manifest.tsv'))
        open(os.path.join(out, '.done'), 'w').write('%d\n' % len(manifest))
    print('done:', out)


if __name__ == '__main__':
    main()
