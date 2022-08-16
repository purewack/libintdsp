#include "../libintdsp.h"
#include <stdlib.h>

node_t* new_node(agraph_t* gg, char* sig){
    node_t* n = (node_t*)malloc(sizeof(node_t));
    n->deps_count = 0;
    n->sig = sig;
    if(gg->nodes_count)
        gg->nodes = (node_t**)realloc(gg->nodes, sizeof(node_t*) * (gg->nodes_count+1));
    else
        gg->nodes = (node_t**)malloc(sizeof(node_t*) * 1);
        
    gg->nodes[gg->nodes_count] = n;
    gg->nodes_count++;
    
    gg->stale = 1;
    
    return n;
}

void del_node(agraph_t* gg, node_t* n){
    LOGL("del_node():");
    LOGL(n->sig);
    // for(int i=0; i<gg->wires_count; i++){
    //     int dis = 0;
    //     if(gg->wires[i].src == n) dis = 1;
    //     if(gg->wires[i].dst == n) dis = 1;
    // }
}


/////////////////////////////////////////////////////////

node_t* new_dac(agraph_t* gg, char* sig, int16_t* out_spl){
    node_t* b = new_node(gg,sig);
    dac_t* n = (dac_t*)malloc(sizeof(dac_t));
    LOGL("new node: created");
    b->processor = n;
    b->processor_func = proc_dac;
    n->io = b;
    n->out_spl = out_spl;
    return b;
}
void proc_dac(void* v){
    dac_t* d = (dac_t*)v;
    *(d->out_spl) = d->io->in;
}



/////////////////////////////////////////////////////////


node_t* new_osc(agraph_t* gg, char* sig){
    node_t* b = new_node(gg,sig);
    osc_t* n = (osc_t*)malloc(sizeof(osc_t));
    LOGL("new node: osc");
    LOGL(sig);
    n->bias = 0;
    n->acc = 900;
    n->table = sint;
    n->gain = 255;
    b->processor = (node_t*)n;
    b->processor_func = proc_osc;
    n->io = b;
    return b;
}

void proc_osc(void* v){
  osc_t* oo = (osc_t*)v;
  int16_t phi_dt, phi_l, phi_h;
  int16_t s_l, s_h;
  int16_t intp;

  phi_dt = oo->phi & 0xFF;
  phi_l = oo->phi >> 8;
  phi_h = (phi_l + 1);
  phi_h = phi_h & 0xFF;

  s_l = oo->table[phi_l];
  s_h = oo->table[phi_h];
  
  intp = lin_interp32(s_l,s_h,phi_dt,8);
  oo->io->out = oo->bias + (((intp)*oo->gain)>>8);

  if(oo->io->deps_count)
    oo->phi += oo->io->in;
  else
    oo->phi += oo->acc;
}

void set_osc_freq(osc_t* oo, uint32_t f_big, uint32_t srate){
  //256 = fixed int resolution for multiplication
  int32_t a = LUT_COUNT*256*f_big;
  oo->acc = a / srate / 10;
}

/////////////////////////////////////////////////////////

node_t* new_adr(agraph_t* gg, char* sig){
    node_t* b = new_node(gg,sig);
    adr_t* n = (adr_t*)malloc(sizeof(adr_t));
    b->processor = (node_t*)n;
    b->processor_func = proc_adr;
    n->io = b;
    n->v = 0;
    n->a_v = 1000;
    n->r_v = 1000;
    n->state = 0;
    n->old_state = 0;
    return b;
}
void proc_adr(void* ad){
  adr_t* aa = (adr_t*)ad;
  if(aa->state && !aa->old_state)
    aa->a = aa->a_v;
  else if(!aa->state && aa->old_state)
    aa->a = -aa->r_v;
  if(aa->state != aa->old_state)
    aa->old_state = aa->state;
  
  //if(aa->aa == 0) return;
  
  aa->vv += aa->a;
  if(aa->vv > 1<<20) aa->vv = (1<<20) - 1;
  if(aa->vv < 0) aa->vv = 0;
  aa->v = aa->vv >> 8;
  aa->io->out = (aa->io->in*aa->v)>>12; 
}

void set_adr_attack_ms(adr_t* a, uint32_t ms, uint32_t srate){
  int32_t s = srate*ms / 1000;
  a->a_v = (1<<20) / s; //16777216.f * 2.f/32000.f)
}
void set_adr_release_ms(adr_t* a, uint32_t ms, uint32_t srate){
  int32_t s = srate*ms / 1000;
  a->r_v = (1<<20) / s; //16777216.f * 2.f/32000.f)
}

//////////////////////////////////////////////////////////

node_t* new_lpf(agraph_t* gg, char* sig){
    node_t* b = new_node(gg,sig);
    lpf_t* n = (lpf_t*)malloc(sizeof(lpf_t));
    b->processor = (node_t*)n;
    b->processor_func = proc_lpf;
    n->io = b;
    n->h = 0;
    n->h2 = 0;
    n->h3 = 0;
    n->a = (1<<12) - 1;
    return b;
}

void proc_lpf(void* l){
  lpf_t* p = (lpf_t*)l;
  p->h = p->h + ((p->a * (p->io->in - p->h))>>12);
  p->h2 = p->h2 + ((p->a * (p->h - p->h2))>>12);
  p->h3 = p->h3 + ((p->a * (p->h2 - p->h3))>>12);
  p->io->out = p->h3;
  
  p->a_big += p->a_dt;
  p->a = p->a_big>>8;
  if(p->a == p->a_target) p->a_dt = 0;
}

void set_lpf_freq(lpf_t* l, uint32_t f_norm, uint32_t srate){
  l->a_target = 0;
  l->a_dt = 0;
  l->a_big = (f_norm<<12) / srate;
  l->a_big <<= 8;
}

void set_lpf_freq_slide(lpf_t* l, uint32_t f_norm, uint32_t t_ms, uint32_t srate){
  l->a_target = (f_norm<<12) / srate;

  int32_t s = srate*t_ms / 1000;
  l->a_dt = (1<<20) / s; 
  if(l->a_target < l->a) l->a_dt *= -1;
}