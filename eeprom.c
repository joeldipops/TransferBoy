#include "eeprom.h"
#include <libdragon.h>

static byte cursorPosition = 0;
static const byte EEPROM_BLOCKS = 64;
static const byte BLOCK_SIZE = 8;

/**
 * Gets the next available eeprom block.
 * @return open eeprom block.
 */
byte getEepromCursorPosition() {
    if (cursorPosition >= EEPROM_BLOCKS) {
        cursorPosition = 0;
    }
    return cursorPosition;
}

/**
 * Writes to eeprom
 * @param block The 8-byte block of memory to start with.
 * @param data The data to write.
 * @return 0 if successful, otherwise an sub-zero error code.
*/
sByte writeToEeprom(const byte blockNumber, const ByteArray* stream) {
    if (!eeprom_present()) {
        return -1;
    }

    natural index = 0;

    while(index <= stream->Size / BLOCK_SIZE) {
        natural start = index * BLOCK_SIZE;
        natural end = start + BLOCK_SIZE;
        byte* block = calloc(BLOCK_SIZE, sizeof(byte));
        if (end <= stream->Size) {
            memcpy(block, stream->Data + start, BLOCK_SIZE);
        } else {
            // pad out the rest of the block with zeroes.
            byte diff = end - stream->Size;
            memcpy(block, stream->Data + start, BLOCK_SIZE - diff);
        }

        eeprom_write(blockNumber + index, block);
        free(block);

        index++;
    }

    cursorPosition += index;
    return 0;
}


