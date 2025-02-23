#include "manager.h"
#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include <stdlib.h>
#include "cutils.h"

void draw_window(OPTION_WINDOW *window){
   DrawRectangle(window->x, window->y, window->width, window->height, GRAY);
   DrawRectangleLines(window->x, window->y, window->width, window->height, GRAY);
}

OPTION_WINDOW create_window(char *name, int x, int y, int w, int h, char *options[] /* NULL TERMINDATED */){
   int i;
   OPTION_WINDOW window = {
      .name = name,
      .x = x,
      .y = y,
      .width = w,
      .height = h,
      .show = 0,
      .selected = -1
   };

   for(i = 0; options[i]; ++i){
      ;
   }

   window.num_options = i;

   return window;
}

WIN_FOCUS select_focus(OPTION_WINDOW *windows, int num_windows, int *selected){
   for(int i = 0; i < num_windows; ++i){
      if(windows[i].show == 0) continue;
      Vector2 mouse_pos = GetMousePosition();
      if(point_in_rect(mouse_pos.x, mouse_pos.y, windows[i].x, windows[i].y, windows[i].width, windows[i].height)){
         *selected = i;
         return OPTION;
      }
   }

   return MAIN;
}
