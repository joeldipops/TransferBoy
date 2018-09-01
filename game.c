#include "utils.h"
#include "game.h"
#include "tpakio.h"
#include "logger.h"
#include "include/gbc_bundle.h"

#include <libdragon.h>

void gameRender(const string text) {
    logInfo(text);
}

void mapGbInputs(const char controllerNumber, const GbButton* buttonMap, const struct controller_data* n64Input, struct player_input* gbInput) {
    bool* pressedButtons = malloc(sizeof(bool) * 16);
    getPressedButtons(n64Input, controllerNumber, pressedButtons);

    for (int i = 0; i < 16; i++) {
        if (!pressedButtons[i]) {
            continue;
        }

        switch(buttonMap[i]) {
            case GbA: gbInput->button_a = true; break;
            case GbB: gbInput->button_b = true; break;
            case GbStart: gbInput->button_start = true; break;
            case GbSelect: gbInput->button_select = true; break;
            case GbUp: gbInput->button_up = true; break;
            case GbDown: gbInput->button_down = true; break;
            case GbLeft: gbInput->button_left = true; break;
            case GbRight: gbInput->button_right = true; break;
            default: break;
        }
    }

    free(pressedButtons);
}

void initialiseEmulator(struct gb_state* state, const ByteArray* romData, const ByteArray* saveData) {
    memset(state, 0, sizeof(struct gb_state));

    state_new_from_rom(state, romData->Data, romData->Size);
    cpu_reset_state(state);

    // Add bios here?
    //dfs_init(DFS_DEFAULT_LOCATION);
    //

    state_load_extram(state, saveData->Data, saveData->Size);
    init_emu_state(state);
    cpu_init_emu_cpu_state(state);
    lcd_init(state); // Here we're initing the code that renders the pixel buffer.
}

void emulatorLoop(const OptionsHash* options, const ByteArray* romData, ByteArray* saveData) {
    struct gb_state* emulatorState = malloc(sizeof(struct gb_state));
    initialiseEmulator(emulatorState, romData, saveData);

    struct player_input* input;
    input = malloc(sizeof(struct player_input));
    memset(input, 0, sizeof(struct player_input));

    while (!emulatorState->emu_state->quit) {
        emu_step_frame(emulatorState);

        controller_scan();
        struct controller_data n64Input = get_keys_pressed();
        mapGbInputs(0, options->ButtonMap, &n64Input, input);

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
                emulatorLoop(options, romData, saveData);
            }
            if(buttons.c[0].B) {
                hasQuit = true;
            }
        }
    }
}
