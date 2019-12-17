#ifndef PROGRESS_BAR_INCLUDED
#define PROGRESS_BAR_INCLUDED

#include <libdragon.h>

/**
 * Sets up handlers to display a progress bar for the cartridge load.
 * @param Address of the global state so it can be referenced in a callback.
 * @param Number identifying player that will load a cartridge.
 */
void startLoadProgressTimer(RootState* rootState, const byte playerNumber);

/**
 * Cleans up handlers for displaying the cartridge load progress bar.
 */
void closeLoadProgressTimer(const byte playerNumber);

#endif