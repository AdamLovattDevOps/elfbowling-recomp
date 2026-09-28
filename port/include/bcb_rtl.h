// bcb_rtl.h - the Borland C++ 5.3 RTL names the game uses that the host libc
// lacks or defines differently. Force-include it in the native game build
// (`-include bcb_rtl.h`); see docs/VCL_PORT.md.
//
//   rand / srand     Borland's generator (seed * 0x015A4E35 + 1, bits 16..30),
//                    RAND_MAX 0x7FFF, so the game draws the same sequences
//                    as the original for the same seed
//   random(n)        Borland: _lrand() % n (64-bit seed, see below)
//   randomize()      srand(time(NULL))
//   min / max        stdlib.h's C++ templates (the if/else form)
//   __abs__          the compiler intrinsic behind abs()
//   stricmp, strcmpi, strnicmp   strcasecmp / strncasecmp
// Implementation: port/vcl/rtl.cpp (in libvcl.a).
#ifndef PORT_BCB_RTL_H
#define PORT_BCB_RTL_H

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif
int bcb_rand(void);
void bcb_srand(unsigned seed);
long bcb_lrand(void);
#ifdef __cplusplus
}
#endif

// Function-like, so `using ::rand;` in <cstdlib> and the like are untouched.
#undef RAND_MAX
#define RAND_MAX 0x7FFFU
#define rand() bcb_rand()
#define srand(seed) bcb_srand(seed)
#define _lrand() bcb_lrand()
#define random(num) bcb_random(num)
#define randomize() bcb_srand((unsigned)time(NULL))

static inline int bcb_random(int num) { return num ? (int)(bcb_lrand() % num) : 0; }
static inline int __abs__(int x) { return x < 0 ? -x : x; }
static inline int stricmp(const char *a, const char *b) { return strcasecmp(a, b); }
static inline int strcmpi(const char *a, const char *b) { return strcasecmp(a, b); }
static inline int strnicmp(const char *a, const char *b, size_t n) { return strncasecmp(a, b, n); }

#ifdef __cplusplus
#ifndef __MINMAX_DEFINED
#define __MINMAX_DEFINED
template <class T> inline const T &min(const T &t1, const T &t2)
{
    if (t1 < t2)
        return t1;
    else
        return t2;
}
template <class T> inline const T &max(const T &t1, const T &t2)
{
    if (t1 > t2)
        return t1;
    else
        return t2;
}
#endif
#endif

#endif
