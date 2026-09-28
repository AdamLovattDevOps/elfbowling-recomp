/* Internal declarations shared by the shim's .c files. */
#ifndef PORT_SHIM_H
#define PORT_SHIM_H

#include "windows.h"

/* Ensure SDL (with the given subsystem flags) is initialised; 0 = ok. */
int port_sdl_init(unsigned flags);

/* Log a call to a stubbed API. */
#define PORT_STUB(name, ...) port_log("stub " name ": " __VA_ARGS__)

#endif
