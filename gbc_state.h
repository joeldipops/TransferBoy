#ifndef STATE_H
#define STATE_H

#include "types.h"
#include <stddef.h>

static const sByte LOAD_ERR_TOO_SMALL = -1;
static const sByte LOAD_ERR_TOO_LARGE = -2;
static const sByte LOAD_ERR_UNSUPPORTED = -3;
static const sByte LOAD_ERR_RTC_UNAVAILABLE = -4;

sByte loadCartridge(GbState* s, GameBoyCartridge* cartridge);
sByte applyBios(GbState* s, ByteArray* bios);


#endif
