#include "core.h"

#include <libdragon.h>
#include <stdlib.h>
#include <string.h>

uLong totalEstimatedMemory = 0;

/**
 * Calculates the largest block of memory that can be allocated (give or take) at start up
 * to understand how much memory is available overall
 * @return The estimated size of the memory space.
 */
uLong getMemoryLimit() {
    if (!totalEstimatedMemory) {
        totalEstimatedMemory = getCurrentMemory();
    }
    return totalEstimatedMemory;
}

/**
 * Calculates the largest block of memory that can currently be allocated (give or take)
 * @return The estimates amount of remaining memory.
 */
uLong getCurrentMemory() {
    uLong limit = 16000000;

    void* freeMe = 0;
    while (!(freeMe = malloc(limit))) {
        limit /= 1.01;
    }
    free(freeMe);

    return limit;
}

/**
 * Cleans up memory held by a ByteArray and resets the values.
 * @param arr The array.
 */
void freeByteArray(ByteArray* arr) {
    free(arr->Data);
    arr->Data = 0;
    arr->Size = 0;
}

/**
 * Reads a number from a substring, stopping after a certain number of characters.
 * @param start pointer to the start of the number in the string.
 * @param maxLength max length of the number as characters.
 * @param base base of the number eg. 10, 16
 * @return The parsed byte.
 */
byte parseByte(const char* start, const byte maxLength, const byte base) {
    // sprite is 2 digit hex, we need to parse it from the string.
    string code = {};
    memcpy(code, start, maxLength);
    char* end = 0;
    return strtol(code, &end, base);
}

/**
 * Allocates memory aligned to a given number of bytes.
 * @param size Size of memory to allocate.
 * @param alignment Number of bytes to align to.
 * @returns struct where p is your aligned pointer.
 */
AlignedPointer malloc_aligned(size_t size, byte alignment) {
    byte offset = alignment - 1;
    AlignedPointer result;
    result.a = malloc(size + offset);
    result.p = (void*)(((uintptr_t)result.a + offset) & ~(uintptr_t)offset);

    return result;
}

/**
 * Frees memory allocated with malloc_aligned
 * @param ptr Holds pointer to memory to be freed.
 */
void free_aligned(AlignedPointer ptr) {
    free(ptr.a);
}
