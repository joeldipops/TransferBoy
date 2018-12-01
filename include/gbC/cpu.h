#ifndef GBC_H
#define GBC_H

#include "types.h"

void cpu_init_emu_cpu_state(struct gb_state *s);
void cpu_reset_state(struct gb_state *s);
void cpu_step(struct gb_state *s);
void cpu_timers_step(struct gb_state *s);

#endif
