#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED
#include "constants.h"

typedef enum { BorderNone } Border;

typedef struct {
    Border border;
    natural players;
    Button startButton;
    Button selectButton;
} OptionsHash;

void optionsLoop(OptionsHash*);

#endif
