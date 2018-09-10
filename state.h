#ifndef STATE_INCLUDED
#define STATE_INCLUDED

#include "core.h"
#include "controller.h"

#define MAX_PLAYERS 2

typedef struct {
    ByteArray RomData;
    ByteArray SaveData;
    string Title;
    bool IsGbcCart;
    bool IsSuperGbCart;
} CartridgeData;

typedef struct {
    Mode ActiveMode;
    Border SelectedBorder;
    GbButton ButtonMap[16];
    N64Button SystemMenuButton;
    CartridgeData Cartridge;
    bool AudioEnabled;
    struct gb_state EmulationState;
    char LastErrorCode;
    char MenuCursorRow;
    char MenuCursorColumn;
} PlayerState;

typedef struct {
    bool RequiresRepaint;
    byte PlayerCount;
    struct controller_data ControllerState;
    float PixelSize;
    display_context_t Frame;
    PlayerState Players[MAX_PLAYERS]; // Really not aiming for anything other than 2, but you never know.
} RootState;

/**
 * Initialise the state struct for a new player.
 * @param playerState struct to initialise.
 */
void generatePlayerState(PlayerState* playerState);

#endif
