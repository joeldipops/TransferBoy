#include "state.h"

#include <string.h>

/**
 * Sets up initial configuration for N64 - Gameboy button map.
 * @out buttonMap map of N64 to Gameboy buttons.
 * @private
 */
void initialiseButtonMap(GbButton* map) {
    GbButton buttons[N64_BUTTON_COUNT];
    memset(buttons, 0x00, sizeof(GbButton) * N64_BUTTON_COUNT);
    buttons[NoButton] = GbNoButton;
    buttons[A] = GbA;
    buttons[B] = GbB;
    buttons[Up] = GbUp;
    buttons[Down] = GbDown;
    buttons[Left] = GbLeft;
    buttons[Right] = GbRight;

    // Start, Select and SystemMenu should be configurable
    buttons[Start] = GbStart;
    buttons[R] = GbSelect;
    buttons[L] = GbSystemMenu;

    memcpy(map, &buttons, sizeof(GbButton) * N64_BUTTON_COUNT);
}

/**
 * Sets a Gameboy button to a particular N64 button, unsetting it from the previous mapping.
 * @param playerState state containing controller mapping to update.
 * @param gbButton gameboy button to set.
 * @param n64Button n64 button to set gb button to.
 */
void setButtonToMap(PlayerState* playerState, const GbButton gbButton, const N64Button n64Button) {
    // Unset old mapping
    for (byte i = 0; i < N64_BUTTON_COUNT; i++) {
        if (playerState->ButtonMap[i] == gbButton) {
            playerState->ButtonMap[i] = GbNoButton;
        }
    }

    // Set new mapping.
    playerState->ButtonMap[n64Button] = gbButton;
    if (gbButton == GbSystemMenu) {
        playerState->SystemMenuButton = n64Button;
    }
}

/**
 * Initialises a new player state struct with default values.
 * @out playerState PlayerState struct to initialise.
 */
void generatePlayerState(PlayerState* playerState) {
    playerState->SelectedBorder = BorderNone;
    playerState->AudioEnabled = true;
    playerState->ActiveMode = Init;
    playerState->MenuCursorRow = -1;
    playerState->MenuCursorColumn = 0;
    initialiseButtonMap(playerState->ButtonMap);

    for (byte i = 0; i < N64_BUTTON_COUNT; i++) {
        if (playerState->ButtonMap[i] == GbSystemMenu) {
            playerState->SystemMenuButton = i;
        } else if (playerState->ButtonMap[i] == GbStart) {
            playerState->GbStartButton = i;
        } else if (playerState->ButtonMap[i] == GbSelect) {
            playerState->GbSelectButton = i;
        }
    }
}
