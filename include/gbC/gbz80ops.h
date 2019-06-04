#ifndef GBZ80OPS_INCLUDED
#define GBZ80OPS_INCLUDED

#include "state.h"
    // $00
    void nop(struct gb_state* state); 
    // $10
    void stop(struct gb_state* state);
    // $20
    void jrNZr8(struct gb_state* state);
    // $30
    void jrNCr8(struct gb_state* state);
    // $01
    void ldBCn16(struct gb_state* state);
    // $11
    void ldDEn16(struct gb_state* state);
    // $12
    void ldHLn16(struct gb_state* state);
    // $13
    void ldSPn16(struct gb_state* state);


#endif