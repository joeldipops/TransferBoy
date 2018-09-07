#ifndef TPAKIO_INLCUDED
#define TPAKIO_INCLUDED
#include "core.h"

/**
 * Determines if there is a transfer pak inserted in to the given controller.
 * @param controllerNumber The controller to check.
 * @return true if there is a transfer pak inserted.
 */
bool isTPakInserted(const unsigned char controllerNumber);

/**
 * Determines if there is a gameboy cartridge inserted in to a controller pak in the given controller.
 * @param controllerNumber The controller to check.
 * @return true if there is a readable cartridge inserted.
 */
bool isCartridgeInserted(const unsigned char controllerNumber);

/**
 * Loads ROM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, ROM will be at this address.
 */
void loadRom(const unsigned char controllerNumber, ByteArray* output);

/**
 * Loads save RAM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, save data will be at this address.
 */
void loadSave(const unsigned char controllerNumber, ByteArray* output);

/**
 * Frees memory held by tpakio subsystem.
 */
void freeTPakIo();

#endif
