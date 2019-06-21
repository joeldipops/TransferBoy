#include <libdragon.h>

/**
 * There's no threading in libdragon yet, so we're just gonna busy-wait until 
 * that becomes a terrible idea.
 */
unsigned int sleep(unsigned int seconds) {
    unsigned long start = get_ticks_ms();
    unsigned long end = start + (seconds * 1000);
    while (get_ticks_ms() < end);

    return 0;
}