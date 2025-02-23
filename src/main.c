#include <stdio.h>
#include "raylib.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "flow.h"
#include "panel.h"

#define DEBUGF(fmt, ...) printf("[DEBUG]: " fmt, __VA_ARGS__)
#define DEBUG(f) printf("[DEBUG]: " f "\n")

int main(void)
{
   // Initialization
   //--------------------------------------------------------------------------------------
   const int screenWidth = 1200;
   const int screenHeight = 800;

   InitWindow(screenWidth, screenHeight, "Rubik's Cube Timer Application");
   SetTargetFPS(20);

   GEN_FLOW *nodes[MAXGRAPHNODES];
   int num_nodes = 0;
   int mod_index = 0;
   int focus = 0;
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

   Vector2 mouse_pos;
   GEN_FLOW *link_node;

   // Main game loop
   while (!WindowShouldClose())    // Detect window close button or ESC key
   {

      if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
         if(link == 0 && focus == 0){
            assert(num_nodes <= MAXGRAPHNODES);

            mouse_pos = GetMousePosition();

            GEN_FLOW *here = get_node_at(nodes, num_nodes, mouse_pos.x, mouse_pos.y);
            if(here == NULL){ // there is no node at this location
               nodes[num_nodes++] = create_branch(EQ, mouse_pos.x, mouse_pos.y, NULL);
               link = -1;
               mod_index = num_nodes - 1;
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
            }
         }
      }else if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
         if(link == 0){
            focus = update_branch_panel(nodes, num_nodes);
            if(focus == 0){
               focus = update_node_panel(nodes, num_nodes);
            }
         }

         if(focus == 0){
            if(link == 0){
               assert(num_nodes <= MAXGRAPHNODES);

               mouse_pos = GetMousePosition();
               nodes[num_nodes++] = create_node("Mouse", mouse_pos.x, mouse_pos.y);
               mod_index = num_nodes - 1;
               link = 1;
            }else if(link > 0){
               mouse_pos = GetMousePosition();
               link_node = get_node_at(nodes, num_nodes, mouse_pos.x, mouse_pos.y);

               if(link_node != NULL){
                  set_node_link((NODE_FLOW *)nodes[mod_index], link_node);
                  link = 0;
               }
            }else{
               mouse_pos = GetMousePosition();

               if(link == -1){
                  link_node = get_node_at(nodes, num_nodes, mouse_pos.x, mouse_pos.y);
                  if(link_node != NULL){
                     link = -2;
                     ((BRANCH_FLOW *)nodes[mod_index])->yes = create_link_from(nodes[mod_index], link_node);
                  }
               }else if(link == -2){
                  link_node = get_node_at(nodes, num_nodes, mouse_pos.x, mouse_pos.y);
                  if(link_node != NULL && link_node != ((BRANCH_FLOW *)nodes[mod_index])->yes.to){
                     ((BRANCH_FLOW *)nodes[mod_index])->no = create_link_from(nodes[mod_index], link_node);
                     link = 0;
                  }
               }
            }
         }
      }

      if(focus == -1) focus = 0;

      BeginDrawing();
         ClearBackground(RAYWHITE);

         draw_nodes(nodes, num_nodes);

         show_branch_flow();
         show_node_flow();
      EndDrawing();

   }

   free_graph(nodes, num_nodes);
   CloseWindow();

   return 0;
}
