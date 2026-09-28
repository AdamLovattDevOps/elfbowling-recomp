// Constructors: TSpriteGroup 0x409844, TScene 0x4098e4.
#include <elf/funcs.h>

TSpriteGroup::TSpriteGroup(const char *nm, TScene *sc, const char *prefix)
{
    count = 0;
    f20 = 0;
    f24 = 0;
    scene = sc;
    fn_402410(name, nm, 0x18);
    for (int i = 0; i < scene->nsprites; i++) {
        TGraphicSprite *s = scene->sprites[i];
        if (fn_40239c(prefix, s->name) == 0)
            fn_4096dc(this, s);
    }
}

// The third argument is the start handler (0x2c, called as SceneStartCb by
// fn_409a6c); the ctor's parameter is spelled SceneCb because every caller
// passes it that way.
TScene::TScene(const char *nm, int fps, SceneStartCb start, SceneCb start2)
{
    fn_401cec();
    period = tmax(4, 1000 / fps);
    started = 0;
    active = 0;
    f10 = 0;
    fn_402410(name, nm, 0xff);
    nsprites = 0;
    nbuttons = 0;
    onKeyDown = 0;
    onKeyUp = 0;
    onStart = start;
    onStart2 = start2;
    retScene = 0;
    onDown = 0;
    onUp = 0;
}
// MATCH 409844 @TSpriteGroup@$bctr$qpxcp6TScenet1
// MATCH 4098e4 @TScene@$bctr$qpxcipqp6TScenec$vpqp6TScene$v
