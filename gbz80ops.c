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

void rlca(GbState* s, u32 instruction) { // debug(s, "rlcA");
    u8 res = (A << 1) | (A >> 7);
    F = (A >> 7) ? FLAG_C : 0;
    A = res;
}
void rlcA(GbState* s, u32 instruction) { // debug(s, "rlcR8");
    CF = A >> 7;
    A = (A << 1) | (A >> 7);
    ZF = A == 0;
    NF = 0;
    HF = 0;
}
void rlcB(GbState* s, u32 instruction) { // debug(s, "rlcR8");
    CF = B >> 7;
    B = (B << 1) | (B >> 7);
    ZF = B == 0;
    NF = 0;
    HF = 0;
}
void rlcC(GbState* s, u32 instruction) { // debug(s, "rlcR8");
    CF = C >> 7;
    C = (C << 1) | (C >> 7);
    ZF = C == 0;
    NF = 0;
    HF = 0;
}
void rlcD(GbState* s, u32 instruction) { // debug(s, "rlcR8");
    CF = D >> 7;
    D = (D << 1) | (D >> 7);
    ZF = D == 0;
    NF = 0;
    HF = 0;
}
void rlcE(GbState* s, u32 instruction) { // debug(s, "rlcR8");
    CF = E >> 7;
    E = (E << 1) | (E >> 7);
    ZF = E == 0;
    NF = 0;
    HF = 0;
}
void rlcH(GbState* s, u32 instruction) { // debug(s, "rlcR8");
    CF = H >> 7;
    H = (H << 1) | (H >> 7);
    ZF = H == 0;
    NF = 0;
    HF = 0;
}
void rlcL(GbState* s, u32 instruction) { // debug(s, "rlcR8");
    CF = L >> 7;
    L = (L << 1) | (L >> 7);
    ZF = L == 0;
    NF = 0;
    HF = 0;
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

#define RRC_R8(r8) \
    CF = r8 & 1;\
    r8 = (r8 >> 1) | ((r8 & 1) << 7);\
    ZF = r8 == 0;\
    NF = 0;\
    HF = 0

void rrcB(GbState* s, u32 instruction) {
    RRC_R8(B);
}
void rrcC(GbState* s, u32 instruction) {
    RRC_R8(C);
}
void rrcD(GbState* s, u32 instruction) {
    RRC_R8(D);
}
void rrcE(GbState* s, u32 instruction) {
    RRC_R8(E);
}
void rrcH(GbState* s, u32 instruction) {
    RRC_R8(H);
}
void rrcL(GbState* s, u32 instruction) {
    RRC_R8(L);
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

void rrcA(GbState* s, u32 instruction) {
    RRC_R8(A);
}

#define RL_R8(r8) \
    u8 temp = (r8 << 1) | (CF ? 1 : 0);\
    CF = r8 >> 7;\
    r8 = temp;\
    ZF = r8 == 0;\
    NF = 0;\
    HF = 0

void rlB(GbState* s, u32 instruction) { // debug(s, "rlR8");
    RL_R8(B);
}
void rlC(GbState* s, u32 instruction) { // debug(s, "rlR8");
    RL_R8(C);
}
void rlD(GbState* s, u32 instruction) { // debug(s, "rlR8");
    RL_R8(D);
}
void rlE(GbState* s, u32 instruction) { // debug(s, "rlR8");
    RL_R8(E);
}
void rlH(GbState* s, u32 instruction) { // debug(s, "rlR8");
    RL_R8(H);
}
void rlL(GbState* s, u32 instruction) { // debug(s, "rlR8");
    RL_R8(L);
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

void rlA(GbState* s, u32 instruction) { // debug(s, "rlR8");
    RL_R8(A);
}

#define RR_R8(r8) \
    u8 temp = (r8 >> 1) | (CF << 7);\
    CF = r8 & 0x1;\
    r8 = temp;\
    ZF = r8 == 0;\
    NF = 0;\
    HF = 0

void rrB(GbState* s, u32 instruction) { // debug(s, "rrR8");
    RR_R8(B);
}
void rrC(GbState* s, u32 instruction) { // debug(s, "rrR8");
    RR_R8(C);
}
void rrD(GbState* s, u32 instruction) { // debug(s, "rrR8");
    RR_R8(D);
}
void rrE(GbState* s, u32 instruction) { // debug(s, "rrR8");
    RR_R8(E);
}
void rrH(GbState* s, u32 instruction) { // debug(s, "rrR8");
    RR_R8(H);
}
void rrL(GbState* s, u32 instruction) { // debug(s, "rrR8");
    RR_R8(L);
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

void rrA(GbState* s, u32 instruction) { // debug(s, "rrR8");
    RR_R8(A);
}



#define SLA_R8(r8) \
    CF = r8 >> 7;\
    r8 = r8 << 1;\
    ZF = r8 == 0;\
    NF = 0;\
    HF = 0

void slaB(GbState* s, u32 instruction) {
    SLA_R8(B);
}
void slaC(GbState* s, u32 instruction) {
    SLA_R8(C);
}
void slaD(GbState* s, u32 instruction) {
    SLA_R8(D);
}
void slaE(GbState* s, u32 instruction) {
    SLA_R8(E);
}
void slaH(GbState* s, u32 instruction) {
    SLA_R8(H);
}
void slaL(GbState* s, u32 instruction) {
    SLA_R8(L);
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

void slaA(GbState* s, u32 instruction) {
    SLA_R8(A);
}

#define SRA_R8(r8) \
    CF = r8 & 0x1;\
    r8 = (r8 >> 1) | (r8 & (1 << 7));\
    ZF = r8 == 0;\
    NF = 0;\
    HF = 0

void sraB(GbState* s, u32 instruction) { // debug(s, "sraR8");
    SRA_R8(B);
}
void sraC(GbState* s, u32 instruction) { // debug(s, "sraR8");
    SRA_R8(C);
}
void sraD(GbState* s, u32 instruction) { // debug(s, "sraR8");
    SRA_R8(D);
}
void sraE(GbState* s, u32 instruction) { // debug(s, "sraR8");
    SRA_R8(E);
}
void sraH(GbState* s, u32 instruction) { // debug(s, "sraR8");
    SRA_R8(H);
}
void sraL(GbState* s, u32 instruction) { // debug(s, "sraR8");
    SRA_R8(L);
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

void sraA(GbState* s, u32 instruction) { // debug(s, "sraR8");
    SRA_R8(A);
}

#define SWAP_R8(r8) \
    r8 = ((r8 << 4) & 0xf0) | ((r8 >> 4) & 0xf);\
    F = r8 == 0 ? FLAG_Z : 0

void swapB(GbState* s, u32 instruction) { // debug(s, "swapR8");
    SWAP_R8(B);
}
void swapC(GbState* s, u32 instruction) { // debug(s, "swapR8");
    SWAP_R8(C);
}
void swapD(GbState* s, u32 instruction) { // debug(s, "swapR8");
    SWAP_R8(D);
}
void swapE(GbState* s, u32 instruction) { // debug(s, "swapR8");
    SWAP_R8(E);
}
void swapH(GbState* s, u32 instruction) { // debug(s, "swapR8");
    SWAP_R8(H);
}
void swapL(GbState* s, u32 instruction) { // debug(s, "swapR8");
    SWAP_R8(L);
}

void swapaHL(GbState* s, u32 instruction) { // debug(s, "swapaHL");
    u8 val = mem(HL);
    u8 res = ((val << 4) & 0xf0) | ((val >> 4) & 0xf);
    F = res == 0 ? FLAG_Z : 0;
    mmu_write(s, HL, res);
}
void swapA(GbState* s, u32 instruction) { // debug(s, "swapR8");
    SWAP_R8(A);
}

void srlA(GbState* s, u32 instruction) {
    F = 0;
    CF = A & 1;
    A = A >> 1;
    ZF = A == 0;
}
void srlB(GbState* s, u32 instruction) {
    F = 0;
    CF = B & 1;
    B = B >> 1;
    ZF = B == 0;
}
void srlC(GbState* s, u32 instruction) {
    F = 0;
    CF = C & 1;
    C = C >> 1;
    ZF = C == 0;
}
void srlD(GbState* s, u32 instruction) {
    F = 0;
    CF = D & 1;
    D = D >> 1;
    ZF = D == 0;
}
void srlE(GbState* s, u32 instruction) {
    F = 0;
    CF = E & 1;
    E = E >> 1;
    ZF = E == 0;
}
void srlH(GbState* s, u32 instruction) {
    F = 0;
    CF = H & 1;
    H = H >> 1;
    ZF = H == 0;
}
void srlL(GbState* s, u32 instruction) {
    F = 0;
    CF = L & 1;
    L = L >> 1;
    ZF = L == 0;
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

#define BIT_U3R8(u3, r8) \
    NF = 0;\
    HF = 1;\
    ZF = (r8 & (1 << u3)) == 0

void bit0B(GbState* s, u32 instruction) {
    BIT_U3R8(0, B);
}
void bit1B(GbState* s, u32 instruction) {
    BIT_U3R8(1, B);
}
void bit2B(GbState* s, u32 instruction) {
    BIT_U3R8(2, B);
}
void bit3B(GbState* s, u32 instruction) {
    BIT_U3R8(3, B);
}
void bit4B(GbState* s, u32 instruction) {
    BIT_U3R8(4, B);
}
void bit5B(GbState* s, u32 instruction) {
    BIT_U3R8(5, B);
}
void bit6B(GbState* s, u32 instruction) {
    BIT_U3R8(6, B);
}
void bit7B(GbState* s, u32 instruction) {
    BIT_U3R8(7, B);
}

void bit0C(GbState* s, u32 instruction) {
    BIT_U3R8(0, C);
}
void bit1C(GbState* s, u32 instruction) {
    BIT_U3R8(1, C);
}
void bit2C(GbState* s, u32 instruction) {
    BIT_U3R8(2, C);
}
void bit3C(GbState* s, u32 instruction) {
    BIT_U3R8(3, C);
}
void bit4C(GbState* s, u32 instruction) {
    BIT_U3R8(4, C);
}
void bit5C(GbState* s, u32 instruction) {
    BIT_U3R8(5, C);
}
void bit6C(GbState* s, u32 instruction) {
    BIT_U3R8(6, C);
}
void bit7C(GbState* s, u32 instruction) {
    BIT_U3R8(7, C);
}

void bit0D(GbState* s, u32 instruction) {
    BIT_U3R8(0, D);
}
void bit1D(GbState* s, u32 instruction) {
    BIT_U3R8(1, D);
}
void bit2D(GbState* s, u32 instruction) {
    BIT_U3R8(2, D);
}
void bit3D(GbState* s, u32 instruction) {
    BIT_U3R8(3, D);
}
void bit4D(GbState* s, u32 instruction) {
    BIT_U3R8(4, D);
}
void bit5D(GbState* s, u32 instruction) {
    BIT_U3R8(5, D);
}
void bit6D(GbState* s, u32 instruction) {
    BIT_U3R8(6, D);
}
void bit7D(GbState* s, u32 instruction) {
    BIT_U3R8(7, D);
}

void bit0E(GbState* s, u32 instruction) {
    BIT_U3R8(0, E);
}
void bit1E(GbState* s, u32 instruction) {
    BIT_U3R8(1, E);
}
void bit2E(GbState* s, u32 instruction) {
    BIT_U3R8(2, E);
}
void bit3E(GbState* s, u32 instruction) {
    BIT_U3R8(3, E);
}
void bit4E(GbState* s, u32 instruction) {
    BIT_U3R8(4, E);
}
void bit5E(GbState* s, u32 instruction) {
    BIT_U3R8(5, E);
}
void bit6E(GbState* s, u32 instruction) {
    BIT_U3R8(6, E);
}
void bit7E(GbState* s, u32 instruction) {
    BIT_U3R8(7, E);
}

void bit0H(GbState* s, u32 instruction) {
    BIT_U3R8(0, H);
}
void bit1H(GbState* s, u32 instruction) {
    BIT_U3R8(1, H);
}
void bit2H(GbState* s, u32 instruction) {
    BIT_U3R8(2, H);
}
void bit3H(GbState* s, u32 instruction) {
    BIT_U3R8(3, H);
}
void bit4H(GbState* s, u32 instruction) {
    BIT_U3R8(4, H);
}
void bit5H(GbState* s, u32 instruction) {
    BIT_U3R8(5, H);
}
void bit6H(GbState* s, u32 instruction) {
    BIT_U3R8(6, H);
}
void bit7H(GbState* s, u32 instruction) {
    BIT_U3R8(7, H);
}

void bit0L(GbState* s, u32 instruction) {
    BIT_U3R8(0, L);
}
void bit1L(GbState* s, u32 instruction) {
    BIT_U3R8(1, L);
}
void bit2L(GbState* s, u32 instruction) {
    BIT_U3R8(2, L);
}
void bit3L(GbState* s, u32 instruction) {
    BIT_U3R8(3, L);
}
void bit4L(GbState* s, u32 instruction) {
    BIT_U3R8(4, L);
}
void bit5L(GbState* s, u32 instruction) {
    BIT_U3R8(5, L);
}
void bit6L(GbState* s, u32 instruction) {
    BIT_U3R8(6, L);
}
void bit7L(GbState* s, u32 instruction) {
    BIT_U3R8(7, L);
}

void bit0A(GbState* s, u32 instruction) {
    BIT_U3R8(0, A);
}
void bit1A(GbState* s, u32 instruction) {
    BIT_U3R8(1, A);
}
void bit2A(GbState* s, u32 instruction) {
    BIT_U3R8(2, A);
}
void bit3A(GbState* s, u32 instruction) {
    BIT_U3R8(3, A);
}
void bit4A(GbState* s, u32 instruction) {
    BIT_U3R8(4, A);
}
void bit5A(GbState* s, u32 instruction) {
    BIT_U3R8(5, A);
}
void bit6A(GbState* s, u32 instruction) {
    BIT_U3R8(6, A);
}
void bit7A(GbState* s, u32 instruction) {
    BIT_U3R8(7, A);
}

#define BIT_U3aHL(u3) \
    ZF = (mem(HL) & (1 << u3)) == 0;\
    NF = 0;\
    HF = 1

void bit0aHL(GbState* s, u32 instruction) { // debug(s, "bitU3aHL");
    BIT_U3aHL(0);
}
void bit1aHL(GbState* s, u32 instruction) { // debug(s, "bitU3aHL");
    BIT_U3aHL(1);
}
void bit2aHL(GbState* s, u32 instruction) { // debug(s, "bitU3aHL");
    BIT_U3aHL(2);
}
void bit3aHL(GbState* s, u32 instruction) { // debug(s, "bitU3aHL");
    BIT_U3aHL(3);
}
void bit4aHL(GbState* s, u32 instruction) { // debug(s, "bitU3aHL");
    BIT_U3aHL(4);
}
void bit5aHL(GbState* s, u32 instruction) { // debug(s, "bitU3aHL");
    BIT_U3aHL(5);
}
void bit6aHL(GbState* s, u32 instruction) { // debug(s, "bitU3aHL");
    BIT_U3aHL(6);
}
void bit7aHL(GbState* s, u32 instruction) { // debug(s, "bitU3aHL");
    BIT_U3aHL(7);
}

#define RES_U3R8(u3, r8) r8 = r8 & ~(1 << u3)

void res0B(GbState* s, u32 instruction) {
    RES_U3R8(0, B);
}
void res1B(GbState* s, u32 instruction) {
    RES_U3R8(1, B);
}
void res2B(GbState* s, u32 instruction) {
    RES_U3R8(2, B);
}
void res3B(GbState* s, u32 instruction) {
    RES_U3R8(3, B);
}
void res4B(GbState* s, u32 instruction) {
    RES_U3R8(4, B);
}
void res5B(GbState* s, u32 instruction) {
    RES_U3R8(5, B);
}
void res6B(GbState* s, u32 instruction) {
    RES_U3R8(6, B);
}
void res7B(GbState* s, u32 instruction) {
    RES_U3R8(7, B);
}
void res0C(GbState* s, u32 instruction) {
    RES_U3R8(0, C);
}
void res1C(GbState* s, u32 instruction) {
    RES_U3R8(1, C);
}
void res2C(GbState* s, u32 instruction) {
    RES_U3R8(2, C);
}
void res3C(GbState* s, u32 instruction) {
    RES_U3R8(3, C);
}
void res4C(GbState* s, u32 instruction) {
    RES_U3R8(4, C);
}
void res5C(GbState* s, u32 instruction) {
    RES_U3R8(5, C);
}
void res6C(GbState* s, u32 instruction) {
    RES_U3R8(6, C);
}
void res7C(GbState* s, u32 instruction) {
    RES_U3R8(7, C);
}
void res0D(GbState* s, u32 instruction) {
    RES_U3R8(0, D);
}
void res1D(GbState* s, u32 instruction) {
    RES_U3R8(1, D);
}
void res2D(GbState* s, u32 instruction) {
    RES_U3R8(2, D);
}
void res3D(GbState* s, u32 instruction) {
    RES_U3R8(3, D);
}
void res4D(GbState* s, u32 instruction) {
    RES_U3R8(4, D);
}
void res5D(GbState* s, u32 instruction) {
    RES_U3R8(5, D);
}
void res6D(GbState* s, u32 instruction) {
    RES_U3R8(6, D);
}
void res7D(GbState* s, u32 instruction) {
    RES_U3R8(7, D);
}
void res0E(GbState* s, u32 instruction) {
    RES_U3R8(0, E);
}
void res1E(GbState* s, u32 instruction) {
    RES_U3R8(1, E);
}
void res2E(GbState* s, u32 instruction) {
    RES_U3R8(2, E);
}
void res3E(GbState* s, u32 instruction) {
    RES_U3R8(3, E);
}
void res4E(GbState* s, u32 instruction) {
    RES_U3R8(4, E);
}
void res5E(GbState* s, u32 instruction) {
    RES_U3R8(5, E);
}
void res6E(GbState* s, u32 instruction) {
    RES_U3R8(6, E);
}
void res7E(GbState* s, u32 instruction) {
    RES_U3R8(7, E);
}
void res0H(GbState* s, u32 instruction) {
    RES_U3R8(0, H);
}
void res1H(GbState* s, u32 instruction) {
    RES_U3R8(1, H);
}
void res2H(GbState* s, u32 instruction) {
    RES_U3R8(2, H);
}
void res3H(GbState* s, u32 instruction) {
    RES_U3R8(3, H);
}
void res4H(GbState* s, u32 instruction) {
    RES_U3R8(4, H);
}
void res5H(GbState* s, u32 instruction) {
    RES_U3R8(5, H);
}
void res6H(GbState* s, u32 instruction) {
    RES_U3R8(6, H);
}
void res7H(GbState* s, u32 instruction) {
    RES_U3R8(7, H);
}
void res0L(GbState* s, u32 instruction) {
    RES_U3R8(0, L);
}
void res1L(GbState* s, u32 instruction) {
    RES_U3R8(1, L);
}
void res2L(GbState* s, u32 instruction) {
    RES_U3R8(2, L);
}
void res3L(GbState* s, u32 instruction) {
    RES_U3R8(3, L);
}
void res4L(GbState* s, u32 instruction) {
    RES_U3R8(4, L);
}
void res5L(GbState* s, u32 instruction) {
    RES_U3R8(5, L);
}
void res6L(GbState* s, u32 instruction) {
    RES_U3R8(6, L);
}
void res7L(GbState* s, u32 instruction) {
    RES_U3R8(7, L);
}

#define RES_U3aHL(u3) \
    u8 val = mem(HL) & ~(1 << u3);\
    mmu_write(s, HL, val)

void res0aHL(GbState* s, u32 instruction) { // debug(s, "resU3aHL");
    RES_U3aHL(0);
}
void res1aHL(GbState* s, u32 instruction) { // debug(s, "resU3aHL");
    RES_U3aHL(1);
}
void res2aHL(GbState* s, u32 instruction) { // debug(s, "resU3aHL");
    RES_U3aHL(2);
}
void res3aHL(GbState* s, u32 instruction) { // debug(s, "resU3aHL");
    RES_U3aHL(3);
}
void res4aHL(GbState* s, u32 instruction) { // debug(s, "resU3aHL");
    RES_U3aHL(4);
}
void res5aHL(GbState* s, u32 instruction) { // debug(s, "resU3aHL");
    RES_U3aHL(5);
}
void res6aHL(GbState* s, u32 instruction) { // debug(s, "resU3aHL");
    RES_U3aHL(6);
}
void res7aHL(GbState* s, u32 instruction) { // debug(s, "resU3aHL");
    RES_U3aHL(7);
}

void res0A(GbState* s, u32 instruction) {
    RES_U3R8(0, A);
}
void res1A(GbState* s, u32 instruction) {
    RES_U3R8(1, A);
}
void res2A(GbState* s, u32 instruction) {
    RES_U3R8(2, A);
}
void res3A(GbState* s, u32 instruction) {
    RES_U3R8(3, A);
}
void res4A(GbState* s, u32 instruction) {
    RES_U3R8(4, A);
}
void res5A(GbState* s, u32 instruction) {
    RES_U3R8(5, A);
}
void res6A(GbState* s, u32 instruction) {
    RES_U3R8(6, A);
}
void res7A(GbState* s, u32 instruction) {
    RES_U3R8(7, A);
}

#define SET_U3R8(u3, r8) \
    r8 = r8 | (1 << u3)

void set0B(GbState* s, u32 instruction) {
    SET_U3R8(0, B);
}
void set1B(GbState* s, u32 instruction) {
    SET_U3R8(1, B);
}
void set2B(GbState* s, u32 instruction) {
    SET_U3R8(2, B);
}
void set3B(GbState* s, u32 instruction) {
    SET_U3R8(3, B);
}
void set4B(GbState* s, u32 instruction) {
    SET_U3R8(4, B);
}
void set5B(GbState* s, u32 instruction) {
    SET_U3R8(5, B);
}
void set6B(GbState* s, u32 instruction) {
    SET_U3R8(6, B);
}
void set7B(GbState* s, u32 instruction) {
    SET_U3R8(7, B);
}
void set0C(GbState* s, u32 instruction) {
    SET_U3R8(0, C);
}
void set1C(GbState* s, u32 instruction) {
    SET_U3R8(1, C);
}
void set2C(GbState* s, u32 instruction) {
    SET_U3R8(2, C);
}
void set3C(GbState* s, u32 instruction) {
    SET_U3R8(3, C);
}
void set4C(GbState* s, u32 instruction) {
    SET_U3R8(4, C);
}
void set5C(GbState* s, u32 instruction) {
    SET_U3R8(5, C);
}
void set6C(GbState* s, u32 instruction) {
    SET_U3R8(6, C);
}
void set7C(GbState* s, u32 instruction) {
    SET_U3R8(7, C);
}
void set0D(GbState* s, u32 instruction) {
    SET_U3R8(0, D);
}
void set1D(GbState* s, u32 instruction) {
    SET_U3R8(1, D);
}
void set2D(GbState* s, u32 instruction) {
    SET_U3R8(2, D);
}
void set3D(GbState* s, u32 instruction) {
    SET_U3R8(3, D);
}
void set4D(GbState* s, u32 instruction) {
    SET_U3R8(4, D);
}
void set5D(GbState* s, u32 instruction) {
    SET_U3R8(5, D);
}
void set6D(GbState* s, u32 instruction) {
    SET_U3R8(6, D);
}
void set7D(GbState* s, u32 instruction) {
    SET_U3R8(7, D);
}
void set0E(GbState* s, u32 instruction) {
    SET_U3R8(0, E);
}
void set1E(GbState* s, u32 instruction) {
    SET_U3R8(1, E);
}
void set2E(GbState* s, u32 instruction) {
    SET_U3R8(2, E);
}
void set3E(GbState* s, u32 instruction) {
    SET_U3R8(3, E);
}
void set4E(GbState* s, u32 instruction) {
    SET_U3R8(4, E);
}
void set5E(GbState* s, u32 instruction) {
    SET_U3R8(5, E);
}
void set6E(GbState* s, u32 instruction) {
    SET_U3R8(6, E);
}
void set7E(GbState* s, u32 instruction) {
    SET_U3R8(7, E);
}
void set0H(GbState* s, u32 instruction) {
    SET_U3R8(0, H);
}
void set1H(GbState* s, u32 instruction) {
    SET_U3R8(1, H);
}
void set2H(GbState* s, u32 instruction) {
    SET_U3R8(2, H);
}
void set3H(GbState* s, u32 instruction) {
    SET_U3R8(3, H);
}
void set4H(GbState* s, u32 instruction) {
    SET_U3R8(4, H);
}
void set5H(GbState* s, u32 instruction) {
    SET_U3R8(5, H);
}
void set6H(GbState* s, u32 instruction) {
    SET_U3R8(6, H);
}
void set7H(GbState* s, u32 instruction) {
    SET_U3R8(7, H);
}
void set0L(GbState* s, u32 instruction) {
    SET_U3R8(0, L);
}
void set1L(GbState* s, u32 instruction) {
    SET_U3R8(1, L);
}
void set2L(GbState* s, u32 instruction) {
    SET_U3R8(2, L);
}
void set3L(GbState* s, u32 instruction) {
    SET_U3R8(3, L);
}
void set4L(GbState* s, u32 instruction) {
    SET_U3R8(4, L);
}
void set5L(GbState* s, u32 instruction) {
    SET_U3R8(5, L);
}
void set6L(GbState* s, u32 instruction) {
    SET_U3R8(6, L);
}
void set7L(GbState* s, u32 instruction) {
    SET_U3R8(7, L);
}

#define SET_U3aHL(u3) \
    u8 val = mem(HL) | (1 << u3);\
    mmu_write(s, HL, val)

void set0aHL(GbState* s, u32 instruction) { // debug(s, "setU3aHL");
    SET_U3aHL(0);
}
void set1aHL(GbState* s, u32 instruction) { // debug(s, "setU3aHL");
    SET_U3aHL(1);
}
void set2aHL(GbState* s, u32 instruction) { // debug(s, "setU3aHL");
    SET_U3aHL(2);
}
void set3aHL(GbState* s, u32 instruction) { // debug(s, "setU3aHL");
    SET_U3aHL(3);
}
void set4aHL(GbState* s, u32 instruction) { // debug(s, "setU3aHL");
    SET_U3aHL(4);
}
void set5aHL(GbState* s, u32 instruction) { // debug(s, "setU3aHL");
    SET_U3aHL(5);
}
void set6aHL(GbState* s, u32 instruction) { // debug(s, "setU3aHL");
    SET_U3aHL(6);
}
void set7aHL(GbState* s, u32 instruction) { // debug(s, "setU3aHL");
    SET_U3aHL(7);
}

void set0A(GbState* s, u32 instruction) {
    SET_U3R8(0, A);
}
void set1A(GbState* s, u32 instruction) {
    SET_U3R8(1, A);
}
void set2A(GbState* s, u32 instruction) {
    SET_U3R8(2, A);
}
void set3A(GbState* s, u32 instruction) {
    SET_U3R8(3, A);
}
void set4A(GbState* s, u32 instruction) {
    SET_U3R8(4, A);
}
void set5A(GbState* s, u32 instruction) {
    SET_U3R8(5, A);
}
void set6A(GbState* s, u32 instruction) {
    SET_U3R8(6, A);
}
void set7A(GbState* s, u32 instruction) {
    SET_U3R8(7, A);
}

void ext(GbState* s, u32 instruction) { // debug(s, "ext");
    PC++;
    u8 extInstruction = instruction >> 8;
    byte extOp = extInstruction & 0xFF;
    extendedOpTable[extOp](s, extInstruction);
}

#define LD_R8N8(r8) \
    r8 = IMM8;\
    PC++

void ldBN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    LD_R8N8(B);
}
void ldCN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    LD_R8N8(C);
}
void ldDN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    LD_R8N8(D);
}
void ldEN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    LD_R8N8(E);
}
void ldHN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    LD_R8N8(H);
}
void ldLN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    LD_R8N8(L);
}
void ldAN8(GbState* s, u32 instruction) { // debug(s, "ldR8N8");
    LD_R8N8(A);
}

void ldaHLN8(GbState* s, u32 instruction) { // debug(s, "ldaHLN8");
    u8 src = IMM8;
    PC++;
    mmu_write(s, HL, src);
}

#define LD_R8R8(r1, r2) r1 = r2

/** 
 * ld B, R8s
 */
/*
void ldBB(GbState* s, u32 instruction) { // debug(s, "ldAB");
    ;
}
*/
void ldBC(GbState* s, u32 instruction) { // debug(s, "ldAC");
    LD_R8R8(B, C);
}
void ldBD(GbState* s, u32 instruction) { // debug(s, "ldAD");
    LD_R8R8(B, D);
}
void ldBE(GbState* s, u32 instruction) { // debug(s, "ldAE");
    LD_R8R8(B, E);
}
void ldBH(GbState* s, u32 instruction) { // debug(s, "ldAH");
    LD_R8R8(B, H);
}
void ldBL(GbState* s, u32 instruction) { // debug(s, "ldAL");
    LD_R8R8(B, L);
}
void ldBA(GbState* s, u32 instruction) { // debug(s, "ldAB");
    LD_R8R8(B, A);
}

/**
 * ld C, R8s
 */
void ldCB(GbState* s, u32 instruction) { // debug(s, "ldCB");
    LD_R8R8(C, B);
}
/*
void ldCC(GbState* s, u32 instruction) { // debug(s, "ldCC");
    ;
}
*/
void ldCD(GbState* s, u32 instruction) { // debug(s, "ldCD");
    LD_R8R8(C, D);
}
void ldCE(GbState* s, u32 instruction) { // debug(s, "ldCE");
    LD_R8R8(C, E);
}
void ldCH(GbState* s, u32 instruction) { // debug(s, "ldCH");
    LD_R8R8(C, H);
}
void ldCL(GbState* s, u32 instruction) { // debug(s, "ldCL");
    LD_R8R8(C, L);
}
void ldCA(GbState* s, u32 instruction) { // debug(s, "ldCA");
    LD_R8R8(C, A);
}

/**
 * ld C, R8s
 */
void ldDB(GbState* s, u32 instruction) { // debug(s, "ldDB");
    LD_R8R8(D, B);
}
void ldDC(GbState* s, u32 instruction) { // debug(s, "ldDC");
    LD_R8R8(D, C);
}
/*
void ldDD(GbState* s, u32 instruction) { // debug(s, "ldDD");
    D = D;
}
*/
void ldDE(GbState* s, u32 instruction) { // debug(s, "ldDE");
    LD_R8R8(D, E);
}
void ldDH(GbState* s, u32 instruction) { // debug(s, "ldDH");
    LD_R8R8(D, H);
}
void ldDL(GbState* s, u32 instruction) { // debug(s, "ldDL");
    LD_R8R8(D, L);
}
void ldDA(GbState* s, u32 instruction) { // debug(s, "ldDA");
    LD_R8R8(D, A);
}

/**
 * ld E, R8s
 */
void ldEB(GbState* s, u32 instruction) {
    LD_R8R8(E, B);
}
void ldEC(GbState* s, u32 instruction) {
    LD_R8R8(E, C);
}
void ldED(GbState* s, u32 instruction) {
    LD_R8R8(E, D);
}
/*
void ldEB(GbState* s, u32 instruction) {
    E = E;
}
*/
void ldEH(GbState* s, u32 instruction) {
    LD_R8R8(E, H);
}
void ldEL(GbState* s, u32 instruction) {
    LD_R8R8(E, L);
}
void ldEA(GbState* s, u32 instruction) {
    LD_R8R8(E, A);
}

/**
 * ld H, R8s
 */
void ldHB(GbState* s, u32 instruction) {
    LD_R8R8(H, B);
}
void ldHC(GbState* s, u32 instruction) {
    LD_R8R8(H, C);
}
void ldHD(GbState* s, u32 instruction) {
    LD_R8R8(H, D);
}
void ldHE(GbState* s, u32 instruction) {
    LD_R8R8(H, E);
}
/*
void ldHH(GbState* s, u32 instruction) {
    H = H;
}
*/
void ldHL(GbState* s, u32 instruction) {
    LD_R8R8(H, L);
}
void ldHA(GbState* s, u32 instruction) {
    LD_R8R8(H, A);
}

/**
 * ld L, R8s
 */
void ldLB(GbState* s, u32 instruction) {
    LD_R8R8(L, B);
}
void ldLC(GbState* s, u32 instruction) {
    LD_R8R8(L, C);
}
void ldLD(GbState* s, u32 instruction) {
    LD_R8R8(L, D);
}
void ldLE(GbState* s, u32 instruction) {
    LD_R8R8(L, E);
}
void ldLH(GbState* s, u32 instruction) {
    LD_R8R8(L, H);
}
/*
void ldLL(GbState* s, u32 instruction) {
    L = L;
}
*/
void ldLA(GbState* s, u32 instruction) {
    LD_R8R8(L, A);
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
    LD_R8R8(A, B);
}
void ldAC(GbState* s, u32 instruction) { // debug(s, "ldAC");
    LD_R8R8(A, C);
}
void ldAD(GbState* s, u32 instruction) { // debug(s, "ldAD");
    LD_R8R8(A, D);
}
void ldAE(GbState* s, u32 instruction) { // debug(s, "ldAE");
    LD_R8R8(A, E);
}
void ldAH(GbState* s, u32 instruction) { // debug(s, "ldAH");
    LD_R8R8(A, H);
}
void ldAL(GbState* s, u32 instruction) { // debug(s, "ldAL");
    LD_R8R8(A, L);
}

#define LD_R8aHL(r8) r8 = mem(HL)

void ldBaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    LD_R8aHL(B);
}
void ldCaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    LD_R8aHL(C);
}
void ldDaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    LD_R8aHL(D);
}
void ldEaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    LD_R8aHL(E);
}
void ldHaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    LD_R8aHL(H);
}
void ldLaHL(GbState* s, u32 instruction) {  // debug(s, "ldR8aHL");
    LD_R8aHL(L);
}

#define LD_aHLR8(r8) mmu_write(s, HL, r8)

void ldaHLB(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    LD_aHLR8(B);
}
void ldaHLC(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    LD_aHLR8(C);
}
void ldaHLD(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    LD_aHLR8(D);
}
void ldaHLE(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    LD_aHLR8(E);
}
void ldaHLH(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    LD_aHLR8(H);
}
void ldaHLL(GbState* s, u32 instruction) { // debug(s, "ldaHLR8");
    LD_aHLR8(L);
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
    F = 0;
    HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
    CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
    HL = (u16)res;
    PC++;
}

void ldaN16SP(GbState* s, u32 instruction) { // debug(s, "ldaN16SP");
    mmu_write16(s, IMM16, s->sp);
    PC += 2; 
}

#define PUSH_R16(r16) mmu_push16(s, r16) 

void pushBC(GbState* s, u32 instruction) { // debug(s, "pushR16");
    PUSH_R16(BC);
}
void pushDE(GbState* s, u32 instruction) { // debug(s, "pushR16");
    PUSH_R16(DE);
}
void pushHL(GbState* s, u32 instruction) { // debug(s, "pushR16");
    PUSH_R16(HL);
}
void pushAF(GbState* s, u32 instruction) { // debug(s, "pushR16");
    PUSH_R16(AF);
}

#define POP_R16(r16) r16 = mmu_pop16(s)

void popBC(GbState* s, u32 instruction) { // debug(s, "popR16");
    POP_R16(BC);
}
void popDE(GbState* s, u32 instruction) { // debug(s, "popR16");
    POP_R16(DE);
}
void popHL(GbState* s, u32 instruction) { // debug(s, "popR16");
    POP_R16(HL);
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

#define ADD_AR8(r8) \
    u16 res = A + r8;\
    ZF = (u8)res == 0;\
    NF = 0;\
    HF = (A ^ r8 ^ res) & 0x10 ? 1 : 0;\
    CF = res & 0x100 ? 1 : 0;\
    A = (u8)res

void addAB(GbState* s, u32 instruction) {
    ADD_AR8(B);
}
void addAC(GbState* s, u32 instruction) {
    ADD_AR8(C);
}
void addAD(GbState* s, u32 instruction) {
    ADD_AR8(D);
}
void addAE(GbState* s, u32 instruction) {
    ADD_AR8(E);
}
void addAH(GbState* s, u32 instruction) {
    ADD_AR8(H);
}
void addAL(GbState* s, u32 instruction) {
    ADD_AR8(L);
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

void addAA(GbState* s, u32 instruction) {
    u16 res = A + A;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ A ^ res) & 0x10 ? 1 : 0;
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

#define ADC_AR8(r8) \
    u16 res = A + r8 + CF;\
    ZF = (u8)res == 0;\
    NF = 0;\
    HF = (A ^ r8 ^ res) & 0x10 ? 1 : 0;\
    CF = res & 0x100 ? 1 : 0;\
    A = (u8)res

void adcAB(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    ADC_AR8(B);
}
void adcAC(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    ADC_AR8(C);
}
void adcAD(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    ADC_AR8(D);
}
void adcAE(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    ADC_AR8(E);
}
void adcAH(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    ADC_AR8(H);
}
void adcAL(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    ADC_AR8(L);
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

void adcAA(GbState* s, u32 instruction) { // debug(s, "adcAR8");
    u16 res = A + A + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ A ^ res) & 0x10 ? 1 : 0;
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

#define SUB_AR8(r8) \
    u8 res = A - r8;\
    ZF = res == 0;\
    NF = 1;\
    HF = ((s32)A & 0xf) - (r8 & 0xf) < 0;\
    CF = A < r8;\
    A = res

void subAB(GbState* s, u32 instruction) { // debug(s, "subAR8");
    SUB_AR8(B);
}
void subAC(GbState* s, u32 instruction) { // debug(s, "subAR8");
    SUB_AR8(C);
}
void subAD(GbState* s, u32 instruction) { // debug(s, "subAR8");
    SUB_AR8(D);
}
void subAE(GbState* s, u32 instruction) { // debug(s, "subAR8");
    SUB_AR8(E);
}
void subAH(GbState* s, u32 instruction) { // debug(s, "subAR8");
    SUB_AR8(H);
}
void subAL(GbState* s, u32 instruction) { // debug(s, "subAR8");
    SUB_AR8(L);
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

void subAA(GbState* s, u32 instruction) { // debug(s, "subAR8");
    F = FLAG_Z | FLAG_N;
    A = 0;
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

#define SBC_AR8(r8) \
    u8 res = A - r8 - CF;\
    ZF = res == 0;\
    NF = 1;\
    HF = ((s32)A & 0xf) - (r8 & 0xf) - CF < 0;\
    CF = A < r8 + CF;\
    A = res

void sbcAB(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    SBC_AR8(B);
}
void sbcAC(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    SBC_AR8(C);
}
void sbcAD(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    SBC_AR8(D);
}
void sbcAE(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    SBC_AR8(E);
}
void sbcAH(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    SBC_AR8(H);
}
void sbcAL(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    SBC_AR8(L);
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

void sbcAA(GbState* s, u32 instruction) { // debug(s, "sbcAR8");
    // TODO: Simplify this.
    u8 res = A - A - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (A & 0xf) - CF < 0;
    CF = A < A + CF;
    A = res;
}

void andAN8(GbState* s, u32 instruction) { // debug(s, "andAN8");
    A = A & IMM8;
    PC++;
    F = FLAG_H;
    ZF = A == 0;
}

#define AND_AR8(r8) \
    A = A & r8;\
    F = FLAG_H;\
    ZF = A == 0

void andAB(GbState* s, u32 instruction) { // debug(s, "andAR8");
    AND_AR8(B);
}
void andAC(GbState* s, u32 instruction) { // debug(s, "andAR8");
    AND_AR8(C);
}
void andAD(GbState* s, u32 instruction) { // debug(s, "andAR8");
    AND_AR8(D);
}
void andAE(GbState* s, u32 instruction) { // debug(s, "andAR8");
    AND_AR8(E);
}
void andAH(GbState* s, u32 instruction) { // debug(s, "andAR8");
    AND_AR8(H);
}
void andAL(GbState* s, u32 instruction) { // debug(s, "andAR8");
    AND_AR8(L);
}

void andAaHL(GbState* s, u32 instruction) { // debug(s, "andAaHL");
    u8 val = mem(HL);
    A = A & val;
    F = FLAG_H;
    ZF = A == 0;
}

void andAA(GbState* s, u32 instruction) { // debug(s, "andAR8");
    A = A & A;
    F = FLAG_H;
    ZF = A == 0;
}

void orAN8(GbState* s, u32 instruction) { // debug(s, "orAN8");
    A |= IMM8;
    F = A ? 0 : FLAG_Z;
    PC++;
}

#define OR_AR8(r8) \
    A |= r8;\
    F = A ? 0 : FLAG_Z

void orAB(GbState* s, u32 instruction) { // debug(s, "orAR8");
    OR_AR8(B);
}
void orAC(GbState* s, u32 instruction) { // debug(s, "orAR8");
    OR_AR8(C);
}
void orAD(GbState* s, u32 instruction) { // debug(s, "orAR8");
    OR_AR8(D);
}
void orAE(GbState* s, u32 instruction) { // debug(s, "orAR8");
    OR_AR8(E);
}
void orAH(GbState* s, u32 instruction) { // debug(s, "orAR8");
    OR_AR8(H);
}
void orAL(GbState* s, u32 instruction) { // debug(s, "orAR8");
    OR_AR8(L);
}

void orAaHL(GbState* s, u32 instruction) { // debug(s, "orAaHL");
    u8 srcval = mem(HL);
    A |= srcval;
    F = A ? 0 : FLAG_Z;
}

void orAA(GbState* s, u32 instruction) { // debug(s, "orAR8");
    A |= A;
    F = A ? 0 : FLAG_Z;
}

void xorAN8(GbState* s, u32 instruction) { // debug(s, "xorAN8");
    A ^= IMM8;
    PC++;
    F = A ? 0 : FLAG_Z;
}

#define XOR_AR8(r8) \
    A ^= r8;\
    F = A ? 0 : FLAG_Z

void xorAB(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    XOR_AR8(B);
}
void xorAC(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    XOR_AR8(C);
}
void xorAD(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    XOR_AR8(D);
}
void xorAE(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    XOR_AR8(E);
}
void xorAH(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    XOR_AR8(H);
}
void xorAL(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    XOR_AR8(L);
}

void xorAaHL(GbState* s, u32 instruction) { // debug(s, "xorAaHL");
    u8 srcval = mem(HL);
    A ^= srcval;
    F = A ? 0 : FLAG_Z;
}

void xorAA(GbState* s, u32 instruction) { // debug(s, "xorAR8");
    A = 0;
    F = FLAG_Z;
}

void cpAN8(GbState* s, u32 instruction) { // debug(s, "cpAN8");
    u8 n = IMM8;
    ZF = A == n;
    NF = 1;
    HF = (A & 0xf) < (n & 0xf);
    CF = A < n;
    PC++;
}

#define CP_AR8(r8) \
    ZF = A == r8;\
    NF = 1;\
    HF = (A & 0xf) < (r8 & 0xf);\
    CF = A < r8

void cpAB(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    CP_AR8(B);
}
void cpAC(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    CP_AR8(C);
}
void cpAD(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    CP_AR8(D);
}
void cpAE(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    CP_AR8(E);
}
void cpAH(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    CP_AR8(H);
}
void cpAL(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    CP_AR8(L);
}
void cpAaHL(GbState* s, u32 instruction) { // debug(s, "cpAaHL");
    u8 regval = mem(HL);
    ZF = A == regval;
    NF = 1;
    HF = (A & 0xf) < (regval & 0xf);
    CF = A < regval;
}
void cpAA(GbState* s, u32 instruction) { // debug(s, "cpAR8");
    F = FLAG_Z | FLAG_N;
}

#define INC_R8(r8) \
    HF = (r8 & 0xf) == 0xf;\
    r8++;\
    ZF = r8 == 0;\
    NF = 0

void incB(GbState* s, u32 instruction) {
    INC_R8(B);
}
void incC(GbState* s, u32 instruction) {
    INC_R8(C);
}
void incD(GbState* s, u32 instruction) {
    INC_R8(D);
}
void incE(GbState* s, u32 instruction) {
    INC_R8(E);
}
void incH(GbState* s, u32 instruction) {
    INC_R8(H);
}
void incL(GbState* s, u32 instruction) {
    INC_R8(L);
}

void incaHL(GbState* s, u32 instruction) { // debug(s, "incaHL");
    u8 val = mem(HL);
    u8 res = val + 1;
    ZF = res == 0;
    NF = 0;
    HF = (val & 0xf) == 0xf;
    mmu_write(s, HL, res);
}

void incA(GbState* s, u32 instruction) {
    INC_R8(A);
}


#define DEC_R8(r8) \
    r8--;\
    HF = (r8 & 0x0F) == 0x0F;\
    NF = 1;\
    ZF = r8 == 0

void decB(GbState* s, u32 instruction) { // debug(s, "decR8");
    DEC_R8(B);
}

void decC(GbState* s, u32 instruction) { // debug(s, "decR8");
    DEC_R8(C);
}

void decD(GbState* s, u32 instruction) { // debug(s, "decR8");
    DEC_R8(D);
}

void decE(GbState* s, u32 instruction) { // debug(s, "decR8");
    DEC_R8(E);
}

void decH(GbState* s, u32 instruction) { // debug(s, "decR8");
    DEC_R8(H);
}

void decL(GbState* s, u32 instruction) { // debug(s, "decR8");
    DEC_R8(L);
}

void decaHL(GbState* s, u32 instruction) { // debug(s, "decaHL");
    u8 val = mem(HL);
    val--;
    NF = 1;
    ZF = val == 0;
    HF = (val & 0x0F) == 0x0F;
    mmu_write(s, HL, val);
}

void decA(GbState* s, u32 instruction) { // debug(s, "decR8");
    DEC_R8(A);
}

#define INC_R16(r16) r16++

void incBC(GbState* s, u32 instruction) {
    INC_R16(BC);
}

void incDE(GbState* s, u32 instruction) {
    INC_R16(DE);
}

void incHL(GbState* s, u32 instruction) {
    INC_R16(HL);
}

void incSP(GbState* s, u32 instruction) {
    INC_R16(s->sp);
}

#define DEC_R16(r16) r16--

void decBC(GbState* s, u32 instruction) { // debug(s, "decR16");
    DEC_R16(BC);
}

void decDE(GbState* s, u32 instruction) { // debug(s, "decR16");
    DEC_R16(DE);
}

void decHL(GbState* s, u32 instruction) { // debug(s, "decR16");
    DEC_R16(HL);
}

void decSP(GbState* s, u32 instruction) { // debug(s, "decR16");
    DEC_R16(s->sp);
}

#define ADD_HLR16(r16) \
    HF = (((HL & 0xfff) + (r16 & 0xfff)) & 0x1000) ? 1 : 0;\
    u32 tmp = HL + r16;\
    NF = 0;\
    CF = tmp > 0xFFFF;\
    HL = tmp

void addHLBC(GbState* s, u32 instruction) { // debug(s, "addHLR16");
    ADD_HLR16(BC);
}
void addHLDE(GbState* s, u32 instruction) { // debug(s, "addHLR16");
    ADD_HLR16(DE);
}
void addHLHL(GbState* s, u32 instruction) { // debug(s, "addHLR16");
    HF = (((HL & 0xfff) + (HL & 0xfff)) & 0x1000) ? 1 : 0;
    u32 tmp = HL + HL;
    NF = 0;
    CF = tmp > 0xFFFF;
    HL = tmp;
}
void addHLSP(GbState* s, u32 instruction) { // debug(s, "addHLR16");
    ADD_HLR16(s->sp);
}

void addSPN8(GbState* s, u32 instruction) { // debug(s, "addSPN8");
    s8 off = (s8)IMM8;
    u32 res = s->sp + off;
    F = 0;
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
void rla(GbState* s, u32 instruction) { // debug(s, "rlA");
    u8 res = A << 1 | (CF ? 1 : 0);
    F = (A & (1 << 7)) ? FLAG_C : 0;
    A = res;
}
void rrca(GbState* s, u32 instruction) { // debug(s, "rrcA");
    F = (A & 1) ? FLAG_C : 0;
    A = (A >> 1) | ((A & 1) << 7);
}
void rra(GbState* s, u32 instruction) { // debug(s, "rrA");
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

#define RST_V(v) \
    mmu_push16(s, PC);\
    PC = v

void rst00(GbState* s, u32 instruction) { // debug(s, "rstVec");
    RST_V(0x00);
}
void rst10(GbState* s, u32 instruction) { // debug(s, "rstVec");
    RST_V(0x10);
}
void rst20(GbState* s, u32 instruction) { // debug(s, "rstVec");
    RST_V(0x20);
}
void rst30(GbState* s, u32 instruction) { // debug(s, "rstVec");
    RST_V(0x30);
}
void rst08(GbState* s, u32 instruction) { // debug(s, "rstVec");
    RST_V(0x08);
}
void rst18(GbState* s, u32 instruction) { // debug(s, "rstVec");
    RST_V(0x18);
}
void rst28(GbState* s, u32 instruction) { // debug(s, "rstVec");
    RST_V(0x28);
}
void rst38(GbState* s, u32 instruction) { // debug(s, "rstVec");
    RST_V(0x38);
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