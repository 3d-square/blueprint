#pragma once
#include "raylib.h"
#include "ui.h"
#include <dirent.h>
#include <sys/types.h>
#include <linux/limits.h>

#define F_DIR 0
#define F_FILE 1

/* files include dirs for this struct */
typedef struct _filedir{
   char **file_names;
   int *file_types;
   int num_files;
} FILEDIR;

typedef struct _filewin{
   int x, y;
   int width, height;
   FILEDIR cwd;
   char load_file[PATH_MAX];
   BUTTON load;
   BUTTON close;
   int visible;
} FILEWIN;

void init_filewin(FILEWIN *filwwin);
void draw_filewin(FILEWIN *filewin);
int num_files(DIR *dp);
int read_all_files(FILEDIR *filedir, char *path);
void free_all_files(FILEDIR *filedir);
void is_dir(char *path);
