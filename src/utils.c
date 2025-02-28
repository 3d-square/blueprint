#include "utils.h"
#include "globals.h"
#include <string.h>
#include <stdlib.h>

int point_in_rect(int x, int y, int rx, int ry, int rw, int rh){
   return (x >= rx) && (y >= ry) && (x <= rx + rw) && (y <= ry + rh);
}

int is_mouse_collision(int rx, int ry, int rw, int rh){
   return point_in_rect(mouse_position.x, mouse_position.y, rx, ry, rw, rh);
}

int str2i(char *str, int *res){
   char *end;

   *res = (int)strtol(str, &end, 10);

   if(*end != '\0'){
      return 0;
   }

   return 1;
}

int str_to_coord(char *str, int *x, int *y){
   char x_coord[9] = {0};
   char y_coord[9] = {0};
   int xc, yc;
   char *comma = strchr(str, ',');

   if(strchr(comma + 1, ',')){
      set_global_message("Invalid coordinate");
      return 0;
   }

   strcpy(y_coord, comma + 1);
   strncpy(x_coord, str, comma - str);

   if(!(str2i(x_coord, &xc) && str2i(y_coord, &yc))){
      set_global_message("Unable to convert");
      return 0;
   }
   *x = xc;
   *y = yc;   

   return 1;
   
}

void update_text_box(char *text, int length){
   int key = GetCharPressed();
   int tmp_len = strlen(text);
   while(key > 0){
      if((key >= 32) && (key <= 125) && (tmp_len < length)){
         text[tmp_len++] = (char)key;
      }
      key = GetCharPressed();
   }
   text[tmp_len] = '\0';
   if(tmp_len > 0 && IsKeyPressed(KEY_BACKSPACE)){
      tmp_len -= 1;
      text[tmp_len] = '\0';
   }
}
