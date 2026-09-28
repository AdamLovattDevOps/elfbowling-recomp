// Sound manager (queue of wave sounds), 0x40ca94-0x40d1ac.
#include <elf/funcs.h>

extern "C" void fn_40ca94(TSoundMgr *p)
{
    if (p->playing)
        fn_40c804(p, 1);
}

extern "C" int fn_40cab4(TSoundMgr *p)
{
    int r = -1;
    if (fn_40cddc(p))
        r = p->queue[0].prio;
    return r;
}

extern "C" char fn_40cae0(TSoundMgr *p, void *owner, TGraphicSprite *s, int id, int prio, SoundDoneCb cb)
{
    char r = 0;
    if (id >= 0 && id < p->nsounds && fn_40cab4(p) <= prio) {
        fn_40cd80(p);
        r = fn_40cbf0(p, owner, s, id, prio, cb);
    }
    return r;
}

extern "C" char fn_40cb34(TSoundMgr *p, void *owner, TGraphicSprite *s, const char *name, int prio, SoundDoneCb cb)
{
    char r = 0;
    int id = fn_40cdec(p, name);
    if (id >= 0)
        r = fn_40cae0(p, owner, s, id, prio, cb);
    return r;
}

extern "C" void fn_40cb7c(TSoundMgr *p, void *owner, const char *name)
{
    p->loopOwner = owner;
    p->loopId = fn_40cdec(p, name);
    if (!fn_40cddc(p))
        fn_40cae0(p, owner, 0, p->loopId, 1, 0);
}

extern "C" void fn_40cbd0(TSoundMgr *p, void *unused)
{
    p->loopOwner = 0;
    p->loopId = -1;
}

extern "C" void fn_40ccd0(TSoundMgr *p, void *owner)
{
    if (p->nqueue > 0 && p->queue[0].owner == owner)
        fn_40ca94(p);
    int i = 0;
    int j = 0;
    for (; i < p->nqueue; i++) {
        if (p->queue[i].owner != owner) {
            if (j < i)
                p->queue[j] = p->queue[i];
            j++;
        }
    }
    p->nqueue = j;
}

extern "C" void fn_40cd80(TSoundMgr *p)
{
    if (p->nqueue > 0) {
        p->nqueue = tmin(1, p->nqueue);
        fn_40ca94(p);
        p->nqueue = 0;
    }
}

extern "C" char fn_40cddc(TSoundMgr *p)
{
    return p->playing;
}

extern "C" int fn_40cdec(TSoundMgr *p, const char *name)
{
    int idx = -1;
    for (int i = 0; i < p->nsounds && idx == -1; i++) {
        if (p->sounds[i].snd != 0 && rtl_stricmp(p->sounds[i].snd->name, name) == 0)
            idx = i;
    }
    if (idx == -1)
        fn_401f30("Could not find sound ", name);
    return idx;
}

extern "C" int fn_40ce60(TSoundMgr *p, const char *name)
{
    int idx = -1;
    for (int i = 0; i < p->nsounds && idx == -1; i++) {
        if (p->sounds[i].snd != 0 && rtl_stricmp(p->sounds[i].snd->name, name) == 0)
            idx = i;
    }
    return idx;
}

extern "C" int fn_40cebc(TSoundMgr *p)
{
    int idx = -1;
    for (int i = 0; i < p->nsounds && idx == -1; i++) {
        if (p->sounds[i].snd == 0)
            idx = i;
    }
    if (idx == -1 && p->nsounds < 100) {
        p->sounds[p->nsounds].snd = 0;
        idx = p->nsounds++;
    }
    return idx;
}

extern "C" void fn_40d058(TSoundMgr *p, const char *name, int arg)
{
    p->defA = fn_40cdec(p, name);
    p->defAarg = arg;
}

extern "C" void fn_40d080(TSoundMgr *p, const char *name, int arg)
{
    p->defB = fn_40cdec(p, name);
    p->defBarg = arg;
}

extern "C" char fn_40d0a8(TSoundMgr *p, void *owner, TGraphicSprite *s, int id, int prio, int dur, SoundDoneCb cb)
{
    char r = fn_40cae0(p, owner, s, id, prio, cb);
    if (r) {
        dur = dur == 0 ? 160 : dur;
        s->period = dur;
        fn_407748(s);
    }
    return r;
}

extern "C" char fn_40d100(TSoundMgr *p, void *owner, TGraphicSprite *s, const char *name, int prio, int dur, SoundDoneCb cb)
{
    char r = 0;
    int id = fn_40cdec(p, name);
    if (id >= 0)
        r = fn_40d0a8(p, owner, s, id, prio, dur, cb);
    return r;
}

extern "C" void fn_40d148(TSoundMgr *p)
{
    if (!p->enabled && p->endTime > 0) {
        int t = fn_4024bc(0);
        if (t >= p->endTime) {
            fn_40c740();
            p->endTime = 0;
        }
    }
    if (p->done)
        fn_40c804(p, 0);
}
