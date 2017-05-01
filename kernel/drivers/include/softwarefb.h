/*----------------------------------------------------------------------
 * Copyright (C) 2016, 2017 Pedro Falcato
 *
 * This file is part of Onyx, and is made available under
 * the terms of the GNU General Public License version 2.
 *
 * You can redistribute it and/or modify it under the terms of the GNU
 * General Public License version 2 as published by the Free Software
 * Foundation.
 *----------------------------------------------------------------------*/
#ifndef _VESA_H
#define _VESA_H

#include <stdint.h>
#include <multiboot2.h>
#include <kernel/video.h>

void put_pixel(unsigned int x,unsigned int y, int color, void* fb);
void softfb_draw_char(unsigned char c, int x, int y, int fgcolor, int bgcolor, void* fb);
void softfb_init(uintptr_t fb_address, uint32_t bpp, uint32_t width, uint32_t height,uint32_t pitch);
void *softfb_getfb(struct video_device *dev);
void softfb_scroll(void*);
struct video_mode *softfb_getvideomode(struct video_device *dev);
void softfb_draw_cursor(int x, int y, int fgcolor, int bgcolor, void* fb);
#endif
