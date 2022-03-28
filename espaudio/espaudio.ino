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

typedef struct LPF_T{
  int16_t spl, h, h2, h3;
  int16_t a;
} lpf_t;

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
    aa->a = -aa->r_v;
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

void lpf_proc(lpf_t* p){
  p->h = p->h + ((p->a * (p->spl-p->h))>>12);
  p->h2 = p->h2 + ((p->a * (p->h-p->h2))>>12);
  p->h3 = p->h3 + ((p->a * (p->h2-p->h3))>>12);
  p->spl = p->h3;
}

int16_t sint[256];
int16_t sawt[256];
osc_t* os1;
osc_t* os2;
osc_t* os3;
osc_t* lfo;
adr_t* adr;
adr_t* fadr;
lpf_t* lpf;

void graph_proc(){
  
//  osc_proc(lfo);
//  lfo->phi += lfo->acc;
//  
//  osc_proc(hfo);
//  hfo->phi += hfo->acc + (lfo->out>>8);

  osc_proc(lfo);
  lfo->phi += lfo->acc;
  fadr->spl = lfo->spl;
  adr_proc(fadr);
  lfo->spl = fadr->spl;
  
  osc_proc(os1);
  osc_proc(os2);
  osc_proc(os3);
  os1->phi += os1->acc + (lfo->spl>>8);
  os2->phi += os2->acc + (lfo->spl>>9);
  os3->phi += os3->acc ;//+ (lfo->spl>>8);

  adr->spl = os1->spl + os2->spl + os3->spl;
  adr_proc(adr);

  lpf->spl = adr->spl;
  lpf->a = 1000 + fadr->v>>1;
  lpf_proc(lpf);
  
  i2s_write_lr(lpf->spl,lpf->spl);
  //i2s_write_lr(hfo->spl,hfo->spl);
}

void setup() {
  for(int i=0; i<256; i++){
    sawt[i] = int16_t(5000.f * float(i)/256.f );
    sint[i] = int16_t(5000.f * sin(2.0f * 3.1415f * float(i)/256.f));
  }
  
  os1 = new osc_t;
  os2 = new osc_t;
  os3 = new osc_t;
  lfo = new osc_t;
  os1->acc = 900; //(t_size * f / FS) << 8
  os2->acc = 1135; 
  os3->acc = 1350; 
  lfo->acc = 4;
  os1->table = sawt;
  os2->table = sawt;
  os3->table = sawt;
  lfo->table = sint;

  adr = new adr_t;
  adr->a_v = int32_t(1048576.f / (0.3f * 32000.f)); //16777216.f * 2.f/32000.f)
  adr->r_v = int32_t(1048576.f / (0.4f * 32000.f));
  
  lpf = new lpf_t;
  fadr = new adr_t;
  fadr->a_v = int32_t(1048576.f / (0.4f * 32000.f)); //16777216.f * 2.f/32000.f)
  fadr->r_v = int32_t(1048576.f / (0.4f * 32000.f));

  pinMode(D1,INPUT);
  pinMode(D2,INPUT);
  
  i2s_begin();
  i2s_set_rate(32000.f);

}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0; i<256; i++)
    graph_proc();

  adr->state = digitalRead(D1);
  fadr->state = digitalRead(D2);
  //yield();
}
