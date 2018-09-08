#ifndef MENU_INCLUDED
#define MENU_INCLUDED

#include "state.h"

/**
 * Handles the pause menu for given player.
 * @param state Program state.
 * @param playerNumber player in menu mode.
 */
void menuLogic(RootState* state, const unsigned char playerNumber);

/**
 * Displays the pause menu for given player.
 * @param state Program state.
 * @param playerNumber player in menu mode.
 */
void menuDraw(const RootState* state, const unsigned char playerNumber);

#endif
