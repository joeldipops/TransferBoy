#include "controller.h"

/**
 * Converts the libdragon controller_data structure in to an array indexed by button indicating which are pressed.
 * @param input The controller object from libdragon.
 * @param controllerNumber The controller we want to know the buttons from.
 * @out Array of pressed buttons.
 */
void getPressedButtons(const struct controller_data* input, const byte controllerNumber, bool* output) {
    // A problem with the emulator (or my shitty code?) seems to mean that controller 2 always has every button pressed.
    // Let's leave them idle for now
    if (controllerNumber > 0) {
        memset(output, 0x00, 16);
        return;
    }

    output[NoButton] = false;
    output[A] = true && input->c[controllerNumber].A;
    output[B] = true && input->c[controllerNumber].B;
    output[L] = true && input->c[controllerNumber].L;
    output[R] = true && input->c[controllerNumber].R;
    output[Z] = true && input->c[controllerNumber].Z;
    output[Start] = true && input->c[controllerNumber].start;
    output[Up] = true && input->c[controllerNumber].up;
    output[Down] = true && input->c[controllerNumber].down;
    output[Left] = true && input->c[controllerNumber].left;
    output[Right] = true && input->c[controllerNumber].right;
    output[CUp] = true && input->c[controllerNumber].C_up;
    output[CDown] = true && input->c[controllerNumber].C_down;
    output[CLeft] = true && input->c[controllerNumber].C_left;
    output[CRight] = true && input->c[controllerNumber].C_right;
    output[Stick] = true && (input->c[controllerNumber].x || input->c[controllerNumber].y);
}
