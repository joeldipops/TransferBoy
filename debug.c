#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "state.h"
#include <libdragon.h>

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