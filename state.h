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

typedef enum {
    SGBSetPalette01 = 0x00, // PAL01
    SGBSetPalette23 = 0x01, // PAL23
    SGBSetPalette03 = 0x02, // PAL03
    SGBSetPalette12 = 0x03, // PAL12
    SGBApplyPaletteBlock = 0x04, // ATTR_BLK
    SGBApplyPaletteLine = 0x05, //ATTR_LIN
    SGBDividePalettes = 0x06, // ATTR_DIV
    SGBApplyPaletteCharacter = 0x07, //ATTRIBUTE_CHR,
    SGBGenerateSound = 0x08, // SOUND
    SGBTransferSound = 0x09, // SOU_TRN
    SGBSetSystemPalette = 0x0A, // PAL_SET
    SGBTransferPalette = 0x0B, //PAL_TRAN
    SGBEnableAttraction = 0x0C, //ATRN_EN
    SGBEnableIcon = 0x0E, // ICON_ENT
    SGBTransferDataToSGB = 0x0F, // DATA_SND
    SGBTransferSGBToSNES = 0x10, //DATA_TRN
    SGBRequestMultiplayer = 0x11, //MLT_REQ
    SGBSNESJump = 0x12, //JUMP
    SGBTransferCharacter = 0x13, //CHR_TRN
    SGBTransferFrame = 0x14, // PCT_TRN
    SGBTransferAttributes = 0x15, // ATTR_TRN
    SGBSetAttribute = 0x16, // ATTR_SET
    SGBMaskWindow = 0x17, // MASK_EN
    SGBSetPalettePriority = 0x19, // PAL_PRI
    SGBNoop = 0x1A // TransferBoy noop
} SuperGameboyCommand;

typedef struct {
    bool HasData:1; // Used
    bool HasPendingBit:1;
    bool PendingBit:1;
    byte NumberOfPackets:3;
    byte pad1:2;
    byte BitBuffer;
    byte BitPointer;
    byte PacketPointer;
    byte BytePointer;
    bool IsTransferring;
    SuperGameboyCommand CurrentCommand;
    byte* Buffer;
} SuperGameboyState;

typedef struct {
    Mode ActiveMode;
    Border SelectedBorder;
    GbButton ButtonMap[N64_BUTTON_COUNT];
    N64Button SystemMenuButton;
    N64Button GbStartButton;
    N64Button GbSelectButton;
    CartridgeData Cartridge;
    bool AudioEnabled;
    GbState EmulationState;
    SuperGameboyState SGBState;
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
