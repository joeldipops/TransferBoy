#include "core.h"
#include "controller.h"
#include "text.h"
#include "options.h"

static const N64Button RESERVED_BUTTONS[N64_BUTTON_COUNT] = {
    1,// NoButton
    1,// A
    1,// B,
    0,// L,
    0,// R,
    0,// Z,
    0,// Start,
    1,// DUp,
    1,// DDown,
    1,// DLeft,
    1,// DRight,
    0,// CUp,
    0,// CDown,
    0,// CLeft,
    0,// CRight,
    1,// StickUp,
    1,// StickDown,
    1,// StickLeft,
    1,// StickRight,
    1,// Up,
    1,// Down,
    1,// Left,
    1,// Right
};

/**
 * Exits the menu by setting the player's mode back to "Play"
 * @param playerState state of the given player.
 * @private
 */
void resumePlayFromOptions(PlayerState* playerState) {
    playerState->MenuCursorRow = -1;
    playerState->OptionsCursorRow = -1;
    playerState->ActiveMode = Play;
}

/**
 * Returns to the main menu by setting the player's mode to "Menu"
 * @param playerState state of the player.
 * @private
 */
void showMainMenu(PlayerState* playerState) {
    playerState->MenuCursorRow = 0;
    playerState->OptionsCursorRow = -1;
    playerState->ActiveMode = Menu;
}

/**
 * Selects the next n64 button that's available to be assigned.
 * @param currentButton button previously assigned button.
 * @param true to select a button with a lower id, false otherwise.
 * @return The next available button.
 */
N64Button selectNextButton(const N64Button currentButton, const bool isMovingLeft) {
    byte result = (byte) currentButton;
    byte max = N64_BUTTON_COUNT - 1;

    while (true) {
        if (isMovingLeft) {
            if (result <= 0) {
                result = max;
            } else {
                result--;
            }
        } else {
            if (result >= max) {
                result = 0;
            } else {
                result++;
            }
        }

        // If this button is not reserved for some other purpose, return it.
        if (!RESERVED_BUTTONS[result]) {
            return result;
        }
    }

    return NoButton;
}

/**
 * @param playerState updating an option for this player.
 * @param option the option to update.
 * @param isMovingLeft if true, picks the option one space to the left of the current one, otherwise one space right.
 * @return success/error code
 ** 0   success
 ** -1  unknown option requested.
 * @private
 */
sByte selectOption(PlayerState* playerState, const OptionType option, const bool isMovingLeft) {
    switch(option) {
        case OptionsAudio:
            playerState->AudioEnabled = !playerState->AudioEnabled;
            break;
        case OptionsMenu:
            playerState->SystemMenuButton = selectNextButton(playerState->SystemMenuButton, isMovingLeft);
            break;
        case OptionsStart:
            playerState->GbStartButton = selectNextButton(playerState->GbStartButton, isMovingLeft);
            break;
        case OptionsSelect:
            playerState->GbSelectButton = selectNextButton(playerState->GbSelectButton, isMovingLeft);
            break;
        default:
            return -1;
    }

    return 0;
}

/**
 * Ensures update options will be used going forward.
 * @param playerState state of player to update.
 * @private
 */
void confirmOptions(PlayerState* playerState) {
    setButtonToMap(playerState, GbSystemMenu, playerState->SystemMenuButton);
    setButtonToMap(playerState, GbStart, playerState->GbStartButton);
    setButtonToMap(playerState, GbSelect, playerState->GbSelectButton);
}

/**
 * Handles the options menu for given player.
 * @param state Program state.
 * @param playerNumber player in options mode.
 */
void optionsLogic(RootState* state, byte playerNumber) {
    PlayerState* playerState = &state->Players[playerNumber];

    bool pressedButtons[N64_BUTTON_COUNT] = {0};
    getPressedButtons(&state->KeysReleased, playerNumber, pressedButtons);

    bool repaintRequired = true;
    bool ctrlReadRequired = true;

    if (playerState->OptionsCursorRow == -1) {
        playerState->OptionsCursorRow = 0;
    // UP AND DOWN change to a different option.
    } else if (pressedButtons[Up]) {
        if (playerState->OptionsCursorRow > 0) {
            playerState->OptionsCursorRow--;
        } else {
            playerState->OptionsCursorRow = OptionsEnd - 1;
        }
    } else if (pressedButtons[Down]) {
        if (playerState->OptionsCursorRow < OptionsEnd - 1) {
            playerState->OptionsCursorRow++;
        } else {
            playerState->OptionsCursorRow = OptionsEnd - 1;
        }
    // LEFT AND RIGHT make changes of the currently selected option.
    } else if (pressedButtons[Left] || pressedButtons[Right]) {
        selectOption(playerState, (OptionType) playerState->OptionsCursorRow, pressedButtons[Left]);
    // B goes back to the menu.
    } else if (pressedButtons[B]) {
        confirmOptions(playerState);
        showMainMenu(playerState);
    // START or MENU button closes the menu and goes back to the action.
    } else if (pressedButtons[Start] || pressedButtons[playerState->SystemMenuButton]) {
        confirmOptions(playerState);
        resumePlayFromOptions(playerState);
    } else {
        repaintRequired = false;
        ctrlReadRequired = false;
    }

    state->RequiresRepaint |= repaintRequired;
    state->RequiresControllerRead |= ctrlReadRequired;
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
    const float scale = (float) screen->Width * TEXT_SCALE_FACTOR;
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

    drawOptionRow(state, playerNumber, text, screen, row);
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
    N64Button n64Button = 0;
    switch(button) {
        case GbStart:
            gbButtonSprite = GB_START_SPRITE;
            n64Button = state->Players[playerNumber].GbStartButton;
            break;
        case GbSelect:
            gbButtonSprite = GB_SELECT_SPRITE;
            n64Button = state->Players[playerNumber].GbSelectButton;
            break;
        case GbSystemMenu:
            gbButtonSprite = MENU_SPRITE;
            n64Button = state->Players[playerNumber].SystemMenuButton;
            break;
        default:
            gbButtonSprite = ERROR_SPRITE;
            break;
    }

    byte n64ButtonSprite = 0;
    char rotation = 0; // Default is up.
    switch(n64Button) {
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
            break;
        case CDown:
            n64ButtonSprite = N64_C_SPRITE;
            rotation = ROTATE_180;
            break;
        case CLeft:
            n64ButtonSprite = N64_C_SPRITE;
            rotation = ROTATE_270;
            break;
        case CRight:
            n64ButtonSprite = N64_C_SPRITE;
            rotation = ROTATE_90;
            break;
        default:
            n64ButtonSprite = ERROR_SPRITE;
            break;
    }

    string text = "";
    if (rotation) {
        // TODO
        sprintf(text, "$%02x : $%c%02x", gbButtonSprite, rotation, n64ButtonSprite);
    } else {
        sprintf(text, "$%02x : $%02x", gbButtonSprite, n64ButtonSprite);
    }

    drawOptionRow(state, playerNumber, text, screen, row);
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
