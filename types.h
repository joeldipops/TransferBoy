#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "tpakio.h"
#include "core.h"
#include "hwdefs.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define FLAG_C 0x10
#define FLAG_H 0x20
#define FLAG_N 0x40
#define FLAG_Z 0x80

struct emu_cpu_state {
    // Lookup tables for the reg-index encoded in instructions to ptr to reg.
    u8 *reg8_lut[9];
    u16 *reg16_lut[4];
    u16 *reg16s_lut[4];
};

struct player_input {
    bool button_left;
    bool button_right;
    bool button_up;
    bool button_down;
    bool button_a;
    bool button_b;
    bool button_start;
    bool button_select;

    bool special_quit;
    bool special_savestate;
    bool special_dbgbreak;
};


#define WAVEDATA_LENGTH 16

typedef enum  __attribute__((packed)) { 
    VRAM_8800 = 0, VRAM_8000 = 1
} TileDataAddress;

typedef enum  __attribute__((packed)) {
    VRAM_9800 = 0, VRAM_9C00 = 1
} TileMapAddress;

typedef enum  __attribute__((packed)) {
    SPRITE_8x8 = 0, SPRITE_8x16 = 1
} SpriteSize;

typedef enum __attribute__((packed)) {
    HBLANK = 0,
    VBLANK = 1,
    OAM_IN_USE = 2,
    VRAM_IN_USE = 3
} LcdMode;

typedef enum __attribute__((packed)) {
    DARKEST = 3,
    DARKER = 2,
    LIGHTER = 1,
    LIGHTEST = 0
} PixelShade;

typedef enum  __attribute__((packed)){
    SoundConsecutive = 0,
    SoundWait = 1
} SoundTimingMode;

typedef enum  __attribute__((packed)) {
    SoundMute = 0,
    SoundNoShift = 1,
    SoundHalfShift = 2,
    SoundQuarterShift = 3
} WavePatternShift;

typedef enum {
    InfraredReadDisabled = 0,
    // Values 1 & 2 are unused
    InfraredReadEnabled = 3
} GbcInfraredMode;

/* State of the cpu part of the emulation, not of the hardware. */
struct emu_cpu_state;

typedef enum {
   ROM_SELECT = 0,
   SRAM_SELECT = 1
} BankSelectType;

#define BIOS_SIZE 0x100

struct gb_state;

typedef void (*mmuWriteOperation)(struct gb_state*, u16, u8);
typedef u32 (*mmuReadOperation)(struct gb_state*, u16);

typedef struct gb_state {
    mmuWriteOperation mmuWrites[0x100];
    mmuReadOperation mmuReads[0x100];

    /*
     * CPU state (registers, interrupts, etc)
     */
    // Registers: allow access to 8-bit and 16-bit regs, and via array.
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    union {
        u8 regs[8];
        struct {
            u16 BC, DE, HL, AF;
        } reg16;
        struct {
            u8 B, C, D, E, H, L, A, F;
        } reg8;
        struct __attribute__((packed)) {
            char padding[7];
            u8 ZF:1;
            u8 NF:1;
            u8 HF:1;
            u8 CF:1;
            u8 pad1:1;
            u8 pad2:1;
            u8 pad3:1;
            u8 pad4:1;
        } flags;
    };
    #else
    // Original Little Endian Setup
    union {
        u8 regs[8];
        struct {
            u16 BC, DE, HL, AF;
        } reg16;
        struct { // little-endian of x86 is not nice here.
            u8 C, B, E, D, L, H, F, A;
        } reg8;
        struct __attribute__((packed)) {
            char padding[6];
            u8 pad1:1;
            u8 pad2:1;
            u8 pad3:1;
            u8 pad4:1;
            u8 CF:1;
            u8 HF:1;
            u8 NF:1;
            u8 ZF:1;
        } flags;
    };
    #endif

    u16 sp;
    u16 pc;

    // Tracks All ROM & SRAM banks
    GameBoyCartridge Cartridge;

    // Optionally can be mapped from $0000 of ROM space.
    byte* BiosFile;
    // A copy of ROM0 with the bios over the top of it.
    byte* BIOS;

    // $8000 - $9fff: Video RAM, 8K non-CGB, 16K CGB (banked)
    byte* VRAMBanks; 
    // $c000 - $dfff: Internal RAM (WRAM), 8K non-CGB, 32K CGB (banked)
    byte* WRAMBanks; 

    byte* ROM0;
    byte* ROMX;
    byte* VRAM;
    byte* SRAM;
    byte* WRAM0;
    byte* WRAMX;

    // $fe00 - fe9f: Sprite/Object attributes
    byte OAM[0xA0];

    // fea0 - feff: unused

    // $ff00 - ff7f: IO registers and HRAM
    union __attribute__((packed)){
        byte HRAM[HRAM_SIZE];
        struct __attribute__((packed)) {
            // FF00 - P1 - JoypadIo
            union __attribute__((packed)) {
                byte JoypadIo;
                struct __attribute__((packed)) {
                    byte unused0:2;
                    byte SelectFace:1;
                    byte SelectDPad:1;
                    byte unused01:4;
                };      
                struct __attribute__((packed)) {
                    byte unused02:4;
                    byte Down:1;
                    byte Up:1;
                    byte Left:1;
                    byte Right:1;
                };
                struct __attribute__((packed)) {
                    byte unused03:4;
                    byte Start:1;
                    byte Select:1;
                    byte B:1;
                    byte A:1;
                };
            };
            // FF01 - SB - Link Data
            byte LinkData;
            // FF02 - SC - Link Control
            union {
                byte LinkControl;
                struct __attribute__((packed)) {
                    bool IsLinkTransferAvailable:1;
                    const byte unused1:6;
                    bool IsLinkClockExternal:1;
                };
            };
            const byte unused2;
            // FF04 - DIV - Timer Clock
            byte TimerClock;
            // FF05 - TIMA - Timer Counter
            byte TimerCounter;
            // FF06 - TMA - Timer Reset Value
            byte TimerResetValue;
            // FF07 - TAC - Timer Control
            union {
                byte TimerControl;
                struct __attribute__((packed)) {
                    const byte unused3:5;
                    bool IsTimerStarted:1;
                    byte TimerFrequency:2;
                };
            };
            const byte unused4[7];
            // FF0F - IF - Interrupt Flags
            union {
                byte InterruptFlags;
                struct __attribute__((packed)) {
                    const byte unused5:3;
                    byte JoypadInterrupt:1;
                    byte LinkInterrupt:1;
                    byte TimerInterrupt:1;
                    byte LcdInterrupt:1;
                    byte VBlankInterrupt:1;
                };
            };
            // FF10 - NR10 - AudioChannel1Sweep
            union {
                byte AudioChannel1Sweep;
                struct __attribute__((packed)) {
                    const byte unused6:1;
                    byte AudioChannel1SweepTime:3;
                    bool AudioChannel1IsFrequencyDecrease:1;
                    byte AudioChannel1SweepShift:3;
                };
            };
            // FF11 - NR11 - AudioChannel1PatternAndLength
            union {
                byte AudioChannel1PatternAndLength;
                struct __attribute__((packed)) {
                    byte AudioChannel1WavePatternDuty:2;
                    byte AudioChannel1SoundLength:6;
                };
            };
            // FF12 - NR12 - AudioChannel1Envelope
            union {
                byte AudioChannel1Envelope;
                struct __attribute__((packed)) {
                    byte AudioChannel1InitialVolume:4;
                    bool AudioChannel1IsVolumeIncreasing:1;
                    byte AudioChannel1EnvelopeSteps:3;
                };
            };
            // FF13 - NR13 - AudioChannel1Frequency
            byte AudioChannel1FrequencyLow;
            // FF14 - NR14 - AudioChannel1Flags
            union {
                byte AudioChannel1Flags;
                struct __attribute__((packed)) {
                    bool AudioChannel1IsSoundReset:1;
                    SoundTimingMode AudioChannel1TimingMode:1;
                    const byte unused7:3;
                    byte AudioChannel1FrequencyHigh:3;
                };
            };
            const byte unused8; //reserved for channel 2 sweep
            // FF16 - NR21 - AudioChannel2PatternAndLength
            union {
                byte AudioChannel2PatternAndLength;
                struct __attribute__((packed)) {
                    byte AudioChannel2WavePatternDuty:2;
                    byte AudioChannel2SoundLength:6;
                };
            };
            // FF17 - NR22 - AudioChannel2Envelope
            union {
                byte AudioChannel2Envelope;
                struct __attribute__((packed)) {
                    byte AudioChannel2InitialVolume:4;
                    bool AudioChannel2IsVolumeIncreasing:1;
                    byte AudioChannel2EnvelopeSteps:3;
                };
            };
            // FF18 - NR23 - AudioChannel1Frequency
            byte AudioChannel2FrequencyLow;
            // FF19 - NR24 - AudioChannel2Flags
            union {
                byte AudioChannel2Flags;
                struct __attribute__((packed)) {
                    bool AudioChannel2IsSoundReset:1;
                    SoundTimingMode AudioChannel2TimingMode:1;
                    const byte unused9:3;
                    byte AudioChannel2FrequencyHigh:3;
                };
            };
            // FF1A - NR30 - AudioChannel3Control
            union {
                byte AudioChannel3Control;
                struct __attribute__((packed)) {
                    bool AudioChannel3IsOn:1;
                    const byte unusedA:7;
                };
            };
            // FF1B - NR31 - AudioChannel3Length
            byte AudioChannel3Length;
            // FF1C - NR32 - AudioChannel3Level
            union {
                byte AudioChannel3Level;
                struct __attribute__((packed)) {
                    const byte unusedB:1;
                    WavePatternShift AudioChannel3WavePatternShift:2;
                    const byte unusedC:5;
                };
            };
            // FF1D - NR33 - AudioChannel3Frequency
            byte AudioChannel3FrequencyLow;
            // FF1E - NR34 - AudioChannel3Flags
            union {
                byte AudioChannel3Flags;
                struct __attribute__((packed)) {
                    bool AudioChannel3IsSoundReset:1;
                    SoundTimingMode AudioChannel3TimingMode:1;
                    const byte unusedD:3;
                    byte AudioChannel3FrequencyHigh:3;
                };
            };  
            const byte unusedE; // reserved for channel 4 sweep
            // FF20 - NR41 - AudioChannel4Length
            byte AudioChannel4Length;
            // FF21 - NR42 - AudioChannel4Envelope
            union {
                byte AudioChannel4Envelope;
                struct __attribute__((packed)) {
                    byte AudioChannel4InitialVolume:4;
                    bool AudioChannel4IsVolumeIncreasing:1;
                    byte AudioChannel4EnvelopeSteps:3;
                };
            };
            // FF22 - NR43 - AudioChannel4RNGParameters
            union {
                byte AudioChannel4RNGParameters;
                struct __attribute__((packed)) {
                    byte NoiseRatioShift:4;
                    byte NoiseStepsCode: 1;
                    byte NoiseRatioFactor:3;
                };
            };
            // FF23 - NR44 AudioChannel4Flags
            union {
                byte AudioChannel4Flags;
                struct __attribute__((packed)) {
                    bool AudioChannel4IsSoundReset:1;
                    SoundTimingMode AudioChannel4TimingMode:1;
                    const const byte unusedF:6;
                };
            };
            // FF24 - NR50 - Audio Speaker Control
            union {
                byte AudioSpeakerControl;
                struct __attribute__((packed)) {
                    bool IsLeftTerminalEnabled:1; // S02
                    byte LeftTerminalVolume:3;
                    bool IsRightTerminalEnabled:1; //S01
                    byte RightTerminalVolume:3;
                };
            };
            // FF25 - NR51 - Audio Speaker Channels
            union {
                byte AudioSpeakerChannels;
                struct __attribute__((packed)) {
                    bool IsChannel4OnLeftTerminal:1;
                    bool IsChannel3OnLeftTerminal:1;
                    bool IsChannel2OnLeftTerminal:1;
                    bool IsChannel1OnLeftTerminal:1;
                    bool IsChannel4OnRightTerminal:1;
                    bool IsChannel3OnRightTerminal:1;
                    bool IsChannel2OnRightTerminal:1;
                    bool IsChannel1OnRightTerminal:1;
                };
            };
            // FF26 - NR52 - Audio Channel Switch
            union {
                byte AudioChannelSwitch;
                struct __attribute__((packed)) {
                    bool IsSoundEnabled:1;
                    byte pad:3;
                    bool IsChannel4Enabled:1;
                    bool IsChannel3Enabled:1;
                    bool IsChannel2Enabled:1;
                    bool IsChannel1Enabled:1;
                };
            };
            const byte unused10[9];
            // FF30
            byte SoundWaveData[WAVEDATA_LENGTH];
            // FF40 - LCDC - LCD Control
            union {
                byte LcdControl;
                struct __attribute__((packed)) {
                    bool IsLcdOn:1;
                    TileMapAddress WindowMap:1;
                    bool IsWindowOn:1;
                    TileDataAddress TileData:1;
                    TileMapAddress BackgroundMap:1;
                    SpriteSize SpriteSize:1;
                    bool AreSpritesOn:1;
                    bool IsBackgroundOn:1;
                };
            };
            // FF41 - STAT - LCD Status
            union {
                byte LcdStatus;
                struct __attribute__((packed)) {
                    const byte unused11:1;
                    bool InterruptOnLineLYC:1;
                    bool InterruptOnOamMode:1;
                    bool InterruptOnVBlank:1;
                    bool InterruptOnHBlank:1;
                    bool IsCurrentLineLYC:1;
                    LcdMode LcdMode:2;
                };
            };
            // FF42 - SCY - Background Scroll Y
            byte BackgroundScrollY;
            // FF43 - SCX - Background Scroll X
            byte BackgroundScrollX;
            // FF44 - LY - Currently Drawing Line
            byte CurrentLine;
            // FF45 - LCY - Next Interrupt Line
            byte NextInterruptLine;
            // FF46 - DMA - Dma Source
            byte DmaSourceAddress;
            // FF47 - BGP - Background Palette
            union {
                byte BackgroundPalette;
                struct __attribute__((packed)) {
                    PixelShade BgData3:2;
                    PixelShade BgData2:2;
                    PixelShade BgData1:2;
                    PixelShade BgData0:2;
                };
            };
            // FF48 - OBP0 - Sprite Palette 0
            union {
                byte SpritePalette0;
                struct __attribute__((packed)) {
                    PixelShade Sprite0Data3:2;
                    PixelShade Sprite0Data2:2;
                    PixelShade Sprite0Data1:2;
                    PixelShade Sprite0Data0:2;
                };
            };      
            // FF49 - OBP1 - Sprite Palette 1
            union {
                byte SpritePalette1;
                struct __attribute__((packed)) {
                    PixelShade Sprite1Data3:2;
                    PixelShade Sprite1Data2:2;
                    PixelShade Sprite1Data1:2;
                    PixelShade Sprite1Data0:2;
                };
            };    
            // FF4A - WY - Window Y position
            byte WindowTop;
            // FF4B - WX - Window X position
            byte WindowLeft;
            
            const byte unused12;
            // FF4D - KEY1 - GBC Speed Switch
            union {
                byte GBCSpeedSwitch;
                struct __attribute__((packed)) {
                    bool IsInDoubleSpeedMode:1;
                    byte unused13:6;
                    bool IsSpeedSwitchPending:1;
                };
            };
            const byte unused14;
            // FF4F - VBK - VRAM Bank selector
            byte GbcVRAMBank;
            // FF50 - ??? - Disable BIOS
            byte BiosSwitch;
            // FF51 HDMA1, HDMA2 - Big endian gbc hdma source address.
            natural GbcHdmaSource;
            // FF53 HDMA3, HDMA4 = Big endian gbc hdma destination address.
            natural GbcHdmaDestination;
            // FF55 - HDMA5 Gbc HDMA control
            union {
                byte GbcHdmaControl;
                struct __attribute__((packed)) {
                    bool IsDmaHBlankMode:1;
                    byte DmaTransferLength:7;
                };
            };
            // FF56 RP - Gbc Infrared Port
            union {
                byte GbcInfraredIo;
                struct __attribute__((packed)) {
                    GbcInfraredMode GbcInfraredMode:2;
                    const byte unused16:4;
                    bool IsInfraredReceiving:1;
                    bool IsInfraredLedOn:1;
                };
            };
            const byte unused17[17];
            // FF68 - BGPI - Gbc Background Palette Index
            // (Also known as BCPS)
            union {
                byte GbcBackgroundPaletteIndexRegister;
                struct __attribute__((packed)) {
                    bool IsBgIndexAutoIncremented:1;
                    const byte unused18:1;
                    byte GbcBackgroundPaletteIndex:6;
                };
            };
            // FF69 - BGPD - Gbc Background Palette Data
            // (Also known as BCPD)
            byte GbcBackroundPaletteDataOut;
            // FF6A - OBPI - Gbc Sprite Palette Index
            // (Also known as OCPS)
            union {
                byte GbcSpritePaletteIndexRegister;
                struct __attribute__((packed)) {
                    bool IsSpriteIndexAutoIncremented:1;
                    const byte unused19:1;
                    byte GbcSpritePaletteIndex:6;
                };
            };
            // FF6B - OBPD - Gbc Sprite Palette Data
            // (Also known as OCPD)
            byte GbcSpritePaletteDataOut;
            const byte unused1A[4];
            // FF70 - SVBK - Gbc WRAM Bank switch
            union {
                byte GbcRamBankSelectRegister;
                struct __attribute__((packed)) {
                    const byte unused1B:5;
                    byte GbcRamBankSelector:3;
                };
            };
            const byte unused1C[15];
            // FF80 - Actual HRAM
            byte mem_HRAM[127];
            // FFFF - IE - Interrupt Switch
            union {
                byte InterruptSwitch;
                struct __attribute__((packed)) {
                    const byte unused1D:3;
                    bool IsJoypadInterruptEnabled:1;
                    bool IsLinkInterruptEnabled:1;
                    bool IsTimerInterruptEnabled:1;
                    bool IsLcdInterruptEnabled:1;
                    bool IsVBlankInterruptEnabled:1;
                };
            };
        };
    };

    bool lcd_entered_hblank; // Set at the end of every HBlank.
    bool lcd_entered_vblank; // Set at the beginning of every VBlank.

    u16* TextureBuffer;
    u16* LastBuffer;
    u16* NextBuffer;

    // The duration of the last intruction. Normally just
    // the CPU executing the instruction, but the MMU could
    // take longer in the case of some DMA ops.
    u32 last_op_cycles;

    bool in_bios:1; // At start BIOS is temporarily mapped at 0000-0100.
    bool halt_for_interrupts:1; // Don't run instructions until interrupt.

    // As controlled by the di and ei instructions.
    bool interrupts_master_enabled:1;

    int io_lcd_mode_cycles_left;
    
    // Palette data for both monochrome (non-CGB) and color (CGB)
    u8 io_lcd_BGPD[0x40]; // Background palettes (color, CGB)
    u8 io_lcd_OBPD[0x40]; // Sprite/object palettes for CGB.

    u16 InternalClock;

    // Not actually a thing in a GB, it's actually a complicated algorithm based on bits 3 through 9 of InternalClock
    u8 TIMAClock;
    bool IsTimerPending;

    u8 io_buttons_dirs;
    u8 io_buttons_buttons;

    // CGB DMA transfers (HDMA)
    bool io_hdma_running:1;
    u16 io_hdma_next_src, io_hdma_next_dst;


    // memory bank controller supporting vars
    byte RomBankLower;
    byte RomBankUpper;
    byte SRAMBankNumber;
    byte WRAMBankCount;
    byte VRAMBankCount;
    // MBC1 - Upper bits ROM bank (if selected).
    
    // MBC1 - Mode for above field (ROM/RAM).
    BankSelectType RomRamSelect;

    // Cartridge hardware metadata
    byte mbc;
    bool hasSRAM;
    bool hasBattery;
    bool hasRumble;
    bool hasRTC;

    // Misc flags.
    bool isRTCLatched;
    bool isVramDirty;
    bool isSRAMDisabled; //Writing 0 to MBC 1 turns off access to external RAM 
    bool isSRAMDirty; // Write battery-backed RAM periodically when dirty.

    byte controllerSlot;

    struct emu_cpu_state *emu_cpu_state;
} GbState;

register u8 rA asm ("$21"); // $s5
register union {
    u8 rF;
    struct __attribute__((packed)) {
        u8 ZF:1;
        u8 NF:1;
        u8 HF:1;
        u8 CF:1;
        u8 pad1:1;
        u8 pad2:1;
        u8 pad3:1;
        u8 pad4:1;
    } flags;
} FF asm ("$22"); // $s6
register u16 PC asm ("$23"); // $s7

typedef struct player_input GbController;


#endif
