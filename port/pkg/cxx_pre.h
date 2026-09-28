/* Force-included before bcb_rtl.h in the libstdc++ builds (pkg.mk: Linux, Android): bcb_rtl.h
 * defines rand() as a macro, and libstdc++'s <algorithm> calls std::rand() in random_shuffle.
 * Pulling the standard headers in first keeps them out of the macro's reach. */
#ifdef __cplusplus
#include <cstdlib>
#include <algorithm>
#include <random>
#endif
