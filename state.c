#include "state.h"

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
            break;
        }
    }
}
