#pragma once

#include "button.h"

typedef struct _options{
   int x, y;
   int width, height;
   char *text;
   BUTTON *buttons;
   BUTTON close;
   int num_buttons;
   int selected;
   int visible;
} OPTION_MENU;

OPTION_MENU create_menu(int x, int y, int w, int h, char *text, char **buttons, int num_buttons); 
void delete_menu(OPTION_MENU *menu);

void set_visible(OPTION_MENU *menu);
void set_invisible(OPTION_MENU *menu);

void draw_menu(OPTION_MENU *menu);

int update_menu(OPTION_MENU *menu);

int get_selected(OPTION_MENU *menu);
