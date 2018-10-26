#include "include/libgbpak.h"
#include <libdragon.h>
#include "logger.h"
#include "tpakio.h"

static GameboyCart* _pakInit[4]  = {null, null, null, null};

/**
 * Determines if the expansion pak is plugged in.
 * @return true if the N64 Memory Expansion Pak is available.
 */
bool isExpansionPakInserted() {
    return getMemoryLimit() > 5e6;
}


/**
 * Prepares a cartridge to be read from the transfer pak.
 * @param controllerNumber Which Transfer pak to read from.
 * @private
 * @return error code
 ** 0  - Success
  */
sByte initPak(const byte controllerNumber) {
    sByte result = 0;
    if(!_pakInit[controllerNumber]) {
        _pakInit[controllerNumber] = malloc(sizeof(GameboyCart));

        result = initialiseCart(controllerNumber, _pakInit[controllerNumber]);
    }
    return result;
}

/**
 * GBC roms can be as big as 2MB, without an expansion pak, we only have 4MB to play with, and we certainly couldn't
 * emulate two 2MB roms at once without an expansion pak.
 * @param controllerNumber number of cartridge to check.
 * @return true if we have enough memory sitting around to load the ROM, false otherwise.
 */
bool isCartridgeSizeOk(const byte controllerNumber) {
    bool result = (initPak(controllerNumber) == 0);
    if (!result) {
        return result;
    }

    return (_pakInit[controllerNumber]->romsize * 3 <= getMemoryLimit());
}

/**
 * Determines if there is a gameboy cartridge inserted in to a controller pak in the given controller.
 * @param controllerNumber The controller to check.
 * @return true if there is a readable cartridge inserted.
 */
bool isCartridgeInserted(const byte controllerNumber) {
    return initPak(controllerNumber) == 0;
}

/**
 * Determines if there is a transfer pak inserted in to the given controller.
 * @param controllerNumber The controller to check.
 * @return true if there is a transfer pak inserted.
 */
bool isTPakInserted(const byte controllerNumber) {
    get_accessories_present();
    if (identify_accessory(controllerNumber) != ACCESSORY_MEMPAK) {
        return false;
    }

    // No idea how to check whether it's mempak or a tpak, just return true for now.
    return true;
}

/**
 * Loads ROM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, ROM will be at this address.
 */
void loadRom(const byte controllerNumber, ByteArray* output) {
    if (initPak(controllerNumber) == 0) {
        char error = importRom(controllerNumber, _pakInit[controllerNumber], output);
        if (error) {
            logAndPause("rom load failed with code %d", error);
        }
    }
}

/**
 * Writes save file back to the cartridge.
 * @param controllerNumber transfer pak to write to.
 * @param save Save data to write.
 */
void persistSave(const byte controllerNumber, const ByteArray* save) {
    if (initPak(controllerNumber) == 0 && _pakInit[controllerNumber]->ramsize) {
        char error = exportSave(controllerNumber, _pakInit[controllerNumber], save);
        if (error) {
            logAndPause("save failed with code %d", error);
        }
    }
}

/**
 * Loads save RAM from game cartridge in to memory.
 * @param controllerNumber identifies transfer pak to read from.
 * @out output Once loaded, save data will be at this address.
 */
 void loadSave(const byte controllerNumber, ByteArray* output) {
    if (initPak(controllerNumber) == 0 && _pakInit[controllerNumber]->ramsize) {
        char error = importSave(controllerNumber, _pakInit[controllerNumber], output);
        if (error) {
            logAndPause("save load failed with code %d", error);
        }
    }
}

/**
 * Reads the meta data of a cartridge from the transfer pak
 * @param controllerNumber The controller to read from.
 * @out output The meta data goes here.
 * @return Error code
 * 0  - Success
  */
sByte getCartridgeMetaData(const byte controllerNumber, CartridgeData* output) {
    sByte result = initPak(controllerNumber);
    if (result) {
        return result;
    }

    strcpy(output->Title, _pakInit[controllerNumber]->title);
    output->IsGbcCart = _pakInit[controllerNumber]->gbc;
    output->IsSuperGbCart = _pakInit[controllerNumber]->sgb;

    return 0;
}

/**
 * Reads a cartridge from a transfer pak and sets up a cartridge data object.
 * @param controllerNumber The controller to read from.
 * @out result cartridge data goes here.
 * @return Error code
 * 0  - Success
 * -1 - Problem initialising the cartridge.
 * -2 - Problem loading the ROM.
 * -3 - Problem loading the save file.
 */
sByte readCartridge(const byte controllerNumber, CartridgeData* result) {
    sByte resultCode = getCartridgeMetaData(controllerNumber, result);
    if (resultCode) {
        return resultCode;
    }

    loadRom(controllerNumber, &result->RomData);
    loadSave(controllerNumber, &result->SaveData);

    freeTPakIo();

    return 0;
}

/**
 * Frees memory held by tpakio subsystem.
 */
void freeTPakIo() {
    for(byte i = 0; i < 4; i++) {
        if (_pakInit[i] != null) {
            free(_pakInit[i]);
            _pakInit[i] = null;
        }
    }
}
