#include "globals.h"
#include "button.h"
#include "raylib.h"
#include "cutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BUTTON create_button(int x, int y, int w, int h, char *text, int text_size){
   BUTTON result = {
      .x = x,
      .y = y,
      .width = w,
      .height = h,
      .text = text,
      .text_size = text_size
   };
   return result;
}

void draw_button(BUTTON *button){
   Color color = button_collision(button) ? LIGHTGRAY : GRAY;

   DrawRectangle(button->x, button->y, button->width, button->height, color);
   DrawRectangleLines(button->x, button->y, button->width, button->height, BLACK);

   DrawText(button->text, button->x + 2, button->y + 1, button->text_size, WHITE);
}

int button_collision(BUTTON *button){
   return is_mouse_collision(button->x, button->y, button->width, button->height);
}

int button_activate(BUTTON *button, MouseButton btn){
   return button_collision(button) && IsMouseButtonPressed(btn);
}
