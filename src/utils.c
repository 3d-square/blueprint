#include "utils.h"

int point_in_rect(int x, int y, int rx, int ry, int rw, int rh){
   return (x >= rx) && (y >= ry) && (x <= rx + rw) && (y <= ry + rh);
}
