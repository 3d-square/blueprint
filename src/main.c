#include <stdio.h>
#include "raylib.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "flow.h"
#include "cutils.h"
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
char file_name[64] = "model.dat";

void rmb_menu_option_selection(OPTION_MENU *menu){
   if(update_menu(menu)){
      switch(get_selected(menu)){
         case 0: // create node_flow
            mod_index = num_nodes;
            nodes[num_nodes++] = create_node("Mouse", mouse_position.x, max(menu->y - 15, 0));
            break;
         case 1: // create branch_flow
            mod_index = num_nodes;
            nodes[num_nodes++] = create_branch(EQ, mouse_position.x,  max(menu->y - 15, 0));
            break;
         case -1:
            break;
      }
   }
}

int main(void)
{
   // Initialization
   //--------------------------------------------------------------------------------------
   InitWindow(screenWidth, screenHeight, "Rubik's Cube Timer Application");
   SetTargetFPS(20);

   char *rmb_buttons[] = {"Create Node", "Create Branch"};
   OPTION_MENU rmb_menu = create_menu(0, 0, 75, 150, "RMB", rmb_buttons, 2);

   int link = 0;

   init_panels();

   // BRANCH_FLOW top 0
   nodes[num_nodes++] = create_branch(EQ, 150, 50);

   // BRANCH_FLOW branch 1
   nodes[num_nodes++] = create_branch(EQ, 350, 100);

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

      if(link == 0){
         update_panel(nodes, &num_nodes);
      }

      if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
         assert(num_nodes <= MAXGRAPHNODES);

         GEN_FLOW *here = get_node_at(nodes, num_nodes, mouse_position.x, mouse_position.y);
         if(here == NULL){ // there is no node at this location
            // nodes[num_nodes++] = create_branch(EQ, mouse_position.x, mouse_position.y, NULL);
            // link = -1;
            // mod_index = num_nodes - 1;
            set_visible(&rmb_menu);
            set_global_message("Right Click");
            link = 0;
         
         }else if(here->type == NODE){
            mod_index = uuid_to_index(here->uuid, nodes, num_nodes);

            if(mod_index == -1){
               link = 0;
            }else{
               link = 1;
            }
            set_global_message("Link Node");
            global_status = 1;
         }else{
            mod_index = uuid_to_index(here->uuid, nodes, num_nodes);
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
            global_status = 1;
         }

         if(rmb_menu.visible && !is_mouse_collision(rmb_menu.x, rmb_menu.y, rmb_menu.width, rmb_menu.height)){
            set_invisible(&rmb_menu);
         }
      }else if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
         if(link > 0){
            link_node = get_node_at(nodes, num_nodes, mouse_position.x, mouse_position.y);

            if(link_node != NULL){
               set_node_link((NODE_FLOW *)nodes[mod_index], link_node);
                  reset_node_panel();
            }else{
               set_global_message("No Node Found");
            }
            link = 0;
            global_status = 0;
         }else if(link != 0){
            DISPLAY_MOUSE_POSITION();
            if(link == -1){
               link_node = get_node_at(nodes, num_nodes, mouse_position.x, mouse_position.y);
               if(link_node != NULL){
                  link = ((BRANCH_FLOW *)nodes[mod_index])->no.to == NULL ? -2 : 0; // Handle case where yes is true and no is false
                  ((BRANCH_FLOW *)nodes[mod_index])->yes = create_link_from(nodes[mod_index], link_node);

                  reset_branch_panel();
               }else{
                  set_global_message("No Node Found");
                  link = 0;
               }
            }else if(link == -2){
               link_node = get_node_at(nodes, num_nodes, mouse_position.x, mouse_position.y);
               if(link_node != NULL && link_node != ((BRANCH_FLOW *)nodes[mod_index])->yes.to){
                  ((BRANCH_FLOW *)nodes[mod_index])->no = create_link_from(nodes[mod_index], link_node);
                  reset_branch_panel();

               }else{
                  set_global_message("No Node Found");
               }
               link = 0;
            }
            global_status = 0;
         }else if(rmb_menu.visible){
            if(is_mouse_collision(rmb_menu.x, rmb_menu.y, rmb_menu.width, rmb_menu.height)){
               if(button_collision(&rmb_menu.close)){
                  set_invisible(&rmb_menu);
               }else{
                  rmb_menu_option_selection(&rmb_menu);
               }
            }else{
               set_invisible(&rmb_menu);
            }
         }
      }

      // printf("focus:%d\n", focus);
      BeginDrawing();
         ClearBackground(RAYWHITE);

         draw_nodes(nodes, num_nodes);

         DrawRectangle(screenWidth - message_width, 0, message_width, 50, DARKGRAY);
         DrawRectangleLines(screenWidth - (message_width), 0, message_width, 50, BLACK);
         DrawText(screen_message, screenWidth - message_width + 10, 17, 15, RED);

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

