#ifndef CORE_INCLUDED
#define CORE_INCLUDED

#define null 0
typedef enum { false, true } bool;

#include "include/gbc_bundle.h"
#include <libdragon.h>

typedef unsigned char byte;
typedef char string[1024];
typedef unsigned short natural;

natural GLOBAL_BACKGROUND_COLOUR = 0;
natural GLOBAL_TEXT_COLOUR = 0;
natural BLANK_SCREEN_COLOUR = 0;

typedef struct {
    unsigned long Size;
    byte* Data;
} ByteArray;

typedef enum { Quit, Init, Play, Menu, Options } Mode;
typedef enum { BorderNone } Border;


unsigned long frameCount = 0;

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
unsigned long long getMemoryLimit();

/**
 * Calculates the largest block of memory that can currently be allocated (give or take)
 * @return The estimates amount of remaining memory.
 */
unsigned long long getCurrentMemory();

#endif
