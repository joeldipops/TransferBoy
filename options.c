#include "utils.h"
#include "options.h"

void initialiseOptions(OptionsHash* options) {
    options->SelectedBorder = BorderNone;
    options->NumberOfPlayers = 1;

    GbButton* buttons = malloc(sizeof(GbButton) * 16);
    buttons[NoButton] = GbNoButton;
    buttons[A] = GbA;
    buttons[B] = GbB;
    buttons[Up] = GbUp;
    buttons[Down] = GbDown;
    buttons[Left] = GbLeft;
    buttons[Right] = GbRight;

    // Start, Select and SystemMenu should be configurable
    buttons[Start] = GbStart;
    buttons[R] = GbSelect;
    buttons[L] = GbSystemMenu;

    // TODO: Implement stick control.
    buttons[Stick] = GbNoButton;

    buttons[Z] = GbNoButton;
    buttons[CUp] = GbNoButton;
    buttons[CDown] = GbNoButton;
    buttons[CLeft] = GbNoButton;
    buttons[CRight] = GbNoButton;

    memcpy(options->ButtonMap, buttons, sizeof(GbButton) * 16);
}

void optionsLoop(OptionsHash* options) {
    initialiseOptions(options);
}
