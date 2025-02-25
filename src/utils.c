#include "utils.h"
#include "globals.h"

int point_in_rect(int x, int y, int rx, int ry, int rw, int rh){
   return (x >= rx) && (y >= ry) && (x <= rx + rw) && (y <= ry + rh);
}

int is_mouse_collision(int rx, int ry, int rw, int rh){
   return point_in_rect(mouse_position.x, mouse_position.y, rx, ry, rw, rh);
}
