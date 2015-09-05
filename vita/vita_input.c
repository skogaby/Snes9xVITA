// Author: skogaby
// Lots of examples were followed using other emulators found on github

#include "vita_input.h"

/***
 * Initializes our keybinding to handle input later on.
 */
void setup_input()
{
	printf("Setting up input data");

	pad = (SceCtrlData*)malloc(sizeof(SceCtrlData));
	keymap = (uint32_t*)malloc(sizeof(uint32_t) * 12);

	sceCtrlSetSamplingMode(PSP2_CTRL_MODE_ANALOG);

    if (!pad || !keymap)
    {
        printf("Unable to allocate memory for input data. Bailing.");
        sceKernelExitProcess(0);
    }
    else
    {
	    printf("Input data initialized");
    }
}

/***
 * Callback for polling for new input.
 */
void retro_input_poll_callback()
{
	sceCtrlPeekBufferPositive(0, pad, 1);
	
    uint32_t keys_down = pad->buttons;

	keymap[RETRO_DEVICE_ID_JOYPAD_A] = keys_down & PSP2_CTRL_CIRCLE;
	keymap[RETRO_DEVICE_ID_JOYPAD_B] = keys_down & PSP2_CTRL_CROSS;
	keymap[RETRO_DEVICE_ID_JOYPAD_X] = keys_down & PSP2_CTRL_TRIANGLE;
	keymap[RETRO_DEVICE_ID_JOYPAD_Y] = keys_down & PSP2_CTRL_SQUARE;
	keymap[RETRO_DEVICE_ID_JOYPAD_L] = keys_down & PSP2_CTRL_LTRIGGER;
	keymap[RETRO_DEVICE_ID_JOYPAD_R] = keys_down & PSP2_CTRL_RTRIGGER;
	keymap[RETRO_DEVICE_ID_JOYPAD_START] = keys_down & PSP2_CTRL_START;
	keymap[RETRO_DEVICE_ID_JOYPAD_SELECT] = keys_down & PSP2_CTRL_SELECT;
	keymap[RETRO_DEVICE_ID_JOYPAD_UP] = (keys_down & PSP2_CTRL_UP) || (pad->ly < 128 - JOY_THRESHOLD);
	keymap[RETRO_DEVICE_ID_JOYPAD_DOWN] = (keys_down & PSP2_CTRL_DOWN) || (pad->ly > 128 + JOY_THRESHOLD);
	keymap[RETRO_DEVICE_ID_JOYPAD_LEFT] = (keys_down & PSP2_CTRL_LEFT) || (pad->lx < 128 - JOY_THRESHOLD);
	keymap[RETRO_DEVICE_ID_JOYPAD_RIGHT] = (keys_down & PSP2_CTRL_RIGHT) || (pad->lx > 128 + JOY_THRESHOLD);

	// TODO: make this user-configurable
	// take the user back to the menu if 
    // they press L + R
	if(keymap[RETRO_DEVICE_ID_JOYPAD_L] && keymap[RETRO_DEVICE_ID_JOYPAD_R])
	{
        ResumeEmulation = 0;
	}
}

/***
 * Callback to set the current input states based on the last polled input.
 */
int16_t retro_input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id)
{
	return keymap[id];
}