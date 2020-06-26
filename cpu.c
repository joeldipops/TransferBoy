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

    // Use GBC mode if compatible.
    if (s->Cartridge.IsGbcSupported) {
        s->reg16.AF = 0x1180;
        s->reg16.BC = 0x0000;
        s->reg16.DE = 0xff56;
        s->reg16.HL = 0x000d;
        s->GBCSpeedSwitch = 0x7E; // Everything returns 1 except first and last bits.        
    } else {
        s->GBCSpeedSwitch = 0xFF;
    }

    s->halt_for_interrupts = 0;
    s->interrupts_master_enabled = 1;
    s->InterruptSwitch  = 0x0;
    s->InterruptFlags = 0x0;

    s->io_lcd_mode_cycles_left = 0;
    s->BackgroundScrollX  = 0x00;
    s->BackgroundScrollY  = 0x00;
    s->WindowLeft   = 0x00;
    s->WindowTop   = 0x00;
    s->LcdControl = 0x91;
    s->LcdStatus = 0x00;
    s->CurrentLine   = 0x00;
    s->NextInterruptLine  = 0x00;
    s->BackgroundPalette  = 0xfc;
    s->SpritePalette0 = 0xff;
    s->SpritePalette1 = 0xff;
    s->GbcBackgroundPaletteIndexRegister = 0x00;
    s->GbcSpritePaletteIndexRegister = 0x00;
    memset(s->io_lcd_BGPD, 0, sizeof(s->io_lcd_BGPD));
    memset(s->io_lcd_OBPD, 0, sizeof(s->io_lcd_OBPD));

    s->io_timer_DIV_cycles = 0x00;
    s->TimerClock  = 0x00;
    s->io_timer_TIMA_cycles = 0x00;
    s->TimerCounter = 0x00;
    s->TimerResetValue = 0x00;
    s->TimerControl = 0x00;

    s->LinkData = 0x00;
    s->LinkControl = 0x00;

    s->GbcInfraredIo = 0x00;

    s->JoypadIo = 0x00;
    s->io_buttons_dirs = 0x0f;
    s->io_buttons_buttons = 0x0f;

    s->AudioChannelSwitch = 0xf1;
    s->AudioSpeakerChannels = 0xf3;
    s->AudioSpeakerControl = 0x77;

    s->AudioChannel1Sweep = 0x80;
    s->AudioChannel1PatternAndLength = 0xbf;
    s->AudioChannel1Envelope = 0xf3;
    s->AudioChannel1FrequencyLow = 0x00;
    s->AudioChannel1Flags = 0xbf;

    s->AudioChannel2PatternAndLength = 0x3f;
    s->AudioChannel2Envelope = 0x00;
    s->AudioChannel2FrequencyLow = 0x00;
    s->AudioChannel2Flags = 0xbf;

    s->AudioChannel3Control = 0x7f;
    s->AudioChannel3Length = 0xff;
    s->AudioChannel3Level = 0x9f;
    s->AudioChannel3FrequencyLow = 0x00;
    s->AudioChannel3Flags = 0xbf;
    memset(s->SoundWaveData, 0, sizeof(s->SoundWaveData));

    s->AudioChannel4Length = 0xff;
    s->AudioChannel4Envelope = 0x00;
    s->AudioChannel4RNGParameters = 0x00;
    s->AudioChannel4Flags = 0xbf;

    s->SRAMBankNumber = 0;
    s->RomBankLower = 1;
    s->GbcRamBankSelectRegister = 1;
    s->GbcVRAMBank = 0;

    s->RomBankUpper = 0;
    s->RomRamSelect = ROM_SELECT;
    s->isSRAMDisabled = true;
}

static void cpu_handle_interrupts(GbState *s) {
    u8 interrupts = s->InterruptSwitch & s->InterruptFlags;

    if (s->interrupts_master_enabled) {
        for (int i = 0; i < 5; i++) {
            if (interrupts & (1 << i)) {
                s->interrupts_master_enabled = 0;
                s->InterruptFlags ^= 1 << i;

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
    u32 freq = s->IsInDoubleSpeedMode ? GB_FREQ : 2 * GB_FREQ;
    u32 div_cycles_per_tick = freq / GB_DIV_FREQ;
    s->io_timer_DIV_cycles += s->last_op_cycles;
    if (s->io_timer_DIV_cycles >= div_cycles_per_tick) {
        s->io_timer_DIV_cycles %= div_cycles_per_tick;
        s->TimerClock++;
    }

    if (s->TimerControl & (1<<2)) { /* Timer enable */
        s->io_timer_TIMA_cycles += s->last_op_cycles;
        u32 timer_hz = GB_TIMA_FREQS[s->TimerControl & 0x3];
        u32 timer_cycles_per_tick = freq / timer_hz;
        if (s->io_timer_TIMA_cycles >= timer_cycles_per_tick) {
            s->io_timer_TIMA_cycles %= timer_cycles_per_tick;
            s->TimerCounter++;
            if (s->TimerCounter == 0) {
                s->TimerCounter = s->TimerCounter;
                s->InterruptFlags |= 1 << 2;
            }
        }
    }
}

void cpu_step(GbState *s) {
    cpu_handle_interrupts(s);

    u32 instruction = mmu_read32(s, s->pc);
    u8 opCode = instruction & 0xFF;

    s->last_op_cycles = cycles_per_instruction[opCode];
    if (opCode == 0xcb) {
        u8 extOp = (instruction >> 8) & 0xFF;
        s->last_op_cycles = cycles_per_instruction_cb[extOp];
    }

    if (!s->halt_for_interrupts) {
        // Move PC forward, then go and run the operation.
        s->pc++;
        opTable[opCode](s, instruction);
    }
}
