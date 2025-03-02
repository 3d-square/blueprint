#include <stdio.h>
#include <stdlib.h>
#include "flow.h"
#include "prints.h"
#include "globals.h"
#include <string.h>

extern char file_name[64];

void branch_flow_info(BRANCH_FLOW *flow, BRANCH_INFO *info){
   info->x = flow->x;
   info->y =  flow->y;
   strcpy(info->uuid, flow->uuid);
   info->btype = flow->btype;
   info->yes_midy = flow->yes.midy;
   info->yes_midx = flow->yes.midx;
   strcpy(info->yes_to_uuid, flow->yes.to->uuid);
   info->no_midy = flow->no.midy;
   info->no_midx = flow->no.midx;
   strcpy(info->no_to_uuid, flow->no.to->uuid);
}

void node_flow_info(NODE_FLOW *flow, NODE_INFO *info){
   info->x = flow->x;
   info->y =  flow->y;
   strcpy(info->uuid, flow->uuid);
   strcpy(info->value, flow->value);
   info->width = flow->width;
   info->height = flow->height;
   info->text_width = flow->text_width; 
   info->midx = flow->next.midx;
   info->midy = flow->next.midy;
   strcpy(info->to_uuid, flow->next.to->uuid);

}

void link_from_uuid(GEN_FLOW *from, FLOW_LINK *link, char *uuid, int x, int y, GEN_FLOW *nodes[], int length){
   int index = uuid_to_index(uuid, nodes, length);
   if(index == -1){
      fprintf(stderr, "Unable to link node\n");
      exit(1);
   }

   *link = (FLOW_LINK){
      .to = nodes[index],
      .from = from,
      .midx = x,
      .midy = y
   };

}

void save_model(GEN_FLOW *flow[], int length){
   FILE *fp = fopen(file_name, "wb");
   if(fp == NULL){
      set_global_message("Unable to open file");

      return;
   }
   BRANCH_INFO branch_info;
   NODE_INFO node_info;

   fwrite(&length, sizeof(int), 1, fp);
   for(int i = 0; i < length; ++i){
      fwrite(&flow[i]->type, sizeof(FLOW), 1, fp);

      if(flow[i]->type == NODE){
         BRANCH_FLOW *branch = (BRANCH_FLOW *)flow[i];
         branch_flow_info(branch, &branch_info);
         fwrite(&branch_info, sizeof(BRANCH_INFO), 1, fp);
      }else if(flow[i]->type == BRANCH){
         NODE_FLOW *node = (NODE_FLOW *)flow[i];
         node_flow_info(node, &node_info);
         fwrite(&node_info, sizeof(NODE_INFO), 1, fp);
      }else{
         fprintf(stderr, "Unable to save flow type\n");
         exit(1);
      }
   }

   fclose(fp);
}

void load_model(GEN_FLOW *nodes[], int *length){
   FILE *fp = fopen(file_name, "wb");
   if(fp == NULL){
      set_global_message("Unable to open file");
      *length = -1;

      return;
   }
   fread(length, sizeof(int), 1, fp);
   

   fclose(fp);
}
