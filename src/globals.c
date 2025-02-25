#include "globals.h"
#include "raylib.h"

Vector2 mouse_position;

void update_globals(void){
   mouse_position = GetMousePosition();
}
