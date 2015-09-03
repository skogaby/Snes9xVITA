// Author: skogaby
// Lots of examples were followed using other emulators found on github

#include "vita_video.h"

// helpers for rendering
unsigned long curr_frame;
clock_t last_render_time;
int scale;
int pos_x, pos_y;
unsigned short h, w;
vita2d_texture *tex;
void *tex_data;

/***
 * Callback for when a new frame is generated that we need to render.
 */
bool retro_video_refresh_callback(const void *data, unsigned width, unsigned height, size_t pitch)
{
	curr_frame++;

	// initialize our main render texture once we get the dimensions for the first time
	if(!tex)
	{
		printf("Initializing main render texture");

		curr_frame = 0;
		scale = 2;
		tex = vita2d_create_empty_texture_format(width, height, SCE_GXM_TEXTURE_FORMAT_R5G6B5);
		tex_data = vita2d_texture_get_datap(tex);
		pos_x = (SCREEN_W / 2) - (width / 2) * scale;
		pos_y = (SCREEN_H / 2) - (height / 2) * scale;
	}

	// copy the input pixels into the output buffer
	const uint16_t* in_pixels = (const uint16_t*)data;
	uint16_t *out_pixels = (uint16_t *)tex_data;

	for (h = 0; h < height; h++, in_pixels += pitch / 2, out_pixels += width) 
	{
		memcpy(out_pixels, in_pixels, width * sizeof(uint16_t));
	}

    // draw the screen
	vita2d_start_drawing();

	vita2d_draw_texture_scale(tex, pos_x, pos_y, scale, scale);
    show_fps();

	vita2d_end_drawing();
	vita2d_swap_buffers();
	return true;
}

/***
 * Prints the current FPS to the screen.
 */
void show_fps()
{
    clock_t now = clock();
    float curr_fps = 1000000.0F / ((float)now - (float)last_render_time);
    last_render_time = now;

    vita2d_draw_rectangle(10, 10, 128, 16, 0xFF000000);
    font_draw_stringf(10, 10, 0xFFFFFFFF, "FPS: %d", (int)curr_fps);
}

/***
 * Free up memory.
 */
void video_shutdown()
{
    free(tex);
    free(tex_data);
}