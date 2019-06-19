#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "hwdefs.h"
#include "emu.h"
#include "state.h"
#include "cpu.h"
#include "mmu.h"
#include "lcd.h"
#include "../../state.h"

#define emu_error(fmt, ...) \
    do { \
        printf("Emu initialization error: " fmt "\n", ##__VA_ARGS__); \
        return 1; \
    } while (0)

/**
 * Initialize the emulator state of the gameboy. This state belongs to the
 * emulator, not the state of the emulated hardware.
 */
void emu_init(GbState *s) {
    s->emu_state = calloc(1, sizeof(struct emu_state));
}    

void emu_step(PlayerState* state) {
    GbState* s = &state->EmulationState;
    cpu_step(s);
    lcd_step(state);
    mmu_step(s);
    cpu_timers_step(s);

    s->emu_state->time_cycles += s->emu_state->last_op_cycles;
    if (s->emu_state->time_cycles >= GB_FREQ) {
        s->emu_state->time_cycles %= GB_FREQ;
        s->emu_state->time_seconds++;
    }

    if (s->emu_state->flush_extram) {
        s->emu_state->flush_extram = 0;
        if (s->emu_state->extram_dirty) {
            ; //Handled by play.c wrapper.
        }
        s->emu_state->extram_dirty = 0;
    }
}

void emu_step_frame(PlayerState* state) {
    do {
        emu_step(state);
    } while (!state->EmulationState.emu_state->lcd_entered_vblank);

    // Save periodically (once per frame) if dirty.
    state->EmulationState.emu_state->flush_extram = 1;

}

void emu_process_inputs(GbState *s, struct player_input *input) {
    if (input->special_quit)
        s->emu_state->quit = 1;

    if (input->special_dbgbreak)
        s->emu_state->dbg_break_next = 1;

    if (input->special_savestate)
        s->emu_state->make_savestate = 1;

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
