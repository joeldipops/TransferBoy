#ifndef INIT_INCLUDED
#define INIT_INCLUDED

#include "state.h"

typedef enum {
    InitStart = 0,
    InitNoTpak, InitNoCartridge, InitRequiresExpansionPak,
    InitPending, InitReady, InitLoading, InitLoaded, InitError
} InitState;

/**
 * Waits for a Start Button pressed, then goes and loads a rom.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initLogic(RootState* state, const byte playerNumber);

/**
 * Draws screen for init mode.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initDraw(const RootState* state, const byte playerNumber);

#endif
