
// types.h

#define verify assert

inline bool is_s8(u32 v) { return (s8)v==(s32)v; }
inline bool is_u8(u32 v) { return (u8)v==(s32)v; }
inline bool is_s16(u32 v) { return (s16)v==(s32)v; }
inline bool is_u16(u32 v) { return (u16)v==(u32)v; }

// sh4.h
enum Sh4RegType
{
	//GPRs
	reg_r0,
	reg_r1,
	reg_r2,
	reg_r3,
	reg_r4,
	reg_r5,
	reg_r6,
	reg_r7,
	reg_r8,
	reg_r9,
	reg_r10,
	reg_r11,
	reg_r12,
	reg_r13,
	reg_r14,
	reg_r15,

	//FPU, bank 0
	reg_fr_0,
	reg_fr_1,
	reg_fr_2,
	reg_fr_3,
	reg_fr_4,
	reg_fr_5,
	reg_fr_6,
	reg_fr_7,
	reg_fr_8,
	reg_fr_9,
	reg_fr_10,
	reg_fr_11,
	reg_fr_12,
	reg_fr_13,
	reg_fr_14,
	reg_fr_15,

	//FPU, bank 1
	reg_xf_0,
	reg_xf_1,
	reg_xf_2,
	reg_xf_3,
	reg_xf_4,
	reg_xf_5,
	reg_xf_6,
	reg_xf_7,
	reg_xf_8,
	reg_xf_9,
	reg_xf_10,
	reg_xf_11,
	reg_xf_12,
	reg_xf_13,
	reg_xf_14,
	reg_xf_15,

	//GPR Interrupt bank
	reg_r0_Bank,
	reg_r1_Bank,
	reg_r2_Bank,
	reg_r3_Bank,
	reg_r4_Bank,
	reg_r5_Bank,
	reg_r6_Bank,
	reg_r7_Bank,

	//Misc regs
	reg_gbr,
	reg_ssr,
	reg_spc,
	reg_sgr,
	reg_dbr,
	reg_vbr,
	reg_mach,
	reg_macl,
	reg_pr,
	reg_fpul,
	reg_nextpc,
	reg_sr,            //Includes T (combined on read/separated on write)
	reg_old_sr_status, //Only the status bits
	reg_sr_status,     //Only the status bits
	reg_sr_T,          //Only T
	reg_old_fpscr,
	reg_fpscr,
	
	reg_pc_dyn,        //Write only, for dynarec only (dynamic block exit address)

	sh4_reg_count,

	/*
		These are virtual registers, used by the dynarec decoder
	*/
	regv_dr_0,
	regv_dr_2,
	regv_dr_4,
	regv_dr_6,
	regv_dr_8,
	regv_dr_10,
	regv_dr_12,
	regv_dr_14,

	regv_xd_0,
	regv_xd_2,
	regv_xd_4,
	regv_xd_6,
	regv_xd_8,
	regv_xd_10,
	regv_xd_12,
	regv_xd_14,

	regv_fv_0,
	regv_fv_4,
	regv_fv_8,
	regv_fv_12,

	regv_xmtrx,
	regv_fmtrx,

	NoReg=-1
};

//shil.h

enum shil_param_type
{
	//2 bits
	FMT_NULL,
	FMT_IMM,
	FMT_I32,
	FMT_F32,
	FMT_F64,
	
	FMT_V2,
	FMT_V3,
	FMT_V4,
	FMT_V8,
	FMT_V16,

	FMT_REG_BASE=FMT_I32,
	FMT_VECTOR_BASE=FMT_V2,

	FMT_MASK=0xFFFF,
};

/*
	formats : 16u 16s 32u 32s, 32f, 64f
	param types: r32, r64
*/


#define shil_opc(n) shop_##n,

enum shilop {

//shop_mov32
shil_opc(mov32)

//shop_mov64
shil_opc(mov64)

//Special opcodes
shil_opc(jdyn)

shil_opc(jcond)

//shop_ifb
shil_opc(ifb)

//mem io
shil_opc(readm)

shil_opc(writem)

//Canonical impl. opcodes !
shil_opc(sync_sr)

shil_opc(sync_fpscr)

//shop_and
shil_opc(and)

//shop_or
shil_opc(or)

//shop_xor
shil_opc(xor)

//shop_not
shil_opc(not)

//shop_add
shil_opc(add)

//shop_sub
shil_opc(sub)

//shop_neg
shil_opc(neg)

//shop_shl,
shil_opc(shl)

//shop_shr
shil_opc(shr)

//shop_sar
shil_opc(sar)

//shop_adc	//add with carry
shil_opc(adc)


//shop_adc	//add with carry
shil_opc(sbc)

//shop_ror
shil_opc(ror)

//shop_rocl
shil_opc(rocl)

//shop_rocr
shil_opc(rocr)

//shop_swaplb -- swap low bytes
shil_opc(swaplb)

//shop_swap -- swap all bytes in word
shil_opc(swap)

//shop_shld
shil_opc(shld)

//shop_shad
shil_opc(shad)

//shop_ext_s8
shil_opc(ext_s8)

//shop_ext_s16
shil_opc(ext_s16)

//shop_mul_u16
shil_opc(mul_u16)

//shop_mul_s16
shil_opc(mul_s16)

//no difference between signed and unsigned when only the lower
//32 bis are used !
//shop_mul_i32
shil_opc(mul_i32)

//shop_mul_u64
shil_opc(mul_u64)

//shop_mul_s64
shil_opc(mul_s64)

//shop_div32u	//divide 32 bits, unsigned
shil_opc(div32u)

//shop_div32s	//divide 32 bits, signed
shil_opc(div32s)

//shop_div32p2	//div32, fixup step (part 2)
shil_opc(div32p2)

//shop_cvt_f2i_t	//float to integer : truncate
shil_opc(cvt_f2i_t)

//shop_cvt_i2f_n	//integer to float : nearest
shil_opc(cvt_i2f_n)

//shop_cvt_i2f_z	//integer to float : round to zero
shil_opc(cvt_i2f_z)

//pref !
shil_opc(pref)

//shop_test
shil_opc(test)

//shop_seteq	//equal
shil_opc(seteq)

//shop_setge	//>=, signed (greater equal)
shil_opc(setge)

//shop_setgt //>, signed	 (greater than)
shil_opc(setgt)

//shop_setae	//>=, unsigned (above equal)
shil_opc(setae)

//shop_setab	//>, unsigned (above)
shil_opc(setab)

//shop_setpeq //set if any pair of bytes is equal
shil_opc(setpeq)

//here come the floating points

//shop_fadd
shil_opc(fadd)

//shop_fsub
shil_opc(fsub)

//shop_fmul
shil_opc(fmul)

//shop_fdiv
shil_opc(fdiv)

//shop_fabs
shil_opc(fabs)

//shop_fneg
shil_opc(fneg)
//shop_fsqrt
shil_opc(fsqrt)

//shop_fipr
shil_opc(fipr)

//shop_ftrv
shil_opc(ftrv)

//shop_fmac
shil_opc(fmac)
//shop_fsrra
shil_opc(fsrra)


//shop_fsca
shil_opc(fsca)

//shop_fseteq
shil_opc(fseteq)

//shop_fsetgt
shil_opc(fsetgt)

//shop_frswap
shil_opc(frswap)

shop_max };


struct shil_param
{
	shil_param()
	{
		type=FMT_NULL;
		_imm=0xFFFFFFFF;
	}
	shil_param(u32 type,u32 imm)
	{
		this->type=type;
		if (type >= FMT_REG_BASE)
			new (this) shil_param((Sh4RegType)imm);
		_imm=imm;
	}

	shil_param(Sh4RegType reg)
	{
		type=FMT_NULL;
		if (reg>=reg_fr_0 && reg<=reg_xf_15)
		{
			type=FMT_F32;
			_imm=reg;
		}
		else if (reg>=regv_dr_0 && reg<=regv_dr_14)
		{
			type=FMT_F64;
			_imm=(reg-regv_dr_0)*2+reg_fr_0;
		}
		else if (reg>=regv_xd_0 && reg<=regv_xd_14)
		{
			type=FMT_F64;
			_imm=(reg-regv_xd_0)*2+reg_xf_0;
		}
		else if (reg>=regv_fv_0 && reg<=regv_fv_12)
		{
			type=FMT_V4;
			_imm=(reg-regv_fv_0)*4+reg_fr_0;
		}
		else if (reg==regv_xmtrx)
		{
			type=FMT_V16;
			_imm=reg_xf_0;
		}
		else if (reg==regv_fmtrx)
		{
			type=FMT_V16;
			_imm=reg_fr_0;
		}
		else
		{
			type=FMT_I32;
			_reg=reg;
		}
		
	}
	union
	{
		u32 _imm;
		Sh4RegType _reg;
	};
	u32 type;

	bool is_null() const { return type==FMT_NULL; }
	bool is_imm() const { return type==FMT_IMM; }
	bool is_reg() const { return type>=FMT_REG_BASE; }

	bool is_r32i() const { return type==FMT_I32; }
	bool is_r32f() const { return type==FMT_F32; }
	u32 is_r32fv()  const { return type>=FMT_VECTOR_BASE?count():0; }
	bool is_r64f() const { return type==FMT_F64; }

	bool is_r32() const { return is_r32i() || is_r32f(); }
	bool is_r64() const { return is_r64f(); }	//just here for symmetry ...

	bool is_imm_s8() const { return is_imm() && is_s8(_imm); }
	bool is_imm_u8() const { return is_imm() && is_u8(_imm); }
	bool is_imm_s16() const { return is_imm() && is_s16(_imm); }
	bool is_imm_u16() const { return is_imm() && is_u16(_imm); }

	u32 imm_value() { verify(is_imm()); return _imm; }

	bool is_vector() const { return type>=FMT_VECTOR_BASE; }

	u32 count() const { return  type==FMT_F64?2:type==FMT_V2?2:
								type==FMT_V3?3:type==FMT_V4?4:type==FMT_V8?8:
								type==FMT_V16?16:1; }	//count of hardware regs

	/*	
		Imms:
		is_imm
		
		regs:
		integer regs            : is_r32i,is_r32,count=1
		fpu regs, single view   : is_r32f,is_r32,count=1
		fpu regs, double view   : is_r64f,count=2
		fpu regs, quad view     : is_vector,is_r32fv=4, count=4
		fpu regs, matrix view   : is_vector,is_r32fv=16, count=16
	*/
};

struct shil_opcode
{
	shilop op;
	u32 Flow;
	u32 flags;
	u32 flags2;

	shil_param rd,rd2;
	shil_param rs1,rs2,rs3;

	u16 host_offs;
	u16 guest_offs;
};