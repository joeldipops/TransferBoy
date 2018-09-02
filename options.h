#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED
#include "utils.h"

typedef enum { BorderNone } Border;

typedef struct {
    Border SelectedBorder;
    char NumberOfPlayers;
    GbButton ButtonMap[16];
} OptionsHash;

/**
 * Set initial state of user-configurable options.
 * @out The options struct to populate.
 */
void initialiseOptions(OptionsHash* options);

void optionsLoop(OptionsHash*  options);

#endif
