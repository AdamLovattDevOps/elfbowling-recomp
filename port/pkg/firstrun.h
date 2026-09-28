/* pkg/firstrun.c: find, verify (SHA-256) and keep a copy of the user's original Elf Bowling.exe. */
#ifndef ELFBOWL_FIRSTRUN_H
#define ELFBOWL_FIRSTRUN_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
/* given: a path from the command line (or NULL); user_dir: an extra folder to look in (iOS
 * Documents, Android external files), or NULL. On success out is the path to load (the copy in
 * app storage when it could be made) and 0 is returned; -1 after telling the user what is missing. */
int elfbowl_first_run(const char *given, const char *user_dir, char *out, size_t outsz);
/* The platform file picker: 0 and a path, -1 cancelled, -2 no picker on this system. */
int elfbowl_pick_file(char *out, size_t outsz);
#ifdef __cplusplus
}
#endif
#endif
