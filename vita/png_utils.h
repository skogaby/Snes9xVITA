// Borrowed from:
// http://www.lemoda.net/c/write-png/

#ifndef __PNG_UTILS_H__
#define __PNG_UTILS_H__

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* A coloured pixel. */
typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} pixel_t;

/* A picture. */
typedef struct
{
    pixel_t *pixels;
    size_t width;
    size_t height;
} bitmap_t;

static pixel_t* pixel_at(bitmap_t * bitmap, int x, int y);
static int save_png_to_file(bitmap_t *bitmap, const char *path);
static int pix(int value, int max);


#endif