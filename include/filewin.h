#pragma once
#include "raylib.h"
#include "ui.h"
#include <dirent.h>
#include <sys/types.h>
#include <linux/limits.h>
#include "directory_info.h"

typedef struct _filewin{
   int x, y;
   int width, height;
   struct dir_info directory;
   char load_file[PATH_MAX];
   BUTTON load;
   BUTTON close;
   OPTION_PANEL buttons;
   int visible;
} FILEWIN;


void init_filewin(FILEWIN *filwwin, int x, int y, int width, int height);
void draw_filewin(FILEWIN *filewin);
void set_cwd(FILEWIN *filewin);
int update_filewin_info(FILEWIN *filewin);
void uninit_filewin(FILEWIN *file);
