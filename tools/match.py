"""Build src_match/*.cpp with BCB3 bcc32 under Wine and compare each
extern "C" function named fn_XXXXXX against the original at 0x00XXXXXX.

Fixup bytes (from the object's FIXUPP records) are masked. A function matches
when every other byte is identical and the lengths agree.

Usage: match.py [FILE.cpp ...] [--flags "..."] [-v] [--no-record]
(Results are recorded only for files under src_match/.)
"""
import glob, json, os, re, subprocess, sys
sys.path.insert(0, os.path.dirname(__file__))
import omf
from pe import PE

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WINE = os.path.join(ROOT, 'toolchain/Wine Devel.app/Contents/Resources/wine/bin/wine')
BCC = os.path.join(ROOT, 'toolchain/bcb3/BIN/BCC32.EXE')
DEFAULT_FLAGS = open(os.path.join(ROOT, 'src_match/FLAGS')).read().split() if os.path.exists(os.path.join(ROOT, 'src_match/FLAGS')) else []


def funcs_tsv():
    out = {}
    p = os.path.join(ROOT, 'build/funcs.tsv')
    if os.path.exists(p):
        for line in open(p):
            s, e, n, name, kind = line.rstrip('\n').split('\t')
            out[int(s, 16)] = int(e, 16)
    return out


def compile_(src, flags):
    obj = os.path.join(ROOT, 'build/obj', os.path.splitext(os.path.basename(src))[0] + '.obj')
    env = dict(os.environ, WINEPREFIX=os.path.join(ROOT, 'toolchain/prefix'), WINEDEBUG='-all')
    inc = lambda p: '-I' + 'Z:' + os.path.join(ROOT, p).replace('/', '\\')
    cmd = [WINE, BCC, '-c', inc('toolchain/bcb3/INCLUDE'), inc('toolchain/bcb3/INCLUDE/VCL'), inc('include'),
           '-n' + 'Z:' + os.path.join(ROOT, 'build/obj').replace('/', '\\')] + flags + ['Z:' + src.replace('/', '\\')]
    r = subprocess.run(cmd, capture_output=True, text=True, env=env, timeout=300)
    if not os.path.exists(obj) or 'Error' in r.stdout:
        print(r.stdout + r.stderr)
        raise SystemExit('compile failed: ' + src)
    return obj


def code_symbols(obj):
    """Yield (name, bytes, mask) for every public in a CODE segment or virtual segment."""
    for m in omf.load(obj):
        for name, si, off in m.publics:
            if si is None or si >= len(m.segments):
                continue
            seg = m.segments[si]
            if seg.cls != 'CODE':
                continue
            starts = sorted(o for n, s, o in m.publics if s == si and o > off)
            end = starts[0] if starts else len(seg.data)
            yield name, bytes(seg.data[off:end]), bytes(seg.mask[off:end])
        for v in m.virtuals:
            yield v.name, bytes(v.data), bytes(v.mask)


def compare(pe, addr, data, mask, fend):
    orig = pe.read(addr, len(data))
    diffs = [i for i, (a, b, m) in enumerate(zip(orig, data, mask)) if not m and a != b]
    olen = (fend - addr) if fend else None
    # Borland pads with NOPs up to 4-byte alignment; ignore trailing padding in original size.
    return diffs, olen


def main():
    args = sys.argv[1:]
    verbose = '-v' in args
    record = '--no-record' not in args
    args = [a for a in args if a not in ('-v', '--no-record')]
    flags = DEFAULT_FLAGS
    if '--flags' in args:
        i = args.index('--flags'); flags = args[i + 1].split(); del args[i:i + 2]
    srcs = [os.path.abspath(a) for a in args] or sorted(glob.glob(os.path.join(ROOT, 'src_match/*.cpp')))
    pe = PE(os.path.join(ROOT, 'orig/1999/Elf Bowling.exe'))
    fends = funcs_tsv()
    total = ok = 0
    os.makedirs(os.path.join(ROOT, 'build/match'), exist_ok=True)
    from concurrent.futures import ThreadPoolExecutor
    with ThreadPoolExecutor(max_workers=min(8, len(srcs) or 1)) as ex:
        objs = list(ex.map(lambda s: compile_(s, flags), srcs))
    for src, obj in zip(srcs, objs):
        results = {}
        # "// MATCH 40609c @Actor@$bctr$qv" maps a mangled C++ symbol to an address.
        directives = {m.group(2): int(m.group(1), 16) for m in
                      re.finditer(r'//\s*MATCH\s+([0-9a-fA-F]{6})\s+(\S+)', open(src, encoding='latin1').read())}
        for name, data, mask in code_symbols(obj):
            m = re.match(r'@?_?fn_([0-9a-f]{6})$', name)
            if m:
                addr = int(m.group(1), 16)
            elif name in directives:
                addr = directives[name]
            else:
                continue
            diffs, olen = compare(pe, addr, data, mask, fends.get(addr))
            pad_ok = olen is None or (len(data) <= olen and set(pe.read(addr + len(data), olen - len(data))) <= {0x90})
            good = not diffs and pad_ok
            total += len(data); ok += len(data) if good else 0
            results['%06x' % addr] = {'ok': good, 'size': olen or len(data), 'src': os.path.relpath(src, ROOT)}
            print('%-4s %s  %d bytes%s%s' % ('OK' if good else 'DIFF', name, len(data),
                  '' if pad_ok else '  (orig %d bytes)' % olen, '' if not diffs else '  first diff @+%#x (%d bytes differ)' % (diffs[0], len(diffs))))
            if verbose and diffs:
                o = pe.read(addr, len(data))
                for i in range(0, len(data), 16):
                    print('   %04x orig %s' % (i, o[i:i + 16].hex()))
                    print('   %04x ours %s' % (i, ''.join('..' if mask[j] else '%02x' % data[j] for j in range(i, min(i + 16, len(data))))))
        if record and os.path.relpath(src, ROOT).startswith('src_match' + os.sep):
            rpath = os.path.join(ROOT, 'build/match', os.path.basename(src) + '.json')
            json.dump(results, open(rpath, 'w'), indent=0, sort_keys=True)
    print('matched %d / %d bytes built' % (ok, total))


if __name__ == '__main__':
    main()
