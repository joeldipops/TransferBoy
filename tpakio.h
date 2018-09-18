#ifndef TPAKIO_INLCUDED
#define TPAKIO_INCLUDED
#include "core.h"

/**
 * Determines if the expansion pak is plugged in.
 * @return true if the N64 Memory Expansion Pak is available.
 */
bool isExpansionPakInserted();

bool isCartridgeSizeOk(const byte controllerNumber);

/**
 * Determines if there is a transfer pak inserted in to the given controller.
 * @param controllerNumber The controller to check.
 * @return true if there is a transfer pak inserted.
 */
bool isTPakInserted(const byte controllerNumber);

/**
 * Determines if there is a gameboy cartridge inserted in to a controller pak in the given controller.
 * @param controllerNumber The controller to check.
 * @return true if there is a readable cartridge inserted.
 */
bool isCartridgeInserted(const byte controllerNumber);

/**
 * Loads ROM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, ROM will be at this address.
 */
void loadRom(const byte controllerNumber, ByteArray* output);

/**
 * Loads save RAM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, save data will be at this address.
 */
void loadSave(const byte controllerNumber, ByteArray* output);

/**
 * Writes save file back to the cartridge.
 * @param controllerNumber transfer pak to write to.
 * @param save Save data to write.
 */
void persistSave(const byte controllerNumber, const ByteArray* save);

/**
 * Frees memory held by tpakio subsystem.
 */
void freeTPakIo();

#endif
