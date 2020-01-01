#ifndef PLAY_INCLUDED
#define PLAY_INCLUDED

#include "state.h"

/**
 * Sets all emulation functions for this player back to a clean slate.
 * @param state The player to reset.
 */
void resetPlayState(PlayerState* state);

/**
 * Handles gameboy emulation.
 * @param playerNumber player in play mode.
 */
void playLogic(const byte playerNumber);

/**
 * Draws gameboy screen.
 * @param playerNumber player in play mode.
 */
void playDraw(const byte playerNumber);

/**
 * Does any necessary cleanup after drawing.
 * @param playerNumber player in play mode.
 */
void playAfter(const byte playerNumber);

#endif
