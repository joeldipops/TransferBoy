#ifndef CONTROLLER_INCLUDED
#define CONTROLLER_INCLUDED

#include "core.h"
#include <libdragon.h>

typedef enum { NoButton, A, B, L, R, Z, Start, Up, Down, Left, Right, CUp, CDown, CLeft, CRight, Stick } N64Button;
typedef enum { GbNoButton, GbA, GbB, GbUp, GbDown, GbLeft, GbRight, GbStart, GbSelect, GbSystemMenu } GbButton;
typedef struct controller_data N64ControllerState;

/**
 * Converts the libdragon controller_data structure in to an array indexed by button indicating which are pressed.
 * @param input The controller object from libdragon.
 * @param controllerNumber The controller we want to know the buttons from.
 * @out Array of pressed buttons.
 */
void getPressedButtons(const N64ControllerState* input, const byte controllerNumber, bool* output);

#endif
