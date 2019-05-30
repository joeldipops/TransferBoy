#ifndef LINK_INCLUDED
#define LINK_INCLUDED

#include "core.h"
#include "state.h"
#include "include/gbc_bundle.h"

typedef struct {
    bool isAvailable:1;
    bool pad1:1;
    bool pad2:1;
    bool pad3:1;
    bool pad4:1;
    bool pad5:1;
    bool isClockFast:1;
    bool isInternalClock:1;
} SerialControl;

/**
 * Checks if gameboy serial control bytes are indicating there is data to send and receive.
 * @param state program state.
 * @return true if gameboy is ready to transfer a byte.
 */
bool isRequestingTransfer(const RootState* state);

/**
 * Sends data between gameboy states, emulating serial link cable transfer.
 * @param states the gameboys to send data between.
 */
void exchangeLinkData(GbState* states[2]);

#endif
