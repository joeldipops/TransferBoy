#ifndef LCD_H
#define LCD_H

#include "state.h"
#include "types.h"

int lcd_init(GbState *s);
void lcd_step(PlayerState* state);

#endif
