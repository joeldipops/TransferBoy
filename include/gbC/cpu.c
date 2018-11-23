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

#include "cpu.h"
#include "mmu.h"
#include "hwdefs.h"
#include "../../logger.h"

#define cpu_error(fmt, ...) \
    do { \
        logInfo("CPU Error: " fmt "\n", ##__VA_ARGS__); \
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
void cpu_reset_state(struct gb_state *s) {
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

static void cpu_handle_interrupts(struct gb_state *s) {
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
#define A s->reg8.A
#define F s->reg8.F
#define B s->reg8.B
#define C s->reg8.C
#define D s->reg8.D
#define E s->reg8.E
#define H s->reg8.H
#define L s->reg8.L
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

static void cpu_do_cb_instruction(struct gb_state *s) {
    u8 op = mmu_read(s, s->pc++);

    if (M(op, 0x00, 0xf8)) { /* RLC reg8 */

        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val << 1) | (val >> 7);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val >> 7;
        if (reg) *reg = res; else mmu_write(s, HL, res);
    } else if (M(op, 0x08, 0xf8)) { /* RRC reg8 */


        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val >> 1) | ((val & 1) << 7);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val & 1;
        if (reg) *reg = res; else mmu_write(s, HL, res);
    } else if (M(op, 0x10, 0xf8)) { /* RL reg8 */

        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val << 1) | (CF ? 1 : 0);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val >> 7;
        if (reg) *reg = res; else mmu_write(s, HL, res);
    } else if (M(op, 0x18, 0xf8)) { /* RR reg8 */

        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val >> 1) | (CF << 7);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val & 0x1;
        if (reg) *reg = res; else mmu_write(s, HL, res);
    } else if (M(op, 0x20, 0xf8)) { /* SLA reg8 */

        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        CF = val >> 7;
        val = val << 1;
        ZF = val == 0;
        NF = 0;
        HF = 0;
        if (reg) *reg = val; else mmu_write(s, HL, val);
    } else if (M(op, 0x28, 0xf8)) { /* SRA reg8 */

        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        CF = val & 0x1;
        val = (val >> 1) | (val & (1<<7));
        ZF = val == 0;
        NF = 0;
        HF = 0;
        if (reg) *reg = val; else mmu_write(s, HL, val);
    } else if (M(op, 0x30, 0xf8)) { /* SWAP reg8 */

        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = ((val << 4) & 0xf0) | ((val >> 4) & 0xf);
        F = res == 0 ? FLAG_Z : 0;
        if (reg) *reg = res; else mmu_write(s, HL, res);
    } else if (M(op, 0x38, 0xf8)) { /* SRL reg8 */

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

static void cpu_do_instruction(struct gb_state *s) {
    u8 op = mmu_read(s, s->pc++);

    if (M(op, 0x00, 0xff)) { /* NOP */
        logAndPause("noop");
    } else if (M(op, 0x01, 0xcf)) { /* LD reg16, u16 */
        logAndPause("ld reg16, nn");
        u16 *dst = REG16(4);
        *dst = IMM16;
        s->pc += 2;
    } else if (M(op, 0x02, 0xff)) { /* LD (BC), A */
        logAndPause("ld (BC), A");
        mmu_write(s, BC, A);
    } else if (M(op, 0x03, 0xcf)) { /* INC reg16 */
        logAndPause("inc reg16");
        u16 *reg = REG16(4);
        *reg += 1;
    } else if (M(op, 0x04, 0xc7)) { /* INC reg8 */
        logAndPause("inc reg8");
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
    } else if (M(op, 0x05, 0xc7)) { /* DEC reg8 */
        logAndPause("dec reg8");
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
    } else if (M(op, 0x06, 0xc7)) { /* LD reg8, imm8 */
        logAndPause("ld reg8, n");
        u8* dst = REG8(3);
        u8 src = IMM8;
        s->pc++;
        if (dst)
            *dst = src;
        else
            mmu_write(s, HL, src);
    } else if (M(op, 0x07, 0xff)) { /* RLCA */
        logAndPause("rlca");
        u8 res = (A << 1) | (A >> 7);
        F = (A >> 7) ? FLAG_C : 0;
        A = res;

    } else if (M(op, 0x08, 0xff)) { /* LD (imm16), SP */
        logAndPause("ld (nn) SP");
        mmu_write16(s, IMM16, s->sp);
        s->pc += 2;

    } else if (M(op, 0x09, 0xcf)) { /* ADD HL, reg16 */
        logAndPause("add HL reg16");
        u16 *src = REG16(4);
        u32 tmp = HL + *src;
        NF = 0;
        HF = (((HL & 0xfff) + (*src & 0xfff)) & 0x1000) ? 1 : 0;
        CF = tmp > 0xffff;
        HL = tmp;
    } else if (M(op, 0x0a, 0xff)) { /* LD A, (BC) */
        logAndPause("ld A, (BC)");
        A = mem(BC);
    } else if (M(op, 0x0b, 0xcf)) { /* DEC reg16 */
        logAndPause("dec reg16");
        u16 *reg = REG16(4);
        *reg -= 1;
    } else if (M(op, 0x0f, 0xff)) { /* RRCA */
        logAndPause("rrca");
        F = (A & 1) ? FLAG_C : 0;
        A = (A >> 1) | ((A & 1) << 7);
    } else if (M(op, 0x10, 0xff)) { /* STOP */
        logAndPause("stop");
        //s->halt_for_interrupts = 1;
    } else if (M(op, 0x12, 0xff)) { /* LD (DE), A */
        logAndPause("ld (DE), A");
        mmu_write(s, DE, A);
    } else if (M(op, 0x17, 0xff)) { /* RLA */
        logAndPause("rla");
        u8 res = A << 1 | (CF ? 1 : 0);
        F = (A & (1 << 7)) ? FLAG_C : 0;
        A = res;
    } else if (M(op, 0x18, 0xff)) { /* JR off8 */
        logAndPause("jr n");
        s->pc += (s8)IMM8 + 1;
    } else if (M(op, 0x1a, 0xff)) { /* LD A, (DE) */
        logAndPause("ld A, (DE)");
        A = mem(DE);
    } else if (M(op, 0x1f, 0xff)) { /* RRA */
        logAndPause("rra");
        u8 res = (A >> 1) | (CF << 7);
        ZF = 0;
        NF = 0;
        HF = 0;
        CF = A & 0x1;
        A = res;
    } else if (M(op, 0x20, 0xe7)) { /* JR cond, off8 */
        logAndPause("jr cc, n");
        u8 flag = (op >> 3) & 3;

        if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1)) {
            s->pc += (s8)IMM8;
        }
        s->pc++;
    } else if (M(op, 0x22, 0xff)) { /* LDI (HL), A */
        logAndPause("ldi (HL), A");
        mmu_write(s, HL, A);
        HL++;
    } else if (M(op, 0x27, 0xff)) { /* DAA */
        logAndPause("daa");
        /* When adding/subtracting two numbers in BCD form, this instructions
         * brings the results back to BCD form too. In BCD form the decimals 0-9
         * are encoded in a fixed number of bits (4). E.g., 0x93 actually means
         * 93 decimal. Adding/subtracting such numbers takes them out of this
         * form since they can results in values where each digit is >9.
         * E.g., 0x9 + 0x1 = 0xA, but should be 0x10. The important thing to
         * note here is that per 4 bits we 'skip' 6 values (0xA-0xF), and thus
         * by adding 0x6 we get: 0xA + 0x6 = 0x10, the correct answer. The same
         * works for the upper byte (add 0x60).
         * So: If the lower byte is >9, we need to add 0x6.
         * If the upper byte is >9, we need to add 0x60.
         * Furthermore, if we carried the lower part (HF, 0x9+0x9=0x12) we
         * should also add 0x6 (0x12+0x6=0x18).
         * Similarly for the upper byte (CF, 0x90+0x90=0x120, +0x60=0x180).
         *
         * For subtractions (we know it was a subtraction by looking at the NF
         * flag) we simiarly need to *subtract* 0x06/0x60/0x66 to again skip the
         * unused 6 values in each byte. The GB does this by only looking at the
         * NF and CF flags then.
         */
        s8 add = 0;
        if ((!NF && (A & 0xf) > 0x9) || HF)
            add |= 0x6;
        if ((!NF && A > 0x99) || CF) {
            add |= 0x60;
            CF = 1;
        }
        A += NF ? -add : add;
        ZF = A == 0;
        HF = 0;
    } else if (M(op, 0x2a, 0xff)) { /* LDI A, (HL) */
        logAndPause("ldi A, (HL)");
        A = mmu_read(s, HL);
        HL++;
    } else if (M(op, 0x2f, 0xff)) { /* CPL */
        logAndPause("cpl");
        A = ~A;
        NF = 1;
        HF = 1;
    } else if (M(op, 0x32, 0xff)) { /* LDD (HL), A */
        logAndPause("ldd (HL), A");
        mmu_write(s, HL, A);
        HL--;
    } else if (M(op, 0x37, 0xff)) { /* SCF */
        logAndPause("scf");
        NF = 0;
        HF = 0;
        CF = 1;
    } else if (M(op, 0x3a, 0xff)) { /* LDD A, (HL) */
        logAndPause("ldd A, (HL)");
        A = mmu_read(s, HL);
        HL--;
    } else if (M(op, 0x3f, 0xff)) { /* CCF */
        logAndPause("ccf");
        CF = CF ? 0 : 1;
        NF = 0;
        HF = 0;
    } else if (M(op, 0x76, 0xff)) { /* HALT */
        logAndPause("halt");
        s->halt_for_interrupts = 1;
    } else if (M(op, 0x40, 0xc0)) { /* LD reg8, reg8 */
        logAndPause("ld reg8, reg8");
        u8* src = REG8(0);
        u8* dst = REG8(3);
        u8 srcval = src ? *src : mem(HL);
        if (dst)
            *dst = srcval;
        else
            mmu_write(s, HL, srcval);
    } else if (M(op, 0x80, 0xf8)) { /* ADD A, reg8 */
        logAndPause("add A, reg8");
        u8* src = REG8(0);
        u8 srcval = src ? *src : mem(HL);
        u16 res = A + srcval;
        ZF = (u8)res == 0;
        NF = 0;
        HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
        CF = res & 0x100 ? 1 : 0;
        A = (u8)res;
    } else if (M(op, 0x88, 0xf8)) { /* ADC A, reg8 */
        logAndPause("adc A, reg8");
        u8* src = REG8(0);
        u8 srcval = src ? *src : mem(HL);
        u16 res = A + srcval + CF;
        ZF = (u8)res == 0;
        NF = 0;
        HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
        CF = res & 0x100 ? 1 : 0;
        A = (u8)res;

    } else if (M(op, 0x90, 0xf8)) { /* SUB reg8 */
        logAndPause("sub reg8");
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = A - val;
        ZF = res == 0;
        NF = 1;
        HF = ((s32)A & 0xf) - (val & 0xf) < 0;
        CF = A < val;
        A = res;
    } else if (M(op, 0x98, 0xf8)) { /* SBC A, reg8 */
        logAndPause("sbc A, reg8");
        u8 *reg = REG8(0);
        u8 regval = reg ? *reg : mem(HL);
        u8 res = A - regval - CF;
        ZF = res == 0;
        NF = 1;
        HF = ((s32)A & 0xf) - (regval & 0xf) - CF < 0;
        CF = A < regval + CF;
        A = res;
    } else if (M(op, 0xa0, 0xf8)) { /* AND reg8 */
        logAndPause("and reg8");
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        A = A & val;
        ZF = A == 0;
        NF = 0;
        HF = 1;
        CF = 0;
    } else if (M(op, 0xa8, 0xf8)) { /* XOR reg8 */
        logAndPause("xor reg8");
        u8* src = REG8(0);
        u8 srcval = src ? *src : mem(HL);
        A ^= srcval;
        F = A ? 0 : FLAG_Z;

    } else if (M(op, 0xb0, 0xf8)) { /* OR reg8 */
        logAndPause("or reg8");
        u8* src = REG8(0);
        u8 srcval = src ? *src : mem(HL);
        A |= srcval;
        F = A ? 0 : FLAG_Z;
    } else if (M(op, 0xb8, 0xf8)) { /* CP reg8 */
        logAndPause("cp reg8");
        u8 *reg = REG8(0);
        u8 regval = reg ? *reg : mem(HL);

        ZF = A == regval;
        NF = 1;
        HF = (A & 0xf) < (regval & 0xf);
        CF = A < regval;
    } else if (M(op, 0xc0, 0xe7)) { /* RET cond */
        logAndPause("ret cc");
        /* TODO cyclecount depends on taken or not */

        u8 flag = (op >> 3) & 3;
        if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
            s->pc = mmu_pop16(s);

    } else if (M(op, 0xc1, 0xcf)) { /* POP reg16 */
        logAndPause("pop reg16");
        u16 *dst = REG16S(4);
        *dst = mmu_pop16(s);
        F = F & 0xf0;
    } else if (M(op, 0xc2, 0xe7)) { /* JP cond, imm16 */
        logAndPause("jp cc, nn");
        u8 flag = (op >> 3) & 3;
        if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
            s->pc = IMM16;
        else
            s->pc += 2;
    } else if (M(op, 0xc3, 0xff)) { /* JP imm16 */
        logAndPause("jp nn");
        s->pc = IMM16;
    } else if (M(op, 0xc4, 0xe7)) { /* CALL cond, imm16 */
        logAndPause("call cc, nn");
        u16 dst = IMM16;
        s->pc += 2;
        u8 flag = (op >> 3) & 3;
        if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1)) {
            mmu_push16(s, s->pc);
            s->pc = dst;
        }
    } else if (M(op, 0xc5, 0xcf)) { /* PUSH reg16 */
        logAndPause("push reg16");
        u16 *src = REG16S(4);
        mmu_push16(s,*src);
    } else if (M(op, 0xc6, 0xff)) { /* ADD A, imm8 */
        logAndPause("add A, n");
        u16 res = A + IMM8;
        ZF = (u8)res == 0;
        NF = 0;
        HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
        CF = res & 0x100 ? 1 : 0;
        A = (u8)res;
        s->pc++;
    } else if (M(op, 0xc7, 0xc7)) { /* RST imm8 */
        logAndPause("rst n");
        mmu_push16(s, s->pc);
        s->pc = ((op >> 3) & 7) * 8;
    } else if (M(op, 0xc9, 0xff)) { /* RET */
        logAndPause("ret");
        s->pc = mmu_pop16(s);
    } else if (M(op, 0xcd, 0xff)) { /* CALL imm16 */
        logAndPause("call nn");
        u16 dst = IMM16;
        mmu_push16(s, s->pc + 2);
        s->pc = dst;
    } else if (M(op, 0xce, 0xff)) { /* ADC imm8 */
        logAndPause("adc n");
        u16 res = A + IMM8 + CF;
        ZF = (u8)res == 0;
        NF = 0;
        HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
        CF = res & 0x100 ? 1 : 0;
        A = (u8)res;
        s->pc++;
    } else if (M(op, 0xd6, 0xff)) { /* SUB imm8 */
        logAndPause("sub n");
        u8 res = A - IMM8;
        ZF = res == 0;
        NF = 1;
        HF = ((s32)A & 0xf) - (IMM8 & 0xf) < 0;
        CF = A < IMM8;
        A = res;
        s->pc++;
    } else if (M(op, 0xd9, 0xff)) { /* RETI */
        logAndPause("reti");
        s->pc = mmu_pop16(s);
        s->interrupts_master_enabled = 1;
    } else if (M(op, 0xde, 0xff)) { /* SBC imm8 */
        logAndPause("sbc n");
        u8 res = A - IMM8 - CF;
        ZF = res == 0;
        NF = 1;
        HF = ((s32)A & 0xf) - (IMM8 & 0xf) - CF < 0;
        CF = A < IMM8 + CF;
        A = res;
        s->pc++;
    } else if (M(op, 0xe0, 0xff)) { /* LD (0xff00 + imm8), A */
        logAndPause("ldh (n), A");
        mmu_write(s, 0xff00 + IMM8, A);
        s->pc++;
    } else if (M(op, 0xe2, 0xff)) { /* LD (0xff00 + C), A */
        logAndPause("ld (C), A");
        mmu_write(s, 0xff00 + C, A);
    } else if (M(op, 0xe6, 0xff)) { /* AND imm8 */
        logAndPause("and n");
        A = A & IMM8;
        s->pc++;
        ZF = A == 0;
        NF = 0;
        HF = 1;
        CF = 0;
    } else if (M(op, 0xe8, 0xff)) { /* ADD SP, imm8s */
        logAndPause("add SP, n");
        s8 off = (s8)IMM8;
        u32 res = s->sp + off;
        ZF = 0;
        NF = 0;
        HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
        CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
        s->sp = res;
        s->pc++;
    } else if (M(op, 0xe9, 0xff)) { /* LD PC, HL (or JP (HL) ) */
        logAndPause("ld pc, (HL)");
        s->pc = HL;
    } else if (M(op, 0xea, 0xff)) { /* LD (imm16), A */
        logAndPause("ld (nn), A");
        mmu_write(s, IMM16, A);
        s->pc += 2;
    } else if (M(op, 0xcb, 0xff)) { /* CB-prefixed extended instructions */
        logAndPause("cb extended");
        return cpu_do_cb_instruction(s);
    } else if (M(op, 0xee, 0xff)) { /* XOR imm8 */
        logAndPause("xor n");
        A ^= IMM8;
        s->pc++;
        F = A ? 0 : FLAG_Z;
    } else if (M(op, 0xf0, 0xff)) { /* LD A, (0xff00 + C) */
        logAndPause("ld A, (n)");
        A = mmu_read(s, 0xff00 + IMM8);
        s->pc++;
    } else if (M(op, 0xf2, 0xff)) { /* LD A, (0xff00 + C) */
        logAndPause("ld A, (C)");
        A = mmu_read(s, 0xff00 + C);
    } else if (M(op, 0xf3, 0xff)) { /* DI */
        logAndPause("di");
        s->interrupts_master_enabled = 0;
    } else if (M(op, 0xf6, 0xff)) { /* OR imm8 */
        logAndPause("or n");
        A |= IMM8;
        F = A ? 0 : FLAG_Z;
        s->pc++;
    } else if (M(op, 0xf8, 0xff)) { /* LD HL, SP + imm8 */
        logAndPause("ldhl sp, n");
        u32 res = (u32)s->sp + (s8)IMM8;
        ZF = 0;
        NF = 0;
        HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
        CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
        HL = (u16)res;
        s->pc++;
    } else if (M(op, 0xf9, 0xff)) { /* LD SP, HL */
        logAndPause("ld SP, HL");
        s->sp = HL;
    } else if (M(op, 0xfa, 0xff)) { /* LD A, (imm16) */
        logAndPause("ld A (nn)");
        A = mmu_read(s, IMM16);
        s->pc += 2;
    } else if (M(op, 0xfb, 0xff)) { /* EI */
        logAndPause("ei");
        s->interrupts_master_enabled = 1;
    } else if (M(op, 0xfe, 0xff)) { /* CP imm8 */
        logAndPause("cp n");
        u8 n = IMM8;

        ZF = A == n;
        NF = 1;
        HF = (A & 0xf) < (n & 0xf);
        CF = A < n;
        s->pc++;
    } else {
        s->pc--;
        cpu_error("Unknown instruction");
    }
}


void cpu_step(struct gb_state *s) {
    u8 op;

    s->emu_state->last_op_cycles = 0;

    cpu_handle_interrupts(s);

    op = mmu_read(s, s->pc);

    s->emu_state->last_op_cycles = cycles_per_instruction[op];
    if (op == 0xcb) {
        op = mmu_read(s, s->pc + 1);
        s->emu_state->last_op_cycles = cycles_per_instruction_cb[op];
    }

    if (!s->halt_for_interrupts)
        cpu_do_instruction(s);
    else
        if (!s->interrupts_enable)
            cpu_error("Waiting for interrupts while disabled, deadlock.\n");

    if (s->pc >= 0x8000 && s->pc < 0xa000)
        cpu_error("PC in VRAM: %.4x\n", s->pc);
    else if (s->pc >= 0xa000 && s->pc < 0xc000)
        cpu_error("PC in external RAM: %.4x\n", s->pc);
    else if (s->pc >= 0xe000 && s->pc < 0xff80)
        cpu_error("PC in ECHO/OAM/IO/unusable: %.4x\n", s->pc);
}
