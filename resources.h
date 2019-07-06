#ifndef RESOURCES_INCLUDED
#define RESOURCES_INCLUDED
#include <libdragon.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"

static const byte GB_START_SPRITE = 0;
static const byte GB_SELECT_SPRITE = 1;
static const byte N64_START_SPRITE = 2;
static const byte MENU_SPRITE = 3;
static const byte N64_L_SPRITE = 4;
static const byte N64_R_SPRITE = 5;
static const byte N64_Z_SPRITE = 6;
static const byte N64_C_SPRITE = 7;
static const byte BLUE_BG_TEXTURE = 8;
static const byte CREAM_BG_TEXTURE = 9;
static const byte GB_BG_TEXTURE = 0x0A;
static const byte ERROR_SPRITE = 0x0B;

typedef enum {
    ROTATE_90 = '>',
    ROTATE_180 = 'v',
    ROTATE_270 = '<',
    FLIP_HORIZONTAL = 'W',
    FLIP_VERTICAL = 'V',
    FADE = '~'
} Transformation;


/**
 * Loads oft-used resources in to memory.
 * @return success/error code
 ** 0   initted successfully
 ** -1  could not find expected resource file.
 */
sByte initResources();

/**
 * Releases memory held by all resources.
 */
void freeResources();

/**
 * Frees up the cache but leaves the resources subsystem initialised.
 */
void emptyResourceCache();

/**
 * Gets sprite sheet of textual characters.
 * @return pointer to sprite sheet.
 */
sprite_t* getCharacterSheet();

/**
 * Gets sprite sheet of icons and textures.
 * @return pointer to sprite sheet.
 */
sprite_t* getSpriteSheet();



/**
 * Loads a ROM stored in the n64 filesystem rather than from the TPAK
 * @out output data for the rom goes here.
 * @returns success/error code
 ** 0   loaded successfully
 ** -1  file not found.
 */
sByte loadInternalRom(ByteArray* output);

/**
 * Takes an existing sprite, transforms it, and stashes it in a cache for next time it's needed.
 * @param sheet Source sprite sheet.
 * @param spriteCode Identifies source sprite on the sheet.
 * @param rotation How the sprite should be transformed.
 * @return Pointer to the new transformed sprite.
 */
sprite_t* transformSprite(const sprite_t* sheet, const byte spriteCode, const Transformation transformation);

#endif
