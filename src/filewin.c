#include "raylib.h"
#include "ui.h"
#include "application.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <linux/limits.h>
#include <string.h>

extern char file_name[64];

void init_filewin(FILEWIN *filewin, int x, int y, int width, int height){
   int button_height = GetFontDefault().baseSize + 4;
   memset(filewin, 0, sizeof(FILEWIN));
   filewin->x = x;
   filewin->y = y;
   filewin->width = width;
   filewin->height = height;
   filewin->load = create_button(x, y + height - 30, width, 30, "Load Model", 15, WHITE, CENTER_JUSTIFY);
   filewin->close = create_button(x + width - 15, y, 15, 15, "X", 15, WHITE, CENTER_JUSTIFY);
   filewin->visible = 0;
   filewin->buttons.selected = -1;

   set_cwd(filewin); // update current directory to .
   get_directory_info(&filewin->directory); // update directory info

   recreate_buttons(&filewin->buttons.buttons, filewin->x, filewin->y + 30, filewin->width - 2, button_height, filewin->directory.file_names, filewin->directory.count); // create buttons for each of the files in the directory
   filewin->buttons.num_buttons = filewin->directory.count; // update number of buttons

}

void draw_filewin(FILEWIN *filewin){
   if(!filewin->visible) return;

   DrawRectangle(filewin->x, filewin->y, filewin->width, filewin->height, GRAY);
   DrawRectangleLines(filewin->x, filewin->y, filewin->width, filewin->height, BLACK);
   DrawRectangleLines(filewin->x, filewin->y, filewin->width, GetFontDefault().baseSize + 10, BLACK);
   DrawText(filewin->load_file, filewin->x + 3, filewin->y + 2, 15, WHITE);

   draw_button(&filewin->load);
   draw_button(&filewin->close);
   draw_option_panel(&filewin->buttons);
}

void uninit_filewin(FILEWIN *filewin){
   delete_option_panel(&filewin->buttons);
}

void set_cwd(FILEWIN *filewin){
   getcwd(filewin->directory.absolute_path, sizeof(filewin->directory.absolute_path));
}

int update_filewin_info(FILEWIN *filewin, GEN_FLOW *graph[], int *num_nodes){
   if(!filewin->visible) return 1;
   for(int i = 0; i < filewin->directory.count; ++i){
      if(filewin->directory.files[i].type == DIRECTORY){
         filewin->buttons.buttons[i].textcolor = BLUE;
      }
   }


   if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
      int button_height = GetFontDefault().baseSize + 4;
      if(button_collision(&filewin->close)){
         filewin->visible = 0;
         return 1;
      }else if(button_collision(&filewin->load)){
         strcpy(file_name, filewin->load_file);
         printf("loding %s\n", file_name);
         free_graph(graph, *num_nodes);
         clear_branch_panel();
         clear_node_panel();
         int status = load_model(graph, num_nodes);
   
         if(status == -1){
            set_global_message("Unable to open file");
         }
      }
      for(int i = 0; i < filewin->directory.count; ++i){
         if(button_collision(&filewin->buttons.buttons[i])){
            filewin->buttons.selected = i;
            break;
         }
      }  
      if(filewin->buttons.selected != -1){
         struct file_info *info = &filewin->directory.files[filewin->buttons.selected];

         if(info->type == DIRECTORY){
            if(strcmp(info->name, "..") == 0){
               char *slash = strrchr(filewin->directory.absolute_path, '/');
               if(slash){
                  *slash = '\0';
               }
            }else{
               strcat(filewin->directory.absolute_path, "/");
               strcat(filewin->directory.absolute_path, info->name);
            }

            get_directory_info(&filewin->directory);

            recreate_buttons(&filewin->buttons.buttons, filewin->x, filewin->y + 30, filewin->width - 2, button_height, filewin->directory.file_names, filewin->directory.count);
            filewin->buttons.num_buttons = filewin->directory.count; // update number of buttons
         }else{
            sprintf(filewin->load_file, "%s/%s", filewin->directory.absolute_path, info->name);
         }
      }
   }
   return 0;
}
