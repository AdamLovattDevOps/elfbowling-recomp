// Borland C++ 5.3 RTL generator (SOURCE/RTL/SOURCE/MATH/RAND.C), for bcb_rtl.h.
#include <bcb_rtl.h>

#include <stdint.h>

static uint32_t s_lo = 1, s_hi = 0;     // __seed_t Seed = { 1, 0 }

extern "C" {

void bcb_srand(unsigned seed)
{
    s_lo = seed;
    s_hi = 0;
}

int bcb_rand(void)
{
    s_lo = 0x015A4E35u * s_lo + 1u;
    return (int)(s_lo >> 16) & 0x7fff;
}

// _lrand's assembly multiplies the 64-bit seed by 0x0000015A00004E35 (the
// 0x015A half of the multiplier lands in the high dword), adds 1, and
// returns the high dword's low 31 bits.
long bcb_lrand(void)
{
    uint64_t s = ((uint64_t)s_hi << 32 | s_lo) * 0x0000015A00004E35ull + 1u;
    s_lo = (uint32_t)s;
    s_hi = (uint32_t)(s >> 32);
    return (long)(s_hi & 0x7fffffffu);
}

} // extern "C"
