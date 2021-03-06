#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"
#include "config.h"
#include "play.h"
#include "controller.h"
#include "screen.h"
#include "link.h"
#include "sound.h"
#include "superGameboy.h"
#include "gbc_state.h"
#include "cpu.h"
#include "emu.h"
#include "lcd.h"
#include "rsp.h"
#include "hwdefs.h"
#include "logger.h"
#include "fps.h"
#include "ppu.h"
#include "debug.h"

#include "rsp.h"

#include <libdragon.h>

typedef enum {GameboyPalette, SuperGameboyPalette, GameboyColorPalette } PaletteType;

/**
 * Loads a gb bios file if one is available.
 * @param state state to copy the bios to.
 * @return 0 if load successful, non-zero for errors.
 */
static sByte loadBios(GbState* state) {
    sByte result = 0;
    byte* biosFile = null;

    dfs_init(DFS_DEFAULT_LOCATION);
    sInt filePointer = dfs_open("/bios.bin");

    sInt biosSize = 0;
    if (filePointer >= 0) {
        biosSize = dfs_size(filePointer);
    } else {
        result = -1;
    }

    if (biosSize > 0) {
        biosFile = malloc(biosSize);
        dfs_read(biosFile, 1, biosSize, filePointer);
        ByteArray bios;
        bios.Data = biosFile;
        bios.Size = biosSize;
        applyBios(state, &bios);
    } else {
        result = -2;
    }

    dfs_close(filePointer);
    free(biosFile);
    biosFile = null;

    return result;
}

/**
 * Passes the gameboy cartridge data in to the emulator and fires it up.
 * @param state emulator state object.
 * @param romData ROM loaded from cartridge.
 * @param saveData Save file RAM loaded from cartridge.
 */
static void initialiseEmulator(GbState* state) {
    loadCartridge(state);
    cpu_reset_state(state);

    loadBios(state);

    emu_init(state);
    cpu_init_emu_cpu_state(state);
    lcd_init(state);
}

/**
 * Sets all emulation functions for this player back to a clean slate.
 * @param state The player to reset.
 */
void resetPlayState(PlayerState* state) {
    initialiseEmulator(&state->EmulationState);
    if (state->EmulationState.Cartridge.IsGbcSupported) {
        prepareMicrocode(FRAME_RENDERER);
    } else {
        ppuInit(state);
    }
    state->Meta.FrameCount = 0;
    resetSGBState(&state->SGBState);
}

/**
 * Converts buttons pressed on the n64 controller into equivalents on the gameboy's.
 * @param controllerNumber The controller to process.
 * @param buttonMap Array indexed by N64Button identifying which buttons go where.
 * @param n64Input struct containing which N64 buttons are currently pressed.
 * @out gbInput struct of gb buttons to fill in.
 * @private
 */
static void mapGbInputs(const char controllerNumber, const GbButton* buttonMap, const N64ControllerState* n64Input, bool* pressedButtons, GbController* gbInput) {
    getPressedButtons(n64Input, controllerNumber, pressedButtons);

    for (byte i = 0; i < N64_BUTTON_COUNT; i++) {
        if (!pressedButtons[i]) {
            continue;
        }

        switch(buttonMap[i]) {
            case GbA: gbInput->button_a = true; break;
            case GbB: gbInput->button_b = true; break;
            case GbStart: gbInput->button_start = true; break;
            case GbSelect: gbInput->button_select = true; break;
            case GbUp: gbInput->button_up = true; break;
            case GbDown: gbInput->button_down = true; break;
            case GbLeft: gbInput->button_left = true; break;
            case GbRight: gbInput->button_right = true; break;
            default: break;
        }
    }
}

void playAudio(const GbState* state) {
    if (!audio_can_write()) {
        return;
    }

    GbSoundChannel channels[4];
    GbSoundControl soundControl;
    getSoundControl(state, &soundControl);

    if (!soundControl.Bits.IsSoundEnabled) {
        return;
    }

    sShort* buffer = calloc(sizeof(sShort), soundControl.BufferLength);

    for (byte i = 0; i < 4; i++) {
        getSoundChannel(state, i + 1, &channels[i]);
        prepareSoundBuffer(&soundControl, &channels[i], buffer);
    }

    audio_write(buffer);
    free(buffer);
    buffer = null;
}

/**
 * Handles gameboy emulation.
 * @param playerNumber player in play mode.
 */
void playLogic(const byte playerNumber) {
    PlayerState* playerState = &rootState.Players[playerNumber];
    GbState* s = &playerState->EmulationState;

    #if MAX_PLAYERS >= 2
    if (rootState.PlayerCount == 2 && isRequestingTransfer()) {
        GbState* states[2] = {
            &rootState.Players[0].EmulationState,
            &rootState.Players[1].EmulationState
        };
        exchangeLinkData(states);
    }
    #endif

    emu_step(playerState);

    #ifdef IS_SGB_ENABLED
        if (s->Cartridge.Header.is_sgb_supported) {
            processSGBData(playerState);
            performSGBFunctions(playerState);
        }
    #endif

    if (s->lcd_entered_vblank) {

        #ifdef SHOW_FRAME_COUNT
            fps_frame();
            playerState->Meta.FrameCount++;
        #endif

        #ifdef FRAMES_TO_SKIP
            if (playerState->Meta.FrameCount % (FRAMES_TO_SKIP + 1)) {
                playerState->WasFrameSkipped = true;
                return;
            } else {
                playerState->WasFrameSkipped = false;
            }
        #endif

        #ifdef IS_SGB_ENABLED
            if (s->Cartridge.Header.is_sgb_supported) {
                applySGBPalettes(
                    &playerState->SGBState, 
                    s->NextBuffer
                );
            }
        #endif

        GbController* input = calloc(1, sizeof(GbController));

        bool pressedButtons[N64_BUTTON_COUNT] = {};

        mapGbInputs(
            playerNumber,
            playerState->ButtonMap,
            &rootState.KeysPressed,
            pressedButtons,
            input
        );

        bool releasedButtons[N64_BUTTON_COUNT] = {};
        getPressedButtons(&rootState.KeysReleased, playerNumber, releasedButtons);

        if (releasedButtons[playerState->SystemMenuButton]) {
            playerState->ActiveMode = Menu;
            rootState.RequiresRepaint = true;
            return;
        }

        emu_process_inputs(s, input);

        free(input);
        input = 0;

        // Write save file back to the cartridge if it has changed.
        if (s->isSRAMDirty) {
            sByte result = exportCartridgeRam(playerNumber, &s->Cartridge);
            if (result) {
                logAndPauseFrame(0, "saving to cartridge failed");
            }

            s->isSRAMDirty = false;
        }

        // Audio off until I can test it properly.
        #ifdef IS_AUDIO_ENABLED
            if (playerState->AudioEnabled) {
                playAudio(s);
            }
        #endif

        rootState.RequiresRepaint = true;
        rootState.RequiresControllerRead = true;
    }
    UPDATE_PROFILE(PROFILE_DEVICE);
}

/**
 * @deprecated 
 */
typedef struct {
    uintptr_t InAddress;
    uintptr_t OutAddress;
    Rectangle Screen;
    bool IsColour;
    byte IsColourPadding[3];
    bool IsBusy;
    byte IsBusyPadding[3];
    uint32_t WordPadding[2];
} Old_RspInterface;

/**
 * Kicks off the RSP to render the next frame.
 * @param inBuffer gameboy screen buffer pixels are picked up by the RSP from here.
 * @param outBuffer after RSP generates a texture, it will DMA it back into DRAM at this address.
 * @param screen size and position of the textures drawn by the RSP.
 * @param isColour if true, inBuffer words represent 2 bit DMG pixels.  Otherwise they are 16bit GBC pixels
 * @deprecated
 */
void renderFrame(uintptr_t inBuffer, uintptr_t outBuffer, Rectangle* screen, bool isColour) {
    Old_RspInterface* rspInterface = allocRspInterface(sizeof(Old_RspInterface));
    data_cache_hit_invalidate(rspInterface, sizeof(Old_RspInterface));
    // Let the RSP finish it's current frame & skip this one.
    //..if (rspInterface->IsBusy) {
    //  return;
    //}

    //if (rootState.Frame) {
      //  rdp_detach_display();
        //display_show(rootState.Frame);
    //}
    //while(!(rootState.Frame = display_lock()));
    rdp_attach_display(rootState.Frame);

    haltRsp();
    rspInterface->InAddress = inBuffer;
    rspInterface->OutAddress = outBuffer;
    rspInterface->Screen = *screen;
    rspInterface->IsColour = isColour;
    rspInterface->IsBusy = true;

    data_cache_hit_writeback(rspInterface, sizeof(Old_RspInterface));

    run_ucode();
}

/**
 * Draws gameboy screen.
 * @param playerNumber player in play mode.
 */
void playDraw(const byte playerNumber) {
    if (rootState.Players[playerNumber].EmulationState.Cartridge.IsGbcSupported) {
        Rectangle screen = {};
        getScreenPosition(playerNumber, &screen);

        PaletteType palette = GameboyColorPalette;

        screen = (Rectangle) { screen.Left, screen.Top, 320, 12 };

        renderFrame(
            (uintptr_t)rootState.Players[playerNumber].EmulationState.NextBuffer,
            (uintptr_t)rootState.Players[playerNumber].EmulationState.TextureBuffer,
            &screen,
            palette == GameboyColorPalette
        );
    }

    #ifdef SHOW_FRAME_COUNT
        string text = "";

        sprintf(text, "FPS: %d %lld", fps_get(), rootState.Players[playerNumber].Meta.FrameCount);
        graphics_set_color(GLOBAL_TEXT_COLOUR, 0x0);
        graphics_draw_box(rootState.Frame, 0, 450, 680, 10, GLOBAL_BACKGROUND_COLOUR);
        graphics_draw_text(rootState.Frame, 5, 450, text);
    #endif
}