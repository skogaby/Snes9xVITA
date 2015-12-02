// Author: skogaby
// Lots of examples were followed using other emulators found on github

#include "vita_input.h"

void handle_button_press(unsigned int mapping, unsigned int pressed);

/***
 * Initializes our keybinding to handle input later on.
 */
void setup_input()
{
	pad = (SceCtrlData*)malloc(sizeof(SceCtrlData));
	keymap = (uint32_t*)malloc(sizeof(uint32_t) * 12);

	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    if (!pad || !keymap)
    {
        printf("Unable to allocate memory for input data. Bailing.");
        sceKernelExitProcess(0);
    }
}

/***
 * Callback for polling for new input.
 */
void retro_input_poll_callback()
{
	sceCtrlPeekBufferPositive(0, pad, 1);
    uint32_t keys_down = pad->buttons;

    // first, handle (single) physical button presses
    unsigned int i;

    for (i = MAP_BUTTON_UP; i <= MAP_BUTTON_START; i++)
    {
        handle_button_press(ActiveConfig.ButtonMap[i], keys_down & PhysicalButtonMap[i]);
    }

    // next, handle multi-button presses (L + R or start + select)
    if ((keys_down & SCE_CTRL_LTRIGGER) && (keys_down & SCE_CTRL_RTRIGGER))
    {
        handle_button_press(ActiveConfig.ButtonMap[MAP_BUTTON_LRTRIGGERS], true);
    }

    if ((keys_down & SCE_CTRL_START) && (keys_down & SCE_CTRL_SELECT))
    {
        handle_button_press(ActiveConfig.ButtonMap[MAP_BUTTON_STARTSELECT], true);
    }

    // finally, handle analog sticks
    if (pad->ly < 128 - JOY_THRESHOLD) // analog up
    {
        handle_button_press(ActiveConfig.ButtonMap[MAP_ANALOG_UP], true);
    }
    else if (pad->ly > 128 + JOY_THRESHOLD) // analog down
    {
        handle_button_press(ActiveConfig.ButtonMap[MAP_ANALOG_DOWN], true);
    }

    if (pad->lx < 128 - JOY_THRESHOLD) // analog left
    {
        handle_button_press(ActiveConfig.ButtonMap[MAP_ANALOG_LEFT], true);
    }
    else if (pad->lx > 128 + JOY_THRESHOLD) // analog right
    {
        handle_button_press(ActiveConfig.ButtonMap[MAP_ANALOG_RIGHT], true);
    }
}

/***
 * Callback to set the current input states based on the last polled input.
 */
int16_t retro_input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id)
{
	return keymap[id];
}

/***
 * Handles an input mapping and whether or not it's actually being pressed
 */
void handle_button_press(unsigned int mapping, unsigned int pressed)
{
    // handle it just being a normal button press
    if (mapping <= RETRO_DEVICE_ID_JOYPAD_R)
    {
        keymap[mapping] = pressed;
    }
    // user wants to go back to the menu
    else if (pressed && (mapping == SPC_MENU))
    {
        ResumeEmulation = 0;
    }
}