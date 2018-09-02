
/**
 * Converts the libdragon controller_data structure in to an array indexed by button indicating which are pressed.
 * @param input The controller object from libdragon.
 * @param controllerNumber The controller we want to know the buttons from.
 * @out Array of pressed buttons.
 */
void getPressedButtons(const struct controller_data* input, const char controllerNumber, bool* output) {
    output[NoButton] = false;
    output[A] = true && input->c[(int)controllerNumber].A;
    output[B] = true && input->c[(int)controllerNumber].B;
    output[L] = true && input->c[(int)controllerNumber].L;
    output[R] = true && input->c[(int)controllerNumber].R;
    output[Z] = true && input->c[(int)controllerNumber].Z;
    output[Start] = true && input->c[(int)controllerNumber].start;
    output[Up] = true && input->c[(int)controllerNumber].up;
    output[Down] = true && input->c[(int)controllerNumber].down;
    output[Left] = true && input->c[(int)controllerNumber].left;
    output[Right] = true && input->c[(int)controllerNumber].right;
    output[CUp] = true && input->c[(int)controllerNumber].C_up;
    output[CDown] = true && input->c[(int)controllerNumber].C_down;
    output[CLeft] = true && input->c[(int)controllerNumber].C_left;
    output[CRight] = true && input->c[(int)controllerNumber].C_right;
    output[Stick] = true && (input->c[(int)controllerNumber].x || input->c[(int)controllerNumber].y);
}
