#ifndef EEPROM_DEFINED
#define EEPROM_DEFINED

#include "core.h"

/**
 * Gets the next available eeprom block.
 * @return open eeprom block.
 */
byte getEepromCursorPosition();

/**
 * Writes to eeprom
 * @param block The 8-byte block of memory to start with.
 * @param data The data to write.
 * @return 0 if successful, otherwise an sub-zero error code.
*/
sByte writeToEeprom(const byte blockNumber, const ByteArray* data);

#endif
