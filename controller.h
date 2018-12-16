#ifndef CONTROLLER_INCLUDED
#define CONTROLLER_INCLUDED

#include "core.h"
#include <libdragon.h>

#define N64_BUTTON_COUNT 23
const sByte STICK_DEADZONE = 42;

typedef enum {
    NoButton  = 0,
    A, B, L, R, Z, Start, DUp, DDown, DLeft, DRight, CUp, CDown, CLeft, CRight, StickUp, StickDown, StickLeft, StickRight,
    Up, Down, Left, Right // Either stick or d-pad.  StickLeft & DRight will cancel each other out.
} N64Button;

typedef enum {
    GbNoButton = 0,
    GbA, GbB, GbUp, GbDown, GbLeft, GbRight, GbStart, GbSelect, GbSystemMenu
} GbButton;

typedef struct controller_data N64ControllerState;

/**
 * Converts the libdragon controller_data structure in to an array indexed by button indicating which are pressed.
 * @param input The controller object from libdragon.
 * @param controllerNumber The controller we want to know the buttons from.
 * @out Array of pressed buttons.
 */
void getPressedButtons(const N64ControllerState* input, const byte controllerNumber, bool* output);

#endif
