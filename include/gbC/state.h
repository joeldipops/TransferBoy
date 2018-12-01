#ifndef STATE_H
#define STATE_H

#include "types.h"
#include <stddef.h>

void print_rom_header_info(u8* rom);
int state_new_from_rom(struct gb_state *s, u8 *rom, size_t rom_size);
void state_add_bios(struct gb_state *s, u8 *bios, size_t bios_size);
void init_emu_state(struct gb_state *s);

/* Store/load dump of entire state (breaks when datastructures change). */
int state_save(struct gb_state *s, u8 **ret_state_buf, size_t *ret_state_size);
int state_load(struct gb_state *s, u8 *state_buf, size_t state_buf_size);

/* Store/load dump for external (battery backed) RAM. */
int state_save_extram(struct gb_state *s, u8 **ret_state_buf,
        size_t *ret_state_size);
int state_load_extram(struct gb_state *s, u8 *state_buf, size_t state_buf_size);

#endif
