#include "../../core.h"
#include "types.h"
#include "mmu.h"
#include "cpu.h"

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
#define M(op, value, mask) (((op) & (mask)) == (value))
#define mem(loc) (mmu_read(s, loc))
#define IMM8  (mmu_read(s, s->pc))
#define IMM16 (mmu_read(s, s->pc) | (mmu_read(s, s->pc + 1) << 8))
#define REG8(bitpos) s->emu_cpu_state->reg8_lut[(op >> bitpos) & 7]
#define REG16(bitpos) s->emu_cpu_state->reg16_lut[((op >> bitpos) & 3)]
#define REG16S(bitpos) s->emu_cpu_state->reg16s_lut[((op >> bitpos) & 3)]
#define FLAG(bitpos) ((op >> bitpos) & 3)

static const u8 flagmasks[] = { FLAG_Z, FLAG_Z, FLAG_C, FLAG_C };

void rlcR8(GbState* s, byte op) {
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val << 1) | (val >> 7);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val >> 7;
        if (reg)
            *reg = res; 
        else
            mmu_write(s, HL, res);    
}
void rrcR8(GbState* s, byte op) {
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val >> 1) | ((val & 1) << 7);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val & 1;
        if (reg)
            *reg = res; 
        else
            mmu_write(s, HL, res);
}
void rlR8(GbState* s, byte op) {
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val << 1) | (CF ? 1 : 0);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val >> 7;
        if (reg)
            *reg = res;
        else 
            mmu_write(s, HL, res);    
}
void rrR8(GbState* s, byte op) {
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = (val >> 1) | (CF << 7);
        ZF = res == 0;
        NF = 0;
        HF = 0;
        CF = val & 0x1;
        if (reg)
            *reg = res;
        else
            mmu_write(s, HL, res);    
}
void slaR8(GbState* s, byte op) {
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        CF = val >> 7;
        val = val << 1;
        ZF = val == 0;
        NF = 0;
        HF = 0;
        if (reg)
            *reg = val;
        else
            mmu_write(s, HL, val);    
}
void sraR8(GbState* s, byte op) {
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        CF = val & 0x1;
        val = (val >> 1) | (val & (1<<7));
        ZF = val == 0;
        NF = 0;
        HF = 0;
        if (reg)
            *reg = val;
        else
            mmu_write(s, HL, val);    
}
void swapR8(GbState* s, byte op) {
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        u8 res = ((val << 4) & 0xf0) | ((val >> 4) & 0xf);
        F = res == 0 ? FLAG_Z : 0;
        if (reg)
            *reg = res;
        else
            mmu_write(s, HL, res);    
}
void srlR8(GbState* s, byte op) {
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        CF = val & 0x1;
        val = val >> 1;
        ZF = val == 0;
        NF = 0;
        HF = 0;
        if (reg)
            *reg = val;
        else
            mmu_write(s, HL, val);    
}
void bitU3R8(GbState* s, byte op) {
    u8 bit = (op >> 3) & 7;
    u8 *reg = REG8(0);
    u8 val = reg ? *reg : mem(HL);
    ZF = ((val >> bit) & 1) == 0;
    NF = 0;
    HF = 1;    
}
void resU3R8(GbState* s, byte op) {
    u8 bit = (op >> 3) & 7;
    u8 *reg = REG8(0);
    u8 val = reg ? *reg : mem(HL);
    val = val & ~(1<<bit);
    if (reg)
        *reg = val;
    else
        mmu_write(s, HL, val);    
}
void setU3R8(GbState* s, byte op) {
        u8 bit = (op >> 3) & 7;
        u8 *reg = REG8(0);
        u8 val = reg ? *reg : mem(HL);
        val |= (1 << bit);
        if (reg)
            *reg = val;
        else
            mmu_write(s, HL, val);    
}
void ext(GbState* s, byte op) {
    cpu_do_cb_instruction(s);
}
void ldR8N8(GbState* s, byte op) {
    u8* dst = REG8(3);
    u8 src = IMM8;
    s->pc++;
    if (dst)
        *dst = src;
    else
        mmu_write(s, HL, src);    
}
void ldR8R8(GbState* s, byte op) {
    u8* src = REG8(0);
    u8* dst = REG8(3);
    u8 srcval = src ? *src : mem(HL);
    if (dst)
        *dst = srcval;
    else
        mmu_write(s, HL, srcval);    
}

void ldaBCA(GbState* s, byte op) {
    mmu_write(s, BC, A);
}
void ldaDEA(GbState* s, byte op) {
    mmu_write(s, DE, A);
}
void ldaHLA(GbState* s, byte op) {
    mmu_write(s, HL, A);
}
void ldAaBC(GbState* s, byte op) {
    A = mem(BC);
}
void ldAaDE(GbState* s, byte op) {
    A = mem(DE);
}
void ldAaHL(GbState* s, byte op) {
    A = mem(HL);
}

void ldAaN16(GbState* s, byte op) {
    A = mmu_read(s, IMM16);
    s->pc += 2;    
}
void ldaR16A(GbState* s, byte op) {}
void ldaN16A(GbState* s, byte op) {
    mmu_write(s, IMM16, A);
    s->pc += 2;
}
void ldAaC(GbState* s, byte op) {
    A = mmu_read(s, 0xff00 + C);
}
void ldaCA(GbState* s, byte op) {
    mmu_write(s, 0xff00 + C, A);    
}
void lddAaHL(GbState* s, byte op) {
    A = mmu_read(s, HL);
    HL--;  
}
void lddaHLA(GbState* s, byte op) {
    mmu_write(s, HL, A);
    HL--;    
}
void ldiaHLA(GbState* s, byte op) {
    A = mmu_read(s, HL);
    HL++;    
}
void ldiAaHL(GbState* s, byte op) {
    mmu_write(s, HL, A);
    HL++;
}
void ldhAaN8(GbState* s, byte op) {
    A = mmu_read(s, 0xff00 + IMM8);
    s->pc++;
}
void ldhaN8A(GbState* s, byte op) {
    mmu_write(s, 0xff00 + IMM8, A);
    s->pc++;    
}
void ldR16N16(GbState* s, byte op) {
    u16 *dst = REG16(4);
    *dst = IMM16;
    s->pc += 2;    
}
void ldSPHL(GbState* s, byte op) {
    s->sp = HL;    
}
void ldHLSPN8(GbState* s, byte op) {
    u32 res = (u32)s->sp + (s8)IMM8;
    ZF = 0;
    NF = 0;
    HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
    CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
    HL = (u16)res;
    s->pc++;    
}

void ldaN16SP(GbState* s, byte op) {
    mmu_write16(s, IMM16, s->sp);
    s->pc += 2; 
}
void pushR16(GbState* s, byte op) {
    u16 *src = REG16S(4);
    mmu_push16(s,*src);    
}
void popR16(GbState* s, byte op) {
    u16 *dst = REG16S(4);
    *dst = mmu_pop16(s);
    F = F & 0xf0;    
}
void addAR8(GbState* s, byte op) {
    u8* src = REG8(0);
    u8 srcval = src ? *src : mem(HL);
    u16 res = A + srcval;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;    
}
void addAN8(GbState* s, byte op) {
    u16 res = A + IMM8;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
    s->pc++;
}
void adcAR8(GbState* s, byte op) {
    u8* src = REG8(0);
    u8 srcval = src ? *src : mem(HL);
    u16 res = A + srcval + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;    
}
void adcAN8(GbState* s, byte op) {
        u16 res = A + IMM8 + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
    s->pc++;
}
void subAR8(GbState* s, byte op) {
        u8 *reg = REG8(0);
    u8 val = reg ? *reg : mem(HL);
    u8 res = A - val;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (val & 0xf) < 0;
    CF = A < val;
    A = res;
}
void subAN8(GbState* s, byte op) {
    u8 res = A - IMM8;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (IMM8 & 0xf) < 0;
    CF = A < IMM8;
    A = res;
    s->pc++;    
}
void sbcAR8(GbState* s, byte op) {
    u8 *reg = REG8(0);
    u8 regval = reg ? *reg : mem(HL);
    u8 res = A - regval - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (regval & 0xf) - CF < 0;
    CF = A < regval + CF;
    A = res;    
}
void sbcAN8(GbState* s, byte op) {
    u8 res = A - IMM8 - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (IMM8 & 0xf) - CF < 0;
    CF = A < IMM8 + CF;
    A = res;
    s->pc++;    
}
void andAR8(GbState* s, byte op) {
    u8 *reg = REG8(0);
    u8 val = reg ? *reg : mem(HL);
    A = A & val;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;    
}
void andAN8(GbState* s, byte op) {
    A = A & IMM8;
    s->pc++;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;    
}
void orAR8(GbState* s, byte op) {
    u8* src = REG8(0);
    u8 srcval = src ? *src : mem(HL);
    A |= srcval;
    F = A ? 0 : FLAG_Z;    
}
void orAN8(GbState* s, byte op) {
    A |= IMM8;
    F = A ? 0 : FLAG_Z;
    s->pc++;    
}
void xorAR8(GbState* s, byte op) {
    u8* src = REG8(0);
    u8 srcval = src ? *src : mem(HL);
    A ^= srcval;
    F = A ? 0 : FLAG_Z; 
}
void xorAN8(GbState* s, byte op) {
    A ^= IMM8;
    s->pc++;
    F = A ? 0 : FLAG_Z;    
}
void cpAR8(GbState* s, byte op) {
        u8 *reg = REG8(0);
    u8 regval = reg ? *reg : mem(HL);

    ZF = A == regval;
    NF = 1;
    HF = (A & 0xf) < (regval & 0xf);
    CF = A < regval;
}
void cpAN8(GbState* s, byte op) {
    u8 n = IMM8;

    ZF = A == n;
    NF = 1;
    HF = (A & 0xf) < (n & 0xf);
    CF = A < n;
    s->pc++;    
}
void incR8(GbState* s, byte op) {
    u8* reg = REG8(3);
    u8 val = reg ? *reg : mem(HL);
    u8 res = val + 1;
    ZF = res == 0;
    NF = 0;
    HF = (val & 0xf) == 0xf;
    if (reg)
        *reg = res;
    else
        mmu_write(s, HL, res);    
}
void decR8(GbState* s, byte op) {
    u8* reg = REG8(3);
    u8 val = reg ? *reg : mem(HL);
    val--;
    NF = 1;
    ZF = val == 0;
    HF = (val & 0x0F) == 0x0F;
    if (reg)
        *reg = val;
    else
        mmu_write(s, HL, val);
}
void incR16(GbState* s, byte op) {
    u16 *reg = REG16(4);
    *reg += 1;    
}
void decR16(GbState* s, byte op) {
    u16 *reg = REG16(4);
    *reg -= 1;
}
void addHLR16(GbState* s, byte op) {
    u16 *src = REG16(4);
    u32 tmp = HL + *src;
    NF = 0;
    HF = (((HL & 0xfff) + (*src & 0xfff)) & 0x1000) ? 1 : 0;
    CF = tmp > 0xffff;
    HL = tmp;   
}
void addSPN8(GbState* s, byte op) {
    s8 off = (s8)IMM8;
    u32 res = s->sp + off;
    ZF = 0;
    NF = 0;
    HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
    CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
    s->sp = res;
    s->pc++;
}
void daa(GbState* s, byte op) {
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
void cpl(GbState* s, byte op) {
    A = ~A;
    NF = 1;
    HF = 1;
}
void ccf(GbState* s, byte op) {
    CF = CF ? 0 : 1;
    NF = 0;
    HF = 0;
}
void scf(GbState* s, byte op) {
    NF = 0;
    HF = 0;
    CF = 1;
}
void rlcA(GbState* s, byte op) {
    u8 res = (A << 1) | (A >> 7);
    F = (A >> 7) ? FLAG_C : 0;
    A = res;    
}
void rlA(GbState* s, byte op) {
    u8 res = A << 1 | (CF ? 1 : 0);
    F = (A & (1 << 7)) ? FLAG_C : 0;
    A = res;    
}
void rrcA(GbState* s, byte op) {
    F = (A & 1) ? FLAG_C : 0;
    A = (A >> 1) | ((A & 1) << 7);    
}
void rrA(GbState* s, byte op) {
    u8 res = (A >> 1) | (CF << 7);
    ZF = 0;
    NF = 0;
    HF = 0;
    CF = A & 0x1;
    A = res;    
}
void halt(GbState* s, byte op){
    s->halt_for_interrupts = 1;    
}
void stop(GbState* s, byte op) {
    // TODO
    s->halt_for_interrupts = 1;     
}
void di(GbState* s, byte op) {
        s->interrupts_master_enabled = 0;
}
void ei(GbState* s, byte op) {
        s->interrupts_master_enabled = 1;
}
void jpN16(GbState* s, byte op) {
    s->pc = IMM16;    
}
void jpCCN16(GbState* s, byte op) {
    u8 flag = (op >> 3) & 3;
    if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
        s->pc = IMM16;
    else
        s->pc += 2;
    return;    
}
void jpaHL(GbState* s, byte op) {
    s->pc = HL;    
}
void jrN8(GbState* s, byte op) {
    s->pc += (s8)IMM8 + 1;    
}
void jrCCN8(GbState* s, byte op) {
    u8 flag = (op >> 3) & 3;

    if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1)) {
        s->pc += (s8)IMM8;
    }
    s->pc++;    
}
void callN16(GbState* s, byte op) {
    u16 dst = IMM16;
    mmu_push16(s, s->pc + 2);
    s->pc = dst;
}
void callCCN16(GbState* s, byte op) {
    u16 dst = IMM16;
    s->pc += 2;
    u8 flag = (op >> 3) & 3;
    if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1)) {
        mmu_push16(s, s->pc);
        s->pc = dst;
    }
}
void rstVec(GbState* s, byte op) {
    mmu_push16(s, s->pc);
    s->pc = ((op >> 3) & 7) * 8;
}
void retCC(GbState* s, byte op) {
    u8 flag = (op >> 3) & 3;
    if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
        s->pc = mmu_pop16(s);
}
void reti(GbState* s, byte op) {
    s->pc = mmu_pop16(s);
    s->interrupts_master_enabled = 1;    
}
void ret(GbState* s, byte op) {
    s->pc = mmu_pop16(s);    
}

/**
 * An undefined op code should cause the cpu to hang.
 */
void undefined(GbState* s, byte op) {}