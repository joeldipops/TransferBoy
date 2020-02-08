#include <stdio.h>
#include <string.h>
#include "init.h"
#include "screen.h"
#include "play.h"
#include "controller.h"
#include "tpakio.h"
#include "resources.h"
#include "logger.h"
#include "text.h"
#include "hwdefs.h"
#include "progressBar.h"
#include <libdragon.h>


/**
 * Gets screen and state ready for Playing GB software.
 * @param state The play state.
 * @param playerNumber identifies player that wants to move to play mode.
 */
static void preparePlayMode(byte playerNumber) {
    Rectangle screen = {};
    getScreenPosition(playerNumber, &screen);
    resetPlayState(&rootState.Players[playerNumber]);
    rootState.Players[playerNumber].EmulationState.controllerSlot = playerNumber;
    rootState.Players[playerNumber].InitState = InitLoaded;
    rootState.Players[playerNumber].ActiveMode = Play;
}

/**
 * Waits for a Start Button pressed, then goes and loads a rom.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initLogic(const byte playerNumber) {
    PlayerState* playerState = &rootState.Players[playerNumber];
    GbState* s = &playerState->EmulationState;

    if (playerState->InitState == InitRestarting) {
        playerState->InitState = InitStart;
        rootState.RequiresRepaint = true;
    }

    if (playerState->InitState == InitStart) {
        sByte result = getCartridgeMetadata(playerNumber, &playerState->EmulationState.Cartridge);

        if (!result) {
            playerState->InitState = InitReady;
            rootState.RequiresRepaint = true;
        } else {
            playerState->InitState = InitError;
            rootState.ErrorCode = result;

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
                    sprintf(playerState->ErrorMessage, "Loading Cartridge failed with error: %d", rootState.ErrorCode);
                    break;
            }
        }
    }

    if (playerState->InitState == InitError || playerState->InitState == InitChanging) {
        bool releasedButtons[N64_BUTTON_COUNT] = {};
        getPressedButtons(&rootState.KeysReleased, playerNumber, releasedButtons);

        // Press A or Start to retry.
        if (releasedButtons[A] || releasedButtons[Start]) {
            rootState.RequiresControllerRead = true;
            rootState.RequiresRepaint = true;
            rootState.Players[playerNumber].InitState = InitRestarting;
        } else if (releasedButtons[CDown]) {
            // Load internal easter egg cartridge.
            rootState.RequiresControllerRead = true;
            rootState.RequiresRepaint = true;
            sInt result = loadInternalRom(&rootState.Players[0].EmulationState.Cartridge.Rom);
            if (result == -1) {
                logAndPauseFrame(0, "Error loading internal ROM");
            }

            s->Cartridge.Type = ROM_ONLY;
            s->Cartridge.Header.is_sgb_supported = true;
            s->Cartridge.IsGbcSupported = false;
            s->Cartridge.RomBankCount = s->Cartridge.Rom.Size / ROM_BANK_SIZE;
            s->Cartridge.RamBankCount = 1;
            s->Cartridge.Ram.Size = 0;

            preparePlayMode(playerNumber);
        }
    } else if (playerState->InitState == InitReady) {
        // Show the loading message.
        rootState.RequiresRepaint = true;
        rootState.Players[playerNumber].InitState = InitLoading;
    } else if (playerState->InitState == InitLoading) {
        rootState.RequiresRepaint = true;

        startLoadProgressTimer(playerNumber);

        sByte result = importCartridge(playerNumber, &s->Cartridge);

        closeLoadProgressTimer(playerNumber);

        if (result) {
            string tmp;
            rootState.ErrorCode = result;
            getText(TextChecksumFailed, tmp);
            sprintf(playerState->ErrorMessage, tmp, result);
            playerState->InitState = InitError;
            return;
        }

        preparePlayMode(playerNumber);
    }
}

/**
 * Draws screen for init mode.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initDraw(const byte playerNumber) {
    Rectangle screen = {};
    getScreenPosition(playerNumber, &screen);

    prepareRdpForSprite(rootState.Frame);
    loadSprite(getSpriteSheet(), GB_BG_TEXTURE, MIRROR_XY);

    // The - 1 and - 8 crept in when I switched from software to hardware rendering.  The RDP has some strange behaviours I can't understand.
    // If I don't subtract these here, there are grey lines along the right and bottom of the play screen in single player mode.
    rdp_draw_textured_rectangle(0,  screen.Left, screen.Top, screen.Left + screen.Width - 1, screen.Top + screen.Height, true);

    natural textTop = screen.Top - TEXT_HEIGHT + (screen.Width / 2);

    string text = "";
    string tmp = "";
    switch (rootState.Players[playerNumber].InitState) {
        case InitStart: break;
        case InitLoaded: break;
        case InitReady:
        case InitRestarting:
        case InitLoading:
            getText(TextLoadingCartridge, text);
            break;
        case InitChanging: 
            getText(TextLoadCartridge, text);
            break;
        case InitError:
            getText(TextRetryCartridgePrompt, tmp);
            sprintf(text, "%s %s", rootState.Players[playerNumber].ErrorMessage, tmp);
            break;
        default:
            strcpy(text, "Unknown Error!!!");
            break;
    }

    drawTextParagraph(rootState.Frame, text, screen.Left + TEXT_WIDTH, textTop, 0.8, screen.Width - TEXT_WIDTH);

    // sleep just to give some indiciation that something has changed.
    // otherwise it may happen too fast and we might not know for sure.
    // that the t-pak was even retried.
    if (rootState.Players[playerNumber].InitState == InitRestarting) {
        // Seems to be some issue with this function, maybe when combined with my use of timer interrupts in this module.
        // This function never finishes executing.
        // So commenting out for now as it's not super important.
        //wait_ms(500);
    }
}
