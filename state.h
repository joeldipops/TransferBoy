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
    GbButton ButtonMap[N64_BUTTON_COUNT];
    N64Button SystemMenuButton;
    CartridgeData Cartridge;
    bool AudioEnabled;
    GbState EmulationState;
    sByte LastErrorCode;
    sByte OptionsCursorRow;
    sByte MenuCursorRow;
    sByte MenuCursorColumn;
} PlayerState;

typedef struct {
    bool RequiresRepaint;
    bool RequiresControllerRead;
    byte PlayerCount;
    N64ControllerState KeysPressed;
    N64ControllerState KeysReleased;
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
