#include <stdio.h>
#include "init.h"
#include "screen.h"
#include "play.h"
#include "controller.h"
#include "tpakio.h"
#include <libdragon.h>
#include "include/gbc_bundle.h"

typedef enum {
    InitStart = 0,
    InitNoTpak, InitNoCartridge, InitRequiresExpansionPak,
    InitPending, InitReady, InitLoading, InitLoaded, InitError
} InitState;

static natural retries = 0;

/**
 * Test code to determine the memory map of anything plugged in to the n64 controller's accessory port.
 */
void mapControllerMemory() {
    logAndPauseFrame(1, "Controller Pak Memory Map");
    byte block[0x40];
    memset(block, 0x00, 0x40);
    byte lastByte = 0xFE;
    for (uInt i = 0x0000; i < 0xFFFF; i+= 0x20) {
        read_mempak_address(0, i, block);
        if (block[0] != lastByte) {
            string caption;
            sprintf(caption, "address = %04x", (natural)i);
            printSegmentToFrame(caption, block, 1);
            lastByte = block[0];
        }
    }
}

/**
 * Waits for a Start Button pressed, then goes and loads a rom.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initLogic(RootState* state, const byte playerNumber) {
    if (state->Players[playerNumber].InitState == InitStart) {
        sByte result = getCartridgeMetadata(playerNumber, &state->Players[playerNumber].Cartridge);

        if (!result) {
            state->Players[playerNumber].InitState = InitPending;
            state->RequiresRepaint = true;
        } else {
            //mapControllerMemory();
            switch(result) {
                case TPAK_ERR_NO_TPAK:
                    state->Players[playerNumber].InitState = InitNoTpak;
                    break;
                case TPAK_ERR_NO_CARTRIDGE:
                    state->Players[playerNumber].InitState = InitNoCartridge;                
                    break;
                case TPAK_ERR_INSUFFICIENT_MEMORY:
                    state->Players[playerNumber].InitState = InitRequiresExpansionPak;
                    break;
                default:
                    state->ErrorCode = result;
                    state->Players[playerNumber].InitState = InitError;
                    break;
            }
        }
    }

    bool loadInternal = false;

    if (state->Players[playerNumber].InitState == InitPending) {
        bool releasedButtons[N64_BUTTON_COUNT] = {};
        getPressedButtons(&state->KeysReleased, playerNumber, releasedButtons);

        if (releasedButtons[A] || releasedButtons[Start]) {
            state->RequiresControllerRead = true;
            state->RequiresRepaint = true;
            state->Players[playerNumber].InitState = InitReady;
        } else if (releasedButtons[B]) {
            state->RequiresControllerRead = true;
            state->Players[playerNumber].InitState = InitStart;
            retries++;
        } else if (releasedButtons[CDown]) {
            // Load internal easter egg cartridge.
            state->Players[playerNumber].InitState = InitLoading;
            loadInternal = true;
        }
    } else if (state->Players[playerNumber].InitState == InitReady) {
        // Show the loading message.
        state->RequiresRepaint = true;
        state->Players[playerNumber].InitState = InitLoading;
    }

    if (state->Players[playerNumber].InitState == InitLoading) {
        state->RequiresRepaint = true;

        if (loadInternal) {
            strcpy(state->Players[0].Cartridge.Header.Title, "Easter egg");
            sInt result = loadInternalRom(&state->Players[0].Cartridge.Rom);
            if (result == -1) {
                logAndPauseFrame(0, "Error loading internal ROM");
            }

            state->Players[playerNumber].Cartridge.Ram.Size = 0x00;
            state->Players[playerNumber].Cartridge.Header.IsSgbSupported = true;            
        } else {
            sByte result = importCartridge(playerNumber, &state->Players[playerNumber].Cartridge);
            if (result) {
                state->ErrorCode = result;
                state->Players[playerNumber].InitState = InitError;
                return;
            }
        }
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
            sprintf(text, tmp, &state->Players[playerNumber].Cartridge.Header.Title);

            if (retries) {
                sprintf(tmp, "%s Retries: %d", text, retries);
                strcpy(text, tmp);
            }

            break;
        case InitError:
            sprintf(text, "Loading Cartridge failed with error: %d", state->ErrorCode);
            break;
        default:
            strcpy(text, "Unknown Error!!!");
            break;
    }

    drawTextParagraph(state->Frame, text, screen.Left + TEXT_WIDTH, textTop, 0.8, screen.Width - TEXT_WIDTH);
}
