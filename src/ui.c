#include "raylib.h"
#include "application.h"
#include "cutils.h"
#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//
//      BUTTON FUNCTIONS
//

BUTTON create_button(int x, int y, int w, int h, char *text, int text_size, Color textcolor, JUSTIFY position){
   BUTTON result = {
      .x = x,
      .y = y,
      .width = w,
      .height = h,
      .text = text,
      .text_size = text_size,
      .textcolor = textcolor,
      .position = position,
   };
   Vector2 text_dim = MeasureTextEx(GetFontDefault(), text, text_size, 1.0);
   // printf("Text['%s'] Dim(%f,%f)\n", text, text_dim.x, text_dim.y);

   switch(position){
      case RIGHT_JUSTIFY:{
         result.text_x = x + w - ((int)text_dim.x + 1);
         result.text_y = y + 1;
      }break;
      case LEFT_JUSTIFY:{
         result.text_x = x + 2;
         result.text_y = y + 1;
      }break;
      case CENTER_JUSTIFY:{
         result.text_x = x + (w / 2) - ((int)text_dim.x / 2);
         result.text_y = y + (h / 2) - ((int)text_dim.y / 2);
      }break;
   }
   
   return result;
}

void draw_button(BUTTON *button){
   Color color = button_collision(button) ? LIGHTGRAY : DARKGRAY;
   // printf("B:%d, %d\n", button->text_x, button->text_y);

   DrawRectangle(button->x, button->y, button->width, button->height, color);
   DrawRectangleLines(button->x, button->y, button->width, button->height, BLACK);

   DrawText(button->text, button->text_x, button->text_y, button->text_size, button->textcolor);
}

int button_collision(BUTTON *button){
   return is_mouse_collision(button->x, button->y, button->width, button->height);
}

int button_activate(BUTTON *button, MouseButton btn){
   return button_collision(button) && IsMouseButtonPressed(btn);
}

//
//      OPTION MENU FUNCTIONS
//

OPTION_MENU create_option_menu(int x, int y, int w, int h, char *text, char **buttons, int num_buttons){
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
      .close = create_button(0, 0, 15, 15, "X", 15, WHITE, LEFT_JUSTIFY),
      .buttons = NULL,
   };  
   
   recreate_buttons(&result.buttons, 0, 0, w - 2, button_height, buttons, num_buttons);
   
   return result;
}

void recreate_buttons(BUTTON **buttons, int x, int y, int w, int h, char **buttons_text, int num_buttons){
   size_t size = sizeof(BUTTON) * num_buttons;

   BUTTON *new_buttons = realloc(*buttons, size);

   if(new_buttons == NULL){
      printf("Unable to allocate new buttons\n");
      exit(-1);
   }

   for(int i = 0; i < num_buttons; ++i){
      new_buttons[i] = create_button(x, (h * i) + y + (2 * i),
                                     w, h, 
                                     buttons_text[i], 10, WHITE, LEFT_JUSTIFY);
   }
   
   *buttons = new_buttons;
}

OPTION_PANEL create_option_panel(int x, int y, int w, int h, char *text, char **buttons, int num_buttons){
   int button_height = GetFontDefault().baseSize + 4;

   OPTION_PANEL result = {
      .x = x,
      .y = y,
      .width = w,
      .height = h,
      .text = text,
      .num_buttons = num_buttons,
      .selected = -1,
      .buttons = NULL,
   };  
    
   recreate_buttons(&result.buttons, 0, 0, w - 2, button_height, buttons, num_buttons);
   return result;
}

void delete_option_menu(OPTION_MENU *menu){
   free(menu->buttons);
   menu->buttons = NULL;
}

void delete_option_panel(OPTION_PANEL *menu){
   free(menu->buttons);
   menu->buttons = NULL;
}

void update_button_position(BUTTON *button, int x, int y){
      Vector2 text_dim = MeasureTextEx(GetFontDefault(), button->text, button->text_size, 1.0);
      button->x = x + 1;
      button->y = y;
      switch(button->position){
         case RIGHT_JUSTIFY:{
            button->text_x = x + button->width - ((int)text_dim.x + 1);
            button->text_y = y + 1;
         }break;
         case LEFT_JUSTIFY:{
            button->text_x = x + 2;
            button->text_y = y + 1;
         }break;
         case CENTER_JUSTIFY:{
            button->text_x = x + (button->width / 2) - ((int)text_dim.x / 2);
            button->text_y = y + (button->height / 2) - ((int)text_dim.y / 2);
         }break;
      }
}

void set_visible(OPTION_MENU *menu){
   int button_height = GetFontDefault().baseSize + 6;
   int offset = mouse_position.y + 25;

   menu->x = mouse_position.x;
   menu->y = mouse_position.y;
   menu->visible = 1;

   update_button_position(&menu->close, menu->x + menu->width - 16, menu->y + 1);

   for(int i = 0; i < menu->num_buttons; ++i){
      update_button_position(&menu->buttons[i], mouse_position.x, (i * button_height) + offset);
   }
}

void update_option_panel_position(OPTION_PANEL *menu){
   int button_height = GetFontDefault().baseSize + 6;
   int offset = mouse_position.y + 25;

   menu->x = mouse_position.x;
   menu->y = mouse_position.y;

   for(int i = 0; i < menu->num_buttons; ++i){
      update_button_position(&menu->buttons[i], mouse_position.x, (i * button_height) + offset);
   }


}

void set_invisible(OPTION_MENU *menu){
   menu->visible = 0;
   menu->selected = 0;
}

void draw_option_menu(OPTION_MENU *menu){
   if(menu->visible != 1) return;
   DrawRectangle(menu->x, menu->y, menu->width, menu->height, GRAY);
   DrawRectangleLines(menu->x, menu->y, menu->width, menu->height, BLACK);
   DrawText(menu->text, menu->x, menu->y, 15, BLACK);
   draw_button(&menu->close);
   
   for(int i = 0; i < menu->num_buttons; ++i){
      draw_button(&menu->buttons[i]);
   }
   
}

int update_option_menu(OPTION_MENU *menu){
   if(menu->visible != 1){
      fprintf(stderr, "Unable to update a menu that is invisibel\n");
      set_global_message("Window Invisible");
      return -69;
   }
   if(!is_mouse_collision(menu->x, menu->y ,menu->width ,menu->height)) return 0;

   if(button_activate(&menu->close, MOUSE_BUTTON_LEFT)){
      set_invisible(menu);
      return RESET_FOCUS;
   }

   for(int i = 0; i < menu->num_buttons; ++i){
      if(button_activate(&menu->buttons[i], MOUSE_BUTTON_LEFT)){
         menu->selected = i;
         return 1;
      }
   }
   
   return 0;
}

void draw_option_panel(OPTION_PANEL *menu){
   DrawRectangle(menu->x, menu->y, menu->width, menu->height, GRAY);
   DrawRectangleLines(menu->x, menu->y, menu->width, menu->height, BLACK);
   DrawText(menu->text, menu->x, menu->y, 15, BLACK);
   
   for(int i = 0; i < menu->num_buttons; ++i){
      draw_button(&menu->buttons[i]);
   }
   
}

int update_option_panel(OPTION_PANEL *menu){
   if(!is_mouse_collision(menu->x, menu->y ,menu->width ,menu->height)) return 0;

   for(int i = 0; i < menu->num_buttons; ++i){
      if(button_activate(&menu->buttons[i], MOUSE_BUTTON_LEFT)){
         menu->selected = i;
         return 1;
      }
   }
   
   return 0;
}

//
//    Text Box Functions
//

TEXT_BOX create_text_box(int x, int y, int width, int height, int capacity, int text_size){
   TEXT_BOX result = {
      .rect = (Rectangle){x, y, width, height},
      .capacity = capacity,
      .length = 0,
      .selected = 0,
      .text_size = text_size
   };

   result.text = malloc(sizeof(char) * capacity + 1);
   assert(result.text);
   result.text[0] = '\0';

   return result;
}

void delete_text_box(TEXT_BOX *textbox){
   free(textbox->text);
}

void draw_text_box(TEXT_BOX *textbox){
   DrawRectangleRec(textbox->rect, GRAY);
   DrawRectangleLinesEx(textbox->rect, 1.0, BLACK);
   DrawText(textbox->text, textbox->rect.x + 2, textbox->rect.y + 2, textbox->text_size, WHITE);
}
int update_text_box(TEXT_BOX *textbox){
   int on_textbox = is_mouse_collision(textbox->rect.x, textbox->rect.y, textbox->rect.width, textbox->rect.height);
   if(!on_textbox){
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
         textbox->selected = 0;
         return -1;
      }
      return 0;
   }

   if(on_textbox && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
         textbox->selected = 1;
   }

   if(textbox->selected != 0){
      textbox->length = update_text_field(textbox->text, textbox->capacity);
      if(IsKeyPressed(KEY_ENTER)){
         return 1;
      }
   }

   return 0;
}
