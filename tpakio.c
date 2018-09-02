#include "include/libgbpak.h"
#include <libdragon.h>
#include "logger.h"
#include "tpakio.h"

static const bool isTPakWorking = true;
static GameboyCart pakInit[4]  = {};

/**
 * Prepares a cartridge to be read from the transfer pak.
 * @param controllerNumber Which Transfer pak to read from.
 * @private
 */
bool initPak(natural controllerNumber) {
    if (pakInit[controllerNumber].romsize == 0) {
        GameboyCart result = initialiseCart(controllerNumber);
        if (result.errorCode) {
            logInfo("gbpak init error");
            return false;
        }

        pakInit[controllerNumber] = result;
    }

    return true;
}

/**
 * Loads ROM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, ROM will be at this address.
 */
void loadRom(natural controllerNumber, ByteArray* output) {
    if (initPak(controllerNumber)) {
        importRom(controllerNumber, pakInit[controllerNumber], output);
    }
}

/**
 * Loads save RAM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, save data will be at this address.
 */
 void loadSave(natural controllerNumber, ByteArray* output) {
    if (initPak(controllerNumber)) {
        importSave(controllerNumber, pakInit[controllerNumber], output);
    }
}
