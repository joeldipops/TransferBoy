#include "core.h"

static uLong totalEstimatedMemory = 0;
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
