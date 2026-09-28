"""Minimal PE32 reader for the Elf Bowling executable."""
import struct

DEFAULT = 'orig/1999/Elf Bowling.exe'


class PE:
    def __init__(self, path=DEFAULT):
        self.data = d = open(path, 'rb').read()
        pe = struct.unpack_from('<I', d, 0x3c)[0]
        opt = pe + 24
        self.image_base = struct.unpack_from('<I', d, opt + 28)[0]
        self.entry = struct.unpack_from('<I', d, opt + 16)[0] + self.image_base
        ndirs = struct.unpack_from('<I', d, opt + 92)[0]
        self.dirs = [struct.unpack_from('<II', d, opt + 96 + 8 * i) for i in range(ndirs)]
        nsec = struct.unpack_from('<H', d, pe + 6)[0]
        off = opt + struct.unpack_from('<H', d, pe + 20)[0]
        self.sections = []
        for i in range(nsec):
            name, vs, va, rs, ro = struct.unpack_from('<8sIIII', d, off + 40 * i)
            self.sections.append((name.rstrip(b'\0').decode(), va, vs, ro, rs))

    def section(self, name):
        for s in self.sections:
            if s[0] == name:
                return s
        raise KeyError(name)

    def rva2off(self, rva):
        for name, va, vs, ro, rs in self.sections:
            if va <= rva < va + max(vs, rs):
                return ro + rva - va
        raise ValueError(hex(rva))

    def va2off(self, va):
        return self.rva2off(va - self.image_base)

    def read(self, va, n):
        o = self.va2off(va)
        return self.data[o:o + n]

    def u32(self, va):
        return struct.unpack('<I', self.read(va, 4))[0]

    def cstr(self, va):
        o = self.va2off(va)
        return self.data[o:self.data.index(b'\0', o)].decode('latin1')

    def exports(self):
        rva, _ = self.dirs[0]
        o = self.rva2off(rva)
        (nfunc, nnames, afunc, anames, aords) = struct.unpack_from('<IIIII', self.data, o + 20)
        base = struct.unpack_from('<I', self.data, o + 16)[0]
        out = {}
        for i in range(nnames):
            nrva = struct.unpack_from('<I', self.data, self.rva2off(anames) + 4 * i)[0]
            ordi = struct.unpack_from('<H', self.data, self.rva2off(aords) + 2 * i)[0]
            frva = struct.unpack_from('<I', self.data, self.rva2off(afunc) + 4 * ordi)[0]
            out[self.cstr(nrva + self.image_base)] = frva + self.image_base
        return out

    def imports(self):
        """Return {iat_va: 'DLL!name'}."""
        rva, _ = self.dirs[1]
        o = self.rva2off(rva)
        out = {}
        while True:
            ilt, _, _, name, iat = struct.unpack_from('<IIIII', self.data, o)
            if not name:
                break
            dll = self.cstr(name + self.image_base)
            src = ilt or iat
            i = 0
            while True:
                ent = struct.unpack_from('<I', self.data, self.rva2off(src) + 4 * i)[0]
                if not ent:
                    break
                if ent & 0x80000000:
                    fn = '#%d' % (ent & 0xffff)
                else:
                    fn = self.cstr(ent + 2 + self.image_base)
                out[iat + 4 * i + self.image_base] = dll + '!' + fn
                i += 1
            o += 20
        return out

    def relocs(self):
        """Return sorted list of VAs that hold absolute (HIGHLOW) relocations."""
        rva, size = self.dirs[5]
        o = self.rva2off(rva)
        end = o + size
        out = []
        while o < end:
            page, blk = struct.unpack_from('<II', self.data, o)
            if not blk:
                break
            for i in range((blk - 8) // 2):
                e = struct.unpack_from('<H', self.data, o + 8 + 2 * i)[0]
                if e >> 12 == 3:
                    out.append(page + (e & 0xfff) + self.image_base)
            o += blk
        return sorted(out)
