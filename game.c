#include "constants.h"
#include "game.h"
#include "tpakio.h"
#include "logger.h"
#include "include/gbc_bundle.h"
#include <libdragon.h>

void gameRender(const string text) {
    logInfo(text);
}

void emulatorLoop(ByteArray* romData, ByteArray* saveData) {
    struct gb_state* emulatorState = malloc(sizeof(struct gb_state));
    state_new_from_rom(emulatorState, romData->Data, romData->Size);
    init_emu_state(emulatorState);
    cpu_init_emu_cpu_state(emulatorState);

    struct player_input* input;
    input = malloc(sizeof(struct player_input));

    while (!emulatorState->emu_state->quit) {
        emu_step_frame(emulatorState);

        // TODO - GET INPUT FROM N64 Controller and map to input_state

        emu_process_inputs(emulatorState, input);

        // TODO - Map from emulatorState->emu_state->lcd_pixbuf to N64 display.

        // TODO - Map from emulatorState->emu_state->audio_sndbuf
    }
}

void gameLoop(OptionsHash* options) {
    // Find what controllers have games & tpacs plugged in
    ByteArray* romData = malloc(sizeof(ByteArray));
    ByteArray* saveData = malloc(sizeof(ByteArray));

    bool hasQuit = false;
    while(!hasQuit) {
        gameRender("Loading Cartridge...");
        get_accessories_present();

        // Only interested in controller 1 while I figure things out.
        if (identify_accessory(0) == ACCESSORY_MEMPAK) {
            loadSave(0, saveData);
            loadRom(0, romData);

            gameRender("Cartidge Loaded.  Press Start");
        } else {
            gameRender("No Cartridge Inserted");
        }

        while(!hasQuit) {
            controller_scan();
            struct controller_data buttons = get_keys_pressed();
            if(buttons.c[0].start) {
                emulatorLoop(romData, saveData);
            }
            if(buttons.c[0].B) {
                hasQuit = true;
            }
        }
    }
}
