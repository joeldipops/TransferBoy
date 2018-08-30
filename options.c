#include "options.h"

void optionsLoop(OptionsHash* options) {
    options->border = BorderNone;
    options->players = 1;
    options->startButton = Start;
    options->selectButton = R;
};
