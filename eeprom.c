#include "eeprom.h"
#include <libdragon.h>

static unsigned long cursorPosition = 0;

/**
 * Gets the next available eeprom block.
 * @return open eeprom block.
 */
unsigned long getEepromCursorPosition() {
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
    while(index < stream->Size) {
        //logAndPause("index=%d", index);
        natural start = index * 8;
        natural end = start + 8;
        byte block[8] = {0};
        if (end <= stream->Size) {
            memcpy(block, stream + start, 8);
        } else {
            // pad out the rest of the block with zeroes.
            byte diff = end - stream->Size;
            memset(block, 0, 8);
            memcpy(block, stream + start, 8 - diff);
        }

        //logAndPause("start=%d end=%d blockNumber=%d", start, end, blockNumber);

        eeprom_write(blockNumber + index, block);
        index++;
    }

    lastWrittenBlock = lastWrittenBlock + index;
    return 0;
}


