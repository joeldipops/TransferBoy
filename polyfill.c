#include <libdragon.h>

/**
 * There's no threading in libdragon yet, so we're just gonna busy-wait until 
 * that becomes a terrible idea.
 */
unsigned int sleep(unsigned int seconds) {
    wait_ms(seconds * 1000);
    return 0;
}