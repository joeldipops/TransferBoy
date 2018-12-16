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

/*
static void cpu_do_instruction(struct gb_state *s) {
    u8 op = mmu_read(s, s->pc++);
    if (M(op, 0x00, 0xff)) { // NOP 
    } else if (M(op, 0x01, 0xcf)) { // LD reg16, u16 
        u16 *dst = REG16(4);
        *dst = IMM16;
        s->pc += 2;
    } else if (M(op, 0x02, 0xff)) { // LD (BC), A 
        mmu_write(s, BC, A);
    } else if (M(op, 0x03, 0xcf)) { // INC reg16 
        u16 *reg = REG16(4);
        *reg += 1;
    } else if (M(op, 0x04, 0xc7)) { // INC reg8 
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
    } else if (M(op, 0x05, 0xc7)) { // DEC reg8 
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
    } else if (M(op, 0x06, 0xc7)) { // LD reg8, imm8 
        u8* dst = REG8(3);
        u8 src = IMM8;
        s->pc++;
        if (dst)
            *dst = src;
        else
            mmu_write(s, HL, src);
    } else if (M(op, 0x07, 0xff)) { // RLCA 
        u8 res = (A << 1) | (A >> 7);
        F = (A >> 7) ? FLAG_C : 0;
        A = res;

    } else if (M(op, 0x08, 0xff)) { // LD (imm16), SP 
        mmu_write16(s, IMM16, s->sp);
        s->pc += 2;

    } else if (M(op, 0x09, 0xcf)) { // ADD HL, reg16 
        u16 *src = REG16(4);
        u32 tmp = HL + *src;
        NF = 0;
        HF = (((HL & 0xfff) + (*src & 0xfff)) & 0x1000) ? 1 : 0;
        CF = tmp > 0xffff;
        HL = tmp;
    } else if (M(op, 0x0a, 0xff)) { // LD A, (BC) 
        A = mem(BC);
    } else if (M(op, 0x0b, 0xcf)) { // DEC reg16 
        u16 *reg = REG16(4);
        *reg -= 1;
    } else if (M(op, 0x0f, 0xff)) { // RRCA 
        F = (A & 1) ? FLAG_C : 0;
        A = (A >> 1) | ((A & 1) << 7);
    } else if (M(op, 0x10, 0xff)) { // STOP 
        //s->halt_for_interrupts = 1;
    } else if (M(op, 0x12, 0xff)) { // LD (DE), A 
        mmu_write(s, DE, A);
    } else if (M(op, 0x17, 0xff)) { // RLA 
        u8 res = A << 1 | (CF ? 1 : 0);
        F = (A & (1 << 7)) ? FLAG_C : 0;
        A = res;
    } else if (M(op, 0x18, 0xff)) { // JR off8 
        s->pc += (s8)IMM8 + 1;
    } else if (M(op, 0x1a, 0xff)) { // LD A, (DE) 
        A = mem(DE);
    } else if (M(op, 0x1f, 0xff)) { // RRA 
        u8 res = (A >> 1) | (CF << 7);
        ZF = 0;
        NF = 0;
        HF = 0;
        CF = A & 0x1;
        A = res;
    } else if (M(op, 0x20, 0xe7)) { // JR cond, off8 
        u8 flag = (op >> 3) & 3;
        if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
            s->pc += (s8)IMM8;
        s->pc++;
    } else if (M(op, 0x22, 0xff)) { // LDI (HL), A 
        mmu_write(s, HL, A);
        HL++;
    } else if (M(op, 0x27, 0xff)) { // DAA 
        // When adding/subtracting two numbers in BCD form, this instructions
         // brings the results back to BCD form too. In BCD form the decimals 0-9
         // are encoded in a fixed number of bits (4). E.g., 0x93 actually means
         // 93 decimal. Adding/subtracting such numbers takes them out of this
         // form since they can results in values where each digit is >9.
         // E.g., 0x9 + 0x1 = 0xA, but should be 0x10. The important thing to
         // note here is that per 4 bits we 'skip' 6 values (0xA-0xF), and thus
         // by adding 0x6 we get: 0xA + 0x6 = 0x10, the correct answer. The same
         // works for the upper byte (add 0x60).
         // So: If the lower byte is >9, we need to add 0x6.
         // If the upper byte is >9, we need to add 0x60.
         // Furthermore, if we carried the lower part (HF, 0x9+0x9=0x12) we
         // should also add 0x6 (0x12+0x6=0x18).
         // Similarly for the upper byte (CF, 0x90+0x90=0x120, +0x60=0x180).
         //
         // For subtractions (we know it was a subtraction by looking at the NF
         // flag) we simiarly need to *subtract 0x06/0x60/0x66 to again skip the
         // unused 6 values in each byte. The GB does this by only looking at the
         // NF and CF flags then.
         ///
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
    } else if (M(op, 0x2a, 0xff)) { // LDI A, (HL) 
        A = mmu_read(s, HL);
        HL++;
    } else if (M(op, 0x2f, 0xff)) { // CPL 
        A = ~A;
        NF = 1;
        HF = 1;
    } else if (M(op, 0x32, 0xff)) { // LDD (HL), A 
        mmu_write(s, HL, A);
        HL--;
    } else if (M(op, 0x37, 0xff)) { // SCF 
        NF = 0;
        HF = 0;
        CF = 1;
    } else if (M(op, 0x3a, 0xff)) { // LDD A, (HL) 
        A = mmu_read(s, HL);
        HL--;
    } else if (M(op, 0x3f, 0xff)) { // CCF 
        CF = CF ? 0 : 1;
        NF = 0;
        HF = 0;
    } else if (M(op, 0x76, 0xff)) { // HALT 
        s->halt_for_interrupts = 1;
    } else if (M(op, 0x40, 0xc0)) { // LD reg8, reg8 
        u8* src = REG8(0);
        u8* dst = REG8(3);
        u8 srcval = src ? *src : mem(HL);
        if (dst)
            *dst = srcval;
        else
            mmu_write(s, HL, srcval);
    } else if (M(op, 0x80, 0xf8)) { // ADD A, reg8 
        u8* src = REG8(0);
        u8 srcval = src ? *src : mem(HL);
        u16 res = A + srcval;
        ZF = (u8)res == 0;
        NF = 0;
        HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
        CF = res & 0x100 ? 1 : 0;
        A = (u8)res;
    } else if (M(op, 0x88, 0xf8)) { // ADC A, reg8 
        u8* src = REG8(0);
        u8 srcval = src ? *src : mem(HL);
        u16 res = A + srcval + CF;
        ZF = (u8)res == 0;
        NF = 0;
        HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
        CF = res & 0x100 ? 1 : 0;
        A = (u8)res;

    } else if (M(op, 0x90, 0xf8)) { // SUB reg8 
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = A - val;
        ZF = res == 0;
        NF = 1;
        HF = ((s32)A & 0xf) - (val & 0xf) < 0;
        CF = A < val;
        A = res;
    } else if (M(op, 0x98, 0xf8)) { // SBC A, reg8 
        u8 *reg = REG8(0);
        u8 regval = reg ? *reg : mem(HL);
        u8 res = A - regval - CF;
        ZF = res == 0;
        NF = 1;
        HF = ((s32)A & 0xf) - (regval & 0xf) - CF < 0;
        CF = A < regval + CF;
        A = res;
    } else if (M(op, 0xa0, 0xf8)) { // AND reg8 
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        A = A & val;
        ZF = A == 0;
        NF = 0;
        HF = 1;
        CF = 0;
    } else if (M(op, 0xa8, 0xf8)) { // XOR reg8 
        u8* src = REG8(0);
        u8 srcval = src ? *src : mem(HL);
        A ^= srcval;
        F = A ? 0 : FLAG_Z;
    } else if (M(op, 0xb0, 0xf8)) { // OR reg8 
        u8* src = REG8(0);
        u8 srcval = src ? *src : mem(HL);
        A |= srcval;
        F = A ? 0 : FLAG_Z;
    } else if (M(op, 0xb8, 0xf8)) { // CP reg8 
        u8 *reg = REG8(0);
        u8 regval = reg ? *reg : mem(HL);
        ZF = A == regval;
        NF = 1;
        HF = (A & 0xf) < (regval & 0xf);
        CF = A < regval;
    } else if (M(op, 0xc0, 0xe7)) { // RET cond 
        // TODO cyclecount depends on taken or not 

        u8 flag = (op >> 3) & 3;
        if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
            s->pc = mmu_pop16(s);

    } else if (M(op, 0xc1, 0xcf)) { // POP reg16 
        u16 *dst = REG16S(4);
        *dst = mmu_pop16(s);
        F = F & 0xf0;
    } else if (M(op, 0xc2, 0xe7)) { // JP cond, imm16 
        u8 flag = (op >> 3) & 3;
        if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
            s->pc = IMM16;
        else
            s->pc += 2;
    } else if (M(op, 0xc3, 0xff)) { // JP imm16 
        s->pc = IMM16;
    } else if (M(op, 0xc4, 0xe7)) { // CALL cond, imm16 
        u16 dst = IMM16;
        s->pc += 2;
        u8 flag = (op >> 3) & 3;
        if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1)) {
            mmu_push16(s, s->pc);
            s->pc = dst;
        }
    } else if (M(op, 0xc5, 0xcf)) { // PUSH reg16 
        u16 *src = REG16S(4);
        mmu_push16(s,*src);
    } else if (M(op, 0xc6, 0xff)) { // ADD A, imm8 
        u16 res = A + IMM8;
        ZF = (u8)res == 0;
        NF = 0;
        HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
        CF = res & 0x100 ? 1 : 0;
        A = (u8)res;
        s->pc++;
    } else if (M(op, 0xc7, 0xc7)) { // RST imm8 
        mmu_push16(s, s->pc);
        s->pc = ((op >> 3) & 7) * 8;
    } else if (M(op, 0xc9, 0xff)) { // RET 
        s->pc = mmu_pop16(s);
    } else if (M(op, 0xcd, 0xff)) { // CALL imm16 
        u16 dst = IMM16;
        mmu_push16(s, s->pc + 2);
        s->pc = dst;
    } else if (M(op, 0xce, 0xff)) { // ADC imm8 
        u16 res = A + IMM8 + CF;
        ZF = (u8)res == 0;
        NF = 0;
        HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
        CF = res & 0x100 ? 1 : 0;
        A = (u8)res;
        s->pc++;
    } else if (M(op, 0xd6, 0xff)) { // SUB imm8 
        u8 res = A - IMM8;
        ZF = res == 0;
        NF = 1;
        HF = ((s32)A & 0xf) - (IMM8 & 0xf) < 0;
        CF = A < IMM8;
        A = res;
        s->pc++;
    } else if (M(op, 0xd9, 0xff)) { // RETI 
        s->pc = mmu_pop16(s);
        s->interrupts_master_enabled = 1;
    } else if (M(op, 0xde, 0xff)) { // SBC imm8 
        u8 res = A - IMM8 - CF;
        ZF = res == 0;
        NF = 1;
        HF = ((s32)A & 0xf) - (IMM8 & 0xf) - CF < 0;
        CF = A < IMM8 + CF;
        A = res;
        s->pc++;
    } else if (M(op, 0xe0, 0xff)) { // LD (0xff00 + imm8), A 
        mmu_write(s, 0xff00 + IMM8, A);
        s->pc++;
    } else if (M(op, 0xe2, 0xff)) { // LD (0xff00 + C), A 
        mmu_write(s, 0xff00 + C, A);
    } else if (M(op, 0xe6, 0xff)) { // AND imm8 
        A = A & IMM8;
        s->pc++;
        ZF = A == 0;
        NF = 0;
        HF = 1;
        CF = 0;
    } else if (M(op, 0xe8, 0xff)) { // ADD SP, imm8s 
        s8 off = (s8)IMM8;
        u32 res = s->sp + off;
        ZF = 0;
        NF = 0;
        HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
        CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
        s->sp = res;
        s->pc++;
    } else if (M(op, 0xe9, 0xff)) { // LD PC, HL (or JP (HL) ) 
        s->pc = HL;
    } else if (M(op, 0xea, 0xff)) { // LD (imm16), A 
        mmu_write(s, IMM16, A);
        s->pc += 2;
    } else if (M(op, 0xcb, 0xff)) { // CB-prefixed extended instructions 
        return cpu_do_cb_instruction(s);
    } else if (M(op, 0xee, 0xff)) { // XOR imm8 
        A ^= IMM8;
        s->pc++;
        F = A ? 0 : FLAG_Z;
    } else if (M(op, 0xf0, 0xff)) { // LD A, (0xff00 + imm8) 
        A = mmu_read(s, 0xff00 + IMM8);
        s->pc++;
    } else if (M(op, 0xf2, 0xff)) { // LD A, (0xff00 + C) 
        A = mmu_read(s, 0xff00 + C);
    } else if (M(op, 0xf3, 0xff)) { // DI 
        s->interrupts_master_enabled = 0;
    } else if (M(op, 0xf6, 0xff)) { // OR imm8 
        A |= IMM8;
        F = A ? 0 : FLAG_Z;
        s->pc++;
    } else if (M(op, 0xf8, 0xff)) { // LD HL, SP + imm8 
        u32 res = (u32)s->sp + (s8)IMM8;
        ZF = 0;
        NF = 0;
        HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
        CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
        HL = (u16)res;
        s->pc++;
    } else if (M(op, 0xf9, 0xff)) { // LD SP, HL 
        s->sp = HL;
    } else if (M(op, 0xfa, 0xff)) { // LD A, (imm16) 
        A = mmu_read(s, IMM16);
        s->pc += 2;
    } else if (M(op, 0xfb, 0xff)) { // EI 
        s->interrupts_master_enabled = 1;
    } else if (M(op, 0xfe, 0xff)) { // CP imm8 
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
*/

static void cpu_do_instruction_tree(struct gb_state *s) {
    u8 op = getOpCodeFromROM(s, s->pc++);

    /*
    // "ld"s seem to be the most common instruction, so prioritise them.
    // hrmm, if anything this seems to be slower.  probably revert...
    if (op >= 0x40 && op <= 0x7F) {
        // 0x76 is an inexplicably unrelated instruction.
        if  (op == 0x76) {
            goto Ox76;
        } else {
            goto Ox40;
        }
    }*/

    if (op <= 0x7F) {
        if (op <= 0x3F) {
            if (op <= 0x1F ) {
                if (op <= 0x0F) {
                    if (op <= 0x07) {
                        if (op <= 0x03) {
                            if (op <= 0x01) {
                                if (op == 0x00) {
                                    goto Ox00;
                                } else {
                                    goto Ox01;
                                }
                            } else {
                                if (op == 0x02) {
                                    goto Ox02;
                                } else {
                                    goto Ox03;
                                }
                            }
                        } else {
                            if (op <= 0x05) {
                                if (op == 0x04) {
                                    goto Ox04;
                                } else {
                                    goto Ox05;
                                }
                            } else {
                                if (op == 0x06) {
                                    goto Ox06;
                                } else {
                                    goto Ox07;
                                }
                            }
                        }
                    } else {
                        if (op <= 0x0B) {
                            if (op <= 0x09) {
                                if (op == 0x08) {
                                    goto Ox08;
                                } else {
                                    goto Ox09;
                                }
                            } else {
                                if (op == 0x0A) {
                                    goto Ox0A;
                                } else {
                                    goto Ox0B;
                                }
                            }
                        } else {
                            if (op <= 0x0D) {
                                if (op == 0x0C) {
                                    goto Ox0C;
                                } else {
                                    goto Ox0D;
                                }
                            } else {
                                if (op == 0x0E) {
                                    goto Ox0E;
                                } else {
                                    goto Ox0F;
                                }
                            }
                        }
                    }
                } else {
                    if (op <= 0x17) {
                        if (op <= 0x13) {
                            if (op <= 0x11) {
                                if (op == 0x10) {
                                    goto Ox10;
                                } else {
                                    goto Ox11;
                                }
                            } else {
                                if (op == 0x12) {
                                    goto Ox12;
                                } else {
                                    goto Ox13;
                                }
                            }
                        } else {
                            if (op <= 0x15) {
                                if (op == 0x14) {
                                    goto Ox14;
                                } else {
                                    goto Ox15;
                                }
                            } else {
                                if (op == 0x16) {
                                    goto Ox16;
                                } else {
                                    goto Ox17;
                                }
                            }
                        }
                    } else {
                        if (op <= 0x1B) {
                            if (op <= 0x19) {
                                if (op == 0x18) {
                                    goto Ox18;
                                } else {
                                    goto Ox19;
                                }
                            } else {
                                if (op == 0x1A) {
                                    goto Ox1A;
                                } else {
                                    goto Ox1B;
                                }
                            }
                        } else {
                            if (op <= 0x1D) {
                                if (op == 0x1C) {
                                    goto Ox1C;
                                } else {
                                    goto Ox1D;
                                }
                            } else {
                                if (op == 0x1E) {
                                    goto Ox1E;
                                } else {
                                    goto Ox1F;
                                }
                            }
                        }
                    }
                }
            } else {
                if (op <= 0x2F) {
                    if (op <= 0x27) {
                        if (op <= 0x23) {
                            if (op <= 0x21) {
                                if (op == 0x20) {
                                    goto Ox20;
                                } else {
                                    goto Ox21;
                                }
                            } else {
                                if (op == 0x22) {
                                    goto Ox22;
                                } else {
                                    goto Ox23;
                                }
                            }
                        } else {
                            if (op <= 0x25) {
                                if (op == 0x24) {
                                    goto Ox24;
                                } else {
                                    goto Ox25;
                                }
                            } else {
                                if (op == 0x26) {
                                    goto Ox26;
                                } else {
                                    goto Ox27;
                                }
                            }
                        }
                    } else {
                        if (op <= 0x2B) {
                            if (op <= 0x29) {
                                if (op == 0x28) {
                                    goto Ox28;
                                } else {
                                    goto Ox29;
                                }
                            } else {
                                if (op == 0x2A) {
                                    goto Ox2A;
                                } else {
                                    goto Ox2B;
                                }
                            }
                        } else {
                            if (op <= 0x2D) {
                                if (op == 0x2C) {
                                    goto Ox2C;
                                } else {
                                    goto Ox2D;
                                }
                            } else {
                                if (op == 0x2E) {
                                    goto Ox2E;
                                } else {
                                    goto Ox2F;
                                }
                            }
                        }
                    }
                } else {
                    if (op <= 0x37) {
                        if (op <= 0x33) {
                            if (op <= 0x31) {
                                if (op == 0x30) {
                                    goto Ox30;
                                } else {
                                    goto Ox31;
                                }
                            } else {
                                if (op == 0x32) {
                                    goto Ox32;
                                } else {
                                    goto Ox33;
                                }
                            }
                        } else {
                            if (op <= 0x35) {
                                if (op == 0x34) {
                                    goto Ox34;
                                } else {
                                    goto Ox35;
                                }
                            } else {
                                if (op == 0x36) {
                                    goto Ox36;
                                } else {
                                    goto Ox37;
                                }
                            }
                        }
                    } else {
                        if (op <= 0x3B) {
                            if (op <= 0x39) {
                                if (op == 0x38) {
                                    goto Ox38;
                                } else {
                                    goto Ox39;
                                }
                            } else {
                                if (op == 0x3A) {
                                    goto Ox3A;
                                } else {
                                    goto Ox3B;
                                }
                            }
                        } else {
                            if (op <= 0x3D) {
                                if (op == 0x3C) {
                                    goto Ox3C;
                                } else {
                                    goto Ox3D;
                                }
                            } else {
                                if (op == 0x3E) {
                                    goto Ox3E;
                                } else {
                                    goto Ox3F;
                                }
                            }
                        }
                    }
                }
            }
        } else {
            if (op <= 0x5F ) {
                if (op <= 0x4F) {
                    if (op <= 0x47) {
                        if (op <= 0x43) {
                            if (op <= 0x41) {
                                if (op == 0x40) {
                                    goto Ox40;
                                } else {
                                    goto Ox41;
                                }
                            } else {
                                if (op == 0x42) {
                                    goto Ox42;
                                } else {
                                    goto Ox43;
                                }
                            }
                        } else {
                            if (op <= 0x45) {
                                if (op == 0x44) {
                                    goto Ox44;
                                } else {
                                    goto Ox45;
                                }
                            } else {
                                if (op == 0x46) {
                                    goto Ox46;
                                } else {
                                    goto Ox47;
                                }
                            }
                        }
                    } else {
                        if (op <= 0x4B) {
                            if (op <= 0x49) {
                                if (op == 0x48) {
                                    goto Ox48;
                                } else {
                                    goto Ox49;
                                }
                            } else {
                                if (op == 0x4A) {
                                    goto Ox4A;
                                } else {
                                    goto Ox4B;
                                }
                            }
                        } else {
                            if (op <= 0x4D) {
                                if (op == 0x4C) {
                                    goto Ox4C;
                                } else {
                                    goto Ox4D;
                                }
                            } else {
                                if (op == 0x4E) {
                                    goto Ox4E;
                                } else {
                                    goto Ox4F;
                                }
                            }
                        }
                    }
                } else {
                    if (op <= 0x57) {
                        if (op <= 0x53) {
                            if (op <= 0x51) {
                                if (op == 0x50) {
                                    goto Ox50;
                                } else {
                                    goto Ox51;
                                }
                            } else {
                                if (op == 0x52) {
                                    goto Ox52;
                                } else {
                                    goto Ox53;
                                }
                            }
                        } else {
                            if (op <= 0x55) {
                                if (op == 0x54) {
                                    goto Ox54;
                                } else {
                                    goto Ox55;
                                }
                            } else {
                                if (op == 0x56) {
                                    goto Ox56;
                                } else {
                                    goto Ox57;
                                }
                            }
                        }
                    } else {
                        if (op <= 0x5B) {
                            if (op <= 0x59) {
                                if (op == 0x58) {
                                    goto Ox58;
                                } else {
                                    goto Ox59;
                                }
                            } else {
                                if (op == 0x5A) {
                                    goto Ox5A;
                                } else {
                                    goto Ox5B;
                                }
                            }
                        } else {
                            if (op <= 0x5D) {
                                if (op == 0x5C) {
                                    goto Ox5C;
                                } else {
                                    goto Ox5D;
                                }
                            } else {
                                if (op == 0x5E) {
                                    goto Ox5E;
                                } else {
                                    goto Ox5F;
                                }
                            }
                        }
                    }
                }
            } else {
                if (op <= 0x6F) {
                    if (op <= 0x67) {
                        if (op <= 0x63) {
                            if (op <= 0x61) {
                                if (op == 0x60) {
                                    goto Ox60;
                                } else {
                                    goto Ox61;
                                }
                            } else {
                                if (op == 0x62) {
                                    goto Ox62;
                                } else {
                                    goto Ox63;
                                }
                            }
                        } else {
                            if (op <= 0x65) {
                                if (op == 0x64) {
                                    goto Ox64;
                                } else {
                                    goto Ox65;
                                }
                            } else {
                                if (op == 0x66) {
                                    goto Ox66;
                                } else {
                                    goto Ox67;
                                }
                            }
                        }
                    } else {
                        if (op <= 0x6B) {
                            if (op <= 0x69) {
                                if (op == 0x68) {
                                    goto Ox68;
                                } else {
                                    goto Ox69;
                                }
                            } else {
                                if (op == 0x6A) {
                                    goto Ox6A;
                                } else {
                                    goto Ox6B;
                                }
                            }
                        } else {
                            if (op <= 0x6D) {
                                if (op == 0x6C) {
                                    goto Ox6C;
                                } else {
                                    goto Ox6D;
                                }
                            } else {
                                if (op == 0x6E) {
                                    goto Ox6E;
                                } else {
                                    goto Ox6F;
                                }
                            }
                        }
                    }
                } else {
                    if (op <= 0x77) {
                        if (op <= 0x73) {
                            if (op <= 0x71) {
                                if (op == 0x70) {
                                    goto Ox70;
                                } else {
                                    goto Ox71;
                                }
                            } else {
                                if (op == 0x72) {
                                    goto Ox72;
                                } else {
                                    goto Ox73;
                                }
                            }
                        } else {
                            if (op <= 0x75) {
                                if (op == 0x74) {
                                    goto Ox74;
                                } else {
                                    goto Ox75;
                                }
                            } else {
                                if (op == 0x76) {
                                    goto Ox76;
                                } else {
                                    goto Ox77;
                                }
                            }
                        }
                    } else {
                        if (op <= 0x7B) {
                            if (op <= 0x79) {
                                if (op == 0x78) {
                                    goto Ox78;
                                } else {
                                    goto Ox79;
                                }
                            } else {
                                if (op == 0x7A) {
                                    goto Ox7A;
                                } else {
                                    goto Ox7B;
                                }
                            }
                        } else {
                            if (op <= 0x7D) {
                                if (op == 0x7C) {
                                    goto Ox7C;
                                } else {
                                    goto Ox7D;
                                }
                            } else {
                                if (op == 0x7E) {
                                    goto Ox7E;
                                } else {
                                    goto Ox7F;
                                }
                            }
                        }
                    }
                }
            }
        }
    }  else {
        if (op <= 0xBF) {
            if (op <= 0x9F ) {
                if (op <= 0x8F) {
                    if (op <= 0x87) {
                        if (op <= 0x83) {
                            if (op <= 0x81) {
                                if (op == 0x80) {
                                    goto Ox80;
                                } else {
                                    goto Ox81;
                                }
                            } else {
                                if (op == 0x82) {
                                    goto Ox82;
                                } else {
                                    goto Ox83;
                                }
                            }
                        } else {
                            if (op <= 0x85) {
                                if (op == 0x84) {
                                    goto Ox84;
                                } else {
                                    goto Ox85;
                                }
                            } else {
                                if (op == 0x86) {
                                    goto Ox86;
                                } else {
                                    goto Ox87;
                                }
                            }
                        }
                    } else {
                        if (op <= 0x8B) {
                            if (op <= 0x89) {
                                if (op == 0x88) {
                                    goto Ox88;
                                } else {
                                    goto Ox89;
                                }
                            } else {
                                if (op == 0x8A) {
                                    goto Ox8A;
                                } else {
                                    goto Ox8B;
                                }
                            }
                        } else {
                            if (op <= 0x8D) {
                                if (op == 0x8C) {
                                    goto Ox8C;
                                } else {
                                    goto Ox8D;
                                }
                            } else {
                                if (op == 0x8E) {
                                    goto Ox8E;
                                } else {
                                    goto Ox8F;
                                }
                            }
                        }
                    }
                } else {
                    if (op <= 0x97) {
                        if (op <= 0x93) {
                            if (op <= 0x91) {
                                if (op == 0x90) {
                                    goto Ox90;
                                } else {
                                    goto Ox91;
                                }
                            } else {
                                if (op == 0x92) {
                                    goto Ox92;
                                } else {
                                    goto Ox93;
                                }
                            }
                        } else {
                            if (op <= 0x95) {
                                if (op == 0x94) {
                                    goto Ox94;
                                } else {
                                    goto Ox95;
                                }
                            } else {
                                if (op == 0x96) {
                                    goto Ox96;
                                } else {
                                    goto Ox97;
                                }
                            }
                        }
                    } else {
                        if (op <= 0x9B) {
                            if (op <= 0x99) {
                                if (op == 0x98) {
                                    goto Ox98;
                                } else {
                                    goto Ox99;
                                }
                            } else {
                                if (op == 0x9A) {
                                    goto Ox9A;
                                } else {
                                    goto Ox9B;
                                }
                            }
                        } else {
                            if (op <= 0x9D) {
                                if (op == 0x9C) {
                                    goto Ox9C;
                                } else {
                                    goto Ox9D;
                                }
                            } else {
                                if (op == 0x9E) {
                                    goto Ox9E;
                                } else {
                                    goto Ox9F;
                                }
                            }
                        }
                    }
                }
            } else {
                if (op <= 0xAF) {
                    if (op <= 0xA7) {
                        if (op <= 0xA3) {
                            if (op <= 0xA1) {
                                if (op == 0xA0) {
                                    goto OxA0;
                                } else {
                                    goto OxA1;
                                }
                            } else {
                                if (op == 0xA2) {
                                    goto OxA2;
                                } else {
                                    goto OxA3;
                                }
                            }
                        } else {
                            if (op <= 0xA5) {
                                if (op == 0xA4) {
                                    goto OxA4;
                                } else {
                                    goto OxA5;
                                }
                            } else {
                                if (op == 0xA6) {
                                    goto OxA6;
                                } else {
                                    goto OxA7;
                                }
                            }
                        }
                    } else {
                        if (op <= 0xAB) {
                            if (op <= 0xA9) {
                                if (op == 0xA8) {
                                    goto OxA8;
                                } else {
                                    goto OxA9;
                                }
                            } else {
                                if (op == 0xAA) {
                                    goto OxAA;
                                } else {
                                    goto OxAB;
                                }
                            }
                        } else {
                            if (op <= 0xAD) {
                                if (op == 0xAC) {
                                    goto OxAC;
                                } else {
                                    goto OxAD;
                                }
                            } else {
                                if (op == 0xAE) {
                                    goto OxAE;
                                } else {
                                    goto OxAF;
                                }
                            }
                        }
                    }
                } else {
                    if (op <= 0xB7) {
                        if (op <= 0xB3) {
                            if (op <= 0xB1) {
                                if (op == 0xB0) {
                                    goto OxB0;
                                } else {
                                    goto OxB1;
                                }
                            } else {
                                if (op == 0xB2) {
                                    goto OxB2;
                                } else {
                                    goto OxB3;
                                }
                            }
                        } else {
                            if (op <= 0xB5) {
                                if (op == 0xB4) {
                                    goto OxB4;
                                } else {
                                    goto OxB5;
                                }
                            } else {
                                if (op == 0xB6) {
                                    goto OxB6;
                                } else {
                                    goto OxB7;
                                }
                            }
                        }
                    } else {
                        if (op <= 0xBB) {
                            if (op <= 0xB9) {
                                if (op == 0xB8) {
                                    goto OxB8;
                                } else {
                                    goto OxB9;
                                }
                            } else {
                                if (op == 0xBA) {
                                    goto OxBA;
                                } else {
                                    goto OxBB;
                                }
                            }
                        } else {
                            if (op <= 0xBD) {
                                if (op == 0xBC) {
                                    goto OxBC;
                                } else {
                                    goto OxBD;
                                }
                            } else {
                                if (op == 0xBE) {
                                    goto OxBE;
                                } else {
                                    goto OxBF;
                                }
                            }
                        }
                    }
                }
            }
        } else {
            if (op <= 0xDF ) {
                if (op <= 0xCF) {
                    if (op <= 0xC7) {
                        if (op <= 0xC3) {
                            if (op <= 0xC1) {
                                if (op == 0xC0) {
                                    goto OxC0;
                                } else {
                                    goto OxC1;
                                }
                            } else {
                                if (op == 0xC2) {
                                    goto OxC2;
                                } else {
                                    goto OxC3;
                                }
                            }
                        } else {
                            if (op <= 0xC5) {
                                if (op == 0xC4) {
                                    goto OxC4;
                                } else {
                                    goto OxC5;
                                }
                            } else {
                                if (op == 0xC6) {
                                    goto OxC6;
                                } else {
                                    goto OxC7;
                                }
                            }
                        }
                    } else {
                        if (op <= 0xCB) {
                            if (op <= 0xC9) {
                                if (op == 0xC8) {
                                    goto OxC8;
                                } else {
                                    goto OxC9;
                                }
                            } else {
                                if (op == 0xCA) {
                                    goto OxCA;
                                } else {
                                    goto OxCB;
                                }
                            }
                        } else {
                            if (op <= 0xCD) {
                                if (op == 0xCC) {
                                    goto OxCC;
                                } else {
                                    goto OxCD;
                                }
                            } else {
                                if (op == 0xCE) {
                                    goto OxCE;
                                } else {
                                    goto OxCF;
                                }
                            }
                        }
                    }
                } else {
                    if (op <= 0xD7) {
                        if (op <= 0xD3) {
                            if (op <= 0xD1) {
                                if (op == 0xD0) {
                                    goto OxD0;
                                } else {
                                    goto OxD1;
                                }
                            } else {
                                if (op == 0xD2) {
                                    goto OxD2;
                                } else {
                                    // UNUSED
                                    // goto OxD3;
                                }
                            }
                        } else {
                            if (op <= 0xD5) {
                                if (op == 0xD4) {
                                    goto OxD4;
                                } else {
                                    goto OxD5;
                                }
                            } else {
                                if (op == 0xD6) {
                                    goto OxD6;
                                } else {
                                    goto OxD7;
                                }
                            }
                        }
                    } else {
                        if (op <= 0xDB) {
                            if (op <= 0xD9) {
                                if (op == 0xD8) {
                                    goto OxD8;
                                } else {
                                    goto OxD9;
                                }
                            } else {
                                if (op == 0xDA) {
                                    goto OxDA;
                                } else {
                                    // UNUSED
                                    //goto OxDB;
                                }
                            }
                        } else {
                            if (op <= 0xDD) {
                                if (op == 0xDC) {
                                    goto OxDC;
                                } else {
                                    // UNUSED
                                    //goto OxDD;
                                }
                            } else {
                                if (op == 0xDE) {
                                    goto OxDE;
                                } else {
                                    goto OxDF;
                                }
                            }
                        }
                    }
                }
            } else {
                if (op <= 0xEF) {
                    if (op <= 0xE7) {
                        if (op <= 0xE3) {
                            if (op <= 0xE1) {
                                if (op == 0xE0) {
                                    goto OxE0;
                                } else {
                                    goto OxE1;
                                }
                            } else {
                                if (op == 0xE2) {
                                    goto OxE2;
                                } else {
                                    // UNUSED
                                    //goto OxE3;
                                }
                            }
                        } else {
                            if (op <= 0xE5) {
                                if (op == 0xE4) {
                                    // UNUSED
                                    //goto OxE4;
                                } else {
                                    goto OxE5;
                                }
                            } else {
                                if (op == 0xE6) {
                                    goto OxE6;
                                } else {
                                    goto OxE7;
                                }
                            }
                        }
                    } else {
                        if (op <= 0xEB) {
                            if (op <= 0xE9) {
                                if (op == 0xE8) {
                                    goto OxE8;
                                } else {
                                    goto OxE9;
                                }
                            } else {
                                if (op == 0xEA) {
                                    goto OxEA;
                                } else {
                                    // UNUSED
                                    //goto OxEB;
                                }
                            }
                        } else {
                            if (op <= 0xED) {
                                // UNUSED
                                //if (op == 0xEC) {
                                //    goto OxEC;
                                //} else {
                                //    goto OxED;
                                //}
                            } else {
                                if (op == 0xEE) {
                                    goto OxEE;
                                } else {
                                    goto OxEF;
                                }
                            }
                        }
                    }
                } else {
                    if (op <= 0xF7) {
                        if (op <= 0xF3) {
                            if (op <= 0xF1) {
                                if (op == 0xF0) {
                                    goto OxF0;
                                } else {
                                    goto OxF1;
                                }
                            } else {
                                if (op == 0xF2) {
                                    goto OxF2;
                                } else {
                                    goto OxF3;
                                }
                            }
                        } else {
                            if (op <= 0xF5) {
                                if (op == 0xF4) {
                                    //UNUSED
                                    //goto OxF4;
                                } else {
                                    goto OxF5;
                                }
                            } else {
                                if (op == 0xF6) {
                                    goto OxF6;
                                } else {
                                    goto OxF7;
                                }
                            }
                        }
                    } else {
                        if (op <= 0xFB) {
                            if (op <= 0xF9) {
                                if (op == 0xF8) {
                                    goto OxF8;
                                } else {
                                    goto OxF9;
                                }
                            } else {
                                if (op == 0xFA) {
                                    goto OxFA;
                                } else {
                                    goto OxFB;
                                }
                            }
                        } else {
                            if (op <= 0xFD) {
                                // UNUSED
                                //if (op == 0xFC) {
                                 //   goto OxFC;
                                //} else {
                                 //   goto OxFD;
                                //}
                            } else {
                                if (op == 0xFE) {
                                    goto OxFE;
                                } else {
                                    goto OxFF;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return;
        OxCB:
            //logAndPause("cb extended");
            cpu_do_cb_instruction(s);
            return;

        // 8-bit loads
        // ld nn, n
        Ox06: // ld B, n
        Ox0E: // ld C, n
        Ox16: // ld D, n
        Ox1E: // ld E, n
        Ox26: // ld H, n
        Ox2E: // ld L, n
        Ox36: // ld (HL), n
        Ox3E: { // ld A, n
            //logAndPause("ld reg8, n");
            u8* dst = REG8(3);
            u8 src = IMM8;
            s->pc++;
            if (dst)
                *dst = src;
            else
                mmu_write(s, HL, src);
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
        Ox47: // ld B, A
        Ox48: // ld C, B
        Ox49: // ld C, C
        Ox4A: // ld C, D
        Ox4B: // ld C, E
        Ox4C: // ld C, H
        Ox4D: // ld C, L
        Ox4E: // ld C, (HL)
        Ox4F: // ld C, A
        Ox50: // ld D, B
        Ox51: // ld D, C
        Ox52: // ld D, D
        Ox53: // ld D, E
        Ox54: // ld D, H
        Ox55: // ld D, L
        Ox56: // ld D, (HL)
        Ox57: // ld D, A
        Ox58: // ld E, B
        Ox59: // ld E, C
        Ox5A: // ld E, D
        Ox5B: // ld E, E
        Ox5C: // ld E, H
        Ox5D: // ld E, L
        Ox5E: // ld E, (HL)
        Ox5F: // ld E, A
        Ox60: // ld H, B
        Ox61: // ld H, C
        Ox62: // ld H, D
        Ox63: // ld H, E
        Ox64: // ld H, H
        Ox65: // ld H, L
        Ox66: // ld H, (HL)
        Ox67: // ld H, A
        Ox68: // ld L, B
        Ox69: // ld L, C
        Ox6A: // ld L, D
        Ox6B: // ld L, E
        Ox6C: // ld L, H
        Ox6D: // ld L, L
        Ox6E: // ld L, (HL)
        Ox6F: // ld L, A
        Ox70: // ld (HL), B
        Ox71: // ld (HL), C
        Ox72: // ld (HL), D
        Ox73: // ld (HL), E
        Ox74: // ld (HL), H
        Ox75: // ld (HL), L
        Ox77: // ld (HL), A
        Ox78: // ld A, B
        Ox79: // ld A, C
        Ox7A: // ld A, D
        Ox7B: // ld A, E
        Ox7C: // ld A, H
        Ox7D: // ld A, L
        Ox7E: // ld A, (HL)
        Ox7F: // ld A, A
        {
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
            A = mem(BC);
            return;
        }

        Ox1A: { // ld A, (DE)
            //logAndPause("ld A, (DE)");
            A = mem(DE);
            return;
        }

        OxFA: { // ld A, (nn)
            //logAndPause("ld A, nn");
            A = mmu_read(s, IMM16);
            s->pc += 2;
            return;
        }

        // LD n, A
        Ox02: { // ld (BC), A
            //logAndPause("ld (BC), A");
            mmu_write(s, BC, A);
            return;
        }

        Ox12: { // ld (DE), A
            //logAndPause("ld (DE), A");
            mmu_write(s, DE, A);
            return;
        }

        OxEA: { // ld (nn), A
            //logAndPause("ld nn, A");
            mmu_write(s, IMM16, A);
            s->pc += 2;
            return;
        }

        OxF2: { // ld A, (C)
           //logAndPause("ld A, (C)");
           A = mmu_read(s, 0xff00 + C);
           return;
        }

        OxE2: { // ld (C), A
            //logAndPause("ld (C), A");
            mmu_write(s, 0xff00 + C, A);
            return;
        }

        // LDD  - decrements HL after load
        Ox3A: { // ldd A, (HL)
            //logAndPause("ldd A, (HL)");
            A = mmu_read(s, HL);
            HL--;
            return;
        }

        Ox32: { // ldd (HL), A
            //logAndPause("ldd (HL), A");
            mmu_write(s, HL, A);
            HL--;
            return;
        }

        // LDI - Increments after load
        Ox2A: { // ldi A, (HL)
            //logAndPause("ldi A, (HL)");
            A = mmu_read(s, HL);
            HL++;
            return;
        }

        Ox22: { // ldi (HL), A
            //logAndPause("ldi (HL), A");
            mmu_write(s, HL, A);
            HL++;
            return;
        }

        // LDH
        OxE0: { // ldh (n), A
            //logAndPause("ldh (n), A");
            mmu_write(s, 0xff00 + IMM8, A);
            s->pc++;
            return;
        }

        OxF0: { // ldh A, (n)
            //logAndPause("ldh A, (n)");
            A = mmu_read(s, 0xff00 + IMM8);
            s->pc++;
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
            s->pc += 2;
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
            s->pc++;
            return;
        }

        Ox08: { // ld (nn), SP
            // push 16 bits on to stack and sp -= 2
            //logAndPause("ld (nn), SP");
            mmu_write16(s, IMM16, s->sp);
            s->pc += 2;
            return;
        }

        OxF5: // push AF
        OxC5: // push BC
        OxD5: // push DE
        OxE5: { // push HL
            //logAndPause("push reg16");
            u16 *src = REG16S(4);
            mmu_push16(s,*src);
            return;
        }

        // pop 16 bits from stack and sp += 2
        OxF1: // pop AF
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
        Ox87: // add A, A
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
            u16 res = A + srcval;
            ZF = (u8)res == 0;
            NF = 0;
            HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
            CF = res & 0x100 ? 1 : 0;
            A = (u8)res;
            return;
        }

        OxC6: { // add A, n
            //logAndPause("add A, n");
            u16 res = A + IMM8;
            ZF = (u8)res == 0;
            NF = 0;
            HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
            CF = res & 0x100 ? 1 : 0;
            A = (u8)res;
            s->pc++;
            return;
        }

        // ADC - add n + carry flag
        Ox8F: // adc A, A
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
            u16 res = A + srcval + CF;
            ZF = (u8)res == 0;
            NF = 0;
            HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
            CF = res & 0x100 ? 1 : 0;
            A = (u8)res;
            return;
        }

        OxCE: { // adc A, n
            //logAndPause("adc A, n");
            u16 res = A + IMM8 + CF;
            ZF = (u8)res == 0;
            NF = 0;
            HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
            CF = res & 0x100 ? 1 : 0;
            A = (u8)res;
            s->pc++;
            return;
        }

        // Subtraction - Always subtracts from A.  Not sure if A is required parameter
        Ox97: // sub A
        Ox90: // sub B
        Ox91: // sub C
        Ox92: // sub D
        Ox93: // sub E
        Ox94: // sub H
        Ox95: // sub L
        Ox96: { // sub (HL)
            //logAndPause("sub reg8");
            u8 *reg = REG8(0);
            u8 val = reg ? *reg : mem(HL);
            u8 res = A - val;
            ZF = res == 0;
            NF = 1;
            HF = ((s32)A & 0xf) - (val & 0xf) < 0;
            CF = A < val;
            A = res;
            return;
        }

        OxD6: { // sub n
            //logAndPause("sub n");
            u8 res = A - IMM8;
            ZF = res == 0;
            NF = 1;
            HF = ((s32)A & 0xf) - (IMM8 & 0xf) < 0;
            CF = A < IMM8;
            A = res;
            s->pc++;
            return;
        }

        // Subtract + carry flag
        Ox9F: // sbc A, A
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
            u8 res = A - regval - CF;
            ZF = res == 0;
            NF = 1;
            HF = ((s32)A & 0xf) - (regval & 0xf) - CF < 0;
            CF = A < regval + CF;
            A = res;
            return;
        }

        OxDE: { // sbc A, n
            //logAndPause("sbc A, n");
            u8 res = A - IMM8 - CF;
            ZF = res == 0;
            NF = 1;
            HF = ((s32)A & 0xf) - (IMM8 & 0xf) - CF < 0;
            CF = A < IMM8 + CF;
            A = res;
            s->pc++;
            return;
        }
        // AND n with A result in A
        OxA7: // and A
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
            A = A & val;
            ZF = A == 0;
            NF = 0;
            HF = 1;
            CF = 0;
            return;
        }

        OxE6: { // and n
            //logAndPause("and n");
            A = A & IMM8;
            s->pc++;
            ZF = A == 0;
            NF = 0;
            HF = 1;
            CF = 0;
            return;
        }

        // OR n with A result in A
        OxB7: // or A
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
            A |= srcval;
            F = A ? 0 : FLAG_Z;
            return;
        }

        OxF6: { // or n
            //logAndPause ("or n");
            A |= IMM8;
            F = A ? 0 : FLAG_Z;
            s->pc++;
            return;
        }

        // XOR n with A result in A
        OxAF: // xor A
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
            A ^= srcval;
            F = A ? 0 : FLAG_Z;
            return;
        }

        OxEE: { // xor n
            //logAndPause("xor n");
            A ^= IMM8;
            s->pc++;
            F = A ? 0 : FLAG_Z;
            return;
        }

        // Compare contents of paramter with A, Z flag set if results equal
        OxBF: // cp A
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

            ZF = A == regval;
            NF = 1;
            HF = (A & 0xf) < (regval & 0xf);
            CF = A < regval;
            return;
        }

        OxFE: { // cp n
            //logAndPause("cp n");
            u8 n = IMM8;

            ZF = A == n;
            NF = 1;
            HF = (A & 0xf) < (n & 0xf);
            CF = A < n;
            s->pc++;
            return;
        }

        // Increment value of parameter
        Ox3C: // inc A
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
        Ox3D: // dec A
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
            s->pc++;
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
            if ((!NF && (A & 0xf) > 0x9) || HF)
                add |= 0x6;
            if ((!NF && A > 0x99) || CF) {
                add |= 0x60;
                CF = 1;
            }
            A += NF ? -add : add;
            ZF = A == 0;
            HF = 0;
            return;
        }
        Ox2F: { // cpl
            // flip all bits in A
            //logAndPause("cpl");
            A = ~A;
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
            u8 res = (A << 1) | (A >> 7);
            F = (A >> 7) ? FLAG_C : 0;
            A = res;
            return;
        }
        Ox17: { // rla
            // rotate a left - don't know how this differs with rlca
            //logAndPause("rla");
            u8 res = A << 1 | (CF ? 1 : 0);
            F = (A & (1 << 7)) ? FLAG_C : 0;
            A = res;
            return;
        }
        Ox0F: { // rrca
            // rotate a right - shift bit 0 to carry
            //logAndPause("rrca");
            F = (A & 1) ? FLAG_C : 0;
            A = (A >> 1) | ((A & 1) << 7);
            return;
        }
        Ox1F: { // rra
            // rotate a right - don't know how this differs with rrca
            //logAndPause("rra");
            u8 res = (A >> 1) | (CF << 7);
            ZF = 0;
            NF = 0;
            HF = 0;
            CF = A & 0x1;
            A = res;
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
            s->pc = IMM16;
            return;
        }
        OxC2: // jp NZ, nn - jump if !Z
        OxCA: // jp Z, nn - jump if Z
        OxD2: // jp NC, nn - jump if !C
        OxDA: { // jp C, nn - jump if C
            //logAndPause("jp cc, nn");
            u8 flag = (op >> 3) & 3;
            if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
                s->pc = IMM16;
            else
                s->pc += 2;
            return;
        }

        OxE9: {// jp (HL)
            //logAndPause("jp, (HL)");
            s->pc = HL;
            return;
        }
        Ox18: { // jr n
            // jump n addresses forward
            //logAndPause("jr, n");
            s->pc += (s8)IMM8 + 1;
            return;
        }
        Ox20: // jr NZ, n
        Ox28: // jr Z, n
        Ox30: // jr NC, n
        Ox38: { // jr C, n
            //logAndPause("jr cc, n");
            u8 flag = (op >> 3) & 3;

            if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1)) {
                s->pc += (s8)IMM8;
            }
            s->pc++;
            return;
        }

        // Push address of next instruction onto stack and then jump to address nn.
        OxCD: { // call nn
            //logAndPause("call nn");
            u16 dst = IMM16;
            mmu_push16(s, s->pc + 2);
            s->pc = dst;
            return;
        }

        OxC4: // call NZ, nn
        OxCC: // call Z, nn
        OxD4: // call NC, cc
        OxDC: { // call C, nn
            //logAndPause("call cc, nn");
            u16 dst = IMM16;
            s->pc += 2;
            u8 flag = (op >> 3) & 3;
            if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1)) {
                mmu_push16(s, s->pc);
                s->pc = dst;
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
            mmu_push16(s, s->pc);
            s->pc = ((op >> 3) & 7) * 8;
            return;
        }

        // Return -- pop two bytes of stack and jump to that address
        OxC9: { // ret
            //logAndPause("ret");
            s->pc = mmu_pop16(s);
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
                s->pc = mmu_pop16(s);
            return;
        }

        OxD9: { // reti
            //logAndPause("reti");
            // return then enable interrupts.
            s->pc = mmu_pop16(s);
            s->interrupts_master_enabled = 1;
            return;
        }
 }

 void cpu_step(struct gb_state *s) {
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
        cpu_do_instruction_tree(s);    
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
