#include <I2S.h>

#define DEBUG
#define VERBOSE
#define LOGL(X) Serial.println(X)
#define LOGNL(X) Serial.print(X)
#include "libintdsp/libintdsp.h"
#include "libintdsp/osc_t.h"
#include "libintdsp/private/libintdsp.cpp"
#include "libintdsp/private/nodes.cpp"

int16_t spl_out_a,spl_out_b;
agraph_t gg;

void setup() {
  Serial.begin(19200);
  
  libintdsp_init(&gg);

  pinMode(D1,INPUT);
  pinMode(D2,INPUT);
  auto os1 = new_osc(&gg,"osca");
  auto os2 = new_osc(&gg,"oscb");
    auto* os1_params = (osc_t*)os1->processor;
    auto* os2_params = (osc_t*)os2->processor;
    os1_params->acc = 1800;
    os2_params->acc = 1200;
    os1_params->gain = 20;
    os2_params->gain = 20;

  auto* os3 = new_osc(&gg,"oscc");
    auto* os3_params = (osc_t*)os3->processor;
    os3_params->acc = 750;
    os3_params->gain = 30;
    
  auto* lfo = new_osc(&gg,"lfo");
    auto* lfo_params = (osc_t*)lfo->processor;
    lfo_params->acc = 4;
    lfo_params->bias = 1500;
    lfo_params->gain = 5;
    lfo_params->table = sint;

  auto* dac = new_dac(&gg,"dac",&spl_out_a);
  auto* dac2 = new_dac(&gg,"dac2",&spl_out_b);

  LOGL("connecting");
  connect(&gg,os1,dac);
  connect(&gg,os2,dac);
  connect(&gg,os3,dac2);
  connect(&gg,lfo,os3);
  LOGL("connected");

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
