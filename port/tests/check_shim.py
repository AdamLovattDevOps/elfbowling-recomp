"""Check shim_test output against the original exe, decoded independently in Python.

Usage: check_shim.py <Elf Bowling.exe> <screen.bmp> <report.txt> <mix.raw> [<driver.raw>]

- Resource sizes reported by the C resource shim must equal a Python PE walk.
- The dumped screen must equal the expected composition, pixel for pixel:
  BowlingLogo.bmp (palette lookup) with SRCCOPY / SRCAND over white / SRCPAINT
  over black, a clipped blit, a 32-bit DIB of palette colours, and a text box
  that may only use the DIB background, the OPAQUE bk colour and the text colour.
- The mixer dump (PORT_AUDIO_DUMP, S16LE stereo at the device rate) must contain
  the WAV's samples exactly (U8 -> S16 is (x - 128) << 8, mono duplicated).
- The SDL disk driver's file (whatever rate SDL chose; sdl2-compat forces
  44100 Hz) must hold a non-silent span of about the sound's duration.
"""
import struct
import sys
import zlib


def pe_resources(path):
    d = open(path, 'rb').read()
    pe = struct.unpack_from('<I', d, 0x3c)[0]
    opt = pe + 24
    nsec = struct.unpack_from('<H', d, pe + 6)[0]
    soff = opt + struct.unpack_from('<H', d, pe + 20)[0]
    secs = [struct.unpack_from('<IIII', d, soff + 40 * i + 8) for i in range(nsec)]

    def r2o(rva):
        for vs, va, rs, ro in secs:
            if va <= rva < va + max(vs, rs):
                return ro + rva - va
        raise ValueError(rva)

    base = r2o(struct.unpack_from('<I', d, opt + 96 + 16)[0])
    out = {}

    def key(e):
        if e & 0x80000000:
            o = base + (e & 0x7fffffff)
            n = struct.unpack_from('<H', d, o)[0]
            return d[o + 2:o + 2 + 2 * n].decode('utf-16le').upper()
        return e

    def walk(off, path):
        nn, ni = struct.unpack_from('<HH', d, base + off + 12)
        for i in range(nn + ni):
            nm, tgt = struct.unpack_from('<II', d, base + off + 16 + 8 * i)
            if tgt & 0x80000000:
                walk(tgt & 0x7fffffff, path + [key(nm)])
            else:
                rva, size = struct.unpack_from('<II', d, base + tgt)
                out.setdefault(tuple(path[:2]), d[r2o(rva):r2o(rva) + size])
    walk(0, [])
    return out


def pack_get(pk, want):
    n = struct.unpack_from('<I', pk, 0x10)[0]
    for i in range(n):
        o = 0x18 + 40 * i
        name = pk[o:o + 24].split(b'\0')[0].decode('latin1').lstrip('#4%')
        cs, us, off = struct.unpack_from('<III', pk, o + 24)
        if name == want:
            return zlib.decompress(pk[off:off + cs])
    raise KeyError(want)


def nearest(pal, rgb):
    best, bestd = 0, 1 << 30
    for i, c in enumerate(pal):
        dd = sum((a - b) ** 2 for a, b in zip(c, rgb))
        if dd < bestd:
            best, bestd = i, dd
            if dd == 0:
                break
    return best


def read_bmp24(path):
    b = open(path, 'rb').read()
    off = struct.unpack_from('<I', b, 10)[0]
    w, h = struct.unpack_from('<ii', b, 18)
    assert struct.unpack_from('<H', b, 28)[0] == 24
    stride = (w * 3 + 3) & ~3
    px = [[None] * w for _ in range(h)]
    for y in range(h):
        r = off + (h - 1 - y) * stride
        for x in range(w):
            bl, g, rd = b[r + 3 * x:r + 3 * x + 3]
            px[y][x] = (rd, g, bl)
    return w, h, px


def s16(path):
    b = open(path, 'rb').read()
    return struct.unpack('<%dh' % (len(b) // 2), b[:len(b) // 2 * 2])


def main():
    exe, scr, rep, raw = sys.argv[1:5]
    drv = sys.argv[5] if len(sys.argv) > 5 else None
    report = dict(l.strip().split('=', 1) for l in open(rep) if '=' in l)
    fails = []

    def check(cond, msg):
        if not cond:
            fails.append(msg)

    check(report.get('c_failures') == '0', 'C test reported failures: %s' % report.get('c_failures'))

    res = pe_resources(exe)
    for k, rk in [(('NVDPACKFILE', 'PACKEDFILE'), 'res_PACKEDFILE'), (('NVDPARMFILE', 'PARMS'), 'res_PARMS'),
                  (('NVDCASTFILE', 'CASTS'), 'res_CASTS'), ((10, 'TFORM1'), 'res_TFORM1'),
                  ((6, 4087), 'res_STRING_4087')]:
        check(int(report.get(rk, -1)) == len(res[k]), '%s: C %s, Python %d' % (rk, report.get(rk), len(res[k])))
    print('resources: %d checked' % 5)

    pk = res[('NVDPACKFILE', 'PACKEDFILE')]
    bmp = pack_get(pk, 'BowlingLogo.bmp')
    off = struct.unpack_from('<I', bmp, 10)[0]
    w, h = struct.unpack_from('<ii', bmp, 18)
    ncol = struct.unpack_from('<I', bmp, 46)[0] or 256
    ct = [(bmp[54 + 4 * i + 2], bmp[54 + 4 * i + 1], bmp[54 + 4 * i]) for i in range(ncol)]
    pal = ct + [(0, 0, 0)] * (256 - ncol)
    stride = (w + 3) & ~3
    src = [[bmp[off + (h - 1 - y) * stride + x] for x in range(w)] for y in range(h)]   # bottom-up

    sw, sh, px = read_bmp24(scr)
    check((sw, sh) == (640, 480), 'screen %dx%d' % (sw, sh))
    black, white = nearest(pal, (0, 0, 0)), nearest(pal, (255, 255, 255))
    exp = [[pal[black]] * sw for _ in range(sh)]
    for y in range(h):
        for x in range(w):
            s = src[y][x]
            exp[8 + y][8 + x] = pal[s]
            exp[160 + y][8 + x] = pal[white & s]
            exp[312 + y][8 + x] = pal[black | s]
            if 600 + x < sw and 460 + y < sh:
                exp[460 + y][600 + x] = pal[s]
    for y in range(64):
        for x in range(64):
            exp[100 + y][430 + x] = pal[((y // 8) * 8 + x // 8) * 4]
    tx, ty, tw, th = map(int, report['text_rect'].split(','))
    text_rgb = pal[nearest(pal, (255, 255, 0))]
    bk_rgb = pal[white]
    allowed = {pal[0], bk_rgb, text_rgb}
    ntext = nbk = 0
    bad = 0
    for y in range(sh):
        for x in range(sw):
            if tx <= x < tx + tw and ty <= y < ty + th:
                c = px[y][x]
                if c not in allowed:
                    bad += 1
                ntext += c == text_rgb
                nbk += c == bk_rgb
            elif px[y][x] != exp[y][x]:
                if bad < 5:
                    print('  mismatch at (%d,%d): got %s want %s' % (x, y, px[y][x], exp[y][x]))
                bad += 1
    check(bad == 0, '%d screen pixels differ' % bad)
    check(ntext >= 50, 'only %d text-coloured pixels' % ntext)
    check(nbk > 0, 'no OPAQUE background pixels')
    print('screen: %dx%d compared, %d mismatches; text box %d text px, %d bk px' % (sw, sh, bad, ntext, nbk))

    wav = pack_get(pk, report['wav'])
    i = wav.find(b'data')
    n = struct.unpack_from('<I', wav, i + 4)[0]
    data = wav[i + 8:i + 8 + n]
    check(report['wav_bits'] == '8' and report['wav_channels'] == '1', 'test expects 8-bit mono')
    check(report['device_rate'] == report['wav_rate'], 'exact audio check needs device rate == wav rate')
    exp_s = []
    for b in data:
        v = (b - 128) << 8
        exp_s += [v, v]
    got = s16(raw)
    le = next(k for k, v in enumerate(exp_s) if v)
    lg = next((k for k, v in enumerate(got) if v), None)
    check(lg is not None, 'audio dump is silent')
    if lg is not None:
        seg = list(got[lg:lg + len(exp_s) - le])
        same = seg == exp_s[le:]
        check(same, 'audio dump differs from the WAV samples')
        print('audio: %d samples, %s; WOM_DONE after %s ms (sound %s ms); callbacks open/done/close %s/%s/%s'
              % (len(exp_s) - le, 'exact match' if same else 'MISMATCH', report['wom_done_ms'], report['wav_ms'],
                 report['wom_open'], report['wom_done'], report['wom_close']))
    if drv:
        d = s16(drv)
        nz = [k for k, v in enumerate(d) if abs(v) > 256]
        span = (nz[-1] - nz[0]) / 2 if nz else 0
        wav_frames = len(data)
        ratio = span / wav_frames if wav_frames else 0
        # one play of the sound (plus possibly part of the reset one); rate ratio 1 or 4
        ok = bool(nz) and any(r * 0.9 <= ratio <= r * 1.6 for r in (1, 4))
        check(ok, 'driver dump: non-silent span %d frames for a %d-frame sound' % (span, wav_frames))
        print('driver dump: non-silent span %d frames, sound %d frames (ratio %.2f)' % (span, wav_frames, ratio))
    check(report.get('wom_open') == '2' and report.get('wom_done') == '2' and report.get('wom_close') == '2',
          'callback counts')

    for f in fails:
        print('FAIL:', f)
    print('check_shim: %s' % ('OK' if not fails else '%d failure(s)' % len(fails)))
    sys.exit(1 if fails else 0)


if __name__ == '__main__':
    main()
