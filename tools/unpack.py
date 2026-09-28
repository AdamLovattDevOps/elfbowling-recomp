"""Unpack the NVDPACKFILE resource ("NVD Rules!" archive) into build/assets.

Archive layout:
  0x00  char[16] "NVD Rules!"
  0x10  u32 count
  0x14  u32 data start (= 0x18 + 40*count)
  0x18  count entries of 40 bytes: char name[24]; u32 csize; u32 usize; u32 offset; u32 zero
Each entry is a zlib stream (inflate 1.0, which is linked into the game).
Name prefixes such as '#4' and '%' are loader flags and are kept in the manifest.
"""
import os, struct, sys, zlib
sys.path.insert(0, os.path.dirname(__file__))
from pe import PE
import rsrc


def entries(d):
    assert d[:10] == b'NVD Rules!'
    n = struct.unpack_from('<I', d, 0x10)[0]
    for i in range(n):
        o = 0x18 + 40 * i
        name = d[o:o + 24].split(b'\0')[0].decode('latin1')
        cs, us, off, z = struct.unpack_from('<IIII', d, o + 24)
        yield name, cs, us, off


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else 'build/assets'
    os.makedirs(out, exist_ok=True)
    res = {tuple(p): data for p, data in rsrc.walk(PE())}
    d = res[('NVDPACKFILE', 'PACKEDFILE', 1033)] if ('NVDPACKFILE', 'PACKEDFILE', 1033) in res else next(
        v for k, v in res.items() if k[0] == 'NVDPACKFILE')
    with open(os.path.join(out, 'manifest.tsv'), 'w') as man:
        for name, cs, us, off in entries(d):
            raw = zlib.decompress(d[off:off + cs])
            assert len(raw) == us, name
            clean = name.lstrip('#4%')
            open(os.path.join(out, clean), 'wb').write(raw)
            man.write('%s\t%s\t%d\n' % (name, clean, us))
    for k, v in res.items():
        if k[0] in ('NVDPARMFILE', 'NVDCASTFILE'):
            open(os.path.join(out, k[1] + '.bin'), 'wb').write(v)
    print('unpacked to', out)


if __name__ == '__main__':
    main()
