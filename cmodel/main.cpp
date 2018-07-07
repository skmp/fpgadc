#include <iostream>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <assert.h>


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;


////

#define SYSRAM_SIZE (16 * 1024 * 1024)
#define VIDEORAM_SIZE (8 * 1024 * 1024)

#define SYSRAM_MASK (SYSRAM_SIZE - 1)
#define VIDEORAM_MASK (SYSRAM_SIZE - 1)

///

struct dreamcast_t;

struct sh4_opcodelistentry {
    void (* oph)(dreamcast_t*, u16);
    uint16_t mask;
    uint16_t key;
    const char* diss;
};

struct dreamcast_t {
    uint8_t sys_ram[SYSRAM_SIZE];
    uint8_t video_ram[VIDEORAM_SIZE];

    struct {
        u32 r[16];
        
        union {
            f32 fr[32];
            u32 fr_u32[32];
            u64 fr_u64[16];
        };

        union {
            f32 xf[32];
            u32 xf_u32[32];
            u64 xf_u64[16];
        };

        u32 pc;
        u32 sr_T;
        u32 sr;
        u32 macl;
        u32 mach;
        u32 fpul;
        u32 fpscr_PR;
        u32 fpscr_SZ;

    } ctx;

    uint32_t pixels[640 * 480];

    void (* OpPtr[0x10000])(dreamcast_t*, u16);
    sh4_opcodelistentry* OpDesc[0x10000];
} state;

//// mem fns

template<typename T>
bool readMem(dreamcast_t *dc, uint32_t addy, T& data) {
    if ((addy >= 0x8C000000 && addy < 0x8D000000) || addy >= 0x0C000000 && addy < 0x0D000000) {
        data = *reinterpret_cast<T*>(&dc->sys_ram[addy & SYSRAM_MASK]);
        return true;
    } else {
        // return dc->video_ram[addy & VIDEORAM_MASK];

        printf("readMem(%08X)\n", addy);

        return false;
    }
}

template<typename T>
bool writeMem(dreamcast_t *dc, uint32_t addy, T data) {
    if ((addy >= 0x8C000000 && addy < 0x8D000000) || addy >= 0x0C000000 && addy < 0x0D000000) {
        *reinterpret_cast<T*>(&dc->sys_ram[addy & SYSRAM_MASK]) = data;
        return true;
    } else if (addy >= 0xA5000000 && addy < 0xA5800000) {
        *reinterpret_cast<T*>(&dc->video_ram[addy & VIDEORAM_MASK]) = data;
        return true;
    } else {
        // return dc->video_ram[addy & VIDEORAM_MASK];

        printf("writeMem %08X\n", addy);

        return false;
    }
}



/////// oplist

#define Mask_n_m 0xF00F
#define Mask_n_m_imm4 0xF000
#define Mask_n 0xF0FF
#define Mask_none 0xFFFF
#define Mask_imm8 0xFF00
#define Mask_imm12 0xF000
#define Mask_n_imm8 0xF000
#define Mask_n_ml3bit 0xF08F
#define Mask_nh3bit 0xF1FF
#define Mask_nh2bit 0xF3FF

#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
#define GetImm4(str) ((str>>0) & 0xf)
#define GetImm8(str) ((str>>0) & 0xff)
#define GetSImm8(str) ((s8)((str>>0) & 0xff))
#define GetImm12(str) ((str>>0) & 0xfff)
#define GetSImm12(str) (((s16)((GetImm12(str))<<4))>>4)

#define sh4impl(x) void x(dreamcast_t* dc, u16 instr)

//Placeholder
sh4impl(iNotImplemented) {
    printf("%08X: %04x %s [iNotImplemented]\n", dc->ctx.pc, instr, dc->OpDesc[instr]->diss);
}

#define sh4op(x) void x(dreamcast_t* dc, u16 instr) { iNotImplemented(dc, instr); }


//stc SR,<REG_N>
sh4op(i0000_nnnn_0000_0010);
//stc GBR,<REG_N>
sh4op(i0000_nnnn_0001_0010);
//stc VBR,<REG_N>
sh4op(i0000_nnnn_0010_0010);
//stc SSR,<REG_N>
sh4op(i0000_nnnn_0011_0010);
 //stc SGR,<REG_N>
sh4op(i0000_nnnn_0011_1010);
//stc SPC,<REG_N>
sh4op(i0000_nnnn_0100_0010);
//stc R0_BANK,<REG_N>
sh4op(i0000_nnnn_1mmm_0010);
//braf <REG_N>
sh4op(i0000_nnnn_0010_0011);
//bsrf <REG_N>
sh4op(i0000_nnnn_0000_0011);
//movca.l R0, @<REG_N>
sh4op(i0000_nnnn_1100_0011);
//ocbi @<REG_N>
sh4op(i0000_nnnn_1001_0011);
//ocbp @<REG_N>
sh4op(i0000_nnnn_1010_0011);
//ocbwb @<REG_N>
sh4op(i0000_nnnn_1011_0011);
//pref @<REG_N>
sh4op(i0000_nnnn_1000_0011);
//mov.b <REG_M>,@(R0,<REG_N>)
sh4op(i0000_nnnn_mmmm_0100);
//mov.w <REG_M>,@(R0,<REG_N>)
sh4op(i0000_nnnn_mmmm_0101);
//mov.l <REG_M>,@(R0,<REG_N>)
sh4op(i0000_nnnn_mmmm_0110);
//mul.l <REG_M>,<REG_N>
sh4impl(i0000_nnnn_mmmm_0111)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);
    dc->ctx.macl = (u32)((((s32)dc->ctx.r[n]) * ((s32)dc->ctx.r[m])));
}
//clrmac
sh4op(i0000_0000_0010_1000);
//clrs
sh4op(i0000_0000_0100_1000);
//clrt
sh4op(i0000_0000_0000_1000);
//ldtlb
sh4op(i0000_0000_0011_1000);
//sets
sh4op(i0000_0000_0101_1000);
//sett
sh4op(i0000_0000_0001_1000);
//div0u
sh4op(i0000_0000_0001_1001);
//movt <REG_N>
sh4op(i0000_nnnn_0010_1001);
//nop
sh4impl(i0000_0000_0000_1001) {

}
//sts FPUL,<REG_N>
sh4impl(i0000_nnnn_0101_1010)
{
    u32 n = GetN(instr);
    dc->ctx.r[n] = dc->ctx.fpul;
}
//sts FPSCR,<REG_N>
sh4op(i0000_nnnn_0110_1010);
//stc GBR,<REG_N>
sh4op(i0000_nnnn_1111_1010);
//sts MACH,<REG_N>
sh4op(i0000_nnnn_0000_1010);
//sts MACL,<REG_N>
sh4impl(i0000_nnnn_0001_1010)
{
    u32 n = GetN(instr);
    dc->ctx.r[n]=dc->ctx.macl;
}

//sts PR,<REG_N>
sh4op(i0000_nnnn_0010_1010);
//rte
sh4op(i0000_0000_0010_1011);
//rts
sh4op(i0000_0000_0000_1011);
//sleep
sh4op(i0000_0000_0001_1011);
//mov.b @(R0,<REG_M>),<REG_N>
sh4op(i0000_nnnn_mmmm_1100);
//mov.w @(R0,<REG_M>),<REG_N>
sh4op(i0000_nnnn_mmmm_1101);
//mov.l @(R0,<REG_M>),<REG_N>
sh4op(i0000_nnnn_mmmm_1110);
//mac.l @<REG_M>+,@<REG_N>+
sh4op(i0000_nnnn_mmmm_1111);
//
// 1xxx

//mov.l <REG_M>,@(<disp>,<REG_N>)
sh4op(i0001_nnnn_mmmm_iiii);
//
//  2xxx

//mov.b <REG_M>,@<REG_N>
sh4impl(i0010_nnnn_mmmm_0000)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);

    writeMem<u8>(dc, dc->ctx.r[n], dc->ctx.r[m]);
}

// mov.w <REG_M>,@<REG_N>
sh4impl(i0010_nnnn_mmmm_0001)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);

    writeMem<u16>(dc, dc->ctx.r[n], dc->ctx.r[m]);
}

// mov.l <REG_M>,@<REG_N>
sh4impl(i0010_nnnn_mmmm_0010)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);

    writeMem<u32>(dc, dc->ctx.r[n], dc->ctx.r[m]);
}

// mov.b <REG_M>,@-<REG_N>
sh4op(i0010_nnnn_mmmm_0100);
//mov.w <REG_M>,@-<REG_N>
sh4op(i0010_nnnn_mmmm_0101);
//mov.l <REG_M>,@-<REG_N>
sh4op(i0010_nnnn_mmmm_0110);
// div0s <REG_M>,<REG_N>
sh4op(i0010_nnnn_mmmm_0111);
// tst <REG_M>,<REG_N>
sh4op(i0010_nnnn_mmmm_1000);
//and <REG_M>,<REG_N>
sh4impl(i0010_nnnn_mmmm_1001)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);
    dc->ctx.r[n] &= dc->ctx.r[m];
}
//xor <REG_M>,<REG_N>
sh4impl(i0010_nnnn_mmmm_1010)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);
    dc->ctx.r[n] ^= dc->ctx.r[m];
}
//or <REG_M>,<REG_N>
sh4op(i0010_nnnn_mmmm_1011);
//cmp/str <REG_M>,<REG_N>
sh4op(i0010_nnnn_mmmm_1100);
//xtrct <REG_M>,<REG_N>
sh4op(i0010_nnnn_mmmm_1101);
//mulu <REG_M>,<REG_N>
sh4op(i0010_nnnn_mmmm_1110);
//muls <REG_M>,<REG_N>
sh4op(i0010_nnnn_mmmm_1111);

//
// 3xxx
// cmp/eq <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_0000);
// cmp/hs <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_0010);
//cmp/ge <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_0011);
//div1 <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_0100);
//dmulu.l <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_0101);
// cmp/hi <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_0110);
//cmp/gt <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_0111);
// sub <REG_M>,<REG_N>
sh4impl(i0011_nnnn_mmmm_1000)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);
    
    dc->ctx.r[n] -= dc->ctx.r[m];
}
//subc <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_1010);
//subv <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_1011);
//add <REG_M>,<REG_N>
sh4impl(i0011_nnnn_mmmm_1100)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);
    dc->ctx.r[n] += dc->ctx.r[m];
}
//dmuls.l <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_1101);
//addc <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_1110);
// addv <REG_M>,<REG_N>
sh4op(i0011_nnnn_mmmm_1111);

//
// 4xxx
//sts.l FPUL,@-<REG_N>
sh4op(i0100_nnnn_0101_0010);
//sts.l FPSCR,@-<REG_N>
sh4op(i0100_nnnn_0110_0010);
//sts.l MACH,@-<REG_N>
sh4op(i0100_nnnn_0000_0010);
//sts.l MACL,@-<REG_N>
sh4op(i0100_nnnn_0001_0010);
//sts.l PR,@-<REG_N>
sh4op(i0100_nnnn_0010_0010);
 //sts.l DBR,@-<REG_N>
sh4op(i0100_nnnn_1111_0010);
//stc.l SR,@-<REG_N>
sh4op(i0100_nnnn_0000_0011);
//stc.l GBR,@-<REG_N>
sh4op(i0100_nnnn_0001_0011);
//stc.l VBR,@-<REG_N>
sh4op(i0100_nnnn_0010_0011);
//stc.l SSR,@-<REG_N>
sh4op(i0100_nnnn_0011_0011);
 //stc.l SGR,@-<REG_N>
sh4op(i0100_nnnn_0011_0010);
//stc.l SPC,@-<REG_N>
sh4op(i0100_nnnn_0100_0011);
//stc Rm_BANK,@-<REG_N>
sh4op(i0100_nnnn_1mmm_0011);
//lds.l @<REG_N>+,MACH
sh4op(i0100_nnnn_0000_0110);
//lds.l @<REG_N>+,MACL
sh4op(i0100_nnnn_0001_0110);
//lds.l @<REG_N>+,PR
sh4op(i0100_nnnn_0010_0110);
//lds.l @<REG_N>+,FPUL
sh4op(i0100_nnnn_0101_0110);
//lds.l @<REG_N>+,FPSCR
sh4op(i0100_nnnn_0110_0110);
//ldc.l @<REG_N>+,DBR
sh4op(i0100_nnnn_1111_0110);
//ldc.l @<REG_N>+,SR
sh4op(i0100_nnnn_0000_0111);
//ldc.l @<REG_N>+,GBR
sh4op(i0100_nnnn_0001_0111);
//ldc.l @<REG_N>+,VBR
sh4op(i0100_nnnn_0010_0111);
//ldc.l @<REG_N>+,SSR
sh4op(i0100_nnnn_0011_0111);
  //ldc.l @<REG_N>+,SGR
sh4op(i0100_nnnn_0011_0110);
//ldc.l @<REG_N>+,SPC
sh4op(i0100_nnnn_0100_0111);
//ldc.l @<REG_N>+,R0_BANK
sh4op(i0100_nnnn_1mmm_0111);
//lds <REG_N>,MACH
sh4op(i0100_nnnn_0000_1010);
//lds <REG_N>,MACL
sh4op(i0100_nnnn_0001_1010);
//lds <REG_N>,PR
sh4op(i0100_nnnn_0010_1010);
//lds <REG_N>,FPUL
sh4impl(i0100_nnnn_0101_1010)
{
    u32 n = GetN(instr);
    dc->ctx.fpul = dc->ctx.r[n];
}
//lds <REG_N>,FPSCR
sh4op(i0100_nnnn_0110_1010);
//ldc <REG_N>,GBR
sh4op(i0100_nnnn_1111_1010);
//ldc <REG_N>,SR
sh4op(i0100_nnnn_0000_1110);
//ldc <REG_N>,GBR
sh4op(i0100_nnnn_0001_1110);
//ldc <REG_N>,VBR
sh4op(i0100_nnnn_0010_1110);
//ldc <REG_N>,SSR
sh4op(i0100_nnnn_0011_1110);
 //ldc <REG_N>,SGR
sh4op(i0100_nnnn_0011_1010);
//ldc <REG_N>,SPC
sh4op(i0100_nnnn_0100_1110);
//ldc <REG_N>,R0_BANK
sh4op(i0100_nnnn_1mmm_1110);
//shll <REG_N>
sh4op(i0100_nnnn_0000_0000);
//4210
//dt <REG_N>
sh4impl(i0100_nnnn_0001_0000)
{
    u32 n = GetN(instr);
    dc->ctx.r[n]-=1;
    if (dc->ctx.r[n] == 0)
        dc->ctx.sr_T=1;
    else
        dc->ctx.sr_T=0;
}
//shal <REG_N>
sh4op(i0100_nnnn_0010_0000);
//shlr <REG_N>
sh4impl(i0100_nnnn_0000_0001)
{
    u32 n = GetN(instr);
    dc->ctx.sr_T = dc->ctx.r[n] & 0x1;
    dc->ctx.r[n] >>= 1;
}

//cmp/pz <REG_N>
sh4op(i0100_nnnn_0001_0001);
//shar <REG_N>
sh4op(i0100_nnnn_0010_0001);
//rotcl <REG_N>
sh4op(i0100_nnnn_0010_0100);
//rotl <REG_N>
sh4op(i0100_nnnn_0000_0100);
//cmp/pl <REG_N>
sh4op(i0100_nnnn_0001_0101);
//rotcr <REG_N>
sh4op(i0100_nnnn_0010_0101);
//rotr <REG_N>
sh4op(i0100_nnnn_0000_0101);
//shll2 <REG_N>
sh4op(i0100_nnnn_0000_1000);
//shll8 <REG_N>
sh4impl(i0100_nnnn_0001_1000)
{
    u32 n = GetN(instr);
    dc->ctx.r[n] <<= 8;
}
//shll16 <REG_N>
sh4op(i0100_nnnn_0010_1000);
//shlr2 <REG_N>
sh4impl(i0100_nnnn_0000_1001)
{
    u32 n = GetN(instr);
    dc->ctx.r[n] >>= 2;
}
//shlr8 <REG_N>
sh4op(i0100_nnnn_0001_1001);
//shlr16 <REG_N>
sh4impl(i0100_nnnn_0010_1001)
{
    u32 n = GetN(instr);
    dc->ctx.r[n] >>= 16;
}
//jmp @<REG_N>
sh4op(i0100_nnnn_0010_1011);
//jsr @<REG_N>
sh4op(i0100_nnnn_0000_1011);
//tas.b @<REG_N>
sh4op(i0100_nnnn_0001_1011);
//shad <REG_M>,<REG_N>
sh4op(i0100_nnnn_mmmm_1100);
//shld <REG_M>,<REG_N>
sh4op(i0100_nnnn_mmmm_1101);
//mac.w @<REG_M>+,@<REG_N>+
sh4op(i0100_nnnn_mmmm_1111);

//
// 5xxx

//mov.l @(<disp>,<REG_M>),<REG_N>
sh4op(i0101_nnnn_mmmm_iiii);
//
// 6xxx
//mov.b @<REG_M>,<REG_N>
sh4impl(i0110_nnnn_mmmm_0000)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);

    s8 data;
    readMem<s8>(dc, dc->ctx.r[m], data);
    dc->ctx.r[n] = (s32)data;
}
//mov.w @<REG_M>,<REG_N>
sh4op(i0110_nnnn_mmmm_0001);
//mov.l @<REG_M>,<REG_N>
sh4op(i0110_nnnn_mmmm_0010);

//mov <REG_M>,<REG_N>
sh4impl(i0110_nnnn_mmmm_0011)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);
    dc->ctx.r[n] = dc->ctx.r[m];
}

//mov.b @<REG_M>+,<REG_N>
sh4op(i0110_nnnn_mmmm_0100);
//mov.w @<REG_M>+,<REG_N>
sh4op(i0110_nnnn_mmmm_0101);
//mov.l @<REG_M>+,<REG_N>
sh4op(i0110_nnnn_mmmm_0110);
//not <REG_M>,<REG_N>
sh4op(i0110_nnnn_mmmm_0111);
//swap.b <REG_M>,<REG_N>
sh4op(i0110_nnnn_mmmm_1000);
//swap.w <REG_M>,<REG_N>
sh4op(i0110_nnnn_mmmm_1001);
//negc <REG_M>,<REG_N>
sh4op(i0110_nnnn_mmmm_1010);
//neg <REG_M>,<REG_N>
sh4impl(i0110_nnnn_mmmm_1011)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);
    dc->ctx.r[n] = -dc->ctx.r[m];
}
//extu.b <REG_M>,<REG_N>
sh4impl(i0110_nnnn_mmmm_1100)
{
    u32 n = GetN(instr);
    u32 m = GetM(instr);
    dc->ctx.r[n] = (u32)(u8)dc->ctx.r[m];
}

//extu.w <REG_M>,<REG_N>
sh4op(i0110_nnnn_mmmm_1101);
//exts.b <REG_M>,<REG_N>
sh4op(i0110_nnnn_mmmm_1110);
//exts.w <REG_M>,<REG_N>
sh4op(i0110_nnnn_mmmm_1111);
//
// 7xxx
//add #<imm>,<REG_N>
sh4impl(i0111_nnnn_iiii_iiii)
{
    u32 n = GetN(instr);
    s32 stmp1 = GetSImm8(instr);
    dc->ctx.r[n] +=stmp1;
}

//
// 8xxx

u32 branch_target_s8(u32 op, u32 pc)
{
    return GetSImm8(op)*2 + 2 + pc;
}

u32 branch_target_s12(u32 op, u32 pc)
{
    return GetSImm12(op)*2 + 2 + pc;
}

void ExecuteDelayslot(dreamcast_t* dc)
{
    u32 addr = dc->ctx.pc;
    dc->ctx.pc += 2;
    
    u16 instr;
    readMem(dc, addr, instr);
    
    if (instr != 0) {
        dc->OpPtr[instr](dc, instr);
    }
}

// bf <bdisp8>
sh4impl(i1000_1011_iiii_iiii)
{
    if (dc->ctx.sr_T==0)
    {
        //direct jump
        dc->ctx.pc = branch_target_s8(instr, dc->ctx.pc);
    }
}
// bf.s <bdisp8>
sh4impl(i1000_1111_iiii_iiii)
{
    if (dc->ctx.sr_T==0)
    {
        //delay 1 instruction
        u32 newpc = branch_target_s8(instr, dc->ctx.pc);
        ExecuteDelayslot(dc);
        dc->ctx.pc = newpc;
    }
}
// bt <bdisp8>
sh4op(i1000_1001_iiii_iiii);
// bt.s <bdisp8>
sh4op(i1000_1101_iiii_iiii);
// cmp/eq #<imm>,R0
sh4op(i1000_1000_iiii_iiii);
// mov.b R0,@(<disp>,<REG_M>)
sh4op(i1000_0000_mmmm_iiii);
// mov.w R0,@(<disp>,<REG_M>)
sh4op(i1000_0001_mmmm_iiii);
// mov.b @(<disp>,<REG_M>),R0
sh4op(i1000_0100_mmmm_iiii);
// mov.w @(<disp>,<REG_M>),R0
sh4op(i1000_0101_mmmm_iiii);

//
// 9xxx

//mov.w @(<disp>,PC),<REG_N>
sh4op(i1001_nnnn_iiii_iiii);
//
// Axxx
// bra <bdisp12>
sh4impl(i1010_iiii_iiii_iiii)
{
    u32 newpc = branch_target_s12(instr, dc->ctx.pc);
    ExecuteDelayslot(dc);
    dc->ctx.pc = newpc;
}
//
// Bxxx
// bsr <bdisp12>
sh4op(i1011_iiii_iiii_iiii);
//
// Cxxx
// mov.b R0,@(<disp>,GBR)
sh4op(i1100_0000_iiii_iiii);
// mov.w R0,@(<disp>,GBR)
sh4op(i1100_0001_iiii_iiii);
// mov.l R0,@(<disp>,GBR)
sh4op(i1100_0010_iiii_iiii);
// trapa #<imm>
sh4op(i1100_0011_iiii_iiii);
// mov.b @(<disp>,GBR),R0
sh4op(i1100_0100_iiii_iiii);
// mov.w @(<disp>,GBR),R0
sh4op(i1100_0101_iiii_iiii);
// mov.l @(<disp>,GBR),R0
sh4op(i1100_0110_iiii_iiii);
// mova @(<disp>,PC),R0
sh4impl(i1100_0111_iiii_iiii)
{
    dc->ctx.r[0] = ((dc->ctx.pc+2)&0xFFFFFFFC)+(GetImm8(instr)<<2);
}

// tst #<imm>,R0
sh4op(i1100_1000_iiii_iiii);
// and #<imm>,R0
sh4op(i1100_1001_iiii_iiii);
// xor #<imm>,R0
sh4op(i1100_1010_iiii_iiii);
// or #<imm>,R0
sh4op(i1100_1011_iiii_iiii);
// tst.b #<imm>,@(R0,GBR)
sh4op(i1100_1100_iiii_iiii);
// and.b #<imm>,@(R0,GBR)
sh4op(i1100_1101_iiii_iiii);
// xor.b #<imm>,@(R0,GBR)
sh4op(i1100_1110_iiii_iiii);
// or.b #<imm>,@(R0,GBR)
sh4op(i1100_1111_iiii_iiii);
//
// Dxxx

// mov.l @(<disp>,PC),<REG_N>
sh4impl(i1101_nnnn_iiii_iiii)
{
    u32 n = GetN(instr);
    u32 disp = GetImm8(instr);

    readMem<u32>(dc, (disp<<2) + ((dc->ctx.pc+2) & 0xFFFFFFFC), dc->ctx.r[n]);
}

//
// Exxx

// mov #<imm>,<REG_N>
sh4impl(i1110_nnnn_iiii_iiii)
{
    u32 n = GetN(instr);
    dc->ctx.r[n] = (u32)(s32)(s8)(GetSImm8(instr));
}

// Fxxx

//fadd <FREG_M>,<FREG_N>
sh4impl(i1111_nnnn_mmmm_0000)
{
    if (dc->ctx.fpscr_PR == 0)
    {
        u32 n = GetN(instr);
        u32 m = GetM(instr);
        dc->ctx.fr[n] += dc->ctx.fr[m];
    }
    else
    {
        assert(false);
    }
}
//fsub <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0001);
//fmul <FREG_M>,<FREG_N>
sh4impl(i1111_nnnn_mmmm_0010)
{
    if (dc->ctx.fpscr_PR == 0)
    {
        u32 n = GetN(instr);
        u32 m = GetM(instr);
        dc->ctx.fr[n] *= dc->ctx.fr[m];
    }
    else
    {
        assert(false);
    }
}
//fdiv <FREG_M>,<FREG_N>
sh4impl(i1111_nnnn_mmmm_0011)
{
    if (dc->ctx.fpscr_PR == 0)
    {
        u32 n = GetN(instr);
        u32 m = GetM(instr);

        dc->ctx.fr[n] /= dc->ctx.fr[m];
    }
    else
    {
        assert(false);
    }
}
//fcmp/eq <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0100);
//fcmp/gt <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0101);
//fmov.s @(R0,<REG_M>),<FREG_N>
sh4op(i1111_nnnn_mmmm_0110);
//fmov.s <FREG_M>,@(R0,<REG_N>)
sh4op(i1111_nnnn_mmmm_0111);
//fmov.s @<REG_M>,<FREG_N>
sh4impl(i1111_nnnn_mmmm_1000)
{
    if (dc->ctx.fpscr_SZ == 0)
    {
        u32 n = GetN(instr);
        u32 m = GetM(instr);
        readMem<u32>(dc, dc->ctx.r[m], dc->ctx.fr_u32[n]);
    }
    else
    {
        assert(false);
    }
}
//fmov.s @<REG_M>+,<FREG_N>
sh4op(i1111_nnnn_mmmm_1001);
//fmov.s <FREG_M>,@<REG_N>
sh4op(i1111_nnnn_mmmm_1010);
//fmov.s <FREG_M>,@-<REG_N>
sh4op(i1111_nnnn_mmmm_1011);
//fmov <FREG_M>,<FREG_N>
sh4impl(i1111_nnnn_mmmm_1100)
{
    if (dc->ctx.fpscr_SZ == 0)
    {
        u32 n = GetN(instr);
        u32 m = GetM(instr);
        dc->ctx.fr[n] = dc->ctx.fr[m];
    }
    else
    {
        assert(false);
    }
}
//fabs <FREG_N>
sh4op(i1111_nnnn_0101_1101);
//FSCA FPUL, DRn//F0FD//1111_nnn0_1111_1101
sh4impl(i1111_nnn0_1111_1101)
{
    int n=GetN(instr) & 0xE;

    //cosine(x) = sine(pi/2 + x).
    if (dc->ctx.fpscr_PR==0)
    {
        u32 pi_index = dc->ctx.fpul&0xFFFF;

        float rads = pi_index/(65536.0f/2)*3.1415f;

        dc->ctx.fr[n + 0] = sinf(rads);
        dc->ctx.fr[n + 1] = cosf(rads);
    }
    else {
        assert(false);
    }
}

//fcnvds <DR_N>,FPUL
sh4op(i1111_nnnn_1011_1101);
//fcnvsd FPUL,<DR_N>
sh4op(i1111_nnnn_1010_1101);
//fipr <FV_M>,<FV_N>
sh4op(i1111_nnmm_1110_1101);
//fldi0 <FREG_N>
sh4op(i1111_nnnn_1000_1101);
//fldi1 <FREG_N>
sh4op(i1111_nnnn_1001_1101);
//flds <FREG_N>,FPUL
sh4op(i1111_nnnn_0001_1101);
//float FPUL,<FREG_N>
sh4impl(i1111_nnnn_0010_1101)
{
    if (dc->ctx.fpscr_PR == 0)
    {
        u32 n = GetN(instr);
        dc->ctx.fr[n] = (f32)(s32)dc->ctx.fpul;
    }
    else
    {
        assert(false);
    }
}
//fneg <FREG_N>
sh4op(i1111_nnnn_0100_1101);
//frchg
sh4op(i1111_1011_1111_1101);
//fschg
sh4op(i1111_0011_1111_1101);
//fsqrt <FREG_N>
sh4op(i1111_nnnn_0110_1101);
//ftrc <FREG_N>, FPUL
sh4impl(i1111_nnnn_0011_1101)
{
    if (dc->ctx.fpscr_PR == 0)
    {
        u32 n = GetN(instr);
        dc->ctx.fpul = (u32)(s32)std::min(dc->ctx.fr[n],(float)0x7FFFFFBF);

        if (dc->ctx.fpul==0x80000000) //this is actually a x86-specific fix. I think ARM saturates
        {
            if (dc->ctx.fr[n]>0)
                dc->ctx.fpul--;
        }
    }
    else
    {
        assert(false);
    }
}

//fsts FPUL,<FREG_N>
sh4op(i1111_nnnn_0000_1101);
//fmac <FREG_0>,<FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_1110);
//ftrv xmtrx,<FV_N>
sh4op(i1111_nn01_1111_1101);
//FSRRA
sh4op(i1111_nnnn_0111_1101);

sh4_opcodelistentry missing_opcode = {iNotImplemented, 0, 0, "missing"};

sh4_opcodelistentry opcodes[] =
{
    //CPU
    {i0000_nnnn_0010_0011   ,Mask_n         ,0x0023 ,"braf <REG_N>"},  //braf <REG_N>
    {i0000_nnnn_0000_0011   ,Mask_n         ,0x0003 ,"bsrf <REG_N>"},  //bsrf <REG_N>
    {i0000_nnnn_1100_0011   ,Mask_n         ,0x00C3 ,"movca.l R0, @<REG_N>"}, //movca.l R0, @<REG_N>
    {i0000_nnnn_1001_0011   ,Mask_n         ,0x0093 ,"ocbi @<REG_N>"},  //ocbi @<REG_N>
    {i0000_nnnn_1010_0011   ,Mask_n         ,0x00A3 ,"ocbp @<REG_N>"},  //ocbp @<REG_N>
    {i0000_nnnn_1011_0011   ,Mask_n         ,0x00B3 ,"ocbwb @<REG_N>"},  //ocbwb @<REG_N>
    {i0000_nnnn_1000_0011   ,Mask_n         ,0x0083 ,"pref @<REG_N>"},  //pref @<REG_N>
    {i0000_nnnn_mmmm_0111   ,Mask_n_m       ,0x0007 ,"mul.l <REG_M>,<REG_N>"}, //mul.l <REG_M>,<REG_N>
    {i0000_0000_0010_1000   ,Mask_none      ,0x0028 ,"clrmac"},  //clrmac
    {i0000_0000_0100_1000   ,Mask_none      ,0x0048 ,"clrs"}, //clrs
    {i0000_0000_0000_1000   ,Mask_none      ,0x0008 ,"clrt"},    //clrt
    {i0000_0000_0011_1000   ,Mask_none      ,0x0038 ,"ldtlb"}   ,//ldtlb
    {i0000_0000_0101_1000   ,Mask_none      ,0x0058 ,"sets"},  //sets
    {i0000_0000_0001_1000   ,Mask_none      ,0x0018 ,"sett"}, //sett
    {i0000_0000_0001_1001   ,Mask_none      ,0x0019 ,"div0u"},//div0u
    {i0000_nnnn_0010_1001   ,Mask_n         ,0x0029 ,"movt <REG_N>"},  //movt <REG_N>
    {i0000_0000_0000_1001   ,Mask_none      ,0x0009 ,"nop"}   ,//nop



    {i0000_0000_0010_1011   ,Mask_none      ,0x002B ,"rte"},  //rte
    {i0000_0000_0000_1011   ,Mask_none      ,0x000B ,"rts"},  //rts
    {i0000_0000_0001_1011   ,Mask_none      ,0x001B ,"sleep"},  //sleep


    {i0000_nnnn_mmmm_1111   ,Mask_n_m       ,0x000F ,"mac.l @<REG_M>+,@<REG_N>+"},  //mac.l @<REG_M>+,@<REG_N>+
    {i0010_nnnn_mmmm_0111   ,Mask_n_m       ,0x2007 ,"div0s <REG_M>,<REG_N>"},   // div0s <REG_M>,<REG_N>
    {i0010_nnnn_mmmm_1000   ,Mask_n_m       ,0x2008 ,"tst <REG_M>,<REG_N>"}, // tst <REG_M>,<REG_N>
    {i0010_nnnn_mmmm_1001   ,Mask_n_m       ,0x2009 ,"and <REG_M>,<REG_N>"},   //and <REG_M>,<REG_N>
    {i0010_nnnn_mmmm_1010   ,Mask_n_m       ,0x200A ,"xor <REG_M>,<REG_N>"},   //xor <REG_M>,<REG_N>
    {i0010_nnnn_mmmm_1011   ,Mask_n_m       ,0x200B ,"or <REG_M>,<REG_N>"},    //or <REG_M>,<REG_N>
    {i0010_nnnn_mmmm_1100   ,Mask_n_m       ,0x200C ,"cmp/str <REG_M>,<REG_N>"},   //cmp/str <REG_M>,<REG_N>
    {i0010_nnnn_mmmm_1101   ,Mask_n_m       ,0x200D ,"xtrct <REG_M>,<REG_N>"},  //xtrct <REG_M>,<REG_N>
    {i0010_nnnn_mmmm_1110   ,Mask_n_m       ,0x200E ,"mulu.w <REG_M>,<REG_N>"},  //mulu.w <REG_M>,<REG_N>
    {i0010_nnnn_mmmm_1111   ,Mask_n_m       ,0x200F ,"muls.w <REG_M>,<REG_N>"}, //muls.w <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_0000   ,Mask_n_m       ,0x3000 ,"cmp/eq <REG_M>,<REG_N>"},    // cmp/eq <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_0010   ,Mask_n_m       ,0x3002 ,"cmp/hs <REG_M>,<REG_N>"},    // cmp/hs <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_0011   ,Mask_n_m       ,0x3003 ,"cmp/ge <REG_M>,<REG_N>"},    //cmp/ge <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_0100   ,Mask_n_m       ,0x3004 ,"div1 <REG_M>,<REG_N>"},  //div1 <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_0101   ,Mask_n_m       ,0x3005 ,"dmulu.l <REG_M>,<REG_N>"},  //dmulu.l <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_0110   ,Mask_n_m       ,0x3006 ,"cmp/hi <REG_M>,<REG_N>"},    // cmp/hi <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_0111   ,Mask_n_m       ,0x3007 ,"cmp/gt <REG_M>,<REG_N>"},    //cmp/gt <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_1000   ,Mask_n_m       ,0x3008 ,"sub <REG_M>,<REG_N>"},   // sub <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_1010   ,Mask_n_m       ,0x300A ,"subc <REG_M>,<REG_N>"},    //subc <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_1011   ,Mask_n_m       ,0x300B ,"subv <REG_M>,<REG_N>"},  //subv <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_1100   ,Mask_n_m       ,0x300C ,"add <REG_M>,<REG_N>"},   //add <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_1101   ,Mask_n_m       ,0x300D ,"dmuls.l <REG_M>,<REG_N>"}, //dmuls.l <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_1110   ,Mask_n_m       ,0x300E ,"addc <REG_M>,<REG_N>"},    //addc <REG_M>,<REG_N>
    {i0011_nnnn_mmmm_1111   ,Mask_n_m       ,0x300F ,"addv <REG_M>,<REG_N>"},  // addv <REG_M>,<REG_N>

    //Normal readm/writem
    {i0000_nnnn_mmmm_0100   ,Mask_n_m       ,0x0004 ,"mov.b <REG_M>,@(R0,<REG_N>)"},  //mov.b <REG_M>,@(R0,<REG_N>)
    {i0000_nnnn_mmmm_0101   ,Mask_n_m       ,0x0005 ,"mov.w <REG_M>,@(R0,<REG_N>)"},  //mov.w <REG_M>,@(R0,<REG_N>)
    {i0000_nnnn_mmmm_0110   ,Mask_n_m       ,0x0006 ,"mov.l <REG_M>,@(R0,<REG_N>)"},  //mov.l <REG_M>,@(R0,<REG_N>)
    {i0000_nnnn_mmmm_1100   ,Mask_n_m       ,0x000C ,"mov.b @(R0,<REG_M>),<REG_N>"},  //mov.b @(R0,<REG_M>),<REG_N>
    {i0000_nnnn_mmmm_1101   ,Mask_n_m       ,0x000D ,"mov.w @(R0,<REG_M>),<REG_N>"},  //mov.w @(R0,<REG_M>),<REG_N>
    {i0000_nnnn_mmmm_1110   ,Mask_n_m       ,0x000E ,"mov.l @(R0,<REG_M>),<REG_N>"},  //mov.l @(R0,<REG_M>),<REG_N>
    {i0001_nnnn_mmmm_iiii   ,Mask_n_imm8    ,0x1000 ,"mov.l <REG_M>,@(<disp4dw>,<REG_N>)"},   //mov.l <REG_M>,@(<disp>,<REG_N>)
    {i0101_nnnn_mmmm_iiii   ,Mask_n_m_imm4  ,0x5000 ,"mov.l @(<disp4dw>,<REG_M>),<REG_N>"},   //mov.l @(<disp>,<REG_M>),<REG_N>
    {i0010_nnnn_mmmm_0000   ,Mask_n_m       ,0x2000 ,"mov.b <REG_M>,@<REG_N>"}, //mov.b <REG_M>,@<REG_N>
    {i0010_nnnn_mmmm_0001   ,Mask_n_m       ,0x2001 ,"mov.w <REG_M>,@<REG_N>"}, // mov.w <REG_M>,@<REG_N>
    {i0010_nnnn_mmmm_0010   ,Mask_n_m       ,0x2002 ,"mov.l <REG_M>,@<REG_N>"}, // mov.l <REG_M>,@<REG_N>
    {i0110_nnnn_mmmm_0000   ,Mask_n_m       ,0x6000 ,"mov.b @<REG_M>,<REG_N>"}, //mov.b @<REG_M>,<REG_N>
    {i0110_nnnn_mmmm_0001   ,Mask_n_m       ,0x6001 ,"mov.w @<REG_M>,<REG_N>"}, //mov.w @<REG_M>,<REG_N>
    {i0110_nnnn_mmmm_0010   ,Mask_n_m       ,0x6002 ,"mov.l @<REG_M>,<REG_N>"}, //mov.l @<REG_M>,<REG_N>
    {i0010_nnnn_mmmm_0100   ,Mask_n_m       ,0x2004 ,"mov.b <REG_M>,@-<REG_N>"},    // mov.b <REG_M>,@-<REG_N>
    {i0010_nnnn_mmmm_0101   ,Mask_n_m       ,0x2005 ,"mov.w <REG_M>,@-<REG_N>"},    //mov.w <REG_M>,@-<REG_N>
    {i0010_nnnn_mmmm_0110   ,Mask_n_m       ,0x2006 ,"mov.l <REG_M>,@-<REG_N>"},    //mov.l <REG_M>,@-<REG_N>
    {i0110_nnnn_mmmm_0100   ,Mask_n_m       ,0x6004 ,"mov.b @<REG_M>+,<REG_N>"},    //mov.b @<REG_M>+,<REG_N>
    {i0110_nnnn_mmmm_0101   ,Mask_n_m       ,0x6005 ,"mov.w @<REG_M>+,<REG_N>"},    //mov.w @<REG_M>+,<REG_N>
    {i0110_nnnn_mmmm_0110   ,Mask_n_m       ,0x6006 ,"mov.l @<REG_M>+,<REG_N>"},    //mov.l @<REG_M>+,<REG_N>
    {i1000_0000_mmmm_iiii   ,Mask_imm8      ,0x8000 ,"mov.b R0,@(<disp4b>,<REG_M>)"},   // mov.b R0,@(<disp>,<REG_M>)
    {i1000_0001_mmmm_iiii   ,Mask_imm8      ,0x8100 ,"mov.w R0,@(<disp4w>,<REG_M>)"},   // mov.w R0,@(<disp>,<REG_M>)
    {i1000_0100_mmmm_iiii   ,Mask_imm8      ,0x8400 ,"mov.b @(<disp4b>,<REG_M>),R0"},   // mov.b @(<disp>,<REG_M>),R0
    {i1000_0101_mmmm_iiii   ,Mask_imm8      ,0x8500 ,"mov.w @(<disp4w>,<REG_M>),R0"},   // mov.w @(<disp>,<REG_M>),R0
    {i1001_nnnn_iiii_iiii   ,Mask_n_imm8    ,0x9000 ,"mov.w @(<PCdisp8w>),<REG_N>"},   // mov.w @(<disp>,PC),<REG_N>
    {i1100_0000_iiii_iiii   ,Mask_imm8      ,0xC000 ,"mov.b R0,@(<disp8b>,GBR)"},  // mov.b R0,@(<disp>,GBR)
    {i1100_0001_iiii_iiii   ,Mask_imm8      ,0xC100 ,"mov.w R0,@(<disp8w>,GBR)"},  // mov.w R0,@(<disp>,GBR)
    {i1100_0010_iiii_iiii   ,Mask_imm8      ,0xC200 ,"mov.l R0,@(<disp8dw>,GBR)"},  // mov.l R0,@(<disp>,GBR)
    {i1100_0100_iiii_iiii   ,Mask_imm8      ,0xC400 ,"mov.b @(<GBRdisp8b>),R0"},  // mov.b @(<disp>,GBR),R0
    {i1100_0101_iiii_iiii   ,Mask_imm8      ,0xC500 ,"mov.w @(<GBRdisp8w>),R0"},  // mov.w @(<disp>,GBR),R0
    {i1100_0110_iiii_iiii   ,Mask_imm8      ,0xC600 ,"mov.l @(<GBRdisp8dw>),R0"},  // mov.l @(<disp>,GBR),R0
    {i1101_nnnn_iiii_iiii   ,Mask_n_imm8    ,0xD000 ,"mov.l @(<PCdisp8d>),<REG_N>"},   // mov.l @(<disp>,PC),<REG_N>

    //normal mov
    {i0110_nnnn_mmmm_0011   ,Mask_n_m       ,0x6003 ,"mov <REG_M>,<REG_N>"},  //mov <REG_M>,<REG_N>
    {i1100_0111_iiii_iiii   ,Mask_imm8      ,0xC700 ,"mova @(<PCdisp8d>),R0"},  // mova @(<disp>,PC),R0
    {i1110_nnnn_iiii_iiii   ,Mask_n_imm8    ,0xE000 ,"mov #<simm8hex>,<REG_N>"}, // mov #<imm>,<REG_N>

    {i0100_nnnn_0101_0010   ,Mask_n         ,0x4052, "sts.l FPUL,@-<REG_N>"},    //sts.l FPUL,@-<REG_N>
    {i0100_nnnn_0110_0010   ,Mask_n         ,0x4062, "sts.l FPSCR,@-<REG_N>"},    //sts.l FPSCR,@-<REG_N>
    {i0100_nnnn_0000_0010   ,Mask_n         ,0x4002, "sts.l MACH,@-<REG_N>"},    //sts.l MACH,@-<REG_N>
    {i0100_nnnn_0001_0010   ,Mask_n         ,0x4012, "sts.l MACL,@-<REG_N>"},    //sts.l MACL,@-<REG_N>
    {i0100_nnnn_0010_0010   ,Mask_n         ,0x4022, "sts.l PR,@-<REG_N>"},    //sts.l PR,@-<REG_N>
    {i0100_nnnn_1111_0010   ,Mask_n         ,0x40F2, "stc.l DBR,@-<REG_N>"},    //sts.l DBR,@-<REG_N>
    {i0100_nnnn_0011_0010   ,Mask_n         ,0x4032, "stc.l SGR,@-<REG_N>"},    //sts.l SGR,@-<REG_N>
    {i0100_nnnn_0000_0011   ,Mask_n         ,0x4003, "stc.l SR,@-<REG_N>"},      //stc.l SR,@-<REG_N>
    {i0100_nnnn_0001_0011   ,Mask_n         ,0x4013, "stc.l GBR,@-<REG_N>"},    //stc.l GBR,@-<REG_N>
    {i0100_nnnn_0010_0011   ,Mask_n         ,0x4023, "stc.l VBR,@-<REG_N>"},    //stc.l VBR,@-<REG_N>
    {i0100_nnnn_0011_0011   ,Mask_n         ,0x4033, "stc.l SSR,@-<REG_N>"},    //stc.l SSR,@-<REG_N>
    {i0100_nnnn_0100_0011   ,Mask_n         ,0x4043, "stc.l SPC,@-<REG_N>"},    //stc.l SPC,@-<REG_N>
    {i0100_nnnn_1mmm_0011   ,Mask_n_ml3bit  ,0x4083, "stc <RM_BANK>,@-<REG_N>"},    //stc RM_BANK,@-<REG_N>
    {i0100_nnnn_0000_0110   ,Mask_n         ,0x4006, "lds.l @<REG_N>+,MACH"},    //lds.l @<REG_N>+,MACH
    {i0100_nnnn_0001_0110   ,Mask_n         ,0x4016, "lds.l @<REG_N>+,MAC"},    //lds.l @<REG_N>+,MACL
    {i0100_nnnn_0010_0110   ,Mask_n         ,0x4026, "lds.l @<REG_N>+,PR"},    //lds.l @<REG_N>+,PR
    {i0100_nnnn_0011_0110   ,Mask_n         ,0x4036, "ldc.l @<REG_N>+,SGR"},    //lds.l @<REG_N>+,SGR
    {i0100_nnnn_0101_0110   ,Mask_n         ,0x4056, "lds.l @<REG_N>+,FPUL"},    //lds.l @<REG_N>+,FPUL
    {i0100_nnnn_0110_0110   ,Mask_n         ,0x4066, "lds.l @<REG_N>+,FPSCR"},  //lds.l @<REG_N>+,FPSCR
    {i0100_nnnn_1111_0110   ,Mask_n         ,0x40F6, "ldc.l @<REG_N>+,DBR"},    //lds.l @<REG_N>+,DBR
    {i0100_nnnn_0000_0111   ,Mask_n         ,0x4007, "ldc.l @<REG_N>+,SR"},  //ldc.l @<REG_N>+,SR
    {i0100_nnnn_0001_0111   ,Mask_n         ,0x4017, "ldc.l @<REG_N>+,GBR"},    //ldc.l @<REG_N>+,GBR
    {i0100_nnnn_0010_0111   ,Mask_n         ,0x4027, "ldc.l @<REG_N>+,VBR"},    //ldc.l @<REG_N>+,VBR
    {i0100_nnnn_0011_0111   ,Mask_n         ,0x4037, "ldc.l @<REG_N>+,SSR"},    //ldc.l @<REG_N>+,SSR
    {i0100_nnnn_0100_0111   ,Mask_n         ,0x4047, "ldc.l @<REG_N>+,SPC"},    //ldc.l @<REG_N>+,SPC
    {i0100_nnnn_1mmm_0111   ,Mask_n_ml3bit  ,0x4087, "ldc.l @<REG_N>+,RM_BANK"},    //ldc.l @<REG_N>+,RM_BANK
    {i0000_nnnn_0000_0010   ,Mask_n         ,0x0002, "stc SR,<REG_N>"},  //stc SR,<REG_N>
    {i0000_nnnn_0001_0010   ,Mask_n         ,0x0012, "stc GBR,<REG_N>"}, //stc GBR,<REG_N>
    {i0000_nnnn_0010_0010   ,Mask_n         ,0x0022, "stc VBR,<REG_N>"}, //stc VBR,<REG_N>
    {i0000_nnnn_0011_0010   ,Mask_n         ,0x0032, "stc SSR,<REG_N>"}, //stc SSR,<REG_N>
    {i0000_nnnn_0100_0010   ,Mask_n         ,0x0042, "stc SPC,<REG_N>"}, //stc SPC,<REG_N>
    {i0000_nnnn_1mmm_0010   ,Mask_n_ml3bit  ,0x0082, "stc RM_BANK,<REG_N>"}, //stc RM_BANK,<REG_N>
    {i0000_nnnn_0000_1010   ,Mask_n         ,0x000A, "sts MACH,<REG_N>"}, //sts MACH,<REG_N>
    {i0000_nnnn_0001_1010   ,Mask_n         ,0x001A, "sts MACL,<REG_N>"}, //sts MACL,<REG_N>
    {i0000_nnnn_0010_1010   ,Mask_n         ,0x002A, "sts PR,<REG_N>"}, //sts PR,<REG_N>
    {i0000_nnnn_0011_1010   ,Mask_n         ,0x003A, "sts SGR,<REG_N>"}, //sts SGR,<REG_N>
    {i0000_nnnn_0101_1010   ,Mask_n         ,0x005A, "sts FPUL,<REG_N>"}, //sts FPUL,<REG_N>
    {i0000_nnnn_0110_1010   ,Mask_n         ,0x006A, "sts FPSCR,<REG_N>"}, //sts FPSCR,<REG_N>
    {i0000_nnnn_1111_1010   ,Mask_n         ,0x00FA, "sts DBR,<REG_N>"}, //sts DBR,<REG_N>
    {i0100_nnnn_0000_1010   ,Mask_n         ,0x400A, "lds <REG_N>,MACH"}, //lds <REG_N>,MACH
    {i0100_nnnn_0001_1010   ,Mask_n         ,0x401A, "lds <REG_N>,MAC"}, //lds <REG_N>,MACL
    {i0100_nnnn_0010_1010   ,Mask_n         ,0x402A, "lds <REG_N>,PR"}, //lds <REG_N>,PR
    {i0100_nnnn_0011_1010   ,Mask_n         ,0x403A, "ldc <REG_N>,SGR"}, //lds <REG_N>,SGR
    {i0100_nnnn_0101_1010   ,Mask_n         ,0x405A, "lds <REG_N>,FPUL"}, //lds <REG_N>,FPUL
    {i0100_nnnn_0110_1010   ,Mask_n         ,0x406A, "lds <REG_N>,FPSCR"},  //lds <REG_N>,FPSCR
    {i0100_nnnn_1111_1010   ,Mask_n         ,0x40FA, "ldc <REG_N>,DBR"}, //lds <REG_N>,DBR
    {i0100_nnnn_0000_1110   ,Mask_n         ,0x400E, "ldc <REG_N>,SR"},  //ldc <REG_N>,SR
    {i0100_nnnn_0001_1110   ,Mask_n         ,0x401E, "ldc <REG_N>,GBR"}, //ldc <REG_N>,GBR
    {i0100_nnnn_0010_1110   ,Mask_n         ,0x402E, "ldc <REG_N>,VBR"}, //ldc <REG_N>,VBR
    {i0100_nnnn_0011_1110   ,Mask_n         ,0x403E, "ldc <REG_N>,SSR"}, //ldc <REG_N>,SSR
    {i0100_nnnn_0100_1110   ,Mask_n         ,0x404E, "ldc <REG_N>,SPC"}, //ldc <REG_N>,SPC
    {i0100_nnnn_1mmm_1110   ,Mask_n_ml3bit  ,0x408E, "ldc <REG_N>,<RM_BANK>"}, //ldc <REG_N>,<RM_BANK>
    {i0100_nnnn_0000_0000   ,Mask_n         ,0x4000, "shll <REG_N>"},    //shll <REG_N>
    {i0100_nnnn_0001_0000   ,Mask_n         ,0x4010, "dt <REG_N>"},  //dt <REG_N>
    {i0100_nnnn_0010_0000   ,Mask_n         ,0x4020, "shal <REG_N>"},     //shal <REG_N>
    {i0100_nnnn_0000_0001   ,Mask_n         ,0x4001, "shlr <REG_N>"},   //shlr <REG_N>
    {i0100_nnnn_0001_0001   ,Mask_n         ,0x4011, "cmp/pz <REG_N>"},  //cmp/pz <REG_N>
    {i0100_nnnn_0010_0001   ,Mask_n         ,0x4021, "shar <REG_N>"},    //shar <REG_N>
    {i0100_nnnn_0010_0100   ,Mask_n         ,0x4024, "rotcl <REG_N>"},  //rotcl <REG_N>
    {i0100_nnnn_0000_0100   ,Mask_n         ,0x4004, "rotl <REG_N>"},  //rotl <REG_N>
    {i0100_nnnn_0001_0101   ,Mask_n         ,0x4015, "cmp/pl <REG_N>"},  //cmp/pl <REG_N>
    {i0100_nnnn_0010_0101   ,Mask_n         ,0x4025, "rotcr <REG_N>"},  //rotcr <REG_N>
    {i0100_nnnn_0000_0101   ,Mask_n         ,0x4005, "rotr <REG_N>"},    //rotr <REG_N>
    {i0100_nnnn_0000_1000   ,Mask_n         ,0x4008, "shll2 <REG_N>"},    //shll2 <REG_N>
    {i0100_nnnn_0001_1000   ,Mask_n         ,0x4018, "shll8 <REG_N>"},    //shll8 <REG_N>
    {i0100_nnnn_0010_1000   ,Mask_n         ,0x4028, "shll16 <REG_N>"},   //shll16 <REG_N>
    {i0100_nnnn_0000_1001   ,Mask_n         ,0x4009, "shlr2 <REG_N>"},   //shlr2 <REG_N>
    {i0100_nnnn_0001_1001   ,Mask_n         ,0x4019, "shlr8 <REG_N>"},   //shlr8 <REG_N>
    {i0100_nnnn_0010_1001   ,Mask_n         ,0x4029, "shlr16 <REG_N>"},  //shlr16 <REG_N>
    {i0100_nnnn_0010_1011   ,Mask_n         ,0x402B, "jmp @<REG_N>"},  //jmp @<REG_N>
    {i0100_nnnn_0000_1011   ,Mask_n         ,0x400B, "jsr @<REG_N>"},  //jsr @<REG_N>
    {i0100_nnnn_0001_1011   ,Mask_n         ,0x401B, "tas.b @<REG_N>"},  //tas.b @<REG_N>
    {i0100_nnnn_mmmm_1100   ,Mask_n_m       ,0x400C, "shad <REG_M>,<REG_N>"},  //shad <REG_M>,<REG_N>
    {i0100_nnnn_mmmm_1101   ,Mask_n_m       ,0x400D, "shld <REG_M>,<REG_N>"},  //shld <REG_M>,<REG_N>
    {i0100_nnnn_mmmm_1111   ,Mask_n_m       ,0x400F, "mac.w @<REG_M>+,@<REG_N>+"},  //mac.w @<REG_M>+,@<REG_N>+
    {i0110_nnnn_mmmm_0111   ,Mask_n_m       ,0x6007, "not <REG_M>,<REG_N>"},    //not <REG_M>,<REG_N>
    {i0110_nnnn_mmmm_1000   ,Mask_n_m       ,0x6008, "swap.b <REG_M>,<REG_N>"}, //swap.b <REG_M>,<REG_N>
    {i0110_nnnn_mmmm_1001   ,Mask_n_m       ,0x6009, "swap.w <REG_M>,<REG_N>"},    //swap.w <REG_M>,<REG_N>
    {i0110_nnnn_mmmm_1010   ,Mask_n_m       ,0x600A, "negc <REG_M>,<REG_N>"},  //negc <REG_M>,<REG_N>
    {i0110_nnnn_mmmm_1011   ,Mask_n_m       ,0x600B, "neg <REG_M>,<REG_N>"},    //neg <REG_M>,<REG_N>
    {i0110_nnnn_mmmm_1100   ,Mask_n_m       ,0x600C, "extu.b <REG_M>,<REG_N>"},  //extu.b <REG_M>,<REG_N>
    {i0110_nnnn_mmmm_1101   ,Mask_n_m       ,0x600D, "extu.w <REG_M>,<REG_N>"},  //extu.w <REG_M>,<REG_N>
    {i0110_nnnn_mmmm_1110   ,Mask_n_m       ,0x600E, "exts.b <REG_M>,<REG_N>"}, //exts.b <REG_M>,<REG_N>
    {i0110_nnnn_mmmm_1111   ,Mask_n_m       ,0x600F, "exts.w <REG_M>,<REG_N>"},//exts.w <REG_M>,<REG_N>
    {i0111_nnnn_iiii_iiii   ,Mask_n_imm8    ,0x7000, "add #<simm8>,<REG_N>"},    //add #<imm>,<REG_N>
    {i1000_1011_iiii_iiii   ,Mask_imm8      ,0x8B00, "bf <bdisp8>"},  // bf <bdisp8>
    {i1000_1111_iiii_iiii   ,Mask_imm8      ,0x8F00, "bf.s <bdisp8>"},  // bf.s <bdisp8>
    {i1000_1001_iiii_iiii   ,Mask_imm8      ,0x8900, "bt <bdisp8>"},  // bt <bdisp8>
    {i1000_1101_iiii_iiii   ,Mask_imm8      ,0x8D00, "bt.s <bdisp8>"},  // bt.s <bdisp8>
    {i1000_1000_iiii_iiii   ,Mask_imm8      ,0x8800, "cmp/eq #<simm8hex>,R0"}, // cmp/eq #<imm>,R0
    {i1010_iiii_iiii_iiii   ,Mask_n_imm8    ,0xA000, "bra <bdisp12>"},  // bra <bdisp12>
    {i1011_iiii_iiii_iiii   ,Mask_n_imm8    ,0xB000, "bsr <bdisp12>"},  // bsr <bdisp12>
    {i1100_0011_iiii_iiii   ,Mask_imm8      ,0xC300, "trapa #<imm8>"},  // trapa #<imm>
    {i1100_1000_iiii_iiii   ,Mask_imm8      ,0xC800, "tst #<imm8>,R0"},  // tst #<imm>,R0
    {i1100_1001_iiii_iiii   ,Mask_imm8      ,0xC900, "and #<imm8>,R0"},   // and #<imm>,R0
    {i1100_1010_iiii_iiii   ,Mask_imm8      ,0xCA00, "xor #<imm8>,R0"},   // xor #<imm>,R0
    {i1100_1011_iiii_iiii   ,Mask_imm8      ,0xCB00, "or #<imm8>,R0"},    // or #<imm>,R0
    {i1100_1100_iiii_iiii   ,Mask_imm8      ,0xCC00, "tst.b #<imm8>,@(R0,GBR)"},  // tst.b #<imm>,@(R0,GBR)
    {i1100_1101_iiii_iiii   ,Mask_imm8      ,0xCD00, "and.b #<imm8>,@(R0,GBR)"},  // and.b #<imm>,@(R0,GBR)
    {i1100_1110_iiii_iiii   ,Mask_imm8      ,0xCE00, "xor.b #<imm8>,@(R0,GBR)"},  // xor.b #<imm>,@(R0,GBR)
    {i1100_1111_iiii_iiii   ,Mask_imm8      ,0xCF00, "or.b #<imm8>,@(R0,GBR)"},  // or.b #<imm>,@(R0,GBR)
    {i1111_nnnn_mmmm_0000   ,Mask_n_m       ,0xF000, "fadd <FREG_M_SD_F>,<FREG_N_SD_F>"},    //fadd <FREG_M>,<FREG_N>
    {i1111_nnnn_mmmm_0001   ,Mask_n_m       ,0xF001, "fsub <FREG_M_SD_F>,<FREG_N_SD_F>"},    //fsub <FREG_M>,<FREG_N>
    {i1111_nnnn_mmmm_0010   ,Mask_n_m       ,0xF002, "fmul <FREG_M_SD_F>,<FREG_N_SD_F>"},    //fmul <FREG_M>,<FREG_N>
    {i1111_nnnn_mmmm_0011   ,Mask_n_m       ,0xF003, "fdiv <FREG_M_SD_F>,<FREG_N_SD_F>"},//fdiv <FREG_M>,<FREG_N>
    {i1111_nnnn_mmmm_0100   ,Mask_n_m       ,0xF004, "fcmp/eq <FREG_M_SD_F>,<FREG_N>_SD_F"}, //fcmp/eq <FREG_M>,<FREG_N>
    {i1111_nnnn_mmmm_0101   ,Mask_n_m       ,0xF005, "fcmp/gt <FREG_M_SD_F>,<FREG_N_SD_F>"}, //fcmp/gt <FREG_M>,<FREG_N>
    {i1111_nnnn_mmmm_0110   ,Mask_n_m       ,0xF006, "fmov.s @(R0,<REG_M>),<FREG_N_SD_A>"},  //fmov.s @(R0,<REG_M>),<FREG_N>
    {i1111_nnnn_mmmm_0111   ,Mask_n_m       ,0xF007, "fmov.s <FREG_M_SD_A>,@(R0,<REG_N>)"},  //fmov.s <FREG_M>,@(R0,<REG_N>)
    {i1111_nnnn_mmmm_1000   ,Mask_n_m       ,0xF008, "fmov.s @<REG_M>,<FREG_N_SD_A>"}, //fmov.s @<REG_M>,<FREG_N>
    {i1111_nnnn_mmmm_1001   ,Mask_n_m       ,0xF009, "fmov.s @<REG_M>+,<FREG_N_SD_A>"},    //fmov.s @<REG_M>+,<FREG_N>
    {i1111_nnnn_mmmm_1010   ,Mask_n_m       ,0xF00A, "fmov.s <FREG_M_SD_A>,@<REG_N>"}, //fmov.s <FREG_M>,@<REG_N>
    {i1111_nnnn_mmmm_1011   ,Mask_n_m       ,0xF00B, "fmov.s <FREG_M_SD_A>,@-<REG_N>"},    //fmov.s <FREG_M>,@-<REG_N>
    {i1111_nnnn_mmmm_1100   ,Mask_n_m       ,0xF00C, "fmov <FREG_M_SD_A>,<FREG_N_SD_A>"},    //fmov <FREG_M>,<FREG_N>
    {i1111_nnnn_0101_1101   ,Mask_n         ,0xF05D, "fabs <FREG_N_SD_F>"}, //fabs <FREG_N>
    {i1111_nnn0_1111_1101   ,Mask_nh3bit    ,0xF0FD, "FSCA FPUL, <DR_N>"},  //FSCA FPUL, DRn//F0FD//1111_nnnn_1111_1101
    {i1111_nnnn_1011_1101   ,Mask_n         ,0xF0BD, "fcnvds <DR_N>,FPUL"},  //fcnvds <DR_N>,FPUL
    {i1111_nnnn_1010_1101   ,Mask_n         ,0xF0AD, "fcnvsd FPUL,<DR_N>"},  //fcnvsd FPUL,<DR_N>
    {i1111_nnmm_1110_1101   ,Mask_n         ,0xF0ED, "fipr <FV_M>,<FV_N>"},    //fipr <FV_M>,<FV_N>
    {i1111_nnnn_1000_1101   ,Mask_n         ,0xF08D, "fldi0 <FREG_N>"}, //fldi0 <FREG_N>
    {i1111_nnnn_1001_1101   ,Mask_n         ,0xF09D, "fldi1 <FREG_N>"},  //fldi1 <FREG_N>
    {i1111_nnnn_0001_1101   ,Mask_n         ,0xF01D, "flds <FREG_N>,FPUL"}, //flds <FREG_N>,FPUL
    {i1111_nnnn_0010_1101   ,Mask_n         ,0xF02D, "float FPUL,<FREG_N_SD_F>"}, //float FPUL,<FREG_N>
    {i1111_nnnn_0100_1101   ,Mask_n         ,0xF04D, "fneg <FREG_N_SD_F> "}, //fneg <FREG_N>
    {i1111_1011_1111_1101   ,Mask_none      ,0xFBFD, "frchg"},  //frchg
    {i1111_0011_1111_1101   ,Mask_none      ,0xF3FD, "fschg"},  //fschg
    {i1111_nnnn_0110_1101   ,Mask_n         ,0xF06D, "fsqrt <FREG_N>"},//fsqrt <FREG_N>
    {i1111_nnnn_0011_1101   ,Mask_n         ,0xF03D, "ftrc <FREG_N>, FPUL"},  //ftrc <FREG_N>, FPUL  //  ,dec_Fill(DM_UnaryOp,PRM_FPUL,PRM_FRN,shop_cvt)
    {i1111_nnnn_0000_1101   ,Mask_n         ,0xF00D, "fsts FPUL,<FREG_N>"}, //fsts FPUL,<FREG_N>
    {i1111_nn01_1111_1101   ,Mask_nh2bit    ,0xF1FD, "ftrv xmtrx,<FV_N>"},  //ftrv xmtrx,<FV_N>
    {i1111_nnnn_mmmm_1110   ,Mask_n_m       ,0xF00E, "fmac <FREG_0>,<FREG_M>,<FREG_N>"},    //fmac <FREG_0>,<FREG_M>,<FREG_N>
    {i1111_nnnn_0111_1101   ,Mask_n         ,0xF07D, "FSRRA <FREG_N>"},    //FSRRA <FREG_N> (1111nnnn 01111101)

    //HLE ops

    //end of list
    {0,0,0,0}
};

void BuildOpcodeTables(dreamcast_t* dc)
{

    for (int i=0;i<0x10000;i++)
    {
        dc->OpPtr[i]=iNotImplemented;
        dc->OpDesc[i]=&missing_opcode;
    }

    for (int i2=0;opcodes[i2].oph;i2++)
    {
        if (opcodes[i2].diss==0)
            opcodes[i2].diss="Unknown Opcode";

        uint32_t shft;
        uint32_t count;
        uint32_t mask=~opcodes[i2].mask;
        uint32_t base=opcodes[i2].key;

        switch(opcodes[i2].mask)
        {
            case Mask_none:
                count=1;
                shft=0;
                break;
            case Mask_n:
                count=16;
                shft=8;
                break;
            case Mask_n_m:
                count=256;
                shft=4;
                break;
            case Mask_n_m_imm4:
                count=256*16;
                shft=0;
                break;

            case Mask_imm8:
                count=256;
                shft=0;
                break;

            case Mask_n_ml3bit:
                count=256;
                shft=4;
                break;

            case Mask_nh3bit:
                count=8;
                shft=9;
                break;

            case Mask_nh2bit:
                count=4;
                shft=10;
                break;
            default:
                assert(false && "Error");
        }

        for (int i=0;i<count;i++)
        {
            uint32_t idx=((i<<shft)&mask)+base;

            dc->OpPtr[idx]=opcodes[i2].oph;

            dc->OpDesc[idx]=&opcodes[i2];
        }
    }
}

void initDreamcast(dreamcast_t *dc) {

    memset(dc, 0, sizeof(*dc));

    BuildOpcodeTables(dc);

    memset(dc->pixels, 0, 640 * 480 * sizeof(uint32_t));

    dc->ctx.pc = 0x8C010000;

    FILE* f = fopen("roto.bin", "rb");

    assert(f != NULL && "Failed to load roto.bin");

    fread(&dc->sys_ram[0x10000], 256, 1, f);

    fclose(f);

#if DUMP
    for (auto i = 0; i < 256; i+=2) {
        auto offset = 0x10000 + i;

        uint16_t* ptr = reinterpret_cast<uint16_t*>(&dc->sys_ram[offset]);
        uint16_t opcode = *ptr;

        // printf("%08X: %04x %s\n", 0x8C000000 + offset, opcode, OpDesc[opcode]->diss);
        printf("%s\n", dc->OpDesc[opcode]->diss);
    }
#endif
}


void runDreamcast(dreamcast_t *dc) {

    u16 instr;

    readMem(dc, dc->ctx.pc, instr);

    // printf("%08X: %04x %s\n", dc->ctx.pc, instr, dc->OpDesc[instr]->diss);

    dc->ctx.pc += 2;

    dc->OpPtr[instr](dc, instr);
}

int main(int argc, char ** argv)
{
    bool leftMouseButtonDown = false;
    bool quit = false;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window * window = SDL_CreateWindow("SDL2 Pixel Drawing",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture * texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STATIC, 640, 480);
    

    dreamcast_t* dc = new dreamcast_t();

    initDreamcast(dc);

    while (!quit)
    {
        for (int i = 0; i <1000000; i++) {
            runDreamcast(dc);
        }

        memcpy(dc->pixels, dc->video_ram, 640 * 480 * 2);

        SDL_Delay(1);

        SDL_UpdateTexture(texture, NULL, dc->pixels, 640 * sizeof(u16));

        while(SDL_PollEvent(&event)) {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT)
                        leftMouseButtonDown = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT)
                        leftMouseButtonDown = true;
                case SDL_MOUSEMOTION:
                    if (leftMouseButtonDown)
                    {
                        int mouseX = event.motion.x;
                        int mouseY = event.motion.y;
                        dc->pixels[mouseY * 640 + mouseX] = 255;
                    }
                    break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    delete dc;

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
