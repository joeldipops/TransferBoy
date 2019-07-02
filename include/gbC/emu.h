#ifndef EMU_H
#define EMU_H

#include "types.h"
#include "../../state.h"
#include "player_input.h"

/**
 * Initialize the emulator state of the gameboy. This state belongs to the
 * emulator, not the state of the emulated hardware.
 */
void emu_init(GbState *s);
void emu_step(PlayerState* state);
void emu_process_inputs(GbState *s, struct player_input *input_state);

#endif
