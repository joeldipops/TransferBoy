#ifndef LOGGER_INCLUDED
#define LOGGER_INCLUDED
#include "include/gbc_bundle.h"

/**
 * Displays the current state of the Gameboy emulator registers.
 * @param s The emulator state structure.
 */
void printRegisters(GbState *s);

/**
 * Throws a line of text up on to the screen a la printf.
 * @param text The text or format string.
 * @param ... Parameters for the format string.
 */
void logInfo(const string text, ... );

/**
 * Throws a line of text up on to the screen a la printf and then stalls execution so you can read it.
 * TODO: Unpause
 * @param text The text or format string.
 * @param ... Parameters for the format string.
 */
void logAndPause(const string text, ...);

/**
 * Shows the log message if we're already in the middle of a frame.
 * @param frame The frame.
 * @param text Text or format string to show.
 * @param ... Parameters for the format string.
 */
void logAndPauseFrame(display_context_t frame, const string text, ...);

/**
 * Print the next 32 bytes of memory from a given address and wait for "start".
 * @param start the starting memory address.
 */
void printSegment(byte* start);

#endif
