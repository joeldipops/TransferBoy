#ifndef CORE_INCLUDED
#define CORE_INCLUDED

#define null 0
typedef enum { false, true } bool;

#include "include/gbc_bundle.h"
#include <libdragon.h>

typedef char string[1024];
typedef unsigned char byte;
typedef unsigned int natural;

const unsigned char VERTICAL_MARGIN = 30;
const unsigned char HORIZONTAL_MARGIN = 30;
const unsigned char VERTICAL_MENU_SPACING = 25;

natural GLOBAL_BACKGROUND_COLOUR = 0;
natural GLOBAL_TEXT_COLOUR = 0;
natural BLANK_SCREEN_COLOUR = 0;

typedef struct {
    unsigned long Size;
    byte* Data;
} ByteArray;

typedef enum { Quit, Init, Play, Menu, Options } Mode;
typedef enum { BorderNone } Border;

#endif
