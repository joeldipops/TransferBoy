#ifndef MENU_INCLUDED
#define MENU_INCLUDED

#include "state.h"

/**
 * Handles the pause menu for given player.
 * @param playerNumber player in menu mode.
 */
void menuLogic(const byte playerNumber);

/**
 * Displays the pause menu for given player.
 * @param playerNumber player in menu mode.
 */
void menuDraw(const byte playerNumber);

#endif
