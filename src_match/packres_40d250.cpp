// TPackedResources member 0x40d250, matched as a real C++ member (this at
// [ebp+8], cdecl). As a free function the compiler orders the loads for
// this->arr[i] / i < this->n differently.
#include <elf/funcs.h>

int TPackedResources::fn_40d250()
{
    PackHdr *h = hdr;
    count = h->count;
    entries = hdr->entries;
    int mx = 0;
    for (int i = 0; i < count; i++)
        mx = tmax_lt(entries[i].size, mx);
    return mx;
}
// MATCH 40d250 @TPackedResources@fn_40d250$qv
