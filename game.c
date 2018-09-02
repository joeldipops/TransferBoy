#include "utils.h"
#include "game.h"
#include "tpakio.h"
#include "logger.h"
#include "include/gbc_bundle.h"

#include <libdragon.h>

// Since the gameboy screen is much smaller than ours, position it somewhere.
// Eventually configuable, especially for 2+ player.
static const natural SCREEN_Y_OFFSET = 80;
static const natural SCREEN_X_OFFSET = 80;

/**
 * Converts buttons pressed on the n64 controller into equivalents on the gameboy's.
 * @param controllerNumber The controller to process.
 * @param buttonMap Array indexed by N64Button identifying which buttons go where.
 * @param n64Input struct containing which N64 buttons are currently pressed.
 * @out gbInput struct of gb buttons to fill in.
 * @private
 */
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

/**
 * Passes the gameboy cartridge data in to the emulator and fires it up.
 * @param state emulator state object.
 * @param romData ROM loaded from cartridge.
 * @param saveData Save file RAM loaded from cartridge.
  * @private
 */
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

/**
 * Take the array of pixels produced by the emulator and throw it up on to the screen.
 * @param pixelBuffer Array of pixels
 * @param isColour True for GBC and super-game-boy enhanced, otherwise false.
 * @private
 */
void renderPixels(const unsigned short* pixelBuffer, const bool isColour) {
    natural* pixels = malloc(GB_LCD_HEIGHT * GB_LCD_WIDTH);
    memset(pixels, 0xFF, GB_LCD_HEIGHT * GB_LCD_WIDTH);

    // Lifted from gbC's gui_lcd_render_frame function.
    if (isColour) {
        /* The colors stored in pixbuf are two byte each, 5 bits per rgb
         * component: -bbbbbgg gggrrrrr. We need to extract these, scale these
         * values from 0-1f to 0-ff and put them in RGBA format. For the scaling
         * we'd have to multiply by 0xff/0x1f, which is 8.23, approx 8, which is
         * a shift by 3. */
        for (int y = 0; y < GB_LCD_HEIGHT; y++) {
            for (int x = 0; x < GB_LCD_WIDTH; x++) {
                int index = x + y * GB_LCD_WIDTH;
                unsigned short rawColour = pixelBuffer[index];

                natural r = ((rawColour >>  0) & 0x1f) << 3;
                natural g = ((rawColour >>  5) & 0x1f) << 3;
                natural b = ((rawColour >> 10) & 0x1f) << 3;
                natural pixel = (r << 24) | (g << 16) | (b << 8) | 0xff;
                pixels[index] = pixel;
            }
        }
    } else {
        /* The colors stored in pixbuf already went through the palette
         * translation, but are still 2 bit monochrome. */
        natural palette[] = { 0xffffffff, 0xaaaaaaaa, 0x66666666, 0x11111111 };
        for (int y = 0; y < GB_LCD_HEIGHT; y++) {
            for (int x = 0; x < GB_LCD_WIDTH; x++) {
                int index = x + y * GB_LCD_WIDTH;
                pixels[index] = palette[pixelBuffer[index]];
            }
        }
    }

    static display_context_t frame = null;
    while (!(frame = display_lock()));
    graphics_fill_screen(frame, GLOBAL_BACKGROUND_COLOUR);

    for (int y = 0; y < GB_LCD_HEIGHT; y++) {
        for (int x = 0; x <  GB_LCD_WIDTH; x++) {
            graphics_draw_pixel(frame, x + SCREEN_X_OFFSET, y + SCREEN_Y_OFFSET, pixels[x + y * GB_LCD_WIDTH]);
        }
    }

    display_show(frame);

    free(pixels);
}

/**
 * Main game loop for the emulator, taking input and displaying output.
 * @param options Options configured from the options screen.
 * @param romData Game ROM loaded from cartridge.
 * @param saveData Save RAM loaded from cartridge.
 * @private
 */
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

        renderPixels(emulatorState->emu_state->lcd_pixbuf, false);

        // TODO - Map from emulatorState->emu_state->audio_sndbuf
    }
}

/**
 * Loads Cartridge in to memory.  TODO - clean-up, helpers etc
 */
void gameLoop(OptionsHash* options) {
    // Find what controllers have games & tpacs plugged in
    ByteArray* romData = malloc(sizeof(ByteArray));
    ByteArray* saveData = malloc(sizeof(ByteArray));

    bool hasQuit = false;
    while(!hasQuit) {
        logInfo("Loading Cartridge...");
        get_accessories_present();

        // Only interested in controller 1 while I figure things out.
        if (identify_accessory(0) == ACCESSORY_MEMPAK) {
            loadSave(0, saveData);
            loadRom(0, romData);

            logInfo("Cartidge Loaded.  Press Start");
        } else {
            logInfo("No Cartridge Inserted");
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
