#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED
#include "core.h"


static bool IS_DEBUGGING = true;
static const bool VALIDATE_CHECKSUMS = false;
// If this is true, carts above a certain size cannot be loaded.  Should only be false for debugging purposes.
static const bool RESERVE_WORKING_MEMORY = true;
static const bool USE_ANTIALIASING = false;
static const bool HIDE_DISABLED_OPTIONS = true;

//#define FRAMES_TO_SKIP 1
#define SHOW_FRAME_COUNT 1
// Abysmal performance aside, 2 player mode is fundamentally broken until the RSP knows how to handle it.
#define MAX_PLAYERS 1
#define LOCK_CPU_TO_PPU 1
//#define IS_AUDIO_ENABLED 1
//#define IS_SGB_ENABLED 1
//#define IS_PROFILING 1

// Current implementation of double buffering is slow as fuck, so leaving it out for now while 
// I track down other performance bottlenecks.
//#define IS_DOUBLE_BUFFERED


#endif