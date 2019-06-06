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

#define emu_error(fmt, ...) \
    do { \
        printf("Emu initialization error: " fmt "\n", ##__VA_ARGS__); \
        return 1; \
    } while (0)

void emu_step(GbState *s) {
    cpu_step(s);
    lcd_step(s);
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

void emu_step_frame(GbState *s) {
    do {
        emu_step(s);
    } while (!s->emu_state->lcd_entered_vblank);

    /* Save periodically (once per frame) if dirty. */
    s->emu_state->flush_extram = 1;

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
