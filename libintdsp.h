#pragma once
#include "types.h"
#include "tables.h"
#include "dsp.h"
#include "lifecycle.h"

//sin function phase FS: -2^15 -> 2^15
void libintdsp_init(agraph_t* gg,  int16_t(*sin_fn)(int16_t phase));

node_t* new_dac(agraph_t* graph, char* signature, int16_t* output_pointer);
node_t* create_node(unsigned int node_type);

int connect(agraph_t* graph, node_t* source, node_t* destination);
void disconnect(agraph_t* graph, int connection_id);
void recalc_graph(agraph_t* graph);
void proc_graph(agraph_t* graph);

void send_message(node_t* node, const char* message);
void post_message(const char* who, const char* message);


//f_10 = desired freq * 10, i.e. provide 503 for 50.3hz, 200,005 for 20.5kHz etc
void set_osc_freq(osc_t* processor, uint32_t f_10, uint32_t srate);

//~30 seconds max
void set_adr_attack_ms(adr_t* processor, uint32_t ms, uint32_t srate);
//~30 seconds max
void set_adr_release_ms(adr_t* processor, uint32_t ms, uint32_t srate);

void set_lpf_freq(lpf_t* processor, uint32_t f_0, uint32_t srate);
void set_lpf_freq_slide(lpf_t* processor, uint32_t f_0, uint32_t time_ms, uint32_t srate);