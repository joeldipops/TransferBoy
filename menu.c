#include "core.h"
#include <libdragon.h>

/**
 * Draws a menu item.
 * @param frame Id of frame to render to.
 * @param label identifies the label of the menu item.
 * @param position Ordinal position of the menu item.
 * @private
 */
void drawMenuItem(const display_context_t frame, const TextId label, const unsigned char position, const bool drawCursor) {
    const unsigned short menuItemOffset = 10;
    const unsigned short cursorOffset = 20;

    string text = "";
    getText(label, text);
    graphics_draw_text(
        frame,
        cursorOffset,
        SINGLE_PLAYER_SCREEN_TOP + SINGLE_PLAYER_SCREEN_HEIGHT + (menuItemOffset * position),
        text
    );

    if (drawCursor) {
        graphics_draw_text(
            frame,
            0,
            SINGLE_PLAYER_SCREEN_TOP + SINGLE_PLAYER_SCREEN_HEIGHT + (menuItemOffset * position),
            ">"
        );
    }
}

/**
 * Handles the pause menu for given player.
 * @param state Program state.
 * @param playerNumber player in menu mode.
 */
void menuLogic(RootState* state, const unsigned char playerNumber) {
    PlayerState* playerState = &state->Players[playerNumber];

    bool pressedButtons[16] = {};
    getPressedButtons(&state->ControllerState, playerNumber, pressedButtons);

    const bool menuPressed = pressedButtons[playerState->SystemMenuButton];

    bool repaintRequired = true;

    const char cursorMax = 3;

    // First entering the loop.
    if (playerState->MenuCursorPosition == -1) {
        playerState->MenuCursorPosition = 0;
    } else if (pressedButtons[B] || menuPressed) {
        playerState->ActiveMode = Play;
    } else if (pressedButtons[Up]) {
        if (playerState->MenuCursorPosition > 0) {
            playerState->MenuCursorPosition--;
        } else {
            playerState->MenuCursorPosition = cursorMax;
        }
    } else if (pressedButtons[Down]) {
        if (playerState->MenuCursorPosition < cursorMax) {
            playerState->MenuCursorPosition++;
        } else {
            playerState->MenuCursorPosition = 0;
        }
    } else if (pressedButtons[A]) {
        switch(playerState->MenuCursorPosition) {
            default: break;
        };
    } else {
        repaintRequired = false;
    }

    if (repaintRequired) {
        state->RequiresRepaint = true;
    }
}

/**
 * Displays the pause menu for given player.
 * @param state Program state.
 * @param playerNumber player in menu mode.
 */
void menuDraw(const RootState* state, const unsigned char playerNumber) {
    // Cover menu section.
    graphics_draw_box(
        state->Frame,
        0,
        SINGLE_PLAYER_SCREEN_TOP + SINGLE_PLAYER_SCREEN_HEIGHT,
        640,
        480 - (SINGLE_PLAYER_SCREEN_TOP + SINGLE_PLAYER_SCREEN_HEIGHT),
        GLOBAL_BACKGROUND_COLOUR
    );

    TextId labels[4] = {
        TextMenuResume,
        TextMenuReset,
        TextMenuChangeCart,
        TextMenuOptions
    };

    // Draw menu items in order.
    for (unsigned char position = 0; position < 4; position++) {
        drawMenuItem(
            state->Frame,
            labels[position],
            position,
            state->Players[playerNumber].MenuCursorPosition == position
        );
    }
}
