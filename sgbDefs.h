#ifndef SGBDEFS_INCLUDED
#define SGBDEFS_INCLUDED

typedef enum {
    SGBSetPalette01 = 0x00, // PAL01
    SGBSetPalette23 = 0x01, // PAL23
    SGBSetPalette03 = 0x02, // PAL03
    SGBSetPalette12 = 0x03, // PAL12
    SGBApplyPaletteToBlocks = 0x04, // ATTR_BLK
    SGBApplyPaletteToLines = 0x05, //ATTR_LIN
    SGBDividePalettes = 0x06, // ATTR_DIV
    SGBApplyPaletteToTiles = 0x07, //ATTR_CHR,
    SGBGenerateSound = 0x08, // SOUND
    SGBTransferSound = 0x09, // SOU_TRN
    SGBSetSystemPalette = 0x0A, // PAL_SET
    SGBTransferPalette = 0x0B, //PAL_TRAN
    SGBEnableAttraction = 0x0C, //ATRN_EN
    SGBEnableIcon = 0x0E, // ICON_ENT
    SGBTransferPacketToSgb = 0x0F, // DATA_SND
    SGBTransferScreenToSGB = 0x10, //DATA_TRN
    SGBRequestMultiplayer = 0x11, //MLT_REQ
    SGBSNESJump = 0x12, //JUMP
    SGBTransferCharacter = 0x13, //CHR_TRN
    SGBTransferOverlay = 0x14, // PCT_TRN
    SGBTransferAttributes = 0x15, // ATTR_TRN
    SGBSetAttribute = 0x16, // ATTR_SET
    SGBMaskWindow = 0x17, // MASK_EN
    SGBSetPalettePriority = 0x19, // PAL_PRI
    SGBNoop = 0x1A // TransferBoy noop
} SuperGameboyCommand;

typedef natural Palette[4];
typedef natural SgbPalette[16];

typedef struct {
    natural Address;
    byte Bank;
    byte ByteCount;
    byte Data[11];
} SnesRamBlock;

typedef byte GbSprite[32];

typedef struct {
    byte SpriteId;
    bool IsYFlipped:1;
    bool IsXFlipped:1;
    byte pad0:1;
    byte PaletteId:3;
    byte pad1:2;
} SgbTile;

typedef struct {
    bool HasBuffer:1;
    bool HasPendingBit:1;
    bool PendingBit:1;
    bool AwaitingStopBit:1; // After 16 bytes per packet are transferred, one more 0 bit is sent to singal the end of the packet.
    bool JoypadRequestResolved:1;
    byte NumberOfPackets:3;

    byte PlayersMode:2; //0-3
    byte CurrentController:2;
    bool IsWindowFrozen:1;
    bool HasPriority:1;
    bool HasRamData:1;
    byte pad0:2;

    byte BitBuffer;
    byte BitPointer;
    byte PacketPointer;
    byte BytePointer;
    bool IsTransferring;
    SuperGameboyCommand CurrentCommand;
    Palette Palettes[4];
    byte SnesRamBlockCount;
    GbSprite SpriteData[256];
    SgbTile OverlayData[1024];
    SgbPalette OverlayPalettes[3];
    byte TilePalettes[360]; // 20 x 18 tiles.
    SnesRamBlock* RamBlocks;
    byte* Buffer;
} SuperGameboyState;

#endif
