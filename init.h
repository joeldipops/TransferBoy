#ifndef INIT_INCLUDED
#define INIT_INCLUDED

#include "state.h"

typedef enum {
    InitStart = 0,
    InitReady, InitLoading, InitLoaded, InitError, InitRestarting, InitChanging
} InitState;

/**
 * Waits for a Start Button pressed, then goes and loads a rom.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initLogic(const byte playerNumber);

/**
 * Draws screen for init mode.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initDraw(const byte playerNumber);

#endif
