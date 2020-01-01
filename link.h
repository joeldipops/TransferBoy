#ifndef LINK_INCLUDED
#define LINK_INCLUDED

#include "core.h"
#include "state.h"

/**
 * Checks if gameboy serial control bytes are indicating there is data to send and receive.
 * @return true if gameboy is ready to transfer a byte.
 */
bool isRequestingTransfer();

/**
 * Sends data between gameboy states, emulating serial link cable transfer.
 * @param states the gameboys to send data between.
 */
void exchangeLinkData(GbState* states[2]);

#endif
