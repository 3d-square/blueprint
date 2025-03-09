#include "globals.h"
#include "raylib.h"
#include <string.h>

Vector2 mouse_position;

struct _message_queue{
   char screen_message[64];
   double timer;
} message_queue[15] = {0};
int queue_tail = -1;
int queue_index = 0;

char screen_message[64] = "";
int message_width = 200;

const int screenWidth = 1200;
const int screenHeight = 800;

int global_status = 0;

void update_globals(void){
   mouse_position = GetMousePosition();

   if(message_queue[queue_index].timer < 0.0){
      message_queue[queue_index].timer = 0.0;
      queue_index++;
      if(queue_index == 15){
         queue_index = 0;
      }

      if(message_queue[queue_index].timer == 0.0) strcpy(screen_message, "");
      else strcpy(screen_message, message_queue[queue_index].screen_message);
   }else if(message_queue[queue_index].timer > 0.0){
      message_queue[queue_index].timer -= GetFrameTime(); 
   }
}

void set_global_message(char *msg){
   queue_tail += 1;
   if(queue_tail == 15){
      queue_tail = 0;
      if(message_queue[0].timer != 0.0){
         return;
      }
   }
   strcpy(message_queue[queue_tail].screen_message, msg);
   message_queue[queue_tail].timer = 2.5;

   
   // message_width = MeasureText(screen_message, 10);
}
