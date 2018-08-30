#include <stdio.h>
#include <malloc.h>
#include "constants.h"
#include "game.h"
#include "tpakio.h"
#include "logger.h"
#include <libdragon.h>

void gameRender(const string text) {
    logInfo(text);
}

void gameLoop(OptionsHash* options) {
    // Find what controllers have games & tpacs plugged in
    byte* romData = {0};
    byte* saveData = {0};

    get_accessories_present();

    // Only interested in controller 1 while I figure things out.
    if (identify_accessory(0) == ACCESSORY_MEMPAK) {
        loadRom(0, romData);
        loadSave(0, saveData);

    } else {
        gameRender("No PAK inserted");
    }

    while(true) {;}
}
