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
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playLogic(RootState* state, const byte playerNumber);

/**
 * Draws gameboy screen.
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playDraw(const RootState* state, const byte playerNumber);

/**
 * Does any necessary cleanup after drawing.
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playAfter(RootState* state, const byte playerNumber);

#endif
