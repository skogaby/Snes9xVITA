// Main menu for Snes9xVITA. Slightly modified version
// of the menu for NeopopVITA
// https://github.com/frangarcj/NeopopVITA

#ifndef __VITA_MENU_H__
#define __VITA_MENU_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>
#include <psp2/ctrl.h>
#include <psp2/types.h>

#include <psplib/pl_file.h>
#include <psplib/image.h>
#include <psplib/ui.h>
#include <psplib/pl_menu.h>
#include <psplib/ctrl.h>
#include <psplib/pl_util.h>
#include <psplib/pl_psp.h>
#include <psplib/pl_ini.h>
#include <psplib/video.h>

#include "../src/snes9x.h"
#include "../src/memmap.h"
#include "../libretro/libretro.h"
#include "vita_video.h"

#define TAB_QUICKLOAD 0
#define TAB_STATE     1
#define TAB_CONTROL   2
#define TAB_OPTION    3
#define TAB_SYSTEM    4
#define TAB_ABOUT     5
#define TAB_MAX       TAB_SYSTEM

#define OPTION_DISPLAY_MODE  1
#define OPTION_EMULATE_SOUND 2
#define OPTION_SYNC_FREQ     3
#define OPTION_FRAMESKIP     4
#define OPTION_VSYNC         5
#define OPTION_CLOCK_FREQ    6
#define OPTION_SHOW_FPS      7
#define OPTION_CONTROL_MODE  8
#define OPTION_ANIMATE       9

#define SYSTEM_RESET         1
#define SYSTEM_SCRNSHOT      2

#define DISPLAY_MODE_UNSCALED    0
#define DISPLAY_MODE_2X 1
#define DISPLAY_MODE_FIT_HEIGHT  2
#define DISPLAY_MODE_FILL_SCREEN 3

// we start these at 12 because 0-11 are reserved
// for SNES buttons
#define SPC_MENU               12
#define SPC_UNMAPPED           13

#define MAP_ANALOG_UP          0
#define MAP_ANALOG_DOWN        1
#define MAP_ANALOG_LEFT        2
#define MAP_ANALOG_RIGHT       3
#define MAP_BUTTON_UP          4
#define MAP_BUTTON_DOWN        5
#define MAP_BUTTON_LEFT        6
#define MAP_BUTTON_RIGHT       7
#define MAP_BUTTON_SQUARE      8
#define MAP_BUTTON_CROSS       9
#define MAP_BUTTON_CIRCLE      10
#define MAP_BUTTON_TRIANGLE    11
#define MAP_BUTTON_LTRIGGER    12
#define MAP_BUTTON_RTRIGGER    13
#define MAP_BUTTON_SELECT      14
#define MAP_BUTTON_START       15
#define MAP_BUTTON_LRTRIGGERS  16
#define MAP_BUTTON_STARTSELECT 17

#define MAP_BUTTONS            18

typedef struct
{
    int ShowFps;
    int VSync;
    int ControlMode;
    int ClockFreq;
    int DisplayMode;
    int UpdateFreq;
    int Frameskip;
    int EmulateSound;
} EmulatorOptions;

struct ButtonConfig
{
    unsigned int ButtonMap[MAP_BUTTONS];
};

extern EmulatorOptions Options;
extern struct ButtonConfig ActiveConfig;
extern struct ButtonConfig DefaultConfig;
extern unsigned int PhysicalButtonMap[MAP_BUTTONS];
extern int OptionsChanged;
extern PspImage *Screen; 

int  InitMenu();
void DisplayMenu();
void TrashMenu();

const char* S9xGetFilename(const char* extension, uint32_t dirtype);
const char* S9xGetDirectory(uint32_t dirtype);

#endif // _EMUMENU_H
