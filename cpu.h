#ifndef GBC_H
#define GBC_H

#include "types.h"

void cpu_init_emu_cpu_state(GbState *s);
void cpu_reset_state(GbState *s);
void cpu_step(GbState *s);
void cpu_timers_step(GbState *s);

#endif
