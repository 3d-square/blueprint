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
   JUSTIFY position;
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

typedef struct _options_panel{
   int x, y;
   int width, height;
   char *text;
   BUTTON *buttons;
   int num_buttons;
   int selected;
} OPTION_PANEL;

#define get_selected_ref(opt) (opt)->selected
#define get_selected(opt) (opt).selected

OPTION_MENU create_option_menu(int x, int y, int w, int h, char *text, char **buttons, int num_buttons); 
void recreate_buttons(BUTTON **buttons, int x, int y, int w, int h, char **buttons_text, int num_buttons);
void delete_option_menu(OPTION_MENU *menu);
void draw_option_menu(OPTION_MENU *menu);
void set_visible(OPTION_MENU *menu);
void set_invisible(OPTION_MENU *menu);
int update_option_menu(OPTION_MENU *menu);

OPTION_PANEL create_option_panel(int x, int y, int w, int h, char *text, char **buttons, int num_buttons); 
void delete_option_panel(OPTION_PANEL *menu);
void update_option_panel_position(OPTION_PANEL *menu);
void draw_option_panel(OPTION_PANEL *menu);


BUTTON create_button(int x, int y, int w, int h, char *text, int text_size, Color textcolor, JUSTIFY position);
void draw_button(BUTTON *button);
int button_collision(BUTTON *button);
int button_activate(BUTTON *button, MouseButton btn);
