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
    if (!isTPakInserted(playerNumber)) {
        state->Players[playerNumber].LastErrorCode = InitNoTpak;
    } else if (!isCartridgeInserted(playerNumber)) {
        state->Players[playerNumber].LastErrorCode = InitNoCartridge;
    } else if (!isCartridgeSizeOk(playerNumber)) {
        state->Players[playerNumber].LastErrorCode =  InitRequiresExpansionPak;
    } else {
        state->Players[playerNumber].LastErrorCode = InitNoError;
    }

    if (state->Players[playerNumber].LastErrorCode == InitNoError) {
        state->RequiresRepaint = true;

        readCartridge(playerNumber, &state->Players[playerNumber].Cartridge);

        Rectangle screen = {};
        getScreenPosition(state, playerNumber, &screen);

        resetPlayState(&state->Players[playerNumber]);

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
    switch (state->Players[playerNumber].LastErrorCode) {
        case InitNoError:
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
        default:
            strcpy(text, "Unknown Error!!!");
            break;
    }

    drawTextParagraph(state->Frame, text, screen.Left + TEXT_WIDTH, textTop, 0.8, screen.Width - TEXT_WIDTH);
}
