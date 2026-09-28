// Constructors: TSound 0x40c544, TSoundMgr 0x40d1ac.
// A declared destructor makes the ctor set the EH state word (mov word [ebp-0x14], 8).
#include <elf/funcs.h>

TSound::TSound(const char *nm)
{
    fn_402410(name, nm, 0x18);
    datasize = 0;
    used = 0;
    data = 0;
    g_4555c0 = 0;
    fn_40c1f0(this, nm);
    msec = datasize * 1000 / fmt.nAvgBytesPerSec;
}

TSoundMgr::TSoundMgr()
{
    fn_401cec();
    enabled = waveOutGetNumDevs() > 0;
    nsounds = 0;
    nqueue = 0;
    defA = -1;
    defB = -1;
    seq = 1;
    done = 0;
    endTime = 0;
    loopOwner = 0;
    loopId = -1;
}
// MATCH 40c544 @TSound@$bctr$qpxc
// MATCH 40d1ac @TSoundMgr@$bctr$qv
