"""Find function starts in the game-code range of .text.

Seeds: entry, exports, published methods, E8 call targets from all of .text,
and relocated pointers into the range from anywhere in the image (vtables and
callbacks). Starts are then refined by recursive descent from each seed. Output:
build/funcs.tsv with columns start, end, size, name, kind, where kind is game
or lib (lib means the start is claimed by build/libmap.tsv).
"""
import os, sys, struct, bisect
sys.path.insert(0, os.path.dirname(__file__))
import capstone
from pe import PE
from vmts import vmts

LO, HI = 0x401000, 0x421b5c


def load_libmap():
    out = []
    for line in open('build/libmap.tsv'):
        s, e, lib, mod, sym = line.rstrip('\n').split('\t')
        out.append((int(s, 16), int(e, 16), lib, mod, sym))
    return out


def main():
    p = PE()
    name, va, vs, ro, rs = p.section('.text')
    tbase = p.image_base + va
    text = p.data[ro:ro + vs]
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    libmap = load_libmap()
    names = {}
    seeds = set()
    for s, e, lib, mod, sym in libmap:
        if LO <= s < HI:
            seeds.add(s)
            names[s] = sym
    for n, a in p.exports().items():
        if LO <= a < HI:
            seeds.add(a); names[a] = n
    for vmt, cls, size, par, methods in vmts(p):
        for a, n in methods:
            if LO <= a < HI:
                seeds.add(a); names[a] = '%s::%s' % (cls, n)
    # call targets from linear sweep of .text
    for i in range(len(text) - 5):
        if text[i] == 0xE8:
            t = tbase + i + 5 + struct.unpack_from('<i', text, i + 1)[0]
            if LO <= t < HI:
                seeds.add(t)
    relocs = p.relocs()
    for r in relocs:
        try:
            t = p.u32(r)
        except ValueError:
            continue
        if LO <= t < HI and not (LO <= r < HI):
            seeds.add(t)
    # verify seeds by decoding a few instructions; drop ones that decode badly
    good = set()
    for s in seeds:
        off = s - tbase
        ok = True
        for n, ins in enumerate(md.disasm(text[off:off + 32], s)):
            if n >= 3:
                break
        else:
            ok = n >= 1
        if ok:
            good.add(s)
    # E8 targets found by linear sweep can be false positives inside data; keep
    # only those reached by recursive descent from strong seeds.
    strong = {a for a in good if a in names} | {LO}
    for r in relocs:
        try:
            t = p.u32(r)
        except ValueError:
            continue
        if LO <= t < HI and not (LO <= r < HI):
            strong.add(t)
    # Borland pads functions to 4 bytes with NOPs; a push ebp/mov ebp,esp
    # prologue after a ret or padding is a function start.
    for i in range(LO - tbase, HI - tbase - 3):
        if text[i:i + 3] == b'\x55\x8b\xec' and (text[i - 1] in (0xc3, 0x90, 0x00) or i % 4 == 0):
            strong.add(tbase + i)
    # E8 targets that are called from code we already trust are added during descent.
    funcs, work = set(), list(strong)
    while work:
        f = work.pop()
        if f in funcs or not (LO <= f < HI):
            continue
        funcs.add(f)
        pc_todo, seen = [f], set()
        while pc_todo:
            pc = pc_todo.pop()
            while LO <= pc < HI and pc not in seen:
                seen.add(pc)
                off = pc - tbase
                ins = next(md.disasm(text[off:off + 16], pc), None)
                if ins is None:
                    break
                pc += ins.size
                m = ins.mnemonic
                if m == 'call' and ins.op_str.startswith('0x'):
                    work.append(int(ins.op_str, 16))
                elif m.startswith('j') and ins.op_str.startswith('0x'):
                    tgt = int(ins.op_str, 16)
                    if m == 'jmp':
                        pc_todo.append(tgt)
                        break
                    pc_todo.append(tgt)
                elif m in ('ret', 'retf', 'jmp', 'hlt'):
                    break
    # Manual corrections: notes/func_ends.tsv (start, true end) and
    # notes/data_in_code*.txt (addresses that are data, not functions).
    import glob
    ends_fix, data = {}, set()
    if os.path.exists('notes/func_ends.tsv'):
        for line in open('notes/func_ends.tsv'):
            if line.strip() and not line.startswith('#'):
                a, b = line.split('\t')[:2]
                ends_fix[int(a, 16)] = int(b, 16)
    gen = set()   # compiler-generated code: produced automatically once units compile whole
    for f in glob.glob('notes/data_in_code*.txt'):
        target = data
        for line in open(f):
            if line.startswith('# Not data'):
                target = gen
            tok = line.split()
            if tok and not tok[0].startswith('#'):
                try:
                    target.add(int(tok[0], 16))
                except ValueError:
                    pass
    for a, b in ends_fix.items():
        funcs.add(a)
        funcs.add(b)
        data.add(b)
        # a corrected range owns everything inside it
        funcs = {f for f in funcs if not (a < f < b)}
        data = {d for d in data if not (a < d < b)}
    # Anything that starts inside a library-claimed range is library code.
    libranges = sorted((s, e) for s, e, *_ in libmap)
    def in_lib(a):
        i = bisect.bisect_right(libranges, (a, 1 << 32)) - 1
        return i >= 0 and libranges[i][0] < a < libranges[i][1]
    funcs = {f for f in funcs if not in_lib(f)}
    names = {a: n for a, n in names.items() if not in_lib(a)}
    starts = sorted(funcs | {s for s, e, *_ in libmap if LO <= s < HI})
    starts = [s for s in starts if not any(a < s < b for a, b in ends_fix.items())]
    libstarts = {s: (e, lib, mod, sym) for s, e, lib, mod, sym in libmap}
    with open('build/funcs.tsv', 'w') as f:
        for i, s in enumerate(starts):
            e = starts[i + 1] if i + 1 < len(starts) else HI
            kind = 'lib' if s in libstarts else ('data' if s in data else 'gen' if s in gen else 'game')
            if kind == 'lib':
                e = libstarts[s][0]
            f.write('%08x\t%08x\t%d\t%s\t%s\n' % (s, e, e - s, names.get(s, 'fn_%06x' % (s & 0xffffff)), kind))
    g = [(s, (starts[i + 1] if i + 1 < len(starts) else HI) - s) for i, s in enumerate(starts) if s not in libstarts and s not in data and s not in gen]
    print('functions: %d game, %d lib-claimed in range; game bytes %d' % (len(g), len(starts) - len(g), sum(n for _, n in g)))


if __name__ == '__main__':
    main()
