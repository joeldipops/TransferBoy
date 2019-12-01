/* fps.c -- fps helpers implementation
 *
 * Copyright (C) 2017 Victor Vieux
 *
 * This software may be modified and distributed under the terms
 * of the Apache license.  See the LICENSE file for details.
 */

#include <libdragon.h>

static volatile bool fps_refresh = false;
static volatile uint8_t fps;

void fps_frame()
{
    static uint8_t frame_count = 0;

    frame_count++;
    if (fps_refresh) {
        fps = frame_count;
        frame_count = 0;
        fps_refresh = false;
    }
}

uint8_t fps_get()
{
    return fps;
}

void fps_timer()
{
    fps_refresh = true;
}