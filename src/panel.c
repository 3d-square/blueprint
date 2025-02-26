#include "raylib.h"
#include "flow.h"
#include "panel.h"
#include "button.h"
#include "cutils.h"
#include <string.h>
#include <stdio.h>
#include "globals.h"

#define MAXCOORDLEN 9

struct branch_panel{
   int width, height;
   char link_midpoint_1[10];
   char link_midpoint_2[10];
   int mpl1;
   int mpl2;
   int focus;
   BRANCH_FLOW *branch;
   BUTTON btn1;
   BUTTON btn2;
   BUTTON delete;
} branch_panel;

struct node_panel{
   int width, height;
   char link_midpoint[10];
   int mpl;
   int focus;
   NODE_FLOW *node;
   BUTTON btn;
   BUTTON delete;
} node_panel;

void init_panels(){
   branch_panel = (struct branch_panel){
      .width = 200,
      .height = 120,
      .mpl1 = 0,
      .mpl2 = 0,
      .focus = 0,
      .link_midpoint_1 = {0},
      .link_midpoint_2 = {0},
      .branch = NULL,
      .btn1 = create_button(0, 0, 15, 15, "X", 15),
      .btn2 = create_button(0, 0, 15, 15, "X", 15),
      .delete = create_button(0, 0, 200, 25, "Delete Branch", 15)
   };
   node_panel = (struct node_panel){
      .width = 125,
      .height = 120,
      .mpl = 0,
      .focus = 0,
      .link_midpoint = {0},
      .node = NULL,
      .btn = create_button(0, 0, 15, 15, "X", 15),
      .delete = create_button(0, 0, 125, 25, "Delete Node", 15)
   };
}

static char title_str[32];
static char buffer[128];
static int title_width = 0;

void show_branch_flow(){
   if(branch_panel.branch == NULL) return;
   // init all the local variables
   int x = branch_panel.branch->x;
   int y = branch_panel.branch->y;
   int text_height = GetFontDefault().baseSize;

   DrawRectangle(x, y, branch_panel.width, branch_panel.height, DARKGRAY);
   DrawRectangleLines(x, y, branch_panel.width, branch_panel.height, BLACK);
   DrawRectangleLines(x + 112, y + 50, 75, text_height + 8, BLACK);
   DrawRectangleLines(x + 13, y + 50, 75, text_height + 8, BLACK);

   DrawText(title_str, x + (branch_panel.width / 2) - (title_width / 2), y + 5, 20, WHITE);
   DrawText(branch_panel.link_midpoint_1, x + 15, y + 55, 10, WHITE);
   DrawText(branch_panel.link_midpoint_2, x + 114, y + 55, 10, WHITE);

   if(branch_panel.branch->yes.to == NULL){
      DrawText("Not Set", x + 25, y + 25, 15, WHITE);
   }else{
      sprintf(buffer, "Set: %s", branch_panel.branch->yes.to->uuid);
      DrawText(buffer, x + 25, y + 25, 15, WHITE);
   }

   if(branch_panel.branch->no.to == NULL){
      DrawText("Not Set", x + 125, y + 25, 15, WHITE);
   }else{
      sprintf(buffer, "Set: %s", branch_panel.branch->no.to->uuid);
      DrawText(buffer, x + 125, y + 25, 15, WHITE);
   }

   draw_button(&branch_panel.btn1);
   draw_button(&branch_panel.btn2);
   draw_button(&branch_panel.delete);
}

void show_node_flow(){
   if(node_panel.node == NULL) return;
   // init all the local variables
   int x = node_panel.node->x;
   int y = node_panel.node->y;
   int text_height = GetFontDefault().baseSize;

   DrawRectangle(x, y, node_panel.width, node_panel.height, DARKGRAY);
   DrawRectangleLines(x, y, node_panel.width, node_panel.height, BLACK);
   DrawRectangleLines(x + 13, y + 50, 100, text_height + 8, BLACK);

   DrawText(title_str, x + (node_panel.width / 2) - (title_width / 2), y + 5, 20, WHITE);
   DrawText(node_panel.link_midpoint, x + 15, y + 55, 10, WHITE);

   if(node_panel.node->next.to == NULL){
      DrawText("Not Set", x + 25, y + 25, 15, WHITE);
   }else{
      sprintf(buffer, "Set: %s", node_panel.node->next.to->uuid);
      DrawText(buffer, x + 25, y + 25, 15, WHITE);
   }

   draw_button(&node_panel.btn);
   draw_button(&node_panel.delete);
}

int update_branch_panel(GEN_FLOW *nodes[], int *length){
   if(branch_panel.branch != NULL){
      if(is_mouse_collision(branch_panel.branch->x, branch_panel.branch->y, branch_panel.width, branch_panel.height)){
         if(button_activate(&branch_panel.btn1, MOUSE_BUTTON_LEFT)){
            branch_panel.branch->yes.to = NULL;
         }else if(button_activate(&branch_panel.btn2, MOUSE_BUTTON_LEFT)){
            branch_panel.branch->no.to = NULL;
         }else if(button_activate(&branch_panel.delete, MOUSE_BUTTON_LEFT)){
            printf("Delete node TODO\n");
         }
      }else{
         branch_panel.branch = NULL;
         return RESET_FOCUS;
      }
   }else{
      GEN_FLOW *node = get_node_at(nodes, *length, mouse_position.x, mouse_position.y);
      if(node != NULL && node->type == BRANCH){
         branch_panel.branch = (BRANCH_FLOW *)node;

         sprintf(branch_panel.link_midpoint_1, "%d,%d", branch_panel.branch->yes.midx, branch_panel.branch->yes.midy);
         sprintf(branch_panel.link_midpoint_2, "%d,%d", branch_panel.branch->no.midx, branch_panel.branch->no.midy);
         sprintf(title_str, "Branch Node: %s", node->uuid);
         title_width = MeasureText(title_str, 20);
         branch_panel.mpl1 = strlen(branch_panel.link_midpoint_1);
         branch_panel.mpl2 = strlen(branch_panel.link_midpoint_2);
         branch_panel.btn1.x = node->x + 7;
         branch_panel.btn1.y = node->y + 25;
         branch_panel.btn2.x = node->x + 107;
         branch_panel.btn2.y = node->y + 25;
         branch_panel.delete.x = node->x;
         branch_panel.delete.y = node->y + branch_panel.height - 25;
      }else{
         return MAIN_FOCUS;
      }
   }
   node_panel.node = NULL;
   return BRANCH_PANEL_FOCUS;
}

int update_node_panel(GEN_FLOW *nodes[], int *length){
   if(node_panel.node!= NULL){
      if(is_mouse_collision(node_panel.node->x, node_panel.node->y, node_panel.width, node_panel.height)){
         if(button_activate(&node_panel.btn, MOUSE_BUTTON_LEFT)){
            node_panel.node->next.to = NULL;
         }else if(button_activate(&node_panel.delete, MOUSE_BUTTON_LEFT)){
            printf("Delete node TODO\n");
         }
      }else{
         node_panel.node = NULL;
         return RESET_FOCUS;
      }
   }else{
      GEN_FLOW *node = get_node_at(nodes, *length, mouse_position.x, mouse_position.y);
      if(node != NULL && node->type == NODE){
         node_panel.node = (NODE_FLOW *)node;

         sprintf(node_panel.link_midpoint, "%d,%d", node_panel.node->next.midx, node_panel.node->next.midy);
         sprintf(title_str, "Node: %s", node->uuid);
         title_width = MeasureText(title_str, 20);
         node_panel.mpl = strlen(node_panel.link_midpoint);
         node_panel.btn.x = node->x + 7;
         node_panel.btn.y = node->y + 25;
         node_panel.delete.x = node->x;
         node_panel.delete.y = node->y + node_panel.height - 25;
      }else{
         return MAIN_FOCUS;
      }
   }

   branch_panel.branch = NULL;

   return NODE_PANEL_FOCUS;
}
