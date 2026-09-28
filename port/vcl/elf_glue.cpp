// The Classes::Rect spellings that return the game's own rect types
// (include/elf/types.h ERect, with a user copy constructor, so it is returned
// through memory and must be the same type on both sides). Part of the native
// game build (needs -I../include); `make -C port elf-glue` compile-checks it.
#include <vcl/vcl.h>
#include <elf/types.h>

static ERect erect(int l, int t, int r, int b)
{
    ERect q;
    q.left = l;
    q.top = t;
    q.right = r;
    q.bottom = b;
    return q;
}

ERect __fastcall Classes_TRect(int l, int t, int r, int b) { return erect(l, t, r, b); }

// 0x43a8dc Classes::Rect as funcs.h declares it
extern "C" ERect __fastcall fn_43a8dc(int l, int t, int r, int b) { return erect(l, t, r, b); }
