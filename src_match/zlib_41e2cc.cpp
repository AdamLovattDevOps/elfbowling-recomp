// zlib (1.0.x era) linked into the game: adler32 and friends, from 0x41e2cc.
// Third-party code with its own zlib typedefs (z_stream, not elf/packres.h's
// ZStream), so it stays off the elf headers. For the native build it has
// ANSI prototypes only, no libc redeclarations and no `register` (removing
// it left every byte the same under bcc32 -Od).
typedef unsigned char Byte;
typedef Byte Bytef;
typedef unsigned int uInt;
// uLong is 32-bit, as on bcc32, so that z_stream has the layout of
// elf/packres.h's ZStream (0x38 bytes on bcc32) in the native build too.
#ifdef __BORLANDC__
typedef unsigned long uLong;
#else
typedef unsigned int uLong;
#endif
#define Z_NULL 0

#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5552

#define DO1(buf)  {s1 += *buf++; s2 += s1;}
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);
#define DO16(buf) DO8(buf); DO8(buf);

/* adler32 */
extern "C" uLong fn_41e2cc(uLong adler, const Bytef *buf, uInt len)
{
    unsigned long s1 = adler & 0xffff;
    unsigned long s2 = (adler >> 16) & 0xffff;
    int k;

    if (buf == Z_NULL) return 1L;

    while (len > 0) {
        k = len < NMAX ? len : NMAX;
        len -= k;
        while (k >= 16) {
            DO16(buf);
            k -= 16;
        }
        if (k != 0) do {
            DO1(buf);
        } while (--k);
        s1 %= BASE;
        s2 %= BASE;
    }
    return (s2 << 16) | s1;
}

/* ---- common zlib 1.0 types (zconf.h, zlib.h, zutil.h) ---- */
typedef char charf;
typedef int intf;
typedef uInt uIntf;
typedef uLong uLongf;
typedef void *voidpf;
typedef voidpf (*alloc_func)(voidpf opaque, uInt items, uInt size);
typedef void (*free_func)(voidpf opaque, voidpf address);
struct internal_state;
typedef struct z_stream_s {
    Bytef *next_in;
    uInt avail_in;
    uLong total_in;
    Bytef *next_out;
    uInt avail_out;
    uLong total_out;
    char *msg;
    struct internal_state *state;
    alloc_func zalloc;
    free_func zfree;
    voidpf opaque;
    int data_type;
    uLong adler;
    uLong reserved;
} z_stream;

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
#define ZLIB_VERSION "1.0"
#define DEF_WBITS 15

typedef uLong (*check_func)(uLong check, const Bytef *buf, uInt len);
#define ZALLOC(strm, items, size) \
           (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))

// bcc32: a plain extern "C" declaration keeps memcpy an ordinary call
// (<string.h> could make it the intrinsic). Natively, the libc prototypes.
#ifdef __BORLANDC__
extern "C" void *memcpy(void *d, const void *s, unsigned n);
#else
#include <string.h>
#endif
#define zmemcpy memcpy

/* inftrees.h */
typedef struct inflate_huft_s inflate_huft;
struct inflate_huft_s {
  union {
    struct {
      Byte Exop;
      Byte Bits;
    } what;
    Bytef *pad;
  } word;
  union {
    uInt Base;
    inflate_huft *Next;
  } more;
};

/* infcodes.c */
/* enums hoisted out of the structs so C++ sees them unqualified */
typedef enum { START, LEN, LENEXT, DIST, DISTEXT, COPY, LIT, WASH, END, BADCODE } inflate_codes_mode;
struct inflate_codes_state {
  inflate_codes_mode mode;
  uInt len;
  union {
    struct {
      inflate_huft *tree;
      uInt need;
    } code;
    uInt lit;
    struct {
      uInt get;
      uInt dist;
    } copy;
  } sub;
  Byte lbits;
  Byte dbits;
  inflate_huft *ltree;
  inflate_huft *dtree;
};
typedef struct inflate_codes_state inflate_codes_statef;

/* infutil.h */
typedef enum {
      TYPE, LENS, STORED, TABLE, BTREE, DTREE, CODES, DRY, DONEB, BADB}
inflate_block_mode;

struct inflate_blocks_state {
  inflate_block_mode  mode;
  union {
    uInt left;
    struct {
      uInt table;
      uInt index;
      uIntf *blens;
      uInt bb;
      inflate_huft *tb;
    } trees;
    struct {
      inflate_huft *tl, *td;
      inflate_codes_statef *codes;
    } decode;
  } sub;
  uInt last;
  uInt bitk;
  uLong bitb;
  Bytef *window;
  Bytef *end;
  Bytef *read;
  Bytef *write;
  check_func checkfn;
  uLong check;
};
typedef struct inflate_blocks_state inflate_blocks_statef;

/* inflate.c */
typedef enum {
      METHOD, FLAG, DICT4, DICT3, DICT2, DICT1, DICT0, BLOCKS,
      CHECK4, CHECK3, CHECK2, CHECK1, IDONE, IBAD}
inflate_mode;
struct internal_state {
  inflate_mode mode;
  union {
    uInt method;
    struct {
      uLong was;
      uLong need;
    } check;
    uInt marker;
  } sub;
  int  nowrap;
  uInt wbits;
  inflate_blocks_statef *blocks;
};

/* cross references, by address */
#define adler32               fn_41e2cc
#define inflate_blocks_reset  fn_41e4c0
#define inflate_blocks_new    fn_41e586
#define inflate_blocks        fn_41e619
#define inflate_blocks_free   fn_41f6c6
#define inflate_set_dictionary fn_41f705
#define inflate_codes_new     fn_41f734
#define inflate_codes         fn_41f783
#define inflate_codes_free    fn_4202d8
#define inflate_fast          fn_4202f0
#define inflateReset          fn_42077c
#define inflateEnd            fn_4207e6
#define inflateInit2_         fn_420850
#define inflateInit_          fn_420984
#define inflate               fn_42099c
#define inflateSetDictionary  fn_420ece
#define inflateSync           fn_420f81
#define huft_build            fn_4210ac
#define inflate_trees_bits    fn_4215d2
#define inflate_trees_dynamic fn_421634
#define falloc                fn_421729
#define inflate_trees_fixed   fn_421751
#define inflate_trees_free    fn_4218de
#define inflate_flush         fn_421950
#define zcalloc               fn_421af4
#define zcfree                fn_421b12

extern "C" void inflate_codes_free(inflate_codes_statef *c, z_stream *z);
extern "C" int inflate_trees_free(inflate_huft *t, z_stream *z);
extern "C" int huft_build(uIntf *b, uInt n, uInt s, uIntf *d, uIntf *e,
                          inflate_huft **t, uIntf *m, z_stream *zs);

/* ---- infblock.c ---- */
extern "C" void inflate_blocks_reset(inflate_blocks_statef *s, z_stream *z, uLongf *c)
{
  if (s->checkfn != 0)
    *c = s->check;
  if (s->mode == BTREE || s->mode == DTREE)
    ZFREE(z, s->sub.trees.blens);
  if (s->mode == CODES)
  {
    inflate_codes_free(s->sub.decode.codes, z);
    inflate_trees_free(s->sub.decode.td, z);
    inflate_trees_free(s->sub.decode.tl, z);
  }
  s->mode = TYPE;
  s->bitk = 0;
  s->bitb = 0;
  s->read = s->write = s->window;
  if (s->checkfn != 0)
    z->adler = s->check = (*s->checkfn)(0L, 0, 0);
}

extern "C" inflate_blocks_statef *inflate_blocks_new(z_stream *z, check_func c, uInt w)
{
  inflate_blocks_statef *s;

  if ((s = (inflate_blocks_statef *)ZALLOC
       (z,1,sizeof(struct inflate_blocks_state))) == 0)
    return s;
  if ((s->window = (Bytef *)ZALLOC(z, 1, w)) == 0)
  {
    ZFREE(z, s);
    return 0;
  }
  s->end = s->window + w;
  s->checkfn = c;
  s->mode = TYPE;
  inflate_blocks_reset(s, z, &s->check);
  return s;
}

extern "C" int inflate_blocks_free(inflate_blocks_statef *s, z_stream *z, uLongf *c)
{
  inflate_blocks_reset(s, z, c);
  ZFREE(z, s->window);
  ZFREE(z, s);
  return Z_OK;
}

extern "C" void inflate_set_dictionary(inflate_blocks_statef *s, z_stream *z, const Bytef *d, uInt n)
{
  zmemcpy((charf *)s->window, d, n);
  s->read = s->write = s->window + n;
}

/* ---- infcodes.c ---- */
extern "C" inflate_codes_statef *inflate_codes_new(uInt bl, uInt bd, inflate_huft *tl,
                                                   inflate_huft *td, z_stream *z)
{
  inflate_codes_statef *c;

  if ((c = (inflate_codes_statef *)
       ZALLOC(z,1,sizeof(struct inflate_codes_state))) != 0)
  {
    c->mode = START;
    c->lbits = (Byte)bl;
    c->dbits = (Byte)bd;
    c->ltree = tl;
    c->dtree = td;
  }
  return c;
}

extern "C" void inflate_codes_free(inflate_codes_statef *c, z_stream *z)
{
  ZFREE(z, c);
}

/* ---- inflate.c ---- */
extern "C" int inflateReset(z_stream *z)
{
  uLong c;

  if (z == 0 || z->state == 0)
    return Z_STREAM_ERROR;
  z->total_in = z->total_out = 0;
  z->msg = 0;
  z->state->mode = z->state->nowrap ? BLOCKS : METHOD;
  inflate_blocks_reset(z->state->blocks, z, &c);
  return Z_OK;
}

extern "C" int inflateEnd(z_stream *z)
{
  uLong c;

  if (z == 0 || z->state == 0 || z->zfree == 0)
    return Z_STREAM_ERROR;
  if (z->state->blocks != 0)
    inflate_blocks_free(z->state->blocks, z, &c);
  ZFREE(z, z->state);
  z->state = 0;
  return Z_OK;
}

extern "C" voidpf zcalloc(voidpf opaque, unsigned items, unsigned size);
extern "C" void zcfree(voidpf opaque, voidpf ptr);

extern "C" int inflateInit2_(z_stream *z, int w, const char *version, int stream_size)
{
  if (version == 0 || version[0] != ZLIB_VERSION[0] ||
      stream_size != sizeof(z_stream))
      return Z_VERSION_ERROR;

  if (z == 0)
    return Z_STREAM_ERROR;
  z->msg = 0;
  if (z->zalloc == 0)
  {
    z->zalloc = zcalloc;
    z->opaque = (voidpf)0;
  }
  if (z->zfree == 0) z->zfree = zcfree;
  if ((z->state = (struct internal_state *)
       ZALLOC(z,1,sizeof(struct internal_state))) == 0)
    return Z_MEM_ERROR;
  z->state->blocks = 0;

  z->state->nowrap = 0;
  if (w < 0)
  {
    w = - w;
    z->state->nowrap = 1;
  }

  if (w < 8 || w > 15)
  {
    inflateEnd(z);
    return Z_STREAM_ERROR;
  }
  z->state->wbits = (uInt)w;

  if ((z->state->blocks =
      inflate_blocks_new(z, z->state->nowrap ? 0 : adler32, (uInt)1 << w))
      == 0)
  {
    inflateEnd(z);
    return Z_MEM_ERROR;
  }

  inflateReset(z);
  return Z_OK;
}

extern "C" int inflateInit_(z_stream *z, const char *version, int stream_size)
{
  return inflateInit2_(z, DEF_WBITS, version, stream_size);
}

extern "C" int inflateSetDictionary(z_stream *z, const Bytef *dictionary, uInt dictLength)
{
  uInt length = dictLength;

  if (z == 0 || z->state == 0 || z->state->mode != DICT0)
    return Z_STREAM_ERROR;
  if (adler32(1L, dictionary, dictLength) != z->adler) return Z_DATA_ERROR;
  z->adler = 1L;

  if (length >= (1<<z->state->wbits))
  {
    length = (1<<z->state->wbits)-1;
    dictionary += dictLength - length;
  }
  inflate_set_dictionary(z->state->blocks, z, dictionary, length);
  z->state->mode = BLOCKS;
  return Z_OK;
}

extern "C" int inflateSync(z_stream *z)
{
  uInt n;
  Bytef *p;
  uInt m;
  uLong r, w;

  if (z == 0 || z->state == 0)
    return Z_STREAM_ERROR;
  if (z->state->mode != IBAD)
  {
    z->state->mode = IBAD;
    z->state->sub.marker = 0;
  }
  if ((n = z->avail_in) == 0)
    return Z_BUF_ERROR;
  p = z->next_in;
  m = z->state->sub.marker;

  while (n && m < 4)
  {
    if (*p == (Byte)(m < 2 ? 0 : 0xff))
      m++;
    else if (*p)
      m = 0;
    else
      m = 4 - m;
    p++, n--;
  }

  z->total_in += p - z->next_in;
  z->next_in = p;
  z->avail_in = n;
  z->state->sub.marker = m;

  if (m != 4)
    return Z_DATA_ERROR;
  r = z->total_in;  w = z->total_out;
  inflateReset(z);
  z->total_in = r;  z->total_out = w;
  z->state->mode = BLOCKS;
  return Z_OK;
}

/* ---- inftrees.c ---- */
#define base more.Base
#define next more.Next
#define exop word.what.Exop
#define bits word.what.Bits

extern "C" uInt g_cplens_45c0f0[31];
extern "C" uInt g_cplext_45c16c[31];
extern "C" uInt g_cpdist_45c1e8[30];
extern "C" uInt g_cpdext_45c260[30];
#define cplens g_cplens_45c0f0
#define cplext g_cplext_45c16c
#define cpdist g_cpdist_45c1e8
#define cpdext g_cpdext_45c260

extern "C" int inflate_trees_bits(uIntf *c, uIntf *bb, inflate_huft **tb, z_stream *z)
{
  int r;

  r = huft_build(c, 19, 19, (uIntf*)0, (uIntf*)0, tb, bb, z);
  if (r == Z_DATA_ERROR)
    z->msg = (char*)"oversubscribed dynamic bit lengths tree";
  else if (r == Z_BUF_ERROR)
  {
    inflate_trees_free(*tb, z);
    z->msg = (char*)"incomplete dynamic bit lengths tree";
    r = Z_DATA_ERROR;
  }
  return r;
}

extern "C" int inflate_trees_dynamic(uInt nl, uInt nd, uIntf *c, uIntf *bl, uIntf *bd,
                                     inflate_huft **tl, inflate_huft **td, z_stream *z)
{
  int r;

  if ((r = huft_build(c, nl, 257, cplens, cplext, tl, bl, z)) != Z_OK)
  {
    if (r == Z_DATA_ERROR)
      z->msg = (char*)"oversubscribed literal/length tree";
    else if (r == Z_BUF_ERROR)
    {
      inflate_trees_free(*tl, z);
      z->msg = (char*)"incomplete literal/length tree";
      r = Z_DATA_ERROR;
    }
    return r;
  }

  if ((r = huft_build(c + nl, nd, 0, cpdist, cpdext, td, bd, z)) != Z_OK)
  {
    if (r == Z_DATA_ERROR)
      z->msg = (char*)"oversubscribed literal/length tree";
    else if (r == Z_BUF_ERROR) {
      inflate_trees_free(*td, z);
      z->msg = (char*)"incomplete literal/length tree";
      r = Z_DATA_ERROR;
    }
    inflate_trees_free(*tl, z);
    return r;
  }

  return Z_OK;
}

#define FIXEDH 530
extern "C" int g_fixed_built_45c2d8;
#ifdef __BORLANDC__
extern "C" inflate_huft g_fixed_mem_460798[FIXEDH];
extern "C" uInt g_fixed_bl, g_fixed_bd;
extern "C" inflate_huft *g_fixed_tl, *g_fixed_td;
#else
// zlib `local` bss variables: no initial value in the exe, and inflate_huft
// is zlib-local, so the native game-data generator cannot define them.
extern "C" {
inflate_huft g_fixed_mem_460798[FIXEDH];
uInt g_fixed_bl, g_fixed_bd;
inflate_huft *g_fixed_tl, *g_fixed_td;
}
#endif
#define fixed_built g_fixed_built_45c2d8
#define fixed_mem g_fixed_mem_460798
#define fixed_bl g_fixed_bl
#define fixed_bd g_fixed_bd
#define fixed_tl g_fixed_tl
#define fixed_td g_fixed_td

extern "C" voidpf falloc(voidpf q, uInt n, uInt s)
{
  *(intf *)q -= n+s-s;
  return (voidpf)(fixed_mem + *(intf *)q);
}

extern "C" int inflate_trees_fixed(uIntf *bl, uIntf *bd, inflate_huft **tl, inflate_huft **td)
{
  if (!fixed_built)
  {
    int k;
    unsigned c[288];
    z_stream z;
    int f = FIXEDH;

    z.zalloc = falloc;
    z.zfree = 0;
    z.opaque = (voidpf)&f;

    for (k = 0; k < 144; k++)
      c[k] = 8;
    for (; k < 256; k++)
      c[k] = 9;
    for (; k < 280; k++)
      c[k] = 7;
    for (; k < 288; k++)
      c[k] = 8;
    fixed_bl = 7;
    huft_build(c, 288, 257, cplens, cplext, &fixed_tl, &fixed_bl, &z);

    for (k = 0; k < 30; k++)
      c[k] = 5;
    fixed_bd = 5;
    huft_build(c, 30, 0, cpdist, cpdext, &fixed_td, &fixed_bd, &z);

    fixed_built = 1;
  }
  *bl = fixed_bl;
  *bd = fixed_bd;
  *tl = fixed_tl;
  *td = fixed_td;
  return Z_OK;
}

extern "C" int inflate_trees_free(inflate_huft *t, z_stream *z)
{
  inflate_huft *p, *q, *r;

  p = 0;
  q = t;
  while (q != 0)
  {
    r = (q - 1)->next;
    (q - 1)->next = p;
    p = q;
    q = r;
  }
  while (p != 0)
  {
    q = (--p)->next;
    ZFREE(z,p);
    p = q;
  }
  return Z_OK;
}

/* ---- infutil.c ---- */
extern "C" int inflate_flush(inflate_blocks_statef *s, z_stream *z, int r)
{
  uInt n;
  Bytef *p, *q;

  p = z->next_out;
  q = s->read;

  n = (uInt)((q <= s->write ? s->write : s->end) - q);
  if (n > z->avail_out) n = z->avail_out;
  if (n && r == Z_BUF_ERROR) r = Z_OK;

  z->avail_out -= n;
  z->total_out += n;

  if (s->checkfn != 0)
    z->adler = s->check = (*s->checkfn)(s->check, q, n);

  zmemcpy(p, q, n);
  p += n;
  q += n;

  if (q == s->end)
  {
    q = s->window;
    if (s->write == s->end)
      s->write = s->window;

    n = (uInt)(s->write - q);
    if (n > z->avail_out) n = z->avail_out;
    if (n && r == Z_BUF_ERROR) r = Z_OK;

    z->avail_out -= n;
    z->total_out += n;

    if (s->checkfn != 0)
      z->adler = s->check = (*s->checkfn)(s->check, q, n);

    zmemcpy(p, q, n);
    p += n;
    q += n;
  }

  z->next_out = p;
  s->read = q;

  return r;
}

/* ---- zutil.c ---- */
// z_error: the game's error hook (elf/game.h g_45c41c; packres_40dce0 sets it).
extern "C" void (*g_45c41c)(const char *msg);

extern "C" void fn_421adc(void *p)
{
  if (g_45c41c)
    g_45c41c((const char *)p);
}

#ifdef __BORLANDC__
extern "C" void *calloc(unsigned n, unsigned size);
extern "C" void free(void *p);
#else
#include <stdlib.h>
#endif

extern "C" voidpf zcalloc(voidpf opaque, unsigned items, unsigned size)
{
    if (opaque) opaque = 0;
    return (voidpf)calloc(items, size);
}

extern "C" void zcfree(voidpf opaque, voidpf ptr)
{
    if (opaque) opaque = 0;
    free(ptr);
}

#ifndef __BORLANDC__
// 0x40e19c is not library code: libmap's std::basic_ifstream::is_open label is
// a false match. Its body is `push s+0x10; call fn_4207e6`, i.e. inflateEnd
// on the z_stream of a TPackedStream (TPackedStreamData::z, elf/packres.h).
// It is outside the game ranges, so the matching build does not define it;
// natively it is defined here against the stream layout (the 0x10 offset is
// the 32-bit one, so no raw offset).
struct elf_packed_stream_head {
    void *vmt;                  // TStream vtable
    char *out;
    int outsize;
    int used;
    z_stream z;
};
extern "C" int fn_40e19c(void *s)
{
    return inflateEnd(&((elf_packed_stream_head *)s)->z);
}
#endif

/* ==== large functions, straight from zlib 1.0 (K&R headers made ANSI) ==== */
/* defines for inflate input/output */
/*   update pointers and return */
#define UPDBITS {s->bitb=b;s->bitk=k;}
#define UPDIN {z->avail_in=n;z->total_in+=p-z->next_in;z->next_in=p;}
#define UPDOUT {s->write=q;}
#define UPDATE {UPDBITS UPDIN UPDOUT}
#define LEAVE {UPDATE return inflate_flush(s,z,r);}
/*   get bytes and bits */
#define LOADIN {p=z->next_in;n=z->avail_in;b=s->bitb;k=s->bitk;}
#define NEEDBYTE {if(n)r=Z_OK;else LEAVE}
#define NEXTBYTE (n--,*p++)
#define NEEDBITS(j) {while(k<(j)){NEEDBYTE;b|=((uLong)NEXTBYTE)<<k;k+=8;}}
#define DUMPBITS(j) {b>>=(j);k-=(j);}
/*   output bytes */
#define WAVAIL (uInt)(q<s->read?s->read-q-1:s->end-q)
#define LOADOUT {q=s->write;m=(uInt)WAVAIL;}
#define WRAP {if(q==s->end&&s->read!=s->window){q=s->window;m=(uInt)WAVAIL;}}
#define FLUSH {UPDOUT r=inflate_flush(s,z,r); LOADOUT}
#define NEEDOUT {if(m==0){WRAP if(m==0){FLUSH WRAP if(m==0) LEAVE}}r=Z_OK;}
#define OUTBYTE(a) {*q++=(Byte)(a);m--;}
/*   load local pointers */
#define LOAD {LOADIN LOADOUT}


#define Trace(x)
#define Tracev(x)
#define Tracevv(x)
#define Assert(c,m)
#define Z_NULL 0
#define local static
#define FAR
extern "C" uInt g_inflate_mask_45c3ac[17];
#define inflate_mask g_inflate_mask_45c3ac
extern "C" int inflate_flush(inflate_blocks_statef *s, z_stream *z, int r);
extern "C" int inflate_trees_bits(uIntf *c, uIntf *bb, inflate_huft **tb, z_stream *z);
extern "C" int inflate_trees_dynamic(uInt nl, uInt nd, uIntf *c, uIntf *bl, uIntf *bd, inflate_huft **tl, inflate_huft **td, z_stream *z);
extern "C" int inflate_trees_fixed(uIntf *bl, uIntf *bd, inflate_huft **tl, inflate_huft **td);
extern "C" inflate_codes_statef *inflate_codes_new(uInt bl, uInt bd, inflate_huft *tl, inflate_huft *td, z_stream *z);
extern "C" int inflate_codes(inflate_blocks_statef *s, z_stream *z, int r);
extern "C" int inflate_fast(uInt bl, uInt bd, inflate_huft *tl, inflate_huft *td, inflate_blocks_statef *s, z_stream *z);
extern "C" int inflate_blocks(inflate_blocks_statef *s, z_stream *z, int r);
extern "C" void inflate_blocks_reset(inflate_blocks_statef *s, z_stream *z, uLongf *c);
extern "C" int inflateReset(z_stream *z);
extern "C" uInt g_border_45bf2c[];
#define border g_border_45bf2c


/* infblock.c: the DTREE loop is written with goto; the original
   "while (t = ..., cond)" makes C++ materialise a bool. */
#define DONE DONEB
#define BAD BADB
extern "C" int inflate_blocks(inflate_blocks_statef *s, z_stream *z, int r)
{
  uInt t;               /* temporary storage */
  uLong b;              /* bit buffer */
  uInt k;               /* bits in bit buffer */
  Bytef *p;             /* input data pointer */
  uInt n;               /* bytes available there */
  Bytef *q;             /* output window write pointer */
  uInt m;               /* bytes to end of window or read pointer */

  /* copy input/output information to locals (UPDATE macro restores) */
  LOAD

  /* process input based on current state */
  while (1) switch (s->mode)
  {
    case TYPE:
      NEEDBITS(3)
      t = (uInt)b & 7;
      s->last = t & 1;
      switch (t >> 1)
      {
        case 0:                         /* stored */
          Trace((stderr, "inflate:     stored block%s\n",
                 s->last ? " (last)" : ""));
          DUMPBITS(3)
          t = k & 7;                    /* go to byte boundary */
          DUMPBITS(t)
          s->mode = LENS;               /* get length of stored block */
          break;
        case 1:                         /* fixed */
          Trace((stderr, "inflate:     fixed codes block%s\n",
                 s->last ? " (last)" : ""));
          {
            uInt bl, bd;
            inflate_huft *tl, *td;

            inflate_trees_fixed(&bl, &bd, &tl, &td);
            s->sub.decode.codes = inflate_codes_new(bl, bd, tl, td, z);
            if (s->sub.decode.codes == Z_NULL)
            {
              r = Z_MEM_ERROR;
              LEAVE
            }
            s->sub.decode.tl = Z_NULL;  /* don't try to free these */
            s->sub.decode.td = Z_NULL;
          }
          DUMPBITS(3)
          s->mode = CODES;
          break;
        case 2:                         /* dynamic */
          Trace((stderr, "inflate:     dynamic codes block%s\n",
                 s->last ? " (last)" : ""));
          DUMPBITS(3)
          s->mode = TABLE;
          break;
        case 3:                         /* illegal */
          DUMPBITS(3)
          s->mode = BAD;
          z->msg = (char*)"invalid block type";
          r = Z_DATA_ERROR;
          LEAVE
      }
      break;
    case LENS:
      NEEDBITS(32)
      if ((((~b) >> 16) & 0xffff) != (b & 0xffff))
      {
        s->mode = BAD;
        z->msg = (char*)"invalid stored block lengths";
        r = Z_DATA_ERROR;
        LEAVE
      }
      s->sub.left = (uInt)b & 0xffff;
      b = k = 0;                      /* dump bits */
      Tracev((stderr, "inflate:       stored length %u\n", s->sub.left));
      s->mode = s->sub.left ? STORED : TYPE;
      break;
    case STORED:
      if (n == 0)
        LEAVE
      NEEDOUT
      t = s->sub.left;
      if (t > n) t = n;
      if (t > m) t = m;
      zmemcpy(q, p, t);
      p += t;  n -= t;
      q += t;  m -= t;
      if ((s->sub.left -= t) != 0)
        break;
      Tracev((stderr, "inflate:       stored end, %lu total out\n",
              z->total_out + (q >= s->read ? q - s->read :
              (s->end - s->read) + (q - s->window))));
      s->mode = s->last ? DRY : TYPE;
      break;
    case TABLE:
      NEEDBITS(14)
      s->sub.trees.table = t = (uInt)b & 0x3fff;
#ifndef PKZIP_BUG_WORKAROUND
      if ((t & 0x1f) > 29 || ((t >> 5) & 0x1f) > 29)
      {
        s->mode = BAD;
        z->msg = (char*)"too many length or distance symbols";
        r = Z_DATA_ERROR;
        LEAVE
      }
#endif
      t = 258 + (t & 0x1f) + ((t >> 5) & 0x1f);
      if (t < 19)
        t = 19;
      if ((s->sub.trees.blens = (uIntf*)ZALLOC(z, t, sizeof(uInt))) == Z_NULL)
      {
        r = Z_MEM_ERROR;
        LEAVE
      }
      DUMPBITS(14)
      s->sub.trees.index = 0;
      Tracev((stderr, "inflate:       table sizes ok\n"));
      s->mode = BTREE;
    case BTREE:
      while (s->sub.trees.index < 4 + (s->sub.trees.table >> 10))
      {
        NEEDBITS(3)
        s->sub.trees.blens[border[s->sub.trees.index++]] = (uInt)b & 7;
        DUMPBITS(3)
      }
      while (s->sub.trees.index < 19)
        s->sub.trees.blens[border[s->sub.trees.index++]] = 0;
      s->sub.trees.bb = 7;
      t = inflate_trees_bits(s->sub.trees.blens, &s->sub.trees.bb,
                             &s->sub.trees.tb, z);
      if (t != Z_OK)
      {
        r = t;
        if (r == Z_DATA_ERROR)
          s->mode = BAD;
        LEAVE
      }
      s->sub.trees.index = 0;
      Tracev((stderr, "inflate:       bits tree ok\n"));
      s->mode = DTREE;
    case DTREE:
      goto dtree_test;
    dtree_body:
      {
        inflate_huft *h;
        uInt i, j, c;

        t = s->sub.trees.bb;
        NEEDBITS(t)
        h = s->sub.trees.tb + ((uInt)b & inflate_mask[t]);
        t = h->word.what.Bits;
        c = h->more.Base;
        if (c < 16)
        {
          DUMPBITS(t)
          s->sub.trees.blens[s->sub.trees.index++] = c;
        }
        else /* c == 16..18 */
        {
          i = c == 18 ? 7 : c - 14;
          j = c == 18 ? 11 : 3;
          NEEDBITS(t + i)
          DUMPBITS(t)
          j += (uInt)b & inflate_mask[i];
          DUMPBITS(i)
          i = s->sub.trees.index;
          t = s->sub.trees.table;
          if (i + j > 258 + (t & 0x1f) + ((t >> 5) & 0x1f) ||
              (c == 16 && i < 1))
          {
            s->mode = BAD;
            z->msg = (char*)"invalid bit length repeat";
            r = Z_DATA_ERROR;
            LEAVE
          }
          c = c == 16 ? s->sub.trees.blens[i - 1] : 0;
          do {
            s->sub.trees.blens[i++] = c;
          } while (--j);
          s->sub.trees.index = i;
        }
      }
    dtree_test:
      t = s->sub.trees.table;
      if (s->sub.trees.index < 258 + (t & 0x1f) + ((t >> 5) & 0x1f))
        goto dtree_body;
      inflate_trees_free(s->sub.trees.tb, z);
      s->sub.trees.tb = Z_NULL;
      {
        uInt bl, bd;
        inflate_huft *tl, *td;
        inflate_codes_statef *c;

        bl = 9;         /* must be <= 9 for lookahead assumptions */
        bd = 6;         /* must be <= 9 for lookahead assumptions */
        t = s->sub.trees.table;
        t = inflate_trees_dynamic(257 + (t & 0x1f), 1 + ((t >> 5) & 0x1f),
                                  s->sub.trees.blens, &bl, &bd, &tl, &td, z);
        if (t != Z_OK)
        {
          if (t == (uInt)Z_DATA_ERROR)
            s->mode = BAD;
          r = t;
          LEAVE
        }
        Tracev((stderr, "inflate:       trees ok\n"));
        if ((c = inflate_codes_new(bl, bd, tl, td, z)) == Z_NULL)
        {
          inflate_trees_free(td, z);
          inflate_trees_free(tl, z);
          r = Z_MEM_ERROR;
          LEAVE
        }
        ZFREE(z, s->sub.trees.blens);
        s->sub.decode.codes = c;
        s->sub.decode.tl = tl;
        s->sub.decode.td = td;
      }
      s->mode = CODES;
    case CODES:
      UPDATE
      if ((r = inflate_codes(s, z, r)) != Z_STREAM_END)
        return inflate_flush(s, z, r);
      r = Z_OK;
      inflate_codes_free(s->sub.decode.codes, z);
      inflate_trees_free(s->sub.decode.td, z);
      inflate_trees_free(s->sub.decode.tl, z);
      LOAD
      Tracev((stderr, "inflate:       codes end, %lu total out\n",
              z->total_out + (q >= s->read ? q - s->read :
              (s->end - s->read) + (q - s->window))));
      if (!s->last)
      {
        s->mode = TYPE;
        break;
      }
      if (k > 7)              /* return unused byte, if any */
      {
        Assert(k < 16, "inflate_codes grabbed too many bytes")
        k -= 8;
        n++;
        p--;                    /* can always return one */
      }
      s->mode = DRY;
    case DRY:
      FLUSH
      if (s->read != s->write)
        LEAVE
      s->mode = DONE;
    case DONE:
      r = Z_STREAM_END;
      LEAVE
    case BAD:
      r = Z_DATA_ERROR;
      LEAVE
    default:
      r = Z_STREAM_ERROR;
      LEAVE
  }
}
#undef DONE
#undef BAD

/* infcodes.c */
extern "C" int inflate_codes(inflate_blocks_statef *s, z_stream *z, int r)
{
  uInt j;               /* temporary storage */
  inflate_huft *t;      /* temporary pointer */
  uInt e;               /* extra bits or operation */
  uLong b;              /* bit buffer */
  uInt k;               /* bits in bit buffer */
  Bytef *p;             /* input data pointer */
  uInt n;               /* bytes available there */
  Bytef *q;             /* output window write pointer */
  uInt m;               /* bytes to end of window or read pointer */
  Bytef *f;             /* pointer to copy strings from */
  inflate_codes_statef *c = s->sub.decode.codes;  /* codes state */

  /* copy input/output information to locals (UPDATE macro restores) */
  LOAD

  /* process input and output based on current state */
  while (1) switch (c->mode)
  {             /* waiting for "i:"=input, "o:"=output, "x:"=nothing */
    case START:         /* x: set up for LEN */
#ifndef SLOW
      if (m >= 258 && n >= 10)
      {
        UPDATE
        r = inflate_fast(c->lbits, c->dbits, c->ltree, c->dtree, s, z);
        LOAD
        if (r != Z_OK)
        {
          c->mode = r == Z_STREAM_END ? WASH : BADCODE;
          break;
        }
      }
#endif /* !SLOW */
      c->sub.code.need = c->lbits;
      c->sub.code.tree = c->ltree;
      c->mode = LEN;
    case LEN:           /* i: get length/literal/eob next */
      j = c->sub.code.need;
      NEEDBITS(j)
      t = c->sub.code.tree + ((uInt)b & inflate_mask[j]);
      DUMPBITS(t->bits)
      e = (uInt)(t->exop);
      if (e == 0)               /* literal */
      {
        c->sub.lit = t->base;
        Tracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
                 "inflate:         literal '%c'\n" :
                 "inflate:         literal 0x%02x\n", t->base));
        c->mode = LIT;
        break;
      }
      if (e & 16)               /* length */
      {
        c->sub.copy.get = e & 15;
        c->len = t->base;
        c->mode = LENEXT;
        break;
      }
      if ((e & 64) == 0)        /* next table */
      {
        c->sub.code.need = e;
        c->sub.code.tree = t->next;
        break;
      }
      if (e & 32)               /* end of block */
      {
        Tracevv((stderr, "inflate:         end of block\n"));
        c->mode = WASH;
        break;
      }
      c->mode = BADCODE;        /* invalid code */
      z->msg = (char*)"invalid literal/length code";
      r = Z_DATA_ERROR;
      LEAVE
    case LENEXT:        /* i: getting length extra (have base) */
      j = c->sub.copy.get;
      NEEDBITS(j)
      c->len += (uInt)b & inflate_mask[j];
      DUMPBITS(j)
      c->sub.code.need = c->dbits;
      c->sub.code.tree = c->dtree;
      Tracevv((stderr, "inflate:         length %u\n", c->len));
      c->mode = DIST;
    case DIST:          /* i: get distance next */
      j = c->sub.code.need;
      NEEDBITS(j)
      t = c->sub.code.tree + ((uInt)b & inflate_mask[j]);
      DUMPBITS(t->bits)
      e = (uInt)(t->exop);
      if (e & 16)               /* distance */
      {
        c->sub.copy.get = e & 15;
        c->sub.copy.dist = t->base;
        c->mode = DISTEXT;
        break;
      }
      if ((e & 64) == 0)        /* next table */
      {
        c->sub.code.need = e;
        c->sub.code.tree = t->next;
        break;
      }
      c->mode = BADCODE;        /* invalid code */
      z->msg = (char*)"invalid distance code";
      r = Z_DATA_ERROR;
      LEAVE
    case DISTEXT:       /* i: getting distance extra */
      j = c->sub.copy.get;
      NEEDBITS(j)
      c->sub.copy.dist += (uInt)b & inflate_mask[j];
      DUMPBITS(j)
      Tracevv((stderr, "inflate:         distance %u\n", c->sub.copy.dist));
      c->mode = COPY;
    case COPY:          /* o: copying bytes in window, waiting for space */
#ifndef __TURBOC__ /* Turbo C bug for following expression */
      f = (uInt)(q - s->window) < c->sub.copy.dist ?
          s->end - (c->sub.copy.dist - (q - s->window)) :
          q - c->sub.copy.dist;
#else
      f = q - c->sub.copy.dist;
      if ((uInt)(q - s->window) < c->sub.copy.dist)
        f = s->end - (c->sub.copy.dist - (uInt)(q - s->window));
#endif
      while (c->len)
      {
        NEEDOUT
        OUTBYTE(*f++)
        if (f == s->end)
          f = s->window;
        c->len--;
      }
      c->mode = START;
      break;
    case LIT:           /* o: got literal, waiting for output space */
      NEEDOUT
      OUTBYTE(c->sub.lit)
      c->mode = START;
      break;
    case WASH:          /* o: got eob, possibly more output */
      FLUSH
      if (s->read != s->write)
        LEAVE
      c->mode = END;
    case END:
      r = Z_STREAM_END;
      LEAVE
    case BADCODE:       /* x: got error */
      r = Z_DATA_ERROR;
      LEAVE
    default:
      r = Z_STREAM_ERROR;
      LEAVE
  }
}

/* inffast.c */
#define GRABBITS(j) {while(k<(j)){b|=((uLong)NEXTBYTE)<<k;k+=8;}}
#define UNGRAB {n+=(c=k>>3);p-=c;k&=7;}
extern "C" int inflate_fast(uInt bl, uInt bd, inflate_huft *tl, inflate_huft *td, inflate_blocks_statef *s, z_stream *z)
{
  inflate_huft *t;      /* temporary pointer */
  uInt e;               /* extra bits or operation */
  uLong b;              /* bit buffer */
  uInt k;               /* bits in bit buffer */
  Bytef *p;             /* input data pointer */
  uInt n;               /* bytes available there */
  Bytef *q;             /* output window write pointer */
  uInt m;               /* bytes to end of window or read pointer */
  uInt ml;              /* mask for literal/length tree */
  uInt md;              /* mask for distance tree */
  uInt c;               /* bytes to copy */
  uInt d;               /* distance back to copy from */
  Bytef *r;             /* copy source pointer */

  /* load input, output, bit values */
  LOAD

  /* initialize masks */
  ml = inflate_mask[bl];
  md = inflate_mask[bd];

  /* do until not enough input or output space for fast loop */
  do {                          /* assume called with m >= 258 && n >= 10 */
    /* get literal/length code */
    GRABBITS(20)                /* max bits for literal/length code */
    if ((e = (t = tl + ((uInt)b & ml))->exop) == 0)
    {
      DUMPBITS(t->bits)
      Tracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
                "inflate:         * literal '%c'\n" :
                "inflate:         * literal 0x%02x\n", t->base));
      *q++ = (Byte)t->base;
      m--;
      continue;
    }
    do {
      DUMPBITS(t->bits)
      if (e & 16)
      {
        /* get extra bits for length */
        e &= 15;
        c = t->base + ((uInt)b & inflate_mask[e]);
        DUMPBITS(e)
        Tracevv((stderr, "inflate:         * length %u\n", c));

        /* decode distance base of block to copy */
        GRABBITS(15);           /* max bits for distance code */
        e = (t = td + ((uInt)b & md))->exop;
        do {
          DUMPBITS(t->bits)
          if (e & 16)
          {
            /* get extra bits to add to distance base */
            e &= 15;
            GRABBITS(e)         /* get extra bits (up to 13) */
            d = t->base + ((uInt)b & inflate_mask[e]);
            DUMPBITS(e)
            Tracevv((stderr, "inflate:         * distance %u\n", d));

            /* do the copy */
            m -= c;
            if ((uInt)(q - s->window) >= d)     /* offset before dest */
            {                                   /*  just copy */
              r = q - d;
              *q++ = *r++;  c--;        /* minimum count is three, */
              *q++ = *r++;  c--;        /*  so unroll loop a little */
            }
            else                        /* else offset after destination */
            {
              e = d - (uInt)(q - s->window); /* bytes from offset to end */
              r = s->end - e;           /* pointer to offset */
              if (c > e)                /* if source crosses, */
              {
                c -= e;                 /* copy to end of window */
                do {
                  *q++ = *r++;
                } while (--e);
                r = s->window;          /* copy rest from start of window */
              }
            }
            do {                        /* copy all or what's left */
              *q++ = *r++;
            } while (--c);
            break;
          }
          else if ((e & 64) == 0)
            e = (t = t->next + ((uInt)b & inflate_mask[e]))->exop;
          else
          {
            z->msg = (char*)"invalid distance code";
            UNGRAB
            UPDATE
            return Z_DATA_ERROR;
          }
        } while (1);
        break;
      }
      if ((e & 64) == 0)
      {
        if ((e = (t = t->next + ((uInt)b & inflate_mask[e]))->exop) == 0)
        {
          DUMPBITS(t->bits)
          Tracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
                    "inflate:         * literal '%c'\n" :
                    "inflate:         * literal 0x%02x\n", t->base));
          *q++ = (Byte)t->base;
          m--;
          break;
        }
      }
      else if (e & 32)
      {
        Tracevv((stderr, "inflate:         * end of block\n"));
        UNGRAB
        UPDATE
        return Z_STREAM_END;
      }
      else
      {
        z->msg = (char*)"invalid literal/length code";
        UNGRAB
        UPDATE
        return Z_DATA_ERROR;
      }
    } while (1);
  } while (m >= 258 && n >= 10);

  /* not enough input or output--restore pointers and return */
  UNGRAB
  UPDATE
  return Z_OK;
}
#undef GRABBITS
#undef UNGRAB

/* inflate.c */
#undef NEEDBYTE
#undef NEXTBYTE
#define NEEDBYTE {if(z->avail_in==0)return r;r=Z_OK;}
#define NEXTBYTE (z->avail_in--,z->total_in++,*z->next_in++)
#define Z_DEFLATED 8
#define PRESET_DICT 0x20
#define DONE IDONE
#define BAD IBAD
extern "C" int inflate(z_stream *z, int f)
{
  int r = f;    /* to avoid warning about unused f */
  uInt b;

  if (z == Z_NULL || z->state == Z_NULL || z->next_in == Z_NULL)
    return Z_STREAM_ERROR;
  r = Z_BUF_ERROR;
  while (1) switch (z->state->mode)
  {
    case METHOD:
      NEEDBYTE
      if (((z->state->sub.method = NEXTBYTE) & 0xf) != Z_DEFLATED)
      {
        z->state->mode = BAD;
        z->msg = (char*)"unknown compression method";
        z->state->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      if ((z->state->sub.method >> 4) + 8 > z->state->wbits)
      {
        z->state->mode = BAD;
        z->msg = (char*)"invalid window size";
        z->state->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      z->state->mode = FLAG;
    case FLAG:
      NEEDBYTE
      b = NEXTBYTE;
      if (((z->state->sub.method << 8) + b) % 31)
      {
        z->state->mode = BAD;
        z->msg = (char*)"incorrect header check";
        z->state->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      Trace((stderr, "inflate: zlib header ok\n"));
      if (!(b & PRESET_DICT))
      {
        z->state->mode = BLOCKS;
	break;
      }
      z->state->mode = DICT4;
    case DICT4:
      NEEDBYTE
      z->state->sub.check.need = (uLong)NEXTBYTE << 24;
      z->state->mode = DICT3;
    case DICT3:
      NEEDBYTE
      z->state->sub.check.need += (uLong)NEXTBYTE << 16;
      z->state->mode = DICT2;
    case DICT2:
      NEEDBYTE
      z->state->sub.check.need += (uLong)NEXTBYTE << 8;
      z->state->mode = DICT1;
    case DICT1:
      NEEDBYTE
      z->state->sub.check.need += (uLong)NEXTBYTE;
      z->adler = z->state->sub.check.need;
      z->state->mode = DICT0;
      return Z_NEED_DICT;
    case DICT0:
      z->state->mode = BAD;
      z->msg = (char*)"need dictionary";
      z->state->sub.marker = 0;       /* can try inflateSync */
      return Z_STREAM_ERROR;
    case BLOCKS:
      r = inflate_blocks(z->state->blocks, z, r);
      if (r == Z_DATA_ERROR)
      {
        z->state->mode = BAD;
        z->state->sub.marker = 0;       /* can try inflateSync */
        break;
      }
      if (r != Z_STREAM_END)
        return r;
      r = Z_OK;
      inflate_blocks_reset(z->state->blocks, z, &z->state->sub.check.was);
      if (z->state->nowrap)
      {
        z->state->mode = DONE;
        break;
      }
      z->state->mode = CHECK4;
    case CHECK4:
      NEEDBYTE
      z->state->sub.check.need = (uLong)NEXTBYTE << 24;
      z->state->mode = CHECK3;
    case CHECK3:
      NEEDBYTE
      z->state->sub.check.need += (uLong)NEXTBYTE << 16;
      z->state->mode = CHECK2;
    case CHECK2:
      NEEDBYTE
      z->state->sub.check.need += (uLong)NEXTBYTE << 8;
      z->state->mode = CHECK1;
    case CHECK1:
      NEEDBYTE
      z->state->sub.check.need += (uLong)NEXTBYTE;

      if (z->state->sub.check.was != z->state->sub.check.need)
      {
        z->state->mode = BAD;
        z->msg = (char*)"incorrect data check";
        z->state->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      Trace((stderr, "inflate: zlib check ok\n"));
      z->state->mode = DONE;
    case DONE:
      return Z_STREAM_END;
    case BAD:
      return Z_DATA_ERROR;
    default:
      return Z_STREAM_ERROR;
  }
}
#undef DONE
#undef BAD

/* inftrees.c */
#define BMAX 15
#define N_MAX 288
extern "C" int huft_build(uIntf *b, uInt n, uInt s, uIntf *d, uIntf *e, inflate_huft **t, uIntf *m, z_stream *zs)
{

  uInt a;                       /* counter for codes of length k */
  uInt c[BMAX+1];               /* bit length count table */
  uInt f;                       /* i repeats in table every f entries */
  int g;                        /* maximum code length */
  int h;                        /* table level */
  uInt i;              /* counter, current code */
  uInt j;              /* counter */
  int k;               /* number of bits in current code */
  int l;                        /* bits per table (returned in m) */
  uIntf *p;            /* pointer into c[], b[], or v[] */
  inflate_huft *q;              /* points to current table */
  struct inflate_huft_s r;      /* table entry for structure assignment */
  inflate_huft *u[BMAX];        /* table stack */
  uInt v[N_MAX];                /* values in order of bit length */
  int w;               /* bits before this table == (l * h) */
  uInt x[BMAX+1];               /* bit offsets, then code stack */
  uIntf *xp;                    /* pointer into x */
  int y;                        /* number of dummy codes added */
  uInt z;                       /* number of entries in current table */


  /* Generate counts for each bit length */
  p = c;
#define C0 *p++ = 0;
#define C2 C0 C0 C0 C0
#define C4 C2 C2 C2 C2
  C4                            /* clear c[]--assume BMAX+1 is 16 */
  p = b;  i = n;
  do {
    c[*p++]++;                  /* assume all entries <= BMAX */
  } while (--i);
  if (c[0] == n)                /* null input--all zero length codes */
  {
    *t = (inflate_huft *)Z_NULL;
    *m = 0;
    return Z_OK;
  }


  /* Find minimum and maximum length, bound *m by those */
  l = *m;
  for (j = 1; j <= BMAX; j++)
    if (c[j])
      break;
  k = j;                        /* minimum code length */
  if ((uInt)l < j)
    l = j;
  for (i = BMAX; i; i--)
    if (c[i])
      break;
  g = i;                        /* maximum code length */
  if ((uInt)l > i)
    l = i;
  *m = l;


  /* Adjust last length count to fill out codes, if needed */
  for (y = 1 << j; j < i; j++, y <<= 1)
    if ((y -= c[j]) < 0)
      return Z_DATA_ERROR;
  if ((y -= c[i]) < 0)
    return Z_DATA_ERROR;
  c[i] += y;


  /* Generate starting offsets into the value table for each length */
  x[1] = j = 0;
  p = c + 1;  xp = x + 2;
  while (--i) {                 /* note that i == g from above */
    *xp++ = (j += *p++);
  }


  /* Make a table of values in order of bit lengths */
  p = b;  i = 0;
  do {
    if ((j = *p++) != 0)
      v[x[j]++] = i;
  } while (++i < n);


  /* Generate the Huffman codes and for each, make the table entries */
  x[0] = i = 0;                 /* first Huffman code is zero */
  p = v;                        /* grab values in bit order */
  h = -1;                       /* no tables yet--level -1 */
  w = -l;                       /* bits decoded == (l * h) */
  u[0] = (inflate_huft *)Z_NULL;        /* just to keep compilers happy */
  q = (inflate_huft *)Z_NULL;   /* ditto */
  z = 0;                        /* ditto */

  /* go through the bit lengths (k already is bits in shortest code) */
  for (; k <= g; k++)
  {
    a = c[k];
    while (a--)
    {
      /* here i is the Huffman code of length k bits for value *p */
      /* make tables up to required level */
      while (k > w + l)
      {
        h++;
        w += l;                 /* previous table always l bits */

        /* compute minimum size table less than or equal to l bits */
        z = (z = g - w) > (uInt)l ? l : z;      /* table size upper limit */
        if ((f = 1 << (j = k - w)) > a + 1)     /* try a k-w bit table */
        {                       /* too few codes for k-w bit table */
          f -= a + 1;           /* deduct codes from patterns left */
          xp = c + k;
          if (j < z)
            while (++j < z)     /* try smaller tables up to z bits */
            {
              if ((f <<= 1) <= *++xp)
                break;          /* enough codes to use up j bits */
              f -= *xp;         /* else deduct codes from patterns */
            }
        }
        z = 1 << j;             /* table entries for j-bit table */

        /* allocate and link in new table */
        if ((q = (inflate_huft *)ZALLOC
             (zs,z + 1,sizeof(inflate_huft))) == Z_NULL)
        {
          if (h)
            inflate_trees_free(u[0], zs);
          return Z_MEM_ERROR;   /* not enough memory */
        }
#ifdef DEBUG
        inflate_hufts += z + 1;
#endif
        *t = q + 1;             /* link to list for huft_free() */
        *(t = &(q->next)) = Z_NULL;
        u[h] = ++q;             /* table starts after link */

        /* connect to last table, if there is one */
        if (h)
        {
          x[h] = i;             /* save pattern for backing up */
          r.bits = (Byte)l;     /* bits to dump before this table */
          r.exop = (Byte)j;     /* bits in this table */
          r.next = q;           /* pointer to this table */
          j = i >> (w - l);     /* (get around Turbo C bug) */
          u[h-1][j] = r;        /* connect to last table */
        }
      }

      /* set up table entry in r */
      r.bits = (Byte)(k - w);
      if (p >= v + n)
        r.exop = 128 + 64;      /* out of values--invalid code */
      else if (*p < s)
      {
        r.exop = (Byte)(*p < 256 ? 0 : 32 + 64);     /* 256 is end-of-block */
        r.base = *p++;          /* simple code is just the value */
      }
      else
      {
        r.exop = (Byte)e[*p - s] + 16 + 64; /* non-simple--look up in lists */
        r.base = d[*p++ - s];
      }

      /* fill code-like entries with r */
      f = 1 << (k - w);
      for (j = i >> w; j < z; j += f)
        q[j] = r;

      /* backwards increment the k-bit code i */
      for (j = 1 << (k - 1); i & j; j >>= 1)
        i ^= j;
      i ^= j;

      /* backup over finished tables */
      while ((i & ((1 << w) - 1)) != x[h])
      {
        h--;                    /* don't need to update q */
        w -= l;
      }
    }
  }


  /* Return Z_BUF_ERROR if we were given an incomplete table */
  return y != 0 && g != 1 ? Z_BUF_ERROR : Z_OK;
}
