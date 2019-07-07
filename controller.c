#include <string.h>
#include "core.h"
#include "controller.h"


const sByte STICK_DEADZONE = 42;

/**
 * Converts the libdragon controller_data structure in to an array indexed by button indicating which are pressed.
 * @param input The controller object from libdragon.
 * @param controllerNumber The controller we want to know the buttons from.
 * @out Array of pressed buttons.
 */
void getPressedButtons(const N64ControllerState* input, const byte controllerNumber, bool* output) {
    output[NoButton] = false;
    output[A] = true && input->c[controllerNumber].A;
    output[B] = true && input->c[controllerNumber].B;
    output[L] = true && input->c[controllerNumber].L;
    output[R] = true && input->c[controllerNumber].R;
    output[Z] = true && input->c[controllerNumber].Z;
    output[Start] = true && input->c[controllerNumber].start;
    output[DUp] = true && input->c[controllerNumber].up;
    output[DDown] = true && input->c[controllerNumber].down;
    output[DLeft] = true && input->c[controllerNumber].left;
    output[DRight] = true && input->c[controllerNumber].right;
    output[CUp] = true && input->c[controllerNumber].C_up;
    output[CDown] = true && input->c[controllerNumber].C_down;
    output[CLeft] = true && input->c[controllerNumber].C_left;
    output[CRight] = true && input->c[controllerNumber].C_right;

    sByte x = input->c[controllerNumber].x;
    if (x < STICK_DEADZONE * -1) {
        output[StickLeft] = true;
    } else if (x > STICK_DEADZONE) {
        output[StickRight] = true;
    } else {
        output[StickLeft] = false;
        output[StickRight] = false;
    }

    sByte y = input->c[controllerNumber].y;
    if (y < STICK_DEADZONE * -1) {
        output[StickDown] = true;
    } else if (y > STICK_DEADZONE) {
        output[StickUp] = true;
    } else {
        output[StickUp] = false;
        output[StickDown] = false;
    }

    output[Up] = (output[StickUp] || output[DUp]) && !(output[StickDown] || output[DDown]);
    output[Down] = (output[StickDown] || output[DDown]) && !(output[StickUp] || output[DUp]);
    output[Left] = (output[StickLeft] || output[DLeft]) && !(output[StickRight] || output[DRight]);
    output[Right] = (output[StickRight] || output[DRight]) && !(output[StickLeft] || output[DLeft]);
}