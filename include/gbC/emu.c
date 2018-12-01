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
//#include "audio.h"
//#include "disassembler.h"
//#include "debugger.h"
//#include "gui.h"
//#include "fileio.h"

#define emu_error(fmt, ...) \
    do { \
        printf("Emu initialization error: " fmt "\n", ##__VA_ARGS__); \
        return 1; \
    } while (0)

/*
void emu_save(struct gb_state *s, char extram, char *out_filename) {
    u8 *state_buf;
    size_t state_buf_size;

    if (extram && !s->has_extram)
        return;

    if (extram)
        state_save_extram(s, &state_buf, &state_buf_size);
    else
        state_save(s, &state_buf, &state_buf_size);

    save_file(out_filename, state_buf, state_buf_size);

    printf("%s saved to \"%s\".\n", extram ? "Ext RAM" : "State", out_filename);
}*/

/*
int emu_init(struct gb_state *s, struct emu_args *args) {
    memset(s, 0, sizeof(struct gb_state));


    if (!args->rom_filename)
        emu_error("Must specify ROM filename");
    if (strlen(args->rom_filename) >
            sizeof(s->emu_state->state_filename_out) - 6)
        emu_error("ROM filename too long (%s)", args->rom_filename);

    if (args->state_filename) {
        printf("Loading savestate from \"%s\" ...\n", args->state_filename);
        u8 *state_buf;
        size_t state_buf_size;
        if (read_file(args->state_filename, &state_buf, &state_buf_size))
            emu_error("Error during reading of state file \"%s\".\n",
                    args->state_filename);

        if (state_load(s, state_buf, state_buf_size))
            emu_error("Error during loading of state, aborting.\n");

        print_rom_header_info(s->mem_ROM);

    } else {
        u8 *rom;
        size_t rom_size;
        printf("Loading ROM \"%s\"\n", args->rom_filename);
        if (read_file(args->rom_filename, &rom, &rom_size))
            emu_error("Error during reading of ROM file \"%s\".\n",
                    args->rom_filename);

        print_rom_header_info(rom);

        if (state_new_from_rom(s, rom, rom_size))
            emu_error("Error loading ROM \"%s\", aborting.\n",
                    args->rom_filename);

        cpu_reset_state(s);

        if (args->bios_filename) {
            u8 *bios;
            size_t bios_size;
            read_file(args->bios_filename, &bios, &bios_size);
            state_add_bios(s, bios, bios_size);
        }

        if (args->save_filename) {
            u8 *state_buf;
            size_t state_buf_size;
            if (read_file(args->save_filename, &state_buf, &state_buf_size))
                emu_error("Error during reading of save file \"%s\".",
                        args->save_filename);

            if (state_load_extram(s, state_buf, state_buf_size))
                emu_error("Error during loading of save, aborting.\n");
        } else {
            char savname[1024];
            snprintf(savname, sizeof(savname), "%ssav", args->rom_filename);
            u8 *state_buf;
            size_t state_buf_size;
            if (read_file(savname, &state_buf, &state_buf_size) == 0)
                if (state_load_extram(s, state_buf, state_buf_size))
                    emu_error("Error during loading of save.\n");
        }
    }
    init_emu_state(s);
    cpu_init_emu_cpu_state(s);

    snprintf(s->emu_state->save_filename_out,
            sizeof(s->emu_state->save_filename_out), "%ssav",
            args->rom_filename);
    snprintf(s->emu_state->state_filename_out,
            sizeof(s->emu_state->state_filename_out), "%sstate",
            args->rom_filename);

    if (lcd_init(s))
        emu_error("Couldn't initialize LCD");

    if (args->audio_enable) {
        if (audio_init(s))
            emu_error("Couldn't initialize audio");
    }

    if (args->break_at_start)
        s->emu_state->dbg_break_next = 1;
    if (args->print_disas)
        s->emu_state->dbg_print_disas = 1;
    if (args->print_mmu)
        s->emu_state->dbg_print_mmu = 1;
    if (args->audio_enable)
        s->emu_state->audio_enable = 1;
    return 0;
}
*/
void emu_step(struct gb_state *s) {
    /*
    if (s->emu_state->dbg_print_disas)
        disassemble(s);
    */
    if (s->emu_state->dbg_break_next ||
        s->pc == s->emu_state->dbg_breakpoint)
        /*
        if (dbg_run_debugger(s)) {
            s->emu_state->quit = 1;
            return;
        }*/

    cpu_step(s);
    lcd_step(s);
    mmu_step(s);
    cpu_timers_step(s);

    s->emu_state->time_cycles += s->emu_state->last_op_cycles;
    if (s->emu_state->time_cycles >= GB_FREQ) {
        s->emu_state->time_cycles %= GB_FREQ;
        s->emu_state->time_seconds++;
    }


    if (s->emu_state->make_savestate) {
        s->emu_state->make_savestate = 0;
        //emu_save(s, 0, s->emu_state->state_filename_out);
    }

    if (s->emu_state->flush_extram) {
        s->emu_state->flush_extram = 0;
        if (s->emu_state->extram_dirty)
            ;//emu_save(s, 1, s->emu_state->save_filename_out);
        s->emu_state->extram_dirty = 0;
    }
}

void emu_step_frame(struct gb_state *s) {
    do {
        emu_step(s);
    } while (!s->emu_state->lcd_entered_vblank);

    /* Save periodically (once per frame) if dirty. */
    s->emu_state->flush_extram = 1;

}

void emu_process_inputs(struct gb_state *s, struct player_input *input) {
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
