// Main menu for Snes9xVITA. Slightly modified version
// of the menu for NeopopVITA
// https://github.com/frangarcj/NeopopVITA

#include "vita_menu.h"

extern PspImage *Screen;

static const char *QuickloadFilter[] = { "SMC", "FIG", "SFC", "GD3", "GD7", "DX2", "BSX", "SWC", NULL };
static const char *ScreenshotDir = "screens";
static const char *SaveStateDir = "savedata";
static const char *ButtonConfigFile = "buttons";
static const char *OptionsFile = "snes9xvita.ini";
static const char *TabLabel[] = { "Game", "Save/Load", "Control", "Options", "System", "About" };
static const char ControlHelpText[] = "\026\250\020 Change mapping\t\026\001\020 Save to \271\t\026\243\020 Load defaults";
static const char PresentSlotText[] = "\026\244\020 Save\t\026\001\020 Load\t\026\243\020 Delete";
static const char EmptySlotText[] = "\026\244\020 Save";

char *GameName;

pl_file_path CurrentGame = "";
pl_file_path GamePath;
pl_file_path SaveStatePath;
pl_file_path ScreenshotPath;
bool8 mute = false;

static int TabIndex;
static int ResumeEmulation;
static PspImage *Background;
static PspImage *NoSaveIcon;

/* Define various menu options */
PL_MENU_OPTIONS_BEGIN(ToggleOptions)
    PL_MENU_OPTION("Disabled", 0)
    PL_MENU_OPTION("Enabled",  1)
PL_MENU_OPTIONS_END
PL_MENU_OPTIONS_BEGIN(ScreenSizeOptions)
    PL_MENU_OPTION("Actual size", DISPLAY_MODE_UNSCALED)
    PL_MENU_OPTION("4:3 scaled (2x)", DISPLAY_MODE_2X)
	PL_MENU_OPTION("4:3 scaled (3x)", DISPLAY_MODE_3X)
    PL_MENU_OPTION("4:3 scaled (fit height)", DISPLAY_MODE_FIT_HEIGHT)
    PL_MENU_OPTION("16:9 scaled (fit screen)", DISPLAY_MODE_FILL_SCREEN)
PL_MENU_OPTIONS_END
PL_MENU_OPTIONS_BEGIN(FrameLimitOptions)
    PL_MENU_OPTION("Disabled",      0)
    PL_MENU_OPTION("60 fps (NTSC)", 60)
PL_MENU_OPTIONS_END
PL_MENU_OPTIONS_BEGIN(FrameSkipOptions)
    PL_MENU_OPTION("No skipping",   1)
    PL_MENU_OPTION("Skip 1 frame",  2)
    PL_MENU_OPTION("Skip 2 frames", 3)
    PL_MENU_OPTION("Skip 3 frames", 4)
    PL_MENU_OPTION("Skip 4 frames", 5)
    PL_MENU_OPTION("Skip 5 frames", 6)
    PL_MENU_OPTION("Skip 6 frames", 7)
PL_MENU_OPTIONS_END
PL_MENU_OPTIONS_BEGIN(PspClockFreqOptions)
    PL_MENU_OPTION("333 MHz", 333)
    PL_MENU_OPTION("444 MHz", 444)
PL_MENU_OPTIONS_END
PL_MENU_OPTIONS_BEGIN(ControlModeOptions)
    PL_MENU_OPTION("\026\242\020 cancels, \026\241\020 confirms (US)",    0)
    PL_MENU_OPTION("\026\241\020 cancels, \026\242\020 confirms (Japan)", 1)
PL_MENU_OPTIONS_END
PL_MENU_OPTIONS_BEGIN(ButtonMapOptions)
    /* Unmapped */
    PL_MENU_OPTION("None", 0)
    /* Special */
    PL_MENU_OPTION("Special: Open Menu", SPC | SPC_MENU)
    /* Buttons */
    PL_MENU_OPTION("Up",       JOY | 0x001)
    PL_MENU_OPTION("Down",     JOY | 0x002)
    PL_MENU_OPTION("Left",     JOY | 0x004)
    PL_MENU_OPTION("Right",    JOY | 0x008)
    PL_MENU_OPTION("Button A", JOY | 0x010)
    PL_MENU_OPTION("Button B", JOY | 0x020)
    PL_MENU_OPTION("Button X", JOY | 0x040)
    PL_MENU_OPTION("Button Y", JOY | 0x080)
    PL_MENU_OPTION("Button L", JOY | 0x100)
    PL_MENU_OPTION("Button R", JOY | 0x200)
    PL_MENU_OPTION("Start",    JOY | 0x400)
    PL_MENU_OPTION("Select",   JOY | 0x800)
PL_MENU_OPTIONS_END

/* Define the actual menus */
PL_MENU_ITEMS_BEGIN(OptionMenuDef)
    PL_MENU_HEADER("Video")
        PL_MENU_ITEM("Screen size", OPTION_DISPLAY_MODE, ScreenSizeOptions, "\026\250\020 Change screen size")
    PL_MENU_HEADER("Performance")
        PL_MENU_ITEM("Frame limiter", OPTION_SYNC_FREQ, FrameLimitOptions, "\026\250\020 Change screen update frequency")
        PL_MENU_ITEM("Frame skipping", OPTION_FRAMESKIP, FrameSkipOptions, "\026\250\020 Change number of frames skipped per update")
        PL_MENU_ITEM("VSync", OPTION_VSYNC, ToggleOptions, "\026\250\020 Enable to reduce tearing; disable to increase speed")
        PL_MENU_ITEM("PSP clock frequency", OPTION_CLOCK_FREQ, PspClockFreqOptions, "\026\250\020 Larger values: faster emulation, faster battery depletion (default: 333MHz)")
        PL_MENU_ITEM("Show FPS counter", OPTION_SHOW_FPS, ToggleOptions, "\026\250\020 Show/hide the frames-per-second counter")
    PL_MENU_HEADER("Menu")
        PL_MENU_ITEM("Button mode", OPTION_CONTROL_MODE, ControlModeOptions, "\026\250\020 Change OK and Cancel button mapping")
        PL_MENU_ITEM("Animate", OPTION_ANIMATE, ToggleOptions, "\026\250\020 Enable/disable in-menu animations")
PL_MENU_ITEMS_END

PL_MENU_ITEMS_BEGIN(ControlMenuDef)
  	PL_MENU_ITEM(PSP_CHAR_ANALUP, MAP_ANALOG_UP, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_ANALDOWN, MAP_ANALOG_DOWN, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_ANALLEFT, MAP_ANALOG_LEFT, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_ANALRIGHT, MAP_ANALOG_RIGHT, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_UP, MAP_BUTTON_UP, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_DOWN, MAP_BUTTON_DOWN, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_LEFT, MAP_BUTTON_LEFT, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_RIGHT, MAP_BUTTON_RIGHT, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_SQUARE, MAP_BUTTON_SQUARE, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_CROSS, MAP_BUTTON_CROSS, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_CIRCLE, MAP_BUTTON_CIRCLE, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_TRIANGLE, MAP_BUTTON_TRIANGLE, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_LTRIGGER, MAP_BUTTON_LTRIGGER, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_RTRIGGER, MAP_BUTTON_RTRIGGER, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_SELECT, MAP_BUTTON_SELECT, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_START, MAP_BUTTON_START, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_LTRIGGER"+"PSP_CHAR_RTRIGGER, MAP_BUTTON_LRTRIGGERS, ButtonMapOptions, ControlHelpText)
    PL_MENU_ITEM(PSP_CHAR_START"+"PSP_CHAR_SELECT, MAP_BUTTON_STARTSELECT, ButtonMapOptions, ControlHelpText)
PL_MENU_ITEMS_END

PL_MENU_ITEMS_BEGIN(SystemMenuDef)
    PL_MENU_HEADER("Audio")
        PL_MENU_ITEM("Sound", SYSTEM_AUDIO, ToggleOptions, "\026\001\020 Enable/disable sound")
    PL_MENU_HEADER("System")
        PL_MENU_ITEM("Reset", SYSTEM_RESET, NULL, "\026\001\020 Reset")
        PL_MENU_ITEM("Save screenshot",  SYSTEM_SCRNSHOT, NULL, "\026\001\020 Save screenshot")
PL_MENU_ITEMS_END

static void LoadOptions();
static int SaveOptions();

static void InitButtonConfig();
static int  SaveButtonConfig();
static int  LoadButtonConfig();

static void DisplayStateTab();
static PspImage* LoadStateIcon(const char *path);

static int OnSplashButtonPress(const struct PspUiSplash *splash, uint32_t button_mask);
static void OnSplashRender(const void *uiobject, const void *null);

static int OnGenericCancel(const void *uiobject, const void *param);
static void OnGenericRender(const void *uiobject, const void *item_obj);
static int OnGenericButtonPress(const PspUiFileBrowser *browser, const char *path, uint32_t button_mask);

int OnQuickloadOk(const void *browser, const void *path);

static int OnSaveStateOk(const void *gallery, const void *item);
static int OnSaveStateButtonPress(const PspUiGallery *gallery, pl_menu_item* item, uint32_t button_mask);

static int OnMenuItemChanged(const struct PspUiMenu *uimenu, pl_menu_item* item, const pl_menu_option* option);
static int OnMenuOk(const void *uimenu, const void* sel_item);
static int OnMenuButtonPress(const struct PspUiMenu *uimenu, pl_menu_item* sel_item, uint32_t button_mask);

void OnSystemRender(const void *uiobject, const void *item_obj);

PspUiSplash SplashScreen =
{
    OnSplashRender,
    OnGenericCancel,
    OnSplashButtonPress,
    NULL
};

PspUiFileBrowser QuickloadBrowser =
{
    OnGenericRender,
    OnQuickloadOk,
    OnGenericCancel,
    OnGenericButtonPress,
    QuickloadFilter,
    0
};

PspUiGallery SaveStateGallery =
{
    OnGenericRender,        /* OnRender() */
    OnSaveStateOk,          /* OnOk() */
    OnGenericCancel,        /* OnCancel() */
    OnSaveStateButtonPress, /* OnButtonPress() */
    NULL                    /* Userdata */
};

PspUiMenu ControlUiMenu =
{
    OnGenericRender,   /* OnRender() */
    OnMenuOk,          /* OnOk() */
    OnGenericCancel,   /* OnCancel() */
    OnMenuButtonPress, /* OnButtonPress() */
    OnMenuItemChanged, /* OnItemChanged() */
};

PspUiMenu OptionUiMenu =
{
    OnGenericRender,   /* OnRender() */
    OnMenuOk,          /* OnOk() */
    OnGenericCancel,   /* OnCancel() */
    OnMenuButtonPress, /* OnButtonPress() */
    OnMenuItemChanged, /* OnItemChanged() */
};

PspUiMenu SystemUiMenu =
{
    OnSystemRender,    /* OnRender() */
    OnMenuOk,          /* OnOk() */
    OnGenericCancel,   /* OnCancel() */
    OnMenuButtonPress, /* OnButtonPress() */
    OnMenuItemChanged, /* OnItemChanged() */
};

/* Game configuration (includes button maps) */
struct ButtonConfig ActiveConfig;

/* Default configuration */
struct ButtonConfig DefaultConfig =
{

    {
        JOY | 0x001, /* Analog Up */
        JOY | 0x002, /* Analog Down */
        JOY | 0x004, /* Analog Left */
        JOY | 0x008, /* Analog Right */
        JOY | 0x001, /* D-pad Up */
        JOY | 0x002, /* D-pad Down */
        JOY | 0x004, /* D-pad Left */
        JOY | 0x008, /* D-pad Right */
        JOY | 0x080, /* Square */
        JOY | 0x020, /* Cross */
        JOY | 0x010, /* Circle */
        JOY | 0x040, /* Triangle */
        JOY | 0x100, /* L Trigger */
        JOY | 0x200, /* R Trigger */
        JOY | 0x800, /* Select */
        JOY | 0x400, /* Start */
        SPC | SPC_MENU, /* L+R Triggers */
        0, /* Start + Select */
    }
};

/* Button masks */
const uint64_t ButtonMask[] =
{
    PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER,
    PSP_CTRL_START    | PSP_CTRL_SELECT,
    PSP_CTRL_ANALUP,
    PSP_CTRL_ANALDOWN,
    PSP_CTRL_ANALLEFT,
    PSP_CTRL_ANALRIGHT,
    PSP_CTRL_UP,
    PSP_CTRL_DOWN,
    PSP_CTRL_LEFT,
    PSP_CTRL_RIGHT,
    PSP_CTRL_SQUARE,
    PSP_CTRL_CROSS,
    PSP_CTRL_CIRCLE,
    PSP_CTRL_TRIANGLE,
    PSP_CTRL_LTRIGGER,
    PSP_CTRL_RTRIGGER,
    PSP_CTRL_SELECT,
    PSP_CTRL_START,
    0 /* End */
};

/* Button map ID's */
const int ButtonMapId[] =
{
    MAP_BUTTON_LRTRIGGERS,
    MAP_BUTTON_STARTSELECT,
    MAP_ANALOG_UP,
    MAP_ANALOG_DOWN,
    MAP_ANALOG_LEFT,
    MAP_ANALOG_RIGHT,
    MAP_BUTTON_UP,
    MAP_BUTTON_DOWN,
    MAP_BUTTON_LEFT,
    MAP_BUTTON_RIGHT,
    MAP_BUTTON_SQUARE,
    MAP_BUTTON_CROSS,
    MAP_BUTTON_CIRCLE,
    MAP_BUTTON_TRIANGLE,
    MAP_BUTTON_LTRIGGER,
    MAP_BUTTON_RTRIGGER,
    MAP_BUTTON_SELECT,
    MAP_BUTTON_START,
    -1
};

int InitMenu()
{
    /* Reset variables */
    TabIndex = TAB_ABOUT;
    Background = NULL;
    GameName = NULL;

    /* Initialize paths */
    sprintf(SaveStatePath, "%ssavedata/", pl_psp_get_app_directory());
    sprintf(ScreenshotPath, "%sscreenshot/", pl_psp_get_app_directory());
    sprintf(GamePath, "%s", pl_psp_get_app_directory());

    if (!pl_file_exists(SaveStatePath))
        pl_file_mkdir_recursive(SaveStatePath);
        

    /* Initialize options */
    LoadOptions();

    ///* Initialize emulation engine */
    //if (!InitEmulation())
    //    return 0;

    /* Load the background image */
    pl_file_path background;
    snprintf(background, sizeof(background) - 1, "%sbackground.png", pl_psp_get_app_directory());
    Background = pspImageLoadPng(background);

    /* Init NoSaveState icon image */
    NoSaveIcon = pspImageCreateOptimized(160, 152, PSP_IMAGE_16BPP);
    pspImageClear(NoSaveIcon, RGB(0x01, 0x33, 0x24));

    /* Initialize state menu */
    //  SaveStateGallery.Menu = pl_menu_create();
    int i;
    pl_menu_item *item;

    for (i = 0; i < 10; i++)
    {
        item = pl_menu_append_item(&SaveStateGallery.Menu, i, NULL);
        pl_menu_set_item_help_text(item, EmptySlotText);
    }

    /* Initialize control menu */
    pl_menu_create(&ControlUiMenu.Menu, ControlMenuDef);

    /* Initialize options menu */
    pl_menu_create(&OptionUiMenu.Menu, OptionMenuDef);

    /* Initialize system menu */
    pl_menu_create(&SystemUiMenu.Menu, SystemMenuDef);

    /* Load default configuration */
    LoadButtonConfig();

    /* Initialize UI components */
    UiMetric.Background = Background;
    UiMetric.Font = &PspStockFont;
    UiMetric.Left = 16;
    UiMetric.Top = 48;
    UiMetric.Right = 944;
    UiMetric.Bottom = 500;
    UiMetric.OkButton = (!Settings.ControlMode) ? PSP_CTRL_CROSS : PSP_CTRL_CIRCLE;
    UiMetric.CancelButton = (!Settings.ControlMode) ? PSP_CTRL_CIRCLE : PSP_CTRL_CROSS;
    UiMetric.ScrollbarColor = PSP_COLOR_GRAY;
    UiMetric.ScrollbarBgColor = 0x44ffffff;
    UiMetric.ScrollbarWidth = 10;
    UiMetric.TextColor = PSP_COLOR_GRAY;
    UiMetric.SelectedColor = PSP_COLOR_YELLOW;
    UiMetric.SelectedBgColor = COLOR(0xff, 0xff, 0xff, 0x44);
    UiMetric.StatusBarColor = PSP_COLOR_WHITE;
    UiMetric.BrowserFileColor = PSP_COLOR_GRAY;
    UiMetric.BrowserDirectoryColor = PSP_COLOR_YELLOW;
    UiMetric.GalleryIconsPerRow = 5;
    UiMetric.GalleryIconMarginWidth = 16;
    UiMetric.MenuItemMargin = 20;
    UiMetric.MenuSelOptionBg = PSP_COLOR_BLACK;
    UiMetric.MenuOptionBoxColor = PSP_COLOR_GRAY;
    UiMetric.MenuOptionBoxBg = COLOR(0x01, 0x33, 0x24, 0xbb);
    UiMetric.MenuDecorColor = PSP_COLOR_YELLOW;
    UiMetric.DialogFogColor = COLOR(0, 0, 0, 88);
    UiMetric.TitlePadding = 8;
    UiMetric.TitleColor = PSP_COLOR_WHITE;
    UiMetric.MenuFps = 30;
    UiMetric.TabBgColor = COLOR(0xc5, 0xe0, 0xd8, 0xff);

    return 1;
}

/* Load options */
void LoadOptions()
{
    pl_file_path path;
    snprintf(path, sizeof(path) - 1, "%snes9xvita.ini", pl_psp_get_app_directory());

    /* Initialize INI structure */
    pl_ini_file init;
    /* Read the file */
    pl_ini_load(&init, path);

    /* Load values */
    Settings.DisplayMode = pl_ini_get_int(&init, "Video", "Display Mode", DISPLAY_MODE_UNSCALED);
    Settings.UpdateFreq = pl_ini_get_int(&init, "Video", "Update Frequency", 0);
    Settings.Frameskip = pl_ini_get_int(&init, "Video", "Frameskip", 1);
    Settings.VSync = pl_ini_get_int(&init, "Video", "VSync", 1);
    Settings.ClockFreq = pl_ini_get_int(&init, "Video", "PSP Clock Frequency", 333);
    Settings.ShowFps = pl_ini_get_int(&init, "Video", "Show FPS", 0);

    Settings.ControlMode = pl_ini_get_int(&init, "Menu", "Control Mode", 0);
    UiMetric.Animate = pl_ini_get_int(&init, "Menu", "Animate", 1);

    // TODO: make this control sound emulation
    mute = !pl_ini_get_int(&init, "System", "Sound", 1);

    pl_ini_get_string(&init, "File", "Game Path", NULL, GamePath, sizeof(GamePath));

    /* Clean up */
    pl_ini_destroy(&init);
}

/* Save options */
int SaveOptions()
{
    pl_file_path path;
    snprintf(path, sizeof(path) - 1, "%snes9xvita.ini", pl_psp_get_app_directory());

    /* Initialize INI structure */
    pl_ini_file init;
    pl_ini_create(&init);

    /* Set values */
    pl_ini_set_int(&init, "Video", "Display Mode", Settings.DisplayMode);
    pl_ini_set_int(&init, "Video", "Update Frequency", Settings.UpdateFreq);
    pl_ini_set_int(&init, "Video", "Frameskip", Settings.Frameskip);
    pl_ini_set_int(&init, "Video", "VSync", Settings.VSync);
    pl_ini_set_int(&init, "Video", "PSP Clock Frequency", Settings.ClockFreq);
    pl_ini_set_int(&init, "Video", "Show FPS", Settings.ShowFps);

    pl_ini_set_int(&init, "Menu", "Control Mode", Settings.ControlMode);
    pl_ini_set_int(&init, "Menu", "Animate", UiMetric.Animate);

    // TODO: make this control sound emulation
    pl_ini_set_int(&init, "System", "Sound", !mute);

    /* Save INI file */
    int status = pl_ini_save(&init, path);

    /* Clean up */
    pl_ini_destroy(&init);

    return status;
}

void DisplayMenu()
{
    int i;
    pl_menu_item *item;

    /* Menu loop */
    do
    {
        ResumeEmulation = 0;

        /* Set normal clock frequency */
        pl_psp_set_clock_freq(333);
        /* Set buttons to autorepeat */
        pspCtrlSetPollingMode(PSP_CTRL_AUTOREPEAT);

        /* Display appropriate tab */
        switch (TabIndex)
        {
        case TAB_QUICKLOAD:
            pspUiOpenBrowser(&QuickloadBrowser, (GameName) ? GameName : GamePath);
            break;
        case TAB_STATE:
            DisplayStateTab();
            break;
        case TAB_CONTROL:
            /* Load current button mappings */
            for (item = ControlUiMenu.Menu.items, i = 0; item; item = item->next, i++)
                pl_menu_select_option_by_value(item, (void*)ActiveConfig.ButtonMap[i]);
            pspUiOpenMenu(&ControlUiMenu, NULL);
            break;
        case TAB_OPTION:
            /* Init menu options */
            item = pl_menu_find_item_by_id(&OptionUiMenu.Menu, OPTION_DISPLAY_MODE);
            pl_menu_select_option_by_value(item, (void*)Settings.DisplayMode);
            item = pl_menu_find_item_by_id(&OptionUiMenu.Menu, OPTION_SYNC_FREQ);
            pl_menu_select_option_by_value(item, (void*)Settings.UpdateFreq);
            item = pl_menu_find_item_by_id(&OptionUiMenu.Menu, OPTION_FRAMESKIP);
            pl_menu_select_option_by_value(item, (void*)(int)Settings.Frameskip);

            if ((item = pl_menu_find_item_by_id(&OptionUiMenu.Menu, OPTION_VSYNC)))
                pl_menu_select_option_by_value(item, (void*)Settings.VSync);

            item = pl_menu_find_item_by_id(&OptionUiMenu.Menu, OPTION_CLOCK_FREQ);
            pl_menu_select_option_by_value(item, (void*)Settings.ClockFreq);
            item = pl_menu_find_item_by_id(&OptionUiMenu.Menu, OPTION_SHOW_FPS);
            pl_menu_select_option_by_value(item, (void*)Settings.ShowFps);
            item = pl_menu_find_item_by_id(&OptionUiMenu.Menu, OPTION_CONTROL_MODE);
            pl_menu_select_option_by_value(item, (void*)Settings.ControlMode);
            item = pl_menu_find_item_by_id(&OptionUiMenu.Menu, OPTION_ANIMATE);
            pl_menu_select_option_by_value(item, (void*)UiMetric.Animate);
            pspUiOpenMenu(&OptionUiMenu, NULL);
            SaveOptions();
            break;
        case TAB_SYSTEM:
            item = pl_menu_find_item_by_id(&SystemUiMenu.Menu, SYSTEM_AUDIO);
            // TODO: make this control sound emulation
            pl_menu_select_option_by_value(item, (void*)!mute);
            pspUiOpenMenu(&SystemUiMenu, NULL);
            SaveOptions();
            break;
        case TAB_ABOUT:
            pspUiSplashScreen(&SplashScreen);
            break;
        }

        if (!ExitPSP)
        {
            /* Set clock frequency during emulation */
            pl_psp_set_clock_freq(Settings.ClockFreq);
            /* Set buttons to normal mode */
            pspCtrlSetPollingMode(PSP_CTRL_NORMAL);

            /* Resume emulation */
            if (ResumeEmulation)
            {
                if (UiMetric.Animate) pspUiFadeout();
                // TODO: run the emulator
                // RunEmulation();
                if (UiMetric.Animate) pspUiFadeout();
            }
        }
    } while (!ExitPSP);
}

void OnSplashRender(const void *splash, const void *null)
{
    int fh, i, x, y, height;

    const char *lines[] =
    {
        PSP_APP_NAME" version "PSP_APP_VER" ("__DATE__")",
        "\026https://github.com/skogaby/Snes9xVITA",
        " ",
        "2015 skogaby",
        "Based on Snes9xNext",
        "https://github.com/libretro/snes9x-next",
        NULL
    };

    fh = pspFontGetLineHeight(UiMetric.Font);

    for (i = 0; lines[i]; i++);
    height = fh * (i - 1);

    /* Render lines */
    for (i = 0, y = SCR_HEIGHT / 2 - height / 2; lines[i]; i++, y += fh)
    {
        x = SCR_WIDTH / 2 - pspFontGetTextWidth(UiMetric.Font, lines[i]) / 2;
        pspVideoPrint(UiMetric.Font, x, y, lines[i], PSP_COLOR_GRAY);
    }

    /* Render PSP status */
    OnGenericRender(splash, null);
}

int OnSplashButtonPress(const struct PspUiSplash *splash, uint32_t button_mask)
{
    return OnGenericButtonPress(NULL, NULL, button_mask);
}

/* Handles drawing of generic items */
void OnGenericRender(const void *uiobject, const void *item_obj)
{
    /* Draw tabs */
    int i, x, width, height = pspFontGetLineHeight(UiMetric.Font);

    for (i = 0, x = 5; i <= TAB_MAX; i++, x += width + 10)
    {
        width = -10;

        if (!GameName && (i == TAB_STATE || i == TAB_SYSTEM))
            continue;

        /* Determine width of text */
        width = pspFontGetTextWidth(UiMetric.Font, TabLabel[i]);

        /* Draw background of active tab */
        if (i == TabIndex)
            pspVideoFillRect(x - 5, 0, x + width + 5, height + 1, UiMetric.TabBgColor);

        /* Draw name of tab */
        pspVideoPrint(UiMetric.Font, x, 0, TabLabel[i], PSP_COLOR_WHITE);
    }
}

int OnGenericButtonPress(const PspUiFileBrowser *browser, const char *path, uint32_t button_mask)
{
    int tab_index;

    /* If L or R are pressed, switch tabs */
    if (button_mask & PSP_CTRL_LTRIGGER)
    {
        TabIndex--;

        do
        {
            tab_index = TabIndex;

            if (!GameName && (TabIndex == TAB_STATE || TabIndex == TAB_SYSTEM)) 
                TabIndex--;

            if (TabIndex < 0) 
                TabIndex = TAB_MAX;
        } while (tab_index != TabIndex);
    }
    else if (button_mask & PSP_CTRL_RTRIGGER)
    {
        TabIndex++;

        do
        {
            tab_index = TabIndex;

            if (!GameName && (TabIndex == TAB_STATE || TabIndex == TAB_SYSTEM)) 
                TabIndex++;

            if (TabIndex > TAB_MAX) 
                TabIndex = 0;
        } while (tab_index != TabIndex);
    }
    else if ((button_mask & (PSP_CTRL_START | PSP_CTRL_SELECT)) == (PSP_CTRL_START | PSP_CTRL_SELECT))
    {
        if (pl_util_save_vram_seq(ScreenshotPath, "ui"))
            pspUiAlert("Saved successfully");
        else
            pspUiAlert("ERROR: Not saved");

        return 0;
    }
    else 
        return 0;

    return 1;
}

int OnGenericCancel(const void *uiobject, const void* param)
{
    if (GameName) 
        ResumeEmulation = 1;

    return 1;
}

int OnQuickloadOk(const void *browser, const void *path)
{
    if (GameName)
    {
        // TODO: unload the ROM
        // system_unload_rom();
        free(GameName);
    }

    GameName = strdup(path);
    pspUiFlashMessage("Loading, please wait...");

    // TODO: load the ROM
    if (false) // !system_load_rom((char*)path))
    {
        pspUiAlert("Error loading cartridge");
        return 0;
    }

    pl_file_get_parent_directory((const char*)path, GamePath, sizeof(GamePath));
    // TODO: reset the emulator
    // reset();
    ResumeEmulation = 1;

    return 1;
}

int OnSaveStateOk(const void *gallery, const void *item)
{
    if (!GameName) 
    { 
        TabIndex++; 
        return 0; 
    }

    char *path;
    const char *config_name = pl_file_get_filename(GameName);

    path = (char*)malloc(strlen(SaveStatePath) + strlen(config_name) + 8);
    sprintf(path, "%s%s.s%02i", SaveStatePath, config_name, ((const pl_menu_item*)item)->id);

    if (pl_file_exists(path) && pspUiConfirm("Load state?"))
    {
        // TODO: make this load the save state
        if (true) // state_restore(path))
        {
            ResumeEmulation = 1;
            pl_menu_find_item_by_id(&((const PspUiGallery*)gallery)->Menu, ((const pl_menu_item*)item)->id);
            free(path);

            return 1;
        }

        pspUiAlert("ERROR: State failed to load");
    }

    free(path);
    return 0;
}

int OnSaveStateButtonPress(const PspUiGallery *gallery, pl_menu_item *sel, uint32_t button_mask)
{
    if (!GameName) 
    { 
        TabIndex++; 
        return 0; 
    }

    if (button_mask & PSP_CTRL_SQUARE || button_mask & PSP_CTRL_TRIANGLE)
    {
        char *path;
        char caption[32];
        const char *config_name = pl_file_get_filename(GameName);
        pl_menu_item *item = pl_menu_find_item_by_id(&gallery->Menu, sel->id);

        path = (char*)malloc(strlen(SaveStatePath) + strlen(config_name) + 8);
        sprintf(path, "%s%s.s%02i", SaveStatePath, config_name, item->id);

        do /* not a real loop; flow control construct */
        {
            if (button_mask & PSP_CTRL_SQUARE)
            {
                if (pl_file_exists(path) && !pspUiConfirm("Overwrite existing state?"))
                    break;

                pspUiFlashMessage("Saving, please wait...");

                // TODO: make this save the state
                if (false) // !state_store(path))
                {
                    pspUiAlert("ERROR: State not saved");
                    break;
                }

                PspImage *icon = LoadStateIcon(path);
                SceIoStat stat;

                /* Trash the old icon (if any) */
                if (item->param && item->param != NoSaveIcon)
                    pspImageDestroy((PspImage*)item->param);

                /* Update icon, help text */
                item->param = icon;
                pl_menu_set_item_help_text(item, PresentSlotText);

                /* Get file modification time/date */
                if (sceIoGetstat(path, &stat) < 0)
                    sprintf(caption, "ERROR");
                else
                    sprintf(caption, "%02i/%02i/%02i %02i:%02i",
                        stat.st_mtime.month,
                        stat.st_mtime.day,
                        stat.st_mtime.year - (stat.st_mtime.year / 100) * 100,
                        stat.st_mtime.hour,
                        stat.st_mtime.minute);

                pl_menu_set_item_caption(item, caption);
            }
            else if (button_mask & PSP_CTRL_TRIANGLE)
            {
                if (!pl_file_exists(path) || !pspUiConfirm("Delete state?"))
                    break;

                if (!pl_file_rm(path))
                {
                    pspUiAlert("ERROR: State not deleted");
                    break;
                }

                /* Trash the old icon (if any) */
                if (item->param && item->param != NoSaveIcon)
                    pspImageDestroy((PspImage*)item->param);

                /* Update icon, caption */
                item->param = NoSaveIcon;
                pl_menu_set_item_help_text(item, EmptySlotText);
                pl_menu_set_item_caption(item, "Empty");
            }
        } while (0);

        if (path) 
            free(path);

        return 0;
    }

    return OnGenericButtonPress(NULL, NULL, button_mask);
}

int OnMenuItemChanged(const struct PspUiMenu *uimenu, pl_menu_item* item, const pl_menu_option* option)
{
    if (uimenu == &ControlUiMenu)
    {
        unsigned int value = (unsigned int)option->value;
        ActiveConfig.ButtonMap[item->id] = value;
    }
    else if (uimenu == &SystemUiMenu)
    {
        // TODO: make this control sound emulation
        mute = !(int)option->value;
    }
    else if (uimenu == &OptionUiMenu)
    {
        int value = (int)option->value;

        switch (item->id)
        {
        case OPTION_DISPLAY_MODE:
            Settings.DisplayMode = value; 
            break;
        case OPTION_SYNC_FREQ:
            Settings.UpdateFreq = value; 
            break;
        case OPTION_FRAMESKIP:
            Settings.Frameskip = value; 
            break;
        case OPTION_VSYNC:
            Settings.VSync = value; 
            break;
        case OPTION_CLOCK_FREQ:
            Settings.ClockFreq = value; 
            break;
        case OPTION_SHOW_FPS:
            Settings.ShowFps = value; 
            break;
        case OPTION_CONTROL_MODE:
            Settings.ControlMode = value;
            UiMetric.OkButton = (!value) ? PSP_CTRL_CROSS : PSP_CTRL_CIRCLE;
            UiMetric.CancelButton = (!value) ? PSP_CTRL_CIRCLE : PSP_CTRL_CROSS;
            break;
        case OPTION_ANIMATE:
            UiMetric.Animate = value; 
            break;
        }
    }

    SaveOptions();

    return 1;
}

int OnMenuOk(const void *uimenu, const void* sel_item)
{
    if (uimenu == &ControlUiMenu)
    {
        /* Save to MS */
        if (SaveButtonConfig())
            pspUiAlert("Changes saved");
        else
            pspUiAlert("ERROR: Changes not saved");
    }
    else if (uimenu == &SystemUiMenu)
    {
        switch (((const pl_menu_item*)sel_item)->id)
        {
        case SYSTEM_RESET:

            /* Reset system */
            if (pspUiConfirm("Reset the system?"))
            {
                ResumeEmulation = 1;
                
                // TODO: reset the console
                // reset();
                return 1;
            }
            break;

        case SYSTEM_SCRNSHOT:

            /* Save screenshot */
            if (!pl_util_save_image_seq(ScreenshotPath, pl_file_get_filename(GameName), Screen))
                pspUiAlert("ERROR: Screenshot not saved");
            else
                pspUiAlert("Screenshot saved successfully");
            break;
        }
    }

    return 0;
}

int OnMenuButtonPress(const struct PspUiMenu *uimenu, pl_menu_item* sel_item, uint32_t button_mask)
{
    if (uimenu == &ControlUiMenu)
    {
        if (button_mask & PSP_CTRL_TRIANGLE)
        {
            pl_menu_item *item;
            int i;

            /* Load default mapping */
            InitButtonConfig();

            /* Modify the menu */
            for (item = ControlUiMenu.Menu.items, i = 0; item; item = item->next, i++)
                pl_menu_select_option_by_value(item, (void*)DefaultConfig.ButtonMap[i]);

            return 0;
        }
    }

    return OnGenericButtonPress(NULL, NULL, button_mask);
}

/* Initialize game configuration */
static void InitButtonConfig()
{
    memcpy(&ActiveConfig, &DefaultConfig, sizeof(struct ButtonConfig));
}

/* Load game configuration */
static int LoadButtonConfig()
{
    char *path;

    if (!(path = (char*)malloc(sizeof(char) * (strlen(pl_psp_get_app_directory())
        + strlen(ButtonConfigFile) + 6)))) 
        return 0;

    sprintf(path, "%s%s.cnf", pl_psp_get_app_directory(), ButtonConfigFile);

    /* Open file for reading */
    SceUID file = sceIoOpen(path, PSP2_O_RDONLY, 0777);
    free(path);

    /* If no configuration, load defaults */
    if (!file)
    {
        InitButtonConfig();
        return 1;
    }

    /* Read contents of struct */
    int nread = sceIoRead(file, &ActiveConfig, sizeof(struct ButtonConfig));
    sceIoClose(file);

    if (nread != 1)
    {
        InitButtonConfig();
        return 0;
    }

    return 1;
}

/* Save game configuration */
static int SaveButtonConfig()
{
    char *path;

    if (!(path = (char*)malloc(sizeof(char) * (strlen(pl_psp_get_app_directory())
        + strlen(ButtonConfigFile) + 6)))) 
        return 0;

    sprintf(path, "%s%s.cnf", pl_psp_get_app_directory(), ButtonConfigFile);

    /* Open file for writing */
    SceUID file = sceIoOpen(path, PSP2_O_WRONLY | PSP2_O_CREAT, 0777);
    free(path);

    if (!file) 
        return 0;

    /* Write contents of struct */
    int nwritten = sceIoWrite(file, &ActiveConfig, sizeof(struct ButtonConfig));
    sceIoClose(file);

    return (nwritten == 1);
}

/* Load state icon */
PspImage* LoadStateIcon(const char *path)
{
    /* Open file for reading */
    SceUID f = sceIoOpen(path, PSP2_O_RDONLY, 0777);

    if (!f) 
        return NULL;

    /* Load image */
    PspImage *image = pspImageLoadPngSCE(f);
    sceIoClose(f);

    return image;
}

static void DisplayStateTab()
{
    if (!GameName) 
    { 
        TabIndex++; 
        return; 
    }

    pl_menu_item *item;
    SceIoStat stat;
    char caption[32];

    const char *config_name = pl_file_get_filename(GameName);
    char *path = (char*)malloc(strlen(SaveStatePath) + strlen(config_name) + 8);
    char *game_name = strdup(config_name);
    char *dot = strrchr(game_name, '.');

    if (dot) 
        *dot = '\0';

    /* Initialize icons */
    for (item = SaveStateGallery.Menu.items; item; item = item->next)
    {
        sprintf(path, "%s%s.s%02i", SaveStatePath, config_name, item->id);

        if (pl_file_exists(path))
        {
            if (sceIoGetstat(path, &stat) < 0)
                sprintf(caption, "ERROR");
            else
                sprintf(caption, "%02i/%02i/%02i %02i:%02i",
                    stat.st_mtime.month,
                    stat.st_mtime.day,
                    stat.st_mtime.year - (stat.st_mtime.year / 100) * 100,
                    stat.st_mtime.hour,
                    stat.st_mtime.minute);

            pl_menu_set_item_caption(item, caption);
            item->param = LoadStateIcon(path);
            pl_menu_set_item_help_text(item, PresentSlotText);
        }
        else
        {
            pl_menu_set_item_caption(item, "Empty");
            item->param = NoSaveIcon;
            pl_menu_set_item_help_text(item, EmptySlotText);
        }
    }

    free(path);
    pspUiOpenGallery(&SaveStateGallery, game_name);
    free(game_name);

    /* Destroy any icons */
    for (item = SaveStateGallery.Menu.items; item; item = item->next)
        if (item->param != NULL && item->param != NoSaveIcon)
            pspImageDestroy((PspImage*)item->param);
}

/* Handles any special drawing for the system menu */
void OnSystemRender(const void *uiobject, const void *item_obj)
{
    int w, h, x, y;
    w = Screen->Viewport.Width*1.5;
    h = Screen->Viewport.Height*1.5;
    x = SCR_WIDTH - w - 16;
    y = SCR_HEIGHT - h - 80;

    /* Draw a small representation of the screen */
    pspVideoShadowRect(x, y, x + w - 1, y + h - 1, PSP_COLOR_BLACK, 3);
    pspVideoPutImage(Screen, x, y, w, h);
    pspVideoDrawRect(x, y, x + w - 1, y + h - 1, PSP_COLOR_GRAY);

    OnGenericRender(uiobject, item_obj);
}

void TrashMenu()
{
    /* Unload ROM */
    if (GameName)
    {
        // TODO: unload the ROM from the emulator
        // system_unload_rom();
        free(GameName);
    }

    // TODO: Free emulation-specific resources
    // TrashEmulation();

    /* Save options */
    SaveOptions();

    /* Free local resources */
    pspImageDestroy(Background);
    pspImageDestroy(NoSaveIcon);

    pl_menu_destroy(&ControlUiMenu.Menu);
    pl_menu_destroy(&SaveStateGallery.Menu);
    pl_menu_destroy(&OptionUiMenu.Menu);
    pl_menu_destroy(&SystemUiMenu.Menu);
}
