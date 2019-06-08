#include "../../core.h"
#include "types.h"

void rlcR8(GbState* s, byte op){}
void rrcR8(GbState* s, byte op){}
void rlR8(GbState* s, byte op){}
void rrR8(GbState* s, byte op){}
void slaR8(GbState* s, byte op){}
void sraR8(GbState* s, byte op){}
void swapR8(GbState* s, byte op){}
void srlR8(GbState* s, byte op){}
void bitU3R8(GbState* s, byte op){}
void resU3R8(GbState* s, byte op){}
void setU3R8(GbState* s, byte op){}
void ext(GbState* s, byte op){}
void ldR8N8(GbState* s, byte op){}
void ldR8R8(GbState* s, byte op){}
void ldAaR16(GbState* s, byte op){}
void ldAaN16(GbState* s, byte op){}
void ldaR16A(GbState* s, byte op){}
void ldaN16A(GbState* s, byte op){}
void ldAaC(GbState* s, byte op){}
void ldaCA(GbState* s, byte op){}
void lddAaHL(GbState* s, byte op){}
void lddaHLA(GbState* s, byte op){}
void ldiaHLA(GbState* s, byte op){}
void ldiAaHL(GbState* s, byte op){}
void ldhAaN8(GbState* s, byte op){}
void ldhaN8A(GbState* s, byte op){}
void ldR16N16(GbState* s, byte op){}
void ldSPHL(GbState* s, byte op){}
void ldHLSPN8(GbState* s, byte op){}
void ldaN16SP(GbState* s, byte op){}
void pushR16(GbState* s, byte op){}
void popR16(GbState* s, byte op){}
void addAR8(GbState* s, byte op){}
void addAN8(GbState* s, byte op){}
void adcAR8(GbState* s, byte op){}
void adcAN8(GbState* s, byte op){}
void subAR8(GbState* s, byte op){}
void subAN8(GbState* s, byte op){}
void sbcAR8(GbState* s, byte op){}
void sbcAN8(GbState* s, byte op){}
void andAR8(GbState* s, byte op){}
void andAN8(GbState* s, byte op){}
void orAR8(GbState* s, byte op){}
void orAN8(GbState* s, byte op){}
void xorAR8(GbState* s, byte op){}
void xorAN8(GbState* s, byte op){}
void cpAR8(GbState* s, byte op){}
void cpAN8(GbState* s, byte op){}
void incR8(GbState* s, byte op){}
void decR8(GbState* s, byte op){}
void incR16(GbState* s, byte op){}
void decR16(GbState* s, byte op){}
void addHLR16(GbState* s, byte op){}
void addSPR8(GbState* s, byte op){}
void daa(GbState* s, byte op){}
void cpl(GbState* s, byte op){}
void ccf(GbState* s, byte op){}
void scf(GbState* s, byte op){}
void rlcA(GbState* s, byte op){}
void rlA(GbState* s, byte op){}
void rrcA(GbState* s, byte op){}
void rrA(GbState* s, byte op){}
void halt(GbState* s, byte op){
    s->halt_for_interrupts = 1;    
}
void stop(GbState* s, byte op){}
void di(GbState* s, byte op){}
void ei(GbState* s, byte op){}
void jpN16(GbState* s, byte op){}
void jpCCN16(GbState* s, byte op){}
void jpaHL(GbState* s, byte op){}
void jrN8(GbState* s, byte op){}
void jrCCN8(GbState* s, byte op){}
void callN16(GbState* s, byte op){}
void callCCN16(GbState* s, byte op){}
void rstVec(GbState* s, byte op){}
void retCC(GbState* s, byte op){}
void reti(GbState* s, byte op){}

/**
 * An undefined op code should cause the cpu to hang.
 */
void undefined(GbState* s, byte op) {}