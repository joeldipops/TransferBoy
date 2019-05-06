#ifndef STATE_INCLUDED
#define STATE_INCLUDED

#include "core.h"
#include "sgbDefs.h"
#include "controller.h"
#include "tpakio.h"

#define MAX_PLAYERS 2

typedef struct {
    Mode ActiveMode;
    Border SelectedBorder;
    GbButton ButtonMap[N64_BUTTON_COUNT];
    N64Button SystemMenuButton;
    N64Button GbStartButton;
    N64Button GbSelectButton;
    GameBoyCartridge Cartridge;
    bool AudioEnabled;
    GbState EmulationState;
    SuperGameboyState SGBState;
    sByte InitState;
    sByte OptionsCursorRow;
    sByte MenuCursorRow;
    sByte MenuCursorColumn;
    bool WasFrameSkipped;
    byte BuffersInitialised;
} PlayerState;

typedef struct {
    sByte ErrorCode;
    byte pad0:2;
    bool RequiresRepaint:1;
    bool RequiresControllerRead:1;
    byte PlayerCount:4;
    N64ControllerState KeysPressed;
    N64ControllerState KeysReleased;
    float PixelSize;
    display_context_t Frame;
    PlayerState Players[MAX_PLAYERS]; // Really not aiming for anything other than 2, but you never know.
} RootState;

/**
 * Sets a Gameboy button to a particular N64 button, unsetting it from the previous mapping.
 * @param playerState state containing controller mapping to update.
 * @param gbButton gameboy button to set.
 * @param n64Button n64 button to set gb button to.
 */
void setButtonToMap(PlayerState* playerState, const GbButton gbButton, const N64Button n64Button);

/**
 * Initialise the state struct for a new player.
 * @param playerState struct to initialise.
 */
void generatePlayerState(PlayerState* playerState);

#endif
