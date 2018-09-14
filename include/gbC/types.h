#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

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
    u16 *lcd_pixbuf; /* 2-bit or 15-bit color per pixel. */

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

/* State of the cpu part of the emulation, not of the hardware. */
struct emu_cpu_state;

enum gb_type {
    GB_TYPE_GB,
    GB_TYPE_CGB,
};

/* TODO split this up into module-managed components (cpu, mmu, ...) */
struct gb_state {

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
};


#endif
