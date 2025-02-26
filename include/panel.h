#pragma once

#include "flow.h"

void init_panels(void);
void show_branch_flow(void);
void show_node_flow(void);
int update_branch_panel(GEN_FLOW *nodes[], int *length);
int update_node_panel(GEN_FLOW *nodes[], int *length);
