#include "link.h"

#include <string.h>
#include "state.h"
#include "include/gbc_bundle.h"
#include "logger.h"

// bit 3 - 0000 1000
const natural SERIAL_INTERRUPT_MAP = 0x08;

/**
 * Checks if gameboys serial control byte is indicating there is data to send.
 * @param state gameboy state.
 * @return true if gameboy is ready to transfer a byte.
 */
bool isRequestingTransfer(const RootState* state) {
    if (state->PlayerCount < 2) {
        return false;
    }

    bool readyToSend = false;

    for (byte i = 0; i < state->PlayerCount; i++) {
        // If a game isn't ready, don't do anything yet.
        // May be able to send even if this bit isn't set?
        if (!state->Players[i].EmulationState.IsLinkTransferAvailable) {
            return false;
        }

        // At least one game must be hosting the connection.
        if (state->Players[i].EmulationState.IsLinkClockExternal) {
            readyToSend = true;
        }
    }

    return readyToSend;
}

/**
 * Sends data between gameboy states, emulating serial link cable transfer.
 * @param states the gameboys to send data between.
 */
void exchangeLinkData(GbState* states[2]) {
    byte datum = states[0]->LinkData;
    states[0]->LinkData = states[1]->LinkData;
    states[1]->LinkData = datum;

    states[0]->IsLinkTransferAvailable = false;
    states[1]->IsLinkTransferAvailable = false;

    // Prepare the serial interrupt
    states[0]->LinkInterrupt = true;
    states[1]->LinkInterrupt = true;
}


