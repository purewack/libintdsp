#include "../libintdsp.h"

spl_t sint[LUT_COUNT];
spl_t sawt[LUT_COUNT];

void libintdsp_init(agraph_t* gg,  int16_t(*sin_fn)(int16_t phase)){
    LOGL("libintdsp->init(): init graph and lookup tables");
    gg->nodes_count = 0;
    gg->wires_count = 0;
    gg->stale = 0;
	
    for(int i=0; i<LUT_COUNT; i++){
        sawt[i] = (i<<8) - ((1<<15)-1);
        sint[i] = sin_fn(i);
    }
}
