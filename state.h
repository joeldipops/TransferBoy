#ifndef STATE_INCLUDED
#define STATE_INCLUDED

#include "core.h"
#include "controller.h"

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
    char PlayerCount;
    struct controller_data ControllerState;
    float PixelSize;
    display_context_t Frame;
    PlayerState Players[8]; // Really not aiming for anything other than 2, but you never know.
} RootState;

#endif
