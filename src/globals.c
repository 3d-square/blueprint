#include "globals.h"
#include "raylib.h"
#include <string.h>

Vector2 mouse_position;

char screen_message[64] = "Main Window";
int message_width = 100;

void update_globals(void){
   mouse_position = GetMousePosition();
}

void set_global_message(char *msg){
   strcpy(screen_message, msg);
   // message_width = MeasureText(screen_message, 10);
}
