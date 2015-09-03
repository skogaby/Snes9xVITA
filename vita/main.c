// Author: skogaby
// Lots of examples were followed using other emulators found on github

#include "main.h"

PspImage *Screen;

/***
 * Main entrypoint of the emulator
 */
int main()
{
    // set the CPU clock speed to 444MHz
    scePowerIsPowerOnline(444);

    // first, initialize Vita2D
    printf("Starting Snes9xVITA");
    vita2d_init();
    vita2d_set_vblank_wait(false);

    // TODO
    Screen = pspImageCreateVram(256, 256, GU_PSM_4444);

    // do some setup
    setup_input();
    setup_callbacks();
    setup_audio();
    pl_psp_init("cache0:/Snes9x/");

    // get the game ready
    if (InitMenu())
    {
        DisplayMenu();
        TrashMenu();
    }

    //load_rom();

    //// finally, enter the main loop
    //should_run = true;

    //while (should_run)
    //{
    //    retro_run();
    //}

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
void retro_environment_callback(unsigned cmd, void *data)
{
    // TODO: do something with this data...
    // Also, retro_init() doesn't work if I just
    // put a return here, hence the stupid printf.
    if (curr_frame == 0)
        printf("Inside of retro_environment_callback");
}

/***
 * Utility function to load up the ROM using the libretro API.
 */
void load_rom()
{
    // let the user choose a ROM
    printf("Letting the user choose a ROM");

    const char *start_path = "cache0:/VitaDefilerClient/Documents";
    const char *supported_ext[] = { "smc", "fig", "sfc", "gd3", "gd7",
                                    "dx2", "bsx", "swc",
                                    "SMC", "FIG", "SFC", "GD3", "GD7",
                                    "DX2", "BSX", "SWC", NULL };

    file_choose(start_path, rom_path, "Choose a Super Nintendo / Super Famicom ROM", supported_ext);

    // load the ROM into the emulator
    printf("Loading the ROM into the emulator...");

    struct retro_game_info game;
    game.path = rom_path;
    game.meta = NULL;
    game.data = NULL;
    game.size = NULL;

    retro_load_game(&game);
    printf("ROM loaded into emulator successfully");

    // if a save file exists for the game, load it up
    LoadSRAM();

    // after we've loaded the rom, clear both buffers
    // since we don't clear buffers between frames
    // during execution. if we don't, the font
    // will remain on the screen.
    // doing less than this resulted in one buffer
    // retaining the text. will clean up later.
    vita2d_start_drawing();
    vita2d_clear_screen();
    vita2d_end_drawing();

    vita2d_swap_buffers();

    vita2d_start_drawing();
    vita2d_clear_screen();
    vita2d_end_drawing();

    vita2d_swap_buffers();

    vita2d_start_drawing();
    vita2d_clear_screen();
    vita2d_end_drawing();
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

/***
 * Should simply return the filepath of the ROM, but replacing its file extension
 * with the given extension.
 */
const char* S9xGetFilename(const char* extension, uint32_t dirtype)
{
    size_t len = strlen(rom_path);
    return strcat(strndup(rom_path, len - 3), extension);
}

/***
 * Should simply return the folder that the ROM was loaded from.
 */
const char* S9xGetDirectory(uint32_t dirtype)
{
    return strndup(rom_path, strrchr(rom_path, '/') - rom_path + 1);
}