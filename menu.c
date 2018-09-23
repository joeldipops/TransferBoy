#include <math.h>
#include "core.h"
#include "screen.h"
#include "text.h"

#include <libdragon.h>

#define COLUMN_COUNT 2
static byte columns[COLUMN_COUNT] = {4, 2 };

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
    const byte playerNumber,
    const TextId label,
    const byte x,
    const byte y,
    const bool drawCursor,
    const Rectangle* screen
) {
    const natural xOffset = screen->Width / 2;
    const natural cursorOffset = 0;
    const float scaleFactor = 0.234 / 100.0;
    const float scale = (float)screen->Width * scaleFactor;
    const natural menuItemOffset = 34 * scale;

    // space the second column with less items in it, out a little more.
    natural top = 0;
    if (x == 1) {
        top = screen->Top + screen->Height + (menuItemOffset * y * 2);
    } else {
        top = screen->Top + screen->Height + (menuItemOffset * y);
    }
    natural left = screen->Left + (xOffset *  x);

    string text = "";
    getText(label, text);
    drawText(frame, text, left + cursorOffset, top, scale);

    if (drawCursor) {
        Rectangle border = {top, left, xOffset, menuItemOffset };
        drawSolidBorder(frame, &border, 2, SELECTED_MENU_ITEM_COLOUR);
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
byte getNewRow(const float oldRowCount, const float newRowCount, const float currentRow) {
    float ratio = oldRowCount / newRowCount;

    if (oldRowCount > newRowCount) {
        return ((byte)ceil((currentRow + 1) / ratio) - 1);
    } else {
        // If we're moving from the smaller column to the larger one, we need to appear at the top of the group.
        return (byte)(ceil((currentRow + 1) / ratio) - (ratio + 1));
    }
}

/**
 * Exits the menu by setting the player's mode back to "Play"
 * @param playerState state of the given player.
 */
void resumePlay(PlayerState* playerState) {
    playerState->MenuCursorRow = -1;
    playerState->ActiveMode = Play;
}

/**
 * Resets the emulator back to it's initial state and spins it up again.
 * @param state Program state.
 * @param playerNumber player that wants to reset.
 */
void resetGame(RootState* state, const byte playerNumber) {
    initialiseEmulator(
        &state->Players[playerNumber].EmulationState,
        &state->Players[playerNumber].Cartridge.RomData,
        &state->Players[playerNumber].Cartridge.SaveData
    );

    state->Players[playerNumber].ActiveMode = Play;
}

/**
 * Resets the given player back to cartridge load mode so they can change to a different game.
 * @param state Program state.
 * @param playerNumber Player that wants to change cartridge.
 */
void changeGame(RootState* state, const byte playerNumber) {
    // Dump Save RAM first, or nah?

    // Reset state and send back to init.
    freeByteArray(&state->Players[playerNumber].Cartridge.RomData);
    freeByteArray(&state->Players[playerNumber].Cartridge.SaveData);

    state->Players[playerNumber].MenuCursorRow = -1;
    state->Players[playerNumber].ActiveMode = Init;
    state->RequiresRepaint = true;
}

/**
 * Initialises another player with the cartridge plugged into controller slot 2
 * @param state Program state
 * @return error code
 * @private
 */
char addGame(RootState* state) {
    if (state->PlayerCount >= MAX_PLAYERS) {
        // No more players supported.
        return -2;
    }

    PlayerState* newPlayer = &state->Players[state->PlayerCount];
    generatePlayerState(newPlayer);

    state->PlayerCount++;
    return 0;
}

/**
 * Initialises another player, using the same ROM & Save as in controller slot 1
 * @param state Program state.
 * @return error code.
 * @private
 */
char addPlayer(RootState* state) {
    if (state->PlayerCount < 1) {
         // Can't copy from 1 if there is no 1
        return -1;
    } else if (state->PlayerCount >= MAX_PLAYERS) {
         // No more players supported.
        return -2;
    }

    PlayerState* newPlayer = &state->Players[state->PlayerCount];
    const PlayerState* playerOne = &state->Players[0];
    generatePlayerState(newPlayer);

    // Copy the most of the cartirdge data by reference.  It won't be changing.
    newPlayer->Cartridge.IsGbcCart = playerOne->Cartridge.IsGbcCart;
    newPlayer->Cartridge.IsSuperGbCart = playerOne->Cartridge.IsSuperGbCart;
    newPlayer->Cartridge.RomData = playerOne->Cartridge.RomData;
    strcpy(newPlayer->Cartridge.Title, playerOne->Cartridge.Title);

    // But maintain a different copy of the save file. We don't want to have two instances messing with a single save file.
    // Non-player-1 save files will never be written back to the cartridge.
    loadSave(0, &newPlayer->Cartridge.SaveData);

    initialiseEmulator(
        &newPlayer->EmulationState,
        &newPlayer->Cartridge.RomData,
        &newPlayer->Cartridge.SaveData
    );

    flushScreen(state);

    newPlayer->ActiveMode = Play;
    state->PlayerCount++;

    // Now that we've flushed, we need to put the GB screen back.
    state->Frame = 1;
    playDraw(state, 0);
    state->Frame = 2;
    playDraw(state, 0);
    state->Frame = 0;
    return 0;
}

/**
 * Carries out the option selected from the menu.
 * @param state program state.
 * @param playerNumber number of player selecting an option.
 * @param x column of selected item.
 * @param y row of selected item.
 */
void executeMenuItem(RootState* state, const byte playerNumber, const byte x, const byte y) {
    typedef enum { Resume, Reset, Change, Options, AddPlayer, AddGame } items;

    byte position = 0;
    bool done = false;
    for(byte col = 0; col <= x && !done; col++) {
        for(byte row = 0; row < columns[col] && !done; row++) {
            if (col == x && row == y) {
                done = true;
            } else {
                position++;
            }
        }
    }

    switch((items)position) {
        case Resume:
            resumePlay(&state->Players[playerNumber]);
            break;
        case Reset:
            resetGame(state, playerNumber);
            break;
        case Change:
            changeGame(state, playerNumber);
            break;
        case AddPlayer:
            addPlayer(state);
            break;
        case AddGame:
            addGame(state);
            break;
        default: ; break;
    }
}

/**
 * Handles the pause menu for given player.
 * @param state Program state.
 * @param playerNumber player in menu mode.
 */
void menuLogic(RootState* state, const byte playerNumber) {
    PlayerState* playerState = &state->Players[playerNumber];

    bool pressedButtons[N64_BUTTON_COUNT] = {};
    getPressedButtons(&state->KeysReleased, playerNumber, pressedButtons);

    const bool menuPressed = pressedButtons[playerState->SystemMenuButton];

    bool repaintRequired = true;
    bool ctrlReadRequired = true;

    byte column = playerState->MenuCursorColumn;

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
        playerState->MenuCursorRow = getNewRow(columns[column], columns[(byte)playerState->MenuCursorColumn], playerState->MenuCursorRow);
    } else if (pressedButtons[Right]) {
        if (playerState->MenuCursorColumn < COLUMN_COUNT - 1) {
            playerState->MenuCursorColumn++;
        } else {
            playerState->MenuCursorColumn = 0;
        }
        playerState->MenuCursorRow = getNewRow(columns[column], columns[(byte)playerState->MenuCursorColumn], playerState->MenuCursorRow);

    // BACK, SELECT ETC,
    } else if (pressedButtons[B] || menuPressed) {
        resumePlay(playerState);
    } else if (pressedButtons[A]) {
        executeMenuItem(state, playerNumber, playerState->MenuCursorColumn, playerState->MenuCursorRow);
    } else {
        repaintRequired = false;
        ctrlReadRequired = false;
    }

    state->RequiresRepaint |= repaintRequired;
    state->RequiresControllerRead |= ctrlReadRequired;
}

/**
 * Displays the pause menu for given player.
 * @param state Program state.
 * @param playerNumber player in menu mode.
 */
void menuDraw(RootState* state, const byte playerNumber) {
    Rectangle screen = {};
    getScreenPosition(state, playerNumber, &screen);

    if (state->Players[playerNumber].ActiveMode != Menu) {
        flushScreen(state);
        state->RequiresRepaint = true;
        return;
    }

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

    byte position = 0;
    for(byte x = 0; x < COLUMN_COUNT; x++) {
        for(byte y = 0; y < columns[x]; y++) {
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
