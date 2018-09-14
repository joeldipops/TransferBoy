#include "eeprom.h"
#include <libdragon.h>

static int cursorPosition = 0;

/**
 * Gets the next available eeprom block.
 * @return open eeprom block.
 */
int getEepromCursorPosition() {
    return cursorPosition;
}

/**
 * Writes to eeprom
 * @param block The 8-byte block of memory to start with.
 * @param data The data to write.
 * @return 0 if successful, otherwise an sub-zero error code.
*/
char writeToEeprom(const int blockNumber, const ByteArray* stream) {
    if (!eeprom_present()) {
        return -1;
    }

    natural index = 0;

    while(index <= stream->Size / 8) {
        natural start = index * 8;
        natural end = start + 8;
        byte* block = malloc(sizeof(byte) * 8);
        if (end <= stream->Size) {
            memcpy(block, stream->Data + start, 8);
        } else {
            // pad out the rest of the block with zeroes.
            byte diff = end - stream->Size;
            memcpy(block, stream->Data + start, 8 - diff);
        }

        eeprom_write(blockNumber + index, block);
        free(block);

        index++;
    }

    cursorPosition += index;
    return 0;
}


