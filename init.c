#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "init.h"
#include "screen.h"
#include "play.h"
#include "controller.h"
#include "tpakio.h"
#include "resources.h"
#include "logger.h"
#include "text.h"
#include "hwdefs.h"
#include <libdragon.h>

/**
 * Gets screen and state ready for Playing GB software.
 * @param state The play state.
 * @param playerNumber identifies player that wants to move to play mode.
 */
static void preparePlayMode(RootState* state, byte playerNumber) {
    Rectangle screen = {};
    getScreenPosition(state, playerNumber, &screen);
    resetPlayState(&state->Players[playerNumber]);
    state->Players[playerNumber].InitState = InitLoaded;
    state->Players[playerNumber].ActiveMode = Play;
}

/**
 * Waits for a Start Button pressed, then goes and loads a rom.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initLogic(RootState* state, const byte playerNumber) {
    PlayerState* playerState = &state->Players[playerNumber];
    GbState* s = &playerState->EmulationState;

    if (playerState->InitState == InitRestarting) {
        playerState->InitState = InitStart;
        state->RequiresRepaint = true;
    }

    if (playerState->InitState == InitStart) {
        sByte result = getCartridgeMetadata(playerNumber, &playerState->EmulationState.Cartridge);

        if (!result) {
            playerState->InitState = InitReady;
            state->RequiresRepaint = true;
        } else {
            playerState->InitState = InitError;
            state->ErrorCode = result;

            switch(result) {
                case TPAK_ERR_NO_TPAK:
                    getText(TextNoTpak, playerState->ErrorMessage);
                    break;
                case TPAK_ERR_NO_CARTRIDGE:
                    getText(TextNoCartridge, playerState->ErrorMessage);
                    break;
                case TPAK_ERR_CORRUPT_HEADER:
                case TPAK_ERR_CORRUPT_DATA:
                    getText(TextChecksumFailed, playerState->ErrorMessage);
                    break;
                case TPAK_ERR_UNSUPPORTED_CARTRIDGE:
                    getText(TextUnsupportedCartridge, playerState->ErrorMessage);
                    break;
                case TPAK_ERR_INSUFFICIENT_MEMORY:
                    getText(TextExpansionPakRequired, playerState->ErrorMessage);
                    break;
                default:
                    sprintf(playerState->ErrorMessage, "Loading Cartridge failed with error: %d", state->ErrorCode);
                    break;
            }
        }
    }

    if (playerState->InitState == InitError) {
        bool releasedButtons[N64_BUTTON_COUNT] = {};
        getPressedButtons(&state->KeysReleased, playerNumber, releasedButtons);

        // Press A or Start to retry.
        if (releasedButtons[A] || releasedButtons[Start]) {
            state->RequiresControllerRead = true;
            state->RequiresRepaint = true;
            state->Players[playerNumber].InitState = InitRestarting;
        } else if (releasedButtons[CDown]) {
            // Load internal easter egg cartridge.
            state->RequiresControllerRead = true;
            state->RequiresRepaint = true;
            sInt result = loadInternalRom(&state->Players[0].EmulationState.Cartridge.Rom);
            if (result == -1) {
                logAndPauseFrame(0, "Error loading internal ROM");
            }

            s->Cartridge.Type = ROM_ONLY;
            s->Cartridge.Header.IsSgbSupported = true;
            s->Cartridge.IsGbcSupported = false;
            s->Cartridge.RomBankCount = s->Cartridge.Rom.Size / ROM_BANK_SIZE;
            s->Cartridge.RamBankCount = 1;
            s->Cartridge.Ram.Size = 0;

            preparePlayMode(state, playerNumber);
        }
    } else if (playerState->InitState == InitReady) {
        // Show the loading message.
        state->RequiresRepaint = true;
        state->Players[playerNumber].InitState = InitLoading;
    } else if (playerState->InitState == InitLoading) {
        state->RequiresRepaint = true;

        sByte result = importCartridge(playerNumber, &s->Cartridge);
        if (result) {
            string tmp;
            state->ErrorCode = result;
            getText(TextChecksumFailed, tmp);
            sprintf(playerState->ErrorMessage, tmp, result);
            playerState->InitState = InitError;
            return;
        }

        preparePlayMode(state, playerNumber);
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
    string tmp = "";
    switch (state->Players[playerNumber].InitState) {
        case InitStart: break;
        case InitLoaded: break;
        case InitReady:
        case InitRestarting:
        case InitLoading:
            getText(TextLoadingCartridge, text);
            break;
        case InitError:
            getText(TextLoadCartridgePrompt, tmp);
            sprintf(text, "%s %s", state->Players[playerNumber].ErrorMessage, tmp);
            break;
        default:
            strcpy(text, "Unknown Error!!!");
            break;
    }

    drawTextParagraph(state->Frame, text, screen.Left + TEXT_WIDTH, textTop, 0.8, screen.Width - TEXT_WIDTH);

    // sleep just to give some indiciation that something has changed.
    // otherwise it may happen too fast and we might not know for sure.
    // that the t-pak was even retried.
    if (state->Players[playerNumber].InitState == InitRestarting) {
        sleep(1);
    }
}
