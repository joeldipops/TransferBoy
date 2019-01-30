#include <libdragon.h>
#include "resources.h"
#include <math.h>

static sprite_t* _textMap = 0;
static sprite_t* _spriteSheet = 0;
static bool resourcesInitted = false;

typedef struct {
    byte SheetIndex;
    byte SpriteCode;
    sByte Transformation;
    sprite_t* Sprite;
    byte Position;
} SpriteLookup;

#define TRANSFORM_CACHE_SIZE 8
static SpriteLookup _transformCache[TRANSFORM_CACHE_SIZE] = {};

/**
 * Gets the sprite from the cache if it exists.
 * @param sheetIndex Identifies the sprite sheet the sprite is in.
 * @param spriteCode Identifies the source sprite on the sheet.
 * @param transformation How the source sprite was tranformed.
 * @return pointer to sprite in the cache, or null if not in the cache.
 * @private
 */
sprite_t* getFromCache(const byte sheetIndex, const byte spriteCode, const Transformation transformation) {
    sprite_t* result = null;
    sByte oldPosition = -1;
    for (byte i = 0; i < TRANSFORM_CACHE_SIZE; i++) {
        if (
            sheetIndex == _transformCache[i].SheetIndex
            && spriteCode == _transformCache[i].SpriteCode
            && transformation == _transformCache[i].Transformation
        ) {
            oldPosition = _transformCache[i].Position;
            _transformCache[i].Position = TRANSFORM_CACHE_SIZE;
            result = _transformCache[i].Sprite;
        }
    }

    // If the sprite was in the cache, put it back on the end of the queue, and push everything that was behind it down one.
    if (result) {
        for (byte i = 0; i < TRANSFORM_CACHE_SIZE; i++) {
            if (_transformCache[i].Position > oldPosition) {
                _transformCache[i].Position--;
            }
        }
    }

    return result;
}

/**
 * Adds a sprite to the cache.
 * @param lookup The sprite along with information used to index/identify it.
 * @private
 */
void cacheSprite(SpriteLookup* lookup) {
    byte cacheTop = 0;

    // Find any slot with position 0
    while(_transformCache[cacheTop].Position) {
        cacheTop++;
    }

    // If the cache is full, free up some memory
    if (_transformCache[cacheTop].Sprite) {
        free(_transformCache[cacheTop].Sprite->data);
        free(_transformCache[cacheTop].Sprite);
        _transformCache[cacheTop].Sprite = null;
    }

    // Add to the cache.
    _transformCache[cacheTop].Sprite = lookup->Sprite;
    _transformCache[cacheTop].SheetIndex = lookup->SheetIndex;
    _transformCache[cacheTop].SpriteCode = lookup->SpriteCode;
    _transformCache[cacheTop].Transformation = lookup->Transformation;
    _transformCache[cacheTop].Position = TRANSFORM_CACHE_SIZE;

    // Shift everything down by 1 so as things are added they get closer to 0.
    for (byte i = 0; i < TRANSFORM_CACHE_SIZE; i++) {
        if (_transformCache[i].Position > 0) {
            _transformCache[i].Position--;
        }
    }
}

/**
 * Loads oft-used resources in to memory.
 * @return success/error code
 ** 0   initted successfully
 ** -1  could not find expected resource file.
 */
sByte initResources() {
    if (resourcesInitted) {
        return 0;
    }

    // Read in character sprite sheet.
    sInt textMapPointer = dfs_open("/textMap.sprite");
    if (!textMapPointer) {
        return -1;
    }
    _textMap = malloc(dfs_size(textMapPointer));
    dfs_read(_textMap, 1, dfs_size(textMapPointer), textMapPointer);
    dfs_close(textMapPointer);

    // Read in other sprites.
    sInt spriteSheetPointer = dfs_open("/spriteSheet.sprite");
    if (!spriteSheetPointer) {
        return -1;
    }
    _spriteSheet = malloc(dfs_size(spriteSheetPointer));
    dfs_read(_spriteSheet, 1, dfs_size(spriteSheetPointer), spriteSheetPointer);
    dfs_close(spriteSheetPointer);

    for (byte i = 0; i < TRANSFORM_CACHE_SIZE; i++) {
        _transformCache[i].Sprite = 0;
    }

    resourcesInitted = true;
    return 0;
}

/**
 * Loads a ROM stored in the n64 filesystem rather than from the TPAK
 * @out output data for the rom goes here.
 * @returns success/error code
 ** 0   loaded successfully
 ** -1  file not found.
 */
sByte loadInternalRom(ByteArray* output) {
    sInt pointer = dfs_open("/superTestBoy.gb");
    if (!pointer) {
        return -1;
    }

    output->Size = dfs_size(pointer);
    output->Data = malloc(output->Size);
    dfs_read(output->Data, 1, output->Size,pointer);
    dfs_close(pointer);

    return 0;
}

/**
 * Gets sprite sheet of textual characters.
 * @return pointer to sprite sheet.
 */
sprite_t* getCharacterSheet() {
    return _textMap;
}

/**
 * Gets sprite sheet of icons and textures.
 * @return pointer to sprite sheet.
 */
sprite_t* getSpriteSheet() {
    return _spriteSheet;
}

/**
 * Takes an existing sprite, transforms it, and stashes it in a cache for next time it's needed.
 * @param sheet Source sprite sheet.
 * @param spriteCode Identifies source sprite on the sheet.
 * @param transformation How the sprite should be transformed.
 * @return Pointer to the new transformed sprite.
 */
sprite_t* transformSprite(const sprite_t* sheet, const byte spriteCode, const Transformation transformation) {
    byte sheetIndex = (sheet == _textMap) ? 0 : 1;

    // Get from cache if we already have it.
    sprite_t* result = getFromCache(sheetIndex, spriteCode, transformation);
    if (result) {
        return result;
    }

    natural spriteWidth = sheet->width / sheet->hslices;
    natural spriteHeight = sheet->height / sheet->vslices;

    byte* data = calloc(spriteHeight * spriteWidth, sheet->bitdepth);

    byte x = spriteCode % sheet->hslices;
    byte y = floor(spriteCode / sheet->hslices);

    byte* source = calloc(sheet->height * sheet->width, sheet->bitdepth);
    memcpy(source, sheet->data, sheet->height * sheet->width * sheet->bitdepth);

    natural index = 0;
    natural destRow = 0;
    natural destColumn = 0;

    switch(transformation) {
        case ROTATE_90:
            destRow = spriteHeight * sheet->bitdepth - sheet->bitdepth;
            for (natural sourceRow = y * spriteHeight; sourceRow < y * spriteHeight + spriteHeight; sourceRow++) {
                destColumn = 0;
                for (natural sourceColumn = x * spriteWidth; sourceColumn < x * spriteWidth + spriteWidth; sourceColumn++) {
                    memcpy(
                        data + destColumn * spriteWidth + destRow,
                        source + (sourceRow * sheet->width + sourceColumn) * sheet->bitdepth,
                        sheet->bitdepth
                    );

                    destColumn += sheet->bitdepth;
                }
                destRow -= sheet->bitdepth;
            }
        break;
        case ROTATE_180:
            destRow = spriteHeight * sheet->bitdepth - sheet->bitdepth;
            for (natural sourceRow = y * spriteHeight; sourceRow < y * spriteHeight + spriteHeight; sourceRow++) {
                destColumn = spriteWidth * sheet->bitdepth - sheet->bitdepth;
                for (natural sourceColumn = x * spriteWidth; sourceColumn < x * spriteWidth + spriteWidth; sourceColumn++) {
                    memcpy(
                        data + destRow * spriteWidth + destColumn,
                        source + (sourceRow * sheet->width + sourceColumn) * sheet->bitdepth,
                        sheet->bitdepth
                    );
                    destColumn -= sheet->bitdepth;
                }
                destRow -= sheet->bitdepth;
            }
        break;
        case ROTATE_270:
            destRow = 0;
            for (natural sourceRow = y * spriteHeight; sourceRow < y * spriteHeight + spriteHeight; sourceRow++) {
                destColumn = spriteWidth * sheet->bitdepth - sheet->bitdepth;
                for (natural sourceColumn = x * spriteWidth; sourceColumn < x * spriteWidth + spriteWidth; sourceColumn++) {
                    memcpy(
                        data + destColumn * spriteWidth + destRow,
                        source + (sourceRow * sheet->width + sourceColumn) * sheet->bitdepth,
                        sheet->bitdepth
                    );
                    destColumn -= sheet->bitdepth;
                }
                destRow += sheet->bitdepth;
            }
        break;
        case FLIP_HORIZONTAL:
            destRow = 0;
            for (natural sourceRow = y * spriteHeight; sourceRow < y * spriteHeight + spriteHeight; sourceRow++) {
                destColumn = spriteWidth * sheet->bitdepth - sheet->bitdepth;
                for (natural sourceColumn = x * spriteWidth; sourceColumn < x * spriteWidth + spriteWidth; sourceColumn++) {
                    memcpy(
                        data + destRow * spriteWidth + destColumn,
                        source + (sourceRow * sheet->width + sourceColumn) * sheet->bitdepth,
                        sheet->bitdepth
                    );

                    destColumn -= sheet->bitdepth;
                }
                destRow += sheet->bitdepth;
            }
        break;
        case FLIP_VERTICAL:
            destRow = spriteHeight * sheet->bitdepth - sheet->bitdepth;
            for (natural sourceRow = y * spriteHeight; sourceRow < y * spriteHeight + spriteHeight; sourceRow++) {
                destColumn = 0;
                for (natural sourceColumn = x * spriteWidth; sourceColumn < x * spriteWidth + spriteWidth; sourceColumn++) {
                    memcpy(
                        data + destRow * spriteWidth + destColumn,
                        source + (sourceRow * sheet->width + sourceColumn) * sheet->bitdepth,
                        sheet->bitdepth
                    );

                    destColumn += sheet->bitdepth;
                }
                destRow -= sheet->bitdepth;
            }
        break;
        default:
            // Straight copy
            for (natural row = y * spriteHeight; row < y * spriteHeight + spriteHeight; row++) {
                for (natural column = x * spriteWidth; column < x * spriteWidth + spriteWidth; column++) {
                    memcpy(
                        data + index,
                        source + (row * sheet->width + column) * sheet->bitdepth,
                        sheet->bitdepth
                    );

                    index += sheet->bitdepth;
                }
            }
    }

    result = calloc(1, sizeof(sprite_t) + spriteHeight * spriteWidth * sheet->bitdepth);
    result->width = spriteWidth;
    result->height = spriteHeight;
    result->vslices = 1;
    result->hslices = 1;
    result->bitdepth = sheet->bitdepth;
    result->format = sheet->format;
    memcpy(result->data, data, result->width * result->height * result->bitdepth);

    // Put the newly generated sprite in to the cache.
    SpriteLookup cacheable;
    cacheable.Sprite = result;
    cacheable.SheetIndex = sheetIndex;
    cacheable.SpriteCode = spriteCode;
    cacheable.Transformation = transformation;
    cacheSprite(&cacheable);

    free(source);
    source = null;
    free(data);
    data = null;

    return result;
}

/**
 * Releases memory held by all resources. only rotate
 */
void freeResources() {
    free(_textMap);
    _textMap = null;
    free(_spriteSheet);
    _spriteSheet = null;

    emptyResourceCache();
    resourcesInitted = false;
}

/**
 * Frees up the cache but leaves the resources subsystem initialised.
 */
void emptyResourceCache() {
    for (byte i = 0; i < TRANSFORM_CACHE_SIZE; i++) {
        if (_transformCache[i].Sprite) {
            free(_transformCache[i].Sprite->data);
            free(_transformCache[i].Sprite);
            _transformCache[i].Sprite = null;
        }
    }
}
