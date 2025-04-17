#include "raylib.h"
#include <dirent.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <ui.h>

/* Begin Flow Structures and Defines */

#define FLOW_VARS()\
FLOW type;\
int x, y;\
char uuid[16]

#define GEN_FLOW_PTR(v) ((GEN_FLOW *)&(v))
#define DOUBLE_TO_BYTES(v) (BYTES){.flt = v}
#define INT_TO_BYTES(v) (BYTES){.num = v}
#define PTR_TO_BYTES(v) (BYTES){.ptr = v}
#define create_link_from(from, to) _create_link_from((GEN_FLOW *)(from), (GEN_FLOW *)(to))
#define auto_link_from(from, to) _create_link_from((GEN_FLOW *)(from), (GEN_FLOW *)(to))
#define empty_link() _create_link_from(NULL, NULL)

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
   /* int (*cmp)(BYTES lhs, BYTES rhs); */
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
   char value[128];
   FLOW_LINK next;
} NODE_FLOW;

/* Begin File Window Structures and Defines */

#define MAX_PATH 255
#define MAX_FILES 1048

typedef enum _ftype {
   BLK_DEV,
   CHAR_DEV,
   DIRECTORY,
   SYMLINK,
   RG_FILE,
   FIFO,
   SOCKET
} F_TYPE;

typedef enum _permissions{
   NONE   
} PERMISSIONS;

struct file_info {
   char name[MAX_PATH * 1];
   F_TYPE type;
};

struct dir_info{
   int count;
   struct file_info files[MAX_FILES];
   char *file_names[MAX_FILES];
   char absolute_path[MAX_PATH + 1];
};

typedef struct _filewin{
   int x, y;
   int width, height;
   struct dir_info directory;
   char load_file[PATH_MAX];
   BUTTON load;
   BUTTON close;
   OPTION_PANEL buttons;
   int visible;
} FILEWIN;

/* Begin Model Saving Structures */

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

/* Start Global Variables and Defines */

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
extern int node_id;

/* Start Flow Functions */

char *gen_type_str(FLOW type);

void draw_link(FLOW_LINK *link, Color color);
void draw_branch(GEN_FLOW *branch);
void draw_node(GEN_FLOW *flow);
void draw_graph(GEN_FLOW *graph);
void draw_nodes(GEN_FLOW *nodes[], int num_nodes);
void draw_directioned_arrow(int x, int y, int startx, int starty, int endx, int endy);
int uuid_to_index(char *uuid, GEN_FLOW *nodes[], int length);

FLOW_LINK _create_link_from(GEN_FLOW *from, GEN_FLOW *to);

void free_graph(GEN_FLOW *nodes[], int num_nodes);
void delete_node(NODE_FLOW *node);
void delete_branch(BRANCH_FLOW *branch);
void delete_flow(GEN_FLOW *flow);

GEN_FLOW *create_branch(BRANCH_TYPE btype, int x, int y);
void set_branch_links(BRANCH_FLOW *branch, GEN_FLOW *yes, GEN_FLOW *no);

GEN_FLOW *create_node(char *value, int x, int y);
void set_node_link(NODE_FLOW *node, GEN_FLOW *next);

void remove_node(GEN_FLOW *removed, GEN_FLOW *nodes[], int length);

GEN_FLOW *get_node_at(GEN_FLOW *nodes[], int num_nodes, int x, int y);

// FLOW_LINK *execute_branch(BRANCH_FLOW *branch);
int int_cmp  (BYTES lhs, BYTES rhs);
int float_cmp(BYTES lhs, BYTES rhs);
int str_cmp  (BYTES lhs, BYTES rhs);

/* Begin File Window Functions */

void set_directory_path(struct dir_info *directory, char path[MAX_PATH + 1]);
int get_directory_info(struct dir_info *directory);
char *ftype_str(F_TYPE type);
void file_permissions(char *filename, F_TYPE *file_type);

void init_filewin(FILEWIN *filwwin, int x, int y, int width, int height);
void draw_filewin(FILEWIN *filewin);
void set_cwd(FILEWIN *filewin);
int update_filewin_info(FILEWIN *filewin, GEN_FLOW *graph[], int *num_nodes);
void uninit_filewin(FILEWIN *filewin);

/* Start Panel Functions */

void init_panels(void);
void show_branch_flow(void);
void show_node_flow(void);
int update_branch_panel(GEN_FLOW *nodes[], int *length);
void reset_branch_panel();
void reset_node_panel();
int update_node_panel(GEN_FLOW *nodes[], int *length);
void update_panel(GEN_FLOW *nodes[], int *length);
void clear_branch_panel();
void clear_node_panel();

/* Being Model Saving Functions */

void branch_flow_info(BRANCH_FLOW *flow, BRANCH_INFO *info);
void node_flow_info(NODE_FLOW *flow, NODE_INFO *info);
void branch_from_info(BRANCH_INFO *info, BRANCH_FLOW *flow);
void node_from_info(NODE_INFO *info, NODE_FLOW *flow);
void link_from_uuid(GEN_FLOW *from, FLOW_LINK *link, char *uuid, int x, int y, GEN_FLOW *nodes[], int length);
void save_model(GEN_FLOW *flow[], int length);
int load_model(GEN_FLOW *flow[], int *length);

/* Miscellaneous Functions */
void set_global_message(char *msg);
void update_globals(void);
