"""Borland/Intel OMF object and library reader.

It reads just enough to get each module's segments, as bytes plus a mask of
the bytes covered by a fixup, together with its public symbols.
"""
import struct

LOC_LEN = {0: 1, 1: 2, 2: 2, 3: 4, 4: 1, 5: 2, 9: 4, 11: 6, 13: 4}


class Segment:
    def __init__(self, name, cls, length):
        self.name, self.cls = name, cls
        self.data = bytearray(length)
        self.mask = bytearray(length)      # 1 = byte is covered by a fixup
        self.fixups = []                   # (offset, loc_type, self_relative, target description)

    def ensure(self, n):
        if len(self.data) < n:
            self.data.extend(b'\0' * (n - len(self.data)))
            self.mask.extend(b'\0' * (n - len(self.mask)))


class Module:
    def __init__(self):
        self.name = None
        self.segments = []
        self.virtuals = []                 # Borland virtual segments (one per function/COMDAT)
        self.publics = []                  # (name, seg_index or None, offset)
        self.externs = []
        self.xnames = []                   # EXTDEF and VIRDEF share one index space


def _idx(d, o):
    b = d[o]
    if b & 0x80:
        return ((b & 0x7f) << 8) | d[o + 1], o + 2
    return b, o + 1


def _name(d, o):
    n = d[o]
    return d[o + 1:o + 1 + n].decode('latin1'), o + 1 + n


def parse_records(d, start, end=None):
    """Yield modules parsed from d[start:end]; follows library page padding."""
    end = end or len(d)
    o = start
    mod = None
    lnames = []
    last = None     # (segment, base offset) of last LEDATA
    page = None
    if d[0] == 0xF0:
        page = struct.unpack_from('<H', d, 1)[0] + 3
        o = page
    while o < end:
        t = d[o]
        L = struct.unpack_from('<H', d, o + 1)[0]
        body = d[o + 3:o + 3 + L - 1]
        wide = t & 1
        if t == 0xF1:
            break
        if t == 0x80:
            mod = Module()
            lnames = [None]
            mod.name, _ = _name(body, 0)
        elif t == 0x96:
            p = 0
            while p < len(body):
                s, p = _name(body, p)
                lnames.append(s)
        elif t in (0x98, 0x99):
            p = 1
            ln = struct.unpack_from('<I' if wide else '<H', body, p)[0]
            p += 4 if wide else 2
            if body[0] & 2 and not wide:
                ln = 0x10000
            ni, p = _idx(body, p)
            ci, p = _idx(body, p)
            mod.segments.append(Segment(lnames[ni], lnames[ci], ln))
        elif t in (0x8C,):
            p = 0
            while p < len(body):
                s, p = _name(body, p)
                _, p = _idx(body, p)
                mod.externs.append(s)
                mod.xnames.append(s)
        elif t in (0x90, 0x91, 0xB6, 0xB7):
            p = 0
            gi, p = _idx(body, p)
            si, p = _idx(body, p)
            if si == 0:
                p += 2
            while p < len(body):
                s, p = _name(body, p)
                off = struct.unpack_from('<I' if wide else '<H', body, p)[0]
                p += 4 if wide else 2
                _, p = _idx(body, p)
                mod.publics.append((s, si - 1 if si else None, off))
        elif t == 0xB0:
            # Borland VIRDEF: defines a virtual segment named after the function.
            # Each entry: name, type index, segment index, COMDEF-style length.
            p = 0
            while p < len(body):
                nm, p = _name(body, p)
                _, p = _idx(body, p)
                seg_i, p = _idx(body, p)
                b = body[p]
                p += 1
                if b <= 0x80:
                    ln = b
                else:
                    k = {0x81: 2, 0x84: 3, 0x88: 4}[b]
                    ln = int.from_bytes(body[p:p + k], 'little')
                    p += k
                v = Segment(nm, 'VIRTUAL', ln)
                v.parent = seg_i
                mod.virtuals.append(v)
                mod.xnames.append(v)
        elif t in (0xA0, 0xA1):
            si, p = _idx(body, 0)
            off = struct.unpack_from('<I' if wide else '<H', body, p)[0]
            p += 4 if wide else 2
            seg = mod.xnames[si - 0x4001] if si >= 0x4000 else mod.segments[si - 1]
            chunk = body[p:]
            seg.ensure(off + len(chunk))
            seg.data[off:off + len(chunk)] = chunk
            last = (seg, off)
        elif t in (0xA2, 0xA3):
            last = None   # iterated data: rare in code, ignore
        elif t in (0x9C, 0x9D):
            p = 0
            while p < len(body):
                b = body[p]
                if not b & 0x80:          # THREAD subrecord
                    method = (b >> 2) & 7
                    p += 1
                    if not (b & 0x40 and method >= 4):
                        _, p = _idx(body, p)
                    continue
                locat = (b << 8) | body[p + 1]
                p += 2
                m = (locat >> 14) & 1
                loc = (locat >> 10) & 0xf
                doff = locat & 0x3ff
                fix = body[p]
                p += 1
                F, frame, T, P, targ = fix >> 7, (fix >> 4) & 7, (fix >> 3) & 1, (fix >> 2) & 1, fix & 3
                if not F and frame < 4:
                    _, p = _idx(body, p)
                tgt = None
                if not T:
                    tgt, p = _idx(body, p)
                    tgt = (targ, tgt)
                disp = 0
                if not P:
                    disp = struct.unpack_from('<I' if wide else '<H', body, p)[0]
                    p += 4 if wide else 2
                if last:
                    seg, base = last
                    at = base + doff
                    n = LOC_LEN.get(loc, 4)
                    seg.ensure(at + n)
                    for i in range(n):
                        seg.mask[at + i] = 1
                    seg.fixups.append((at, loc, not m, tgt, disp))
        elif t in (0x8A, 0x8B):
            if mod:
                yield mod
            mod = None
            if page:
                o = o + 3 + L
                o = (o + page - 1) // page * page
                continue
        o += 3 + L


def load(path):
    return list(parse_records(open(path, 'rb').read(), 0))
