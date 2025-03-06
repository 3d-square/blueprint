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
   BUTTON dir_buttons[MAX_FILES];
   char load_file[PATH_MAX];
   BUTTON load;
   BUTTON close;
   int visible;
} FILEWIN;

void init_filewin(FILEWIN *filwwin);
void draw_filewin(FILEWIN *filewin);
void set_cwd(FILEWIN *filewin);
void update_filewin_info(FILEWIN *filewin);
