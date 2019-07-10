#ifndef STATE_H
#define STATE_H

#include "types.h"
#include <stddef.h>

sByte loadCartridge(GbState* s, GameBoyCartridge* cartridge);
sByte applyBios(GbState* s, byte* bios);


#endif
