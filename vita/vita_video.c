// Author: skogaby
// Lots of examples were followed using other emulators found on github

#include "vita_video.h"

// helpers for rendering
unsigned long curr_frame;
float curr_fps;
float scale_x, scale_y;
int pos_x, pos_y;
unsigned short h, w;
vita2d_texture *tex;
void *tex_data;
PspImage *Screen;

/***
 * Callback for when a new frame is generated that we need to render.
 */
bool retro_video_refresh_callback(const void *data, unsigned width, unsigned height, size_t pitch)
{
	curr_frame++;

    // initialize our render variables if they're uninitalized, or
    // if they've changed due to user action
	if(!tex || OptionsChanged)
	{
        OptionsChanged = false;

		// handle changes to scale from the options
        switch (Options.DisplayMode)
        {
        case DISPLAY_MODE_UNSCALED:
            scale_x = 1.0f;
            scale_y = 1.0f;
            break;
        case DISPLAY_MODE_2X:
            scale_x = 2.0f;
            scale_y = 2.0f;
            break;
        case DISPLAY_MODE_FIT_HEIGHT:
            scale_y = (float)SCREEN_H / (float)height;
            scale_x = scale_y;
            break;
        case DISPLAY_MODE_FILL_SCREEN:
            scale_x = (float)SCREEN_W / (float)width;
            scale_y = (float)SCREEN_H / (float)height;
            break;
        }

		curr_frame = 0;
        curr_fps = 0.0f;
		pos_x = (SCREEN_W / 2) - (width / 2) * scale_x;
		pos_y = (SCREEN_H / 2) - (height / 2) * scale_y;
	}

    // initialize our main render texture once we get the dimensions for the first time
    if (!tex)
    {
        tex = vita2d_create_empty_texture_format(width, height, SCE_GXM_TEXTURE_FORMAT_R5G6B5);
        tex_data = vita2d_texture_get_datap(tex);

        // initialize PSPImage
        if (Screen)
            free(Screen);
        int size = width * height * 2;

        Screen = (PspImage*)malloc(sizeof(PspImage));

        Screen->TextureFormat = SCE_GXM_TEXTURE_FORMAT_R5G6B5;
        Screen->PalSize = (unsigned short)0;
        memset(tex_data, 0, size);

        Screen->Width = width;
        Screen->Height = height;
        Screen->Pixels = tex_data;
        Screen->Texture = tex;

        Screen->Viewport.X = 0;
        Screen->Viewport.Y = 0;
        Screen->Viewport.Width = width;
        Screen->Viewport.Height = height;

        int i;
        for (i = 1; i < width; i *= 2);
            Screen->PowerOfTwo = (i == width);
        Screen->BytesPerPixel = 2;
        Screen->FreeBuffer = 0;
        Screen->Depth = 16;
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

	vita2d_draw_texture_scale(tex, pos_x, pos_y, scale_x, scale_y);

    if(Options.ShowFps)
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
    static char fps_display[32];
    sprintf(fps_display, "FPS: %3.02f", curr_fps);

    int width = pspFontGetTextWidth(&PspStockFont, fps_display);
    int height = pspFontGetLineHeight(&PspStockFont);

    pspVideoFillRect(0, 0, width, height, PSP_COLOR_BLACK);
    pspVideoPrint(&PspStockFont, 0, 0, fps_display, PSP_COLOR_WHITE);
}

/***
 * Free up memory.
 */
void video_shutdown()
{
    free(tex);
    free(tex_data);
}