#pragma once

// x, y marks the top left corner of the rectangle
int point_in_rect(int x, int y, int rx, int ry, int rw, int rh);

int is_mouse_collision(int rx, int ry, int rw, int rh);
