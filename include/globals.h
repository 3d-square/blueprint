#pragma once

#include "raylib.h"

#define RESET_FOCUS -1
#define MAIN_FOCUS 0
#define BRANCH_PANEL_FOCUS 1
#define NODE_PANEL_FOCUS 2
#define RMB_MENU_FOCUS 3
#define MENU_BUTTON_CLICKED 1000

#define DISPLAY_MOUSE_POSITION() printf("(%f, %f)\n", mouse_position.x, mouse_position.y)

extern Vector2 mouse_position;
extern char screen_message[64];
extern int message_width;
extern const int screenHeight;
extern const int screenWidth;
extern int global_status;

void set_global_message(char *msg);
void update_globals(void);
