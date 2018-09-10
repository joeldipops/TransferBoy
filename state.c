#include "state.h"

void generatePlayerState(PlayerState* playerState) {
    playerState->SelectedBorder = BorderNone;
    playerState->AudioEnabled = true;
    playerState->ActiveMode = Init;
    playerState->MenuCursorRow = -1;
    playerState->MenuCursorColumn = 0;
    initialiseButtonMap(playerState->ButtonMap);

    for (int i = 0; i < 16; i++) {
        if (playerState->ButtonMap[i] == GbSystemMenu) {
            playerState->SystemMenuButton = i;
            break;
        }
    }
}
