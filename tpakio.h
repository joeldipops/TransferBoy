#ifndef TPAKIO_INLCUDED
#define TPAKIO_INCLUDED
#include "utils.h"

/**
 * Loads ROM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, ROM will be at this address.
 */
void loadRom(natural controllerNumber, ByteArray* output);

/**
 * Loads save RAM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, save data will be at this address.
 */
void loadSave(natural controllerNumber, ByteArray* output);

#endif
