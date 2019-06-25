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
        SerialControl controlByte = {};
        memset(&controlByte, state->Players[i].EmulationState.LinkControl, 1);

        // If a game isn't ready, don't do anything yet.
        // May be able to send even if this bit isn't set?
        if (!state->Players[i].EmulationState.IsTransferToStart) {
            return false;
        }

        // At least one game must be hosting the connection.
        if (state->Players[i].EmulationState.IsClockExternal) {
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

    SerialControl* ctrl1;
    SerialControl* ctrl2;
    ctrl1 = (SerialControl*)&states[0]->LinkControl;
    ctrl2 = (SerialControl*)&states[1]->LinkControl;
    ctrl1->isAvailable = false;
    ctrl2->isAvailable = false;

    // Prepare the serial interrupt
    states[0]->InterruptFlags = states[0]->InterruptFlags | SERIAL_INTERRUPT_MAP;
    states[1]->InterruptFlags = states[1]->InterruptFlags | SERIAL_INTERRUPT_MAP;
}


