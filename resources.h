#ifndef RESOURCES_INCLUDED
#define RESOURCES_INCLUDED
#include <libdragon.h>



const byte GB_START_SPRITE = 0;
const byte GB_SELECT_SPRITE = 1;
const byte N64_START_SPRITE = 2;
const byte MENU_SPRITE = 3;
const byte N64_L_SPRITE = 4;
const byte N64_R_SPRITE = 5;
const byte N64_Z_SPRITE = 6;
const byte N64_C_SPRITE = 7;
const byte BLUE_BG_TEXTURE = 8;
const byte CREAM_BG_TEXTURE = 9;
const byte GB_BG_TEXTURE = 10;
const byte ERROR_SPRITE = 11;

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
 * Gets sprite sheet of textual characters.
 * @return pointer to sprite sheet.
 */
sprite_t* getCharacterSheet();

/**
 * Gets sprite sheet of icons and textures.
 * @return pointer to sprite sheet.
 */
sprite_t* getSpriteSpreet();
#endif
