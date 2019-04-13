#ifndef TPAKIO_INTERFACE_INLCUDED
#define TPAKIO_INTERFACE_INCLUDED
#include "core.h"

/**
 * Determines if the expansion pak is plugged in.
 * @return true if the N64 Memory Expansion Pak is available.
 */
bool isExpansionPakInserted();



/**
 * GBC roms can be as big as 2MB, without an expansion pak, we only have 4MB to play with, and we certainly couldn't
 * emulate two 2MB roms at once without an expansion pak.
 * @param controllerNumber number of cartridge to check.
 * @return true if we have enough memory sitting around to load the ROM, false otherwise.
 */
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
 * Reads a cartridge from a transfer pak dumps into a catridge data object.
 * @param controllerNumber The controller to read from.
 * @out result cartridge data goes here.
 */
sByte readCartridge(const byte controllerNumber, CartridgeData* output);

/**
 * Reads the meta data of a cartridge from the transfer pak
 * @param controllerNumber The controller to read from.
 * @out result The meta data goes here.
 * @return Error code
 * 0  - Success
 */
sByte getCartridgeMetaData(const byte controllerNumber, CartridgeData* result);

/**
 * Frees memory held by tpakio subsystem.
 */
void freeTPakIo();

#endif
