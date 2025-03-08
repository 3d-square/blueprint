#include "directory_info.h"
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <sys/stat.h>

char *ftype_str(F_TYPE type){
   switch(type){
      case BLK_DEV:   return "BLK_DEV";
      case CHAR_DEV:  return "CHAR_DEV";
      case DIRECTORY: return "DIRECTORY";
      case SYMLINK:   return "SYMLINK";
      case RG_FILE:   return "RG_FILE";
      case FIFO:      return "FIFO";
      case SOCKET:    return "SOCKET";
   }

   return "";
}

void file_permissions(char *filename, F_TYPE *file_type){
   struct stat sb;
   stat(filename, &sb);
	switch (sb.st_mode & S_IFMT) {
      case S_IFBLK: *file_type = BLK_DEV; break;
      case S_IFCHR: *file_type = CHAR_DEV; break;
      case S_IFDIR: *file_type = DIRECTORY; break;
      case S_IFIFO: *file_type = FIFO; break;
      case S_IFLNK: *file_type = SYMLINK; break;
      case S_IFREG: *file_type = RG_FILE; break;
      case S_IFSOCK:*file_type = SOCKET; break;
      default:printf("unknown?\n");break;
   }
}

void set_directory_path(struct dir_info *directory, char path[MAX_PATH + 1]){
   strcpy(directory->absolute_path, path);
}

int get_directory_info(struct dir_info *directory){
   DIR *dir;
   struct dirent *ent;
   char buffer_path[2 + MAX_PATH * 2];
    directory->count = 0;
  
   if ((dir = opendir (directory->absolute_path)) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
         if(strcmp(ent->d_name, ".") == 0) continue;

         sprintf(buffer_path, "%s/%s", directory->absolute_path, ent->d_name);
         strcpy(directory->files[directory->count].name, ent->d_name);
         file_permissions(buffer_path, &directory->files[directory->count].type);
         
         printf("[%-9.9s][%s]\n", ftype_str(directory->files[directory->count].type), directory->files[directory->count].name);
         directory->count++;
      }
      closedir (dir);
   } else{
      /* could not open directory */
      perror("Unable to open directory");
      return 1;
   }

   return 0;
}

char *files[MAX_FILES] = {0};

void free_files_array(){
   for(int i = 0; i < MAX_FILES; ++i){
      free(files[i]);
   }
}

char **files_to_char_array(struct dir_info *directory){
   free_files_array(files);
   memset(files, 0, MAX_FILES * sizeof(char *));
   for(int i = 0; i < directory->count; ++i){
      files[i] = strdup(directory->files[i].name);
   }

   return files;
}
