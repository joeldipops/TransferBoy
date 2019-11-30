#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libdragon.h>

#include "types.h"
#include "hwdefs.h"
#include "emu.h"
#include "gbc_state.h"
#include "cpu.h"
#include "mmu.h"
#include "lcd.h"
#include "state.h"

/**
 * Initialize the emulator state of the gameboy. This state belongs to the
 * emulator, not the state of the emulated hardware.
 */
void emu_init(GbState *s) {

}

void emu_step(PlayerState* state) {
    GbState* s = &state->EmulationState;
    cpu_step(s);
    lcd_step(state);
    mmu_step(s);
    cpu_timers_step(s);

    s->time_cycles += s->last_op_cycles;
    if (s->time_cycles >= GB_FREQ) {
        s->time_cycles %= GB_FREQ;
        s->time_seconds++;
    }
}

void emu_process_inputs(GbState *s, struct player_input *input) {
#define BTN(type, button, bit) \
    do { \
        if (input->button_ ## button) \
            s->io_buttons_ ## type &= ~(1 << (bit)); \
        else \
            s->io_buttons_ ## type |= 1 << (bit); \
    } while (0)

    BTN(buttons,  start,   3);
    BTN(buttons,  select,  2);
    BTN(buttons,  b,       1);
    BTN(buttons,  a,       0);
    BTN(dirs,     down,    3);
    BTN(dirs,     up,      2);
    BTN(dirs,     left,    1);
    BTN(dirs,     right,   0);

#undef BTN
}
