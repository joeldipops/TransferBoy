#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "state.h"
#include "logger.h"
#include <libdragon.h>

void startDebugging() {
    IsDebugging = true;
}

void stopDebugging() {
    IsDebugging = false;
}

#ifdef IS_DEBUGGING

static long offset = 0;

// This is SRAM, which is only 0x8000 bytes. I know I can write to the entire cart-space, but I don't know if CEN64 can give me the related log file.
#define LOGFILE = 0xA8000000;

/**
 * If in debugging mode, print gb registers and message then wait for input.
 */
void debug(GbState *s, string message) {
    if (IsDebugging) {
        u32 length = strlen(message);
        dma_write(message, 0xA8000000 + offset, length);
        offset += length;
    }
}

#endif

#ifdef IS_PROFILING

#define MAX_ENTRIES 64

static ProfileEntry entries[MAX_ENTRIES];
static long long lastTime = 0;

/**
 * Reset all profile information back to 0 to start a new profiling session.
 */
void resetProfile() {
    memset(&entries, 0, sizeof(entries));
    lastTime = timer_ticks();
}

/**
 * Update profile data for a given profile section.
 * @param id Identifies the profile section.
 */
void updateProfile(int id) {
    long long now = timer_ticks();
    entries[id].sum += now - lastTime;
    entries[id].count++;
    lastTime = now;
}

/**
 * Prints out all available profile data.
 */
void displayProfile() {
    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (entries[i].count) {
            long avg = entries[i].sum / entries[i].count;
            string line = "";
            sprintf(line, "%d: %lu ticks", i, avg);
            graphics_draw_text(1, 30, i * 20, line);
            graphics_draw_text(2, 30, i * 20, line);
        }
    }
    while(true);
}
#endif