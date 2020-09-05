#include "core.h"
#include "types.h"
#include "mmu.h"
#include "cpu.h"
#include "gbz80ops.h"
#include "logger.h"
#include "config.h"
#include "debug.h"

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

#ifdef IS_DEBUGGING
    #define DEBUG(opName) debug(s, opName);
#else
    #define DEBUG(opName)
#endif

static const u8 flagmasks[] = { FLAG_Z, FLAG_Z, FLAG_C, FLAG_C };



void rlcR8(GbState* s, byte op) { DEBUG("rlcR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    u8 res = (val << 1) | (val >> 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val >> 7;
    *reg = res; 
}
void rlcaHL(GbState* s, byte op) { DEBUG("rlcaHL");
    u8 val = mem(HL);
    u8 res = (val << 1) | (val >> 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val >> 7;
    mmu_write(s, HL, res);
}

void rrcR8(GbState* s, byte op) { DEBUG("rrcR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    u8 res = (val >> 1) | ((val & 1) << 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val & 1;
    *reg = res; 
}
void rrcaHL(GbState* s, byte op) { DEBUG("rrcaHL");
    u8 val = mem(HL);
    u8 res = (val >> 1) | ((val & 1) << 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val & 1;
    mmu_write(s, HL, res);
}

void rlR8(GbState* s, byte op) { DEBUG("rlR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    u8 res = (val << 1) | (CF ? 1 : 0);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val >> 7;
    *reg = res;
}
void rlaHL(GbState* s, byte op) { DEBUG("rlaHL");
    u8 val = mem(HL);
    u8 res = (val << 1) | (CF ? 1 : 0);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val >> 7;
    mmu_write(s, HL, res);    
}

void rrR8(GbState* s, byte op) { DEBUG("rrR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    u8 res = (val >> 1) | (CF << 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val & 0x1;
    *reg = res;
}
void rraHL(GbState* s, byte op) { DEBUG("rraHL");
    u8 val = mem(HL);
    u8 res = (val >> 1) | (CF << 7);
    ZF = res == 0;
    NF = 0;
    HF = 0;
    CF = val & 0x1;
    mmu_write(s, HL, res);    
}

void slaR8(GbState* s, byte op) { DEBUG("slaR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    CF = val >> 7;
    val = val << 1;
    ZF = val == 0;
    NF = 0;
    HF = 0;
    *reg = val;
}
void slaaHL(GbState* s, byte op) { DEBUG("slaaHL");
    u8 val = mem(HL);
    CF = val >> 7;
    val = val << 1;
    ZF = val == 0;
    NF = 0;
    HF = 0;
    mmu_write(s, HL, val);    
}

void sraR8(GbState* s, byte op) { DEBUG("sraR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    CF = val & 0x1;
    val = (val >> 1) | (val & (1<<7));
    ZF = val == 0;
    NF = 0;
    HF = 0;
    *reg = val;
}
void sraaHL(GbState* s, byte op) { DEBUG("sraaHL");
    u8 val = mem(HL);
    CF = val & 0x1;
    val = (val >> 1) | (val & (1<<7));
    ZF = val == 0;
    NF = 0;
    HF = 0;
    mmu_write(s, HL, val);    
}

void swapR8(GbState* s, byte op) { DEBUG("swapR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    u8 res = ((val << 4) & 0xf0) | ((val >> 4) & 0xf);
    F = res == 0 ? FLAG_Z : 0;
    *reg = res;
}
void swapaHL(GbState* s, byte op) { DEBUG("swapaHL");
    u8 val = mem(HL);
    u8 res = ((val << 4) & 0xf0) | ((val >> 4) & 0xf);
    F = res == 0 ? FLAG_Z : 0;
    mmu_write(s, HL, res);    
}

void srlR8(GbState* s, byte op) { DEBUG("srlR8");
    u8 *reg = REG8(0);
    u8 val = *reg;
    CF = val & 0x1;
    val = val >> 1;
    ZF = val == 0;
    NF = 0;
    HF = 0;
    *reg = val;
}
void srlaHL(GbState* s, byte op) { DEBUG("srlaHL");
    u8 val = mem(HL);
    CF = val & 0x1;
    val = val >> 1;
    ZF = val == 0;
    NF = 0;
    HF = 0;
    mmu_write(s, HL, val);    
}

void bitU3R8(GbState* s, byte op) { DEBUG("bitU3R8");
    u8 bit = (op >> 3) & 7;
    u8 val = *REG8(0);
    ZF = ((val >> bit) & 1) == 0;
    NF = 0;
    HF = 1;    
}
void bitU3aHL(GbState* s, byte op) { DEBUG("bitU3aHL");
    u8 bit = (op >> 3) & 7;
    u8 val = mem(HL);
    ZF = ((val >> bit) & 1) == 0;
    NF = 0;
    HF = 1;    
}

void resU3R8(GbState* s, byte op) { DEBUG("resU3R8");
    u8 bit = (op >> 3) & 7;
    u8 *reg = REG8(0);
    u8 val = *reg;
    val = val & ~(1<<bit);
    *reg = val;
}
void resU3aHL(GbState* s, byte op) { DEBUG("resU3aHL");
    u8 bit = (op >> 3) & 7;
    u8 val = mem(HL);
    val = val & ~(1<<bit);
    mmu_write(s, HL, val);    
}

void setU3R8(GbState* s, byte op) { DEBUG("setU3R8");
    u8 bit = (op >> 3) & 7;
    u8 *reg = REG8(0);
    u8 val = *reg;
    val |= (1 << bit);
    *reg = val;
}
void setU3aHL(GbState* s, byte op) { DEBUG("setU3aHL");
    u8 bit = (op >> 3) & 7;
    u8 val = mem(HL);
    val |= (1 << bit);
    mmu_write(s, HL, val);    
}

void ext(GbState* s, byte op) { DEBUG("ext");
    byte extOp = mmu_read(s, s->pc++);
    extendedOpTable[extOp](s, extOp);
}

void ldR8N8(GbState* s, byte op) { DEBUG("ldR8N8");
    u8* dst = REG8(3);
    u8 src = IMM8;
    s->pc++;
    *dst = src;
}
void ldaHLN8(GbState* s, byte op) { DEBUG("ldaHLN8");
    u8 src = IMM8;
    s->pc++;
    mmu_write(s, HL, src);    
}
void ldR8R8(GbState* s, byte op) { DEBUG("ldR8R8");
    u8* src = REG8(0);
    u8* dst = REG8(3);
    u8 srcval = *src;
    *dst = srcval;
}
void ldR8aHL(GbState* s, byte op) { DEBUG("ldR8aHL");
    u8* dst = REG8(3);
    u8 srcval = mem(HL);
    *dst = srcval;
}
void ldaHLR8(GbState* s, byte op) { DEBUG("ldaHLR8");
    u8* src = REG8(0);
    mmu_write(s, HL, *src);    
}

void ldaBCA(GbState* s, byte op) { DEBUG("ldaBCA");
    mmu_write(s, BC, A);
}
void ldaDEA(GbState* s, byte op) { DEBUG("ldaDEA");
    mmu_write(s, DE, A);
}
void ldaHLA(GbState* s, byte op) { DEBUG("ldaHLA");
    mmu_write(s, HL, A);
}
void ldAaBC(GbState* s, byte op) { DEBUG("ldAaBC");
    A = mem(BC);
}
void ldAaDE(GbState* s, byte op) { DEBUG("ldAaDE");
    A = mem(DE);
}
void ldAaHL(GbState* s, byte op) { DEBUG("ldAaHL");
    A = mem(HL);
}

void ldAaN16(GbState* s, byte op) { DEBUG("ldAaN16");
    A = mmu_read(s, IMM16);
    s->pc += 2;    
}

void ldaN16A(GbState* s, byte op) { DEBUG("ldaN16A");
    mmu_write(s, IMM16, A);
    s->pc += 2;
}
void ldAaC(GbState* s, byte op) { DEBUG("ldAaC");
    A = mmu_read(s, 0xff00 + C);
}
void ldaCA(GbState* s, byte op) { DEBUG("ldaCA");
    mmu_write(s, 0xff00 + C, A);    
}
void lddAaHL(GbState* s, byte op) { DEBUG("lddAaHL");
    A = mmu_read(s, HL);
    HL--;  
}
void lddaHLA(GbState* s, byte op) { DEBUG("lddaHLA");
    mmu_write(s, HL, A);
    HL--;    
}
void ldiaHLA(GbState* s, byte op) { DEBUG("ldiaHLA");
    mmu_write(s, HL, A);
    HL++;
}
void ldiAaHL(GbState* s, byte op) { DEBUG("ldiAaHL");
    A = mmu_read(s, HL);
    HL++;
}
void ldhAaN8(GbState* s, byte op) { DEBUG("ldhAaN8");
    A = mmu_read(s, 0xff00 + IMM8);
    s->pc++;
}
void ldhaN8A(GbState* s, byte op) { DEBUG("ldhaN8A");
    mmu_write(s, 0xff00 + IMM8, A);
    s->pc++;    
}
void ldR16N16(GbState* s, byte op) { DEBUG("ldR16N16");
    u16 *dst = REG16(4);
    *dst = IMM16;
    s->pc += 2;    
}
void ldSPHL(GbState* s, byte op) { DEBUG("ldSPHL");
    s->sp = HL;    
}
void ldHLSPN8(GbState* s, byte op) { DEBUG("ldHLSPN8");
    u32 res = (u32)s->sp + (s8)IMM8;
    ZF = 0;
    NF = 0;
    HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
    CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
    HL = (u16)res;
    s->pc++;    
}

void ldaN16SP(GbState* s, byte op) { DEBUG("ldaN16SP");
    mmu_write16(s, IMM16, s->sp);
    s->pc += 2; 
}
void pushR16(GbState* s, byte op) { DEBUG("pushR16");
    u16 *src = REG16S(4);
    mmu_push16(s,*src);    
}
void popR16(GbState* s, byte op) { DEBUG("popR16");
    u16 *dst = REG16S(4);
    *dst = mmu_pop16(s);
    F = F & 0xf0;    
}

void addAN8(GbState* s, byte op) { DEBUG("addAN8");
    u16 res = A + IMM8;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
    s->pc++;
}
void addAR8(GbState* s, byte op) { DEBUG("addAR8");
    u8 srcval = *REG8(0);
    u16 res = A + srcval;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;    
}
void addAaHL(GbState* s, byte op) { DEBUG("addAaHL"); 
    u8 srcval = mem(HL);
    u16 res = A + srcval;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;   
}

void adcAN8(GbState* s, byte op) { DEBUG("adcAN8");
    u16 res = A + IMM8 + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ IMM8 ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;
    s->pc++;
}
void adcAR8(GbState* s, byte op) { DEBUG("adcAR8");
    u8 srcval = *REG8(0);
    u16 res = A + srcval + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res;    
}
void adcAaHL(GbState* s, byte op) { DEBUG("adcAaHL");
    u8 srcval = mem(HL);
    u16 res = A + srcval + CF;
    ZF = (u8)res == 0;
    NF = 0;
    HF = (A ^ srcval ^ res) & 0x10 ? 1 : 0;
    CF = res & 0x100 ? 1 : 0;
    A = (u8)res; 
}

void subAN8(GbState* s, byte op) { DEBUG("subAN8");
    u8 res = A - IMM8;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (IMM8 & 0xf) < 0;
    CF = A < IMM8;
    A = res;
    s->pc++;
}
void subAR8(GbState* s, byte op) { DEBUG("subAR8");
    u8 val = *REG8(0);
    u8 res = A - val;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (val & 0xf) < 0;
    CF = A < val;
    A = res;
}
void subAaHL(GbState* s, byte op) { DEBUG("subAaHL");
    u8 val = mem(HL);
    u8 res = A - val;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (val & 0xf) < 0;
    CF = A < val;
    A = res;
}

void sbcAN8(GbState* s, byte op) { DEBUG("sbcAN8");
    u8 res = A - IMM8 - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (IMM8 & 0xf) - CF < 0;
    CF = A < IMM8 + CF;
    A = res;
    s->pc++;    
}
void sbcAR8(GbState* s, byte op) { DEBUG("sbcAR8");
    u8 regval = *REG8(0);
    u8 res = A - regval - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (regval & 0xf) - CF < 0;
    CF = A < regval + CF;
    A = res;    
}
void sbcAaHL(GbState* s, byte op) { DEBUG("sbcAaHL");
    u8 regval = mem(HL);
    u8 res = A - regval - CF;
    ZF = res == 0;
    NF = 1;
    HF = ((s32)A & 0xf) - (regval & 0xf) - CF < 0;
    CF = A < regval + CF;
    A = res;    
}

void andAN8(GbState* s, byte op) { DEBUG("andAN8");
    A = A & IMM8;
    s->pc++;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;
}
void andAR8(GbState* s, byte op) { DEBUG("andAR8");
    u8 val = *REG8(0);
    A = A & val;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;    
}
void andAaHL(GbState* s, byte op) { DEBUG("andAaHL");
    u8 val = mem(HL);
    A = A & val;
    ZF = A == 0;
    NF = 0;
    HF = 1;
    CF = 0;    
}

void orAN8(GbState* s, byte op) { DEBUG("orAN8");
    A |= IMM8;
    F = A ? 0 : FLAG_Z;
    s->pc++;    
}
void orAR8(GbState* s, byte op) { DEBUG("orAR8");
    u8 srcval = *REG8(0);
    A |= srcval;
    F = A ? 0 : FLAG_Z;    
}
void orAaHL(GbState* s, byte op) { DEBUG("orAaHL");
    u8 srcval = mem(HL);
    A |= srcval;
    F = A ? 0 : FLAG_Z;    
}

void xorAN8(GbState* s, byte op) { DEBUG("xorAN8");
    A ^= IMM8;
    s->pc++;
    F = A ? 0 : FLAG_Z;    
}
void xorAR8(GbState* s, byte op) { DEBUG("xorAR8");
    u8 srcval = *REG8(0);
    A ^= srcval;
    F = A ? 0 : FLAG_Z; 
}
void xorAaHL(GbState* s, byte op) { DEBUG("xorAaHL");
    u8 srcval = mem(HL);
    A ^= srcval;
    F = A ? 0 : FLAG_Z; 
}

void cpAN8(GbState* s, byte op) { DEBUG("cpAN8");
    u8 n = IMM8;
    ZF = A == n;
    NF = 1;
    HF = (A & 0xf) < (n & 0xf);
    CF = A < n;
    s->pc++;    
}
void cpAR8(GbState* s, byte op) { DEBUG("cpAR8");
    u8 regval = *REG8(0);
    ZF = A == regval;
    NF = 1;
    HF = (A & 0xf) < (regval & 0xf);
    CF = A < regval;
}
void cpAaHL(GbState* s, byte op) { DEBUG("cpAaHL");
    u8 regval = mem(HL);
    ZF = A == regval;
    NF = 1;
    HF = (A & 0xf) < (regval & 0xf);
    CF = A < regval;
}

void incR8(GbState* s, byte op) { DEBUG("incR8");
    u8* reg = REG8(3);
    u8 val = *reg;
    u8 res = val + 1;
    ZF = res == 0;
    NF = 0;
    HF = (val & 0xf) == 0xf;
    *reg = res;
}
void incaHL(GbState* s, byte op) { DEBUG("incaHL");
    u8 val = mem(HL);
    u8 res = val + 1;
    ZF = res == 0;
    NF = 0;
    HF = (val & 0xf) == 0xf;
    mmu_write(s, HL, res);    
}

void decR8(GbState* s, byte op) { DEBUG("decR8");
    u8* reg = REG8(3);
    u8 val = *reg;
    val--;
    NF = 1;
    ZF = val == 0;
    HF = (val & 0x0F) == 0x0F;
    *reg = val;
}
void decaHL(GbState* s, byte op) { DEBUG("decaHL");
    u8 val = mem(HL);
    val--;
    NF = 1;
    ZF = val == 0;
    HF = (val & 0x0F) == 0x0F;
    mmu_write(s, HL, val);
}

void incR16(GbState* s, byte op) { DEBUG("incR16");
    u16 *reg = REG16(4);
    *reg += 1;    
}
void decR16(GbState* s, byte op) { DEBUG("decR16");
    u16 *reg = REG16(4);
    *reg -= 1;
}
void addHLR16(GbState* s, byte op) { DEBUG("addHLR16");
    u16 *src = REG16(4);
    u32 tmp = HL + *src;
    NF = 0;
    HF = (((HL & 0xfff) + (*src & 0xfff)) & 0x1000) ? 1 : 0;
    CF = tmp > 0xffff;
    HL = tmp;   
}
void addSPN8(GbState* s, byte op) { DEBUG("addSPN8");
    s8 off = (s8)IMM8;
    u32 res = s->sp + off;
    ZF = 0;
    NF = 0;
    HF = (s->sp & 0xf) + (IMM8 & 0xf) > 0xf;
    CF = (s->sp & 0xff) + (IMM8 & 0xff) > 0xff;
    s->sp = res;
    s->pc++;
}
void daa(GbState* s, byte op) { DEBUG("daa");
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
void cpl(GbState* s, byte op) { DEBUG("cpl");
    A = ~A;
    NF = 1;
    HF = 1;
}
void ccf(GbState* s, byte op) { DEBUG("ccf");
    CF = CF ? 0 : 1;
    NF = 0;
    HF = 0;
}
void scf(GbState* s, byte op) { DEBUG("scf");
    NF = 0;
    HF = 0;
    CF = 1;
}
void rlcA(GbState* s, byte op) { DEBUG("rlcA");
    u8 res = (A << 1) | (A >> 7);
    F = (A >> 7) ? FLAG_C : 0;
    A = res;    
}
void rlA(GbState* s, byte op) { DEBUG("rlA");
    u8 res = A << 1 | (CF ? 1 : 0);
    F = (A & (1 << 7)) ? FLAG_C : 0;
    A = res;    
}
void rrcA(GbState* s, byte op) { DEBUG("rrcA");
    F = (A & 1) ? FLAG_C : 0;
    A = (A >> 1) | ((A & 1) << 7);    
}
void rrA(GbState* s, byte op) { DEBUG("rrA");
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
void stop(GbState* s, byte op) { DEBUG("stop");
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
void di(GbState* s, byte op) { DEBUG("di");
    s->interrupts_master_enabled = 0;
}
void ei(GbState* s, byte op) { DEBUG("ei");
    s->interrupts_master_enabled = 1;
}
void jpN16(GbState* s, byte op) { DEBUG("jpN16");
    s->pc = IMM16;    
}
void jpCCN16(GbState* s, byte op) { DEBUG("jpCCN16");
    u8 flag = (op >> 3) & 3;
    if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
        s->pc = IMM16;
    else
        s->pc += 2;
    return;    
}
void jpHL(GbState* s, byte op) { DEBUG("jpHL");
    s->pc = HL;    
}
void jrN8(GbState* s, byte op) { DEBUG("jrN8");
    s->pc += (s8)IMM8 + 1;    
}
void jrCCN8(GbState* s, byte op) { DEBUG("jrCCN8");
    u8 flag = (op >> 3) & 3;

    if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1)) {
        s->pc += (s8)IMM8;
    }
    s->pc++;    
}
void callN16(GbState* s, byte op) { DEBUG("callN16");
    u16 dst = IMM16;
    mmu_push16(s, s->pc + 2);
    s->pc = dst;
}
void callCCN16(GbState* s, byte op) { DEBUG("callCCN16");
    u16 dst = IMM16;
    s->pc += 2;
    u8 flag = (op >> 3) & 3;
    if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1)) {
        mmu_push16(s, s->pc);
        s->pc = dst;
    }
}
void rstVec(GbState* s, byte op) {  DEBUG("rstVec");
    mmu_push16(s, s->pc);
    s->pc = ((op >> 3) & 7) * 8;
}
void retCC(GbState* s, byte op) { DEBUG("retCC");
    u8 flag = (op >> 3) & 3;
    if (((F & flagmasks[flag]) ? 1 : 0) == (flag & 1))
        s->pc = mmu_pop16(s);
}
void reti(GbState* s, byte op) { DEBUG("reti");
    s->pc = mmu_pop16(s);
    s->interrupts_master_enabled = 1;
}
void ret(GbState* s, byte op) { DEBUG("ret");
    s->pc = mmu_pop16(s);
}

/**
 * An undefined op code should cause the cpu to hang.
 */
void undefined(GbState* s, byte op) { DEBUG("undefined");
    logAndPauseFrame(0, "UNDEFINED");
}