#ifndef SCREEN_INCLUDED
#define SCREEN_INCLUDED

#include "state.h"

const unsigned short RESOLUTION_X = 640;
const unsigned short RESOLUTION_Y = 480;

typedef struct {
    unsigned short Top;
    unsigned short Left;
    unsigned short Width;
    unsigned short Height;
} ScreenPosition;

void flushScreen();

/**
 * Get the gameboy screen rectangle based on player number.
 * @param state program state including number of players.
 * @param playerNumber number of a given player.
 * @out output The calculated screen size & position.
 */
void getScreenPosition(const RootState* state, const byte playerNumber, ScreenPosition* output);

#endif
