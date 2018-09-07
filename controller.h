#ifndef CONTROLLER_INCLUDED
#define CONTROLLER_INCLUDED

#include <libdragon.h>

/**
 * Converts the libdragon controller_data structure in to an array indexed by button indicating which are pressed.
 * @param input The controller object from libdragon.
 * @param controllerNumber The controller we want to know the buttons from.
 * @out Array of pressed buttons.
 */
void getPressedButtons(const struct controller_data* input, const char controllerNumber, bool* output);

#endif
