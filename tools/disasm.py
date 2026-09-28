"""Disassemble a game function with names annotated.

Usage: dis.py ADDR   (hex, e.g. 411bac)
Annotates call targets (library symbols, imports, known function names),
string literals, and relocated operands.
"""
import os, sys, struct
sys.path.insert(0, os.path.dirname(__file__))
import capstone
from pe import PE

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    addr = int(sys.argv[1], 16)
    p = PE(os.path.join(ROOT, 'orig/1999/Elf Bowling.exe'))
    names, ends = {}, {}
    for line in open(os.path.join(ROOT, 'build/funcs.tsv')):
        s, e, n, name, kind = line.rstrip('\n').split('\t')
        names[int(s, 16)] = name; ends[int(s, 16)] = int(e, 16)
    for line in open(os.path.join(ROOT, 'build/libmap.tsv')):
        s, e, lib, mod, sym = line.rstrip('\n').split('\t')
        names.setdefault(int(s, 16), sym)
    imports = p.imports()
    relocs = set(p.relocs())
    end = ends.get(addr, addr + 256)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    print('; %s  %08x-%08x (%d bytes)' % (names.get(addr, '?'), addr, end, end - addr))
    for ins in md.disasm(p.read(addr, end - addr), addr):
        note = []
        for r in range(ins.address, ins.address + ins.size - 3):
            if r in relocs:
                v = p.u32(r)
                if v in names:
                    note.append(names[v])
                elif v in imports:
                    note.append(imports[v])
                else:
                    try:
                        s = p.cstr(v)
                        if s and all(32 <= ord(c) < 127 for c in s[:40]) and len(s) >= 2:
                            note.append('"%s"' % s[:60])
                        else:
                            note.append('&%08x' % v)
                    except Exception:
                        note.append('&%08x' % v)
        if ins.mnemonic in ('call', 'jmp') and ins.op_str.startswith('0x'):
            t = int(ins.op_str, 16)
            if t in names:
                note.append(names[t])
            else:
                # jmp thunk to import
                b = p.read(t, 6)
                if b[:2] == b'\xff\x25':
                    note.append(imports.get(struct.unpack('<I', b[2:])[0], '?'))
        print('%08x  %-24s %-8s %s%s' % (ins.address, ins.bytes.hex(), ins.mnemonic, ins.op_str,
                                        ('   ; ' + ', '.join(note)) if note else ''))


if __name__ == '__main__':
    main()
