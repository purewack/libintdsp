#pragma once
#include <stdint.h>
typedef int16_t spl_t;

typedef struct node_t{
    struct node_t* deps[6];
    int deps_count;
    char* sig;
    spl_t in;
    spl_t out;
    
    //[in] already pre summed at time of calling processor_func
    void* processor;
    void (*processor_func)(void*);
} node_t; 

typedef struct wire_t{
    node_t* src;
    node_t* dst;
} wire_t;

typedef struct agraph_t{
    node_t** nodes;
    int nodes_count;
    
    wire_t* wires;
    int wires_count;
    
    uint8_t stale;
} agraph_t;


typedef struct dac_t{
    node_t* io;
    spl_t* out_spl;
} dac_t;


typedef struct osc_t  {
  node_t* io;
  uint16_t phi;
  uint16_t acc;
  int16_t bias;
  int16_t gain;
  int16_t* table;
} osc_t;

typedef struct adr_t{
  node_t* io;
  int32_t a_v, r_v, a;
  int32_t vv,v;
  uint16_t state, old_state;
} adr_t;

typedef struct lpf_t{
  node_t* io;
  int32_t h, h2, h3;
  int32_t a;
  int32_t a_big;
  int32_t a_target;
  int32_t a_dt;
} lpf_t;
