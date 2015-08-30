// Author: skogaby
// Heavily modified version of file_chooser written by xerpi:
// https://github.com/xerpi/realboy-vita

#include "file_chooser.h"

/***
 * Adds an entry to the given file list.
 */
static void file_list_add_entry(file_list *list, file_list_entry *entry)
{
    if (list->head == NULL)
    {
        list->head = entry;
        entry->next = NULL;
    }
    else
    {
        file_list_entry *tmp = list->head;
        list->head = entry;
        entry->next = tmp;
    }

    list->length++;
}

/***
 * Empties out the given file list and frees its memory.
 */
static void file_list_empty(file_list *list)
{
    file_list_entry *p = list->head;
    file_list_entry *q;

    while (p)
    {
        q = p->next;
        free(p);
        p = q;
    }

    memset(list, 0, sizeof(*list));
}

/***
 * Says whether or not the given filename is supported, according
 * to our given file extensions.
 */
static int file_supported(const char *filename, const char *supported_ext[])
{
    int i;
    const char *ext = strrchr(filename, '.');

    if (ext)
    {
        i = 0;

        while (supported_ext[i])
        {
            if (strcmp(ext + 1, supported_ext[i]) == 0)
            {
                return 1;
            }

            i++;
        }
    }

    return 0;
}

/***
 * Builds the file list from the given path and the supported extensions.
 */
static int file_list_build(const char *path, file_list *list, const char *supported_ext[])
{
    SceUID dir;
    SceIoDirent dirent;

    dir = sceIoDopen(path);

    if (dir < 0) 
    {
        return 0;
    }

    memset(&dirent, 0, sizeof(dirent));
    memset(list, 0, sizeof(*list));

    // get the list of files in our current path
    while (sceIoDread(dir, &dirent) > 0)
    {
        file_list_entry *entry = malloc(sizeof(*entry));
        strcpy(entry->name, dirent.d_name);
        entry->is_dir = PSP2_S_ISDIR(dirent.d_stat.st_mode);

        if (!entry->is_dir) 
        {
            entry->supported = file_supported(entry->name, supported_ext);
        }

        file_list_add_entry(list, entry);
        memset(&dirent, 0, sizeof(dirent));
    }

    sceIoDclose(dir);

    // add an entry to go up one directory
    file_list_entry *up = malloc(sizeof(*up));
    strcpy(up->name, "..");
    up->is_dir = 1;
    up->next = NULL;
    file_list_add_entry(list, up);

    return 0;
}

/***
 * Gets the Nth entry of the file list
 */
static file_list_entry *file_list_get_nth_entry(const file_list *list, int n)
{
    file_list_entry *entry = list->head;

    while (n--) 
    {
        entry = entry->next;
    }

    return entry;
}

/***
 * Goes up one directory from the given path, or
 * to the root of the volume otherwise.
 */
static void dir_up(char *path)
{
    char *pch;
    size_t len_in = strlen(path);

    if (len_in == 1) 
    {
        strcpy(path, "/");
        return;
    }

    if (path[len_in - 1] == '/') 
    {
        path[len_in - 1] = '\0';
    }

    pch = strrchr(path, '/');

    if (pch) 
    {
        size_t s = len_in - (pch - path);
        memset(pch + 1, '\0', s);
    }

    if (strcmp(path, "cache0:/") < 0) 
    {
        strcpy(path, "cache0:/");
    }
}

/***
 * Draws the buttons for going up or down a page.
 */
static void draw_page_button(const int offset_x, const int offset_y, const int up)
{
    vita2d_draw_rectangle(offset_x + 10, offset_y + 10, PAGE_BUTTON_W - 20, PAGE_BUTTON_H - 20, WHITE);
    vita2d_draw_rectangle(offset_x + 15, offset_y + 15, PAGE_BUTTON_W - 30, PAGE_BUTTON_H - 30, BLACK);

    if (up)
    {
        font_draw_string(offset_x + 34, offset_y + 119, WHITE, "UP");
    }
    else
    {
        font_draw_string(offset_x + 18, offset_y + 119, WHITE, "DOWN");
    }
}

/***
 * Draws the buttons for selecting a game, given its game title.
 */
static void draw_game_button(const int offset_x, const int offset_y, const char* title, int supported)
{
    vita2d_draw_rectangle(offset_x + 10, offset_y + 10, GAME_BUTTON_W - 20, GAME_BUTTON_H - 20, WHITE);
    vita2d_draw_rectangle(offset_x + 15, offset_y + 15, GAME_BUTTON_W - 30, GAME_BUTTON_H - 30, BLACK);

    // draw the game title, spanning multiple rows if needed
    int row = 0;
    int col = 0;
    int i = 0;

    for (i = 0; title[i] != 0 && row < 6; i++)
    {
        font_draw_char(offset_x + 17 + (16 * col), offset_y + 17 + (16 * row), 
            supported ? GREEN : WHITE, title[i]);
        col++;

        if (col >= 11)
        {
            col = 0;
            row++;
        }
    }
}

/***
 * Gets the button that's pressed by the user, if any, on the touch screen.
 * -1 means nothing is pressed.
 * 0-15 are for the game buttons
 * 16 is the up button
 * 17 is the down button
 */
static int get_pressed_button(SceTouchData touch_pad)
{
    if (touch_pad.reportNum < 1)
    {
        return -1;
    }

    // we only consider the first touch in the array when determining button presses
    SceTouchReport report = touch_pad.report[0];
    int pos_x = report.x / 2;
    int pos_y = report.y / 2;

    // if the y is above 36, it can't possibly be pressing any buttons
    if (pos_y <= 36)
    {
        return -1;
    }
    // if the x is less than the screen width minus the page button width, it's a 
    // game button press
    else if (pos_x <= SCREEN_W - PAGE_BUTTON_W)
    {
        int col = floor(pos_x / GAME_BUTTON_W);
        int row = floor((pos_y - 36) / GAME_BUTTON_H);

        return (col + (row * GAME_ROWS));
    }
    // otherwise, the user has to be pressing a page button
    else
    {
        // user is pressing page up
        if (pos_y <= 36 + PAGE_BUTTON_H)
        {
            return BUTTON_PAGE_UP;
        }
        else
        {
            return BUTTON_PAGE_DOWN;
        }
    }
}

/***
* Main method that lets the user choose a file based on the given
* start path and supported file extensions.
*/
int file_choose(const char *start_path, char *chosen_file, const char *title, const char *supported_ext[])
{
    int i;
    int row, col;
    static char cur_path[PATH_MAX];
    int button = -1;
    int old_button = -1;
    int curr_page = 0;
    SceTouchData touch_pad;

    strcpy(cur_path, start_path);

    file_list list;
    file_list_entry *entry;

    // build the list of files in our current directory
    file_list_build(cur_path, &list, supported_ext);

    // just keep looping until the user actually picks a file
    while (1)
    {
        // get the current touch data
        sceTouchPeek(0, &touch_pad, 1);
        old_button = button;
        button = get_pressed_button(touch_pad);

        // handle the user input
        if (button != old_button && old_button != -1)
        {
            // user pressed a page button
            if (old_button == BUTTON_PAGE_UP)
            {
                if (curr_page > 0)
                    curr_page--;
            }
            else if (old_button == BUTTON_PAGE_DOWN)
            {
                if (((curr_page + 1) * GAME_ROWS * GAME_COLS) < list.length)
                    curr_page++;
            }
            // user pressed a game button
            else
            {
                int selected = old_button + (curr_page * GAME_ROWS * GAME_COLS);
                file_list_entry *entry = file_list_get_nth_entry(&list, selected);
                
                if (entry->is_dir)
                {
                    // go up one directory
                    if (strcmp(entry->name, "..") == 0)
                    {
                        dir_up(cur_path);
                    }
                    // otherwise, go one directory down to the new directory
                    else
                    {
                        char new_path[PATH_MAX];
                        sprintf(new_path, "%s/%s", cur_path, entry->name);
                        strcpy(cur_path, new_path);
                    }

                    // build the new file list based on the new directory
                    file_list_empty(&list);
                    file_list_build(cur_path, &list, supported_ext);
                    curr_page = 0;
                }
                // if the user selected a supported file, return the file path
                // and exit the function
                else if (entry->supported)
                {
                    sprintf(chosen_file, "%s/%s", cur_path, entry->name);
                    file_list_empty(&list);

                    return 1;
                }
            }
        }

        // now we need to actually draw the file list
        vita2d_start_drawing();
        vita2d_clear_screen();

        font_draw_stringf(10, 10, WHITE, title);

        // first, draw the up and down page buttons
        draw_page_button(SCREEN_W - PAGE_BUTTON_W, 36, true);
        draw_page_button(SCREEN_W - PAGE_BUTTON_W, 36 + PAGE_BUTTON_H, false);

        entry = file_list_get_nth_entry(&list, (curr_page * GAME_ROWS * GAME_COLS));
        row = 0;
        col = 0;

        for (i = (curr_page * GAME_ROWS * GAME_COLS); i < list.length && row < GAME_ROWS; i++)
        {
            draw_game_button(col * GAME_BUTTON_W, (row * GAME_BUTTON_H) + 36, entry->name, entry->supported);
            col++;

            if (col >= GAME_COLS)
            {
                col = 0;
                row++;
            }

            entry = entry->next;
        }
        
        vita2d_end_drawing();
        vita2d_swap_buffers();
    }

    return 0;
}
