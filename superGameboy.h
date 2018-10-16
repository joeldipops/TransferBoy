#ifndef SUPER_GAMEBOY_INCLUDED
#define SUPER_GAMEBOY_INCLUDED

#include "state.h"

/**
 * Puts SuperGameboy data back to their initial state.
 * @param state The state to reset.
 */
void resetSGBState(SuperGameboyState* state);

void processSGBData(PlayerState* state);

#endif
