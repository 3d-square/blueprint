#include "globals.h"
#include "options.h"
#include "button.h"
#include "raylib.h"
#include "cutils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OPTION_MENU create_menu(int x, int y, int w, int h, char *text, char **buttons, int num_buttons, int focus){
   size_t size = sizeof(BUTTON) * num_buttons;
   int button_height = GetFontDefault().baseSize + 4;

   OPTION_MENU result = {
      .x = x,
      .y = y,
      .width = w,
      .height = h,
      .text = text,
      .num_buttons = num_buttons,
      .selected = -1,
      .visible = 0,
      .close = create_button(0, 0, 15, 15, "X", 15),
      .focus = focus,
   };  
   
   result.buttons = malloc(size);
   assert(result.buttons);

   for(int i = 0; i < num_buttons; ++i){
      result.buttons[i] = create_button(0, 0,
                                        w - 2,
                                        button_height, 
                                        buttons[i], 10);
   }
   
   return result;
}

void delete_menu(OPTION_MENU *menu){
   free(menu->buttons);
   menu->buttons = NULL;
}

void set_visible(OPTION_MENU *menu){
   int button_height = GetFontDefault().baseSize + 6;
   int offset = mouse_position.y + 25;

   menu->x = mouse_position.x;
   menu->y = mouse_position.y;
   menu->visible = 1;
   menu->close.x = menu->x + menu->width - 16;
   menu->close.y = menu->y + 1;

   for(int i = 0; i < menu->num_buttons; ++i){
      menu->buttons[i].x = mouse_position.x + 1;
      menu->buttons[i].y = offset + (button_height * i);
   }
}

void set_invisible(OPTION_MENU *menu){
   menu->visible = 0;
   menu->selected = 0;
}

void draw_menu(OPTION_MENU *menu){
   if(menu->visible != 1) return;
   DrawRectangle(menu->x, menu->y, menu->width, menu->height, GRAY);
   DrawRectangleLines(menu->x, menu->y, menu->width, menu->height, BLACK);
   DrawText(menu->text, menu->x, menu->y, 15, BLACK);
   draw_button(&menu->close);
   
   for(int i = 0; i < menu->num_buttons; ++i){
      draw_button(&menu->buttons[i]);
   }
   
}
int update_menu(OPTION_MENU *menu){
   if(menu->visible != 1){
      fprintf(stderr, "Unable to update a menu that is invisibel\n");
      set_global_message("Window Invisible");
      return -69;
   }
   if(!is_mouse_collision(menu->x, menu->y ,menu->width ,menu->height)) return menu->focus;

   if(button_activate(&menu->close, MOUSE_BUTTON_LEFT)){
      set_invisible(menu);
      return RESET_FOCUS;
   }

   for(int i = 0; i < menu->num_buttons; ++i){
      if(button_activate(&menu->buttons[i], MOUSE_BUTTON_LEFT)){
         menu->selected = i;
         return MENU_BUTTON_CLICKED;
      }
   }
   
   return menu->focus;
}

int get_selected(OPTION_MENU *menu){
   return menu->selected;
}
