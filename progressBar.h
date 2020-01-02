#ifndef PROGRESS_BAR_INCLUDED
#define PROGRESS_BAR_INCLUDED

#include <libdragon.h>

/**
 * Sets up handlers to display a progress bar for the cartridge load.
 * @param playerNumber identifying player that will load a cartridge.
 */
void startLoadProgressTimer(const byte playerNumber);

/**
 * Cleans up handlers for displaying the cartridge load progress bar.
 */
void closeLoadProgressTimer(const byte playerNumber);

#endif