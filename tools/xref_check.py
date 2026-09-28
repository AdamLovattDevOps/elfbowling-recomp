"""Verify fixup targets of matched functions, which match.py masks.

For each matched function, every fixup that refers to an external `fn_XXXXXX`
(a call or an absolute pointer) must resolve to the same address the original
refers to at that position. This catches a call to the wrong function, which
the byte comparison alone cannot see.

Usage: xref_check.py [FILE.cpp ...]
"""
import glob, os, re, struct, sys
sys.path.insert(0, os.path.dirname(__file__))
import omf, match
from pe import PE

ROOT = match.ROOT
FN = re.compile(r'^[@_]*fn_([0-9a-f]{6})')


def main():
    srcs = [os.path.abspath(a) for a in sys.argv[1:]] or sorted(glob.glob(os.path.join(ROOT, 'src_match/*.cpp')))
    pe = PE(os.path.join(ROOT, 'orig/1999/Elf Bowling.exe'))
    bad = checked = 0
    for src in srcs:
        obj = os.path.join(ROOT, 'build/obj', os.path.splitext(os.path.basename(src))[0] + '.obj')
        if not os.path.exists(obj):
            obj = match.compile_(src, match.DEFAULT_FLAGS)
        text = open(src, encoding='latin1').read()
        directives = {m.group(2): int(m.group(1), 16) for m in re.finditer(r'//\s*MATCH\s+([0-9a-fA-F]{6})\s+(\S+)', text)}
        for m in omf.load(obj):
            pubs = []
            for name, si, off in m.publics:
                if si is not None and si < len(m.segments) and m.segments[si].cls == 'CODE':
                    pubs.append((si, off, name))
            units = []
            for si, off, name in pubs:
                seg = m.segments[si]
                later = sorted(o for s2, o, n in pubs if s2 == si and o > off)
                units.append((name, seg, off, later[0] if later else len(seg.data)))
            for v in m.virtuals:
                units.append((v.name, v, 0, len(v.data)))
            for name, seg, lo, hi in units:
                fm = FN.match(name)
                addr = int(fm.group(1), 16) if fm else directives.get(name)
                if addr is None:
                    continue
                for at, loc, rel, tgt, disp in seg.fixups:
                    if not (lo <= at < hi) or not tgt or tgt[0] != 2:
                        continue
                    x = m.xnames[tgt[1] - 1] if tgt[1] - 1 < len(m.xnames) else None
                    if not isinstance(x, str):
                        continue
                    tm = FN.match(x)
                    if not tm:
                        continue
                    want = int(tm.group(1), 16)
                    site = addr + (at - lo)
                    raw = struct.unpack('<I', pe.read(site, 4))[0]
                    got = (site + 4 + raw) & 0xffffffff if rel else raw
                    checked += 1
                    if got != want:
                        bad += 1
                        print('BAD  %s @%06x+%#x refers to %s but original targets %08x' % (
                            os.path.basename(src), addr, at - lo, x, got))
    print('fn_ references checked: %d, wrong: %d' % (checked, bad))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
