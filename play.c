#include "core.h"
#include "play.h"
#include "controller.h"
#include "include/gbc_bundle.h"

#include <libdragon.h>
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
 * Take the array of pixels produced by the emulator and throw it up on to the screen.
 * @param frame Id of frame to render to.
 * @param pixelBuffer Array of pixels.
 * @param isColour True for GBC and super-game-boy enhanced, otherwise false.
 * @private
 */
void renderPixels(const display_context_t frame, const unsigned short* pixelBuffer, const bool isColour) {
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

    for (int y = 0; y < GB_LCD_HEIGHT; y++) {
        for (int x = 0; x <  GB_LCD_WIDTH; x++) {
            graphics_draw_pixel(frame, x + SINGLE_PLAYER_SCREEN_LEFT, y + SINGLE_PLAYER_SCREEN_TOP, pixels[x + y * GB_LCD_WIDTH]);
        }
    }

    graphics_draw_text(frame, HORIZONTAL_MARGIN, VERTICAL_MARGIN, "HIHI TEXT");

    free(pixels);
}

/**
 * Handles gameboy emulation.
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playLogic(RootState* state, const unsigned char playerNumber) {
    struct gb_state* emulatorState = &state->Players[playerNumber].EmulationState;

    if (emulatorState->emu_state->quit) {
        state->Players[playerNumber].ActiveMode = Quit;
        return;
    }

    struct player_input* input;
    input = malloc(sizeof(struct player_input));
    memset(input, 0, sizeof(struct player_input));

    emu_step_frame(emulatorState);

    mapGbInputs(
        0,
        state->Players[playerNumber].ButtonMap,
        &state->ControllerState,
        input
    );

    emu_process_inputs(emulatorState, input);

    // TODO - Map from emulatorState->emu_state->audio_sndbuf
}

/**
 * Draws gameboy screen.
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playDraw(const RootState* state, const unsigned char playerNumber) {
    renderPixels(
        state->Frame,
        state->Players[playerNumber].EmulationState.emu_state->lcd_pixbuf,
        state->Players[playerNumber].Cartridge.IsGbcCart || state->Players[playerNumber].Cartridge.IsSuperGbCart
    );
}
