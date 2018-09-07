#include "init.h"
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

/**
 * Waits for a Start Button pressed, then goes and loads a rom.
 * @param state program state.
 * @param playerNumber player in init mode.
 */
void initLogic(RootState* state, const unsigned char playerNumber) {
    if (state->ControllerState.c[playerNumber].start) {
        state->RequiresRepaint = true;

        loadRom(playerNumber, &state->Players[playerNumber].Cartridge.RomData);
        loadSave(playerNumber, &state->Players[playerNumber].Cartridge.SaveData);

        // TODO
        strcpy(state->Players[playerNumber].Cartridge.Title, "TODO");
        state->Players[playerNumber].Cartridge.IsSuperGbCart = false;
        state->Players[playerNumber].Cartridge.IsGbcCart = false;
        state->PixelSize = (float)SINGLE_PLAYER_SCREEN_HEIGHT / (float)GB_LCD_HEIGHT;

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
void initDraw(const RootState* state, const unsigned char playerNumber) {
    if (playerNumber != 0) {
        return;
    }

    graphics_draw_box(state->Frame, 0, 0, 640, 480, GLOBAL_BACKGROUND_COLOUR);
    graphics_draw_box(
        state->Frame,
        SINGLE_PLAYER_SCREEN_LEFT,
        SINGLE_PLAYER_SCREEN_TOP,
        SINGLE_PLAYER_SCREEN_WIDTH,
        SINGLE_PLAYER_SCREEN_HEIGHT,
        BLANK_SCREEN_COLOUR
    );

    const char TEXT_HEIGHT = 30;
    const char TEXT_WIDTH = 10;

    natural textTop = SINGLE_PLAYER_SCREEN_TOP - TEXT_HEIGHT + (SINGLE_PLAYER_SCREEN_WIDTH / 2);
    natural textLeft = SINGLE_PLAYER_SCREEN_LEFT + TEXT_WIDTH;

    string text = "";
    if (!isTPakInserted(playerNumber)) {
        getText(TextNoTpak, text);
    } else if (!isCartridgeInserted(playerNumber)) {
        getText(TextNoCartridge, text);
    } else {
        getText(TextLoadCartridge, text);
    }

    graphics_draw_text(state->Frame, textLeft, textTop, text);
}
