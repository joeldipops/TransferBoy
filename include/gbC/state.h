#ifndef STATE_H
#define STATE_H

#include "types.h"
#include <stddef.h>

void print_rom_header_info(u8* rom);
int state_new_from_rom(GbState *s, u8 *rom, size_t rom_size);
void state_add_bios(GbState *s, u8 *bios, size_t bios_size);
void init_emu_state(GbState *s);

/* Store/load dump of entire state (breaks when datastructures change). */
int state_save(GbState *s, u8 **ret_state_buf, size_t *ret_state_size);
int state_load(GbState *s, u8 *state_buf, size_t state_buf_size);

/* Store/load dump for external (battery backed) RAM. */
int state_save_extram(GbState *s, u8 **ret_state_buf,
        size_t *ret_state_size);
int state_load_extram(GbState *s, u8 *state_buf, size_t state_buf_size);

#endif
