#pragma once
#include "libintdsp.h"

struct osc_t{
  node_t* io;
  uint16_t phi;
  uint16_t acc;
  int16_t bias;
  int16_t gain;
  int16_t* table;
};
//range 0-65xxx
node_t* new_osc(agraph_t* graph, char* signature);
void proc_osc(void* processor);
//f_10 = desired freq * 10, i.e. provide 503 for 50.3hz, 200,005 for 20.5kHz etc
void set_osc_freq(osc_t* processor, uint32_t f_10, uint32_t srate);
