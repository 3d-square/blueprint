#include "globals.h"
#include "raylib.h"
#include <string.h>

Vector2 mouse_position;
double timer = 0.0;

char screen_message[64] = "";
int message_width = 200;

const int screenWidth = 1200;
const int screenHeight = 800;

void update_globals(void){
   mouse_position = GetMousePosition();

   if(timer < 0.0){
      timer = 0.0;
      strcpy(screen_message, "");
   }else if(timer > 0.0){
      timer -= GetFrameTime(); 
   }
}

void set_global_message(char *msg){
   strcpy(screen_message, msg);
   timer = 5.0;
   // message_width = MeasureText(screen_message, 10);
}
