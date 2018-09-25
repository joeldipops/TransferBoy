#include "core.h"
#include "state.h"
#include "text.h"
#include "screen.h"

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
void hudDraw(const RootState* state, const display_context_t frame) {
    string text = "";
    getText(TextSplash, text);
    drawText(frame, text, 170, 10, 1);
}

/**
 * Rerenders the background over both display buffers to cover whatever junk was there previously.
 * @param state program state
 */
void flushScreen(const RootState* state) {
    display_show(2);
    graphics_draw_box(1, 0, 0, RESOLUTION_X, RESOLUTION_Y, GLOBAL_BACKGROUND_COLOUR);
    hudDraw(state, 1);
    display_show(1);
    graphics_draw_box(2, 0, 0, RESOLUTION_X, RESOLUTION_Y, GLOBAL_BACKGROUND_COLOUR);
    hudDraw(state, 2);
}

/**
 * Gets the RDP module ready to render a new texture.
 * @param frame identifies frame to render to.
 */
void prepareRdpForTexture(const display_context_t frame) {
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


