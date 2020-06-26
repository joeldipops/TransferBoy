#ifndef GBZ80OPS_INCLUDED
#define GBZ80OPS_INCLUDED

#include "core.h"
#include "types.h"

void rlcR8(GbState* state, u32 instruction);
void rlcaHL(GbState* state, u32 instruction);

void rrcR8(GbState* state, u32 instruction);
void rrcaHL(GbState* state, u32 instruction);

void rlR8(GbState* state, u32 instruction);
void rlaHL(GbState* state, u32 instruction);

void rrR8(GbState* state, u32 instruction);
void rraHL(GbState* state, u32 instruction);

void slaR8(GbState* state, u32 instruction);
void slaaHL(GbState* state, u32 instruction);

void sraR8(GbState* state, u32 instruction);
void sraaHL(GbState* state, u32 instruction);

void swapR8(GbState* state, u32 instruction);
void swapaHL(GbState* state, u32 instruction);

void srlR8(GbState* state, u32 instruction);
void srlaHL(GbState* state, u32 instruction);

void bitU3R8(GbState* state, u32 instruction);
void bitU3aHL(GbState* state, u32 instruction);

void resU3R8(GbState* state, u32 instruction);
void resU3aHL(GbState* state, u32 instruction);

void setU3R8(GbState* state, u32 instruction);
void setU3aHL(GbState* state, u32 instruction);

void ext(GbState* state, u32 instruction);

void ldR8N8(GbState* state, u32 instruction);
void ldaHLN8(GbState* state, u32 instruction);

void ldR8R8(GbState* state, u32 instruction);
void ldAA(GbState* state, u32 instruction);
void ldAB(GbState* state, u32 instruction);
void ldAC(GbState* state, u32 instruction);
void ldAD(GbState* state, u32 instruction);
void ldAE(GbState* state, u32 instruction);
void ldAH(GbState* state, u32 instruction);
void ldAL(GbState* state, u32 instruction);

void ldBA(GbState* state, u32 instruction);
void ldBB(GbState* state, u32 instruction);
void ldBC(GbState* state, u32 instruction);
void ldBD(GbState* state, u32 instruction);
void ldBE(GbState* state, u32 instruction);
void ldBH(GbState* state, u32 instruction);
void ldBL(GbState* state, u32 instruction);

void ldCA(GbState* state, u32 instruction);
void ldCB(GbState* state, u32 instruction);
void ldCC(GbState* state, u32 instruction);
void ldCD(GbState* state, u32 instruction);
void ldCE(GbState* state, u32 instruction);
void ldCH(GbState* state, u32 instruction);
void ldCL(GbState* state, u32 instruction);

void ldDA(GbState* state, u32 instruction);
void ldDB(GbState* state, u32 instruction);
void ldDC(GbState* state, u32 instruction);
void ldDD(GbState* state, u32 instruction);
void ldDE(GbState* state, u32 instruction);
void ldDH(GbState* state, u32 instruction);
void ldDL(GbState* state, u32 instruction);

void ldaHLR8(GbState* state, u32 instruction);
void ldR8aHL(GbState* state, u32 instruction);

void ldAaBC(GbState* state, u32 instruction);
void ldAaDE(GbState* state, u32 instruction);
void ldAaN16(GbState* state, u32 instruction);
void ldaBCA(GbState* state, u32 instruction);
void ldaDEA(GbState* state, u32 instruction);
void ldaN16A(GbState* state, u32 instruction);
void ldAaC(GbState* state, u32 instruction);
void ldaCA(GbState* state, u32 instruction);
void lddAaHL(GbState* state, u32 instruction);
void lddaHLA(GbState* state, u32 instruction);
void ldiaHLA(GbState* state, u32 instruction);
void ldiAaHL(GbState* state, u32 instruction);
void ldhAaN8(GbState* state, u32 instruction);
void ldhaN8A(GbState* state, u32 instruction);
void ldR16N16(GbState* state, u32 instruction);
void ldSPHL(GbState* state, u32 instruction);
void ldHLSPN8(GbState* state, u32 instruction);
void ldaN16SP(GbState* state, u32 instruction);
void pushR16(GbState* state, u32 instruction);
void popR16(GbState* state, u32 instruction);

void addAN8(GbState* state, u32 instruction);
void addAR8(GbState* state, u32 instruction);
void addAaHL(GbState* state, u32 instruction);

void adcAN8(GbState* state, u32 instruction);
void adcAR8(GbState* state, u32 instruction);
void adcAaHL(GbState* state, u32 instruction);

void subAN8(GbState* state, u32 instruction);
void subAR8(GbState* state, u32 instruction);
void subAaHL(GbState* state, u32 instruction);

void sbcAN8(GbState* state, u32 instruction);
void sbcAR8(GbState* state, u32 instruction);
void sbcAaHL(GbState* state, u32 instruction);

void andAN8(GbState* state, u32 instruction);
void andAR8(GbState* state, u32 instruction);
void andAaHL(GbState* state, u32 instruction);

void orAN8(GbState* state, u32 instruction);
void orAR8(GbState* state, u32 instruction);
void orAaHL(GbState* state, u32 instruction);

void xorAN8(GbState* state, u32 instruction);
void xorAR8(GbState* state, u32 instruction);
void xorAaHL(GbState* state, u32 instruction);

void cpAN8(GbState* state, u32 instruction);
void cpAR8(GbState* state, u32 instruction);
void cpAaHL(GbState* state, u32 instruction);

void incA(GbState* state, u32 instruction);
void incR8(GbState* state, u32 instruction);
void incaHL(GbState* state, u32 instruction);

void decR8(GbState* state, u32 instruction);
void decaHL(GbState* state, u32 instruction);

void incR16(GbState* state, u32 instruction);
void decR16(GbState* state, u32 instruction);
void addHLR16(GbState* state, u32 instruction);
void addSPN8(GbState* state, u32 instruction);
void daa(GbState* state, u32 instruction);
void cpl(GbState* state, u32 instruction);
void ccf(GbState* state, u32 instruction);
void scf(GbState* state, u32 instruction);
void rlcA(GbState* state, u32 instruction);
void rlA(GbState* state, u32 instruction);
void rrcA(GbState* state, u32 instruction);
void rrA(GbState* state, u32 instruction);
void nop(GbState* state, u32 instruction);
void halt(GbState* state, u32 instruction);
void stop(GbState* state, u32 instruction);
void di(GbState* state, u32 instruction);
void ei(GbState* state, u32 instruction);
void jpN16(GbState* state, u32 instruction);
void jpCCN16(GbState* state, u32 instruction);
void jpHL(GbState* state, u32 instruction);
void jrN8(GbState* state, u32 instruction);
void jrCCN8(GbState* state, u32 instruction);
void callN16(GbState* state, u32 instruction);
void callCCN16(GbState* state, u32 instruction);
void rstVec(GbState* state, u32 instruction);
void retCC(GbState* state, u32 instruction);
void reti(GbState* state, u32 instruction);
void ret(GbState* state, u32 instruction);

// handler for undefined op codes.
void undefined(GbState* state, u32 instruction);

typedef void (*gbz80Operation)(GbState*, u32);
static gbz80Operation opTable[] = {
//        0         1           2           3           4           5           6           7           8           9           A           B           C           D           E           F   
/*   0 */ nop,      ldR16N16,   ldaBCA,     incR16,     incR8,      decR8,      ldR8N8,     rlcA,       ldaN16SP,   addHLR16,   ldAaBC,     decR16,     incR8,      decR8,      ldR8N8,     rrcA,
/*   1 */ stop,     ldR16N16,   ldaDEA,     incR16,     incR8,      decR8,      ldR8N8,     rlA,        jrN8,       addHLR16,   ldAaDE,     decR16,     incR8,      decR8,      ldR8N8,     rrA,
/*   2 */ jrCCN8,   ldR16N16,   ldiaHLA,    incR16,     incR8,      decR8,      ldR8N8,     daa,        jrCCN8,     addHLR16,   ldiAaHL,    decR16,     incR8,      decR8,      ldR8N8,     cpl,
/*   3 */ jrCCN8,   ldR16N16,   lddaHLA,    incR16,     incaHL,     decaHL,     ldaHLN8,    scf,        jrCCN8,     addHLR16,   lddAaHL,    decR16,     incA,      decR8,      ldR8N8,     ccf,
/*   4 */ ldBB,     ldBC,       ldBD,       ldBE,       ldBH,       ldBL,       ldR8aHL,    ldBA,       ldCB,       ldCC,       ldCD,       ldCE,       ldCH,       ldCL,     ldR8aHL,    ldCA,
/*   5 */ ldDB,     ldDC,       ldDD,       ldDE,       ldDH,       ldDL,       ldR8aHL,    ldDA,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8aHL,    ldR8R8,
/*   6 */ ldR8R8,   ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8aHL,    ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8R8,     ldR8aHL,    ldR8R8,
/*   7 */ ldaHLR8,  ldaHLR8,    ldaHLR8,    ldaHLR8,    ldaHLR8,    ldaHLR8,    halt,       ldaHLR8,    ldAB,       ldAC,       ldAD,       ldAE,       ldAH,       ldAL,       ldR8aHL,    ldAA,
/*   8 */ addAR8,   addAR8,     addAR8,     addAR8,     addAR8,     addAR8,     addAaHL,    addAR8,     adcAR8,     adcAR8,     adcAR8,     adcAR8,     adcAR8,     adcAR8,     adcAaHL,    adcAR8,
/*   9 */ subAR8,   subAR8,     subAR8,     subAR8,     subAR8,     subAR8,     subAaHL,    subAR8,     sbcAR8,     sbcAR8,     sbcAR8,     sbcAR8,     sbcAR8,     sbcAR8,     sbcAaHL,    sbcAR8,
/*   A */ andAR8,   andAR8,     andAR8,     andAR8,     andAR8,     andAR8,     andAaHL,    andAR8,     xorAR8,     xorAR8,     xorAR8,     xorAR8,     xorAR8,     xorAR8,     xorAaHL,    xorAR8,
/*   B */ orAR8,    orAR8,      orAR8,      orAR8,      orAR8,      orAR8,      orAaHL,     orAR8,      cpAR8,      cpAR8,      cpAR8,      cpAR8,      cpAR8,      cpAR8,      cpAaHL,     cpAR8,
/*   C */ retCC,    popR16,     jpCCN16,    jpN16,      callCCN16,  pushR16,    addAN8,     rstVec,     retCC,      ret,        jpCCN16,    ext,        callCCN16,  callN16,    adcAN8,     rstVec,
/*   D */ retCC,    popR16,     jpCCN16,    undefined,  callCCN16,  pushR16,    subAN8,     rstVec,     retCC,      reti,       jpCCN16,    undefined,  callCCN16,  undefined,  sbcAN8,     rstVec,
/*   E */ ldhaN8A,  popR16,     ldaCA,      undefined,  undefined,  pushR16,    andAN8,     rstVec,     addSPN8,    jpHL,       ldaN16A,    undefined,  undefined,  undefined,  xorAN8,     rstVec,
/*   F */ ldhAaN8,  popR16,     ldAaC,      di,         undefined,  pushR16,    orAN8 ,     rstVec,     ldHLSPN8,   ldSPHL,     ldAaN16,    ei,         undefined,  undefined,  cpAN8,      rstVec 
};

static gbz80Operation extendedOpTable[] = {
//          0       1       2       3       4       5       6       7       8       9       A       B       C       D       E       F   
/*   0 */   rlcR8,  rlcR8,  rlcR8,  rlcR8,  rlcR8,  rlcR8,  rlcaHL, rlcR8,  rrcR8,  rrcR8,  rrcR8,  rrcR8,  rrcR8,  rrcR8,  rrcaHL,  rrcR8,  
/*   1 */   rlR8,   rlR8,   rlR8,   rlR8,   rlR8,   rlR8,   rlaHL,  rlR8,   rrR8,   rrR8,   rrR8,   rrR8,   rrR8,   rrR8,   rraHL,   rrR8,     
/*   2 */   slaR8,  slaR8,  slaR8,  slaR8,  slaR8,  slaR8,  slaaHL, slaR8,  sraR8,  sraR8,  sraR8,  sraR8,  sraR8,  sraR8,  sraaHL,  sraR8,
/*   3 */   swapR8, swapR8, swapR8, swapR8, swapR8, swapR8, swapaHL,swapR8, srlR8,  srlR8,  srlR8,  srlR8,  srlR8,  srlR8,  srlaHL,  srlR8,
/*   4 */   bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3aHL,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3aHL,bitU3R8,
/*   5 */   bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3aHL,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3aHL,bitU3R8,
/*   6 */   bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3aHL,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3aHL,bitU3R8,
/*   7 */   bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3aHL,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3R8,bitU3aHL,bitU3R8,
/*   8 */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,
/*   9 */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,
/*   A */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,
/*   B */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,
/*   C */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8,
/*   D */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8,
/*   E */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8,
/*   F */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8
};

#endif