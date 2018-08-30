#include "include/libgbpak.h"
#include <libdragon.h>
#include "logger.h"
#include "tpakio.h"

static const bool isTPakWorking = true;
static bool pakInit[4]  = {false, false, false, false};
bool initPak(natural controllerNumber) {
    if (!pakInit[controllerNumber]) {
        if(init_gbpak()) {
            logInfo("gbpak init error");
            return false;
        }

        pakInit[controllerNumber] = true;
    }

    return true;
}

void loadRom(natural controllerNumber, byte* output) {
    if (initPak(controllerNumber)) {
        copy_gbRom_toRAM(output);
    }
}

void loadSave(natural controllerNumber, byte* output) {
    if (initPak(controllerNumber)) {
        copy_save_toGbRam(output);
    }
}
