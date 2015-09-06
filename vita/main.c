// Author: skogaby
// Lots of examples were followed using other emulators found on github

#include "main.h"

/***
 * Main entrypoint of the emulator
 */
int main()
{
    // first, initialize Vita2D
    printf("Starting Snes9xVITA");
    vita2d_init_advanced(8 * 1024 * 1024);

    // do some setup
    setup_input();
    setup_callbacks();
    setup_audio();

    pl_psp_init("cache0:/Snes9xVITA/");

    // get the game ready
    if (InitMenu())
    {
        DisplayMenu();
        TrashMenu();
    }

    // once emulation is complete, shut down and exit
    vita_cleanup();

    sceKernelExitProcess(0);
    return 0;
}

/***
 * Sets up all of our appropriate callback functions with libretro.
 */
void setup_callbacks()
{
    printf("Setting up libretro callbacks...");

    retro_set_environment(&retro_environment_callback);
    retro_set_video_refresh(&retro_video_refresh_callback);
    retro_set_input_poll(&retro_input_poll_callback);
    retro_set_input_state(&retro_input_state_callback);
    retro_set_audio_sample_batch(&retro_audio_sample_batch_callback);

    printf("Libretro callbacks created successfully!");
}

/***
 * Callback for updating the libretro environment.
 */
int retro_environment_callback(unsigned cmd, void *data)
{
    // TODO: do something with this data...
    // Also, retro_init() doesn't work if I just
    // put a return here, hence the stupid printf.
    if (curr_frame == 0)
        printf("Inside of retro_environment_callback");

    return 0;
}

/***
 * Cleans up the long-lived memory we've allocated.
 */
void vita_cleanup()
{
    free(keymap);
    free(pad);

    vita2d_fini();
    audio_shutdown();
    video_shutdown();
}
