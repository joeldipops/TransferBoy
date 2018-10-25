#include <stdio.h>
#include "init.h"
#include "tpakio.h"
#include "screen.h"
#include "play.h"
#include "controller.h"
#include <libdragon.h>
#include "include/gbc_bundle.h"

typedef enum {
    InitStart = 0,
    InitNoTpak, InitNoCartridge, InitRequiresExpansionPak,
    InitPending, InitReady, InitLoading, InitLoaded
} InitState;

/**
 * Waits for a Start Button pressed, then goes and loads a rom.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initLogic(RootState* state, const byte playerNumber) {
    if (state->Players[playerNumber].InitState == InitStart) {
        if (!isTPakInserted(playerNumber)) {
            state->Players[playerNumber].InitState = InitNoTpak;
        } else if (!isCartridgeInserted(playerNumber)) {
            state->Players[playerNumber].InitState = InitNoCartridge;
        } else if (!isCartridgeSizeOk(playerNumber)) {
            state->Players[playerNumber].InitState =  InitRequiresExpansionPak;
        } else {
            state->Players[playerNumber].InitState = InitPending;
            state->RequiresRepaint = true;
        }
    }

    if (state->Players[playerNumber].InitState == InitPending) {
        getCartridgeMetaData(playerNumber, &state->Players[playerNumber].Cartridge);

        bool releasedButtons[N64_BUTTON_COUNT] = {};
        getPressedButtons(&state->KeysReleased, playerNumber, releasedButtons);

        if (releasedButtons[A] /*|| releasedButtons[Start]*/) {
            state->RequiresControllerRead = true;
            state->RequiresRepaint = true;
            state->Players[playerNumber].InitState = InitReady;
        } else if (releasedButtons[B]) {
            state->RequiresControllerRead = true;
            state->Players[playerNumber].InitState = InitStart;
        }
    } else if (state->Players[playerNumber].InitState == InitReady) {
        // Show the loading message.
        state->RequiresRepaint = true;
        state->Players[playerNumber].InitState = InitLoading;
    }

    if (state->Players[playerNumber].InitState == InitLoading) {
        state->RequiresRepaint = true;

        readCartridge(playerNumber, &state->Players[playerNumber].Cartridge);

        Rectangle screen = {};
        getScreenPosition(state, playerNumber, &screen);

        resetPlayState(&state->Players[playerNumber]);

        state->Players[playerNumber].InitState = InitLoaded;
        state->Players[playerNumber].ActiveMode = Play;
    }
}

/**
 * Draws screen for init mode.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initDraw(const RootState* state, const byte playerNumber) {
    Rectangle screen = {};
    getScreenPosition(state, playerNumber, &screen);

    prepareRdpForSprite(state->Frame);
    loadSprite(getSpriteSheet(), GB_BG_TEXTURE, MIRROR_ENABLED);
    rdp_draw_textured_rectangle(0,  screen.Left, screen.Top, screen.Left +  screen.Width, screen.Top + screen.Height);

    const char TEXT_HEIGHT = 100;
    const char TEXT_WIDTH = 10;
    natural textTop = screen.Top - TEXT_HEIGHT + (screen.Width / 2);

    string text = "";
    switch (state->Players[playerNumber].InitState) {
        case InitStart: break;
        case InitLoaded: break;
        case InitReady:
            getText(TextLoadingCartridge, text);
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
        case InitPending:
            ;
            string tmp = "";
            getText(TextLoadCartridgePrompt, tmp);
            sprintf(text, tmp, &state->Players[playerNumber].Cartridge.Title);
            break;
        default:
            strcpy(text, "Unknown Error!!!");
            break;
    }

    drawTextParagraph(state->Frame, text, screen.Left + TEXT_WIDTH, textTop, 0.8, screen.Width - TEXT_WIDTH);
}
