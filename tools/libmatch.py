"""Identify BCB3 library code inside the game executable.

Each library module's code segments and virtual segments (roughly one per
function) are searched for in the game's .text section, with fixup bytes
masked out. Output: build/libmap.tsv with columns start, end, library,
module, symbol.
"""
import glob, os, sys, bisect
sys.path.insert(0, os.path.dirname(__file__))
import omf
from pe import PE

LIBS = sorted(glob.glob('toolchain/bcb3/LIB/RELEASE/*.LIB')) + [
    'toolchain/bcb3/LIB/CP32MT.LIB', 'toolchain/bcb3/LIB/CW32MT.LIB',
    'toolchain/bcb3/LIB/CP32MTI.LIB', 'toolchain/bcb3/LIB/NOEH32.LIB',
    'toolchain/bcb3/LIB/MEMMGR.LIB'] + sorted(glob.glob('toolchain/bcb3/LIB/*.OBJ'))
MIN = 8


def anchor(data, mask):
    best, bs, run = 0, 0, 0
    for i, m in enumerate(mask):
        if m:
            run = 0
            continue
        run += 1
        if run > best:
            best, bs = run, i - run + 1
    return bs, best


def matches(text, pos, data, mask):
    t = text[pos:pos + len(data)]
    if len(t) != len(data):
        return False
    for a, b, m in zip(t, data, mask):
        if not m and a != b:
            return False
    return True


def chunks():
    for lib in LIBS:
        try:
            mods = omf.load(lib)
        except Exception as e:
            print('skip', lib, e, file=sys.stderr)
            continue
        ln = os.path.basename(lib)
        for m in mods:
            code = {i + 1 for i, s in enumerate(m.segments) if s.cls == 'CODE'}
            for s in m.segments:
                if s.cls == 'CODE' and len(s.data) >= MIN:
                    yield ln, m.name, s.name, s
            for v in m.virtuals:
                if v.parent in code and len(v.data) >= MIN:
                    yield ln, m.name, v.name, v


def main():
    pe = PE()
    name, va, vs, ro, rs = pe.section('.text')
    text = pe.data[ro:ro + vs]
    base = pe.image_base + va
    claimed = []          # (start, end, lib, module, sym)
    seen = set()
    for lib, mod, sym, seg in sorted(chunks(), key=lambda c: -len(c[3].data)):
        data, mask = bytes(seg.data), bytes(seg.mask)
        key = (data, mask)
        a, n = anchor(data, mask)
        if n < 4:
            continue
        needle = data[a:a + n]
        p = text.find(needle)
        while p >= 0:
            st = p - a
            if st >= 0 and matches(text, st, data, mask):
                en = st + len(data)
                i = bisect.bisect_left(claimed, (st,))
                clash = (i < len(claimed) and claimed[i][0] < en) or (i > 0 and claimed[i - 1][1] > st)
                if not clash:
                    claimed.insert(i, (st, en, lib, mod, sym))
            p = text.find(needle, p + 1)
    os.makedirs('build', exist_ok=True)
    with open('build/libmap.tsv', 'w') as f:
        for st, en, lib, mod, sym in claimed:
            f.write('%08x\t%08x\t%s\t%s\t%s\n' % (st + base, en + base, lib, mod, sym))
    cov = sum(e - s for s, e, *_ in claimed)
    print('library chunks matched: %d, bytes %d / %d (%.1f%%)' % (len(claimed), cov, len(text), 100 * cov / len(text)))
    # uncovered ranges
    gaps, prev = [], 0
    for s, e, *_ in claimed:
        if s - prev >= 16:
            gaps.append((prev, s))
        prev = max(prev, e)
    if len(text) - prev >= 16:
        gaps.append((prev, len(text)))
    with open('build/gaps.tsv', 'w') as f:
        for s, e in gaps:
            f.write('%08x\t%08x\t%d\n' % (s + base, e + base, e - s))
    print('unclaimed ranges >=16B: %d, bytes %d' % (len(gaps), sum(e - s for s, e in gaps)))


if __name__ == '__main__':
    main()
