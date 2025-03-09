#pragma once

#include "flow.h"

typedef struct _branch_info{
   int x, y;
   char uuid[16];
   BRANCH_TYPE btype;  
   int yes_midx, yes_midy;
   char yes_to_uuid[16];
   int no_midx, no_midy;
   char no_to_uuid[16];
} BRANCH_INFO;

typedef struct _node_info{
   int x, y;
   char uuid[16];
   int width, height;
   int text_width;
   char value[128];
   int midx, midy;
   char to_uuid[16];
} NODE_INFO;

typedef union _flow_info{
   NODE_INFO node_info;
   BRANCH_INFO branch_info;
} FLOW_INFO;

void branch_flow_info(BRANCH_FLOW *flow, BRANCH_INFO *info);
void node_flow_info(NODE_FLOW *flow, NODE_INFO *info);
void branch_from_info(BRANCH_INFO *info, BRANCH_FLOW *flow);
void node_from_info(NODE_INFO *info, NODE_FLOW *flow);
void link_from_uuid(GEN_FLOW *from, FLOW_LINK *link, char *uuid, int x, int y, GEN_FLOW *nodes[], int length);
void save_model(GEN_FLOW *flow[], int length);
int load_model(GEN_FLOW *flow[], int *length);
