#include "core.h"
#include "types.h"
#include "mmu.h"
#include "cpu.h"
#include "gbz80ops.h"
#include "logger.h"
#include "state.h"

#define CF s->flags.CF
#define HF s->flags.HF
#define NF s->flags.NF
#define ZF s->flags.ZF
#define A s->reg8.A
#define F s->reg8.F
#define B s->reg8.B
#define C s->reg8.C
#define D s->reg8.D
#define E s->reg8.E
#define H s->reg8.H
#define L s->reg8.L
#define AF s->reg16.AF
#define BC s->reg16.BC
#define DE s->reg16.DE
#define HL s->reg16.HL
#define op (instruction & 0xFF)
#define M(op, value, mask) (((op) & (mask)) == (value))
#define mem(loc) (mmu_read(s, loc))
#define IMM8  ((instruction >> 8) & 0xFF)
#define IMM16 ((instruction >> 8) & 0xFFFF)
#define REG8(bitpos) s->emu_cpu_state->reg8_lut[(op >> bitpos) & 7]
#define REG16(bitpos) s->emu_cpu_state->reg16_lut[((op >> bitpos) & 3)]
#define REG16S(bitpos) s->emu_cpu_state->reg16s_lut[((op >> bitpos) & 3)]
#define FLAG(bitpos) ((op >> bitpos) & 3)

static const u8 flagmasks[] = { FLAG_Z, FLAG_Z, FLAG_C, FLAG_C };

void rlcR8(GbState* s, u32 instruction) { // debug(s, "rlcR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    u8 res = (val << 1) | (val >> 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val >> 7;
    *reg = res; 
}
void rlcaHL(GbState* s, u32 instruction) { // debug(s, "rlcaHL");
    u8 val = mem(HL);
    u8 res = (val << 1) | (val >> 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val >> 7;
    mmu_write(s, HL, res);
}

void rrcR8(GbState* s, u32 instruction) { // debug(s, "rrcR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    u8 res = (val >> 1) | ((val & 1) << 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val & 1;
    *reg = res; 
}
void rrcaHL(GbState* s, u32 instruction) { // debug(s, "rrcaHL");
    u8 val = mem(HL);
    u8 res = (val >> 1) | ((val & 1) << 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val & 1;
    mmu_write(s, HL, res);
}

void rlR8(GbState* s, u32 instruction) { // debug(s, "rlR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    u8 res = (val << 1) | (CF ? 1 : 0);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val >> 7;
    *reg = res;
}
void rlaHL(GbState* s, u32 instruction) { // debug(s, "rlaHL");
    u8 val = mem(HL);
    u8 res = (val << 1) | (CF ? 1 : 0);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val >> 7;
    mmu_write(s, HL, res);
}

void rrR8(GbState* s, u32 instruction) { // debug(s, "rrR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    u8 res = (val >> 1) | (CF << 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val & 0x1;
    *reg = res;
}
void rraHL(GbState* s, u32 instruction) { // debug(s, "rraHL");
    u8 val = mem(HL);
    u8 res = (val >> 1) | (CF << 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val & 0x1;
    mmu_write(s, HL, res);
}

void slaR8(GbState* s, u32 instruction) { // debug(s, "slaR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    CF = val >> 7;
    val = val << 1;
    ZF = val == 0;
    NF = 0;
    HF = 0;
    *reg = val;
}
void slaaHL(GbState* s, u32 instruction) { // debug(s, "slaaHL");
    u8 val = mem(HL);
    CF = val >> 7;
    val = val << 1;
    ZF = val == 0;
    NF = 0;
    HF = 0;
    mmu_write(s, HL, val);
}

void sraR8(GbState* s, u32 instruction) { // debug(s, "sraR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    CF = val & 0x1;
    val = (val >> 1) | (val & (1<<7));
    ZF = val == 0;
    NF = 0;
    HF = 0;
    *reg = val;
}
void sraaHL(GbState* s, u32 instruction) { // debug(s, "sraaHL");
    u8 val = mem(HL);
    CF = val & 0x1;
    val = (val >> 1) | (val & (1<<7));
    ZF = val == 0;
    NF = 0;
    HF = 0;
    mmu_write(s, HL, val);
}

void swapR8(GbState* s, u32 instruction) { // debug(s, "swapR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    u8 res = ((val << 4) & 0xf0) | ((val >> 4) & 0xf);
    F = res == 0 ? FLAG_Z : 0;
    *reg = res;
}
void swapaHL(GbState* s, u32 instruction) { // debug(s, "swapaHL");
    u8 val = mem(HL);
    u8 res = ((val << 4) & 0xf0) | ((val >> 4) & 0xf);
    F = res == 0 ? FLAG_Z : 0;
    mmu_write(s, HL, res);
}

void srlR8(GbState* s, u32 instruction) { // debug(s, "srlR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    CF = val & 0x1;
    val = val >> 1;
    ZF = val == 0;
    NF = 0;
    HF = 0;
    *reg = val;
}
void srlaHL(GbState* s, u32 instruction) { // debug(s, "srlaHL");
    u8 val = mem(HL);
    CF = val & 0x1;
    val = val >> 1;
    ZF = val == 0;
    NF = 0;
    HF = 0;
    mmu_write(s, HL, val);
}

void bitU3R8(GbState* s, u32 instruction) { // debug(s, "bitU3R8");
    u8 bit = (op >> 3) & 7;
    u8 val = *REG8(0);
    ZF = ((val >> bit) & 1) == 0;
    NF = 0;
    HF = 1;
}
void bitU3aHL(GbState* s, u32 instruction) { // debug(s, "bitU3aHL");
    u8 bit = (op >> 3) & 7;
    u8 val = mem(HL);
    ZF = ((val >> bit) & 1) == 0;
    NF = 0;
    HF = 1; 
}

void resU3R8(GbState* s, u32 instruction) { // debug(s, "resU3R8");
    u8 bit = (op >> 3) & 7;
    u8 *reg = REG8(0);
    u8 val = *reg;
    val = val & ~(1<<bit);
    *reg = val;
}
void resU3aHL(GbState* s, u32 instruction) { // debug(s, "resU3aHL");
    u8 bit = (op >> 3) & 7;
    u8 val = mem(HL);
    val = val & ~(1<<bit);
    mmu_write(s, HL, val);
}

void setU3R8(GbState* s, u32 instruction) { // debug(s, "setU3R8");
    u8 bit = (op >> 3) & 7;
    u8 *reg = REG8(0);
    u8 val = *reg;
    val |= (1 << bit);
    *reg = val;
}
void setU3aHL(GbState* s, u32 instruction) { // debug(s, "setU3aHL");
    u8 bit = (op >> 3) & 7;
    u8 val = mem(HL);
    val |= (1 << bit);
    mmu_write(s, HL, val);
}

void ext(GbState* s, u32 instruction) { // debug(s, "ext");
    PC++;
    u8 extInstruction = instruction >> 8;
    byte extOp = extInstruction & 0xFF;
    extendedOpTable[extOp](s, extInstruction);
}

void ldAN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    A = IMM8;
    PC++;
}

void ldBN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    B = IMM8;
    PC++;
}

void ldDN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    D = IMM8;
    PC++;
}

void ldHN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    H = IMM8;
    PC++;
}

void ldCN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    C = IMM8;
    PC++;
}

void ldEN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    E = IMM8;
    PC++;
}

void ldLN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    L = IMM8;
    PC++;
}

void ldaHLN8(GbState* s, u32 instruction) { // debug(s, "ldaHLN8");
    u8 src = IMM8;
    PC++;
    mmu_write(s, HL, src);
}

/**
 * ld E, R8s
 */
void ldEB(GbState* s, u32 instruction) {
    E = B;
}
void ldEC(GbState* s, u32 instruction) {
    E = C;
}
void ldED(GbState* s, u32 instruction) {
    E = D;
}
/*
void ldEB(GbState* s, u32 instruction) {
    E = E;
}
*/
void ldEH(GbState* s, u32 instruction) {
    E = H;
}
void ldEL(GbState* s, u32 instruction) {
    E = L;
}
void ldEA(GbState* s, u32 instruction) {
    E = A;
}

/**
 * ld H, R8s
 */
void ldHB(GbState* s, u32 instruction) {
    H = B;
}
void ldHC(GbState* s, u32 instruction) {
    H = C;
}
void ldHD(GbState* s, u32 instruction) {
    H = D;
}
void ldHE(GbState* s, u32 instruction) {
    H = E;
}
/*
void ldHH(GbState* s, u32 instruction) {
    H = H;
}
*/
void ldHL(GbState* s, u32 instruction) {
    H = L;
}
void ldHA(GbState* s, u32 instruction) {
    H = A;
}

/**
 * ld L, R8s
 */
void ldLB(GbState* s, u32 instruction) {
    L = B;
}
void ldLC(GbState* s, u32 instruction) {
    L = C;
}
void ldLD(GbState* s, u32 instruction) {
    L = D;
}
void ldLE(GbState* s, u32 instruction) {
    L = E;
}
void ldLH(GbState* s, u32 instruction) {
    L = H;
}
/*
void ldLL(GbState* s, u32 instruction) {
    L = L;
}
*/
void ldLA(GbState* s, u32 instruction) {
    L = A;
}


/** 
 * ld B, R8s
 */
void ldBA(GbState* s, u32 instruction) { // debug(s, "ldAB");
    B = A;
}
/*
void ldBB(GbState* s, u32 instruction) { // debug(s, "ldAB");
    ;
}
*/
void ldBC(GbState* s, u32 instruction) { // debug(s, "ldAC");
    B = C;
}
void ldBD(GbState* s, u32 instruction) { // debug(s, "ldAD");
    B = D;
}
void ldBE(GbState* s, u32 instruction) { // debug(s, "ldAE");
    B = E;
}
void ldBH(GbState* s, u32 instruction) { // debug(s, "ldAH");
    B = H;
}
void ldBL(GbState* s, u32 instruction) { // debug(s, "ldAL");
    B = L;
}

/**
 * ld C, R8s
 */
void ldCA(GbState* s, u32 instruction) { // debug(s, "ldCA");
    C = A;
}
void ldCB(GbState* s, u32 instruction) { // debug(s, "ldCB");
    C = B;
}
/*
void ldCC(GbState* s, u32 instruction) { // debug(s, "ldCC");
    ;
}
*/
void ldCD(GbState* s, u32 instruction) { // debug(s, "ldCD");
    C = D;
}
void ldCE(GbState* s, u32 instruction) { // debug(s, "ldCE");
    C = E;
}
void ldCH(GbState* s, u32 instruction) { // debug(s, "ldCH");
    C = H;
}
void ldCL(GbState* s, u32 instruction) { // debug(s, "ldCL");
    C = L;
}

/**
 * ld C, R8s
 */
void ldDA(GbState* s, u32 instruction) { // debug(s, "ldDA");
    D = A;
}
void ldDB(GbState* s, u32 instruction) { // debug(s, "ldDB");
    D = B;
}
void ldDC(GbState* s, u32 instruction) { // debug(s, "ldDC");
    D = C;
}
/*
void ldDD(GbState* s, u32 instruction) { // debug(s, "ldDD");
    D = D;
}
*/
void ldDE(GbState* s, u32 instruction) { // debug(s, "ldDE");
    D = E;
}
void ldDH(GbState* s, u32 instruction) { // debug(s, "ldDH");
    D = H;
}
void ldDL(GbState* s, u32 instruction) { // debug(s, "ldDL");
    D = L;
}

/**
 * ld A, R8s
 */
/*
void ldAA(GbState* s, u32 instruction) { // debug(s, "ldAB");
    ;
}
*/
void ldAB(GbState* s, u32 instruction) { // debug(s, "ldAB");
    A = B;
}
void ldAC(GbState* s, u32 instruction) { // debug(s, "ldAC");
    A = C;
}
void ldAD(GbState* s, u32 instruction) { // debug(s, "ldAD");
    A = D;
}
void ldAE(GbState* s, u32 instruction) { // debug(s, "ldAE");
    A = E;
}
void ldAH(GbState* s, u32 instruction) { // debug(s, "ldAH");
    A = H;
}
void ldAL(GbState* s, u32 instruction) { // debug(s, "ldAL");
    A = L;
}

void ldBaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    B = mem(HL);
}
void ldCaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    C = mem(HL);
}
void ldDaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    D = mem(HL);
}
void ldEaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    E = mem(HL);
}
void ldHaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    H = mem(HL);
}
void ldLaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    L = mem(HL);
}

void ldaHLB(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    mmu_write(s, HL, B);
}
void ldaHLC(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    mmu_write(s, HL, C);
}
void ldaHLD(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    mmu_write(s, HL, D);
}
void ldaHLE(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    mmu_write(s, HL, E);
}
void ldaHLH(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    mmu_write(s, HL, H);
}
void ldaHLL(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    mmu_write(s, HL, L);
}

void ldaBCA(GbState* s, u32 instruction) { // debug(s, "ldaBCA");
    mmu_write(s, BC, A);
}
void ldaDEA(GbState* s, u32 instruction) { // debug(s, "ldaDEA");
    mmu_write(s, DE, A);
}
void ldaHLA(GbState* s, u32 instruction) { // debug(s, "ldaHLA");
    mmu_write(s, HL, A);
}
void ldAaBC(GbState* s, u32 instruction) { // debug(s, "ldAaBC");
    A = mem(BC);
}
void ldAaDE(GbState* s, u32 instruction) { // debug(s, "ldAaDE");
    A = mem(DE);
}
void ldAaHL(GbState* s, u32 instruction) { // debug(s, "ldAaHL");
    A = mem(HL);
}

void ldAaN16(GbState* s, u32 instruction) { // debug(s, "ldAaN16");
    A = mmu_read(s, IMM16);
    PC += 2;
}

void ldaN16A(GbState* s, u32 instruction) { // debug(s, "ldaN16A");
    mmu_write(s, IMM16, A);
    PC += 2;
}
void ldAaC(GbState* s, u32 instruction) { // debug(s, "ldAaC");
    A = mmu_read(s, 0xff00 + C);
}
void ldaCA(GbState* s, u32 instruction) { // debug(s, "ldaCA");
    mmu_write(s, 0xff00 + C, A);
}
void lddAaHL(GbState* s, u32 instruction) { // debug(s, "lddAaHL");
    A = mmu_read(s, HL);
    HL--;
}
void lddaHLA(GbState* s, u32 instruction) { // debug(s, "lddaHLA");
    mmu_write(s, HL, A);
    HL--;
}
void ldiaHLA(GbState* s, u32 instruction) { // debug(s, "ldiaHLA");
    mmu_write(s, HL, A);
    HL++;
}
void ldiAaHL(GbState* s, u32 instruction) { // debug(s, "ldiAaHL");
    A = mmu_read(s, HL);
    HL++;
}
void ldhAaN8(GbState* s, u32 instruction) { // debug(s, "ldhAaN8");
    A = mmu_read(s, 0xff00 + IMM8);
    PC++;
}
void ldhaN8A(GbState* s, u32 instruction) { // debug(s, "ldhaN8A");
    mmu_write(s, 0xff00 + IMM8, A);
    PC++;
}

void ldBCN16(GbState* s, u32 instruction) {
    BC = IMM16;
    PC += 2;
}

void ldDEN16(GbState* s, u32 instruction) {
    DE = IMM16;
    PC += 2;
}

void ldHLN16(GbState* s, u32 instruction) {
    HL = IMM16;
    PC += 2;
}

void ldSPN16(GbState* s, u32 instruction) {
    s->sp = IMM16;
    PC += 2;
}

void ldSPHL(GbState* s, u32 instruction) { // debug(s, "ldSPHL");
    s->sp = HL;
}

void ldHLSPN8(GbState* s, u32 instruction) { // debug(s, "ldHLSPN8");
    u32 res = (u32)s->sp + (s8)IMM8;
    ZF = 0;
    NF = 0;
    HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
    CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
    HL = (u16)res;
    PC++;
}

void ldaN16SP(GbState* s, u32 instruction) { // debug(s, "ldaN16SP");
    mmu_write16(s, IMM16, s->sp);
    PC += 2; 
}

void pushBC(GbState* s, u32 instruction) { // debug(s, "pushR16");
    mmu_push16(s, BC);
}
void pushDE(GbState* s, u32 instruction) { // debug(s, "pushR16");
    mmu_push16(s, DE);
}
void pushHL(GbState* s, u32 instruction) { // debug(s, "pushR16");
    mmu_push16(s, HL);
}
void pushAF(GbState* s, u32 instruction) { // debug(s, "pushR16");
    mmu_push16(s, AF);
}

void popBC(GbState* s, u32 instruction) { // debug(s, "popR16");
    BC = mmu_pop16(s);
}
void popDE(GbState* s, u32 instruction) { // debug(s, "popR16");
    DE = mmu_pop16(s);
}
void popHL(GbState* s, u32 instruction) { // debug(s, "popR16");
    HL = mmu_pop16(s);
}
void popAF(GbState* s, u32 instruction) { // debug(s, "popR16");
    AF = mmu_pop16(s);
    // Bottom 4 bits of F don't exist.
    F = F & 0xf0;
}

void addAN8(GbState* s, u32 instruction) { // debug(s, "addAN8");
    u16 res = A + IMM8;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
    PC++;
}

void addAA(GbState* s, u32 instruction) {
    u16 res = A + A;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ A ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void addAB(GbState* s, u32 instruction) {
    u16 res = A + B;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ B ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void addAC(GbState* s, u32 instruction) {
    u16 res = A + C;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ C ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void addAD(GbState* s, u32 instruction) {
    u16 res = A + D;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ D ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void addAE(GbState* s, u32 instruction) {
    u16 res = A + E;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ E ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void addAH(GbState* s, u32 instruction) {
    u16 res = A + H;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ H ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void addAL(GbState* s, u32 instruction) {
    u16 res = A + L;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ L ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void addAaHL(GbState* s, u32 instruction) { // debug(s, "addAaHL"); 
    u8 srcval = mem(HL);
    u16 res = A + srcval;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}

void adcAN8(GbState* s, u32 instruction) { // debug(s, "adcAN8");
    u16 res = A + IMM8 + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
    PC++;
}

void adcAA(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    u16 res = A + A + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ A ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void adcAB(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    u16 res = A + B + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ B ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void adcAC(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    u16 res = A + C + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ C ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void adcAD(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    u16 res = A + D + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ D ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void adcAE(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    u16 res = A + E + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ E ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void adcAH(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    u16 res = A + H + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ H ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}
void adcAL(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    u16 res = A + L + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ L ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
}

void adcAaHL(GbState* s, u32 instruction) { // debug(s, "adcAaHL");
    u8 srcval = mem(HL);
    u16 res = A + srcval + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res; 
}

void subAN8(GbState* s, u32 instruction) { // debug(s, "subAN8");
    u8 res = A - IMM8;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - ((s32)IMM8 & 0xf) < 0;
    CF = A < IMM8;
    A = res;
    PC++;
}
void subAA(GbState* s, u32 instruction) { // debug(s, "subAR8");
    F = FLAG_Z | FLAG_N;
    A = 0;
}
void subAB(GbState* s, u32 instruction) { // debug(s, "subAR8");
    u8 res = A - B;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (B & 0xf) < 0;
    CF = A < B;
    A = res;
}
void subAC(GbState* s, u32 instruction) { // debug(s, "subAR8");
    u8 res = A - C;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (C & 0xf) < 0;
    CF = A < C;
    A = res;
}
void subAD(GbState* s, u32 instruction) { // debug(s, "subAR8");
    u8 res = A - D;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (D & 0xf) < 0;
    CF = A < D;
    A = res;
}
void subAE(GbState* s, u32 instruction) { // debug(s, "subAR8");
    u8 res = A - E;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (E & 0xf) < 0;
    CF = A < E;
    A = res;
}
void subAH(GbState* s, u32 instruction) { // debug(s, "subAR8");
    u8 res = A - H;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (H & 0xf) < 0;
    CF = A < H;
    A = res;
}
void subAL(GbState* s, u32 instruction) { // debug(s, "subAR8");
    u8 res = A - L;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (L & 0xf) < 0;
    CF = A < L;
    A = res;
}
void subAaHL(GbState* s, u32 instruction) { // debug(s, "subAaHL");
    u8 val = mem(HL);
    u8 res = A - val;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (val & 0xf) < 0;
    CF = A < val;
    A = res;
}

void sbcAN8(GbState* s, u32 instruction) { // debug(s, "sbcAN8");
    u8 res = A - IMM8 - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - ((s32)IMM8 & 0xf) - CF < 0;
    CF = A < IMM8 + CF;
    A = res;
    PC++;
}

void sbcAA(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    // TODO: Simplify this.
    u8 res = A - A - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (A & 0xf) - CF < 0;
    CF = A < A + CF;
    A = res;
}
void sbcAB(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    u8 res = A - B - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (B & 0xf) - CF < 0;
    CF = A < B + CF;
    A = res;
}
void sbcAC(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    u8 res = A - C - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (C & 0xf) - CF < 0;
    CF = A < C + CF;
    A = res;
}
void sbcAD(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    u8 res = A - D - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (D & 0xf) - CF < 0;
    CF = A < D + CF;
    A = res;
}
void sbcAE(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    u8 res = A - E - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (E & 0xf) - CF < 0;
    CF = A < E + CF;
    A = res;
}
void sbcAH(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    u8 res = A - H - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (H & 0xf) - CF < 0;
    CF = A < H + CF;
    A = res;
}
void sbcAL(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    u8 res = A - L - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (L & 0xf) - CF < 0;
    CF = A < L + CF;
    A = res;
}
void sbcAaHL(GbState* s, u32 instruction) { // debug(s, "sbcAaHL");
    u8 regval = mem(HL);
    u8 res = A - regval - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (regval & 0xf) - CF < 0;
    CF = A < regval + CF;
    A = res;
}

void andAN8(GbState* s, u32 instruction) { // debug(s, "andAN8");
    A = A & IMM8;
    PC++;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;
}

void andAA(GbState* s, u32 instruction) { // debug(s, "andAR8");
    A = A & A;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;
}

void andAB(GbState* s, u32 instruction) { // debug(s, "andAR8");
    A = A & B;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;
}

void andAC(GbState* s, u32 instruction) { // debug(s, "andAR8");
    A = A & C;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;
}

void andAD(GbState* s, u32 instruction) { // debug(s, "andAR8");
    A = A & D;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;
}

void andAE(GbState* s, u32 instruction) { // debug(s, "andAR8");
    A = A & E;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;
}

void andAH(GbState* s, u32 instruction) { // debug(s, "andAR8");
    A = A & H;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;
}

void andAL(GbState* s, u32 instruction) { // debug(s, "andAR8");
    A = A & L;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;
}

void andAaHL(GbState* s, u32 instruction) { // debug(s, "andAaHL");
    u8 val = mem(HL);
    A = A & val;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;
}

void orAN8(GbState* s, u32 instruction) { // debug(s, "orAN8");
    A |= IMM8;
    F = A ? 0 : FLAG_Z;
    PC++;
}

void orAA(GbState* s, u32 instruction) { // debug(s, "orAR8");
    A |= A;
    F = A ? 0 : FLAG_Z;
}
void orAB(GbState* s, u32 instruction) { // debug(s, "orAR8");
    A |= B;
    F = A ? 0 : FLAG_Z;
}
void orAC(GbState* s, u32 instruction) { // debug(s, "orAR8");
    A |= C;
    F = A ? 0 : FLAG_Z;
}
void orAD(GbState* s, u32 instruction) { // debug(s, "orAR8");
    A |= D;
    F = A ? 0 : FLAG_Z;
}
void orAE(GbState* s, u32 instruction) { // debug(s, "orAR8");
    A |= E;
    F = A ? 0 : FLAG_Z;
}
void orAH(GbState* s, u32 instruction) { // debug(s, "orAR8");
    A |= H;
    F = A ? 0 : FLAG_Z;
}
void orAL(GbState* s, u32 instruction) { // debug(s, "orAR8");
    A |= L;
    F = A ? 0 : FLAG_Z;
}

void orAaHL(GbState* s, u32 instruction) { // debug(s, "orAaHL");
    u8 srcval = mem(HL);
    A |= srcval;
    F = A ? 0 : FLAG_Z;
}

void xorAN8(GbState* s, u32 instruction) { // debug(s, "xorAN8");
    A ^= IMM8;
    PC++;
    F = A ? 0 : FLAG_Z;
}
void xorAA(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    A = 0;
    F = FLAG_Z; 
}
void xorAB(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    A ^= B;
    F = A ? 0 : FLAG_Z; 
}
void xorAC(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    A ^= C;
    F = A ? 0 : FLAG_Z; 
}
void xorAD(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    A ^= D;
    F = A ? 0 : FLAG_Z; 
}
void xorAE(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    A ^= E;
    F = A ? 0 : FLAG_Z; 
}
void xorAH(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    A ^= H;
    F = A ? 0 : FLAG_Z; 
}
void xorAL(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    A ^= L;
    F = A ? 0 : FLAG_Z; 
}
void xorAaHL(GbState* s, u32 instruction) { // debug(s, "xorAaHL");
    u8 srcval = mem(HL);
    A ^= srcval;
    F = A ? 0 : FLAG_Z; 
}

void cpAN8(GbState* s, u32 instruction) { // debug(s, "cpAN8");
    u8 n = IMM8;
    ZF = A == n;
    NF = 1;
    HF = (A & 0xf) < (n & 0xf);
    CF = A < n;
    PC++;
}

void cpAA(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    F = FLAG_Z | FLAG_N;
    /*
    ZF = 1
    NF = 1;
    HF = 0;
    CF = 0;
    */
}
void cpAB(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    ZF = A == B;
    NF = 1;
    HF = (A & 0xf) < (B & 0xf);
    CF = A < B;
}
void cpAC(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    ZF = A == C;
    NF = 1;
    HF = (A & 0xf) < (C & 0xf);
    CF = A < C;
}
void cpAD(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    ZF = A == D;
    NF = 1;
    HF = (A & 0xf) < (D & 0xf);
    CF = A < D;
}
void cpAE(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    ZF = A == E;
    NF = 1;
    HF = (A & 0xf) < (E & 0xf);
    CF = A < E;
}
void cpAH(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    ZF = A == H;
    NF = 1;
    HF = (A & 0xf) < (H & 0xf);
    CF = A < H;
}
void cpAL(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    ZF = A == L;
    NF = 1;
    HF = (A & 0xf) < (L & 0xf);
    CF = A < L;
}
void cpAaHL(GbState* s, u32 instruction) { // debug(s, "cpAaHL");
    u8 regval = mem(HL);
    ZF = A == regval;
    NF = 1;
    HF = (A & 0xf) < (regval & 0xf);
    CF = A < regval;
}

void incA(GbState* s, u32 instruction) {
    HF = (A & 0xf) == 0xf;
    A++;
    ZF = A == 0;
    NF = 0;
}

void incB(GbState* s, u32 instruction) {
    HF = (B & 0xf) == 0xf;
    B++;
    ZF = B == 0;
    NF = 0;
}

void incD(GbState* s, u32 instruction) {
    HF = (D & 0xf) == 0xf;
    D++;
    ZF = D == 0;
    NF = 0;
}

void incH(GbState* s, u32 instruction) {
    HF = (H & 0xf) == 0xf;
    H++;
    ZF = H == 0;
    NF = 0;
}

void incC(GbState* s, u32 instruction) {
    HF = (C & 0xf) == 0xf;
    C++;
    ZF = C == 0;
    NF = 0;
}

void incE(GbState* s, u32 instruction) {
    HF = (E & 0xf) == 0xf;
    E++;
    ZF = E == 0;
    NF = 0;
}

void incL(GbState* s, u32 instruction) {
    HF = (L & 0xf) == 0xf;
    L++;
    ZF = L == 0;
    NF = 0;
}

void incaHL(GbState* s, u32 instruction) { // debug(s, "incaHL");
    u8 val = mem(HL);
    u8 res = val + 1;
    ZF = res == 0;
    NF = 0;
    HF = (val & 0xf) == 0xf;
    mmu_write(s, HL, res);
}

void decA(GbState* s, u32 instruction) { // debug(s, "decR8");
    A--;
    HF = (A & 0x0F) == 0x0F;
    NF = 1;
    ZF = A == 0;
}

void decB(GbState* s, u32 instruction) { // debug(s, "decR8");
    B--;
    HF = (B & 0x0F) == 0x0F;
    NF = 1;
    ZF = B == 0;
}

void decD(GbState* s, u32 instruction) { // debug(s, "decR8");
    D--;
    HF = (D & 0x0F) == 0x0F;
    NF = 1;
    ZF = D == 0;
}

void decH(GbState* s, u32 instruction) { // debug(s, "decR8");
    H--;
    HF = (H & 0x0F) == 0x0F;
    NF = 1;
    ZF = H == 0;
}

void decC(GbState* s, u32 instruction) { // debug(s, "decR8");
    C--;
    HF = (C & 0x0F) == 0x0F;
    NF = 1;
    ZF = C == 0;
}

void decE(GbState* s, u32 instruction) { // debug(s, "decR8");
    E--;
    HF = (E & 0x0F) == 0x0F;
    NF = 1;
    ZF = E == 0;
}

void decL(GbState* s, u32 instruction) { // debug(s, "decR8");
    L--;
    HF = (L & 0x0F) == 0x0F;
    NF = 1;
    ZF = L == 0;
}

void decaHL(GbState* s, u32 instruction) { // debug(s, "decaHL");
    u8 val = mem(HL);
    val--;
    NF = 1;
    ZF = val == 0;
    HF = (val & 0x0F) == 0x0F;
    mmu_write(s, HL, val);
}

void incBC(GbState* s, u32 instruction) {
    BC++;
}

void incDE(GbState* s, u32 instruction) {
    DE++;
}

void incHL(GbState* s, u32 instruction) {
    HL++;
}

void incSP(GbState* s, u32 instruction) {
    s->sp++;
}

void decBC(GbState* s, u32 instruction) { // debug(s, "decR16");
    BC--;
}

void decDE(GbState* s, u32 instruction) { // debug(s, "decR16");
    DE--;
}

void decHL(GbState* s, u32 instruction) { // debug(s, "decR16");
    HL--;
}

void decSP(GbState* s, u32 instruction) { // debug(s, "decR16");
    s->sp--;
}

void addHLBC(GbState* s, u32 instruction) { // debug(s, "addHLR16");
    HF = (((HL & 0xfff) + (BC & 0xfff)) & 0x1000) ? 1 : 0;
    u32 tmp = HL + BC;
    NF = 0;
    CF = tmp > 0xFFFF;
    HL = tmp;
}

void addHLDE(GbState* s, u32 instruction) { // debug(s, "addHLR16");
    HF = (((HL & 0xfff) + (DE & 0xfff)) & 0x1000) ? 1 : 0;
    u32 tmp = HL + DE;
    NF = 0;
    CF = tmp > 0xFFFF;
    HL = tmp;
}

void addHLHL(GbState* s, u32 instruction) { // debug(s, "addHLR16");
    HF = (((HL & 0xfff) + (HL & 0xfff)) & 0x1000) ? 1 : 0;
    u32 tmp = HL + HL;
    NF = 0;
    CF = tmp > 0xFFFF;
    HL = tmp;
}

void addHLSP(GbState* s, u32 instruction) { // debug(s, "addHLR16");
    HF = (((HL & 0xfff) + (s->sp & 0xfff)) & 0x1000) ? 1 : 0;
    u32 tmp = HL + s->sp;
    NF = 0;
    CF = tmp > 0xFFFF;
    HL = tmp;
}

void addSPN8(GbState* s, u32 instruction) { // debug(s, "addSPN8");
    s8 off = (s8)IMM8;
    u32 res = s->sp + off;
    ZF = 0;
    NF = 0;
    HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
    CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
    s->sp = res;
    PC++;
}
void daa(GbState* s, u32 instruction) { // debug(s, "daa");
    s8 add = 0;
    if ((!NF && (A & 0xf) > 0x9) || HF)
        add |= 0x6;
    if ((!NF && A > 0x99) || CF) {
        add |= 0x60;
        CF = 1;
    }
    A += NF ? -add : add;
    ZF = A == 0;
    HF = 0;
}
void cpl(GbState* s, u32 instruction) { // debug(s, "cpl");
    A = ~A;
    F = F | FLAG_N | FLAG_H;
}
void ccf(GbState* s, u32 instruction) { // debug(s, "ccf");
    CF = CF ? 0 : 1;
    NF = 0;
    HF = 0;
}
void scf(GbState* s, u32 instruction) { // debug(s, "scf");
    NF = 0;
    HF = 0;
    CF = 1;
}
void rlcA(GbState* s, u32 instruction) { // debug(s, "rlcA");
    u8 res = (A << 1) | (A >> 7);
    F = (A >> 7) ? FLAG_C : 0;
    A = res;
}
void rlA(GbState* s, u32 instruction) { // debug(s, "rlA");
    u8 res = A << 1 | (CF ? 1 : 0);
    F = (A & (1 << 7)) ? FLAG_C : 0;
    A = res;
}
void rrcA(GbState* s, u32 instruction) { // debug(s, "rrcA");
    F = (A & 1) ? FLAG_C : 0;
    A = (A >> 1) | ((A & 1) << 7);
}
void rrA(GbState* s, u32 instruction) { // debug(s, "rrA");
    u8 res = (A >> 1) | (CF << 7);
    F = 0;
    CF = A & 0x1;
    A = res;
}
void halt(GbState* s, u32 instruction){
    s->halt_for_interrupts = 1;
}
void stop(GbState* s, u32 instruction) { // debug(s, "stop");
    // For GBC games, stop instruction also handles the speed switch.
    if (s->IsSpeedSwitchPending && s->Cartridge.IsGbcSupported) {
        s->IsInDoubleSpeedMode = !s->IsInDoubleSpeedMode;
        s->IsSpeedSwitchPending = 0;

        if (s->IsInDoubleSpeedMode) {
            s->io_lcd_mode_cycles_left *= 2;
        } else {
            s->io_lcd_mode_cycles_left /= 2;
        }

    } else {
        ; // TODO
    }
}
void di(GbState* s, u32 instruction) { // debug(s, "di");
    s->interrupts_master_enabled = 0;
}
void ei(GbState* s, u32 instruction) { // debug(s, "ei");
    s->interrupts_master_enabled = 1;
}
void jpN16(GbState* s, u32 instruction) { // debug(s, "jpN16");
    PC = IMM16;
}
void jpNZN16(GbState* s, u32 instruction) { // debug(s, "jpCCN16");
    if (!(F & FLAG_Z)) {
        PC = IMM16;
    } else {
        PC += 2;
    }
}
void jpNCN16(GbState* s, u32 instruction) { // debug(s, "jpCCN16");
    if (!(F & FLAG_C)) {
        PC = IMM16;
    } else {
        PC += 2;
    }
}
void jpZN16(GbState* s, u32 instruction) { // debug(s, "jpCCN16");
    if (F & FLAG_Z) {
        PC = IMM16;
    } else {
        PC += 2;
    }
}
void jpCN16(GbState* s, u32 instruction) { // debug(s, "jpCCN16");
    if (F & FLAG_C) {
        PC = IMM16;
    } else {
        PC += 2;
    }
}

void jpHL(GbState* s, u32 instruction) { // debug(s, "jpHL");
    PC = HL;
}
void jrN8(GbState* s, u32 instruction) { // debug(s, "jrN8");
    PC += (s8)IMM8 + 1;
}

void jrNZN8(GbState* s, u32 instruction) { // debug(s, "jrCCN8");
    if (!(F & FLAG_Z)) {
        PC += (s8)IMM8;
    }
    PC++;
}

void jrNCN8(GbState* s, u32 instruction) { // debug(s, "jrCCN8");
    if (!(F & FLAG_C)) {
        PC += (s8)IMM8;
    }
    PC++;
}

void jrZN8(GbState* s, u32 instruction) { // debug(s, "jrCCN8");
    if (F & FLAG_Z) {
        PC += (s8)IMM8;
    }
    PC++;
}

void jrCN8(GbState* s, u32 instruction) { // debug(s, "jrCCN8");
    if (F & FLAG_C) {
        PC += (s8)IMM8;
    }
    PC++;
}

void callN16(GbState* s, u32 instruction) { // debug(s, "callN16");
    u16 dst = IMM16;
    mmu_push16(s, PC + 2);
    PC = dst;
}
void callNZN16(GbState* s, u32 instruction) { // debug(s, "callCCN16");
    PC += 2;
    if (!(F & FLAG_Z)) {
        mmu_push16(s, PC);
        PC = IMM16;
    }
}
void callNCN16(GbState* s, u32 instruction) { // debug(s, "callCCN16");
    PC += 2;
    if (!(F & FLAG_C)) {
        mmu_push16(s, PC);
        PC = IMM16;
    }
}
void callZN16(GbState* s, u32 instruction) { // debug(s, "callCCN16");
    PC += 2;
    if (F & FLAG_Z) {
        mmu_push16(s, PC);
        PC = IMM16;
    }
}
void callCN16(GbState* s, u32 instruction) { // debug(s, "callCCN16");
    PC += 2;
    if (F & FLAG_C) {
        mmu_push16(s, PC);
        PC = IMM16;
    }
}

void rstVec(GbState* s, u32 instruction) { // debug(s, "rstVec");
    mmu_push16(s, PC);
    PC = ((op >> 3) & 7) * 8;
}
void retNZ(GbState* s, u32 instruction) { // debug(s, "retCC");
    if (!(F & FLAG_Z)) {
        PC = mmu_pop16(s);
    }
}
void retNC(GbState* s, u32 instruction) { // debug(s, "retCC");
    if (!(F & FLAG_C)) {
        PC = mmu_pop16(s);
    }
}
void retZ(GbState* s, u32 instruction) { // debug(s, "retCC");
    if (F & FLAG_Z) {
        PC = mmu_pop16(s);
    }
}
void retC(GbState* s, u32 instruction) { // debug(s, "retCC");
    if (F & FLAG_C) {
        PC = mmu_pop16(s);
    }
}

void reti(GbState* s, u32 instruction) { // debug(s, "reti");
    PC = mmu_pop16(s);
    s->interrupts_master_enabled = 1;
}
void ret(GbState* s, u32 instruction) { // debug(s, "ret");
    PC = mmu_pop16(s);
}

/**
 * An undefined op code should cause the cpu to hang.
 */
void undefined(GbState* s, u32 instruction) { // debug(s, "undefined");
    logAndPauseFrame(0, "UNDEFINED");
}