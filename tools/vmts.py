"""List Delphi/VCL class VMTs and their published methods (Delphi 3 VMT layout)."""
import struct, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from pe import PE

SELF, METHODS, NAME, SIZE, PARENT = -76, -52, -44, -40, -36


def vmts(p):
    d = p.data
    for name, va, vs, ro, rs in p.sections:
        if name not in ('.text', '.data'):
            continue
        for o in range(ro, ro + rs - 4, 4):
            v = struct.unpack_from('<I', d, o)[0]
            at = va + (o - ro) + p.image_base
            if v != at - SELF:
                continue
            vmt = v
            try:
                cn = p.u32(vmt + NAME)
                cls = p.read(cn + 1, p.read(cn, 1)[0]).decode('latin1')
                methods = []
                mt = p.u32(vmt + METHODS)
                if mt:
                    q = mt + 2
                    for _ in range(struct.unpack('<H', p.read(mt, 2))[0]):
                        sz = struct.unpack('<H', p.read(q, 2))[0]
                        addr = p.u32(q + 2)
                        methods.append((addr, p.read(q + 7, p.read(q + 6, 1)[0]).decode('latin1')))
                        q += sz
                par = p.u32(vmt + PARENT)
                yield vmt, cls, p.u32(vmt + SIZE), par, methods
            except Exception:
                pass


if __name__ == '__main__':
    p = PE()
    for vmt, cls, size, par, methods in vmts(p):
        if methods and any(0x401000 <= a < 0x422000 for a, _ in methods):
            print('%08x %s size=%d' % (vmt, cls, size))
            for a, n in methods:
                print('    %08x %s' % (a, n))
