#ifndef GBC_H
#define GBC_H

#include "types.h"

void cpu_init_emu_cpu_state(GbState *s);
void cpu_reset_state(GbState *s);
void cpu_step(GbState *s);
void cpu_handle_interrupts(GbState *s);

/**
 * Updates the timer registers and sets the timer interrupt when in GBC double speed mode.
 */
void timersStepDouble(GbState* s);

/**
 * Updates the timer registers and sets the timer interrupt.
 */
void timersStep(GbState* s);

/**
 * Called the cycle after the timer overflows. Sets the interrupt flag to actually trigger.
 * Then continues with the timer step as normal.
 */
void timersStepPendingInterrupt(GbState* s);

#endif
