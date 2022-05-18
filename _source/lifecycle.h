#pragma once
#include "types.h"

node_t* new_node(agraph_t* gg, char* sig);
void del_node(agraph_t* gg, node_t* n);

node_t* new_osc(agraph_t* graph, char* signature);

node_t* new_adr(agraph_t* graph, char* signature);

node_t* new_lpf(agraph_t* graph, char* signature);
