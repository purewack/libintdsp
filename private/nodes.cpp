#include "../libintdsp.h"
#include "../osc_t.h"
#include "../adr_t.h"

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
  
  intp = ( (s_h-s_l)*phi_dt ) >> 8;
  oo->io->out = oo->bias + (((s_l + intp)*oo->gain)>>8);

  if(oo->io->deps_count)
    oo->phi += oo->io->in;
  else
    oo->phi += oo->acc;
}
node_t* new_osc(agraph_t* gg, char* sig){
    node_t* b = new_node(gg,sig);
    osc_t* n = (osc_t*)malloc(sizeof(osc_t));
    LOGL("new node: osc");
    LOGL(sig);
    n->bias = 0;
    n->acc = 900;
    n->table = sint;
    n->gain = 255;
    b->processor = n;
    b->processor_func = proc_osc;
    n->io = b;
    return b;
}
void set_osc_freq(osc_t* oo, uint32_t f_big, uint32_t srate){
  //256 = fixed int resolution for multiplication
  auto a = LUT_COUNT*256*f_big;
  oo->acc = a / srate / 10;
}

node_t* new_adr(agraph_t* gg, char* sig){
    node_t* b = new_node(gg,sig);
    adr_t* n = (adr_t*)malloc(sizeof(adr_t));
    b->processor = n;
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
  auto aa = (adr_t*)ad;
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
  auto s = srate*ms / 1000;
  a->a_v = (1<<20) / s; //16777216.f * 2.f/32000.f)
}
void set_adr_release_ms(adr_t* a, uint32_t ms, uint32_t srate){
  auto s = srate*ms / 1000;
  a->r_v = (1<<20) / s; //16777216.f * 2.f/32000.f)
}

// void proc_lpf(lpf_t* p){
//   p->h = p->h + ((p->a * (p->spl-p->h))>>12);
//   p->h2 = p->h2 + ((p->a * (p->h-p->h2))>>12);
//   p->h3 = p->h3 + ((p->a * (p->h2-p->h3))>>12);
//   p->spl = p->h3;
// }
