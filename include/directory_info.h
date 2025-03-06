#pragma once

#define MAX_PATH 255
#define MAX_FILES 1048

typedef enum _ftype {
   BLK_DEV,
   CHAR_DEV,
   DIRECTORY,
   SYMLINK,
   RG_FILE,
   FIFO,
   SOCKET
} F_TYPE;

typedef enum _permissions{
   NONE   
} PERMISSIONS;

struct file_info {
   char name[MAX_PATH * 1];
   F_TYPE type;
};

struct dir_info{
   int count;
   struct file_info files[MAX_FILES];
   char absolute_path[MAX_PATH + 1];
};
void set_directory_path(struct dir_info *directory, char path[MAX_PATH + 1]);
int get_directory_info(struct dir_info *directory);
char *ftype_str(F_TYPE type);
void file_permissions(char *filename, F_TYPE *file_type);
