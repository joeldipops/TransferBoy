#ifndef CONSTANTS_INCLUDED
#define CONSTANTS_INCLUDED

#define null 0
typedef enum { false, true } bool;

typedef unsigned int natural;
typedef char string[1024];
typedef unsigned char byte;
typedef unsigned int natural;
typedef enum { NoButton, A, B, L, R, Z, Start, Up, Down, Left, Right, CUp, CDown, CLeft, CRight, Stick } N64Button;
typedef enum { GbNoButton, GbA, GbB, GbUp, GbDown, GbLeft, GbRight, GbStart, GbSelect, GbSystemMenu } GbButton;
const natural VERTICAL_MARGIN = 30;
const natural HORIZONTAL_MARGIN = 30;
const natural VERTICAL_MENU_SPACING = 25;
natural GLOBAL_BACKGROUND_COLOUR = 0;
natural GLOBAL_TEXT_COLOUR = 0;

typedef struct {
    unsigned long Size;
    byte* Data;
} ByteArray;

#include <libdragon.h>

void getPressedButtons(const struct controller_data* input, const char controllerNumber, bool* output);

#endif
