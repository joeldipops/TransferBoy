/*
 * Flag info per instruction:
 * http://www.chrisantonellis.com/files/gameboy/gb-instructions.txt
 * http://gameboy.mongenel.com/dmg/opcodes.html
 *
 * BIOS explanation:
 * https://realboyemulator.wordpress.com/2013/01/03/a-look-at-the-game-boy-bootstrap-let-the-fun-begin/
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gbz80ops.h"
#include "cpu.h"
#include "mmu.h"
#include "hwdefs.h"

#define cpu_error(fmt, ...) \
    do { \
    } while (0)

static const u8 flagmasks[] = { FLAG_Z, FLAG_Z, FLAG_C, FLAG_C };

static int cycles_per_instruction[] = {
  /* 0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f       */
     4, 12,  8,  8,  4,  4,  8,  4, 20,  8,  8,  8,  4,  4,  8,  4, /* 0 */
     4, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8,  8,  4,  4,  8,  4, /* 1 */
     8, 12,  8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4, /* 2 */
     8, 12,  8,  8, 12, 12, 12,  4,  8,  8,  8,  8,  4,  4,  8,  4, /* 3 */
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, /* 4 */
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, /* 5 */
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, /* 6 */
     8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4, /* 7 */
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, /* 8 */
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, /* 9 */
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, /* a */
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  8,  4,  8,  4, /* b */
     8, 12, 12, 16, 12, 16,  8, 16,  8, 16, 12,  0, 12, 24,  8, 16, /* c */
     8, 12, 12,  4, 12, 16,  8, 16,  8, 16, 12,  4, 12,  4,  8, 16, /* d */
    12, 12,  8,  4,  4, 16,  8, 16, 16,  4, 16,  4,  4,  4,  8, 16, /* e */
    12, 12,  8,  4,  4, 16,  8, 16, 12,  8, 16,  4,  0,  4,  8, 16, /* f */
};

static int cycles_per_instruction_cb[] = {
  /* 0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f       */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* 0 */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* 1 */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* 2 */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* 3 */
     8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8, /* 4 */
     8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8, /* 5 */
     8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8, /* 6 */
     8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8, /* 7 */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* 8 */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* 9 */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* a */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* b */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* c */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* d */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* e */
     8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8, /* f */
};

void cpu_init_emu_cpu_state(GbState *s) {
    s->emu_cpu_state = calloc(1, sizeof(struct emu_cpu_state));
    s->emu_cpu_state->reg8_lut[0] = &s->reg8.B;
    s->emu_cpu_state->reg8_lut[1] = &s->reg8.C;
    s->emu_cpu_state->reg8_lut[2] = &s->reg8.D;
    s->emu_cpu_state->reg8_lut[3] = &s->reg8.E;
    s->emu_cpu_state->reg8_lut[4] = &s->reg8.H;
    s->emu_cpu_state->reg8_lut[5] = &s->reg8.L;
    s->emu_cpu_state->reg8_lut[6] = NULL;
    s->emu_cpu_state->reg8_lut[7] = &s->reg8.A;
    s->emu_cpu_state->reg16_lut[0] = &s->reg16.BC;
    s->emu_cpu_state->reg16_lut[1] = &s->reg16.DE;
    s->emu_cpu_state->reg16_lut[2] = &s->reg16.HL;
    s->emu_cpu_state->reg16_lut[3] = &s->sp;
    s->emu_cpu_state->reg16s_lut[0] = &s->reg16.BC;
    s->emu_cpu_state->reg16s_lut[1] = &s->reg16.DE;
    s->emu_cpu_state->reg16s_lut[2] = &s->reg16.HL;
    s->emu_cpu_state->reg16s_lut[3] = &s->reg16.AF;
}

/* Resets the CPU state (registers and such) to the state at bootup. */
void cpu_reset_state(GbState *s) {
    s->reg16.AF = 0x01B0;
    s->reg16.BC = 0x0013;
    s->reg16.DE = 0x00D8;
    s->reg16.HL = 0x014D;

    s->sp = 0xFFFE;
    s->pc = 0x0100;

    if (s->gb_type == GB_TYPE_CGB) {
        s->reg16.AF = 0x1180;
        s->reg16.BC = 0x0000;
        s->reg16.DE = 0xff56;
        s->reg16.HL = 0x000d;
    }

    s->halt_for_interrupts = 0;
    s->interrupts_master_enabled = 1;
    s->interrupts_enable  = 0x0;
    s->interrupts_request = 0x0;

    s->io_lcd_mode_cycles_left = 0;
    s->io_lcd_SCX  = 0x00;
    s->io_lcd_SCY  = 0x00;
    s->io_lcd_WX   = 0x00;
    s->io_lcd_WY   = 0x00;
    s->io_lcd_LCDC = 0x91;
    s->io_lcd_STAT = 0x00;
    s->io_lcd_LY   = 0x00;
    s->io_lcd_LYC  = 0x00;
    s->io_lcd_BGP  = 0xfc;
    s->io_lcd_OBP0 = 0xff;
    s->io_lcd_OBP1 = 0xff;
    s->io_lcd_BGPI = 0x00;
    s->io_lcd_OBPI = 0x00;
    memset(s->io_lcd_BGPD, 0, sizeof(s->io_lcd_BGPD));
    memset(s->io_lcd_OBPD, 0, sizeof(s->io_lcd_OBPD));

    s->io_timer_DIV_cycles = 0x00;
    s->io_timer_DIV  = 0x00;
    s->io_timer_TIMA_cycles = 0x00;
    s->io_timer_TIMA = 0x00;
    s->io_timer_TMA  = 0x00;
    s->io_timer_TAC  = 0x00;

    s->io_serial_data    = 0x00;
    s->io_serial_control = 0x00;

    s->io_infrared = 0x00;

    s->io_buttons = 0x00;
    s->io_buttons_dirs = 0x0f;
    s->io_buttons_buttons = 0x0f;

    s->io_sound_enabled = 0xf1;
    s->io_sound_out_terminal = 0xf3;
    s->io_sound_terminal_control = 0x77;

    s->io_sound_channel1_sweep = 0x80;
    s->io_sound_channel1_length_pattern = 0xbf;
    s->io_sound_channel1_envelope = 0xf3;
    s->io_sound_channel1_freq_lo = 0x00;
    s->io_sound_channel1_freq_hi = 0xbf;

    s->io_sound_channel2_length_pattern = 0x3f;
    s->io_sound_channel2_envelope = 0x00;
    s->io_sound_channel2_freq_lo = 0x00;
    s->io_sound_channel2_freq_hi = 0xbf;

    s->io_sound_channel3_enabled = 0x7f;
    s->io_sound_channel3_length = 0xff;
    s->io_sound_channel3_level = 0x9f;
    s->io_sound_channel3_freq_lo = 0x00;
    s->io_sound_channel3_freq_hi = 0xbf;
    memset(s->io_sound_channel3_ram, 0, sizeof(s->io_sound_channel3_ram));

    s->io_sound_channel4_length = 0xff;
    s->io_sound_channel4_envelope = 0x00;
    s->io_sound_channel4_poly = 0x00;
    s->io_sound_channel4_consec_initial = 0xbf;


    s->mem_bank_rom = 1;
    s->mem_bank_wram = 1;
    s->mem_bank_extram = 0;
    s->mem_bank_vram = 0;

    s->mem_mbc1_extrambank = 0;
    s->mem_mbc1_rombankupper = 0;
    s->mem_mbc1_romram_select = 0;
    s->mem_mbc3_extram_rtc_select = 0;

    memset(s->mem_WRAM, 0, s->mem_num_banks_wram * WRAM_BANKSIZE);
    memset(s->mem_EXTRAM, 0, s->mem_num_banks_extram * EXTRAM_BANKSIZE);
    memset(s->mem_VRAM, 0, s->mem_num_banks_vram * VRAM_BANKSIZE);
    memset(s->mem_OAM, 0, 0xa0);
    memset(s->mem_HRAM, 0, 0x7f);

    s->mem_latch_rtc = 0x01;
    memset(s->mem_RTC, 0, 0x05);
}

static void cpu_handle_interrupts(GbState *s) {
    u8 interrupts = s->interrupts_enable & s->interrupts_request;

    if (s->interrupts_master_enabled) {
        for (int i = 0; i < 5; i++) {
            if (interrupts & (1 << i)) {
                s->interrupts_master_enabled = 0;
                s->interrupts_request ^= 1 << i;

                mmu_push16(s, s->pc);

                s->pc = i * 0x8 + 0x40;

                s->halt_for_interrupts = 0;
                return;
            }
        }
    } else if (interrupts) {
        s->halt_for_interrupts = 0;
    }
}

void cpu_timers_step(GbState *s) {
    u32 freq = s->double_speed ? GB_FREQ : 2 * GB_FREQ;
    u32 div_cycles_per_tick = freq / GB_DIV_FREQ;
    s->io_timer_DIV_cycles += s->emu_state->last_op_cycles;
    if (s->io_timer_DIV_cycles >= div_cycles_per_tick) {
        s->io_timer_DIV_cycles %= div_cycles_per_tick;
        s->io_timer_DIV++;
    }

    if (s->io_timer_TAC & (1<<2)) { /* Timer enable */
        s->io_timer_TIMA_cycles += s->emu_state->last_op_cycles;
        u32 timer_hz = GB_TIMA_FREQS[s->io_timer_TAC & 0x3];
        u32 timer_cycles_per_tick = freq / timer_hz;
        if (s->io_timer_TIMA_cycles >= timer_cycles_per_tick) {
            s->io_timer_TIMA_cycles %= timer_cycles_per_tick;
            s->io_timer_TIMA++;
            if (s->io_timer_TIMA == 0) {
                s->io_timer_TIMA = s->io_timer_TMA;
                s->interrupts_request |= 1 << 2;
            }
        }
    }
}

#define CF s->flags.CF
#define HF s->flags.HF
#define NF s->flags.NF
#define ZF s->flags.ZF
#define a s->reg8.A
#define F s->reg8.F
#define b s->reg8.B
#define C s->reg8.C
#define D s->reg8.D
#define E s->reg8.E
#define H s->reg8.H
#define l s->reg8.L
#define AF s->reg16.AF
#define BC s->reg16.BC
#define DE s->reg16.DE
#define HL s->reg16.HL
#define M(op, value, mask) (((op) & (mask)) == (value))
#define mem(loc) (mmu_read(s, loc))
#define IMM8  (mmu_read(s, s->pc))
#define IMM16 (mmu_read(s, s->pc) | (mmu_read(s, s->pc + 1) << 8))
#define REG8(bitpos) s->emu_cpu_state->reg8_lut[(op >> bitpos) & 7]
#define REG16(bitpos) s->emu_cpu_state->reg16_lut[((op >> bitpos) & 3)]
#define REG16S(bitpos) s->emu_cpu_state->reg16s_lut[((op >> bitpos) & 3)]
#define FLAG(bitpos) ((op >> bitpos) & 3)

void cpu_do_cb_instruction(GbState *s) {
    u8 op = mmu_read(s, s->pc++);

    u8 maskedF8 = op & 0xf8;

    if (maskedF8 == 0x00) { /* RLC reg8 */
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val << 1) | (val >> 7);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val >> 7;
        if (reg) *reg = res; else mmu_write(s, HL, res);
    } else if (maskedF8 == 0x08) { /* RRC reg8 */
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val >> 1) | ((val & 1) << 7);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val & 1;
        if (reg) *reg = res; else mmu_write(s, HL, res);
    } else if (maskedF8 == 0x10) { /* RL reg8 */
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val << 1) | (CF ? 1 : 0);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val >> 7;
        if (reg) *reg = res; else mmu_write(s, HL, res);
    } else if (maskedF8 == 0x18) { /* RR reg8 */
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val >> 1) | (CF << 7);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val & 0x1;
        if (reg) *reg = res; else mmu_write(s, HL, res);
    } else if (maskedF8 == 0x20) { /* SLA reg8 */
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        CF = val >> 7;
        val = val << 1;
        ZF = val == 0;
        NF = 0;
        HF = 0;
        if (reg) *reg = val; else mmu_write(s, HL, val);
    } else if (maskedF8 == 0x28) { /* SRA reg8 */
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        CF = val & 0x1;
        val = (val >> 1) | (val & (1<<7));
        ZF = val == 0;
        NF = 0;
        HF = 0;
        if (reg) *reg = val; else mmu_write(s, HL, val);
    } else if (maskedF8 == 0x30) { /* SWAP reg8 */
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = ((val << 4) & 0xf0) | ((val >> 4) & 0xf);
        F = res == 0 ? FLAG_Z : 0;
        if (reg) *reg = res; else mmu_write(s, HL, res);
    } else if (maskedF8 == 0x38) { /* SRL reg8 */
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        CF = val & 0x1;
        val = val >> 1;
        ZF = val == 0;
        NF = 0;
        HF = 0;
        if (reg) *reg = val; else mmu_write(s, HL, val);
    } else if (M(op, 0x40, 0xc0)) { /* BIT bit, reg8 */
        u8 bit = (op >> 3) & 7;
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        ZF = ((val >> bit) & 1) == 0;
        NF = 0;
        HF = 1;
    } else if (M(op, 0x80, 0xc0)) { /* RES bit, reg8 */
        u8 bit = (op >> 3) & 7;
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        val = val & ~(1<<bit);
        if (reg) *reg = val; else mmu_write(s, HL, val);
    } else if (M(op, 0xc0, 0xc0)) { /* SET bit, reg8 */
        u8 bit = (op >> 3) & 7;
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        val |= (1 << bit);
        if (reg) *reg = val; else mmu_write(s, HL, val);
    } else {
        s->pc -= 2;
        cpu_error("Unknown instruction");
    }
}

static void cpu_do_instruction(GbState *s) {
    u8 op = getOpCodeFromROM(s, s->pc++);
    opTable[op](s, op);
}

void cpu_step(GbState *s) {
    u8 op;

    s->emu_state->last_op_cycles = 0;

    cpu_handle_interrupts(s);
   
    op = getOpCodeFromROM(s, s->pc);
    
    s->emu_state->last_op_cycles = cycles_per_instruction[op];
    if (op == 0xcb) {
        op = getOpCodeFromROM(s, s->pc + 1);
        s->emu_state->last_op_cycles = cycles_per_instruction_cb[op];
    }

    if (!s->halt_for_interrupts) {
        //if (true) {
            cpu_do_instruction(s);
        //} else {
          //  cpu_do_instruction_tree(s);
        //}
    } else {
        if (!s->interrupts_enable)
            cpu_error("Waiting for interrupts while disabled, deadlock.\n");
    }
    /*
    // Commented out to save a few instructions per cycle.
    if (s->pc >= 0x8000 && s->pc < 0xa000)
        cpu_error("PC in VRAM: %.4x\n", s->pc);
    else if (s->pc >= 0xa000 && s->pc < 0xc000)
        cpu_error("PC in external RAM: %.4x\n", s->pc);
    else if (s->pc >= 0xe000 && s->pc < 0xff80)
        cpu_error("PC in ECHO/OAM/IO/unusable: %.4x\n", s->pc);
    */
}
