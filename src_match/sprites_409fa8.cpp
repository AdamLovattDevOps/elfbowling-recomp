// TScene key handler setters, 0x409fa8-0x409fc8 (old: S24 - these set
// TScene::onKeyDown / onKeyUp; they are not sprite functions).
#include <elf/funcs.h>

extern "C" void fn_409fa8(TScene *sc, SceneKeyCb cb) { sc->onKeyDown = cb; }
extern "C" void fn_409fb8(TScene *sc, SceneKeyCb cb) { sc->onKeyUp = cb; }
