#ifndef GBZ80OPS_INCLUDED
#define GBZ80OPS_INCLUDED

#include "core.h"
#include "types.h"

void rlca(GbState* state, u32 instruction);
void rlcA(GbState* state, u32 instruction);
void rlcB(GbState* state, u32 instruction);
void rlcC(GbState* state, u32 instruction);
void rlcD(GbState* state, u32 instruction);
void rlcE(GbState* state, u32 instruction);
void rlcH(GbState* state, u32 instruction);
void rlcL(GbState* state, u32 instruction);
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

void srlA(GbState* state, u32 instruction);
void srlB(GbState* state, u32 instruction);
void srlC(GbState* state, u32 instruction);
void srlD(GbState* state, u32 instruction);
void srlE(GbState* state, u32 instruction);
void srlH(GbState* state, u32 instruction);
void srlL(GbState* state, u32 instruction);
void srlaHL(GbState* state, u32 instruction);

void bit0A(GbState* state, u32 instruction);
void bit1A(GbState* state, u32 instruction);
void bit2A(GbState* state, u32 instruction);
void bit3A(GbState* state, u32 instruction);
void bit4A(GbState* state, u32 instruction);
void bit5A(GbState* state, u32 instruction);
void bit6A(GbState* state, u32 instruction);
void bit7A(GbState* state, u32 instruction);

void bit0B(GbState* state, u32 instruction);
void bit1B(GbState* state, u32 instruction);
void bit2B(GbState* state, u32 instruction);
void bit3B(GbState* state, u32 instruction);
void bit4B(GbState* state, u32 instruction);
void bit5B(GbState* state, u32 instruction);
void bit6B(GbState* state, u32 instruction);
void bit7B(GbState* state, u32 instruction);

void bit0C(GbState* state, u32 instruction);
void bit1C(GbState* state, u32 instruction);
void bit2C(GbState* state, u32 instruction);
void bit3C(GbState* state, u32 instruction);
void bit4C(GbState* state, u32 instruction);
void bit5C(GbState* state, u32 instruction);
void bit6C(GbState* state, u32 instruction);
void bit7C(GbState* state, u32 instruction);

void bit0D(GbState* state, u32 instruction);
void bit1D(GbState* state, u32 instruction);
void bit2D(GbState* state, u32 instruction);
void bit3D(GbState* state, u32 instruction);
void bit4D(GbState* state, u32 instruction);
void bit5D(GbState* state, u32 instruction);
void bit6D(GbState* state, u32 instruction);
void bit7D(GbState* state, u32 instruction);

void bit0E(GbState* state, u32 instruction);
void bit1E(GbState* state, u32 instruction);
void bit2E(GbState* state, u32 instruction);
void bit3E(GbState* state, u32 instruction);
void bit4E(GbState* state, u32 instruction);
void bit5E(GbState* state, u32 instruction);
void bit6E(GbState* state, u32 instruction);
void bit7E(GbState* state, u32 instruction);

void bit0H(GbState* state, u32 instruction);
void bit1H(GbState* state, u32 instruction);
void bit2H(GbState* state, u32 instruction);
void bit3H(GbState* state, u32 instruction);
void bit4H(GbState* state, u32 instruction);
void bit5H(GbState* state, u32 instruction);
void bit6H(GbState* state, u32 instruction);
void bit7H(GbState* state, u32 instruction);

void bit0L(GbState* state, u32 instruction);
void bit1L(GbState* state, u32 instruction);
void bit2L(GbState* state, u32 instruction);
void bit3L(GbState* state, u32 instruction);
void bit4L(GbState* state, u32 instruction);
void bit5L(GbState* state, u32 instruction);
void bit6L(GbState* state, u32 instruction);
void bit7L(GbState* state, u32 instruction);

void bit0aHL(GbState* state, u32 instruction);
void bit1aHL(GbState* state, u32 instruction);
void bit2aHL(GbState* state, u32 instruction);
void bit3aHL(GbState* state, u32 instruction);
void bit4aHL(GbState* state, u32 instruction);
void bit5aHL(GbState* state, u32 instruction);
void bit6aHL(GbState* state, u32 instruction);
void bit7aHL(GbState* state, u32 instruction);

void bitU3R8(GbState* state, u32 instruction);

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
void xorAA(GbState* state, u32 instruction);
void xorAB(GbState* state, u32 instruction);
void xorAC(GbState* state, u32 instruction);
void xorAD(GbState* state, u32 instruction);
void xorAE(GbState* state, u32 instruction);
void xorAH(GbState* state, u32 instruction);
void xorAL(GbState* state, u32 instruction);
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
void addSPN8(GbState* state, u32 instruction);
void daa(GbState* state, u32 instruction);
void cpl(GbState* state, u32 instruction);
void ccf(GbState* state, u32 instruction);
void scf(GbState* state, u32 instruction);
void rlA(GbState* state, u32 instruction);
void rrcA(GbState* state, u32 instruction);
void rrA(GbState* state, u32 instruction);
void nop(GbState* state, u32 instruction);
void halt(GbState* state, u32 instruction);
void stop(GbState* state, u32 instruction);
void di(GbState* state, u32 instruction);
void ei(GbState* state, u32 instruction);
void jpN16(GbState* state, u32 instruction);
void jpNZN16(GbState* state, u32 instruction);
void jpNCN16(GbState* state, u32 instruction);
void jpZN16(GbState* state, u32 instruction);
void jpCN16(GbState* state, u32 instruction);
void jpHL(GbState* state, u32 instruction);
void jrN8(GbState* state, u32 instruction);
void jrNZN8(GbState* state, u32 instruction);
void jrNCN8(GbState* state, u32 instruction);
void jrZN8(GbState* state, u32 instruction);
void jrCN8(GbState* state, u32 instruction);
void callN16(GbState* state, u32 instruction);
void callNZN16(GbState* state, u32 instruction);
void callNCN16(GbState* state, u32 instruction);
void callZN16(GbState* state, u32 instruction);
void callCN16(GbState* state, u32 instruction);
void rst00(GbState* state, u32 instruction);
void rst10(GbState* state, u32 instruction);
void rst20(GbState* state, u32 instruction);
void rst30(GbState* state, u32 instruction);
void rst08(GbState* state, u32 instruction);
void rst18(GbState* state, u32 instruction);
void rst28(GbState* state, u32 instruction);
void rst38(GbState* state, u32 instruction);
void retNZ(GbState* state, u32 instruction);
void retNC(GbState* state, u32 instruction);
void retZ(GbState* state, u32 instruction);
void retC(GbState* state, u32 instruction);
void reti(GbState* state, u32 instruction);
void ret(GbState* state, u32 instruction);

// handler for undefined op codes.
void undefined(GbState* state, u32 instruction);

typedef void (*gbz80Operation)(GbState*, u32);
static gbz80Operation opTable[] = {
//        0         1           2           3           4           5           6           7           8           9           A           B           C           D           E       F   
/*   0 */ nop,      ldBCN16,    ldaBCA,     incBC,      incB,       decB,       ldBN8,      rlca,       ldaN16SP,   addHLBC,    ldAaBC,     decBC,      incC,       decC,       ldCN8,  rrcA,
/*   1 */ stop,     ldDEN16,    ldaDEA,     incDE,      incD,       decD,       ldDN8,      rlA,        jrN8,       addHLDE,    ldAaDE,     decDE,      incE,       decE,       ldEN8,  rrA,
/*   2 */ jrNZN8,   ldHLN16,    ldiaHLA,    incHL,      incH,       decH,       ldHN8,      daa,        jrZN8,      addHLHL,    ldiAaHL,    decHL,      incL,       decL,       ldLN8,  cpl,
/*   3 */ jrNCN8,   ldSPN16,    lddaHLA,    incSP,      incaHL,     decaHL,     ldaHLN8,    scf,        jrCN8,      addHLSP,    lddAaHL,    decSP,      incA,       decA,       ldAN8,  ccf,
/*   4 */ nop,      ldBC,       ldBD,       ldBE,       ldBH,       ldBL,       ldBaHL,     ldBA,       ldCB,       nop,        ldCD,       ldCE,       ldCH,       ldCL,       ldCaHL, ldCA,
/*   5 */ ldDB,     ldDC,       nop,        ldDE,       ldDH,       ldDL,       ldDaHL,     ldDA,       ldEB,       ldEC,       ldED,       nop,        ldEH,       ldEL,       ldEaHL, ldEA,
/*   6 */ ldHB,     ldHC,       ldHD,       ldHE,       nop,        ldHL,       ldHaHL,     ldHA,       ldLB,       ldLC,       ldLD,       ldLE,       ldLH,       nop,        ldLaHL, ldLA,
/*   7 */ ldaHLB,   ldaHLC,     ldaHLD,     ldaHLE,     ldaHLH,     ldaHLL,     halt,       ldaHLA,     ldAB,       ldAC,       ldAD,       ldAE,       ldAH,       ldAL,       ldAaHL, nop,
/*   8 */ addAB,    addAC,      addAD,      addAE,      addAH,      addAL,      addAaHL,    addAA,      adcAB,      adcAC,      adcAD,      adcAE,      adcAH,      adcAL,      adcAaHL,adcAA,
/*   9 */ subAB,    subAC,      subAD,      subAE,      subAH,      subAL,      subAaHL,    subAA,      sbcAB,      sbcAC,      sbcAD,      sbcAE,      sbcAH,      sbcAL,      sbcAaHL,sbcAA,
/*   A */ andAB,    andAC,      andAD,      andAE,      andAH,      andAL,      andAaHL,    andAA,      xorAB,      xorAC,      xorAD,      xorAE,      xorAH,      xorAL,      xorAaHL,xorAA,
/*   B */ orAB,     orAC,       orAD,       orAE,       orAH,       orAL,       orAaHL,     orAA,       cpAB,       cpAC,       cpAD,       cpAE,       cpAH,       cpAL,       cpAaHL, cpAA,
/*   C */ retNZ,    popBC,      jpNZN16,    jpN16,      callNZN16,  pushBC,     addAN8,     rst00,      retZ,       ret,        jpZN16,     ext,        callZN16,   callN16,    adcAN8, rst08,
/*   D */ retNC,    popDE,      jpNCN16,    undefined,  callNCN16,  pushDE,     subAN8,     rst10,      retC,       reti,       jpCN16,     undefined,  callCN16,   undefined,  sbcAN8, rst18,
/*   E */ ldhaN8A,  popHL,      ldaCA,      undefined,  undefined,  pushHL,     andAN8,     rst20,      addSPN8,    jpHL,       ldaN16A,    undefined,  undefined,  undefined,  xorAN8, rst28,
/*   F */ ldhAaN8,  popAF,      ldAaC,      di,         undefined,  pushAF,     orAN8 ,     rst30,      ldHLSPN8,   ldSPHL,     ldAaN16,    ei,         undefined,  undefined,  cpAN8,  rst38
};

static gbz80Operation extendedOpTable[] = {
//          0       1       2       3       4       5       6           7       8       9       A       B       C       D       E       F   
/*   0 */   rlcB,   rlcC,   rlcD,   rlcE,   rlcH,   rlcL,   rlcaHL,     rlcA,   rrcR8,  rrcR8,  rrcR8,  rrcR8,  rrcR8,  rrcR8,  rrcaHL, rrcR8,
/*   1 */   rlR8,   rlR8,   rlR8,   rlR8,   rlR8,   rlR8,   rlaHL,      rlR8,   rrR8,   rrR8,   rrR8,   rrR8,   rrR8,   rrR8,   rraHL,  rrR8,
/*   2 */   slaR8,  slaR8,  slaR8,  slaR8,  slaR8,  slaR8,  slaaHL,     slaR8,  sraR8,  sraR8,  sraR8,  sraR8,  sraR8,  sraR8,  sraaHL, sraR8,
/*   3 */   swapR8, swapR8, swapR8, swapR8, swapR8, swapR8, swapaHL,    swapR8, srlB,   srlC,   srlD,   srlE,   srlH,   srlL,   srlaHL, srlA,
/*   4 */   bit0B,  bit0C,  bit0D,  bit0E,  bit0H,  bit0L,  bit0aHL,    bit0A,  bit1B,  bit1C,  bit1D,  bit1E,  bit1H,  bit1L,  bit1aHL,bit1A,
/*   5 */   bit2B,  bit2C,  bit2D,  bit2E,  bit2H,  bit2L,  bit2aHL,    bit2A,  bit3B,  bit3C,  bit3D,  bit3E,  bit3H,  bit3L,  bit3aHL,bit3A,
/*   6 */   bit4B,  bit4C,  bit4D,  bit4E,  bit4H,  bit4L,  bit4aHL,    bit4A,  bit5B,  bit5C,  bit5D,  bit5E,  bit5H,  bit5L,  bit5aHL,bit5A,
/*   7 */   bit6B,  bit6C,  bit6D,  bit6E,  bit6H,  bit6L,  bit6aHL,    bit6A,  bit7B,  bit7C,  bit7D,  bit7E,  bit7H,  bit7L,  bit7aHL,bit7A,
/*   8 */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,
/*   9 */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,
/*   A */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,
/*   B */   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,   resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3R8,resU3aHL,resU3R8,
/*   C */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8,
/*   D */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8,
/*   E */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8,
/*   F */   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,   setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3R8,setU3aHL,setU3R8
};

#endif