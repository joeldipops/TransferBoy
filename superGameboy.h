#ifndef SUPER_GAMEBOY_INCLUDED
#define SUPER_GAMEBOY_INCLUDED

#include "state.h"
#include "sgbDefs.h"

/**
 * Puts SuperGameboy data back to their initial state.
 * @param state The state to reset.
 */
void resetSGBState(SuperGameboyState* state);

/**
 * Checks whether there is new Supergameboy data to process, and carries out any sgb commands.
 * @param state The state of the player we're processing.
 */
void processSGBData(PlayerState* state);

/**
 * Adjusts emulation to perform addition super gameboy functions.
 * @param state The state of the player we're processing.
 */
void performSGBFunctions(PlayerState* state);


void generateSGBPixels(const SuperGameboyState* state, const natural* pixelBuffer, uInt* pixels);
#endif
