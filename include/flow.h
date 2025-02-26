#pragma once

#define FLOW_VARS()\
FLOW type;\
int x, y;\
char uuid[16]

#define GEN_FLOW_PTR(v) ((GEN_FLOW *)&(v))
#define DOUBLE_TO_BYTES(v) (BYTES){.flt = v}
#define INT_TO_BYTES(v) (BYTES){.num = v}
#define PTR_TO_BYTES(v) (BYTES){.ptr = v}

// does not use do while because that would be inconvienent
#define ASSERT_BRANCH_CAST(ptr, varname)\
if(ptr->type != BRANCH){\
   fprintf(stderr, "Invalid type for " #ptr " in %s: Expected BRANCH_FLOW but got %s\n", __func__, gen_type_str(ptr->type));\
   exit(1);\
}\
BRANCH_FLOW *varname = (BRANCH_FLOW *)ptr;

#define ASSERT_NODE_CAST(ptr, varname)\
if(ptr->type != NODE){\
   fprintf(stderr, "Invalid type for " #ptr " in %s: Expected NODE_FLOW but got %s\n", __func__, gen_type_str(ptr->type));\
   exit(1);\
}\
NODE_FLOW *varname = (NODE_FLOW *)ptr;

#define MAXGRAPHNODES 1000

typedef union _bytes{
   char bytes[sizeof(void *)];
   void *ptr;
   long num;
   double flt;
} BYTES;

typedef enum _flow{
   BRANCH,
   NODE,
} FLOW;

typedef struct _gen_flow{
   FLOW type;
   int x, y;
   char uuid[16];
} GEN_FLOW;

typedef struct _link{
   GEN_FLOW *from;
   GEN_FLOW *to;
   int midx, midy;
} FLOW_LINK;

typedef enum _branch_type{
   GT,
   LT,
   EQ,
   GE,
   LE
} BRANCH_TYPE;

typedef struct _branch_flow{
   FLOW_VARS();
   int (*cmp)(BYTES lhs, BYTES rhs);
   BRANCH_TYPE btype;
   BYTES lhs; // Usually changeable value
   BYTES rhs; // Usually constant value
   FLOW_LINK yes;
   FLOW_LINK no;
} BRANCH_FLOW;

typedef struct _node_flow{
   FLOW_VARS();
   int width, height;
   int text_width;
   char *value;
   FLOW_LINK next;
} NODE_FLOW;

char *gen_type_str(FLOW type);

void draw_link(FLOW_LINK *link, Color color);
void draw_branch(GEN_FLOW *branch);
void draw_node(GEN_FLOW *flow);
void draw_graph(GEN_FLOW *graph);
void draw_nodes(GEN_FLOW *nodes[], int num_nodes);
void draw_directioned_arrow(int x, int y, int startx, int starty, int endx, int endy);
int uuid_to_index(GEN_FLOW *node, GEN_FLOW *nodes[], int length);

#define create_link_from(from, to) _create_link_from((GEN_FLOW *)(from), (GEN_FLOW *)(to))
#define auto_link_from(from, to) _create_link_from((GEN_FLOW *)(from), (GEN_FLOW *)(to))
#define empty_link() _create_link_from(NULL, NULL)
FLOW_LINK _create_link_from(GEN_FLOW *from, GEN_FLOW *to);

void free_graph(GEN_FLOW *nodes[], int num_nodes);
void delete_node(NODE_FLOW *node);
void delete_branch(BRANCH_FLOW *branch);

GEN_FLOW *create_branch(BRANCH_TYPE btype, int x, int y, int (*cmp)(BYTES, BYTES));
void set_branch_links(BRANCH_FLOW *branch, GEN_FLOW *yes, GEN_FLOW *no);

GEN_FLOW *create_node(char *value, int x, int y);
void set_node_link(NODE_FLOW *node, GEN_FLOW *next);

void remove_node(GEN_FLOW *removed, GEN_FLOW *nodes[], int lenght);

GEN_FLOW *get_node_at(GEN_FLOW *nodes[], int num_nodes, int x, int y);

FLOW_LINK *execute_branch(BRANCH_FLOW *branch);
int int_cmp  (BYTES lhs, BYTES rhs);
int float_cmp(BYTES lhs, BYTES rhs);
int str_cmp  (BYTES lhs, BYTES rhs);
