// Constructor of TButtonSprite (0x3cc bytes), 0x409100 (old: TalkActor).
#include <elf/funcs.h>

TButtonSprite::TButtonSprite(TStage *stage, const char *name, const char *up, const char *down, TPoint pos)
    : TGraphicSprite(name, stage, pos)
{
    fn_408f84(this);
    fn_406724(this, up);
    fn_406724(this, down);
    onPress = 0;
    onRelease = 0;
    onEnter = 0;
    onExit = 0;
    hoverSnd = -1;
    clickSnd = -1;
    type = 3;
}
// MATCH 409100 @TButtonSprite@$bctr$qp6TStagepxct2t28tagPOINT
