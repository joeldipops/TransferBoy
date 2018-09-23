#ifndef CORE_INCLUDED
#define CORE_INCLUDED

#define null 0
typedef enum { false, true } bool;

#include "include/gbc_bundle.h"
#include <libdragon.h>

typedef uint8_t byte;
typedef int8_t sByte;
typedef int16_t sShort;
typedef uint16_t natural;
typedef int32_t sInt;
typedef uint32_t uInt;
typedef uint64_t uLong;


typedef char string[128];

natural GLOBAL_BACKGROUND_COLOUR = 0;
natural GLOBAL_TEXT_COLOUR = 0;
natural BLANK_SCREEN_COLOUR = 0;
natural SELECTED_MENU_ITEM_COLOUR = 0;

typedef struct {
    uLong Size;
    byte* Data;
} ByteArray;

typedef enum { Quit, Init, Play, Menu, Options } Mode;
typedef enum { BorderNone } Border;

uLong frameCount = 0;

/**
 * Cleans up memory held by a ByteArray and resets the values.
 * @param arr The array.
 */
void freeByteArray(ByteArray* arr);

/**
 * Calculates the largest block of memory that can be allocated (give or take) at start up
 * to understand how much memory is available overall
 * @return The estimated size of the memory space.
 */
uLong getMemoryLimit();

/**
 * Calculates the largest block of memory that can currently be allocated (give or take)
 * @return The estimates amount of remaining memory.
 */
uLong getCurrentMemory();

#endif
