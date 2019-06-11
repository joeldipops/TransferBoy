#ifndef LCD_H
#define LCD_H

#include "../../state.h"
#include "types.h"

int lcd_init(struct gb_state *s);
void lcd_step(PlayerState* state);

#endif
