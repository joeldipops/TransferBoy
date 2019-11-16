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
    state->BuffersInitialised = 0;
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

static uint32_t outBuffer[0x1000];

/**
 * Take the array of pixels produced by the emulator and throw it up on to the screen.
 * @param frame Id of frame to render to.
 * @param pixelBuffer Array of pixels.
 * @param isColour True for GBC and super-game-boy enhanced, otherwise false.
 * @param avgPixelSize upscaled TV size of gameboy pixels
 * @param left left bound of the gb screen to render.
 * @param top top bound of the gb screen to render.
 * @param sgbState SuperGameBoy related variables.
 * @param bgColourIndex which colour fills the most pixels and should therefore be 
 *        drawn once as a background
 * @private
 */
static inline void renderPixels(
    const PlayerState* state,
    const PaletteType paletteType,
    const u16 left,
    const u16 top,
    const u16 width,
    const u16 height
) {
    // Data to be DMAd must be aligned to 16 bytes.
    AlignedPointer pointer = malloc_aligned(sizeof(RspIn), 16);
    RspIn* input = (RspIn*) pointer.p;

    input->InAddress = (uint32_t)state->EmulationState.NextBuffer;
    input->OutAddress = (uint32_t)outBuffer;
    input->IsColour = (paletteType == GameboyColorPalette);

    // TODO Calculate block height
    input->Screen = (Rectangle){ left, top, width, 12 };

    rdp_enable_texture_copy();

    data_cache_hit_writeback(input, sizeof(RspIn));

    load_data(input, sizeof(RspIn));
    run_ucode();

    free_aligned(pointer);
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
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playLogic(RootState* state, const byte playerNumber) {
    PlayerState* playerState = &state->Players[playerNumber];
    GbState* s = &state->Players[playerNumber].EmulationState;

    if (state->PlayerCount == 2 && isRequestingTransfer(state)) {
        GbState* states[2] = {
            &state->Players[0].EmulationState,
            &state->Players[1].EmulationState
        };
        exchangeLinkData(states);
    }

    emu_step(playerState);

    if (IS_SGB_ENABLED && s->Cartridge.Header.IsSgbSupported) {
        processSGBData(playerState);
        performSGBFunctions(playerState);
    }

    if (s->lcd_entered_vblank) {
        playerState->Meta.FrameCount++;

        if (FRAMES_TO_SKIP && (playerState->Meta.FrameCount % (FRAMES_TO_SKIP + 1))) {
            playerState->WasFrameSkipped = true;
            return;
        } else {
            playerState->WasFrameSkipped = false;
        }

        playerState->Meta.LastClock = playerState->Meta.NextClock;
        playerState->Meta.NextClock = get_ticks_ms();

        if (IS_SGB_ENABLED && s->Cartridge.Header.IsSgbSupported) {
            applySGBPalettes(
                &playerState->SGBState, 
                s->NextBuffer
            );
        }

        GbController* input = calloc(1, sizeof(GbController));

        bool pressedButtons[N64_BUTTON_COUNT] = {};

        mapGbInputs(
            playerNumber,
            playerState->ButtonMap,
            &state->KeysPressed,
            pressedButtons,
            input
        );

        bool releasedButtons[N64_BUTTON_COUNT] = {};
        getPressedButtons(&state->KeysReleased, playerNumber, releasedButtons);

        if (releasedButtons[playerState->SystemMenuButton]) {
            playerState->ActiveMode = Menu;
            state->RequiresRepaint = true;
            return;
        } else if (releasedButtons[CUp]) {
            FRAMES_TO_SKIP = !FRAMES_TO_SKIP;
        }

        emu_process_inputs(s, input);

        free(input);
        input = 0;

        // Write save file back to the catridge if it has changed.
        if (s->isSRAMDirty) {
            sByte result = exportCartridgeRam(playerNumber, &s->Cartridge);
            if (result) {
                logAndPauseFrame(0, "saving to cartridge failed");
            }

            s->isSRAMDirty = false;
        }

        // Audio off until I can test it properly.
        if (IS_AUDIO_ENABLED && playerState->AudioEnabled) {
            playAudio(s);
        }

        state->RequiresRepaint = true;
        state->RequiresControllerRead = true;
    }
}

/**
 * Draws gameboy screen.
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playDraw(const RootState* state, const byte playerNumber) {
    // Main background.
    Rectangle screen = {};
    getScreenPosition(state, playerNumber, &screen);

    PaletteType palette = GameboyPalette;
    if (state->Players[playerNumber].EmulationState.Cartridge.IsGbcSupported) {
        palette = GameboyColorPalette;
    } else if (IS_SGB_ENABLED && state->Players[playerNumber].EmulationState.Cartridge.Header.IsSgbSupported) {
        palette = SuperGameboyPalette;
    }

    bool isInitialised = state->Players[playerNumber].BuffersInitialised >= 2;

    renderPixels(
        &state->Players[playerNumber],
        palette,
        screen.Left,
        screen.Top,
        screen.Width,
        screen.Height
    );
}

/**
 * Does any necessary cleanup after drawing.
 * @param state program state.
 * @param playerNumber player in play mode.
 */
void playAfter(RootState* state, const byte playerNumber) {
}


