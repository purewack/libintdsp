#include <I2S.h>

typedef struct AGRAPH_T {
  
} agraph_t;

typedef struct OSC_T{
  uint16_t phi;
  uint16_t acc;
  int16_t* table;
  int16_t spl;
} osc_t;

typedef struct ADR_T{
  int16_t spl;
  int32_t a_v, r_v, a;
  int32_t acc,vv,v;
  bool state, old_state;
} adr_t;

typedef struct LPF_P_T{
  int16_t spl, prev;
  int32_t f;
} lpf_p_t;

//range 0-65xxx
void osc_proc(osc_t* oo){
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
  oo->spl = s_l + intp;
}

void adr_proc(adr_t* aa){
  if(aa->state && !aa->old_state)
    aa->a = aa->a_v;
  else if(!aa->state && aa->old_state)
    aa->a = aa->r_v;
  if(aa->state != aa->old_state)
    aa->old_state = aa->state;
  
  //if(aa->aa == 0) return;
  
  aa->vv += aa->a;
  if(aa->vv > 1048576) aa->vv = 1048576;
  if(aa->vv < 0) aa->vv = 0;
  aa->v = aa->vv >> 8;
  
  aa->acc = (aa->spl * aa->v);
  aa->spl = aa->acc >> 12; 
}

void lpf_p_proc(lpf_p_t* p){
    
}

int16_t sint[256];
int16_t sawt[256];
osc_t* hfo;
osc_t* lfo;
adr_t* adr;

void graph_proc(){
  
//  osc_proc(lfo);
//  lfo->phi += lfo->acc;
//  
//  osc_proc(hfo);
//  hfo->phi += hfo->acc + (lfo->out>>8);

  osc_proc(hfo);
  hfo->phi += hfo->acc;

  adr->spl = hfo->spl;
  adr_proc(adr);
  i2s_write_lr(adr->spl,adr->spl);
  //i2s_write_lr(hfo->spl,hfo->spl);
}

void setup() {
  for(int i=0; i<256; i++){
    sawt[i] = int16_t(16000.f * float(i)/256.f );
    sint[i] = int16_t(16000.f * sin(2.0f * 3.1415f * float(i)/256.f));
  }
  
  lfo = new osc_t;
  hfo = new osc_t;

  lfo->acc = 2;
  hfo->acc = 900; //t_size * f / FS
  lfo->table = sawt;
  hfo->table = sawt;

  adr = new adr_t;
  adr->a_v = int32_t(1048576.f / (0.3f * 32000.f)); //16777216.f * 2.f/32000.f)
  adr->r_v = int32_t(-1048576.f / (0.4f * 32000.f));
  adr->state = 0;
  adr->old_state = 0;

  pinMode(D1,INPUT);
  
  i2s_begin();
  i2s_set_rate(32000.f);

}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0; i<256; i++)
    graph_proc();

  adr->state = digitalRead(D1);
  //yield();
}
