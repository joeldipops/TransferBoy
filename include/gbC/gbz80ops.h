#ifndef GBZ80OPS_INCLUDED
#define GBZ80OPS_INCLUDED

#include "../../core.h"
#include "types.h"

void rlcR8(GbState* state, byte op);
void rrcR8(GbState* state, byte op);
void rlR8(GbState* state, byte op);
void rlR8(GbState* state, byte op);
void rrR8(GbState* state, byte op);
void slaR8(GbState* state, byte op);
void sraR8(GbState* state, byte op);
void swapR8(GbState* state, byte op);
void srlR8(GbState* state, byte op);
void bitU3R8(GbState* state, byte op);
void resU3R8(GbState* state, byte op);
void setU3R8(GbState* state, byte op);
void ext(GbState* state, byte op);
void ldR8N8(GbState* state, byte op);
void ldR8R8(GbState* state, byte op);
void ldAaR16(GbState* state, byte op);
void ldAaN16(GbState* state, byte op);
void ldaR16A(GbState* state, byte op);
void ldaN16A(GbState* state, byte op);
void ldAaC(GbState* state, byte op);
void ldaCA(GbState* state, byte op);
void lddAaHL(GbState* state, byte op);
void lddaHLA(GbState* state, byte op);
void ldiaHLA(GbState* state, byte op);
void ldiAaHL(GbState* state, byte op);
void ldhAaN8(GbState* state, byte op);
void ldhaN8A(GbState* state, byte op);
void ldR16N16(GbState* state, byte op);
void ldSPHL(GbState* state, byte op);
void ldHLSPN8(GbState* state, byte op);
void ldaR16SP(GbState* state, byte op);
void pushR16(GbState* state, byte op);
void popR16(GbState* state, byte op);
void addAR8(GbState* state, byte op);
void addAN8(GbState* state, byte op);
void adcAR8(GbState* state, byte op);
void adcAN8(GbState* state, byte op);
void subAR8(GbState* state, byte op);
void subAN8(GbState* state, byte op);
void sbcAR8(GbState* state, byte op);
void sbcAN8(GbState* state, byte op);
void andAR8(GbState* state, byte op);
void andAN8(GbState* state, byte op);
void orAR8(GbState* state, byte op);
void orAN8(GbState* state, byte op);
void xorAR8(GbState* state, byte op);
void xorAN8(GbState* state, byte op);
void cpAR8(GbState* state, byte op);
void cpAN8(GbState* state, byte op);
void incR8(GbState* state, byte op);
void decR8(GbState* state, byte op);
void incR16(GbState* state, byte op);
void decR16(GbState* state, byte op);
void addHLR16(GbState* state, byte op);
void addSPR8(GbState* state, byte op);
void daa(GbState* state, byte op);
void cpl(GbState* state, byte op);
void ccf(GbState* state, byte op);
void scf(GbState* state, byte op);
void rlcA(GbState* state, byte op);
void rlA(GbState* state, byte op);
void rrcA(GbState* state, byte op);
void rrA(GbState* state, byte op);
void nop(GbState* state, byte op);
void halt(GbState* state, byte op);
void stop(GbState* state, byte op);
void di(GbState* state, byte op);
void ei(GbState* state, byte op);
void jpN16(GbState* state, byte op);
void jpCCN16(GbState* state, byte op);
void jpaHL(GbState* state, byte op);
void jrCCR8(GbState* state, byte op);
void jrR8(GbState* state, byte op);
void jrN8(GbState* state, byte op);
void jrCCN8(GbState* state, byte op);
void callN16(GbState* state, byte op);
void callCCN16(GbState* state, byte op);
void rstVec(GbState* state, byte op);
void retCC(GbState* state, byte op);
void reti(GbState* state, byte op);
void ret(GbState* state, byte op);

// handler for undefined op codes.
void undefined(GbState* state, byte op);

#endif