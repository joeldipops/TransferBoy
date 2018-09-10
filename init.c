#include "init.h"
#include "tpakio.h"
#include "screen.h"
#include <libdragon.h>
#include "include/gbc_bundle.h"

/**
 * Passes the gameboy cartridge data in to the emulator and fires it up.
 * @param state emulator state object.
 * @param romData ROM loaded from cartridge.
 * @param saveData Save file RAM loaded from cartridge.
 * @private
 */
void initialiseEmulator(struct gb_state* state, const ByteArray* romData, const ByteArray* saveData) {
    memset(state, 0, sizeof(struct gb_state));

    state_new_from_rom(state, romData->Data, romData->Size);
    cpu_reset_state(state);

    // Add bios here?
    /*
    dfs_init(DFS_DEFAULT_LOCATION);
    int filePointer = dfs_open("/bios.bin");
    int biosSize = dfs_size(filePointer);
    byte* biosFile = malloc(biosSize);
    dfs_read(biosFile, 1, biosSize, filePointer);
    dfs_close(filePointer);
    state_add_bios(state, biosFile, biosSize);
    free(biosFile);
    */

    state_load_extram(state, saveData->Data, saveData->Size);
    init_emu_state(state);
    cpu_init_emu_cpu_state(state);
    lcd_init(state); // Here we're initing the code that renders the pixel buffer.
}

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

        loadRom(playerNumber, &state->Players[playerNumber].Cartridge.RomData);
        loadSave(playerNumber, &state->Players[playerNumber].Cartridge.SaveData);

        freeTPakIo();

        // TODO
        strcpy(state->Players[playerNumber].Cartridge.Title, "TODO");
        state->Players[playerNumber].Cartridge.IsSuperGbCart = false;
        state->Players[playerNumber].Cartridge.IsGbcCart = false;

        ScreenPosition screen = {};
        getScreenPosition(state, playerNumber, &screen);
        state->PixelSize = (float)screen.Height / (float)GB_LCD_HEIGHT;

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

    graphics_draw_box(state->Frame, 0, 0, RESOLUTION_X, RESOLUTION_Y, GLOBAL_BACKGROUND_COLOUR);
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

    graphics_draw_text(state->Frame, textLeft, textTop, text);
}
