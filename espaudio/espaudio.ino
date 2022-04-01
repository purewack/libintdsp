#include <I2S.h>
typedef struct NODE_T node_t;
typedef struct NODE_T{
    node_t* deps[6];
    int deps_count;
    char* sig;
    int in;
    int out;
    
    //[in] already pre summed
    void* processor;
    void (*processor_func)(void*);
} node_t;

typedef struct GRAPH_T agraph_t;
node_t* new_node(agraph_t* gg, char* sig);
void del_node(agraph_t* gg, node_t* n);
int connect(agraph_t* gg, node_t* src, node_t* dst);
void disconnect(agraph_t* gg, int con);
void recalc_graph(agraph_t* gg);
void proc_graph(agraph_t* gg);


int16_t sint[256];
int16_t sawt[256];
int16_t spl_out_a,spl_out_b;

typedef struct DAC_T {
    node_t* io;
    int16_t* out_spl;
} dac_t;
void proc_dac(void* v){
    dac_t* d = (dac_t*)v;
    *(d->out_spl) = d->io->in;
}
node_t* new_dac(agraph_t* gg, char* sig, int16_t* out_spl){
    node_t* b = new_node(gg,sig);
    dac_t* n = (dac_t*)malloc(sizeof(dac_t));
    Serial.println("creaded processor dac");
    b->processor = n;
    b->processor_func = proc_dac;
    n->io = b;
    n->out_spl = out_spl;
    return b;
}


typedef struct OSC_T{
  node_t* io;
  uint16_t phi;
  uint16_t acc;
  int16_t bias;
  int16_t gain;
  int16_t* table;
} osc_t;
//range 0-65xxx
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
    Serial.println("creaded processor osc");
    n->bias = 0;
    n->acc = 900;
    n->table = sint;
    n->gain = 255;
    b->processor = n;
    b->processor_func = proc_osc;
    n->io = b;
    return b;
}


// typedef struct ADR_T{
//   int16_t spl;
//   int32_t a_v, r_v, a;
//   int32_t acc,vv,v;
//   bool state, old_state;
// } adr_t;
// void proc_adr(adr_t* aa){
//   if(aa->state && !aa->old_state)
//     aa->a = aa->a_v;
//   else if(!aa->state && aa->old_state)
//     aa->a = -aa->r_v;
//   if(aa->state != aa->old_state)
//     aa->old_state = aa->state;
  
//   //if(aa->aa == 0) return;
  
//   aa->vv += aa->a;
//   if(aa->vv > 1048576) aa->vv = 1048576;
//   if(aa->vv < 0) aa->vv = 0;
//   aa->v = aa->vv >> 8;
  
//   aa->acc = (aa->spl * aa->v);
//   aa->spl = aa->acc >> 12; 
// }
// node_t* new_adr(agraph_t* gg, char* sig){
//     node_t* b = new(gg,sig);
//     adr_t* n = malloc(sizeof(adr_t));
//     b->processor = n;
//     b->processor_func = proc_adr;
//     n->io = b;
//     return b;
// }

// typedef struct LPF_T{
//   int16_t spl, h, h2, h3;
//   int16_t a;
// } lpf_t;
// void proc_lpf(lpf_t* p){
//   p->h = p->h + ((p->a * (p->spl-p->h))>>12);
//   p->h2 = p->h2 + ((p->a * (p->h-p->h2))>>12);
//   p->h3 = p->h3 + ((p->a * (p->h2-p->h3))>>12);
//   p->spl = p->h3;
// }





typedef struct WIRE_T{
    node_t* src;
    node_t* dst;
} wire_t;

typedef struct GRAPH_T{
    node_t** nodes = nullptr;
    int nodes_count = 0;
    
    wire_t* wires = nullptr;
    int wires_count = 0;
    
    int stale;
} agraph_t;


node_t* new_node(agraph_t* gg, char* sig){
    //delay(2000);
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
    for(int i=0; i<gg->wires_count; i++){
        int dis = 0;
        if(gg->wires[i].src == n) dis = 1;
        if(gg->wires[i].dst == n) dis = 1;
    }
}

int connect(agraph_t* gg, node_t* src, node_t* dst){
    
    int dupe = 0;
    int wc = gg->wires_count;
    for(int i=0; i < wc; i++){
        if(gg->wires[i].src == src){
            for(int j=0; j < wc; j++){
                if(gg->wires[i].dst == dst){
                    dupe = 1;
                    break;
                }
            }   
        }
    }
    
    if(dupe) return -1;
    
    gg->wires_count++;
    wire_t* w = (wire_t*)malloc(sizeof(wire_t) * gg->wires_count);
    
    for(int i=0; i<wc; i++)
        w[i] = gg->wires[i];
        
    w[gg->wires_count-1].src = src;
    w[gg->wires_count-1].dst = dst;
    
    if(gg->wires)
        free(gg->wires);
    
    gg->wires = w;
    gg->stale = 1;
    return gg->wires_count-1;
}

void disconnect(agraph_t* gg, int con){
    if(con < 0 || con > gg->wires_count-1) return;
    
    int wc = gg->wires_count;
    gg->wires_count--;
    wire_t* w = (wire_t*)malloc(sizeof(wire_t) * gg->wires_count);
    
    int k = 0;
    for(int i=0; i<wc; i++){
        if(i != con)
            w[k++] = gg->wires[i];
    }
    
    if(gg->wires)
        free(gg->wires);
    
    gg->stale = 1;
    gg->wires = w;
}

void recalc_graph(agraph_t* gg){
    if(gg->nodes_count == 0) return;
    if(gg->wires_count == 0) return;
    gg->stale = 0;

    //recalc deps from wires
    for(int i=0; i<gg->nodes_count; i++){
        gg->nodes[i]->deps_count = 0;
    }
    for(int i=0; i<gg->wires_count; i++){
        wire_t* w = &gg->wires[i];
        w->dst->deps[w->dst->deps_count] = w->src;
        w->dst->deps_count++;
    }
    
    //prepoulate sorted with indeps
    node_t** sorted = (node_t**)malloc(gg->nodes_count * sizeof(node_t*));
    int sorted_count = 0;
    for(int i=0; i<gg->nodes_count; i++){
        node_t* n = gg->nodes[i];
        if(n->deps_count == 0){
            sorted[sorted_count] = (node_t*)malloc(sizeof(node_t));
            sorted[sorted_count] = n;
            sorted_count++;
        }
    }
  
    //sort it out lol
    while(sorted_count != gg->nodes_count){
        
        node_t* remain[gg->nodes_count];
        int remain_count = 0;
        
        //get remaining nodes
        for(int i=0; i<gg->nodes_count; i++){
            node_t* n = gg->nodes[i];
            
            int rem = 1;
            for(int j=0; j<sorted_count; j++){
                node_t* s = sorted[j];
                //Serial.println("%p (%s) %p (%s)\n",s,s->sig,n,n->sig);
                if(n == s) rem = 0;
            }
                
            if(rem) {
                remain[remain_count] = n;
                remain_count++;
                //Serial.println("%p %s\n",n,n->sig);
            }
        }
        
        
        //go through remain and determine next indep
        for(int i=0; i<remain_count; i++){
            node_t* r = remain[i];
            
            int deps_done = 0;
            int dp = r->deps_count;
            
            for(int j=0; j<dp; j++){
                node_t* cd = r->deps[j];
                
                for(int k=0; k<sorted_count; k++){
                    node_t* s = sorted[k];
                    if(s == cd) deps_done += 1;
                }
            }
            
            if(deps_done == r->deps_count){
                sorted[sorted_count] = r;
                sorted_count++;
                break;
            }
        }
        

    }

    //replace realoc array
    if(gg->nodes)
        free(gg->nodes);
    gg->nodes = sorted;
    
    
    // Serial.println("sorted nodes========\n");
    // for(int i=0; i < gg->nodes_count; i++){
    //     node_t* n = gg->nodes[i];
    //     Serial.print(i);
    //     Serial.print(" ");
    //     Serial.println(n->sig);
        
    //     if(n->deps_count == 0) continue;
        
    //     Serial.println("\tDeps:\n");
    //     for(int j=0; j<n->deps_count;j++){
    //         node_t* d = n->deps[j];
    //         Serial.print("\t");
    //         Serial.println(d->sig);
    //     }
    //     Serial.println("\n");
    // }
}

void proc_graph(agraph_t* gg){
    if(gg->stale) recalc_graph(gg);

    for(int i=0; i < gg->nodes_count; i++){
        node_t* n = gg->nodes[i];
        n->in = 0;
        
        
        //Serial.println(">>> processing node %p (%s)->[%d] \n",n,n->sig,n->deps_count);
        
        if(n->deps_count){
            for(int j=0; j < n->deps_count; j++){
                node_t* d = n->deps[j];
                n->in += d->out;
                //Serial.println("\t using -> %p (%s) = %d\n",d,d->sig,d->out);
            }
        }
        
        //Serial.println(">>> proc %p \n",n->processor);
        n->processor_func(n->processor);
        //Serial.println("\tres[%d]",n->out);
        //Serial.println("\tin=%d, out=%d \n",n->in,n->out);
        
    }
}

// osc_t* os2;
// osc_t* os3;
// osc_t* lfo;
// adr_t* adr;
// adr_t* fadr;
// lpf_t* lpf;

//void graph_proc(){
  
// //  osc_proc(lfo);
// //  lfo->phi += lfo->acc;
// //  
// //  osc_proc(hfo);
// //  hfo->phi += hfo->acc + (lfo->out>>8);

//   osc_proc(lfo);
//   lfo->phi += lfo->acc;
//   fadr->spl = lfo->spl;
//   adr_proc(fadr);
//   lfo->spl = fadr->spl;
  
//   osc_proc(os1);
//   osc_proc(os2);
//   osc_proc(os3);
//   os1->phi += os1->acc + (lfo->spl>>8);
//   os2->phi += os2->acc + (lfo->spl>>9);
//   os3->phi += os3->acc ;//+ (lfo->spl>>8);

//   adr->spl = os1->spl + os2->spl + os3->spl;
//   adr_proc(adr);

//   lpf->spl = adr->spl;
//   lpf->a = 1000 + fadr->v>>1;
//   lpf_proc(lpf);
  
  //i2s_write_lr(hfo->spl,hfo->spl);
//}

agraph_t gg;
node_t* dac2;
node_t* os4;

int b, ob;
void setup() {
  Serial.begin(19200);
  
  for(int i=0; i<256; i++){
    sawt[i] = int16_t(5000.f * float(i)/256.f );
    sint[i] = int16_t(5000.f * sin(2.0f * 3.1415f * float(i)/256.f));
  }
  
  pinMode(D1,INPUT);
  pinMode(D2,INPUT);

  node_t* os1 = new_osc(&gg,"osca");
  node_t* os2 = new_osc(&gg,"oscb");
    osc_t* os1_params = (osc_t*)os1->processor;
    osc_t* os2_params = (osc_t*)os2->processor;
    os1_params->acc = 1800;
    os2_params->acc = 1200;
  node_t* dac = new_dac(&gg,"dac",&spl_out_a);

  node_t* os3 = new_osc(&gg,"oscc");
    osc_t* os3_params = (osc_t*)os3->processor;
    os3_params->acc = 750;
    
  node_t* lfo = new_osc(&gg,"lfo");
    osc_t* lfo_params = (osc_t*)lfo->processor;
    lfo_params->acc = 4;
    lfo_params->bias = 1500;
    lfo_params->gain = 20;
    lfo_params->table = sint;

  dac2 = new_dac(&gg,"dac2",&spl_out_b);

  Serial.println("connecting");
  connect(&gg,os1,dac);
  connect(&gg,os2,dac);
  connect(&gg,os3,dac2);
  connect(&gg,lfo,os3);
  Serial.println("connected");

  // os1 = new osc_t;
  // os2 = new osc_t;
  // os3 = new osc_t;
  // lfo = new osc_t;
  // os1->acc = 900; //(t_size * f / FS) << 8
  // os2->acc = 1135; 
  // os3->acc = 1350; 
  // lfo->acc = 4;
  // os1->table = sawt;
  // os2->table = sawt;
  // os3->table = sawt;
  // lfo->table = sint;

  // adr = new adr_t;
  // adr->a_v = int32_t(1048576.f / (0.3f * 32000.f)); //16777216.f * 2.f/32000.f)
  // adr->r_v = int32_t(1048576.f / (0.4f * 32000.f));
  
  // lpf = new lpf_t;
  // fadr = new adr_t;
  // fadr->a_v = int32_t(1048576.f / (0.4f * 32000.f)); //16777216.f * 2.f/32000.f)
  // fadr->r_v = int32_t(1048576.f / (0.4f * 32000.f));
  
  i2s_begin();
  i2s_set_rate(32000.f); 
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0; i<256; i++){
    proc_graph(&gg);
    i2s_write_lr(spl_out_a,spl_out_b);
  }

  //adr->state = digitalRead(D1);
  //fadr->state = digitalRead(D2);

//  if(Serial.availableForWrite() > 8)
//    Serial.write("hello\n");
  //yield();
}
