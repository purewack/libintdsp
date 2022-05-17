#pragma once 
#include "libintdsp.h"

typedef struct lpf_t lpf_t;

struct lpf_t{
  node_t* io;
  int32_t h, h2, h3;
  int32_t a;
};

node_t* new_lpf(agraph_t* graph, char* signature);
void proc_lpf(void* node);
void set_lpf_freq(lpf_t* processor, uint32_t f_0, uint32_t srate);