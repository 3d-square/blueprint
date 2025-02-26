#include <stdio.h>
#include "raylib.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "flow.h"
#include "panel.h"
#include "globals.h"
#include "options.h"

#define DEBUGF(fmt, ...) printf("[DEBUG]: " fmt, __VA_ARGS__)
#define DEBUG(f) printf("[DEBUG]: " f "\n")

#undef max
#define max(a, b) (a) > (b) ? (a) : (b) 

GEN_FLOW *nodes[MAXGRAPHNODES];
int num_nodes = 0;
int mod_index = 0;
int focus = MAIN_FOCUS;

void rmb_menu_option_selection(OPTION_MENU *menu){
   focus = update_menu(menu);

   if(focus == MENU_BUTTON_CLICKED){
      switch(get_selected(menu)){
         case 0: // create node_flow
            mod_index = num_nodes;
            nodes[num_nodes++] = create_node("Mouse", mouse_position.x, max(menu->y - 15, 0));
            break;
         case 1: // create branch_flow
            mod_index = num_nodes;
            nodes[num_nodes++] = create_branch(EQ, mouse_position.x,  max(menu->y - 15, 0), NULL);
            break;
         case -1:
            break;
      }
      focus = menu->focus;
   }
}

int main(void)
{
   // Initialization
   //--------------------------------------------------------------------------------------
   InitWindow(screenWidth, screenHeight, "Rubik's Cube Timer Application");
   SetTargetFPS(20);

   char *rmb_buttons[] = {"Create Node", "Create Branch"};
   OPTION_MENU rmb_menu = create_menu(0, 0, 75, 150, "RMB", rmb_buttons, 2, RMB_MENU_FOCUS);

   int link = 0;

   init_panels();

   // BRANCH_FLOW top 0
   nodes[num_nodes++] = create_branch(EQ, 150, 50, NULL);

   // BRANCH_FLOW branch 1
   nodes[num_nodes++] = create_branch(EQ, 350, 100, int_cmp);

   // NODE_FLOW node1 2
   nodes[num_nodes++] = create_node("Yes?", 250, 250);

   // NODE_FLOW node2 3
   nodes[num_nodes++] = create_node("NO?", 450, 250);

   // NODE_FLOW midpoint 4
   nodes[num_nodes++] = create_node("MID", 150, 125);

   set_branch_links((BRANCH_FLOW *)nodes[0], nodes[1], nodes[3]);
   set_branch_links((BRANCH_FLOW *)nodes[1], nodes[2], nodes[3]);

   set_node_link((NODE_FLOW *)nodes[2], nodes[4]);
   set_node_link((NODE_FLOW *)nodes[4], nodes[0]);

   GEN_FLOW *link_node;

   // Main game loop
   while (!WindowShouldClose())    // Detect window close button or ESC key
   {
      update_globals();
      if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
         if(focus == MAIN_FOCUS){
            assert(num_nodes <= MAXGRAPHNODES);

            GEN_FLOW *here = get_node_at(nodes, num_nodes, mouse_position.x, mouse_position.y);
            if(here == NULL){ // there is no node at this location
               // nodes[num_nodes++] = create_branch(EQ, mouse_position.x, mouse_position.y, NULL);
               // link = -1;
               // mod_index = num_nodes - 1;
               set_visible(&rmb_menu);
               focus = RMB_MENU_FOCUS;
               set_global_message("Right Click");
            
            }else if(here->type == NODE){
               mod_index = uuid_to_index(here, nodes, num_nodes);

               if(mod_index == -1){
                  // do some error handling
                  // maybe reset state
                  //
                  link = 0;
               }else{
                  link = 1;
               }
               set_global_message("Link Node");
            }else{
               mod_index = uuid_to_index(here, nodes, num_nodes);
               if(((BRANCH_FLOW *)here)->yes.to == NULL){
                  if(mod_index == -1){
                     // do some error handling
                     // maybe reset state
                     //
                     link = 0;
                  }else{
                     link = -1;
                  }              
               }else if(((BRANCH_FLOW *)here)->no.to == NULL){
                  if(mod_index == -1){
                     // do some error handling
                     // maybe reset state
                     //
                     link = 0;
                  }else{
                     link = -2;
                  }
               }
               set_global_message("Link Branch");
            }
         }else if(focus == RMB_MENU_FOCUS){
            focus = MAIN_FOCUS; // TODO: If a window is open and rmb is pressed close the window 
            set_invisible(&rmb_menu);
         }else{
            focus = MAIN_FOCUS;
         }
      }else if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
         if(link == 0 && focus == MAIN_FOCUS){
            focus = update_branch_panel(nodes, &num_nodes);
            if(focus == MAIN_FOCUS){
               focus = update_node_panel(nodes, &num_nodes);
               if(focus == MAIN_FOCUS){

               }else if(focus != RESET_FOCUS){
                  set_global_message("Node Info");
               }
            }else if(focus != RESET_FOCUS){
               set_global_message("Branch Info");
            }
         }

         if(focus == MAIN_FOCUS){
            if(link > 0){
               link_node = get_node_at(nodes, num_nodes, mouse_position.x, mouse_position.y);

               if(link_node != NULL){
                  set_node_link((NODE_FLOW *)nodes[mod_index], link_node);
                  link = 0;
               }
            }else if(link != 0){
               if(link == -1){
                  link_node = get_node_at(nodes, num_nodes, mouse_position.x, mouse_position.y);
                  if(link_node != NULL){
                     link = ((BRANCH_FLOW *)nodes[mod_index])->no.to == NULL ? -2 : 0; // Handle case where yes is true and no is false
                     ((BRANCH_FLOW *)nodes[mod_index])->yes = create_link_from(nodes[mod_index], link_node);
                  }
               }else if(link == -2){
                  link_node = get_node_at(nodes, num_nodes, mouse_position.x, mouse_position.y);
                  if(link_node != NULL && link_node != ((BRANCH_FLOW *)nodes[mod_index])->yes.to){
                     ((BRANCH_FLOW *)nodes[mod_index])->no = create_link_from(nodes[mod_index], link_node);
                     link = 0;
                  }
               }
            }
         }else if(focus == RMB_MENU_FOCUS){ //rmb menu open 
            rmb_menu_option_selection(&rmb_menu);
         }else{
            focus = MAIN_FOCUS;
         }
      }

      if(focus == RESET_FOCUS){
         focus = 0;
      }

      // printf("focus:%d\n", focus);
      BeginDrawing();
         ClearBackground(RAYWHITE);

         draw_nodes(nodes, num_nodes);

         DrawRectangle(screenWidth - message_width, 0, message_width, 50, DARKGRAY);
         DrawRectangleLines(screenWidth - (message_width), 0, message_width, 50, BLACK);
         DrawText(screen_message, screenWidth - message_width + 50, 17, 20, RED);

         show_branch_flow();
         show_node_flow();
         draw_menu(&rmb_menu);
      EndDrawing();
   }

   delete_menu(&rmb_menu);
   free_graph(nodes, num_nodes);
   CloseWindow();

   return 0;
}
