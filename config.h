#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED
#include "core.h"

static sByte FRAMES_TO_SKIP = 1;
static bool IS_DEBUGGING = true;
static const bool VALIDATE_CHECKSUMS = false;
// If this is true, carts above a certain size cannot be loaded.  Should only be false for debugging purposes.
static const bool RESERVE_WORKING_MEMORY = true;
static const bool SHOW_FRAME_COUNT = true;
static const bool IS_AUDIO_ENABLED = false;
static const bool IS_SGB_ENABLED = false;
static const bool USE_ANTIALIASING = false;
static const bool HIDE_DISABLED_OPTIONS = true;

#define LOCK_CPU_TO_PPU 1

#endif