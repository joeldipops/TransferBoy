#ifndef DEBUG_INCLUDED
#define DEBUG_INCLUDED

#include "config.h"
#include "state.h"

void startDebugging();
void stopDebugging();
void isDebugging();

/**
 * Print current instruction for debugging.
 */
void debug(string message, ...);

#ifdef IS_PROFILING

    #include <stdio.h>

    #define UPDATE_PROFILE(id) updateProfile(id);
    #define INIT_PROFILE resetProfile();
    #define DISPLAY_PROFILE displayProfile();
    typedef struct {
        int id;
        long sum;
        long count;
    } ProfileEntry;

    /**
     * Update profile data for a given profile section.
     * @param id Identifies the profile section.
     */
    void updateProfile(int id);

    /**
     * Reset all profile information back to 0 to start a new profiling session.
     */
    void resetProfile();

    /**
     * Prints out all available profile data.
     */
    void displayProfile();

    #else
        #define UPDATE_PROFILE(id)
        #define INIT_PROFILE
        #define DISPLAY_PROFILE
    #endif
#endif