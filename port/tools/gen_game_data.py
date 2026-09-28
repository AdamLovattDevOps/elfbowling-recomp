#!/usr/bin/env python3
"""Generate game_data.cpp: a native definition of every g_XXXXXX global that
include/elf/game.h declares, plus a table that fills in their initial values
from the original exe's .data AT RUN TIME (elf_game_data_load, called by
port/vcl/main.cpp once port_res_open has read the user's exe).

The generated file holds no bytes of the exe: only types, file offsets, sizes
and the pointer structure (which slot points at which global, function, or
string at which file offset). It is generated once from the exe and kept in
the source tree (port/native/game_data.cpp), so a build needs no exe; the
table only fits the one exe it was made from (EXE_SIZE, and the SHA-256 the
packages check, port/pkg/firstrun.c).

In the original, the game's globals sit at fixed addresses in .data
(0x455000..0x460200 initialised, the rest of the section zero-filled like
.bss). The native build cannot reproduce those addresses (pointers are 8
bytes), so each global becomes its own C++ definition:

  * the type and name come from game.h (every `extern` in its extern "C" block);
  * the address comes from the name (g_45aaf4 -> 0x45aaf4);
  * an array declared with an unknown first bound (`[]`) is sized from the gap
    to the next known symbol (game.h plus every g_XXXXXX used in src_match/),
    or for pointer tables up to the first dword that is neither relocated nor 0;
  * the initial value is read from the exe:
      - integers / bool / char (and arrays of them): a (file offset, count,
        32-bit size) record, widened to the native type at load;
      - pointers: the relocation table says which dwords are addresses; an
        address inside a known global becomes `&g_x` / `(T)((char *)g_x + off)`,
        any other .data address is a NUL-terminated string: the slot is set
        at load to point into the loaded exe image (a file-offset record),
        a .text address a function `fn_XXXXXX` (declared through funcs.h);
      - structs without pointers (FrameScore, PinState): copied from the image
        at load, after a sizeof check against the 32-bit size.
  * globals in the zero-filled part are zero-initialised (no initialiser).

Excluded: g_45fee0 / g_45fee4 / g_45fee8 (the VCL indirection cells, defined
in port/vcl/entry.cpp), and anything listed with --exclude.

Usage:
  gen_game_data.py --exe "orig/1999/Elf Bowling.exe" --header include/elf/game.h \\
                   --src src_match --out build/native/game_data.cpp
"""
import argparse
import glob
import re
import struct
import sys

EXCLUDE = {'g_45fee0', 'g_45fee4', 'g_45fee8'}

# 32-bit sizes of the element types game.h uses.
SCALARS = {
    'int': (4, 'i'), 'unsigned': (4, 'I'), 'unsigned int': (4, 'I'), 'long': (4, 'i'),
    'unsigned long': (4, 'I'), 'short': (2, 'h'), 'unsigned short': (2, 'H'),
    'char': (1, 'b'), 'signed char': (1, 'b'), 'unsigned char': (1, 'B'), 'bool': (1, 'B'),
    'uInt': (4, 'I'),   # zlib_41e2cc's typedef
}
HANDLES = {'HGDIOBJ', 'HDC', 'HPALETTE', 'HFONT', 'HBITMAP', 'HINSTANCE', 'HWND', 'HANDLE'}
STRUCTS = {'FrameScore': 16, 'PinState': 8}      # pointer-free structs, 32-bit size


class PE:
    def __init__(self, path):
        self.d = d = open(path, 'rb').read()
        pe = struct.unpack_from('<I', d, 0x3c)[0]
        if d[pe:pe + 4] != b'PE\0\0' or struct.unpack_from('<H', d, pe + 24)[0] != 0x10b:
            sys.exit(f'{path}: not a PE32 image')
        opt = pe + 24
        self.base = struct.unpack_from('<I', d, opt + 28)[0]
        nsec = struct.unpack_from('<H', d, pe + 6)[0]
        off = opt + struct.unpack_from('<H', d, pe + 20)[0]
        self.secs = []
        for i in range(nsec):
            name, vs, va, rs, ro = struct.unpack_from('<8sIIII', d, off + 40 * i)
            self.secs.append((name.rstrip(b'\0').decode(), self.base + va, vs, ro, rs))
        rva, sz = struct.unpack_from('<II', d, opt + 96 + 8 * 5)
        self.rel = set()
        o = self.off(self.base + rva)
        end = o + sz
        while o < end:
            page, bs = struct.unpack_from('<II', d, o)
            if bs < 8:
                break
            for i in range((bs - 8) // 2):
                e = struct.unpack_from('<H', d, o + 8 + 2 * i)[0]
                if e >> 12 == 3:
                    self.rel.add(self.base + page + (e & 0xfff))
            o += bs

    def sec(self, name):
        return next(s for s in self.secs if s[0] == name)

    def section_of(self, va):
        for s in self.secs:
            if s[1] <= va < s[1] + max(s[2], s[4]):
                return s[0]
        return None

    def off(self, va):
        for name, sva, vs, ro, rs in self.secs:
            if sva <= va < sva + max(vs, rs):
                return ro + va - sva if va - sva < rs else None
        raise ValueError(hex(va))

    def read(self, va, n):
        """n bytes at va; the zero-filled tail of a section reads as zeros."""
        out = bytearray()
        for a in range(va, va + n):
            o = self.off(a)
            out.append(self.d[o] if o is not None else 0)
        return bytes(out)

    def u32(self, va):
        return struct.unpack('<I', self.read(va, 4))[0]

    def cstr(self, va):
        o = self.off(va)
        return self.d[o:self.d.index(b'\0', o)]


def strip_comments(s):
    s = re.sub(r'/\*.*?\*/', '', s, flags=re.S)
    return re.sub(r'//[^\n]*', '', s)


GNAME = r'g_(?:[A-Za-z]\w*?_)?[0-9a-f]{6}'


def gaddr(name):
    m = re.search(r'([0-9a-f]{6})$', name)
    return int(m.group(1), 16) if m else None


def parse_header(path, src_mode=False):
    """[(name, type, dims, is_ptr, fnptr)] for every extern g_ in extern "C" blocks.
    src_mode: a .cpp unit; only `extern "C" <decl>;` lines are read."""
    text = strip_comments(open(path, errors='replace').read())
    out = []
    pat = r'\bextern\s+"C"\s+(?!\{)([^;{}]*?);' if src_mode else r'\bextern\s+(?!"C")([^;{}]*?);'
    for m in re.finditer(pat, text):
        decl = ' '.join(m.group(1).split())
        fp = re.match(r'(.*?)\(\s*\*\s*(' + GNAME + r')\s*\)\s*(\(.*\))$', decl)
        if fp:
            out.append(dict(name=fp.group(2), base=fp.group(1).strip(), dims=[], ptr=True,
                            fnptr=fp.group(3), const=False))
            continue
        # "<type> <d1>, <d2>" where each declarator is [*]g_xxx[dims]
        first = re.search(r'\**\s*\bg_\w+', decl)
        if not first or '(' in decl:
            continue
        base = decl[:first.start()].strip()
        for d in decl[first.start():].split(','):
            dm = re.match(r'\s*(\**)\s*(g_\w+?)\s*((?:\[[^\]]*\])*)\s*$', d)
            if not dm:
                if src_mode:
                    continue
                sys.exit(f'cannot parse declarator {d!r} in {decl!r}')
            dims = [x.strip() for x in re.findall(r'\[([^\]]*)\]', dm.group(3))]
            try:
                dims = [int(x, 0) if x else None for x in dims]
            except ValueError:
                dims = ['?']
            out.append(dict(name=dm.group(2), base=base, stars=dm.group(1), dims=dims,
                            ptr=bool(dm.group(1)) or base in HANDLES, fnptr=None,
                            const=base.startswith('const ') and not dm.group(1)))
    return out


def c_string(b):
    s = '"'
    hexlast = False
    for c in b:
        ch = chr(c)
        if 0x20 <= c < 0x7f:
            if hexlast and ch in '0123456789abcdefABCDEF':
                s += '" "'      # a hex escape would swallow the next hex digit
            s += '\\' + ch if ch in '"\\' else ch
            if ch == '?':
                s += '" "'      # no trigraphs
            hexlast = False
        elif c in (9, 10, 13):
            s += {9: '\\t', 10: '\\n', 13: '\\r'}[c]
            hexlast = False
        else:
            s += '\\x%02x' % c
            hexlast = True
    return s + '"'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe', required=True)
    ap.add_argument('--header', required=True)
    ap.add_argument('--src', default=None, help='src_match dir: more symbol addresses for sizing')
    ap.add_argument('--out', required=True)
    ap.add_argument('--exclude', action='append', default=[])
    a = ap.parse_args()

    pe = PE(a.exe)
    data = pe.sec('.data')
    dstart, dvs, draw = data[1], data[2], data[4]
    dinit_end = dstart + draw

    decls = [d for d in parse_header(a.header) if d['name'] not in EXCLUDE | set(a.exclude)]
    seen = set()
    uniq = []
    for d in decls:
        if d['name'] not in seen:
            seen.add(d['name'])
            uniq.append(d)
    decls = uniq
    unresolved = []
    if a.src:
        for f in sorted(glob.glob(a.src + '/*.cpp')):
            for d in parse_header(f, src_mode=True):
                if d['name'] in seen or d['name'] in EXCLUDE | set(a.exclude):
                    continue
                seen.add(d['name'])
                base = d['base'].replace('const ', '').strip()
                ok = (gaddr(d['name']) is not None and '?' not in d['dims'] and
                      (d['ptr'] or base in SCALARS or base in STRUCTS))
                if ok:
                    d['unit'] = f.split('/')[-1]
                    decls.append(d)
                else:
                    unresolved.append((d['name'], d['base'], f.split('/')[-1]))

    # every known symbol address, for sizing [] arrays and naming pointer targets
    addrs = {gaddr(d['name']) for d in decls} | {0x45fedc, 0x45fee0, 0x45fee4, 0x45fee8}
    if a.src:
        for f in glob.glob(a.src + '/*.cpp'):
            addrs |= {int(x, 16) for x in re.findall(r'\bg_(?:[A-Za-z]\w*?_)?([0-9a-f]{6})\b', open(f, errors='replace').read())}
    addrs.add(dstart + dvs)
    sorted_addrs = sorted(addrs)

    def next_addr(va):
        for x in sorted_addrs:
            if x > va:
                return x
        return dstart + dvs

    def elem_size(d):
        if d['ptr']:
            return 4
        if d['base'] in STRUCTS:
            return STRUCTS[d['base']]
        b = d['base'].replace('const ', '').strip()
        if b not in SCALARS:
            sys.exit(f"{d['name']}: unknown element type {d['base']!r}")
        return SCALARS[b][0]

    by_addr = {}
    for d in decls:
        va = gaddr(d['name'])
        d['va'] = va
        es = elem_size(d)
        inner = 1
        for x in d['dims'][1:]:
            inner *= x
        if d['dims'] and d['dims'][0] is None:
            gap = next_addr(va) - va
            n = gap // (es * inner)
            if d['ptr']:
                # a pointer table ends at the first dword that is not an address or 0
                k = 0
                while k < n and ((va + 4 * k) in pe.rel or pe.u32(va + 4 * k) == 0):
                    k += 1
                n = k
            d['dims'][0] = max(n, 1)
            d['sized'] = True
        count = 1
        for x in d['dims']:
            count *= x
        d['count'] = count
        d['size32'] = count * es
        by_addr[va] = d

    ranges = sorted((d['va'], d['va'] + d['size32'], d) for d in decls)

    def sym_at(target):
        for lo, hi, d in ranges:
            if lo <= target < hi:
                return d, target - lo
        return None, 0

    notes = []
    body = []
    recs = []      # (dst expr, file offset, count, size32, native elem size expr, kind)

    def ptr_expr(d, slot_va):
        v = pe.u32(slot_va)
        ctype = d['base'] + ' ' + d.get('stars', '')
        if d['fnptr']:
            ctype = None
        if v == 0:
            return 'nullptr' if d['base'] not in HANDLES else '0'
        if slot_va not in pe.rel:
            notes.append(f"{d['name']}+{slot_va - d['va']:#x}: non-relocated pointer value {v:#x}, emitted as 0")
            return 'nullptr'
        sec = pe.section_of(v)
        if sec == '.text':
            if not d['fnptr']:
                notes.append(f"{d['name']}+{slot_va - d['va']:#x}: code pointer {v:#x} in a data pointer, emitted as 0")
                return 'nullptr'
            return f'({d["name"]}_elem_t)fn_{v:06x}'
        t, off = sym_at(v)
        if t is not None:
            e = f'(char *){t["name"]}' if t['dims'] else f'(char *)&{t["name"]}'
            return f'({ctype.strip()})({e} + {off:#x})' if off else f'({ctype.strip()})({e})'
        if sec == '.data':
            idx = (slot_va - d['va']) // 4
            slot = f'&{d["name"]}[{idx}]' if d['dims'] else f'&{d["name"]}'
            recs.append((slot, pe.off(v), 1, 0, 'sizeof(void *)', 'S'))
            return 'nullptr' if d['base'] not in HANDLES else '0'

        notes.append(f"{d['name']}+{slot_va - d['va']:#x}: pointer {v:#x} into {sec}, emitted as 0")
        return 'nullptr'

    for d in decls:
        va, name = d['va'], d['name']
        dims = ''.join(f'[{x}]' for x in d['dims'])
        in_data = dstart <= va < dstart + dvs
        initialised = in_data and va < dinit_end
        raw = pe.read(va, d['size32']) if in_data else bytes(d['size32'])
        nonzero = any(raw)
        stars = d.get('stars', '')
        ctype = {'uInt': 'unsigned'}.get(d['base'], d['base'])     # unit-local typedefs
        if d['fnptr']:
            decl = f"{ctype} (*{name}){d['fnptr']}"
        else:
            decl = f"{ctype} {stars}{name}{dims}"
        comment = f"// {va:#x}, {d['size32']} bytes (32-bit)"
        if d.get('unit'):
            comment += f", declared in {d['unit']}"
        if d.get('sized'):
            comment += f", first bound {d['dims'][0]} from the gap"
        if not in_data:
            comment += ', outside .data?'
            notes.append(f'{name}: {va:#x} is outside .data')
        if not nonzero:
            # zero (or .bss): value-initialised; const needs an explicit {}
            body.append(f"{decl}{' = {}' if d['const'] else ''};   {comment}")
            continue
        if d['const'] and not d['ptr']:
            # filled in at load, so it cannot live in read-only memory: a writable
            # definition under the same (extern "C") symbol name
            name = name + '_rw'
            decl = f"{ctype.replace('const ', '', 1)} {stars}{name}{dims} ELF_ASM({d['name']})"
        foff = pe.off(va)
        if d['ptr']:
            vals = [ptr_expr(d, va + 4 * i) for i in range(d['count'])]
            if d['fnptr']:
                # function pointer: the cast goes through a helper typedef
                body.append(f"typedef {d['base']} (*{name}_elem_t){d['fnptr']};")
            init = vals[0] if not d['dims'] else '{\n    ' + ',\n    '.join(vals) + ',\n}'
            body.append(f'{decl} = {init};   {comment}')
        elif d['base'] in STRUCTS:
            body.append(f'{decl};   {comment}')
            body.append(f'static_assert(sizeof({name}) == {d["size32"]}, "{name}: native layout differs from 32-bit");')
            recs.append((f'&{name}', foff, 1, d['size32'], f'sizeof({name})', 'R'))
        else:
            b = d['base'].replace('const ', '').strip()
            sz, fmt = SCALARS[b]
            if b == 'bool':
                vals = struct.unpack('<%dB' % d['count'], raw)
                if any(x > 1 for x in vals):
                    notes.append(f'{name}: bool with byte values > 1, loaded as true')
            kind = 'B' if b == 'bool' else ('I' if fmt.isupper() else 'i')
            body.append(f'{decl};   {comment}')
            recs.append((f'&{name}', foff, d['count'], sz, f'sizeof({name}) / {d["count"]}', kind))

    out = []
    out.append('// GENERATED by port/tools/gen_game_data.py from include/elf/game.h and the')
    out.append("// layout of the original exe's .data section. Do not edit; `make -C port game-data`.")
    out.append('// Every g_XXXXXX the game declares. Their initial values are NOT here: elf_game_data_load')
    out.append("// copies them from the user's exe at startup (file offsets only; no bytes of the exe).")
    out.append('#include <vcl/vcl.h>')
    out.append('#include <elf/funcs.h>')
    out.append('#include <string.h>')
    out.append('')
    out.append('#define ELF_STR2(x) #x')
    out.append('#define ELF_STR(x) ELF_STR2(x)')
    out.append('#define ELF_ASM(n) __asm__(ELF_STR(__USER_LABEL_PREFIX__) #n)')
    out.append('')
    out.append('extern "C" {')
    out.append('')
    out.extend(body)
    out.append('')
    out.append('}   // extern "C"')
    out.append('')
    out.append('// kind: i/I signed/unsigned integers, B bool, R raw bytes, S pointer to the string at off')
    out.append('struct ElfDataRec { void *dst; unsigned off, count, size32, nsize; char kind; };')
    out.append('static const ElfDataRec k_elf_data[] = {')
    for dst, off, cnt, sz, nsz, kind in recs:
        out.append(f"    {{(void *){dst}, {off:#x}, {cnt}, {sz}, (unsigned)({nsz}), '{kind}'}},")
    out.append('};')
    out.append('')
    out.append(f'#define ELF_EXE_SIZE {len(pe.d)}u   // the exe this table was made from')
    out.append('extern "C" int elf_game_data_load(const unsigned char *img, size_t size)')
    out.append('{')
    out.append('    if (!img || size != ELF_EXE_SIZE)')
    out.append('        return -1;')
    out.append('    for (const ElfDataRec &r : k_elf_data) {')
    out.append("        if (r.kind == 'S') {")
    out.append('            const void *p = img + r.off;')
    out.append('            memcpy(r.dst, &p, sizeof p);')
    out.append('            continue;')
    out.append('        }')
    out.append("        if (r.kind == 'R') {")
    out.append('            memcpy(r.dst, img + r.off, r.size32);')
    out.append('            continue;')
    out.append('        }')
    out.append('        for (unsigned i = 0; i < r.count; i++) {')
    out.append('            const unsigned char *s = img + r.off + i * r.size32;')
    out.append('            unsigned long long v = 0;')
    out.append('            for (unsigned k = 0; k < r.size32; k++)')
    out.append('                v |= (unsigned long long)s[k] << (8 * k);')
    out.append("            if (r.kind == 'i' && r.size32 < 8 && (v >> (8 * r.size32 - 1)) & 1)")
    out.append('                v |= ~0ull << (8 * r.size32);    // sign-extend')
    out.append("            if (r.kind == 'B')")
    out.append('                v = v != 0;')
    out.append('            unsigned char *d = (unsigned char *)r.dst + i * r.nsize;')
    out.append('            for (unsigned k = 0; k < r.nsize; k++)   // little-endian targets only')
    out.append('                d[k] = (unsigned char)(v >> (8 * k));')
    out.append('        }')
    out.append('    }')
    out.append('    return 0;')
    out.append('}')
    for n, t, u in unresolved:
        notes.append(f'{n} ({t}, {u}): no address or not a plain type; the unit must define it natively')
    if notes:
        out.append('')
        out.append('// notes:')
        out.extend('//   ' + n for n in notes)
    open(a.out, 'w').write('\n'.join(out) + '\n')
    for n in notes:
        print('gen_game_data:', n, file=sys.stderr)
    print(f'gen_game_data: {len(decls)} globals, {len(recs)} load records -> {a.out}', file=sys.stderr)


if __name__ == '__main__':
    main()
