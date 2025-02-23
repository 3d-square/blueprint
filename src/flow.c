#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "flow.h"
#include "cutils.h"

int font_size = 10;

int node_id = 0;

char *gen_type_str(FLOW type){
   switch(type){
      case BRANCH: return "BRANCH_FLOW";
      case NODE: return "NODE_FLOW";
   }

   return NULL;
}

void print_link(FLOW_LINK *link){
   printf("FLOW_LINK{\n"
          "  from: %p(%d,%d)\n"
          "  to  : %p(%d,%d)\n"
          "}\n",
          link->from, link->from->x, link->from->y, link->to, link->to->x, link->to->y
   );
}

void set_branch_links(BRANCH_FLOW *branch, GEN_FLOW *yes, GEN_FLOW *no){
   if(branch == NULL) return;

   if(yes) branch->yes = create_link_from(branch, yes);
   else branch->yes = empty_link();

   if(no) branch->no = create_link_from(branch, no);
   else branch->no = empty_link();
}

GEN_FLOW *create_branch(BRANCH_TYPE btype, int x, int y, int (*cmp)(BYTES, BYTES)){
   BRANCH_FLOW branch = {
      .type = BRANCH,
	   .btype = btype,
	   .cmp = cmp,
	   .x = x,
	   .y = y,
      .yes = {0},
      .no = {0},
   };
   GEN_FLOW *result = calloc(1, sizeof(BRANCH_FLOW));
   assert(result);
   memcpy(result, &branch, sizeof(BRANCH_FLOW));
   sprintf(result->uuid, "B%d", node_id++);
   

   return result;
}

GEN_FLOW *create_node(char *value, int x, int y){
   int text_height = GetFontDefault().baseSize;
   int text_width = MeasureText(value, font_size);

   if(text_width < 20){
      text_width = 20;
   }

   NODE_FLOW node = {
      .type = NODE,
	   .x = x,
	   .y = y,
	   .value = value,
      .next = {0},
      .width = text_width + 4,
      .height = text_height + 4,
      .text_width = text_width,
   };
   GEN_FLOW *result = calloc(1, sizeof(NODE_FLOW));
   assert(result);
   memcpy(result, &node, sizeof(NODE_FLOW));
   sprintf(result->uuid, "N%d", node_id++);

   return result;
}

void free_graph(GEN_FLOW *nodes[], int num_nodes){
   for(int i = 0; i < num_nodes; ++i){
      if(nodes[i]->type == BRANCH){
         delete_branch((BRANCH_FLOW *)nodes[i]);
      }else{
         delete_node((NODE_FLOW *)nodes[i]);
      }
   }
}
void delete_node(NODE_FLOW *node){
   free(node);
}
void delete_branch(BRANCH_FLOW *branch){
   free(branch);
}

void set_node_link(NODE_FLOW *node, GEN_FLOW *next){
   if(node == NULL) return;

   if(next) node->next = create_link_from(node, next);
   else node->next = empty_link();
}

FLOW_LINK _create_link_from(GEN_FLOW *from, GEN_FLOW *to){
   int text_height = GetFontDefault().baseSize;
   int midy = from->y;

   if(to->type == NODE) midy += text_height + 4;

   
   return (FLOW_LINK){from, to, to->x, midy};
}

void draw_link(FLOW_LINK *link, Color color){
   if(link->from == NULL || link->to == NULL) return;
   
   int text_height = GetFontDefault().baseSize;
   int startx = link->from->x, 
       starty = link->from->y,
       endx   = link->to->x, 
       endy   = link->to->y;
   
   if(link->from->type == NODE){
      starty += text_height + 4;
   }

   if(starty < endy){
      if(link->to->type == BRANCH){
         endy -= 10;
      }
   }else{
      if(link->to->type == NODE){
         endy += text_height + 4;
      }
   }

   DrawLine(startx, starty, link->midx, link->midy, color);
   DrawLine(link->midx, link->midy, endx, endy, color);

   draw_directioned_arrow(endx, endy, startx, starty, endx, endy);
   // draw_directioned_arrow(link->midx, link->midy, midx, starty, link->midx, link->midy);
}

void draw_directioned_arrow(int x, int y, int startx, int starty, int endx, int endy){
   if(starty > endy){
      /* Arrow Points Up */
      DrawTriangle(
         (Vector2){x - 4, y + 4},
         (Vector2){x + 4, y + 4},
         (Vector2){x, y},
         BLACK
      );
   }else if(starty < endy){
      /* Arrow Points Down */
      DrawTriangle(
         (Vector2){x, y},
         (Vector2){x + 4, y - 4},
         (Vector2){x - 4, y - 4},
         BLACK
      );
   }else{
      if(startx < endx){
         /* Arrow Points Right */
         DrawTriangle(
            (Vector2){x, y},
            (Vector2){x - 4, y + 4},
            (Vector2){x - 4, y - 4},
            BLACK
         );
      }else{
         /* Arrow Points left*/
         DrawTriangle(
            (Vector2){x + 4, y - 4},
            (Vector2){x + 4, y + 4},
            (Vector2){x, y},
            BLACK
         );
      }
   }
}

void print_vec2(Vector2 v){
   printf("Vec2(%f, %f)\n", v.x, v.y);
}

void draw_branch(GEN_FLOW *flow){
   ASSERT_BRANCH_CAST(flow, branch);

   draw_link(&branch->yes, BLUE);
   draw_link(&branch->no, BLUE);

   DrawTriangle(
      (Vector2){branch->x, branch->y}, 
      (Vector2){branch->x + 10.0, branch->y - 10.0}, 
      (Vector2){branch->x - 10.0, branch->y - 10.0}, 
      RED
   );
}

void draw_node(GEN_FLOW *flow){
   ASSERT_NODE_CAST(flow, node);

   draw_link(&node->next, BLACK);

   DrawText(node->value, node->x - node->text_width/2 + 2, node->y + 2, font_size, BLACK);

   DrawRectangleLines(node->x - (node->text_width)/2, node->y, node->width, node->height, BLACK);
}

void draw_nodes(GEN_FLOW *nodes[], int num_nodes){
   for(int i = 0; i < num_nodes; ++i){
      if(nodes[i]->type == NODE){
         draw_node(nodes[i]);
      }else{
         draw_branch(nodes[i]);
      }
   }
}

void draw_graph_helper(GEN_FLOW *graph, GEN_FLOW *seen[], int *nodes_seen){
   if(graph == NULL || *nodes_seen >= MAXGRAPHNODES) return;

   /* Look to see if node was drawn before, prevents against infinite loops */
   for(int i = 0; i < *nodes_seen; ++i){
      if(graph == seen[i]){
         return;
      }
   }

   int text_height = GetFontDefault().baseSize;

   seen[*nodes_seen] = graph;
   *nodes_seen = *nodes_seen + 1;

   if(graph->type == NODE){
      NODE_FLOW *node = (NODE_FLOW *)graph;
      draw_link(&node->next, BLUE);

      draw_graph_helper(node->next.to, seen, nodes_seen);

      int text_width = MeasureText(node->value, font_size);

      if(text_width < 20){
         text_width = 20;
      }

      DrawRectangle(node->x - text_width/2, node->y, text_width + 4, text_height + 4, RAYWHITE);
      DrawText(node->value, node->x - node->text_width/2 + 2, node->y + 2, font_size, BLACK);

      DrawRectangleLines(node->x - text_width/2, node->y, text_width + 4, text_height + 4, BLACK);
   }else{
      BRANCH_FLOW *branch = (BRANCH_FLOW *)graph;

      draw_link(&branch->yes, BLUE);
      draw_link(&branch->no, BLUE);
  
      draw_graph_helper(branch->yes.to, seen, nodes_seen);
      draw_graph_helper(branch->no.to, seen, nodes_seen);

      DrawTriangle(
         (Vector2){branch->x, branch->y}, 
         (Vector2){branch->x + 10.0, branch->y - 10.0}, 
         (Vector2){branch->x - 10.0, branch->y - 10.0}, 
         RED
      );
   }
}

void draw_graph(GEN_FLOW *graph){
   GEN_FLOW *seen[MAXGRAPHNODES];
   int nodes_seen = 0;

   draw_graph_helper(graph, seen, &nodes_seen);

   if(nodes_seen >= MAXGRAPHNODES){
      printf("Too many nodes on the screen to draw them all\n");
   }
}

FLOW_LINK *execute_branch(BRANCH_FLOW *branch){
    int cmp_val = branch->cmp(branch->lhs, branch->lhs);
    FLOW_LINK *result = &branch->yes;
    switch(branch->btype){
        case GT:{
            if(cmp_val <= 0) result = &branch->no;
        }break;
        case LT:{
            if(cmp_val >= 0) result = &branch->no;
        }break;
        case EQ:{
            if(cmp_val != 0) result = &branch->no;
        }break;
        case GE:{
            if(cmp_val < 0) result = &branch->no;
        }break;
        case LE:{
            if(cmp_val > 0) result = &branch->no;
        }break;
    }

    return result;
}

GEN_FLOW *get_node_at(GEN_FLOW *nodes[], int num_nodes, int x, int y){
   for(int i = 0; i < num_nodes; ++i){
      if(nodes[i]->type == NODE){
         NODE_FLOW *node = (NODE_FLOW *)nodes[i];
         if(point_in_rect(x, y, node->x - (node->text_width)/2, node->y, node->width, node->height)){
            return nodes[i];
         }
      }else{
         BRANCH_FLOW *branch = (BRANCH_FLOW *)nodes[i];
         // width 20 height 10
         
         if(point_in_rect(x, y, branch->x - 10, branch->y - 10, 20, 10)){
            return nodes[i];
         }
      }
   }

   return NULL;
}

int int_cmp(BYTES lhs, BYTES rhs){
    return lhs.num - rhs.num;
}

int float_cmp(BYTES lhs, BYTES rhs){
   float diff = lhs.flt - rhs.flt;
   int result = 0;

   if(diff < 0.0) result = -1;
   else if(diff > 0.0) result = 1;

   return result;
}

int str_cmp(BYTES lhs, BYTES rhs){
   return strcmp((char *)lhs.ptr, (char *)rhs.ptr);
}


int uuid_to_index(GEN_FLOW *node, GEN_FLOW *nodes[], int length){
   for(int i = 0; i < length; ++i){
      if(strcmp(node->uuid, nodes[i]->uuid) == 0) return i;
   }

   return -1;
}
