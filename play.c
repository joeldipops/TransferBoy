#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "core.h"
#include "play.h"
#include "controller.h"
#include "screen.h"
#include "include/gbc_bundle.h"
#include "tpakio.h"
#include "link.h"

#include <libdragon.h>

/**
 * Loads a gb bios file if one is available.
 * @param state state to copy the bios to.
 * @return 0 if load successful, non-zero for errors.
 */
sByte loadBios(GbState* state) {
    sByte result = 0;
    byte* biosFile = null;

    dfs_init(DFS_DEFAULT_LOCATION);
    sInt filePointer = dfs_open("/bios.bin");

    sInt biosSize = 0;
    if (filePointer >= 0) {
        biosSize = dfs_size(filePointer);
    } else {
        result = -1;
    }

    if (biosSize > 0) {
        biosFile = malloc(biosSize);
        dfs_read(biosFile, 1, biosSize, filePointer);
        state_add_bios(state, biosFile, biosSize);
    } else {
        result = -2;
    }

    dfs_close(filePointer);
    free(biosFile);

    return result;
}

/**
 * Passes the gameboy cartridge data in to the emulator and fires it up.
 * @param state emulator state object.
 * @param romData ROM loaded from cartridge.
 * @param saveData Save file RAM loaded from cartridge.
 */
void initialiseEmulator(GbState* state, const ByteArray* romData, const ByteArray* saveData) {
    memset(state, 0, sizeof(GbState));

    state_new_from_rom(state, romData->Data, romData->Size);
    cpu_reset_state(state);

    loadBios(state);

    state_load_extram(state, saveData->Data, saveData->Size);
    init_emu_state(state);
    cpu_init_emu_cpu_state(state);
    lcd_init(state); // Here we're initing the code that renders the pixel buffer.
}


/**
 * Converts buttons pressed on the n64 controller into equivalents on the gameboy's.
 * @param controllerNumber The controller to process.
 * @param buttonMap Array indexed by N64Button identifying which buttons go where.
 * @param n64Input struct containing which N64 buttons are currently pressed.
 * @out gbInput struct of gb buttons to fill in.
 * @private
 */
void mapGbInputs(const char controllerNumber, const GbButton* buttonMap, const struct controller_data* n64Input, bool* pressedButtons, GbController* gbInput) {
    getPressedButtons(n64Input, controllerNumber, pressedButtons);

    for (byte i = 0; i < 16; i++) {
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
}

/**
 * Take the array of pixels produced by the emulator and throw it up on to the screen.
 * @param frame Id of frame to render to.
 * @param pixelBuffer Array of pixels.
 * @param isColour True for GBC and super-game-boy enhanced, otherwise false.
 * @param avgPixelSize upscaled TV size of gameboy pixels
 * @param left left bound of the gb screen to render.
 * @param top top bound of the gb screen to render.
 * @private
 */
void renderPixels(
    const display_context_t frame,
    const natural* pixelBuffer,
    const bool isColour,
    const float avgPixelSize,
    const natural left,
    const natural top
) {
    uInt* pixels = calloc(GB_LCD_HEIGHT * GB_LCD_WIDTH, sizeof(uInt));
    if (!pixels) {
        logInfo("Out of memory!!!");
        return;
    }

    // Lifted from gbC's gui_lcd_render_frame function.
    if (isColour) {
        /* The colors stored in pixbuf are two byte each, 5 bits per rgb
         * component: -bbbbbgg gggrrrrr. We need to extract these, scale these
         * values from 0-1f to 0-ff and put them in RGBA format. For the scaling
         * we'd have to multiply by 0xff/0x1f, which is 8.23, approx 8, which is
         * a shift by 3. */
        for (natural y = 0; y < GB_LCD_HEIGHT; y++) {
            for (natural x = 0; x < GB_LCD_WIDTH; x++) {
                natural index = x + y * GB_LCD_WIDTH;
                natural rawColour = pixelBuffer[index];

                uInt r = ((rawColour >>  0) & 0x1f) << 3;
                uInt g = ((rawColour >>  5) & 0x1f) << 3;
                uInt b = ((rawColour >> 10) & 0x1f) << 3;
                uInt pixel = graphics_make_color(r, g, b, 0x00);
                pixels[index] = pixel;
            }
        }
    } else {
        // The colors stored in pixbuf already went through the palette
        //translation, but are still 2 bit monochrome.
        uInt palette[] = { 0xffffffff, 0xaaaaaaaa, 0x66666666, 0x11111111 };
        for (natural y = 0; y < GB_LCD_HEIGHT; y++) {
            for (natural x = 0; x < GB_LCD_WIDTH; x++) {
                natural index = x + y * GB_LCD_WIDTH;
                pixels[index] = palette[pixelBuffer[index]];
            }
        }
    }

    // TODO - Scaling for when between whole number scales.

    rdp_set_default_clipping();
    rdp_attach_display(frame);
    rdp_enable_primitive_fill();
    for (natural y = 0; y < GB_LCD_HEIGHT; y ++) {
        for (natural x = 0; x <  GB_LCD_WIDTH; x++) {
            natural index = x + y * GB_LCD_WIDTH;

            rdp_set_primitive_color(pixels[index]);
            natural tx = x * avgPixelSize + left;
            natural ty = y * avgPixelSize + top;

            rdp_draw_filled_rectangle(tx, ty, tx + avgPixelSize, ty + avgPixelSize);
        }
    }
    string text = "";
    sprintf(text, "Frames: %lld", frameCount);
    graphics_draw_text(frame, left, top - 50, text);

    frameCount++;

    free(pixels);
    rdp_detach_display();
}

/**
 * Handles gameboy emulation.
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playLogic(RootState* state, const byte playerNumber) {
    GbState* emulatorState = &state->Players[playerNumber].EmulationState;

    if (emulatorState->emu_state->quit) {
        state->Players[playerNumber].ActiveMode = Quit;
        return;
    }

    if (state->PlayerCount == 2 && isRequestingTransfer(state)) {
        GbState* states[2] = {
            &state->Players[0].EmulationState,
            &state->Players[1].EmulationState
        };
        exchangeLinkData(states);
    }

    GbController* input = calloc(1, sizeof(struct player_input));
    emu_step_frame(emulatorState);

    bool pressedButtons[16] = {};

    mapGbInputs(
        playerNumber,
        state->Players[playerNumber].ButtonMap,
        &state->ControllerState,
        pressedButtons,
        input
    );

    if (pressedButtons[state->Players[playerNumber].SystemMenuButton]) {
        state->Players[playerNumber].ActiveMode = Menu;
        state->RequiresRepaint = true;
    }

    emu_process_inputs(emulatorState, input);

    free(input);

    // Write save file back to the catridge if it has changed.
    if (emulatorState->emu_state->extram_dirty && isCartridgeInserted(playerNumber)) {
        memcpy(
            state->Players[playerNumber].Cartridge.SaveData.Data,
            emulatorState->mem_EXTRAM,
            state->Players[playerNumber].Cartridge.SaveData.Size
        );
        persistSave(playerNumber, &state->Players[playerNumber].Cartridge.SaveData);
        emulatorState->emu_state->extram_dirty = false;
    }
    state->RequiresRepaint = true;

    // TODO - Map from emulatorState->emu_state->audio_sndbuf
}

/**
 * Draws gameboy screen.
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playDraw(const RootState* state, const byte playerNumber) {
    // Main background.
    Rectangle screen = {};
    getScreenPosition(state, playerNumber, &screen);

    graphics_draw_box(
        state->Frame,
        screen.Left,
        screen.Top,
        screen.Width,
        screen.Height,
        BLANK_SCREEN_COLOUR
    );

    renderPixels(
        state->Frame,
        state->Players[playerNumber].EmulationState.emu_state->lcd_pixbuf,
        state->Players[playerNumber].Cartridge.IsGbcCart,
        (float)screen.Height / (float)GB_LCD_HEIGHT,
        screen.Left,
        screen.Top
    );
}
