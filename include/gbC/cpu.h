#ifndef GBC_H
#define GBC_H

#include "types.h"

// TODO - remove to gbz80ops.
void cpu_do_cb_instruction(GbState* s);

void cpu_init_emu_cpu_state(GbState *s);
void cpu_reset_state(GbState *s);
void cpu_step(GbState *s);
void cpu_timers_step(GbState *s);

#endif
