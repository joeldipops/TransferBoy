#ifndef CORE_INCLUDED
#define CORE_INCLUDED

#include <libdragon.h>

#define null 0

typedef uint8_t byte;
typedef int8_t sByte;
typedef int16_t sShort;
typedef uint16_t natural;
typedef int32_t sInt;
typedef uint32_t uInt;
typedef uint64_t uLong;

typedef char string[128];

natural GLOBAL_BACKGROUND_COLOUR;
natural GLOBAL_TEXT_COLOUR;
natural SELECTED_MENU_ITEM_COLOUR;
natural SELECTED_OPTIONS_ITEM_COLOUR;
static const uInt AUDIO_SAMPLE_RATE = 44100;

typedef struct {
    uLong Size;
    byte* Data;
} ByteArray;

typedef enum { Quit, Init, Play, Menu, Options } Mode;
typedef enum { BorderNone } Border;

/**
 * Reads a number from a substring, stopping after a certain number of characters.
 * @param start pointer to the start of the number in the string.
 * @param maxLength max length of the number as characters.
 * @param base base of the number eg. 10, 16
 * @return The parsed byte.
 */
byte parseByte(const char* start, const byte maxLength, const byte base);

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

/**
 * There's no threading in libdragon yet, so we're just gonna busy-wait until 
 * that becomes a terrible idea.
 */
unsigned int sleep(unsigned int seconds);

typedef struct {
    void* p;
    void* a;
} AlignedPointer;

/**
 * Allocates memory aligned to a given number of bytes.
 * @param size Size of memory to allocate.
 * @param alignment Number of bytes to align to.
 * @returns struct where p is your aligned pointer.
 */
AlignedPointer malloc_aligned(size_t size, byte alignment);

/**
 * Frees memory allocated with malloc_aligned
 * @param ptr Holds pointer to memory to be freed.
 */
void free_aligned(AlignedPointer ptr);

typedef enum {
    PROFILE_JUNK,           // 0
    PROFILE_C_SCAN,         // 1
    PROFILE_CPU,            // 2
    PROFILE_LCD,            // 3
    PROFILE_MMU,            // 4
    PROFILE_TIMERS,         // 5
    PROFILE_CYCLE_ADJUST,   // 6
    PROFILE_DEVICE,         // 7
    PROFILE_DRAW            // 8
} ProfileSection;
#endif
