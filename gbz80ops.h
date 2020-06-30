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

void ldAN8(GbState* state, u32 instruction);
void ldBN8(GbState* state, u32 instruction);
void ldDN8(GbState* state, u32 instruction);
void ldHN8(GbState* state, u32 instruction);
void ldCN8(GbState* state, u32 instruction);
void ldEN8(GbState* state, u32 instruction);
void ldLN8(GbState* state, u32 instruction);
void ldaHLN8(GbState* state, u32 instruction);

void ldAB(GbState* state, u32 instruction);
void ldAC(GbState* state, u32 instruction);
void ldAD(GbState* state, u32 instruction);
void ldAE(GbState* state, u32 instruction);
void ldAH(GbState* state, u32 instruction);
void ldAL(GbState* state, u32 instruction);

void ldBA(GbState* state, u32 instruction);
void ldBC(GbState* state, u32 instruction);
void ldBD(GbState* state, u32 instruction);
void ldBE(GbState* state, u32 instruction);
void ldBH(GbState* state, u32 instruction);
void ldBL(GbState* state, u32 instruction);

void ldCA(GbState* state, u32 instruction);
void ldCB(GbState* state, u32 instruction);
void ldCD(GbState* state, u32 instruction);
void ldCE(GbState* state, u32 instruction);
void ldCH(GbState* state, u32 instruction);
void ldCL(GbState* state, u32 instruction);

void ldDA(GbState* state, u32 instruction);
void ldDB(GbState* state, u32 instruction);
void ldDC(GbState* state, u32 instruction);
void ldDE(GbState* state, u32 instruction);
void ldDH(GbState* state, u32 instruction);
void ldDL(GbState* state, u32 instruction);

void ldEA(GbState* state, u32 instruction);
void ldEB(GbState* state, u32 instruction);
void ldEC(GbState* state, u32 instruction);
void ldED(GbState* state, u32 instruction);
void ldEH(GbState* state, u32 instruction);
void ldEL(GbState* state, u32 instruction);

void ldHA(GbState* state, u32 instruction);
void ldHB(GbState* state, u32 instruction);
void ldHC(GbState* state, u32 instruction);
void ldHD(GbState* state, u32 instruction);
void ldHE(GbState* state, u32 instruction);
void ldHL(GbState* state, u32 instruction);

void ldLA(GbState* state, u32 instruction);
void ldLB(GbState* state, u32 instruction);
void ldLC(GbState* state, u32 instruction);
void ldLD(GbState* state, u32 instruction);
void ldLE(GbState* state, u32 instruction);
void ldLH(GbState* state, u32 instruction);

void ldaHLA(GbState* state, u32 instruction);
void ldaHLB(GbState* state, u32 instruction);
void ldaHLC(GbState* state, u32 instruction);
void ldaHLD(GbState* state, u32 instruction);
void ldaHLE(GbState* state, u32 instruction);
void ldaHLH(GbState* state, u32 instruction);
void ldaHLL(GbState* state, u32 instruction);

void ldAaHL(GbState* state, u32 instruction);
void ldBaHL(GbState* state, u32 instruction);
void ldCaHL(GbState* state, u32 instruction);
void ldDaHL(GbState* state, u32 instruction);
void ldEaHL(GbState* state, u32 instruction);
void ldHaHL(GbState* state, u32 instruction);
void ldLaHL(GbState* state, u32 instruction);

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
void ldBCN16(GbState* state, u32 instruction);
void ldDEN16(GbState* state, u32 instruction);
void ldHLN16(GbState* state, u32 instruction);
void ldSPN16(GbState* state, u32 instruction);
void ldSPHL(GbState* state, u32 instruction);
void ldHLSPN8(GbState* state, u32 instruction);
void ldaN16SP(GbState* state, u32 instruction);

void pushBC(GbState* state, u32 instruction);
void pushDE(GbState* state, u32 instruction);
void pushHL(GbState* state, u32 instruction);
void pushAF(GbState* state, u32 instruction);

void popBC(GbState* state, u32 instruction);
void popDE(GbState* state, u32 instruction);
void popHL(GbState* state, u32 instruction);
void popAF(GbState* state, u32 instruction);

void addAN8(GbState* state, u32 instruction);
void addAA(GbState* state, u32 instruction);
void addAB(GbState* state, u32 instruction);
void addAC(GbState* state, u32 instruction);
void addAD(GbState* state, u32 instruction);
void addAE(GbState* state, u32 instruction);
void addAH(GbState* state, u32 instruction);
void addAL(GbState* state, u32 instruction);
void addAaHL(GbState* state, u32 instruction);

void adcAN8(GbState* state, u32 instruction);
void adcAA(GbState* state, u32 instruction);
void adcAB(GbState* state, u32 instruction);
void adcAC(GbState* state, u32 instruction);
void adcAD(GbState* state, u32 instruction);
void adcAE(GbState* state, u32 instruction);
void adcAH(GbState* state, u32 instruction);
void adcAL(GbState* state, u32 instruction);
void adcAaHL(GbState* state, u32 instruction);

void subAN8(GbState* state, u32 instruction);
void subAA(GbState* state, u32 instruction);
void subAB(GbState* state, u32 instruction);
void subAC(GbState* state, u32 instruction);
void subAD(GbState* state, u32 instruction);
void subAE(GbState* state, u32 instruction);
void subAH(GbState* state, u32 instruction);
void subAL(GbState* state, u32 instruction);
void subAaHL(GbState* state, u32 instruction);

void sbcAN8(GbState* state, u32 instruction);
void sbcAA(GbState* state, u32 instruction);
void sbcAB(GbState* state, u32 instruction);
void sbcAC(GbState* state, u32 instruction);
void sbcAD(GbState* state, u32 instruction);
void sbcAE(GbState* state, u32 instruction);
void sbcAH(GbState* state, u32 instruction);
void sbcAL(GbState* state, u32 instruction);
void sbcAaHL(GbState* state, u32 instruction);

void andAN8(GbState* state, u32 instruction);
void andAA(GbState* state, u32 instruction);
void andAB(GbState* state, u32 instruction);
void andAC(GbState* state, u32 instruction);
void andAD(GbState* state, u32 instruction);
void andAE(GbState* state, u32 instruction);
void andAH(GbState* state, u32 instruction);
void andAL(GbState* state, u32 instruction);
void andAaHL(GbState* state, u32 instruction);

void orAN8(GbState* state, u32 instruction);
void orAA(GbState* state, u32 instruction);
void orAB(GbState* state, u32 instruction);
void orAC(GbState* state, u32 instruction);
void orAD(GbState* state, u32 instruction);
void orAE(GbState* state, u32 instruction);
void orAH(GbState* state, u32 instruction);
void orAL(GbState* state, u32 instruction);
void orAaHL(GbState* state, u32 instruction);

void xorAN8(GbState* state, u32 instruction);
void xorAR8(GbState* state, u32 instruction);
void xorAaHL(GbState* state, u32 instruction);

void cpAN8(GbState* state, u32 instruction);
void cpAA(GbState* state, u32 instruction);
void cpAB(GbState* state, u32 instruction);
void cpAC(GbState* state, u32 instruction);
void cpAD(GbState* state, u32 instruction);
void cpAE(GbState* state, u32 instruction);
void cpAH(GbState* state, u32 instruction);
void cpAL(GbState* state, u32 instruction);
void cpAaHL(GbState* state, u32 instruction);

void incA(GbState* state, u32 instruction);
void incB(GbState* state, u32 instruction);
void incD(GbState* state, u32 instruction);
void incH(GbState* state, u32 instruction);
void incC(GbState* state, u32 instruction);
void incE(GbState* state, u32 instruction);
void incL(GbState* state, u32 instruction);
void incaHL(GbState* state, u32 instruction);

void decA(GbState* state, u32 instruction);
void decB(GbState* state, u32 instruction);
void decD(GbState* state, u32 instruction);
void decH(GbState* state, u32 instruction);
void decC(GbState* state, u32 instruction);
void decE(GbState* state, u32 instruction);
void decL(GbState* state, u32 instruction);
void decaHL(GbState* state, u32 instruction);

void incBC(GbState* state, u32 instruction);
void incDE(GbState* state, u32 instruction);
void incHL(GbState* state, u32 instruction);
void incSP(GbState* state, u32 instruction);
void decBC(GbState* state, u32 instruction);
void decDE(GbState* state, u32 instruction);
void decHL(GbState* state, u32 instruction);
void decSP(GbState* state, u32 instruction);
void addHLBC(GbState* state, u32 instruction);
void addHLDE(GbState* state, u32 instruction);
void addHLHL(GbState* state, u32 instruction);
void addHLSP(GbState* state, u32 instruction);
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
void jrCCN8(GbState* state, u32 instruction);
void jrN8(GbState* state, u32 instruction);
void jrNZN8(GbState* state, u32 instruction);
void jrNCN8(GbState* state, u32 instruction);
void jrZN8(GbState* state, u32 instruction);
void jrCN8(GbState* state, u32 instruction);
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
//        0         1           2           3           4           5           6           7           8           9           A           B           C           D          E            F   
/*   0 */ nop,      ldBCN16,    ldaBCA,     incBC,      incB,       decB,       ldBN8,      rlcA,        ldaN16SP,   addHLBC,   ldAaBC,     decBC,      incC,      decC,       ldCN8,       rrcA,
/*   1 */ stop,     ldDEN16,    ldaDEA,     incDE,      incD,       decD,       ldDN8,      rlA,         jrN8,       addHLDE,   ldAaDE,     decDE,      incE,      decE,       ldEN8,       rrA,
/*   2 */ jrNZN8,   ldHLN16,    ldiaHLA,    incHL,      incH,       decH,       ldHN8,      daa,        jrZN8,      addHLHL,    ldiAaHL,    decHL,      incL,      decL,       ldLN8,       cpl,
/*   3 */ jrNCN8,   ldSPN16,    lddaHLA,    incSP,      incaHL,     decaHL,     ldaHLN8,    scf,        jrCN8,      addHLSP,    lddAaHL,    decSP,      incA,      decA,       ldAN8,       ccf,
/*   4 */ nop,      ldBC,       ldBD,       ldBE,       ldBH,       ldBL,       ldBaHL,     ldBA,       ldCB,       nop,        ldCD,       ldCE,       ldCH,      ldCL,       ldCaHL,      ldCA,
/*   5 */ ldDB,     ldDC,       nop,        ldDE,       ldDH,       ldDL,       ldDaHL,     ldDA,       ldEB,       ldEC,       ldED,       nop,        ldEH,      ldEL,       ldEaHL,      ldEA,
/*   6 */ ldHB,     ldHC,       ldHD,       ldHE,       nop,        ldHL,       ldHaHL,     ldHA,       ldLB,       ldLC,       ldLD,       ldLE,       ldLH,      nop,        ldLaHL,      ldLA,
/*   7 */ ldaHLB,   ldaHLC,     ldaHLD,     ldaHLE,     ldaHLH,     ldaHLL,     halt,       ldaHLA,     ldAB,       ldAC,       ldAD,       ldAE,       ldAH,      ldAL,       ldAaHL,      nop,
/*   8 */ addAB,    addAC,      addAD,      addAE,      addAH,      addAL,      addAaHL,    addAA,      adcAB,      adcAC,      adcAD,      adcAE,      adcAH,     adcAL,      adcAaHL,     adcAA,
/*   9 */ subAB,    subAC,      subAD,      subAE,      subAH,      subAL,      subAaHL,    subAA,      sbcAB,      sbcAC,      sbcAD,      sbcAE,      sbcAH,     sbcAL,      sbcAaHL,     sbcAA,
/*   A */ andAB,    andAC,      andAD,      andAE,      andAH,      andAL,      andAaHL,    andAA,      xorAR8,     xorAR8,     xorAR8,     xorAR8,     xorAR8,      xorAR8,     xorAaHL,   xorAR8,
/*   B */ orAB,     orAC,       orAD,       orAE,       orAH,       orAL,      orAaHL,      orAA,       cpAB,       cpAC,       cpAD,       cpAE,       cpAH,       cpAL,       cpAaHL,     cpAA,
/*   C */ retCC,    popBC,      jpCCN16,    jpN16,      callCCN16,  pushBC,    addAN8,      rstVec,     retCC,      ret,        jpCCN16,    ext,        callCCN16,  callN16,    adcAN8,     rstVec,
/*   D */ retCC,    popDE,      jpCCN16,    undefined,  callCCN16,  pushDE,    subAN8,      rstVec,     retCC,      reti,       jpCCN16,    undefined,  callCCN16,  undefined,  sbcAN8,     rstVec,
/*   E */ ldhaN8A,  popHL,      ldaCA,      undefined,  undefined,  pushHL,    andAN8,      rstVec,     addSPN8,    jpHL,       ldaN16A,    undefined,  undefined,  undefined,  xorAN8,     rstVec,
/*   F */ ldhAaN8,  popAF,      ldAaC,      di,         undefined,  pushAF,    orAN8 ,      rstVec,     ldHLSPN8,   ldSPHL,     ldAaN16,    ei,         undefined,  undefined,  cpAN8,      rstVec 
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