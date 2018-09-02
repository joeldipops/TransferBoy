#ifndef LOGGER_INCLUDED
#define LOGGER_INCLUDED
#include "utils.h"

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

#endif
