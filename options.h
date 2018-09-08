#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED

#include "state.h"

/**
 * Displays the options menu for given player.
 * @param state Program state.
 * @param playerNumber player in options mode.
 */
void optionsDraw(const RootState*  state, char playerNumber);

/**
 * Handles the options menu for given player.
 * @param state Program state.
 * @param playerNumber player in options mode.
 */
void optionsLogic(RootState*  state, char playerNumber);

#endif
