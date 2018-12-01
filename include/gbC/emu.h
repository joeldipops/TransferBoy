#ifndef EMU_H
#define EMU_H

#include "types.h"
#include "player_input.h"

struct emu_args {
    char *rom_filename;
    char *bios_filename;
    char *state_filename;
    char *save_filename;
    char break_at_start;
    char print_disas;
    char print_mmu;
    char audio_enable;
};

void emu_step(struct gb_state *s);
void emu_step_frame(struct gb_state *s);
void emu_process_inputs(struct gb_state *s, struct player_input *input_state);

#endif
