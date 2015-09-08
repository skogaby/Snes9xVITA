// Author: skogaby
// Lots of examples were followed using other emulators found on github

#ifndef __VITA_VIDEO_H__
#define __VITA_VIDEO_H__

#define SCREEN_W 960
#define SCREEN_H 544

#include <stdint.h>
#include <stdlib.h>

#include <psp2/display.h>
#include <psp2/gxm.h>

#include <vita2d.h>
#include <psplib/video.h>
#include <psplib/pl_perf.h>

#include "vita_menu.h"
#include "../libretro/libretro.h"

extern unsigned long curr_frame;
extern float curr_fps;
extern float scale_x, scale_y;
extern vita2d_texture *tex;

bool retro_video_refresh_callback(const void *data, unsigned width, unsigned height, size_t pitch);
void show_fps();
void video_shutdown();

#endif