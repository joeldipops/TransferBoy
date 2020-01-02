#include <math.h>
#include <stdio.h>
#include "core.h"
#include "screen.h"
#include "text.h"
#include "resources.h"
#include "init.h"
#include "play.h"
#include "config.h"

#include <libdragon.h>

#define MAX_COLUMN_COUNT 2
#define MAX_ROW_COUNT 4

/**
 * Draws a menu item.
 * @param frame Id of frame to render to.
 * @param label identifies the label of the menu item.
 * @param position Ordinal position of the menu item.
 * @param drawCursor If true, add the cursor to the menu item.
 * @param disabled If true, label will be drawn in a muted colour.
 * @param screen The size/position of the gameboy screen.
 * @private
 */
static void drawMenuItem(
    const display_context_t frame,
    const byte playerNumber,
    const TextId label,
    const byte x,
    const byte y,
    const bool drawCursor,
    const bool disabled,
    const Rectangle* screen
) {
    const natural xOffset = screen->Width / 2;
    const float scale = (float)screen->Width * TEXT_SCALE_FACTOR;
    const natural menuItemOffset = 34 * scale;

    // space the second column with less items in it, out a little more.
    natural top = 0;
    if (x == 1) {
        top = screen->Top + screen->Height + (menuItemOffset * y * 2);
    } else {
        top = screen->Top + screen->Height + (menuItemOffset * y);
    }
    natural left = screen->Left + (xOffset *  x);

    if (!disabled || !HIDE_DISABLED_OPTIONS) {
        string text = "";
        getText(label, text);

        if (disabled) {
            string tmp;
            sprintf(tmp, "~%s", text);
            strcpy(text, tmp);
        }

        drawText(frame, text, left, top, scale);
    }

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
static byte getNewRow(const float oldRowCount, const float newRowCount, const float currentRow) {
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
 * @private
 */
static void resumePlay(PlayerState* playerState) {
    playerState->MenuCursorRow = -1;
    rootState.RequiresRepaint = false;
    playerState->ActiveMode = Play;
}

/**
 * Exits the main menu and brings up the options menu.
 * @param playerState state of the player going to the options menu.
 * @private
 */
static void showOptionsMenu(PlayerState* playerState) {
    playerState->MenuCursorRow = -1;
    playerState->ActiveMode = Options;
}

/**
 * Resets the emulator back to it's initial state and spins it up again.
 * @param state Program state.
 * @param playerNumber player that wants to reset.
 */
static  void resetGame(const byte playerNumber) {
    resetPlayState(&rootState.Players[playerNumber]);

    rootState.Players[playerNumber].ActiveMode = Play;
}

/**
 * Resets the given player back to cartridge load mode so they can change to a different game.
 * @param state Program state.
 * @param playerNumber Player that wants to change cartridge.
 */
static void changeGame(const byte playerNumber) {
    // Dump Save RAM first, or nah?

    // Reset state and send back to init.
    freeByteArray(&rootState.Players[playerNumber].EmulationState.Cartridge.Rom);
    freeByteArray(&rootState.Players[playerNumber].EmulationState.Cartridge.Ram);

    rootState.Players[playerNumber].MenuCursorRow = -1;
    rootState.Players[playerNumber].ActiveMode = Init;
    rootState.Players[playerNumber].InitState = InitChanging;
    rootState.RequiresRepaint = true;
}

/**
 * Initialises another player with the cartridge plugged into controller slot 2
 * @return error code
 * @private
 */
static char addGame() {
    if (rootState.PlayerCount >= MAX_PLAYERS) {
        // No more players supported.
        return -2;
    }

    PlayerState* newPlayer = &rootState.Players[rootState.PlayerCount];
    generatePlayerState(newPlayer);

    rootState.PlayerCount++;
    return 0;
}

/**
 * Initialises another player, using the same ROM & Save as in controller slot 1
 * @return error code.
 * @private
 */
static char addPlayer() {
    if (rootState.PlayerCount < 1) {
         // Can't copy from 1 if there is no 1
        return -1;
    } else if (rootState.PlayerCount >= MAX_PLAYERS) {
         // No more players supported.
        return -2;
    }

    PlayerState* newPlayer = &rootState.Players[rootState.PlayerCount];
    const GameBoyCartridge* playerOne = &rootState.Players[0].EmulationState.Cartridge;
    generatePlayerState(newPlayer);

    // Copy the most of the cartridge data by reference.  It won't be changing.
    newPlayer->EmulationState.Cartridge.IsGbcSupported = playerOne->IsGbcSupported;
    newPlayer->EmulationState.Cartridge.Header = playerOne->Header;
    newPlayer->EmulationState.Cartridge.Rom = playerOne->Rom;

    // But maintain a different copy of the save file. We don't want to have two instances messing with a single save file.
    // Non-player-1 save files will never be written back to the cartridge.
    importCartridgeRam(0, &newPlayer->EmulationState.Cartridge);

    resetPlayState(newPlayer);

    flushScreen();

    newPlayer->ActiveMode = Play;
    rootState.PlayerCount++;

    // Now that we've flushed, we need to put the GB screen back.
    playDraw(0);
    return 0;
}

/**
 * Carries out the option selected from the menu.
  * @param playerNumber number of player selecting an option.
 * @param x column of selected item.
 * @param y row of selected item.
 */
static void executeMenuItem(const byte playerNumber, const byte x, const byte y) {
    typedef enum { Resume, Reset, Change, Options, AddPlayer, AddGame } items;

    byte position = 0;
    bool done = false;
    for(byte col = 0; col <= x && !done; col++) {
        for(byte row = 0; row < rootState.Players[playerNumber].MenuLayout[col] && !done; row++) {
            if (col == x && row == y) {
                done = true;
            } else {
                position++;
            }
        }
    }

    switch((items)position) {
        case Resume:
            resumePlay(&rootState.Players[playerNumber]);
            break;
        case Reset:
            resetGame(playerNumber);
            break;
        case Change:
            changeGame(playerNumber);
            break;
        case AddPlayer:
            addPlayer();
            break;
        case AddGame:
            addGame();
            break;
        case Options:
            showOptionsMenu(&rootState.Players[playerNumber]);
            break;
        default: ; break;
    }
}

/**
 * Handles the pause menu for given player.
 * @param state Program state.
 * @param playerNumber player in menu mode.
 */
void menuLogic(const byte playerNumber) {
    PlayerState* playerState = &rootState.Players[playerNumber];

    playerState->MenuLayout[0] = 4;

    // If next controller not plugged in, disable add player & add game options.
    if (rootState.PlayerCount > 1 || !(rootState.ControllersPresent & (CONTROLLER_2_INSERTED))) {
        playerState->MenuLayout[1] = 0;
        // Can't be in the second column if there is no second column.
        playerState->MenuCursorColumn = 0;
    } else {
        playerState->MenuLayout[1] = 2;
    }

    bool pressedButtons[N64_BUTTON_COUNT] = {};
    getPressedButtons(&rootState.KeysReleased, playerNumber, pressedButtons);

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
            playerState->MenuCursorRow = playerState->MenuLayout[column] - 1;
        }
    } else if (pressedButtons[Down]) {
        if (playerState->MenuCursorRow < playerState->MenuLayout[column] - 1) {
            playerState->MenuCursorRow++;
        } else {
            playerState->MenuCursorRow = 0;
        }

    // LEFT AND RIGHT
    } else if (pressedButtons[Left]) {
        if (playerState->MenuCursorColumn > 0) {
            playerState->MenuCursorColumn--;
        } else {
            // Don't move the cursor if there are no rows next door.
            if (playerState->MenuLayout[MAX_COLUMN_COUNT - 1]) {
                playerState->MenuCursorColumn = MAX_COLUMN_COUNT - 1;
            } else {
                return;
            }
        }
        playerState->MenuCursorRow = getNewRow(
            playerState->MenuLayout[column], 
            playerState->MenuLayout[(byte)playerState->MenuCursorColumn], 
            playerState->MenuCursorRow
        );
    } else if (pressedButtons[Right]) {
        if (playerState->MenuCursorColumn < MAX_COLUMN_COUNT - 1) {
            // Don't move the cursor if there are no rows next door.
            if (playerState->MenuLayout[column + 1]) {
                playerState->MenuCursorColumn++;
            } else {
                return;
            }
        } else {
            playerState->MenuCursorColumn = 0;
        }
        playerState->MenuCursorRow = getNewRow(
            playerState->MenuLayout[column], 
            playerState->MenuLayout[(byte)playerState->MenuCursorColumn], 
            playerState->MenuCursorRow
        );

    // BACK, SELECT ETC,
    } else if (pressedButtons[B] || menuPressed) {
        resumePlay(playerState);
    } else if (pressedButtons[A]) {
        executeMenuItem(playerNumber, playerState->MenuCursorColumn, playerState->MenuCursorRow);
    } else {
        repaintRequired = false;
        ctrlReadRequired = false;
    }

    rootState.RequiresRepaint |= repaintRequired;
    rootState.RequiresControllerRead |= ctrlReadRequired;
}

/**
 * Displays the pause menu for given player.
 * @param state Program state.
 * @param playerNumber player in menu mode.
 */
void menuDraw(const byte playerNumber) {
    Rectangle screen = {};
    getScreenPosition(playerNumber, &screen);

    prepareRdpForSprite(rootState.Frame);
    loadSprite(getSpriteSheet(), BLUE_BG_TEXTURE, MIRROR_XY);

    // Cover menu section.
    rdp_draw_textured_rectangle(
        0,
        0,
        screen.Top + screen.Height,
        RESOLUTION_X,
        RESOLUTION_Y - screen.Top + screen.Height,
        MIRROR_DISABLED
    );

    if (rootState.Players[playerNumber].ActiveMode != Menu) {
        rootState.RequiresRepaint = true;
        return;
    }

    TextId labels[MAX_COLUMN_COUNT * MAX_ROW_COUNT] = {
        TextMenuResume,
        TextMenuReset,
        TextMenuChangeCart,
        TextMenuOptions,
        TextMenuAddPlayer,
        TextMenuAddGame
    };

    // Draw menu items in order.
    byte position = 0;
    for(byte x = 0; x < MAX_COLUMN_COUNT; x++) {
        for(byte y = 0; y < MAX_ROW_COUNT; y++) {
            drawMenuItem(
                rootState.Frame,
                playerNumber,
                labels[position],
                x,
                y,
                rootState.Players[playerNumber].MenuCursorRow == y && rootState.Players[playerNumber].MenuCursorColumn == x,
                y >= rootState.Players[playerNumber].MenuLayout[x],
                &screen
            );
            position++;
        }
    }

    rdp_detach_display();
}
