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
    ByteArray* romData = malloc(sizeof(ByteArray));
    ByteArray* saveData = malloc(sizeof(ByteArray));

    get_accessories_present();

    // Only interested in controller 1 while I figure things out.
    if (identify_accessory(0) == ACCESSORY_MEMPAK) {
        loadSave(0, saveData);
        logInfo("Save loaded");
        loadRom(0, romData);
        logInfo("Rom loaded");
    } else {
        gameRender("No PAK inserted");
    }

    while(true) {;}
}
