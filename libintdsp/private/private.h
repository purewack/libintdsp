#pragma once
#include "config.h"
#include "../libintdsp.h"

extern spl_t sint[LUT_COUNT];
extern spl_t sawt[LUT_COUNT];

node_t* new_node(agraph_t* gg, char* sig);
void del_node(agraph_t* gg, node_t* n);