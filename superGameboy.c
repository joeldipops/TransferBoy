#include "superGameboy.h"
#include "state.h"

static const byte SGB_PACKET_LENGTH = 16;

/**
 * Put the emulator in a state where each time the joypad register
 * receives a certain signal, it updates with the status of a different controller.
 * This command is used to signal that the SBG is connect, we of course want to signal that it is.
 * @param state Progam state including supergameboy request data.
 */
void mlt_req(PlayerState* state) {
    state->SGBState.PlayersMode = state->SGBState.Buffer[1];
    state->SGBState.CurrentController = 0;
}

/**
 * Sets colours of two of the four palettes.
 * @param state Progam state including supergameboy request data.
 * @param p Id of the first palette.
 * @param q Id of the other palette.
 * @return Error code
 ** 0  Success.
 ** -1 Data received in incorrect format.
 */
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

/**
 * Set the colours of palettes 0 and 1
 * @param state Progam state including supergameboy request data.
 */
void pal01(PlayerState* state) {
    palpq(state, 0, 1);
}

/**
 * Covers the screen so no-one sees anything ugly while we're getting SGB stuff ready.
 * @param state Progam state including supergameboy request data.
 * @return Error code or 0 if successful
 ** -1: Incorrect number of packets
 ** -2: Invalid value.
 */
sByte mask_en(PlayerState* state) {
    if (state->SGBState.NumberOfPackets != 1) {
        return -1;
    }

    if (state->SGBState.Buffer[1] > 3) {
        return -2;
    }

    natural colour;
    switch (state->SGBState.Buffer[1] & 0x03) {
        // Mask by leaving the screen as-is until unmasked
        case 1:
            state->SGBState.IsWindowFrozen = true;
            return 0;
        // Paint the entire screen black
        case 2:
            colour = 0x8000;
            break;
        // Paint the entire screen with the colour shared between palettes.
        case 3:
            colour = state->SGBState.Palettes[0][0];
            break;
        // Don't mask anymore.
        case 0:
        default:
            state->SGBState.IsWindowFrozen = false;
            return 0;
    }

    for (byte i = 0; i < 4; i++) {
        for (byte j = 0; j < 4; j++) {
            state->SGBState.Palettes[i][j] = colour;
        }
    }

    return 0;
}

/**
 * Transfer 128 sprites from the cartridge to memory.
 * These are mainly used to draw borders, but can be used to overlay and colourise the screen as well.
 * @param state Progam state including supergameboy request data.
 */
void chr_trn(PlayerState* state) {
    natural index = 0;
    // There are 256 characters in total, but we can only transfer one at a time,
    // So this bit determines whether it's the first or second 128.
    if(state->SGBState.Buffer[1] & 0x01) {
        index = 128 * 32;
    }

    // The data is lifted from the screen buffer.
    // Usually when this happens, the screen is masked with mask_en, so it looks blank
    // but actually it's filled with 8x8 sprites.

    // lcd_pixbuf is probably not the right place to read from.  probably need to get it from the Gameboy buffers rather than
    // gbC's wrapper.
    memcpy(state->SGBState.SpriteData + index, state->EmulationState.emu_state->lcd_pixbuf, 128 * 32);
}


/**
 * Transfer 1024 tiles and 3 palettes from cartridge to memory.
 * The data is read from the gameboy screen buffer.
 * @param state Progam state including supergameboy request data.
 */
sByte pct_trn(PlayerState* state) {
    // TODO, need to read the data from the gameboy MMU, not the lcd_pixbuf
    return 0;
}

/**
 * Adds to memory data sent from the GB destined for what it thinks is SNES WRAM
 * @return Error code
 ** 0  - Success
 ** -1 - Too many bytes
 */
sByte data_snd(PlayerState* state) {
    natural address = (state->SGBState.Buffer[2] << 8) | state->SGBState.Buffer[1];
    byte bank = state->SGBState.Buffer[3];
    byte byteCount = state->SGBState.Buffer[4];

    if (byteCount > 11) {
        return -1;
    }

    SnesRamBlock* block = 0;

    // Check if there's already data at this 'address' and if so, use it.
    for(byte i = 0; i < state->SGBState.SnesRamBlockCount; i += sizeof(SnesRamBlock)) {
        SnesRamBlock* candidate = (SnesRamBlock*)(state->SGBState.RamBlocks + i);

        if (candidate->Address == address && candidate->Bank == bank) {
            block = candidate;
            break;
        }
    }

    // If there's not already data at this address push something onto the list.
    if (!block) {
        state->SGBState.SnesRamBlockCount++;

        state->SGBState.RamBlocks = realloc(state->SGBState.RamBlocks, sizeof(SnesRamBlock) * state->SGBState.SnesRamBlockCount);

        block = state->SGBState.RamBlocks + sizeof(SnesRamBlock) * (state->SGBState.SnesRamBlockCount - 1);

        block->Address = address;
        block->Bank = bank;
    }

    // byte count may have changed, so update it.
    block->ByteCount = byteCount;

    // Copy the data out of the buffer.
    memcpy(block->Data, &state->SGBState.Buffer[5], block->ByteCount);

    return 0;
}

/**
 * Sets the palette priority.
 * @param state Progam state including supergameboy request data.
 */
void pal_pri(PlayerState* state) {
    // On actual hardware 0 means use custom palette,
    // 1 the palette set in code.
    // We'll just use the grey-scale palette when priority is off.
    state->SGBState.HasPriority = state->SGBState.Buffer[1] & 0x01;
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
        case SGBRequestMultiplayer:
            mlt_req(state);
            break;
        case SGBTransferCharacter:
            chr_trn(state);
            break;
        case SGBMaskWindow:
            mask_en(state);
            break;
        case SGBSetPalettePriority:
            pal_pri(state);
            break;
        case SGBTransferOverlay:
            pct_trn(state);
            break;
        default: logAndPause("SGB Command: %02x", state->SGBState.CurrentCommand); break;
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
    state->IsWindowFrozen = false;
    state->HasPriority = false;
    state->PlayersMode = 0;
    state->CurrentController = 0;
    state->SnesRamBlockCount = 0;
    state->RamBlocks = 0;
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

/**
 *
 * @param stateCurrent Super Gameboy data state.
 * @param pixelBuffer Greyscale Gameboy pixel buffer
 * @out pixels Pixels having gone through sgb transformations.
 */
void generateSGBPixels(const SuperGameboyState* state, const natural* pixelBuffer, uInt* pixels) {
    // TODO
    if (state->HasPriority) {
        for (natural y = 0; y < GB_LCD_HEIGHT; y++) {
            for (natural x = 0; x < GB_LCD_WIDTH; x++) {
                natural index = x + y * GB_LCD_WIDTH;
                // TODO
                pixels[index] = massageColour(state->Palettes[0][pixelBuffer[index]]);
            }
        }
    } else {
        for (natural y = 0; y < GB_LCD_HEIGHT; y++) {
            for (natural x = 0; x < GB_LCD_WIDTH; x++) {
                natural index = x + y * GB_LCD_WIDTH;
                pixels[index] = MONOCHROME_PALETTE[pixelBuffer[index]];
            }
        }
    }
}
