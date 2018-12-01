#ifndef LCD_H
#define LCD_H

#include "types.h"

int lcd_init(struct gb_state *s);
void lcd_step(struct gb_state *s);

#endif
