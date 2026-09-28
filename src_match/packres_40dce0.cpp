// TPackedResources ctor 0x40dce0 / dtor 0x40dd90, TSaveCasts ctor 0x40e1b0, TSaveParms ctor 0x40e37c.
// A declared destructor makes the ctor set EH state 8 first (TSaveParms has none).
#include <elf/funcs.h>
#include <string.h>

TPackedResources::TPackedResources(void *unused, const char *nm)
{
    fn_401cec();
    g_45c41c = fn_40dc04;
    outbuf = 0;
    loaded = 0;
    inbuf = 0;
    casts = 0;
    fn_402410(name, nm, 0x80);
    int ok = fn_40d2c8(this);
    if (ok)
        g_455524 = this;
}

TPackedResources::~TPackedResources()
{
    if (loaded) {
        for (int i = 0; i < count; i++) {
            if (!loaded[i])
                fn_401cf4("Warning: did not use resource %s\n", entries[i].name);
        }
    }
    if (loaded)
        fn_402380(loaded, "~TPackedResources");
    if (outbuf)
        fn_402380(outbuf, "~TPackedResources");
    if (inbuf)
        fn_402380(inbuf, "~TPackedResources");
    if (casts)
        delete casts;
    g_455524 = 0;
}

TSaveCasts::TSaveCasts()
{
    fn_401cec();
    count = 0;
    hdrsize = 0x28;
    strcpy(magic, "The NStorm Cannon Rules!");
    entries = 0;
    data = 0;
}

TSaveParms::TSaveParms()
{
    fn_401cec();
    count = 0;
    hdrsize = 0x30;
    strcpy(magic, "NV us, you strange little monkey!");
    entries = 0;
    data = 0;
}
// MATCH 40dce0 @TPackedResources@$bctr$qpvpxc
// MATCH 40e1b0 @TSaveCasts@$bctr$qv
// MATCH 40e37c @TSaveParms@$bctr$qv
// MATCH 40dd90 @TPackedResources@$bdtr$qv
