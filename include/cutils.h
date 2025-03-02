#pragma once


#define remove_index(array, length, index)\
for(int _i = index; _i < length - 1; ++_i){\
   array[_i] = array[_i + 1];\
}\
// x, y marks the top left corner of the rectangle

#undef max
#define max(a, b) (a) > (b) ? (a) : (b) 


int point_in_rect(int x, int y, int rx, int ry, int rw, int rh);

int is_mouse_collision(int rx, int ry, int rw, int rh);

int str_to_coord(char *str, int *x, int *y);

int str2i(char *str, int *);

void update_text_box(char *text, int length);
