#include "core.h"
#include "controller.h"
#include "text.h"
#include "options.h"

/**
 * Handles the options menu for given player.
 * @param state Program state.
 * @param playerNumber player in options mode.
 */
void optionsLogic(RootState* state, byte playerNumber) {
    ;
}

/**
 * Draws the option name and current state to the screen.
 * @param state program state.
 * @param playerNumber number of the player with options to draw.
 * @param text text include option and value.
 * @param screen area of the screen to draw to.
 * @param row option will be drawn in this row.
 * @private
 */
void drawOptionRow(const RootState* state, const byte playerNumber, const string text, const Rectangle* screen, const byte row) {
    const float scaleFactor = 0.234 / 100.0;
    const float scale = (float) screen->Width * scaleFactor;
    const natural menuItemOffset = 34 * scale;

    natural top = screen->Top + screen->Height + (menuItemOffset * row);

    drawText(state->Frame, text, screen->Left, top, scale);

    // draw border if selected.
    if (state->Players[playerNumber].OptionsCursorRow == row) {
        // TODO: draw left & right arrows.
        Rectangle border = { top, screen->Left, screen->Width, menuItemOffset };
        drawSolidBorder(state->Frame, &border, 2, SELECTED_OPTIONS_ITEM_COLOUR);
    }
}

/**
 * Draws the Audio Enable/Disable option.
 * @param state program state.
 * @param playerNumber number of the player with options to draw.
 * @param screen area of the screen to draw to.
 * @param row option will be drawn in this row.
 * @private
 */
void drawAudioOption(const RootState* state, const byte playerNumber, const Rectangle* screen, const byte row) {
    string text = "";
    if (state->Players[playerNumber].AudioEnabled) {
        getText(TextAudioOn, text);
    } else {
        getText(TextAudioOff, text);
    }

    drawTextOption(state, playerNumber, text, screen, row);
}

/**
 * Draws one of the button map options, drawing the button images.
 * @param state program state.
 * @param playerNumber number of the player with options to draw.
 * @param button the gameboy/system button we are assigning to an N64 controller button.
 * @param screen area of the screen to draw to.
 * @param row option will be drawn in this row.
 * @private
 */
void drawButtonMapOption(const RootState* state, const byte playerNumber, const GbButton button, const Rectangle* screen, const byte row) {
    byte gbButtonSprite = 0;

    switch(button) {
        case GbStart:
            gbButtonSprite = GB_START_SPRITE;
            break;
        case GbSelect:
            gbButtonSprite = GB_SELECT_SPRITE;
            break;
        case GbSystemMenu:
            gbButtonSprite = MENU_SPRITE;
            break;
        default:
            gbButtonSprite = ERROR_SPRITE;
            break;
    }

    // Look up the N64 button currently assigned to the gb button.
    N64Button n64button = 0;
    for (byte i = 0; i < N64_BUTTON_COUNT; i++) {
        if (state->Players[playerNumber].ButtonMap[i] == button) {
            n64button = i;
        }
    }

    byte n64ButtonSprite = 0;
    //byte rotation = Up; // Default is up.
    switch(n64button) {
        case Start:
            n64ButtonSprite = N64_START_SPRITE;
            break;
        case L:
            n64ButtonSprite = N64_L_SPRITE;
            break;
        case R:
            n64ButtonSprite = N64_R_SPRITE;
            break;
        case Z:
            n64ButtonSprite = N64_Z_SPRITE;
            break;
        case CUp:
            n64ButtonSprite = N64_C_SPRITE;
            //rotation = Up;
            break;
        case CDown:
            n64ButtonSprite = N64_C_SPRITE;
            //rotation = Down;
            break;
        case CLeft:
            n64ButtonSprite = N64_C_SPRITE;
            //rotation = Left;
            break;
        case CRight:
            n64ButtonSprite = N64_C_SPRITE;
            //rotation = Right;
            break;
        default:
            n64ButtonSprite = 8;
    }

    string text = "";
    sprintf(text, "$%02x : $%02x", gbButtonSprite, n64ButtonSprite);
    drawTextOption(state, playerNumber, text, screen, row);

    return;

}

/**
 * Determines how to draw each possible option.
 * @param state program state.
 * @param playerNumber number of the player with options to draw.
 * @param option the option being drawn.
 * @param screen area of the screen to draw pto.
 * @param row option will be drawn in this row.
 * @private
 */
sByte drawOption(const RootState* state, const byte playerNumber, const OptionType option, const Rectangle* screen, const byte row) {
    switch (option) {
        case OptionsAudio:
            drawAudioOption(state, playerNumber, screen, row);
            break;
        case OptionsMenu:
            drawButtonMapOption(state, playerNumber, GbSystemMenu, screen, row);
            break;
        case OptionsStart:
            drawButtonMapOption(state, playerNumber, GbStart, screen, row);
            break;
        case OptionsSelect:
            drawButtonMapOption(state, playerNumber, GbSelect, screen, row);
            break;
        default:
            return -1;
    }

    return 0;
}

/**
 * Displays the options menu for given player.
 * @param state Program state.
 * @param playerNumber player in options mode.
 */
void optionsDraw(RootState* state, const byte playerNumber) {
    Rectangle screen = {};
    getScreenPosition(state, playerNumber, &screen);

    prepareRdpForSprite(state->Frame);
    loadSprite(getSpriteSheet(), BLUE_BG_TEXTURE, MIRROR_ENABLED);

    // Cover menu section.
    rdp_draw_textured_rectangle(0, 0, screen.Top + screen.Height, RESOLUTION_X, RESOLUTION_Y);

    if (state->Players[playerNumber].ActiveMode != Options) {
        state->RequiresRepaint = true;
        return;
    }

    for (byte i = 0; i < OptionsEnd; i++) {
        drawOption(state, playerNumber, (OptionType) i, &screen, i);
    }

    rdp_detach_display();
}
