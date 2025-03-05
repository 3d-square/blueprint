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

int num_files(DIR *dp){
   if(dp == NULL) return 0;

   int count = 0;
   struct dirent *dent;

   while((dent = readdir(dp)) != NULL){
      count++;
   }  

   rewinddir(dp);
   return count;
}

int read_all_files(FILEDIR *filedir, char *path){
   DIR *dp = opendir(path);

   if(dp == NULL){
      fprintf(stderr, "Unable to open %s\n", path);
      return -1;
   }
   int num_entries = num_files(dp);

   
}

void free_all_files(FILEDIR *filedir){

}

void is_dir(char *path);
