#include "include/libgbpak.h"
#include <libdragon.h>
#include "logger.h"
#include "tpakio.h"

static const bool isTPakWorking = true;
static GameboyCart pakInit[4]  = {};

/**
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

void loadRom(natural controllerNumber, ByteArray* output) {
    if (initPak(controllerNumber)) {
        importRom(controllerNumber, pakInit[controllerNumber], output);
    }
}

void loadSave(natural controllerNumber, ByteArray* output) {
    if (initPak(controllerNumber)) {
        importSave(controllerNumber, pakInit[controllerNumber], output);
    }
}
