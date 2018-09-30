#include "core.h"
#include "controller.h"
#include "text.h"
#include "options.h"

#define RESERVED_BUTTONS_SIZE 15
static const N64Button RESERVED_BUTTONS[RESERVED_BUTTONS_SIZE] = {
    NoButton, A, B, DUp, DDown, DLeft, DRight, StickUp, StickDown, StickLeft, StickRight, Up, Down, Left, Right
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

N64Button selectNextButton(const GbButton button, const GbButton* buttonMap, const bool isMovingLeft) {
    bool buttonList[N64_BUTTON_COUNT] = {0};
    for (byte i = 0; i < RESERVED_BUTTONS_SIZE; i++) {
        buttonList[RESERVED_BUTTONS[i]] = true;
    }

    byte result = (byte) NoButton;
    for (byte i = 0; i < N64_BUTTON_COUNT; i++) {
        if (buttonMap[i] == button) {
            result = i;
            break;
        }
    }

    while (true) {
        if (isMovingLeft) {
            if (result <= 0) {
                result = N64_BUTTON_COUNT;
            } else {
                result--;
            }
        } else {
            if (result >= N64_BUTTON_COUNT) {
                result = 0;
            } else {
                result++;
            }
        }
        // If this button is not reserved for some other purpose, return it.
        if (!buttonList[result]) {
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
    N64Button selected = NoButton;
    switch(option) {
        case OptionsAudio:
            playerState->AudioEnabled = !playerState->AudioEnabled;
            break;
        case OptionsMenu:
            selected = selectNextButton(GbSystemMenu, playerState->ButtonMap, isMovingLeft);
            setButtonToMap(playerState, GbSystemMenu, selected);
            break;
        case OptionsStart:
            selected = selectNextButton(GbStart, playerState->ButtonMap, isMovingLeft);
            setButtonToMap(playerState, GbStart, selected);
            break;
        case OptionsSelect:
            selected = selectNextButton(GbSelect, playerState->ButtonMap, isMovingLeft);
            setButtonToMap(playerState, GbSelect, selected);
            break;
        default:
            return -1;
    }

    return 0;
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
        showMainMenu(playerState);
    // START or MENU button closes the menu and goes back to the action.
    } else if (pressedButtons[Start] || pressedButtons[playerState->SystemMenuButton]) {
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
            break;
    }

    string text = "";
    sprintf(text, "$%02x : $%02x", gbButtonSprite, n64ButtonSprite);
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
