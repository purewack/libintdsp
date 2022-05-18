#include "../libintdsp.h"
#include <stdlib.h>

int connect(agraph_t* gg, node_t* src, node_t* dst){
    
    uint8_t dupe = 0;
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
        
        node_t* remain[64];
        int remain_count = 0;
        
        //get remaining nodes
        for(int i=0; i<gg->nodes_count; i++){
            node_t* n = gg->nodes[i];
            
            int rem = 1;
            for(int j=0; j<sorted_count; j++){
                node_t* s = sorted[j];
                if(n == s) rem = 0;
            }
                
            if(rem) {
                remain[remain_count] = n;
                remain_count++;
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
    
    #ifdef DEBUG
    #ifdef VERBOSE
    LOGL("sorted nodes========\n");
    for(int i=0; i < gg->nodes_count; i++){
        node_t* n = gg->nodes[i];
        LOGNL(i);
        LOGNL(" ");
        LOGL(n->sig);
        
        if(n->deps_count == 0) continue;
        
        LOGL("\tDeps:");
        for(int j=0; j<n->deps_count;j++){
            node_t* d = n->deps[j];
            LOGNL("\t");
            LOGNL(d->sig);
        }
        LOGL("");
    }
    #endif
    #endif
}


void proc_graph(agraph_t* gg){
    if(gg->stale) recalc_graph(gg);

    for(int i=0; i < gg->nodes_count; i++){
        node_t* n = gg->nodes[i];	
        n->in = 0;
        
        if(n->deps_count){
            for(int j=0; j < n->deps_count; j++){
                node_t* d = n->deps[j];
				if(n && d)
                n->in += d->out;
            }
        }
        
		n->processor_func((void*)n->processor);
        
    }
}

