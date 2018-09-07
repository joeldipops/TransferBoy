#ifndef PLAY_INCLUDED
#define PLAY_INCLUDED

#include "core.h"

/**
 * Handles gameboy emulation.
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playLogic(RootState* state, const unsigned char playerNumber);

/**
 * Draws gameboy screen.
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playDraw(const RootState* state, const unsigned char playerNumber);

#endif
