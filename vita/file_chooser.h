// Author: skogaby
// Heavily modified version of file_chooser written by xerpi:
// https://github.com/xerpi/realboy-vita

#ifndef FILE_CHOOSER_H
#define FILE_CHOOSER_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/syslimits.h>
#include <psp2/types.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include <psp2/kernel/processmgr.h>

#include <vita2d.h>

#include "font.h"
#include "vita_video.h"

#define BLACK 0xFF000000
#define WHITE 0xFFFFFFFF
#define GREEN 0xFF00FF00

#define GAME_BUTTON_W 215
#define GAME_BUTTON_H 127
#define PAGE_BUTTON_W 100
#define PAGE_BUTTON_H 254
#define GAME_ROWS 4
#define GAME_COLS 4
#define BUTTON_PAGE_UP 16
#define BUTTON_PAGE_DOWN 17

/***
 * Represents an entry in a file list, either a file or a directory.
 */
typedef struct file_list_entry 
{
    char name[PATH_MAX];
    int is_dir;
    int supported;
    struct file_list_entry *next;
} file_list_entry;

/***
 * Represents a list of either files or directories, implemented
 * as a linked list.
 */
typedef struct file_list 
{
    file_list_entry *head;
    unsigned int length;
} file_list;

// helpers for building file paths
static void file_list_add_entry(file_list *list, file_list_entry *entry);
static void file_list_empty(file_list *list);
static int file_supported(const char *filename, const char *supported_ext[]);
static int file_list_build(const char *path, file_list *list, const char *supported_ext[]);
static file_list_entry *file_list_get_nth_entry(const file_list *list, int n);
static void dir_up(char *path);

// helpers for drawing the crude touch-based menu
static void draw_page_button(const int offset_x, const int offset_y, const int up);
static void draw_game_button(const int offset_x, const int offset_y, const char* title, int supported);
static int get_pressed_button(SceTouchData touch_pad);

// main entrypoint for file selection
int file_choose(const char *start_path, char *chosen_file, const char *title, const char *supported_ext[]);

#endif
