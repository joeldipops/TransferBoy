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

#include "types.h"
#include "cpu.h"
#include "mmu.h"
#include "hwdefs.h"
#include <libdragon.h>

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

struct emu_cpu_state {
    /* Lookup tables for the reg-index encoded in instructions to ptr to reg. */
    u8 *reg8_lut[9];
    u16 *reg16_lut[4];
    u16 *reg16s_lut[4];
};

void cpu_init_emu_cpu_state(struct gb_state *s) {
    s->emu_cpu_state = calloc(1, sizeof(struct emu_cpu_state));
    s->emu_cpu_state->reg8_lut[0] = &s->reg8.B;
    s->emu_cpu_state->reg8_lut[1] = &s->reg8.C;
    s->emu_cpu_state->reg8_lut[2] = &s->reg8.D;
    s->emu_cpu_state->reg8_lut[3] = &s->reg8.E;
    s->emu_cpu_state->reg8_lut[4] = &s->reg8.H;
    s->emu_cpu_state->reg8_lut[5] = &s->reg8.L;
    s->emu_cpu_state->reg8_lut[6] = NULL;
    s->emu_cpu_state->reg8_lut[7] = NULL;
    s->emu_cpu_state->reg16_lut[0] = &s->reg16.BC;
    s->emu_cpu_state->reg16_lut[1] = &s->reg16.DE;
    s->emu_cpu_state->reg16_lut[2] = &s->reg16.HL;
    s->emu_cpu_state->reg16_lut[3] = &s->sp;
    s->emu_cpu_state->reg16s_lut[0] = &s->reg16.BC;
    s->emu_cpu_state->reg16s_lut[1] = &s->reg16.DE;
    s->emu_cpu_state->reg16s_lut[2] = &s->reg16.HL;
    s->emu_cpu_state->reg16s_lut[3] = NULL;
}

/* Resets the CPU state (registers and such) to the state at bootup. */
void cpu_reset_state(struct gb_state *s) {
    regA = 0x01;
    s->reg8.F = 0xB0;
    s->reg16.BC = 0x0013;
    s->reg16.DE = 0x00D8;
    s->reg16.HL = 0x014D;

    s->sp = 0xFFFE;
    PC = 0x0100;

    if (s->gb_type == GB_TYPE_CGB) {
        regA = 0x11;
        s->reg8.F = 0x80;
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

static void cpu_handle_interrupts(struct gb_state *s) {
    u8 interrupts = s->interrupts_enable & s->interrupts_request;

    if (s->interrupts_master_enabled) {
        for (int i = 0; i < 5; i++) {
            if (interrupts & (1 << i)) {
                s->interrupts_master_enabled = 0;
                s->interrupts_request ^= 1 << i;

                mmu_push16(s, PC);

                PC = i * 0x8 + 0x40;

                s->halt_for_interrupts = 0;
                return;
            }
        }
    } else if (interrupts) {
        s->halt_for_interrupts = 0;
    }
}

void cpu_timers_step(struct gb_state *s) {
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
#define F s->reg8.F
#define B s->reg8.B
#define C s->reg8.C
#define D s->reg8.D
#define E s->reg8.E
#define H s->reg8.H
#define L s->reg8.L
//#define AF s->reg16.AF
#define BC s->reg16.BC
#define DE s->reg16.DE
#define HL s->reg16.HL
#define M(op, value, mask) (((op) & (mask)) == (value))
#define mem(loc) (mmu_read(s, loc))
#define IMM8  (mmu_read(s, PC))
#define IMM16 (mmu_read(s, PC) | (mmu_read(s, PC + 1) << 8))
#define REG8(bitpos) s->emu_cpu_state->reg8_lut[(op >> bitpos) & 7]
#define REG16(bitpos) s->emu_cpu_state->reg16_lut[((op >> bitpos) & 3)]
#define REG16S(bitpos) s->emu_cpu_state->reg16s_lut[((op >> bitpos) & 3)]
#define FLAG(bitpos) ((op >> bitpos) & 3)

static void cpu_do_cb_instruction(struct gb_state *s) {
    u8 op = mmu_read(s, PC++);

    if ((op & 0x0F) != 0x07 && (op & 0x0F) != 0x0F) {
        goto next;
    }

    switch(op) {
        case 0x27: // sla A
            CF = regA >> 7;
            regA = regA << 1;
            ZF = regA == 0;
            NF = 0;
            HF = 0;
            return;
        case 0x37: { // swap A
            u8 res = ((regA << 4) & 0xf0) | ((regA >> 4) & 0xf);
            F = res == 0 ? FLAG_Z : 0;
            regA = res;
            return;
        }
        case 0x2F: // sra A
            CF = regA & 0x1;
            regA = (regA >> 1) | (regA & (1<<7));
            ZF = regA == 0;
            NF = 0;
            HF = 0;
            return;
        case 0x3F: // srl A 
            CF = regA & 0x1;
            regA = regA >> 1;
            ZF = regA == 0;
            NF = 0;
            HF = 0;
            return;
        case 0x47: // bit 0, A
        case 0x57: // bit 2, A
        case 0x67: // bit 4, A
        case 0x77: // bit 6, A
        case 0x4F: // bit 1, A
        case 0x5F: // bit 3, A
        case 0x6F: // bit 5, A
        case 0x7F: { // bit 7, A        
            u8 bit = (op >> 3) & 7;
            ZF = ((regA >> bit) & 1) == 0;
            NF = 0;
            HF = 1;        
            return;
        }
        case 0x87: // res 0, A
        case 0x97: // res 2, A
        case 0xA7: // res 4, A
        case 0xB7: // res 6, A
        case 0x8F: // res 1, A
        case 0x9F: // res 3, A
        case 0xAF: // res 5, A
        case 0xBF: { // res 7, A        
            u8 bit = (op >> 3) & 7;
            regA = regA & ~(1<<bit);
            return;
        }
        case 0xC7: // set 0, A
        case 0xD7: // set 2, A
        case 0xE7: // set 4, A
        case 0xF7: // set 6, A
        case 0xCF: // set 1, A
        case 0xDF: // set 3, A
        case 0xEF: // set 5, A
        case 0xFF: { // set 7, A     
            u8 bit = (op >> 3) & 7;
            regA |= (1 << bit);
            return;
        }
        default: ;   
    }

    next:;

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
        PC -= 2;
        cpu_error("Unknown instruction");
    }
}

static void cpu_do_instruction(struct gb_state *s) {
    u8 op = getOpCodeFromROM(s, PC++);
    switch (op) {
        case 0x00: goto Ox00;
        case 0x01: goto Ox01;
        case 0x02: goto Ox02;
        case 0x03: goto Ox03;
        case 0x04: goto Ox04;
        case 0x05: goto Ox05;
        case 0x06: goto Ox06;
        case 0x07: goto Ox07;
        case 0x08: goto Ox08;
        case 0x09: goto Ox09;
        case 0x0A: goto Ox0A;
        case 0x0B: goto Ox0B;
        case 0x0C: goto Ox0C;
        case 0x0D: goto Ox0D;
        case 0x0E: goto Ox0E;
        case 0x0F: goto Ox0F;

        case 0x10: goto Ox10;
        case 0x11: goto Ox11;
        case 0x12: goto Ox12;
        case 0x13: goto Ox13;
        case 0x14: goto Ox14;
        case 0x15: goto Ox15;
        case 0x16: goto Ox16;
        case 0x17: goto Ox17;
        case 0x18: goto Ox18;
        case 0x19: goto Ox19;
        case 0x1A: goto Ox1A;
        case 0x1B: goto Ox1B;
        case 0x1C: goto Ox1C;
        case 0x1D: goto Ox1D;
        case 0x1E: goto Ox1E;
        case 0x1F: goto Ox1F;

        case 0x20: goto Ox20;
        case 0x21: goto Ox21;
        case 0x22: goto Ox22;
        case 0x23: goto Ox23;
        case 0x24: goto Ox24;
        case 0x25: goto Ox25;
        case 0x26: goto Ox26;
        case 0x27: goto Ox27;
        case 0x28: goto Ox28;
        case 0x29: goto Ox29;
        case 0x2A: goto Ox2A;
        case 0x2B: goto Ox2B;
        case 0x2C: goto Ox2C;
        case 0x2D: goto Ox2D;
        case 0x2E: goto Ox2E;
        case 0x2F: goto Ox2F;

        case 0x30: goto Ox30;
        case 0x31: goto Ox31;
        case 0x32: goto Ox32;
        case 0x33: goto Ox33;
        case 0x34: goto Ox34;
        case 0x35: goto Ox35;
        case 0x36: goto Ox36;
        case 0x37: goto Ox37;
        case 0x38: goto Ox38;
        case 0x39: goto Ox39;
        case 0x3A: goto Ox3A;
        case 0x3B: goto Ox3B;
        case 0x3C: goto Ox3C;
        case 0x3D: goto Ox3D;
        case 0x3E: goto Ox3E;
        case 0x3F: goto Ox3F;        

        case 0x40: goto Ox40;
        case 0x41: goto Ox41;
        case 0x42: goto Ox42;
        case 0x43: goto Ox43;
        case 0x44: goto Ox44;
        case 0x45: goto Ox45;
        case 0x46: goto Ox46;
        case 0x47: goto Ox47;
        case 0x48: goto Ox48;
        case 0x49: goto Ox49;
        case 0x4A: goto Ox4A;
        case 0x4B: goto Ox4B;
        case 0x4C: goto Ox4C;
        case 0x4D: goto Ox4D;
        case 0x4E: goto Ox4E;
        case 0x4F: goto Ox4F;

        case 0x50: goto Ox50;
        case 0x51: goto Ox51;
        case 0x52: goto Ox52;
        case 0x53: goto Ox53;
        case 0x54: goto Ox54;
        case 0x55: goto Ox55;
        case 0x56: goto Ox56;
        case 0x57: goto Ox57;
        case 0x58: goto Ox58;
        case 0x59: goto Ox59;
        case 0x5A: goto Ox5A;
        case 0x5B: goto Ox5B;
        case 0x5C: goto Ox5C;
        case 0x5D: goto Ox5D;
        case 0x5E: goto Ox5E;
        case 0x5F: goto Ox5F;

        case 0x60: goto Ox60;
        case 0x61: goto Ox61;
        case 0x62: goto Ox62;
        case 0x63: goto Ox63;
        case 0x64: goto Ox64;
        case 0x65: goto Ox65;
        case 0x66: goto Ox66;
        case 0x67: goto Ox67;
        case 0x68: goto Ox68;
        case 0x69: goto Ox69;
        case 0x6A: goto Ox6A;
        case 0x6B: goto Ox6B;
        case 0x6C: goto Ox6C;
        case 0x6D: goto Ox6D;
        case 0x6E: goto Ox6E;
        case 0x6F: goto Ox6F;

        case 0x70: goto Ox70;
        case 0x71: goto Ox71;
        case 0x72: goto Ox72;
        case 0x73: goto Ox73;
        case 0x74: goto Ox74;
        case 0x75: goto Ox75;
        case 0x76: goto Ox76;
        case 0x77: goto Ox77;
        case 0x78: goto Ox78;
        case 0x79: goto Ox79;
        case 0x7A: goto Ox7A;
        case 0x7B: goto Ox7B;
        case 0x7C: goto Ox7C;
        case 0x7D: goto Ox7D;
        case 0x7E: goto Ox7E;
        case 0x7F: goto Ox7F;

        case 0x80: goto Ox80;
        case 0x81: goto Ox81;
        case 0x82: goto Ox82;
        case 0x83: goto Ox83;
        case 0x84: goto Ox84;
        case 0x85: goto Ox85;
        case 0x86: goto Ox86;
        case 0x87: goto Ox87;
        case 0x88: goto Ox88;
        case 0x89: goto Ox89;
        case 0x8A: goto Ox8A;
        case 0x8B: goto Ox8B;
        case 0x8C: goto Ox8C;
        case 0x8D: goto Ox8D;
        case 0x8E: goto Ox8E;
        case 0x8F: goto Ox8F;

        case 0x90: goto Ox90;
        case 0x91: goto Ox91;
        case 0x92: goto Ox92;
        case 0x93: goto Ox93;
        case 0x94: goto Ox94;
        case 0x95: goto Ox95;
        case 0x96: goto Ox96;
        case 0x97: goto Ox97;
        case 0x98: goto Ox98;
        case 0x99: goto Ox99;
        case 0x9A: goto Ox9A;
        case 0x9B: goto Ox9B;
        case 0x9C: goto Ox9C;
        case 0x9D: goto Ox9D;
        case 0x9E: goto Ox9E;
        case 0x9F: goto Ox9F;

        case 0xA0: goto OxA0;
        case 0xA1: goto OxA1;
        case 0xA2: goto OxA2;
        case 0xA3: goto OxA3;
        case 0xA4: goto OxA4;
        case 0xA5: goto OxA5;
        case 0xA6: goto OxA6;
        case 0xA7: goto OxA7;
        case 0xA8: goto OxA8;
        case 0xA9: goto OxA9;
        case 0xAA: goto OxAA;
        case 0xAB: goto OxAB;
        case 0xAC: goto OxAC;
        case 0xAD: goto OxAD;
        case 0xAE: goto OxAE;
        case 0xAF: goto OxAF;

        case 0xB0: goto OxB0;
        case 0xB1: goto OxB1;
        case 0xB2: goto OxB2;
        case 0xB3: goto OxB3;
        case 0xB4: goto OxB4;
        case 0xB5: goto OxB5;
        case 0xB6: goto OxB6;
        case 0xB7: goto OxB7;
        case 0xB8: goto OxB8;
        case 0xB9: goto OxB9;
        case 0xBA: goto OxBA;
        case 0xBB: goto OxBB;
        case 0xBC: goto OxBC;
        case 0xBD: goto OxBD;
        case 0xBE: goto OxBE;
        case 0xBF: goto OxBF;

        case 0xC0: goto OxC0;
        case 0xC1: goto OxC1;
        case 0xC2: goto OxC2;
        case 0xC3: goto OxC3;
        case 0xC4: goto OxC4;
        case 0xC5: goto OxC5;
        case 0xC6: goto OxC6;
        case 0xC7: goto OxC7;
        case 0xC8: goto OxC8;
        case 0xC9: goto OxC9;
        case 0xCA: goto OxCA;
        case 0xCB: goto OxCB;
        case 0xCC: goto OxCC;
        case 0xCD: goto OxCD;
        case 0xCE: goto OxCE;
        case 0xCF: goto OxCF;

        case 0xD0: goto OxD0;
        case 0xD1: goto OxD1;
        case 0xD2: goto OxD2;
        case 0xD3: goto OxD3;
        case 0xD4: goto OxD4;
        case 0xD5: goto OxD5;
        case 0xD6: goto OxD6;
        case 0xD7: goto OxD7;
        case 0xD8: goto OxD8;
        case 0xD9: goto OxD9;
        case 0xDA: goto OxDA;
        case 0xDB: goto OxDB;
        case 0xDC: goto OxDC;
        case 0xDD: goto OxDD;
        case 0xDE: goto OxDE;
        case 0xDF: goto OxDF;

        case 0xE0: goto OxE0;
        case 0xE1: goto OxE1;
        case 0xE2: goto OxE2;
        case 0xE3: goto OxE3;
        case 0xE4: goto OxE4;
        case 0xE5: goto OxE5;
        case 0xE6: goto OxE6;
        case 0xE7: goto OxE7;
        case 0xE8: goto OxE8;
        case 0xE9: goto OxE9;
        case 0xEA: goto OxEA;
        case 0xEB: goto OxEB;
        case 0xEC: goto OxEC;
        case 0xED: goto OxED;
        case 0xEE: goto OxEE;
        case 0xEF: goto OxEF;

        case 0xF0: goto OxF0;
        case 0xF1: goto OxF1;
        case 0xF2: goto OxF2;
        case 0xF3: goto OxF3;
        case 0xF4: goto OxF4;
        case 0xF5: goto OxF5;
        case 0xF6: goto OxF6;
        case 0xF7: goto OxF7;
        case 0xF8: goto OxF8;
        case 0xF9: goto OxF9;
        case 0xFA: goto OxFA;
        case 0xFB: goto OxFB;
        case 0xFC: goto OxFC;
        case 0xFD: goto OxFD;
        case 0xFE: goto OxFE;
        case 0xFF: goto OxFF;                
    }
    return;

        // unuseds
        OxFD:
        OxFC:
        OxF4:
        OxED:
        OxEC:
        OxEB:
        OxE4:
        OxE3:
        OxDD:
        OxDB:
        OxD3:
            goto Ox00;


        OxCB:
            //logAndPause("cb extended");
            cpu_do_cb_instruction(s);
            return;

        Ox3E: { // ld A, n
            u8 src = IMM8;
            PC++;
            regA = src;
            return;
        }
        
        // 8-bit loads
        // ld nn, n
        Ox06: // ld B, n
        Ox0E: // ld C, n
        Ox16: // ld D, n
        Ox1E: // ld E, n
        Ox26: // ld H, n
        Ox2E: // ld L, n
        Ox36: { // ld (HL), n
            //logAndPause("ld reg8, n");
            u8* dst = REG8(3);
            u8 src = IMM8;
            PC++;
            if (dst)
                *dst = src;
            else
                mmu_write(s, HL, src);
            return;
        }

        Ox47: // ld B, A
        Ox4F: // ld C, A
        Ox57: // ld D, A
        Ox5F: // ld E, A
        Ox67: // ld H, A        
        Ox6F: // ld L, A        
        Ox77: { // ld (HL), A       
            u8* dst = REG8(3);
            if (dst)
                *dst = regA;
            else
                mmu_write(s, HL, regA);
            return;
        } 

        Ox78: // ld A, B
        Ox79: // ld A, C
        Ox7A: // ld A, D
        Ox7B: // ld A, E
        Ox7C: // ld A, H
        Ox7D: // ld A, L
        Ox7E: { // ld A, (HL)
            u8* src = REG8(0);
            u8 srcval = src ? *src : mem(HL);
            regA = srcval;
            return;
        }

        Ox7F: { // ld A, A        
            regA = regA;   
            return;     
        }        

        // ld r1, r2
        Ox40: // ld B, B
        Ox41: // ld B, C
        Ox42: // ld B, D
        Ox43: // ld B, E
        Ox44: // ld B, H
        Ox45: // ld B, L
        Ox46: // ld B, (HL)
        Ox48: // ld C, B
        Ox49: // ld C, C
        Ox4A: // ld C, D
        Ox4B: // ld C, E
        Ox4C: // ld C, H
        Ox4D: // ld C, L
        Ox4E: // ld C, (HL)
        Ox50: // ld D, B
        Ox51: // ld D, C
        Ox52: // ld D, D
        Ox53: // ld D, E
        Ox54: // ld D, H
        Ox55: // ld D, L
        Ox56: // ld D, (HL)
        Ox58: // ld E, B
        Ox59: // ld E, C
        Ox5A: // ld E, D
        Ox5B: // ld E, E
        Ox5C: // ld E, H
        Ox5D: // ld E, L
        Ox5E: // ld E, (HL)
        Ox60: // ld H, B
        Ox61: // ld H, C
        Ox62: // ld H, D
        Ox63: // ld H, E
        Ox64: // ld H, H
        Ox65: // ld H, L
        Ox66: // ld H, (HL)
        Ox68: // ld L, B
        Ox69: // ld L, C
        Ox6A: // ld L, D
        Ox6B: // ld L, E
        Ox6C: // ld L, H
        Ox6D: // ld L, L
        Ox6E: // ld L, (HL)
        Ox70: // ld (HL), B
        Ox71: // ld (HL), C
        Ox72: // ld (HL), D
        Ox73: // ld (HL), E
        Ox74: // ld (HL), H
        Ox75: { // ld (HL), L
            //logAndPause("ld reg8, reg8");
            u8* src = REG8(0);
            u8* dst = REG8(3);
            u8 srcval = src ? *src : mem(HL);
            if (dst)
                *dst = srcval;
            else
                mmu_write(s, HL, srcval);
            return;
        }

        // LD A, n
        Ox0A: { // ld A, (BC)
            //logAndPause("ld A, (BC)");
            regA = mem(BC);
            return;
        }

        Ox1A: { // ld A, (DE)
            //logAndPause("ld A, (DE)");
            regA = mem(DE);
            return;
        }

        OxFA: { // ld A, (nn)
            //logAndPause("ld A, nn");
            regA = mmu_read(s, IMM16);
            PC += 2;
            return;
        }

        Ox02: { // ld (BC), A
            //logAndPause("ld (BC), A");
            mmu_write(s, BC, regA);
            return;
        }

        Ox12: { // ld (DE), A
            //logAndPause("ld (DE), A");
            mmu_write(s, DE, regA);
            return;
        }

        OxEA: { // ld (nn), A
            //logAndPause("ld nn, A");
            mmu_write(s, IMM16, regA);
            PC += 2;
            return;
        }

        OxF2: { // ld A, (C)
           //logAndPause("ld A, (C)");
           regA = mmu_read(s, 0xff00 + C);
           return;
        }

        OxE2: { // ld (C), A
            //logAndPause("ld (C), A");
            mmu_write(s, 0xff00 + C, regA);
            return;
        }

        // LDD  - decrements HL after load
        Ox3A: { // ldd A, (HL)
            //logAndPause("ldd A, (HL)");
            regA = mmu_read(s, HL);
            HL--;
            return;
        }

        Ox32: { // ldd (HL), A
            //logAndPause("ldd (HL), A");
            mmu_write(s, HL, regA);
            HL--;
            return;
        }

        // LDI - Increments after load
        Ox2A: { // ldi A, (HL)
            //logAndPause("ldi A, (HL)");
            regA = mmu_read(s, HL);
            HL++;
            return;
        }

        Ox22: { // ldi (HL), A
            //logAndPause("ldi (HL), A");
            mmu_write(s, HL, regA);
            HL++;
            return;
        }

        // LDH
        OxE0: { // ldh (n), A
            //logAndPause("ldh (n), A");
            mmu_write(s, 0xff00 + IMM8, regA);
            PC++;
            return;
        }

        OxF0: { // ldh A, (n)
            //logAndPause("ldh A, (n)");
            regA = mmu_read(s, 0xff00 + IMM8);
            PC++;
            return;
        }

        // 16 Bit Loads
        Ox01: // ld BC, nn
        Ox11: // ld DE, nn
        Ox21: // ld HL, nn
        Ox31: { // ld SP, nn
            //logAndPause("ld reg16, nn");
            u16 *dst = REG16(4);
            *dst = IMM16;
            PC += 2;
            return;
        }


        OxF9: { // ld SP, HL
            //logAndPause ("ld SP, HL");
            s->sp = HL;
            return;
        }

        OxF8: { // ldhl SP, n
            // ld HL, SP + n
            //logAndPause ("ldhl SP, n");
            u32 res = (u32)s->sp + (s8)IMM8;
            ZF = 0;
            NF = 0;
            HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
            CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
            HL = (u16)res;
            PC++;
            return;
        }

        Ox08: { // ld (nn), SP
            // push 16 bits on to stack and sp -= 2
            //logAndPause("ld (nn), SP");
            mmu_write16(s, IMM16, s->sp);
            PC += 2;
            return;
        }

        OxF5: {// push AF
            u16 src = (regA << 8) | F;
            mmu_push16(s, src);
            return;
        }
        OxC5: // push BC
        OxD5: // push DE
        OxE5: { // push HL
            //logAndPause("push reg16");
            u16 *src = REG16S(4);
            mmu_push16(s,*src);
            return;
        }

        // pop 16 bits from stack and sp += 2
        OxF1: { // pop AF
            u16 dst = mmu_pop16(s);
            regA = dst >> 8;
            F = dst & 0x00f0;
            return;
        }

        OxC1: // pop BC
        OxD1: // pop DE
        OxE1: { // pop HL
            //logAndPause("pop reg16");
            u16 *dst = REG16S(4);
            *dst = mmu_pop16(s);
            F = F & 0xf0;
            return;
        }

        // 8 bit maths
        // Addition
        Ox87: { // add A, A
            u16 res = regA + regA;
            ZF = (u8)res == 0;
            NF = 0;
            HF = (regA ^ regA ^ res) & 0x10 ? 1 : 0;
            CF = res & 0x100 ? 1 : 0;
            regA = (u8)res;
            return;
        }
        Ox80: // add A, B
        Ox81: // add A, C
        Ox82: // add A, D
        Ox83: // add A, E
        Ox84: // add A, H
        Ox85: // add A, L
        Ox86: { // add A, (HL)
            //logAndPause("add A, reg8");
            u8* src = REG8(0);
            u8 srcval = src ? *src : mem(HL);
            u16 res = regA + srcval;
            ZF = (u8)res == 0;
            NF = 0;
            HF = (regA ^ srcval ^ res) & 0x10 ? 1 : 0;
            CF = res & 0x100 ? 1 : 0;
            regA = (u8)res;
            return;
        }

        OxC6: { // add A, n
            //logAndPause("add A, n");
            u16 res = regA + IMM8;
            ZF = (u8)res == 0;
            NF = 0;
            HF = (regA ^ IMM8 ^ res) & 0x10 ? 1 : 0;
            CF = res & 0x100 ? 1 : 0;
            regA = (u8)res;
            PC++;
            return;
        }

        // ADC - add n + carry flag
        Ox8F: {// adc A, A
            u16 res = regA + regA + CF;
            ZF = (u8)res == 0;
            NF = 0;
            HF = (regA ^ regA ^ res) & 0x10 ? 1 : 0;
            CF = res & 0x100 ? 1 : 0;
            regA = (u8)res;
            return;
        }
        Ox88: // adc A, B
        Ox89: // adc A, C
        Ox8A: // adc A, D
        Ox8B: // adc A, E
        Ox8C: // adc A, H
        Ox8D: // adc A, L
        Ox8E: { // adc A, (HL)
            //logAndPause("adc A, reg8");
            u8* src = REG8(0);
            u8 srcval = src ? *src : mem(HL);
            u16 res = regA + srcval + CF;
            ZF = (u8)res == 0;
            NF = 0;
            HF = (regA ^ srcval ^ res) & 0x10 ? 1 : 0;
            CF = res & 0x100 ? 1 : 0;
            regA = (u8)res;
            return;
        }

        OxCE: { // adc A, n
            //logAndPause("adc A, n");
            u16 res = regA + IMM8 + CF;
            ZF = (u8)res == 0;
            NF = 0;
            HF = (regA ^ IMM8 ^ res) & 0x10 ? 1 : 0;
            CF = res & 0x100 ? 1 : 0;
            regA = (u8)res;
            PC++;
            return;
        }

        // Subtraction - Always subtracts from A.  Not sure if A is required parameter
        Ox97: {// sub A, A
            ZF = 1;
            NF = 1;
            HF = 0;
            CF = 0;
            regA = 0;
            return;
        }
        Ox90: // sub A, B
        Ox91: // sub A, C
        Ox92: // sub A, D
        Ox93: // sub A, E
        Ox94: // sub A, H
        Ox95: // sub A, L
        Ox96: { // sub A, (HL)
            //logAndPause("sub reg8");
            u8 *reg = REG8(0);
            u8 val = reg ? *reg : mem(HL);
            u8 res = regA - val;
            ZF = res == 0;
            NF = 1;
            HF = ((s32)regA & 0xf) - (val & 0xf) < 0;
            CF = regA < val;
            regA = res;
            return;
        }

        OxD6: { // sub A, n
            //logAndPause("sub n");
            u8 res = regA - IMM8;
            ZF = res == 0;
            NF = 1;
            HF = ((s32)A & 0xf) - (IMM8 & 0xf) < 0;
            CF = regA < IMM8;
            regA = res;
            PC++;
            return;
        }

        // Subtract + carry flag
        Ox9F: { // sbc A, A
            u8 res = 0 - CF;
            ZF = res == 0;
            NF = 1;
            HF = ((s32)regA & 0xf) - (regA & 0xf) - CF < 0;
            CF = regA < regA + CF;
            regA = res;
            return;
        }
        Ox98: // sbc A, B
        Ox99: // sbc A, C
        Ox9A: // sbc A, D
        Ox9B: // sbc A, E
        Ox9C: // sbc A, H
        Ox9D: // sbc A, L
        Ox9E: { // sbc A, (HL)
            //logAndPause("sbc A, reg8");
            u8 *reg = REG8(0);
            u8 regval = reg ? *reg : mem(HL);
            u8 res = regA - regval - CF;
            ZF = res == 0;
            NF = 1;
            HF = ((s32)regA & 0xf) - (regval & 0xf) - CF < 0;
            CF = regA < regval + CF;
            regA = res;
            return;
        }

        OxDE: { // sbc A, n
            //logAndPause("sbc A, n");
            u8 res = regA - IMM8 - CF;
            ZF = res == 0;
            NF = 1;
            HF = ((s32)regA & 0xf) - (IMM8 & 0xf) - CF < 0;
            CF = regA < IMM8 + CF;
            regA = res;
            PC++;
            return;
        }
        // AND n with A result in A
        OxA7: { // and A
            ZF = regA == 0;
            NF = 0;
            HF = 1;
            CF = 0;
            return;        
        }
        OxA0: // and B
        OxA1: // and C
        OxA2: // and D
        OxA3: // and E
        OxA4: // and H
        OxA5: // and L
        OxA6: { // and (HL)
            //logAndPause("and reg8");
            u8 *reg = REG8(0);
            u8 val = reg ? *reg : mem(HL);
            regA = regA & val;
            ZF = regA == 0;
            NF = 0;
            HF = 1;
            CF = 0;
            return;
        }

        OxE6: { // and n
            //logAndPause("and n");
            regA = regA & IMM8;
            PC++;
            ZF = regA == 0;
            NF = 0;
            HF = 1;
            CF = 0;
            return;
        }

        // OR n with A result in A
        OxB7: { // or A
            F = regA ? 0 : FLAG_Z;
            return;
        }
        OxB0: // or B
        OxB1: // or C
        OxB2: // or D
        OxB3: // or E
        OxB4: // or H
        OxB5: // or L
        OxB6: { // or (HL)
            //logAndPause("or reg8");
            u8* src = REG8(0);
            u8 srcval = src ? *src : mem(HL);
            regA |= srcval;
            F = regA ? 0 : FLAG_Z;
            return;
        }

        OxF6: { // or n
            //logAndPause ("or n");
            regA |= IMM8;
            F = regA ? 0 : FLAG_Z;
            PC++;
            return;
        }

        // XOR n with A result in A
        OxAF: { // xor A
            regA = 0;
            F = FLAG_Z;
            return;
        }
        OxA8: // xor B
        OxA9: // xor C
        OxAA: // xor D
        OxAB: // xor E
        OxAC: // xor H
        OxAD: // xor L
        OxAE: { // xor (HL)
            //logAndPause("xor reg8");
            u8* src = REG8(0);
            u8 srcval = src ? *src : mem(HL);
            regA ^= srcval;
            F = regA ? 0 : FLAG_Z;
            return;
        }

        OxEE: { // xor n
            //logAndPause("xor n");
            regA ^= IMM8;
            PC++;
            F = regA ? 0 : FLAG_Z;
            return;
        }

        // Compare contents of paramter with A, Z flag set if results equal
        OxBF: {// cp A
            ZF = 1;
            NF = 1;
            HF = 0;
            CF = 0;
            return;
        }
        OxB8: // cp B
        OxB9: // cp C
        OxBA: // cp D
        OxBB: // cp E
        OxBC: // cp H
        OxBD: // cp L
        OxBE: { // cp (HL)
            //logAndPause("cp reg8");
            u8 *reg = REG8(0);
            u8 regval = reg ? *reg : mem(HL);

            ZF = regA == regval;
            NF = 1;
            HF = (regA & 0xf) < (regval & 0xf);
            CF = regA < regval;
            return;
        }

        OxFE: { // cp n
            //logAndPause("cp n");
            u8 n = IMM8;

            ZF = regA == n;
            NF = 1;
            HF = (regA & 0xf) < (n & 0xf);
            CF = regA < n;
            PC++;
            return;
        }

        // Increment value of parameter
        Ox3C: { // inc A
            u8 res = regA + 1;
            ZF = res == 0;
            NF = 0;
            HF = (regA & 0xf) == 0xf;
            regA = res;
            return;
        }
        Ox04: // inc B
        Ox0C: // inc C
        Ox14: // inc D
        Ox1C: // inc E
        Ox24: // inc H
        Ox2C: // inc L
        Ox34: { // inc (HL)
            //logAndPause("inc reg8");
            u8* reg = REG8(3);
            u8 val = reg ? *reg : mem(HL);
            u8 res = val + 1;
            ZF = res == 0;
            NF = 0;
            HF = (val & 0xf) == 0xf;
            if (reg)
                *reg = res;
            else
                mmu_write(s, HL, res);
            return;
        }

        // Decrement value of parameter
        Ox3D: { // dec A
            regA--;
            NF = 1;
            ZF = regA == 0;
            HF = (regA & 0x0F) == 0x0F;
            return;            
        }
        Ox05: // dec B
        Ox0D: // dec C
        Ox15: // dec D
        Ox1D: // dec E
        Ox25: // dec H
        Ox2D: // dec L
        Ox35: { // dec (HL)
            //logAndPause("dec reg8");
            u8* reg = REG8(3);
            u8 val = reg ? *reg : mem(HL);
            val--;
            NF = 1;
            ZF = val == 0;
            HF = (val & 0x0F) == 0x0F;
            if (reg)
                *reg = val;
            else
                mmu_write(s, HL, val);
            return;
        }

        // 16 bit maths
        Ox09: // add HL, BC
        Ox19: // add HL, DE
        Ox29: // add HL, HL
        Ox39: { // add HL, SP
            //logAndPause("add HL, reg16");
            u16 *src = REG16(4);
            u32 tmp = HL + *src;
            NF = 0;
            HF = (((HL & 0xfff) + (*src & 0xfff)) & 0x1000) ? 1 : 0;
            CF = tmp > 0xffff;
            HL = tmp;
            return;
        }

        OxE8: { // add SP, n
            //logAndPause("add SP, n");
            s8 off = (s8)IMM8;
            u32 res = s->sp + off;
            ZF = 0;
            NF = 0;
            HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
            CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
            s->sp = res;
            PC++;
            return;
        }

        Ox03: // inc BC
        Ox13: // inc DE
        Ox23: // inc HL
        Ox33: { // inc SP
            //logAndPause("inc reg16");
            u16 *reg = REG16(4);
            *reg += 1;
            return;
        }

        Ox0B: // dec BC
        Ox1B: // dec DE
        Ox2B: // dec HL
        Ox3B: { // dec SP
            //logAndPause("dec reg16");
            u16 *reg = REG16(4);
            *reg -= 1;
            return;
        }

        Ox27: { // daa
            // Transformations
            // "DAA - This instruction adjusts register A so that the
            // correct representation of Binary Coded Decimal (BCD)
            // is obtained."

            // koenk adds:
            // When adding/subtracting two numbers in BCD form, this instructions
            // brings the results back to BCD form too. In BCD form the decimals 0-9
            // are encoded in a fixed number of bits (4). E.g., Ox93 actually means
            // 93 decimal. Adding/subtracting such numbers takes them out of this
            // form since they can results in values where each digit is >9.
            // E.g., Ox9 + Ox1 = OxA, but should be Ox10. The important thing to
            // note here is that per 4 bits we 'skip' 6 values (OxA-OxF), and thus
            // by adding Ox6 we get: OxA + Ox6 = Ox10, the correct answer. The same
            // works for the upper byte (add Ox60).
            // So: If the lower byte is >9, we need to add Ox6.
            // If the upper byte is >9, we need to add Ox60.
            // Furthermore, if we carried the lower part (HF, Ox9+Ox9=Ox12) we
            // should also add Ox6 (Ox12+Ox6=Ox18).
            // Similarly for the upper byte (CF, Ox90+Ox90=Ox120, +Ox60=Ox180).

            // For subtractions (we know it was a subtraction by looking at the NF
            // flag) we simiarly need to *subtract* Ox06/Ox60/Ox66 to again skip the
            // unused 6 values in each byte. The GB does this by only looking at the
            // NF and CF flags then.
            //logAndPause("daa");
            s8 add = 0;
            if ((!NF && (regA & 0xf) > 0x9) || HF)
                add |= 0x6;
            if ((!NF && regA > 0x99) || CF) {
                add |= 0x60;
                CF = 1;
            }
            regA += NF ? -add : add;
            ZF = regA == 0;
            HF = 0;
            return;

        }
        Ox2F: { // cpl
            // flip all bits in A
            //logAndPause("cpl");
            regA = ~regA;
            NF = 1;
            HF = 1;
            return;
        }
        Ox3F: { // ccf
            //flip carry flag
            //logAndPause("ccf");
            CF = CF ? 0 : 1;
            NF = 0;
            HF = 0;
            return;
        }
        Ox37: { // scf
            // set carry flag
            //logAndPause("scf");
            NF = 0;
            HF = 0;
            CF = 1;
            return;
        }
        Ox07: { // rlca - rotate a left - shift bit 7 to carry.
            //logAndPause("rlca");
            u8 res = (regA << 1) | (regA >> 7);
            F = (regA >> 7) ? FLAG_C : 0;
            regA = res;
            return;
        }
        Ox17: { // rla
            // rotate a left - don't know how this differs with rlca
            //logAndPause("rla");
            u8 res = regA << 1 | (CF ? 1 : 0);
            F = (regA & (1 << 7)) ? FLAG_C : 0;
            regA = res;
            return;
        }
        Ox0F: { // rrca
            // rotate a right - shift bit 0 to carry
            //logAndPause("rrca");
            F = (regA & 1) ? FLAG_C : 0;
            regA = (regA >> 1) | ((regA & 1) << 7);
            return;
        }
        Ox1F: { // rra
            // rotate a right - don't know how this differs with rrca
            //logAndPause("rra");
            u8 res = (regA >> 1) | (CF << 7);
            ZF = 0;
            NF = 0;
            HF = 0;
            CF = regA & 0x1;
            regA = res;
            return;
        }

        // Control
        Ox00: { // noop
            //logAndPause("noop");
            return;
        }
        Ox76: { // halt
            // halt - do nothing until an interrupt occurs
            // implementation of interrupts / HALT op could be related to poor performance.
            //logAndPause("halt");
            s->halt_for_interrupts = 1;
            return;
        }
        Ox10: // stop
            //logAndPause("stop");
            // TODO
            return;

        OxF3: { // di
            // Disable interrupts
            //logAndPause("di");
            s->interrupts_master_enabled = 0;
            return;
        }
        OxFB: { // ei
            // Enable interrupts
            //logAndPause("ei");
            s->interrupts_master_enabled = 1;
            return;
        }
        OxC3: { // jp nn
            // jump to nn
            //logAndPause("jp nn");
            PC = IMM16;
            return;
        }
        OxC2: // jp NZ, nn - jump if !Z
        OxCA: // jp Z, nn - jump if Z
        OxD2: // jp NC, nn - jump if !C
        OxDA: { // jp C, nn - jump if C
            //logAndPause("jp cc, nn");
            u8 flag = (op >> 3) & 3;
            if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
                PC = IMM16;
            else
                PC += 2;
            return;
        }

        OxE9: {// jp (HL)
            //logAndPause("jp, (HL)");
            PC = HL;
            return;
        }
        Ox18: { // jr n
            // jump n addresses forward
            //logAndPause("jr, n");
            PC += (s8)IMM8 + 1;
            return;
        }
        Ox20: // jr NZ, n
        Ox28: // jr Z, n
        Ox30: // jr NC, n
        Ox38: { // jr C, n
            //logAndPause("jr cc, n");
            u8 flag = (op >> 3) & 3;

            if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1)) {
                PC += (s8)IMM8;
            }
            PC++;
            return;
        }

        // Push address of next instruction onto stack and then jump to address nn.
        OxCD: { // call nn
            //logAndPause("call nn");
            u16 dst = IMM16;
            mmu_push16(s, PC + 2);
            PC = dst;
            return;
        }

        OxC4: // call NZ, nn
        OxCC: // call Z, nn
        OxD4: // call NC, cc
        OxDC: { // call C, nn
            //logAndPause("call cc, nn");
            u16 dst = IMM16;
            PC += 2;
            u8 flag = (op >> 3) & 3;
            if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1)) {
                mmu_push16(s, PC);
                PC = dst;
            }
            return;
        }

        // push current address to stack then jump to 0 + n
        OxC7: // rst $00
        OxCF: // rst $08
        OxD7: // rst $10
        OxDF: // rst $18
        OxE7: // rst $20
        OxEF: // rst $28
        OxF7: // rst $30
        OxFF: { // rst $38
            //logAndPause("rst n");
            mmu_push16(s, PC);
            PC = ((op >> 3) & 7) * 8;
            return;
        }

        // Return -- pop two bytes of stack and jump to that address
        OxC9: { // ret
            //logAndPause("ret");
            PC = mmu_pop16(s);
            return;
        }
        OxC0: // ret NZ
        OxC8: // ret Z
        OxD0: // ret NC
        OxD8: { // ret C
            // TODO cyclecount depends on taken or not
            //logAndPause("ret cc");
            u8 flag = (op >> 3) & 3;
            if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
                PC = mmu_pop16(s);
            return;
        }

        OxD9: { // reti
            //logAndPause("reti");
            // return then enable interrupts.
            PC = mmu_pop16(s);
            s->interrupts_master_enabled = 1;
            return;
        }    
}

void cpu_step(struct gb_state *s) {
    u8 op;

    s->emu_state->last_op_cycles = 0;

    cpu_handle_interrupts(s);
   
    op = getOpCodeFromROM(s, PC);
    
    s->emu_state->last_op_cycles = cycles_per_instruction[op];
    if (op == 0xcb) {
        op = getOpCodeFromROM(s, PC + 1);
        s->emu_state->last_op_cycles = cycles_per_instruction_cb[op];
    }

    if (!s->halt_for_interrupts) {
        if (true) {
            cpu_do_instruction(s);
        }
    } else {
        if (!s->interrupts_enable)
            cpu_error("Waiting for interrupts while disabled, deadlock.\n");
    }
}

