#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED

#include "state.h"

typedef enum { OptionsAudio, OptionsStart, OptionsSelect, OptionsMenu, OptionsEnd } OptionType;

/**
 * Displays the options menu for given player.
 * @param playerNumber player in options mode.
 */
void optionsDraw(const byte playerNumber);

/**
 * Handles the options menu for given player.
 * @param playerNumber player in options mode.
 */
void optionsLogic(byte playerNumber);

#endif
