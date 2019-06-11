#ifndef EMU_H
#define EMU_H

#include "types.h"
#include "../../state.h"
#include "player_input.h"

void emu_step(PlayerState* state);
void emu_step_frame(PlayerState* state);
void emu_process_inputs(GbState *s, struct player_input *input_state);

#endif
