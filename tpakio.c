#include "include/libgbpak.h"
#include <libdragon.h>
#include "logger.h"
#include "tpakio.h"

static const bool isTPakWorking = true;
static GameboyCart* _pakInit[4]  = {null, null, null, null};

/**
 * Prepares a cartridge to be read from the transfer pak.
 * @param controllerNumber Which Transfer pak to read from.
 * @private
 */
bool initPak(const unsigned char controllerNumber) {
    if(!_pakInit[controllerNumber]) {
        _pakInit[controllerNumber] = malloc(sizeof(GameboyCart));

        initialiseCart(controllerNumber, _pakInit[controllerNumber]);
        if (_pakInit[controllerNumber]->errorCode) {
            logInfo("gbpak init error");
            return false;
        }
    }

    return true;
}

/**
 * Determines if there is a gameboy cartridge inserted in to a controller pak in the given controller.
 * @param controllerNumber The controller to check.
 * @return true if there is a readable cartridge inserted.
 */
bool isCartridgeInserted(const unsigned char controllerNumber) {
    return initPak(controllerNumber);
}

/**
 * Determines if there is a transfer pak inserted in to the given controller.
 * @param controllerNumber The controller to check.
 * @return true if there is a transfer pak inserted.
 */
bool isTPakInserted(const unsigned char controllerNumber) {
    get_accessories_present();
    if (identify_accessory(0) != ACCESSORY_MEMPAK) {
        return false;
    }

    // No idea how to check this, just return true for now.
    if (controllerNumber == 0)
        return true;
    else
        return false;
}

/**
 * Loads ROM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, ROM will be at this address.
 */
void loadRom(const unsigned char controllerNumber, ByteArray* output) {
    if (initPak(controllerNumber)) {
        importRom(controllerNumber, *_pakInit[controllerNumber], output);
    }
}

/**
 * Loads save RAM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, save data will be at this address.
 */
 void loadSave(const unsigned char controllerNumber, ByteArray* output) {
    if (initPak(controllerNumber)) {
        importSave(controllerNumber, *_pakInit[controllerNumber], output);
    }
}

/**
 * Frees memory held by tpakio subsystem.
 */
void freeTPakIo() {
    for(unsigned char i = 0; i < 4; i++) {
        if (_pakInit[i] != null) {
            free(_pakInit[i]);
        }
    }
}
