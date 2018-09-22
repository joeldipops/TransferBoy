#include "link.h"
#include "include/gbc_bundle.h"

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
        memset(&controlByte, state->Players[i].EmulationState.io_serial_control, 1);

        // If a game isn't ready, don't do anything yet.
        // May be able to send even if this bit isn't set?
        if (!controlByte.isAvailable) {
            return false;
        }

        // At least one game must be hosting the connection.
        if (controlByte.isInternalClock) {
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
    byte datum = states[0]->io_serial_data;
    states[0]->io_serial_data = states[1]->io_serial_data;
    states[1]->io_serial_data = datum;

    SerialControl* ctrl1;
    SerialControl* ctrl2;
    ctrl1 = (SerialControl*)&states[0]->io_serial_control;
    ctrl2 = (SerialControl*)&states[1]->io_serial_control;
    ctrl1->isAvailable = false;
    ctrl2->isAvailable = false;

    // Prepare the serial interrupt
    states[0]->interrupts_request = states[0]->interrupts_request | SERIAL_INTERRUPT_MAP;
    states[1]->interrupts_request = states[1]->interrupts_request | SERIAL_INTERRUPT_MAP;
}


