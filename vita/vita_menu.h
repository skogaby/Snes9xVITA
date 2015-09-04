// Main menu for Snes9xVITA. Slightly modified version
// of the menu for NeopopVITA
// https://github.com/frangarcj/NeopopVITA

#ifndef __VITA_MENU_H__
#define __VITA_MENU_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/io/stat.h>
#include <psp2/ctrl.h>
#include <psp2/types.h>
#include <psp2/io/fcntl.h>

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
#include "../libretro/libretro.h"

#define PSP_APP_NAME "Snes9xVITA"
#define PSP_APP_VER  "1.0.0 beta"

#define TAB_QUICKLOAD 0
#define TAB_STATE     1
#define TAB_CONTROL   2
#define TAB_OPTION    3
#define TAB_SYSTEM    4
#define TAB_ABOUT     5
#define TAB_MAX       TAB_SYSTEM

#define OPTION_DISPLAY_MODE 1
#define OPTION_SYNC_FREQ    2
#define OPTION_FRAMESKIP    3
#define OPTION_VSYNC        4
#define OPTION_CLOCK_FREQ   5
#define OPTION_SHOW_FPS     6
#define OPTION_CONTROL_MODE 7
#define OPTION_ANIMATE      8

#define SYSTEM_SCRNSHOT     1
#define SYSTEM_RESET        2
#define SYSTEM_AUDIO		3

#define DISPLAY_MODE_UNSCALED    0
#define DISPLAY_MODE_2X 1
#define DISPLAY_MODE_3X 2
#define DISPLAY_MODE_FIT_HEIGHT  3
#define DISPLAY_MODE_FILL_SCREEN 4

#define JOY 0x1000
#define SPC 0x2000

#define CODE_MASK(x) (x & 0xff)

#define SPC_MENU               1

#define MAP_BUTTONS            18

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

typedef struct
{
    int   ShowFps;
    int   VSync;
    int   ControlMode;
    int   ClockFreq;
    int   DisplayMode;
    int   UpdateFreq;
    int   Frameskip;
} EmulatorOptions;

struct ButtonConfig
{
  unsigned int ButtonMap[MAP_BUTTONS];
};

int  InitMenu();
void DisplayMenu();
void TrashMenu();

const char* S9xGetFilename(const char* extension, uint32_t dirtype);
const char* S9xGetDirectory(uint32_t dirtype);

#endif // _EMUMENU_H
