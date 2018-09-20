#ifndef SCREEN_INCLUDED
#define SCREEN_INCLUDED

#include "state.h"

const unsigned short RESOLUTION_X = 640;
const unsigned short RESOLUTION_Y = 480;

typedef struct {
    natural Top;
    natural Left;
    natural Width;
    natural Height;
} ScreenPosition;

/**
 * Resets all screen buffers to a known state.
 * @param state program state.
 */
void flushScreen(RootState* state);

/**
 * Gets the RDP module ready to render a new texture.
 * @param frame identifies frame to render to.
 */
void prepareRdpForTexture(const display_context_t frame);

/**
 * Get the gameboy screen rectangle based on player number.
 * @param state program state including number of players.
 * @param playerNumber number of a given player.
 * @out output The calculated screen size & position.
 */
void getScreenPosition(const RootState* state, const byte playerNumber, ScreenPosition* output);

#endif
