#include <stdio.h>
#include "init.h"
#include "tpakio.h"
#include "screen.h"
#include "play.h"
#include <libdragon.h>
#include "include/gbc_bundle.h"

typedef enum { InitNoError, InitNoTpak, InitNoCartridge, InitRequiresExpansionPak } InitError;

/**
 * Waits for a Start Button pressed, then goes and loads a rom.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initLogic(RootState* state, const byte playerNumber) {
    if (!state->ControllerState.c[playerNumber].start) {
        if (!isTPakInserted(playerNumber)) {
            state->Players[playerNumber].LastErrorCode = InitNoTpak;
        } else if (!isCartridgeInserted(playerNumber)) {
            state->Players[playerNumber].LastErrorCode = InitNoCartridge;
        } else if (!isCartridgeSizeOk(playerNumber)) {
            state->Players[playerNumber].LastErrorCode =  InitRequiresExpansionPak;
        } else {
            state->Players[playerNumber].LastErrorCode =  InitNoError;
        }
    } else {
        state->RequiresRepaint = true;

        readCartridge(playerNumber, &state->Players[playerNumber].Cartridge);

        ScreenPosition screen = {};
        getScreenPosition(state, playerNumber, &screen);

        initialiseEmulator(
            &state->Players[playerNumber].EmulationState,
            &state->Players[playerNumber].Cartridge.RomData,
            &state->Players[playerNumber].Cartridge.SaveData
        );

        state->Players[playerNumber].ActiveMode = Play;
    }
}

/**
 * Draws screen for init mode.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initDraw(const RootState* state, const byte playerNumber) {
    ScreenPosition screen = {};
    getScreenPosition(state, playerNumber, &screen);

    graphics_draw_box(state->Frame, screen.Left, screen.Top, screen.Width, screen.Height, BLANK_SCREEN_COLOUR);

    const char TEXT_HEIGHT = 30;
    const char TEXT_WIDTH = 10;
    natural textTop = screen.Top - TEXT_HEIGHT + (screen.Width / 2);
    natural textLeft = screen.Left + TEXT_WIDTH;

    string text = "";
    switch (state->Players[playerNumber].LastErrorCode) {
        case InitNoError:
            getText(TextLoadCartridge, text);
            break;
        case InitNoTpak:
            getText(TextNoTpak, text);
            break;
        case InitNoCartridge:
            getText(TextNoCartridge, text);
            break;
        case InitRequiresExpansionPak:
            getText(TextExpansionPakRequired, text);
            break;
        default:
            strcpy(text, "Unknown Error!!!");
            break;
    }

    //drawText(state->Frame, text, textLeft, textTop);
    graphics_draw_text(state->Frame, textLeft, textTop + 30, text);
}
