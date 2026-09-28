// Sound manager: queue a sound (fn_40cbf0). The destination-first store order
// ("mov ecx,[q]; mov eax,[arg]; mov [ecx],eax") needs q to be a non-enregisterable
// local: `TSoundQueueEntry * volatile q`. Taking &q or storing inside a try block
// gives the same order.
#include <elf/funcs.h>

extern "C" char fn_40cbf0(TSoundMgr *p, void *owner, TGraphicSprite *s, int id, int prio, SoundDoneCb cb)
{
    char r = 0;
    if (id >= 0 && id < p->nsounds && p->sounds[id].snd != 0 && p->nqueue < 20) {
        r = 1;
        TSoundQueueEntry * volatile q = &p->queue[p->nqueue++];
        q->owner = owner;
        q->sprite = s;
        q->seq = p->seq++;
        q->snd = p->sounds[id].snd;
        q->prio = prio;
        q->cb = cb;
        q->id = id;
        if (p->nqueue == 1)
            r = fn_40c960(p, q->snd);
    }
    return r;
}
