#pragma once

#include "raylib.h"

typedef enum _justify{
   RIGHT_JUSTIFY,
   LEFT_JUSTIFY,
   CENTER_JUSTIFY,
} JUSTIFY;

typedef struct _button{
   int x, y;
   int text_x, text_y;
   int width, height;
   char *text;
   int text_size;
   Color textcolor;
} BUTTON;

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

BUTTON create_button(int x, int y, int w, int h, char *text, int text_size, Color textcolor, JUSTIFY position);
void draw_button(BUTTON *button);
int button_collision(BUTTON *button);
int button_activate(BUTTON *button, MouseButton btn);
