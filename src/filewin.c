#include "raylib.h"
#include "ui.h"
#include "filewin.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>

void init_filewin(FILEWIN *filewin){
   memcpy(filewin, 0, sizeof(FILEWIN));
}

void draw_filewin(FILEWIN *filewin){

}

void set_cwd(FILEWIN *filewin){
   getcwd(filewin->directory.absolute_path, sizeof(filewin->directory.absolute_path));
}

void update_filewin_info(FILEWIN *filewin){
   get_directory_info(&filewin->directory);
}
