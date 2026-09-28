"""Walk the PE resource tree. Usage: rsrc.py [--extract DIR]"""
import os, struct, sys
sys.path.insert(0, os.path.dirname(__file__))
from pe import PE

TYPES = {1: 'CURSOR', 2: 'BITMAP', 3: 'ICON', 4: 'MENU', 5: 'DIALOG', 6: 'STRING', 10: 'RCDATA',
         12: 'GROUP_CURSOR', 14: 'GROUP_ICON', 16: 'VERSION'}


def walk(p):
    rva, _ = p.dirs[2]
    base = p.rva2off(rva)
    d = p.data

    def name_of(e):
        if e & 0x80000000:
            o = base + (e & 0x7fffffff)
            n = struct.unpack_from('<H', d, o)[0]
            return d[o + 2:o + 2 + 2 * n].decode('utf-16le')
        return e

    def dir_(off, path):
        nnamed, nid = struct.unpack_from('<HH', d, base + off + 12)
        for i in range(nnamed + nid):
            nm, tgt = struct.unpack_from('<II', d, base + off + 16 + 8 * i)
            key = name_of(nm)
            if tgt & 0x80000000:
                yield from dir_(tgt & 0x7fffffff, path + [key])
            else:
                drva, size = struct.unpack_from('<II', d, base + tgt)
                yield path + [key], p.data[p.rva2off(drva):p.rva2off(drva) + size]
    yield from dir_(0, [])


def main():
    p = PE()
    out = sys.argv[2] if len(sys.argv) > 2 and sys.argv[1] == '--extract' else None
    for path, data in walk(p):
        t = TYPES.get(path[0], path[0])
        print('%-10s %-30s %8d %s' % (t, path[1], len(data), data[:8].hex()))
        if out:
            dd = os.path.join(out, str(t))
            os.makedirs(dd, exist_ok=True)
            fn = str(path[1])
            if t == 'BITMAP':
                # prepend BITMAPFILEHEADER
                hs, = struct.unpack_from('<I', data, 0)
                bpp, = struct.unpack_from('<H', data, 14)
                clr, = struct.unpack_from('<I', data, 32)
                ncol = clr or (1 << bpp if bpp <= 8 else 0)
                off = 14 + hs + 4 * ncol
                data = b'BM' + struct.pack('<IHHI', 14 + len(data), 0, 0, off) + data
                fn += '.bmp'
            open(os.path.join(dd, fn), 'wb').write(data)


if __name__ == '__main__':
    main()
