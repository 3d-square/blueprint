#pragma once
#include "raylib.h"

typedef struct _button{
   int x, y;
   int width, height;
   char *text;
   int text_size;
} BUTTON;

BUTTON create_button(int x, int y, int w, int h, char *text, int text_size);
void draw_button(BUTTON *button);
int button_collision(BUTTON *button);
int button_activate(BUTTON *button, MouseButton btn);
