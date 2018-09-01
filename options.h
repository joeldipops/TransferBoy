#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED
#include "utils.h"

typedef enum { BorderNone } Border;

typedef struct {
    Border SelectedBorder;
    char NumberOfPlayers;
    GbButton ButtonMap[16];
} OptionsHash;

void initialiseOptions(OptionsHash* options);
void optionsLoop(OptionsHash*  options);

#endif
