// Sound manager: free/load sounds, 0x40cf24-0x40d058 (EH frames from delete/new of TSound).
#include <elf/funcs.h>

extern "C" void fn_40cf24(TSoundMgr *p)
{
    for (int i = 0; i < p->nsounds; i++) {
        if (p->sounds[i].snd && !p->sounds[i].keep) {
            delete p->sounds[i].snd;
            p->sounds[i].snd = 0;
        }
    }
    while (p->nsounds > 0 && p->sounds[p->nsounds - 1].snd == 0)
        p->nsounds--;
}

extern "C" int fn_40cfb0(TSoundMgr *p, const char *name, char keep)
{
    int id = fn_40ce60(p, name);
    if (id == -1) {
        id = fn_40cebc(p);
        if (id != -1) {
            p->sounds[id].snd = new TSound(name);
            p->sounds[id].keep = keep;
        }
    } else {
        p->sounds[id].keep |= keep;
    }
    return id;
}
