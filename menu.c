#include <math.h>
#include "core.h"
#include <libdragon.h>

#define COLUMN_COUNT 2
static unsigned char columns[COLUMN_COUNT] = {4, 2 };

/**
 * Draws a menu item.
 * @param frame Id of frame to render to.
 * @param label identifies the label of the menu item.
 * @param position Ordinal position of the menu item.
 * @param drawCursor If true, add the cursor to the menu item.
 * @param screen The size/position of the gameboy screen.
 * @private
 */
void drawMenuItem(
    const display_context_t frame,
    const unsigned char playerNumber,
    const TextId label,
    const unsigned char x,
    const unsigned char y,
    const bool drawCursor,
    const ScreenPosition* screen
) {
    const unsigned short menuItemOffset = 10;
    const unsigned short xOffset = 150;
    const unsigned short cursorOffset = 20;

    // space the second column with less items in it, out a little more.
    unsigned short top = 0;
    if (x == 1) {
        top = screen->Top + screen->Height + (menuItemOffset * y * 2);
    } else {
        top = screen->Top + screen->Height + (menuItemOffset * y);
    }
    unsigned short left = screen->Left + (xOffset *  x);

    string text = "";
    getText(label, text);
    graphics_draw_text(frame, left + cursorOffset, top, text);

    if (drawCursor) {
        graphics_draw_text(frame, left, top, ">");
    }
}

/**
 * When moving between column, calculates the new row position.
 * @param oldRowCount number of items in previous column.
 * @param newRowCount nember of items in new column.
 * @param currentRow current position of the cursor.
 * @return New position in the new column.
 * @private
 */
unsigned char getNewRow(const float oldRowCount, const float newRowCount, float currentRow) {
    float ratio = oldRowCount / newRowCount;

    if (oldRowCount > newRowCount) {
        return ((unsigned char)ceil((currentRow + 1) / ratio) - 1);
    } else {
        // If we're moving from the smaller column to the larger one, we need to appear at the top of the group.
        return (unsigned char)(ceil((currentRow + 1) / ratio) - (ratio + 1));
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

    unsigned char column = playerState->MenuCursorColumn;

    // First entering the loop.
    if (playerState->MenuCursorRow == -1) {
        playerState->MenuCursorRow = 0;
    // UP AND DOWN
    } else if (pressedButtons[Up]) {
        if (playerState->MenuCursorRow > 0) {
            playerState->MenuCursorRow--;
        } else {
            playerState->MenuCursorRow = columns[column] - 1;
        }
    } else if (pressedButtons[Down]) {
        if (playerState->MenuCursorRow < columns[column] - 1) {
            playerState->MenuCursorRow++;
        } else {
            playerState->MenuCursorRow = 0;
        }

    // LEFT AND RIGHT
    } else if (pressedButtons[Left]) {
        if (playerState->MenuCursorColumn > 0) {
            playerState->MenuCursorColumn--;
        } else {
            playerState->MenuCursorColumn = COLUMN_COUNT - 1;
        }
        playerState->MenuCursorRow = getNewRow(columns[column], columns[(unsigned char)playerState->MenuCursorColumn], playerState->MenuCursorRow);
    } else if (pressedButtons[Right]) {
        if (playerState->MenuCursorColumn < COLUMN_COUNT - 1) {
            playerState->MenuCursorColumn++;
        } else {
            playerState->MenuCursorColumn = 0;
        }
        playerState->MenuCursorRow = getNewRow(columns[column], columns[(unsigned char)playerState->MenuCursorColumn], playerState->MenuCursorRow);

    // BACK, SELECT ETC,
    } else if (pressedButtons[B] || menuPressed) {
        playerState->MenuCursorRow = -1;
        playerState->ActiveMode = Play;
    } else if (pressedButtons[A]) {
        executeMenuItem(playerState->MenuCursorRow, playerState->MenuCursorColumn);
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
    ScreenPosition screen = {};
    getScreenPosition(state, playerNumber, &screen);

    // Cover menu section.
    graphics_draw_box(
        state->Frame,
        0,
        screen.Top + screen.Height,
        RESOLUTION_X,
        RESOLUTION_Y - (screen.Top + screen.Height),
        GLOBAL_BACKGROUND_COLOUR
    );

    TextId labels[6] = {
        TextMenuResume,
        TextMenuReset,
        TextMenuChangeCart,
        TextMenuOptions,
        TextMenuAddPlayer,
        TextMenuAddGame
    };

    // Draw menu items in order.

    unsigned char position = 0;
    for(unsigned char x = 0; x < COLUMN_COUNT; x++) {
        for(unsigned char y = 0; y < columns[x]; y++) {
            drawMenuItem(
                state->Frame,
                playerNumber,
                labels[position],
                x,
                y,
                state->Players[playerNumber].MenuCursorRow == y && state->Players[playerNumber].MenuCursorColumn == x,
                &screen
            );
            position++;
        }
    }
}
