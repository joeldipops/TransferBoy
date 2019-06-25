#ifndef STATE_H
#define STATE_H

#include "types.h"
#include <stddef.h>

int state_new_from_rom(GbState *s, u8 *rom, size_t rom_size);
void state_add_bios(GbState *s, u8 *bios, size_t bios_size);
int state_load_extram(GbState *s, u8 *state_buf, size_t state_buf_size);

#endif
