#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "../../core.h"

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

/* State of the emulator itself, not of the hardware. */
struct emu_state {
    bool quit;
    bool make_savestate;

    bool audio_enable;
    u8 *audio_sndbuf;

    bool lcd_entered_hblank; /* Set at the end of every HBlank. */
    bool lcd_entered_vblank; /* Set at the beginning of every VBlank. */
    u16* LastBuffer;
    u16* NextBuffer;

    bool flush_extram; /* Flush battery-backed RAM when it's disabled. */
    bool extram_dirty; /* Write battery-backed RAM periodically when dirty. */

    bool dbg_break_next;
    bool dbg_print_disas;
    bool dbg_print_mmu;
    u16 dbg_breakpoint;

    u32 last_op_cycles; /* The duration of the last intruction. Normally just
                           the CPU executing the instruction, but the MMU could
                           take longer in the case of some DMA ops. */
    u32 time_cycles;
    u32 time_seconds;

    char state_filename_out[1024];
    char save_filename_out[1024];
};

#define WAVEDATA_LENGTH 16

typedef enum { VRAM_8800 = 0, VRAM_8000 = 1 } TileDataAddress;
typedef enum { VRAM_9800 = 0, VRAM_9C00 = 1 } TileMapAddress;
typedef enum { SPRITE_8x8 = 0, SPRITE_8x16 = 1} SpriteSize;
typedef enum {
    HBLANK = 0,
    VBLANK = 1,
    OAM_IN_USE = 2,
    VRAM_IN_USE = 3
} LcdMode;

typedef enum {
    DARKEST = 3,
    DARKER = 2,
    LIGHTER = 1,
    LIGHTEST = 0
} PixelShade;

typedef enum {
    SoundConsecutive = 0,
    SoundWait = 1
} SoundTimingMode;

typedef enum {
    SoundMute = 0,
    SoundNoShift = 1,
    SoundHalfShift = 2,
    SoundQuarterShift = 3
} WavePatternShift;

typedef union {
    byte HRAM[0xff];
    struct {
        // P1 - JoypadIo
        union {
            byte JoypadIo;
            struct {
                byte unused0:2;
                byte SelectFace:1;
                byte SelectDPad:1;                
                union {
                    struct {
                        byte Down:1;
                        byte Up:1;
                        byte Left:1;
                        byte Right:1;
                    };
                    struct {
                        byte Start:1;
                        byte Select:1;
                        byte B:1;
                        byte A:1;
                    };
                };
            };
        };
        // SB - Link Data
        byte LinkData;
        // SC - Link Control
        union {
            byte LinkControl;
            struct {
                bool IsTransferToStart:1;
                byte unused1:6;
                bool IsClockExternal:1;
            };
        };
        byte unused2;
        // DIV - Timer Clock
        byte TimerClock;
        // TIMA - Timer Counter
        byte TimerCounter;
        // TMA - Timer Reset Value
        byte TimerResetValue;
        // TAC - Timer Control
        union {
            byte TimerControl;
            struct {
                byte unused3:5;
                bool IsTimerStarted:1;
                byte TimerFrequency:2;
            };
        };
        byte unused4[14];
        // IF - Interrupt Flags
        union {
            byte InterruptFlags;
            struct {
                byte unused5:3;
                byte JoypadInterrupt:1;
                byte LinkInterrupt:1;
                byte TimerInterrupt:1;
                byte LcdInterrupt:1;
                byte VBlankInterrupt:2;
            };
        };
        // NR10 - AudioChannel1Sweep
        union {
            byte AudioChannel1Sweep;
            struct {
                byte unused6:1;
                byte AudioChannel1SweepTime:3;
                bool AudioChannel1IsFrequencyDecrease:1;
                byte AudioChannel1SweepShift:3;
            };
        };
        // NR11 - AudioChannel1PatternAndLength
        union {
            byte AudioChannel1PatternAndLength;
            struct {
                byte AudioChannel1WavePatternDuty:2;
                byte AudioChannel1SoundLength:6;
            };
        };
        // NR12 - AudioChannel1Envelope
        union {
            byte AudioChannel1Envelope;
            struct {
                byte AudioChannel1InitialVolume:4;
                bool AudioChannel1IsVolumeIncreasing:1;
                byte AudioChannel1EnvelopeSteps:3;                
            };
        };
        // NR13 - AudioChannel1Frequency
        byte AudioChannel1FrequencyLow;
        // NR14 - AudioChannel1Flags
        union {
            byte AudioChannel1Flags;
            struct {
                bool AudioChannel1IsSoundReset:1;
                SoundTimingMode AudioChannel1TimingMode:1;
                byte unused7:4;
                byte AudioChannel1FrequencyHigh:3;                    
            };
        };
        byte unused8; //reserved for channel 2 sweep
        // NR21 - AudioChannel2PatternAndLength
        union {
            byte AudioChannel2PatternAndLength;
            struct {
                byte AudioChannel2WavePatternDuty:2;
                byte AudioChannel2SoundLength:6;
            };
        };
        // NR22 - AudioChannel2Envelope
        union {
            byte AudioChannel2Envelope;
            struct {
                byte AudioChannel2InitialVolume:4;
                bool AudioChannel2IsVolumeIncreasing:1;
                byte AudioChannel2EnvelopeSteps:3;                
            };
        };
        // NR23 - AudioChannel1Frequency
        byte AudioChannel2FrequencyLow;
        // NR24 - AudioChannel2Flags
        union {
            byte AudioChannel2Flags;
            struct {
                bool AudioChannel2IsSoundReset:1;
                SoundTimingMode AudioChannel2TimingMode:1;
                byte unused9:3;
                byte AudioChannel2FrequencyHigh:3;                    
            };
        };
        // NR30 - AudioChannel3Control
        union {
            byte AudioChannel3Control;
            struct {
                bool AudioChannel3IsOn:1;
                byte unusedA:7;
            };
        };
        // NR31 - AudioChannel3Length
        byte AudioChannel3Length;
        // NR32 - AudioChannel3Level
        union {
            byte AudioChannel3Level;
            struct {
                byte unusedB:1;
                WavePatternShift AudioChannel3WavePatternShift:2;
                byte unusedC:5;
            };
        };
        // NR33 - AudioChannel3Frequency
        byte AudioChannel3FrequencyLow;
        // NR34 - AudioChannel3Flags
        union {
            byte AudioChannel3Flags;
            struct {
                bool AudioChannel3IsSoundReset:1;
                SoundTimingMode AudioChannel3TimingMode:1;
                byte unusedD:3;
                byte AudioChannel3FrequencyHigh:3;                    
            };
        };  
        byte unusedE; // reserved for channel 4 sweep      
        // NR41 - AudioChannel4Length
        byte AudioChannel4Length;
        // NR42 - AudioChannel4Envelope
        union {
            byte AudioChannel4Envelope;
            struct {
                byte AudioChannel4InitialVolume:4;
                bool AudioChannel4IsVolumeIncreasing:1;
                byte AudioChannel4EnvelopeSteps:3;                
            };
        };
        // NR43 - AudioChannel4RNGParameters
        union {
            byte AudioChannel4RNGParameters;
            struct {
                byte NoiseRatioShift:4;
                byte NoiseStepsCode: 1;
                byte NoiseRatioFactor:3;                
            };
        };
        // NR44 AudioChannel4Flags
        union {
            byte AudioChannel4Flags;
            struct {
                bool AudioChannel4IsSoundReset:1;
                SoundTimingMode AudioChannel4TimingMode:1;
                byte unusedF:6;
            };
        };
        // NR50 - Audio Speaker Control
        union {
            byte AudioSpeakerControl;
            struct {
                bool IsLeftTerminalEnabled:1; // S02
                byte LeftTerminalVolume:3;
                bool IsRightTerminalEnabled:1; //S01
                byte RightTerminalVolume:3;                
            };
        };
        // NR51 - Audio Speaker Channels
        union {
            byte AudioSpeakerChannels;
            struct {
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
        // NR52 - Audio Channel Switch
        union {
            byte AudioChannelSwitch;
            struct {
                bool IsSoundEnabled:1;
                byte pad:3;
                bool IsChannel4Enabled:1;
                bool IsChannel3Enabled:1;
                bool IsChannel2Enabled:1;
                bool IsChannel1Enabled:1;
            };
        };
        byte unused10[10];
        byte SoundWaveData[WAVEDATA_LENGTH];       
        // LCDC - LCD Control
        union {
            byte LcdControl;
            struct {
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
        // STAT - LCD Status
        union {
            byte LcdStatus;
            struct {
                byte unused11:1;
                bool InterruptOnLineLYC:1;
                bool InterruptOnOamMode:1;
                bool InterruptOnVBlank:1;
                bool InterruptOnHBlank:1;
                bool IsCurrentLineLYC:1;
                LcdMode LcdMode:2;
            };
        };
        // SCY - Background Scroll Y
        byte BackgroundScrollY;
        // SCX - Background Scroll X
        byte BackgroundScrollX;
        // LY - Currently Drawing Line
        byte CurrentLine;
        // LCY - Next Interrupt Line
        byte NextInterruptLine;
        // DMA - Dma Source
        byte DmaSourceAddress;
        // BGP - Background Palette
        union {
            byte BackgroundPalette;
            struct {
                PixelShade BgData3:2;
                PixelShade BgData2:2;
                PixelShade BgData1:2;
                PixelShade BgData0:2;
            };
        };
        // OBP0 - Sprite Palette 0
        union {
            byte SpritePalette0;
            struct {
                PixelShade Sprite0Data3:2;
                PixelShade Sprite0Data2:2;
                PixelShade Sprite0Data1:2;
                PixelShade Sprite0Data0:2;
            };
        };      
        // OBP1 - Sprite Palette 1
        union {
            byte SpritePalette1;
            struct {
                PixelShade Sprite1Data3:2;
                PixelShade Sprite1Data2:2;
                PixelShade Sprite1Data1:2;
                PixelShade Sprite1Data0:2;
            };
        };    
        // WY - Window Y position
        byte WindowTop;
        // WX - Window X position
        byte WindowLeft;
        byte unused12[52];
        // Actual HRAM
        byte Ram[127];
        // IE - Interrupt Switch
        union {
            byte InterruptSwitch;
            struct {
                byte unused13:3;
                bool IsJoypadInterruptEnabled:1;
                bool IsLinkInterruptEnabled:1;
                bool IsTimerInterruptEnabled:1;
                bool IsLcdInterruptEnabled:1;
                bool IsVBlankInterruptEnabled:1;
            };
        };
    };
} HRAM;

/* State of the cpu part of the emulation, not of the hardware. */
struct emu_cpu_state;

enum gb_type {
    GB_TYPE_GB,
    GB_TYPE_CGB,
};

typedef struct {

    /*
     * CPU state (registers, interrupts, etc)
     */
    /* Registers: allow access to 8-bit and 16-bit regs, and via array. */
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

    char in_bios:1; /* At start BIOS is temporarily mapped at 0000-0100. */
    char halt_for_interrupts:1; /* Don't run instructions until interrupt. */
    char double_speed:1; /* CGB: we can run at double CPU speed. */

    u8 interrupts_master_enabled:1;
    u8 interrupts_enable; /* Bitmask of which interrupts are enabled. */
    u8 interrupts_request; /* Bitmask of which interrupts are pending. */


    /*
     * I/O ports (and some additional variables to manage I/O)
     */

    int io_lcd_mode_cycles_left;
    u8 io_lcd_SCX;  /* BG scroll X */
    u8 io_lcd_SCY;  /* BG scroll Y */
    u8 io_lcd_WX;   /* Window X */
    u8 io_lcd_WY;   /* Window Y */
    u8 io_lcd_LCDC; /* LCD Control bits:
                        0: BG enable,
                        1: OBJ/sprites enable,
                        2: OBJ/sprites-size (8x8 or 8x16),
                        3: BG tilemap (9800-9bff or 9c00-9fff),
                        4: BG+Win tile data (8800-97FF+signed idx or 8000-8FFF),
                        5: Window enable,
                        6: Window tilemap (9800-9bff or 9c00-9fff),
                        7: LCD enable */
    u8 io_lcd_STAT; /* LCD Status bits
                        0-1: Current mode,
                          2: Current LYC==LY status,
                          3: Trigger interrupt on entering mode 0,
                          4: Trigger interrupt on entering mode 1,
                          5: Trigger interrupt on entering mode 2,
                          6: Trigger interrupt on LY=LYC */
    u8 io_lcd_LY;   /* Current LCD Y line */
    u8 io_lcd_LYC;  /* LCD Y line compare  */

    /* Palette data for both monochrome (non-CGB) and color (CGB) */
    u8 io_lcd_BGP;  /* Background palette data (monochrome, non-CGB) */
    u8 io_lcd_OBP0; /* Object palette 0 data (monochrome, non-CGB) */
    u8 io_lcd_OBP1; /* Object palette 1 data (monochrome, non-CGB) */
    u8 io_lcd_BGPI; /* Background palette index (color, CGB) */
    u8 io_lcd_BGPD[0x40]; /* Background palettes (color, CGB) */
    u8 io_lcd_OBPI; /* Sprite palette index (color, CGB) */
    u8 io_lcd_OBPD[0x40]; /* Sprite/object palettes for CGB. */

    u8 io_timer_DIV;
    u32 io_timer_DIV_cycles;
    u8 io_timer_TIMA;
    u32 io_timer_TIMA_cycles;
    u8 io_timer_TMA;
    u8 io_timer_TAC;

    u8 io_serial_data;
    u8 io_serial_control;

    u8 io_infrared;

    u8 io_buttons;
    u8 io_buttons_dirs;
    u8 io_buttons_buttons;

    u8 io_sound_enabled;
    u8 io_sound_out_terminal;
    u8 io_sound_terminal_control;

    u8 io_sound_channel1_sweep;
    u8 io_sound_channel1_length_pattern;
    u8 io_sound_channel1_envelope;
    u8 io_sound_channel1_freq_lo;
    u8 io_sound_channel1_freq_hi;

    u8 io_sound_channel2_length_pattern;
    u8 io_sound_channel2_envelope;
    u8 io_sound_channel2_freq_lo;
    u8 io_sound_channel2_freq_hi;

    u8 io_sound_channel3_enabled;
    u8 io_sound_channel3_length;
    u8 io_sound_channel3_level;
    u8 io_sound_channel3_freq_lo;
    u8 io_sound_channel3_freq_hi;
    u8 io_sound_channel3_ram[0x10];

    u8 io_sound_channel4_length;
    u8 io_sound_channel4_envelope;
    u8 io_sound_channel4_poly;
    u8 io_sound_channel4_consec_initial;

    /* CGB DMA transfers (HDMA) */
    u8 io_hdma_src_high, io_hdma_src_low;
    u8 io_hdma_dst_high, io_hdma_dst_low;
    u8 io_hdma_status; /* (remaining) length in lower bits, high bit 1=done. */
    char io_hdma_running:1;
    u16 io_hdma_next_src, io_hdma_next_dst;


    /*
     * Memory (MMU) state
     */

    int mem_bank_rom, mem_num_banks_rom;
    int mem_bank_wram, mem_num_banks_wram;
    int mem_bank_extram, mem_num_banks_extram;
    int mem_bank_vram, mem_num_banks_vram;
    u8 mem_mbc1_rombankupper; /* MBC1 - Upper bits ROM bank (if selected). */
    u8 mem_mbc1_extrambank; /* MBC1 - EXT_RAM bank (if selected). */
    u8 mem_mbc1_romram_select; /* MBC1 - Mode for above field (ROM/RAM). */
    u8 mem_mbc3_extram_rtc_select;
    u8 mem_mbc5_extrambank;

    u8 *mem_ROM; /* Between 16K and 4M (banked) */
    u8 *mem_WRAM; /* Internal RAM (WRAM), 8K non-CGB, 32K CGB (banked) */
    u8 *mem_EXTRAM; /* External (cartridge) RAM, optional, max 32K (banked) */
    u8 *mem_VRAM; /* Video RAM, 8K non-CGB, 16K CGB (banked) */
    u8 mem_OAM[0xa0]; /* Sprite/Object attributes */
    u8 mem_HRAM[0x7f];
    u8 *mem_BIOS;

    u8 mem_latch_rtc;
    u8 mem_RTC[0x0c]; /* Real time clock, select by extram banks 0x08-0x0c */


    /*
     * Cartridge hardware (including memory bank controller)
     */

    enum gb_type gb_type;
    int mbc;
    char has_extram;
    char has_battery;
    char has_rtc;


    /*
     * Internal emulator state
     */

    struct emu_state *emu_state;
    struct emu_cpu_state *emu_cpu_state;
} GbState;

struct emu_cpu_state {
    // Lookup tables for the reg-index encoded in instructions to ptr to reg.
    u8 *reg8_lut[9];
    u16 *reg16_lut[4];
    u16 *reg16s_lut[4];
};

typedef struct player_input GbController;


#endif
