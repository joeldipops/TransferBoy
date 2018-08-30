#include "include/libgbpak.h"
#include <libdragon.h>
#include "logger.h"
#include "tpakio.h"

static const bool isTPakWorking = true;

void loadRom(natural controllerNumber, byte* output) {
    // Read from disk until we can figure out transfer pak
    if (isTPakWorking) {
        if (init_gbpak()) {
            logInfo("gbpak init error");
        }

        copy_gbRom_toRAM(output);
        logInfo("Copied");
    } else {
        output = malloc(32);
    }
}

void loadSave(natural controllerNumber, byte* output) {
    // Read from disk until we can figure out transfer pak
    if (isTPakWorking) {
        init_gbpak();
        copy_save_toGbRam(output);
        output = malloc(32);
    }
}
