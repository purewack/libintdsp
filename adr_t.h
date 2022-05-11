#pragma once
#include "libintdsp.h"

struct adr_t{
  node_t* io;
  int32_t a_v, r_v, a;
  int32_t acc,vv,v;
  bool state, old_state;
};

node_t* new_adr(agraph_t* graph, char* signature);
void proc_adr(adr_t* processor);