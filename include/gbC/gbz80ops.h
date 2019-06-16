#ifndef GBZ80OPS_INCLUDED
#define GBZ80OPS_INCLUDED

#include "../../core.h"
#include "types.h"

void rlcR8(GbState* state, byte op);
void rlcaHL(GbState* state, byte op);

void rrcR8(GbState* state, byte op);
void rrcaHL(GbState* state, byte op);

void rlR8(GbState* state, byte op);
void rlaHL(GbState* state, byte op);

void rrR8(GbState* state, byte op);
void rraHL(GbState* state, byte op);

void slaR8(GbState* state, byte op);
void slaaHL(GbState* state, byte op);

void sraR8(GbState* state, byte op);
void sraaHL(GbState* state, byte op);

void swapR8(GbState* state, byte op);
void swapaHL(GbState* state, byte op);

void srlR8(GbState* state, byte op);
void srlaHL(GbState* state, byte op);

void bitU3R8(GbState* state, byte op);
void bitU3aHL(GbState* state, byte op);

void resU3R8(GbState* state, byte op);
void resU3aHL(GbState* state, byte op);

void setU3R8(GbState* state, byte op);
void setU3aHL(GbState* state, byte op);

void ext(GbState* state, byte op);

void ldR8N8(GbState* state, byte op);
void ldaHLN8(GbState* state, byte op);

void ldR8R8(GbState* state, byte op);
void ldaHLR8(GbState* state, byte op);
void ldR8aHL(GbState* state, byte op);

void ldAaBC(GbState* state, byte op);
void ldAaDE(GbState* state, byte op);
void ldAaN16(GbState* state, byte op);
void ldaBCA(GbState* state, byte op);
void ldaDEA(GbState* state, byte op);
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
void ldaN16SP(GbState* state, byte op);
void pushR16(GbState* state, byte op);
void popR16(GbState* state, byte op);

void addAN8(GbState* state, byte op);
void addAR8(GbState* state, byte op);
void addAaHL(GbState* state, byte op);

void adcAN8(GbState* state, byte op);
void adcAR8(GbState* state, byte op);
void adcAaHL(GbState* state, byte op);

void subAN8(GbState* state, byte op);
void subAR8(GbState* state, byte op);
void subAaHL(GbState* state, byte op);

void sbcAN8(GbState* state, byte op);
void sbcAR8(GbState* state, byte op);
void sbcAaHL(GbState* state, byte op);

void andAN8(GbState* state, byte op);
void andAR8(GbState* state, byte op);
void andAaHL(GbState* state, byte op);

void orAN8(GbState* state, byte op);
void orAR8(GbState* state, byte op);
void orAaHL(GbState* state, byte op);

void xorAN8(GbState* state, byte op);
void xorAR8(GbState* state, byte op);
void xorAaHL(GbState* state, byte op);

void cpAN8(GbState* state, byte op);
void cpAR8(GbState* state, byte op);
void cpAaHL(GbState* state, byte op);

void incR8(GbState* state, byte op);
void incaHL(GbState* state, byte op);

void decR8(GbState* state, byte op);
void decaHL(GbState* state, byte op);

void incR16(GbState* state, byte op);
void decR16(GbState* state, byte op);
void addHLR16(GbState* state, byte op);
void addSPN8(GbState* state, byte op);
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
void jpHL(GbState* state, byte op);
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

typedef void (*gbz80Operation)(GbState*, byte);
static gbz80Operation opTable[] = {
//        0         1           2           3           4           5           6           7           8           9           A           B           C           D           E           F   
/*   0 */ nop,      ldR16N16,   ldaBCA,     incR16,     incR8,      decR8,      ldR8N8,     rlcA,       ldaN16SP,   addHLR16,   ldAaBC,     decR16,     incR8,      decR8,      ldR8N8,     rrcA,
/*   1 */ stop,     ldR16N16,   ldaDEA,     incR16,     incR8,      decR8,      ldR8N8,     rlA,        jrN8,       addHLR16,   ldAaDE,     decR16,     incR8,      decR8,      ldR8N8,     rrA,
/*   2 */ jrCCN8,   ldR16N16,   ldiaHLA,    incR16,     incR8,      decR8,      ldR8N8,     daa,        jrCCN8,     addHLR16,   ldiAaHL,    decR16,     incR8,      decR8,      ldR8N8,     cpl,
/*   3 */ jrCCN8,   ldR16N16,   lddaHLA,    incR16,     incR8,      decR8,      ldaHLN8,    scf,        jrCCN8,     addHLR16,   lddAaHL,    decR16,     incR8,      decR8,      ldR8N8,     ccf,
/*   4 */ ldR8R8,   ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8aHL,    ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8aHL,    ldR8R8,
/*   5 */ ldR8R8,   ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8aHL,    ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8aHL,    ldR8R8,
/*   6 */ ldR8R8,   ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8aHL,    ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8aHL,    ldR8R8,
/*   7 */ ldaHLR8,  ldaHLR8,    ldaHLR8,    ldaHLR8,    ldaHLR8,    ldaHLR8,    halt,       ldaHLR8,    ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8aHL,    ldR8R8,
/*   8 */ addAR8,   addAR8,     addAR8,     addAR8,     addAR8,     addAR8,     addAR8,     addAR8,     adcAR8,     adcAR8,     adcAR8,     adcAR8,     adcAR8,     adcAR8,     adcAR8,     adcAR8,
/*   9 */ subAR8,   subAR8,     subAR8,     subAR8,     subAR8,     subAR8,     subAR8,     subAR8,     sbcAR8,     sbcAR8,     sbcAR8,     sbcAR8,     sbcAR8,     sbcAR8,     sbcAR8,     sbcAR8,
/*   A */ andAR8,   andAR8,     andAR8,     andAR8,     andAR8,     andAR8,     andAR8,     andAR8,     xorAR8,     xorAR8,     xorAR8,     xorAR8,     xorAR8,     xorAR8,     xorAR8,     xorAR8,
/*   B */ orAR8,    orAR8,      orAR8,      orAR8,      orAR8,      orAR8,      orAR8,      orAR8,      cpAR8,      cpAR8,      cpAR8,      cpAR8,      cpAR8,      cpAR8,      cpAR8,      cpAR8,
/*   C */ retCC,    popR16,     jpCCN16,    jpN16,      callCCN16,  pushR16,    addAN8,     rstVec,     retCC,      ret,        jpCCN16,    ext,        callCCN16,  callN16,    adcAN8,     rstVec,
/*   D */ retCC,    popR16,     jpCCN16,    undefined,  callCCN16,  pushR16,    subAN8,     rstVec,     retCC,      reti,       jpCCN16,    undefined,  callCCN16,  undefined,  sbcAN8,     rstVec,
/*   E */ ldhaN8A,  popR16,     ldaCA,      undefined,  undefined,  pushR16,    andAN8,     rstVec,     addSPN8,    jpHL,       ldaN16A,    undefined,  undefined,  undefined,  xorAN8,     rstVec,
/*   F */ ldhAaN8,  popR16,     ldAaC,      di,         undefined,  pushR16,    orAN8 ,     rstVec,     ldHLSPN8,   ldSPHL,     ldAaN16,    ei,         undefined,  undefined,  cpAN8,      rstVec 
};

static gbz80Operation extendedOpTable[] = {
//          0       1       2       3       4       5       6       7       8       9       A       B       C       D       E       F   
/*   0 */   rlcR8,  rlcR8,  rlcR8,  rlcR8,  rlcR8,  rlcR8,  rlcR8,  rlcR8,  rrcR8,  rrcR8,  rrcR8,  rrcR8,  rrcR8,  rrcR8,  rrcR8,  rrcR8,  
/*   1 */   rlR8,   rlR8,   rlR8,   rlR8,   rlR8,   rlR8,   rlR8,   rlR8,   rrR8,   rrR8,   rrR8,   rrR8,   rrR8,   rrR8,   rrR8,   rrR8,     
/*   2 */   slaR8,  slaR8,  slaR8,  slaR8,  slaR8,  slaR8,  slaR8,  slaR8,  sraR8,  sraR8,  sraR8,  sraR8,  sraR8,  sraR8,  sraR8,  sraR8,
/*   3 */   swapR8, swapR8, swapR8, swapR8, swapR8, swapR8, swapR8, swapR8, srlR8,  srlR8,  srlR8,  srlR8,  srlR8,  srlR8,  srlR8,  srlR8,
/*   4 */   bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,
/*   5 */   bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,
/*   6 */   bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,
/*   7 */   bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,
/*   8 */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,
/*   9 */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,
/*   A */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,
/*   B */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,
/*   C */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,
/*   D */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,
/*   E */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,
/*   F */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8
};

#endif