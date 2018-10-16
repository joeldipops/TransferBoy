#include "superGameboy.h"
#include "state.h"

static const byte SGB_PACKET_LENGTH = 16;

/**
 * Handles a multiplayer request, used to determine if the SGB is connected.
 * We of course want to signal that it is.
 */
void mlt_req(PlayerState* state) {
    // Last four bits of the joypad register indicate joypad id. 1110 is joypad 2.
    //state->EmulationState.io_buttons = 0x0E;
    state->EmulationState.io_buttons = 0x3E;
}

void pal01(PlayerState* state) {

}

void sound(PlayerState* state) {

}

void chr_trn(PlayerState* state) {

}

/**
 * Executes one of sgb commands, if implemented.
 * @param state The state of the player to execute the command against.
 * @private
 */
void executeSgbCommand(PlayerState* state) {
    switch(state->SGBState.CurrentCommand) {
        case SGBSetPalette01:
            pal01(state);
            break;
        case SGBGenerateSound:
            sound(state);
            break;
        case SGBRequestMultiplayer:
            mlt_req(state);
            break;
        case SGBTransferCharacter:
            chr_trn(state);
            break;
        default: break;
    }
}

/**
 * Adds the next bit to the supergameboy transfer buffer and increments pointers.
 * @param state The sgb data we're juggling.
 * @private
 */
void pushBit(SuperGameboyState* state) {
    state->BitBuffer |= (state->PendingBit << (state->BitPointer));
    state->HasPendingBit = 0;
    state->BitPointer++;

    if (state->BitPointer == 8) {

        // If it's the first packet, set us up.
        if (state->BytePointer == 0 && state->PacketPointer == 0) {
            state->CurrentCommand = (state->BitBuffer & 0xF8) >> 3;
            state->NumberOfPackets = state->BitBuffer & 0x07;
            state->Buffer = calloc(state->NumberOfPackets, sizeof(byte) * SGB_PACKET_LENGTH);
            state->HasData = true;
        }

        state->Buffer[SGB_PACKET_LENGTH * state->PacketPointer + state->BytePointer] = state->BitBuffer;
        state->BytePointer++;
        state->BitBuffer = 0;
        state->BitPointer = 0;

        if (state->BytePointer >= SGB_PACKET_LENGTH) {
            state->PacketPointer++;
            state->BytePointer = 0;
        }
    }
}

/**
 * Puts SuperGameboy data back to their initial state.
 * @param state The state to reset.
 */
void resetSGBState(SuperGameboyState* state) {
    state->BitBuffer = 0;
    state->BitPointer = 0;
    state->Buffer = 0;
    state->BytePointer = 0;
    state->PacketPointer = 0;
    state->CurrentCommand = SGBNoop;
    state->IsTransferring = false;
    state->NumberOfPackets = 0;
    state->PendingBit = 0;
    state->HasPendingBit = 0;
    if (state->HasData) {
        free(state->Buffer);
        state->HasData = false;
    }

}

/**
 * Checks whether there is new Supergameboy data to process, and carries out any sgb commands.
 * @param state The state of the player we're processing.
 */
void processSGBData(PlayerState* state) {
    SuperGameboyState* sgb = &state->SGBState;

    byte bits = (state->EmulationState.io_buttons & 0x30) >> 4;

    // 0x00 Ends the transfer
    // 0x03 Starts the transfer and confirms that the last bit should be added to the buffer.
    // 0x01 sets an ON/1 bit pending the next 0x03
    // 0x02 sets an OFF/0 bit pending the next 0x03

    if (!sgb->IsTransferring) {
        if (bits == 0x03) {
            sgb->IsTransferring = true;
        }
    } else {
        if (bits == 0x03 && sgb->HasPendingBit) {
            pushBit(sgb);
            if (sgb->NumberOfPackets >= 1 && sgb->PacketPointer >= sgb->NumberOfPackets) {
                executeSgbCommand(state);
                resetSGBState(sgb);
            }
        } else if (sgb->IsTransferring && bits == 0x01) {
            sgb->HasPendingBit = 1;
            sgb->PendingBit = 1;
        } else if (sgb->IsTransferring && bits == 0x02) {
            sgb->HasPendingBit = 1;
            sgb->PendingBit = 0;
        } else if (bits == 0x00) {
            resetSGBState(sgb);
        }
    }
}
