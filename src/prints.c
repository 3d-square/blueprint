#include <stdio.h>
#include <stdlib.h>
#include "flow.h"
#include "prints.h"
#include "globals.h"
#include "cutils.h"
#include <string.h>
#include <assert.h>

extern char file_name[64];

void branch_flow_info(BRANCH_FLOW *flow, BRANCH_INFO *info){
   info->x = flow->x;
   info->y =  flow->y;
   strcpy(info->uuid, flow->uuid);
   info->btype = flow->btype;
   info->yes_midy = flow->yes.midy;
   info->yes_midx = flow->yes.midx;
   if(flow->yes.to){
      strcpy(info->yes_to_uuid, flow->yes.to->uuid);
   }else{
      info->yes_to_uuid[0] = '\0';
   }
   info->no_midy = flow->no.midy;
   info->no_midx = flow->no.midx;
   if(flow->no.to){
      strcpy(info->no_to_uuid, flow->no.to->uuid);
   }else{
      info->no_to_uuid[0] = '\0';
   }
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
   if(flow->next.to){
      strcpy(info->to_uuid, flow->next.to->uuid);
   }else{
      info->to_uuid[0] = '\0';
   }

}

void branch_from_info(BRANCH_INFO *info, BRANCH_FLOW *flow){
   flow->x = info->x;
   flow->y = info->y;
   strcpy(flow->uuid, info->uuid);
   flow->btype = info->btype;
}

void node_from_info(NODE_INFO *info, NODE_FLOW *flow){
   flow->x = info->x;
   flow->y = info->y;
   flow->width = info->width;
   flow->height = info->height;
   strcpy(flow->uuid, info->uuid);
   flow->text_width = info->text_width;
   strcpy(flow->value, info->value);
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
   printf("%s[%p] : %s[%p]\n", from->uuid, from, nodes[index]->uuid, nodes[index]);
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

      if(flow[i]->type == BRANCH){
         BRANCH_FLOW *branch = (BRANCH_FLOW *)flow[i];
         branch_flow_info(branch, &branch_info);
         fwrite(&branch_info, sizeof(BRANCH_INFO), 1, fp);
      }else if(flow[i]->type == NODE){
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
   FLOW_INFO infos[MAXGRAPHNODES];
   FILE *fp = fopen(file_name, "rb");
   if(fp == NULL){
      set_global_message("Unable to open file");
      *length = -1;

      return;
   }
   fread(length, sizeof(int), 1, fp);

   int type;
   for(int i = 0; i < *length; ++i){
      fread(&type, sizeof(FLOW), 1, fp);
      if(type == NODE){
         fread(&infos[i].node_info, sizeof(NODE_INFO), 1, fp);
         nodes[i] = calloc(sizeof(NODE_FLOW), 1);
         assert(nodes[i]);

         node_from_info(&infos[i].node_info, (NODE_FLOW *)nodes[i]);
      }else if(type == BRANCH){
         fread(&infos[i].branch_info, sizeof(BRANCH_INFO), 1, fp);
         nodes[i] = calloc(sizeof(BRANCH_FLOW), 1);
         assert(nodes[i]);

         branch_from_info(&infos[i].branch_info, (BRANCH_FLOW *)nodes[i]);
      }else{
         fprintf(stderr, "Unable to read the next flow node\n");
         free_graph(nodes, i);
      }
      nodes[i]->type = type;
   }

   for(int i = 0; i < *length; ++i){
      if(nodes[i]->type == NODE){
         NODE_FLOW *node = (NODE_FLOW *)nodes[i];
         if(infos[i].node_info.to_uuid[0]){
            link_from_uuid(nodes[i], &node->next, infos[i].node_info.to_uuid, infos[i].node_info.midx, infos[i].node_info.midy, nodes, *length);
         }else{
            node->next = (FLOW_LINK){0};
         }
      }else if(nodes[i]->type == BRANCH){
         BRANCH_FLOW *branch = (BRANCH_FLOW *)nodes[i];
         if(infos[i].branch_info.yes_to_uuid[0]){
            link_from_uuid(nodes[i], &branch->yes, infos[i].branch_info.yes_to_uuid, infos[i].branch_info.yes_midx, infos[i].branch_info.yes_midy, nodes, *length);
         }else{
            branch->yes = (FLOW_LINK){0};
         }
         if(infos[i].branch_info.no_to_uuid[0]){
            link_from_uuid(nodes[i], &branch->no, infos[i].branch_info.no_to_uuid, infos[i].branch_info.no_midx, infos[i].branch_info.no_midy, nodes, *length);
         }else{
            branch->no = (FLOW_LINK){0};
         }
      }else{
         fprintf(stderr, "Unimplemented node\n");
         exit(1);
      }

      node_id = max(node_id, atoi(nodes[i]->uuid + 1));
   }

   node_id = node_id + 1;
   fclose(fp);
}
