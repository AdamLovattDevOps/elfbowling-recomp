// TSoundMgr: pop the finished sound and start the next, 0x40c880 (member).
#include <elf/funcs.h>

char TSoundMgr::fn_40c880(char flag)
{
    char r = 0;
    if (nqueue > 0) {
        nqueue--;
        for (int i = 0; i < nqueue; i++)
            queue[i] = queue[i + 1];
        if (!flag && nqueue > 0)
            r = fn_40c960(this, queue[0].snd);
    }
    if (!flag && nqueue == 0 && loopId >= 0)
        fn_40cbf0(this, loopOwner, 0, loopId, 1, 0);
    return r;
}
// MATCH 40c880 @TSoundMgr@fn_40c880$qc
