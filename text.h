#ifndef TEXT_INCLUDED
#define TEXT_INCLUDED

#include "core.h"
typedef enum {
    TextEmpty,
    TextNoTpak,
    TextNoCartridge,
    TextLoadCartridge,
    TextLoadingCartridge,
    TextEnd
} TextId;

/**
 * Gets the text string with the given id code, and loads it into output.
 * @param textId The text id code.
 * @out output the destination string.
 */
void getText(TextId textId, string output);

#endif
