#include "superGameboy.h"
#include "sgbDefs.h"
#include "state.h"

static const byte SGB_PACKET_LENGTH = 16;
static const byte HORIZONTAL_TILES = 20;
static const byte VERTICAL_TILES = 18;


/**
 * Put the emulator in a state where each time the joypad register
 * receives a certain signal, it updates with the status of a different controller.
 * This command is used to signal that the SBG is connect, we of course want to signal that it is.
 * @param state Program state including supergameboy request data.
 */
void mlt_req(PlayerState* state) {
    state->SGBState.PlayersMode = state->SGBState.Buffer[1];
    state->SGBState.CurrentController = 0;
}

/**
 * Sets colours of two of the four palettes.
 * @param state Program state including supergameboy request data.
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
 * @param state Program state including supergameboy request data.
 */
void pal01(PlayerState* state) {
    palpq(state, 0, 1);
}

/**
 * Set the colours of palettes 2 and 3
 * @param state Program state including supergameboy request data.
 */
void pal23(PlayerState* state) {
    palpq(state, 2, 3);
}

/**
 * Set the colours of palettes 0 and 3
 * @param state Program state including supergameboy request data.
 */
void pal03(PlayerState* state) {
    palpq(state, 0, 3);
}

/**
 * Set the colours of palettes 1 and 2
 * @param state Program state including supergameboy request data.
 */
void pal12(PlayerState* state) {
    palpq(state, 1, 2);
}

/**
 * Covers the screen so no-one sees anything ugly while we're getting SGB stuff ready.
 * @param state Program state including supergameboy request data.
 * @return Error code
 ** 0  - Success
 ** -1 - Incorrect number of packets
 ** -2 - Invalid value.
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
 * @param state Program state including supergameboy request data.
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
 * @param state Program state including supergameboy request data.
 * @return Error code
 ** 0  Success
 ** -1 Not Implemented
 */
sByte pct_trn(PlayerState* state) {
    // TODO, need to read the data from the gameboy MMU, not the lcd_pixbuf
    return -1;
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
 * @param state Program state including supergameboy request data.
 */
void pal_pri(PlayerState* state) {
    // On actual hardware 0 means use custom palette,
    // 1 the palette set in code.
    // We'll just use the grey-scale palette when priority is off.
    state->SGBState.HasPriority = state->SGBState.Buffer[1] & 0x01;
}

/**
 * Specifies a palette for a number of tiles in sequence.
 * @return Error code
 * 0  - Success
 * -1 - Too many tiles in sequence.
 */
sByte attr_chr(PlayerState* state) {
    byte startX = state->SGBState.Buffer[1] & 0x1F;
    byte startY = state->SGBState.Buffer[2] & 0x1F;

    natural itemCount = ((state->SGBState.Buffer[4] & 0x01) << 8) + state->SGBState.Buffer[3];

    if (itemCount > 360) {
        return -1;
    }

    enum { Horizontal = 0, Vertical = 1 } axis = state->SGBState.Buffer[5] & 0x01;

    // TODO won't work if we start in a position that means we have to wrap around half way through a byte.

    natural byteIndex = 6;
    byte byteCount = itemCount / 4;
    if (axis == Horizontal) {
        for (byte y = startY; y < VERTICAL_TILES && byteIndex < byteCount; y++) {
            for (
                byte x = startX;
                x < HORIZONTAL_TILES && byteIndex < byteCount;
                x += 4
            ) {
                byte palettes = state->SGBState.Buffer[byteIndex];

                state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x] = (palettes & 0xC0) >> 6;
                state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x + 1] = (palettes & 0x30) >> 4;
                state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x + 2] = (palettes & 0x0C) >> 2;
                state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x + 3] = (palettes & 0x03);

                byteIndex++;
            }

            // After the first iteration, start at the beginning of the next line
            startX = 0;
        }
    } else {
        // Vertical lines
        for (byte x = startX; x < HORIZONTAL_TILES && byteIndex < byteCount; x++) {
            for (
                byte y = startY;
                y < VERTICAL_TILES && byteIndex < byteCount;
                y += 4
            ) {
                byte palettes = state->SGBState.Buffer[byteIndex];

                state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x] = (palettes & 0xC0) >> 6;
                state->SGBState.TilePalettes[(y + 1) * HORIZONTAL_TILES + x] = (palettes & 0x30) >> 4;
                state->SGBState.TilePalettes[(y + 2) * HORIZONTAL_TILES + x] = (palettes & 0x0C) >> 2;
                state->SGBState.TilePalettes[(y + 3) * HORIZONTAL_TILES + x] = (palettes & 0x03);

                byteIndex++;
            }
            startY = 0;
        }
    }

    return 0;
}

/**
 * Divides the gameboy screen in to three sections with different palettes.
 * One of the sections is a line one sprite wide/high.
 * @param state Program state including supergameboy request data.
 */
void attr_div(PlayerState* state) {
    // Gonna assume if you have an attr_div, it overwrites whatever else you had.
    enum { Vertical, Horizontal } axis = (state->SGBState.Buffer[1] & 0x40) >> 6;
    byte linePaletteId = (state->SGBState.Buffer[1] & 0x30) >> 4;
    byte beforePaletteId = (state->SGBState.Buffer[1] & 0x0C) >> 2;
    byte afterPaletteId = state->SGBState.Buffer[1] & 0x03;
    byte lineNumber = state->SGBState.Buffer[2] & 0x1F;

   for (byte y = 0; y < VERTICAL_TILES; y++) {
        for (byte x = 0; x < HORIZONTAL_TILES; x++) {
            byte div = (axis == Vertical) ? x : y;

            if (axis == Vertical) {
                if (div < lineNumber) {
                    state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x] = beforePaletteId;
                } else if (div == lineNumber) {
                    state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x] = linePaletteId;
                } else {
                    state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x] = afterPaletteId;
                }
            }
        }
    }
}


/**
 * Sets palettes to vertical or horizontal lines of tiles.
 * @param state Program state including supergameboy request data.
 */
void attr_lin(PlayerState* state) {
    byte lineCount = state->SGBState.Buffer[1];

    enum { Vertical, Horizontal } axis = 0;

    for (byte i = 0; i < lineCount; i++) {
        byte datum = state->SGBState.Buffer[i + 2];
        axis = (datum & 0x80) >> 7;
        byte paletteId = (datum & 0x60) >> 5;
        byte lineNumber = datum & 0x1F;

        if (axis == Horizontal) {
            byte y = lineNumber;
            for (byte x = 0; x < HORIZONTAL_TILES; x++) {
                state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x] = paletteId;
            }
        } else {
            byte x = lineNumber;
            for (byte y = 0; y < VERTICAL_TILES; y++) {
                state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x] = paletteId;
            }
        }
    }
}

/**
 * Sets palettes for retangular areas of the screen.  Can set for tiles inside, outside and on the border of the specified rectangle.
 * @param state Program state including supergameboy request data.
 * @return Error code
 ** 0  - Success
 ** -1 - Invalid number of blocks.
 */
sByte attr_blk(PlayerState* state) {
    byte blockCount = state->SGBState.Buffer[1] & 0x1F;

    if (blockCount > 0x12) {
        return -1;
    }

    byte max = blockCount * 6 + 2;
    for (byte i = 2; i < max; i += 6) {
        // bits 0,1,2 of byte 1 are flags for what to replace.
        bool replaceInside = (state->SGBState.Buffer[i] & 0x01);
        bool replaceLine = (state->SGBState.Buffer[i] & 0x02) >> 1;
        bool replaceOutside = (state->SGBState.Buffer[i] & 0x04) >> 2;

        if (!(replaceOutside || replaceLine || replaceInside)) {
            continue;
        }

        // Pull the palette ids from byte 2
        byte outsidePalette = (state->SGBState.Buffer[i+1] & 0x30) >> 4;
        byte linePalette = (state->SGBState.Buffer[i+1] & 0x0C) >> 2;
        byte insidePalette = state->SGBState.Buffer[i+1] & 0x03;

        // And the corners of the rectangle from these 4 bytes.
        byte x1 = state->SGBState.Buffer[i+2] & 0x1F;
        byte y1 = state->SGBState.Buffer[i+3] & 0x1F;
        byte x2 = state->SGBState.Buffer[i+4] & 0x1F;
        byte y2 = state->SGBState.Buffer[i+5] & 0x1F;

        for (byte y = 0; y < VERTICAL_TILES; y++) {
            for (byte x = 0; x < HORIZONTAL_TILES; x++) {
                if (
                    replaceLine && (
                        x == x1 ||
                        x == x2 ||
                        y == y1 ||
                        y == y2
                    )
                ) {
                    state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x] = linePalette;
                } else if (
                    replaceInside && (
                        x > x1 &&
                        x < x2 &&
                        y > y1 &&
                        y > y2
                    )
                ) {
                    state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x] = insidePalette;
                } else if (replaceOutside) {
                    state->SGBState.TilePalettes[y * HORIZONTAL_TILES + x] = outsidePalette;
                }
            }
        }
    }
    return 0;
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
        case SGBSetPalette23:
            pal23(state);
            break;
        case SGBSetPalette03:
            pal03(state);
            break;
        case SGBSetPalette12:
            pal12(state);
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
        //case SGBTransferOverlay:
        //  pct_trn(state);
        //    break;
        case SGBDividePalettes:
            attr_div(state);
            break;
        case SGBTransferPacketToSgb:
            data_snd(state);
            break;
        case SGBApplyPaletteToTiles:
            attr_chr(state);
            break;
        case SGBApplyPaletteToLines:
            attr_lin(state);
            break;
        case SGBApplyPaletteToBlocks:
            attr_blk(state);
            break;
        default: break;//logAndPause("SGB Command: %02x", state->SGBState.CurrentCommand); break;
    }
}

/**
 * Adds the next bit to the supergameboy transfer buffer and increments pointers.
 * @param state The sgb data we're juggling.
 * @private
 */
void pushBit(SuperGameboyState* state) {
    // Super Gameboy transfers 1 bit at a time, controlled by the  two 'out' bits in the
    // Joypad In/Out register.
    // Packetes are 16 bytes long + 1 final "stop" bit.

    // The first byte contains the command and the number of packets to expect.

    // If we have received a packets, don't accept a new command or packet until we've received the stop bit.
    if (state->AwaitingStopBit) {
        if (state->PendingBit == 0) {
            state->AwaitingStopBit = false;
        }

        return;
    }

    // Add the bit to a single byte buffer.
    state->BitBuffer |= (state->PendingBit << (state->BitPointer));
    state->HasPendingBit = 0;
    state->BitPointer++;

    // If the single byte buffer is full, add it to the buffer and increment.
    if (state->BitPointer == 8) {
        // If it's the first packet, set us up.
        if (state->BytePointer == 0 && state->PacketPointer == 0) {
            state->CurrentCommand = (state->BitBuffer & 0xF8) >> 3;
            state->NumberOfPackets = state->BitBuffer & 0x07;
            state->Buffer = calloc(state->NumberOfPackets, sizeof(byte) * SGB_PACKET_LENGTH);
            state->HasBuffer = true;
        }

        state->Buffer[SGB_PACKET_LENGTH * state->PacketPointer + state->BytePointer] = state->BitBuffer;
        state->BytePointer++;
        state->BitBuffer = 0;
        state->BitPointer = 0;

        // The ByteBuffer also fill up to a full packet.  Increment to the next Packet,
        // But we don't accept the next data until we've received the stop bit.
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
    if (state->HasBuffer) {
        free(state->Buffer);
        state->HasBuffer = false;
    }
}

/**
 * Resets SGB back to initial state
 * @param state state to reset.
 */
void resetSGBState(SuperGameboyState* state) {
    // TODO Get rid of HasBuffer & HasRamData, we can just set to 0 and check that.
    resetSGBTransfer(state);
    for (byte i = 0; i < 4; i++) {
        for (byte j = 0; j < 4; j++) {
            state->Palettes[i][j] = 0;
        }
    }

    for (natural i = 0; i < 360; i++) {
       state->TilePalettes[i] = 0;
    }

    state->IsWindowFrozen = false;
    state->HasPriority = false;
    state->PlayersMode = 0;
    state->CurrentController = 0;
    state->SnesRamBlockCount = 0;
    if (state->HasRamData) {
        free(state->RamBlocks);
        state->RamBlocks = 0;
        state->HasRamData = false;
    }

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
 * Applies Supergameboy colourisation to the pixels in the buffer.
 * @param stateCurrent Super Gameboy data state.
 * @param pixelBuffer Greyscale Gameboy pixel buffer
 * @out pixels Pixels having gone through sgb transformations.
 */
void generateSGBPixels(const SuperGameboyState* state, const natural* pixelBuffer, uInt* pixels) {
    if (state->HasPriority) {
        for (natural y = 0; y < GB_LCD_HEIGHT; y++) {
            for (natural x = 0; x < GB_LCD_WIDTH; x++) {
                natural index = x + y * GB_LCD_WIDTH;

                natural tile = (x / 8) + (y / 8 * HORIZONTAL_TILES);
                natural colour = state->Palettes[state->TilePalettes[tile]][pixelBuffer[index]];

                pixels[index] = massageColour(colour);
            }
        }
    } else {
        // If priority not set, use greyscale.
        for (natural y = 0; y < GB_LCD_HEIGHT; y++) {
            for (natural x = 0; x < GB_LCD_WIDTH; x++) {
                natural index = x + y * GB_LCD_WIDTH;
                pixels[index] = MONOCHROME_PALETTE[pixelBuffer[index]];
            }
        }
    }
}
