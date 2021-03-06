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

/**
 * Applies Supergameboy colourisation to the pixels in the buffer.
 * @param stateCurrent Super Gameboy data state.
 * @param pixelBuffer Greyscale Gameboy pixel buffer that will be overwritten.
 * @return Error code
 ** 0  Success.
 ** -1 Unknown mask type.
 */
sByte applySGBPalettes(const SuperGameboyState* state, natural* pixelBuffer);
#endif
