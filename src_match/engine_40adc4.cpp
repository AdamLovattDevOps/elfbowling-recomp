// Area (TStage dirty-area rect) constructor at 0x40adc4. It is out of line,
// so the TStage ctor builds its area arrays with _vector_new_ldtc_. Borland
// still emits the EH frame (__InitExceptBlockLDTC) for an empty ctor.
#include <elf/funcs.h>

Area::Area()
{
}
// MATCH 40adc4 @Area@$bctr$qv
