#include "superGameboy.h"
#include "state.h"

static const byte SGB_PACKET_LENGTH = 16;

/**
 * Handles a multiplayer request, used to determine if the SGB is connected.
 * We of course want to signal that it is.
 */
void mlt_req(PlayerState* state) {
    state->SGBState.PlayersMode = state->SGBState.Buffer[1];
    state->SGBState.CurrentController = 0;
    // Set bits 4 & 5 to 0.
    state->EmulationState.io_buttons = state->EmulationState.io_buttons & 0xCF;
}

sByte palpq(PlayerState* state, const byte p, const byte q) {
    if (state->SGBState.NumberOfPackets != 1) {
        return -1;
    }

    // Start at 1 because the first byte is the command.
    for (byte i = 1; i < SGB_PACKET_LENGTH; i+=2) {
        natural colour = state->SGBState.Buffer[i] + (state->SGBState.Buffer[i+1] << 8);

        if (i <=  8) {
            // Next 8 bytes are the first palette.
            state->SGBState.Palettes[p][(i-1)/2] = colour;
        } else if (i <= 14) {
            // Next 6 bytes are the second palette.
            state->SGBState.Palettes[q][(i-7)/2] = colour;
        }
        // Final byte is padding
    }

    // The first colour should be shared across all palettes (for some reason)
    for (byte i = 0; i < 4; i++) {
        state->SGBState.Palettes[i][0] = state->SGBState.Palettes[p][0];
    }

    return 0;
}

void pal01(PlayerState* state) {
    palpq(state, 0, 1);
}

void sound(PlayerState* state) {

}

void chr_trn(PlayerState* state) {

}

void data_snd(PlayerState* state) {

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
        case SGBTransferDataToSGB:
            data_snd(state);
        default: break;
    }
}

/**
 * Adds the next bit to the supergameboy transfer buffer and increments pointers.
 * @param state The sgb data we're juggling.
 * @private
 */
void pushBit(SuperGameboyState* state) {

    if (state->AwaitingStopBit) {
        if (state->PendingBit == 0) {
            state->AwaitingStopBit = false;
        }

        return;
    }

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
            state->AwaitingStopBit = true;
            state->PacketPointer++;
            state->BytePointer = 0;
        }
    }
}

/**
 * Gets SuperGameboy system ready to accept data again.
 * @param state The state to reset.
 */
void resetSGBTransfer(SuperGameboyState* state) {
    state->BitBuffer = 0;
    state->BitPointer = 0;
    state->AwaitingStopBit = false;
    state->Buffer = 0;
    state->BytePointer = 0;
    state->PacketPointer = 0;
    state->CurrentCommand = SGBNoop;
    state->IsTransferring = false;
    state->NumberOfPackets = 0;
    state->PendingBit = 0;
    state->HasPendingBit = 0;
    state->JoypadRequestResolved = false;
    if (state->HasData) {
        free(state->Buffer);
        state->HasData = false;
    }
}

/**
 * Resets SGB back to initial state
 * @param state state to reset.
 */
void resetSGBState(SuperGameboyState* state) {
    resetSGBTransfer(state);
    for (byte i = 0; i < 4; i++) {
        for (byte j = 0; j < 4; j++) {
            state->Palettes[i][j] = 0;
        }
    }

    state->PlayersMode = 0;
    state->CurrentController = 0;
}


static const byte JOYPAD_IDS[4] = {0x0F, 0x0E, 0x0D, 0x0C};

/**
 * Adjusts emulation to perform addition super gameboy functions.
 * @param state The state of the player we're processing.
 */
void performSGBFunctions(PlayerState* state) {
    // Increment the joypad register for multiplayer
    if (state->SGBState.PlayersMode > 0) {
        // Check if controller ID is being requested
        if ((state->EmulationState.io_buttons & 0x30) == 0x30) {
            if (!state->SGBState.JoypadRequestResolved) {
                if (state->SGBState.CurrentController < 3) {
                    state->SGBState.CurrentController++;
                } else {
                    state->SGBState.CurrentController = 0;
                }
                state->EmulationState.io_buttons = state->EmulationState.io_buttons | JOYPAD_IDS[state->SGBState.CurrentController];
                state->SGBState.JoypadRequestResolved = true;
            }
        } else {
            state->SGBState.JoypadRequestResolved = false;
        }
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
            if (sgb->NumberOfPackets >= 1 && sgb->PacketPointer >= sgb->NumberOfPackets && !sgb->AwaitingStopBit) {
                executeSgbCommand(state);
                resetSGBTransfer(sgb);
            }
        } else if (sgb->IsTransferring && bits == 0x01) {
            sgb->HasPendingBit = 1;
            sgb->PendingBit = 1;
        } else if (sgb->IsTransferring && bits == 0x02) {
            sgb->HasPendingBit = 1;
            sgb->PendingBit = 0;
        } else if (bits == 0x00) {
            resetSGBTransfer(sgb);
        }
    }
}
