#include "core.h"
#include "state.h"
#include "text.h"
#include "resources.h"
#include "screen.h"
#include "logger.h"

#include <libdragon.h>

/*
// Idealish size, but part pixels
const unsigned short SINGLE_PLAYER_SCREEN_TOP = 50;
const unsigned short SINGLE_PLAYER_SCREEN_LEFT = 140;
const unsigned short SINGLE_PLAYER_SCREEN_WIDTH = 360;
const unsigned short SINGLE_PLAYER_SCREEN_HEIGHT = 324;
*/

// A little too small, but crisp screen.
const natural SINGLE_PLAYER_SCREEN_TOP = 50;
const natural SINGLE_PLAYER_SCREEN_LEFT = 160;
const natural SINGLE_PLAYER_SCREEN_WIDTH = 320;
const natural SINGLE_PLAYER_SCREEN_HEIGHT = 288;

const natural PLAYER_1_SCREEN_TOP = 50;
const natural PLAYER_1_SCREEN_LEFT = 27;
const natural PLAYER_1_SCREEN_WIDTH = 267;
const natural PLAYER_1_SCREEN_HEIGHT = 240;

const natural PLAYER_2_SCREEN_TOP = 50;
const natural PLAYER_2_SCREEN_LEFT = 347;
const natural PLAYER_2_SCREEN_WIDTH = 267;
const natural PLAYER_2_SCREEN_HEIGHT = 240;

/**
 * Draws permanent screen artifacts like the logo and borders
 * @param state program state.
 * @param frame identifies the frame to draw on.
 * @private
 */
static void hudDraw(const RootState* state, const display_context_t frame) {
    string text = "";
    getText(TextSplash, text);
    drawText(frame, text, 170, 10, 1);
}

/**
 * Gets the RDP module ready to render a new texture.
 * @param frame identifies frame to render to.
 */
void prepareRdpForSprite(const display_context_t frame) {
    // Assure RDP is ready for new commands
    rdp_sync(SYNC_PIPE);
    // Remove any clipping windows
    rdp_set_default_clipping();
    // Enable sprite display instead of solid color fill
    rdp_enable_texture_copy();
    // Attach RDP to display
    rdp_attach_display(frame);
}

/**
 * Loads a single sprite sheet in to the primary texture slot
 * @param spriteSheet sheet the sprite is on.
 * @param spriteCode position of the sprite in the sheet.
 * @param mirrorSetting how the sprite should behave when mirroring.
 */
void loadSprite(sprite_t* spriteSheet, const byte spriteCode, const mirror_t mirrorSetting) {
    // The other text slots don't seem to work for whatever reason, so let's abstract them away.
    rdp_load_texture_stride(0, 0, mirrorSetting, spriteSheet, spriteCode);
}

/**
 * Rerenders the background over both display buffers to cover whatever junk was there previously.
 * @param state program state
 */
void flushScreen(RootState* state) {
    // Background
    display_show(2);
    prepareRdpForSprite(1);
    loadSprite(getSpriteSheet(), BLUE_BG_TEXTURE, MIRROR_ENABLED);
    rdp_draw_textured_rectangle(0, 0, 0, RESOLUTION_X, RESOLUTION_Y);
    rdp_detach_display();
    display_show(1);
    rdp_attach_display(2);
    rdp_draw_textured_rectangle(0, 0, 0, RESOLUTION_X, RESOLUTION_Y);
    rdp_detach_display();

    display_show(2);
    hudDraw(state, 1);
    display_show(1);
    hudDraw(state, 2);
    rdp_detach_display();
}

/**
 * Draws an unfilled rectangle of a certain thickness.
 * @param frame frame to draw the border on.
 * @param position Where to draw the border on the screeen.
 * @param thickness How thick the border is.
 * @param colour The colour of the border.
 */
void drawSolidBorder(const display_context_t frame, const Rectangle* position, const natural thickness, const natural colour) {
    rdp_set_default_clipping();
    rdp_attach_display(frame);
    rdp_enable_primitive_fill();
    rdp_set_primitive_color(colour);

    rdp_draw_filled_rectangle(position->Left, position->Top, position->Left + thickness, position->Top + position->Height);
    rdp_draw_filled_rectangle(position->Left, position->Top + position->Height -thickness, position->Left + position->Width, position->Top + position->Height);
    rdp_draw_filled_rectangle(position->Left, position->Top, position->Left + position->Width, position->Top + thickness);
    rdp_draw_filled_rectangle(position->Left + position->Width - thickness, position->Top, position->Left + position->Width, position->Top + position->Height);

    rdp_detach_display();
}

/**
 * Get the gameboy screen rectangle based on player number.
 * @param state program state including number of players.
 * @param playerNumber number of a given player.
 * @out output The calculated screen size & position.
 */
void getScreenPosition(const RootState* state, const byte playerNumber, Rectangle* output) {
    if (state->PlayerCount == 1) {
        output->Top = SINGLE_PLAYER_SCREEN_TOP;
        output->Left = SINGLE_PLAYER_SCREEN_LEFT;
        output->Width = SINGLE_PLAYER_SCREEN_WIDTH;
        output->Height = SINGLE_PLAYER_SCREEN_HEIGHT;
    } else if (state->PlayerCount == 2) {
        if (playerNumber == 0) {
            output->Top = PLAYER_1_SCREEN_TOP;
            output->Left = PLAYER_1_SCREEN_LEFT;
            output->Width = PLAYER_1_SCREEN_WIDTH;
            output->Height = PLAYER_1_SCREEN_HEIGHT;
        } else if (playerNumber == 1) {
            output->Top = PLAYER_2_SCREEN_TOP;
            output->Left = PLAYER_2_SCREEN_LEFT;
            output->Width = PLAYER_2_SCREEN_WIDTH;
            output->Height = PLAYER_2_SCREEN_HEIGHT;
        } else {
            logAndPause("Unimplemented getScreenPosition");
        }
    } else {
        logAndPause("Unimplemented getScreenPosition");
    }
}

/**
 * Fade/Brighten a colour by a given amount.
 * @param colour The 16bit colour
 * @param fadeAmount This amount will be added to each component.
 * @returns The new faded colour.
 */
u16 fadeColour(const natural colour, const byte fadeAmount) {
    byte red = colour >> 11;
    byte green = (colour >> 6) & 0x1F;
    byte blue = (colour >> 1) & 0x1F;
    byte t = colour & 1;

    // Find how much space we have to fade.
    byte room[3] = {
        0x1F - red,
        0x1F - green,
        0x1F - blue
    };

    byte factor = fadeAmount;
    for (byte i = 0; i < 3; i++) {
        factor = factor < room[i] ? factor : room[i];
    }

    red = red + factor;
    green = green + factor;
    blue = blue + factor;

    return (red << 11) | (green << 6) | (blue << 1) | t;
}

/**
 * Takes a 16bit gameboy colour and returns a 32 bit libdragon colour
 * @param colour The colour from the gameboy
 * @return The colour that libdragon can render.
 */
uInt massageColour(const natural colour) {
    // Colours are in tBbbbbGggggRrrrr order, but we need to flip them to RrrrrGggggBbbbbt
    natural b = colour & 0x7C00;
    natural g = colour & 0x03E0;
    natural r = colour & 0x001F;
    natural t = colour & 0x8000;
    natural reversed = (r << 11 | (g << 1) | (b >> 9) | t >> 15);

    return (reversed << 16) | reversed;
}


