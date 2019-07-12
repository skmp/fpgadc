
#define die(x) assert(false && x)

void Emit(shilop op,shil_param rd=shil_param(),shil_param rs1=shil_param(),shil_param rs2=shil_param(),u32 flags=0,shil_param rs3=shil_param(),shil_param rd2=shil_param())
{
	
}

void dec_fallback(u32 op)
{
	printf("ASSERT_FALLBACK(%04X)", op);
}


#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
#define GetImm4(str) ((str>>0) & 0xf)
#define GetImm8(str) ((str>>0) & 0xff)
#define GetSImm8(str) ((s8)((str>>0) & 0xff))
#define GetImm12(str) ((str>>0) & 0xfff)
#define GetSImm12(str) (((s16)((GetImm12(str))<<4))>>4)


#define SR_STATUS_MASK 0x700083F2
#define SR_T_MASK 1

shil_param mk_imm(u32 immv)
{
	return shil_param(FMT_IMM,immv);
}
shil_param mk_reg(Sh4RegType reg)
{
	return shil_param(reg);
}
shil_param mk_regi(int reg)
{
	return mk_reg((Sh4RegType)reg);
}

#if 0

u32 dec_set_pr()
{
	u32 retaddr=state.cpu.rpc + 4;
	Emit(shop_mov32,reg_pr,mk_imm(retaddr));
	return retaddr;
}
void dec_write_sr(shil_param src)
{
	Emit(shop_and,mk_reg(reg_sr_status),src,mk_imm(SR_STATUS_MASK));
	Emit(shop_and,mk_reg(reg_sr_T),src,mk_imm(SR_T_MASK));
}

u32 dec_jump_simm8(u32 op)
{
	return state.cpu.rpc + GetSImm8(op)*2 + 4;
}
u32 dec_jump_simm12(u32 op)
{
	return state.cpu.rpc + GetSImm12(op)*2 + 4;
}

void dec_DynamicSet(u32 regbase,u32 offs=0)
{
	if (offs==0)
		Emit(shop_jdyn,reg_pc_dyn,mk_reg((Sh4RegType)regbase));
	else
		Emit(shop_jdyn,reg_pc_dyn,mk_reg((Sh4RegType)regbase),mk_imm(offs));
}

void dec_End(u32 dst,BlockEndType flags,bool delay)
{
	if (state.ngen.OnlyDynamicEnds && flags == BET_StaticJump)
	{
		Emit(shop_mov32,mk_reg(reg_nextpc),mk_imm(dst));
		dec_DynamicSet(reg_nextpc);
		dec_End(0xFFFFFFFF,BET_DynamicJump,delay);
		return;
	}

	if (state.ngen.OnlyDynamicEnds)
	{
		verify(flags == BET_DynamicJump);
	}

	state.BlockType=flags;
	state.NextOp=delay?NDO_Delayslot:NDO_End;
	state.DelayOp=NDO_End;
	state.JumpAddr=dst;
	state.NextAddr=state.cpu.rpc+2+(delay?2:0);
}


//bf <bdisp8>
sh4dec(i1000_1011_iiii_iiii)
{
	dec_End(dec_jump_simm8(op),BET_Cond_0,false);
}
//bf.s <bdisp8>
sh4dec(i1000_1111_iiii_iiii)
{
	blk->has_jcond=true;
	Emit(shop_jcond,reg_pc_dyn,reg_sr_T);
	dec_End(dec_jump_simm8(op),BET_Cond_0,true);
}
//bt <bdisp8>
sh4dec(i1000_1001_iiii_iiii)
{
	dec_End(dec_jump_simm8(op),BET_Cond_1,false);
}
//bt.s <bdisp8>
sh4dec(i1000_1101_iiii_iiii)
{
	blk->has_jcond=true;
	Emit(shop_jcond,reg_pc_dyn,reg_sr_T);
	dec_End(dec_jump_simm8(op),BET_Cond_1,true);
}
//bra <bdisp12>
sh4dec(i1010_iiii_iiii_iiii)
{
	dec_End(dec_jump_simm12(op),BET_StaticJump,true);
}
//braf <REG_N>
sh4dec(i0000_nnnn_0010_0011)
{
	u32 n = GetN(op);

	dec_DynamicSet(reg_r0+n,state.cpu.rpc + 4);
	dec_End(0xFFFFFFFF,BET_DynamicJump,true);
}
//jmp @<REG_N>
sh4dec(i0100_nnnn_0010_1011)
{
	u32 n = GetN(op);

	dec_DynamicSet(reg_r0+n);
	dec_End(0xFFFFFFFF,BET_DynamicJump,true);
}
//bsr <bdisp12>
sh4dec(i1011_iiii_iiii_iiii)
{
	//TODO: set PR
	dec_set_pr();
	dec_End(dec_jump_simm12(op),BET_StaticCall,true);
}
//bsrf <REG_N>
sh4dec(i0000_nnnn_0000_0011)
{
	u32 n = GetN(op);
	//TODO: set PR
	u32 retaddr=dec_set_pr();
	dec_DynamicSet(reg_r0+n,retaddr);
	dec_End(0xFFFFFFFF,BET_DynamicCall,true);
}
//jsr @<REG_N>
sh4dec(i0100_nnnn_0000_1011) 
{
	u32 n = GetN(op);

	//TODO: Set pr
	dec_set_pr();
	dec_DynamicSet(reg_r0+n);
	dec_End(0xFFFFFFFF,BET_DynamicCall,true);
}
//rts
sh4dec(i0000_0000_0000_1011)
{
	dec_DynamicSet(reg_pr);
	dec_End(0xFFFFFFFF,BET_DynamicRet,true);
}
//rte
sh4dec(i0000_0000_0010_1011)
{
	//TODO: Write SR, Check intr
	dec_write_sr(reg_ssr);
	Emit(shop_sync_sr);
	dec_DynamicSet(reg_spc);
	dec_End(0xFFFFFFFF,BET_DynamicIntr,true);
}
//trapa #<imm>
sh4dec(i1100_0011_iiii_iiii)
{
	//TODO: ifb
	dec_fallback(op);
	dec_DynamicSet(reg_nextpc);
	dec_End(0xFFFFFFFF,BET_DynamicJump,false);
}
//sleep
sh4dec(i0000_0000_0001_1011)
{
	//TODO: ifb
	dec_fallback(op);
	dec_DynamicSet(reg_nextpc);
	dec_End(0xFFFFFFFF,BET_DynamicJump,false);
}

//ldc.l @<REG_N>+,SR
sh4dec(i0100_nnnn_0000_0111)
{
	/*
	u32 sr_t;
	ReadMemU32(sr_t,r[n]);
	if (sh4_exept_raised)
		return;
	sr.SetFull(sr_t);
	r[n] += 4;
	if (UpdateSR())
	{
		//FIXME only if interrupts got on .. :P
		UpdateINTC();
	}*/
	dec_End(0xFFFFFFFF,BET_StaticIntr,false);
}

//ldc <REG_N>,SR
sh4dec(i0100_nnnn_0000_1110)
{
	u32 n = GetN(op);

	dec_write_sr((Sh4RegType)(reg_r0+n));
	Emit(shop_sync_sr);
	dec_End(0xFFFFFFFF,BET_StaticIntr,false);
}

//nop !
sh4dec(i0000_0000_0000_1001)
{
}

sh4dec(i1111_0011_1111_1101)
{
	//fpscr.SZ is bit 20
	Emit(shop_xor,reg_fpscr,reg_fpscr,mk_imm(1<<20));
	state.cpu.FSZ64=!state.cpu.FSZ64;
}

//frchg
sh4dec(i1111_1011_1111_1101)
{
	Emit(shop_xor,reg_fpscr,reg_fpscr,mk_imm(1<<21));
	Emit(shop_mov32,reg_old_fpscr,reg_fpscr);
	shil_param rmn;//null param
	Emit(shop_frswap,regv_xmtrx,regv_fmtrx,regv_xmtrx,0,rmn,regv_fmtrx);
}

//not-so-elegant, but avoids extra opcodes and temporalities ..
//rotcl
sh4dec(i0100_nnnn_0010_0100)
{
	u32 n = GetN(op);
	Sh4RegType rn=(Sh4RegType)(reg_r0+n);
	
	Emit(shop_rocl,rn,rn,reg_sr_T,0,shil_param(),reg_sr_T);
	/*
	Emit(shop_ror,rn,rn,mk_imm(31));
	Emit(shop_xor,rn,rn,reg_sr_T);              //Only affects last bit (swap part a)
	Emit(shop_xor,reg_sr_T,reg_sr_T,rn);        //srT -> rn
	Emit(shop_and,reg_sr_T,reg_sr_T,mk_imm(1)); //Keep only last bit
	Emit(shop_xor,rn,rn,reg_sr_T);              //Only affects last bit (swap part b)
	*/
}

//rotcr
sh4dec(i0100_nnnn_0010_0101)
{
	u32 n = GetN(op);
	Sh4RegType rn=(Sh4RegType)(reg_r0+n);

	Emit(shop_rocr,rn,rn,reg_sr_T,0,shil_param(),reg_sr_T);
	/*
	Emit(shop_xor,rn,rn,reg_sr_T);              //Only affects last bit (swap part a)
	Emit(shop_xor,reg_sr_T,reg_sr_T,rn);        //srT -> rn
	Emit(shop_and,reg_sr_T,reg_sr_T,mk_imm(1)); //Keep only last bit
	Emit(shop_xor,rn,rn,reg_sr_T);              //Only affects last bit (swap part b)

	Emit(shop_ror,rn,rn,mk_imm(1));
	*/
}
#endif

const Sh4RegType SREGS[] =
{
	reg_mach,
	reg_macl,
	reg_pr,
	reg_sgr,
	NoReg,
	reg_fpul,
	reg_fpscr,
	NoReg,

	NoReg,
	NoReg,
	NoReg,
	NoReg,
	NoReg,
	NoReg,
	NoReg,
	reg_dbr,
};

const Sh4RegType CREGS[] =
{
	reg_sr,
	reg_gbr,
	reg_vbr,
	reg_ssr,
	reg_spc,
	NoReg,
	NoReg,
	NoReg,

	reg_r0_Bank,
	reg_r1_Bank,
	reg_r2_Bank,
	reg_r3_Bank,
	reg_r4_Bank,
	reg_r5_Bank,
	reg_r6_Bank,
	reg_r7_Bank,
};

void dec_param(DecParam p,shil_param& r1,shil_param& r2, bool FSZ64)
{
	switch(p)
	{
		//constants
	case PRM_ZERO:
		r1= mk_imm(0);
		break;

	case PRM_ONE:
		r1= mk_imm(1);
		break;

	case PRM_TWO:
		r1= mk_imm(2);
		break;

	case PRM_TWO_INV:
		r1= mk_imm(~2);
		break;

	case PRM_ONE_F32:
		r1= mk_imm(0x3f800000);
		break;

#if 0 // TODO
	case PRM_PC_D8_x2:
		r1=mk_imm((state.cpu.rpc+4)+(GetImm8(op)<<1));
		break;

	case PRM_PC_D8_x4:
		r1=mk_imm(((state.cpu.rpc+4)&0xFFFFFFFC)+(GetImm8(op)<<2));
		break;

	//imms
	case PRM_SIMM8:
		r1=mk_imm(GetSImm8(op));
		break;
	case PRM_UIMM8:
		r1=mk_imm(GetImm8(op));
		break;

	//direct registers
	case PRM_R0:
		r1=mk_reg(reg_r0);
		break;

	case PRM_RN:
		r1=mk_regi(reg_r0+GetN(op));
		break;

	case PRM_RM:
		r1=mk_regi(reg_r0+GetM(op));
		break;

	case PRM_FRN_SZ:
		if (FSZ64)
		{
			int rx=GetN(op)/2;
			if (GetN(op)&1)
				rx+=regv_xd_0;
			else
				rx+=regv_dr_0;

			r1=mk_regi(rx);
			break;
		}
	case PRM_FRN:
		r1=mk_regi(reg_fr_0+GetN(op));
		break;

	case PRM_FRM_SZ:
		if (FSZ64)
		{
			int rx=GetM(op)/2;
			if (GetM(op)&1)
				rx+=regv_xd_0;
			else
				rx+=regv_dr_0;

			r1=mk_regi(rx);
			break;
		}
	case PRM_FRM:
		r1=mk_regi(reg_fr_0+GetM(op));
		break;

	case PRM_FPUL:
		r1=mk_regi(reg_fpul);
		break;

	case PRM_FPN:	//float pair, 3 bits
		r1=mk_regi(regv_dr_0+GetN(op)/2);
		break;

	case PRM_FVN:	//float quad, 2 bits
		r1=mk_regi(regv_fv_0+GetN(op)/4);
		break;

	case PRM_FVM:	//float quad, 2 bits
		r1=mk_regi(regv_fv_0+(GetN(op)&0x3));
		break;

	case PRM_XMTRX:	//float matrix, 0 bits
		r1=mk_regi(regv_xmtrx);
		break;

	case PRM_FRM_FR0:
		r1=mk_regi(reg_fr_0+GetM(op));
		r2=mk_regi(reg_fr_0);
		break;

	case PRM_SR_T:
		r1=mk_regi(reg_sr_T);
		break;

	case PRM_SR_STATUS:
		r1=mk_regi(reg_sr_status);
		break;

	case PRM_SREG:	//FPUL/FPSCR/MACH/MACL/PR/DBR/SGR
		r1=mk_regi(SREGS[GetM(op)]);
		break;
	case PRM_CREG:	//SR/GBR/VBR/SSR/SPC/<RM_BANK>
		r1=mk_regi(CREGS[GetM(op)]);
		break;
	
	//reg/imm reg/reg
	case PRM_RN_D4_x1:
	case PRM_RN_D4_x2:
	case PRM_RN_D4_x4:
		{
			u32 shft=p-PRM_RN_D4_x1;
			r1=mk_regi(reg_r0+GetN(op));
			r2=mk_imm(GetImm4(op)<<shft);
		}
		break;

	case PRM_RN_R0:
		r1=mk_regi(reg_r0+GetN(op));
		r2=mk_regi(reg_r0);
		break;

	case PRM_RM_D4_x1:
	case PRM_RM_D4_x2:
	case PRM_RM_D4_x4:
		{
			u32 shft=p-PRM_RM_D4_x1;
			r1=mk_regi(reg_r0+GetM(op));
			r2=mk_imm(GetImm4(op)<<shft);
		}
		break;

	case PRM_RM_R0:
		r1=mk_regi(reg_r0+GetM(op));
		r2=mk_regi(reg_r0);
		break;

	case PRM_GBR_D8_x1:
	case PRM_GBR_D8_x2:
	case PRM_GBR_D8_x4:
		{
			u32 shft=p-PRM_GBR_D8_x1;
			r1=mk_regi(reg_gbr);
			r2=mk_imm(GetImm8(op)<<shft);
		}
		break;
#endif
	default:
		die("Non-supported parameter used");
	}
}

#define MASK_N_M 0xF00F
#define MASK_N   0xF0FF
#define MASK_NONE   0xFFFF

#define DIV0U_KEY 0x0019
#define DIV0S_KEY 0x2007
#define DIV1_KEY 0x3004
#define ROTCL_KEY 0x4024

Sh4RegType div_som_reg1;
Sh4RegType div_som_reg2;
Sh4RegType div_som_reg3;

/*
//This ended up too rare (and too hard to match)
bool MatchDiv0S_0(u32 pc)
{
	if (ReadMem16(pc+0)==0x233A && //XOR   r3,r3
		ReadMem16(pc+2)==0x2137 && //DIV0S r3,r1
		ReadMem16(pc+4)==0x322A && //SUBC  r2,r2
		ReadMem16(pc+6)==0x313A && //SUBC  r3,r1
		(ReadMem16(pc+8)&0xF00F)==0x2007) //DIV0S x,x
		return true;
	else
		return false;
}
*/

bool dec_generic(u32 dec_op, u32 opkey, bool FPR64, bool FSZ64, bool RoundToZero)
{
	DecMode mode;DecParam d;DecParam s;shilop natop;u32 e;
	if (dec_op==0)
		return false;
	
	u64 inf=dec_op;

	e=(u32)(inf>>32);
	mode=(DecMode)((inf>>24)&0xFF);
	d=(DecParam)((inf>>16)&0xFF);
	s=(DecParam)((inf>>8)&0xFF);
	natop=(shilop)((inf>>0)&0xFF);

	/*
	if ((op&0xF00F)==0x300E)
	{
		return false;
	}*/

	/*
	if (mode==DM_ADC)
		return false;
	*/

	bool transfer_64=false;
	if (opkey>=0xF000)
	{
		//return false;//FPU off for now
		if (FPR64 /*|| FSZ64*/)
			return false;

		if (FSZ64 && (d==PRM_FRN_SZ || d==PRM_FRM_SZ || s==PRM_FRN_SZ || s==PRM_FRM_SZ))
		{
			transfer_64=true;
		}
	}

	shil_param rs1,rs2,rs3,rd;

	dec_param(s,rs2,rs3,FSZ64);
	dec_param(d,rs1,rs3,FSZ64);

	switch(mode)
	{
	case DM_ReadSRF:
		Emit(shop_mov32,rs1,reg_sr_status);
		Emit(shop_or,rs1,rs1,reg_sr_T);
		break;

	case DM_WriteTOp:
		Emit(natop,reg_sr_T,rs1,rs2);
		break;

	case DM_DT:
		verify(natop==shop_sub);
		Emit(natop,rs1,rs1,rs2);
		Emit(shop_seteq,mk_reg(reg_sr_T),rs1,mk_imm(0));
		break;

	case DM_Shift:
		if (natop==shop_shl && e==1)
			Emit(shop_shr,mk_reg(reg_sr_T),rs1,mk_imm(31));
		else if (e==1)
			Emit(shop_and,mk_reg(reg_sr_T),rs1,mk_imm(1));

		Emit(natop,rs1,rs1,mk_imm(e));
		break;

	case DM_Rot:
		if (!(((s32)e>=0?e:-e)&0x1000))
		{
			if ((s32)e<0)
			{
				//left rotate
				Emit(shop_shr,mk_reg(reg_sr_T),rs2,mk_imm(31));
				e=-e;
			}
			else
			{
				//right rotate
				Emit(shop_and,mk_reg(reg_sr_T),rs2,mk_imm(1));
			}
		}
		e&=31;

		Emit(natop,rs1,rs2,mk_imm(e));
		break;

	case DM_BinaryOp://d=d op s
		if (e&1)
			Emit(natop,rs1,rs1,rs2,0,rs3);
		else
			Emit(natop,shil_param(),rs1,rs2,0,rs3);
		break;

	case DM_UnaryOp: //d= op s
		if (transfer_64 && natop==shop_mov32) 
			natop=shop_mov64;

		if (natop==shop_cvt_i2f_n && RoundToZero)
			natop=shop_cvt_i2f_z;

		if (e&1)
			Emit(natop,shil_param(),rs1);
		else
			Emit(natop,rs1,rs2);
		break;

	case DM_WriteM: //write(d,s)
		{
			//0 has no effect, so get rid of it
			if (rs3.is_imm() && rs3._imm==0)
				rs3=shil_param();

			if (transfer_64) e=(s32)e*2;
			bool update_after=false;
			if ((s32)e<0)
			{
				if (rs1._reg!=rs2._reg) //reg shouldn't be updated if its written
				{
					Emit(shop_sub,rs1,rs1,mk_imm(-e));
				}
				else
				{
					verify(rs3.is_null());
					rs3=mk_imm(e);
					update_after=true;
				}
			}

			Emit(shop_writem,shil_param(),rs1,rs2,(s32)e<0?-e:e,rs3);

			if (update_after)
			{
				Emit(shop_sub,rs1,rs1,mk_imm(-e));
			}
		}
		break;

	case DM_ReadM:
		//0 has no effect, so get rid of it
		if (rs3.is_imm() && rs3._imm==0)
				rs3=shil_param();

		if (transfer_64) e=(s32)e*2;

		Emit(shop_readm,rs1,rs2,shil_param(),(s32)e<0?-e:e,rs3);
		if ((s32)e<0)
		{
			if (rs1._reg!=rs2._reg)//the reg shouldn't be updated if it was just read.
				Emit(shop_add,rs2,rs2,mk_imm(-e));
		}
		break;

	case DM_fiprOp:
		{
			shil_param rdd=mk_regi(rs1._reg+3);
			Emit(natop,rdd,rs1,rs2);
		}
		break;

	case DM_EXTOP:
		{
			Emit(natop,rs1,rs2,mk_imm(e==1?0xFF:0xFFFF));
		}
		break;
	
	case DM_MUL:
		{
			shilop op;
			shil_param rd=mk_reg(reg_macl);
			shil_param rd2=shil_param();

			switch((s32)e)
			{
				case 16:  op=shop_mul_u16; break;
				case -16: op=shop_mul_s16; break;

				case -32: op=shop_mul_i32; break;

				case 64:  op=shop_mul_u64; rd2 = mk_reg(reg_mach); break;
				case -64: op=shop_mul_s64; rd2 = mk_reg(reg_mach); break;

				default:
					die("DM_MUL: Failed to classify opcode");
			}

			Emit(op,rd,rs1,rs2,0,shil_param(),rd2);
		}
		break;

	case DM_DIV0:
		{
			if (e==1)
			{
				//clear QM (bits 8,9)
				u32 qm=(1<<8)|(1<<9);
				Emit(shop_and,mk_reg(reg_sr_status),mk_reg(reg_sr_status),mk_imm(~qm));
				//clear T !
				Emit(shop_mov32,mk_reg(reg_sr_T),mk_imm(0));
			}
			else
			{
				//sr.Q=r[n]>>31;
				//sr.M=r[m]>>31;
				//sr.T=sr.M^sr.Q;

				//This is nasty because there isn't a temp reg ..
				//VERY NASTY

				//Clear Q & M
				Emit(shop_and,mk_reg(reg_sr_status),mk_reg(reg_sr_status),mk_imm(~((1<<8)|(1<<9))));

				//sr.Q=r[n]>>31;
				Emit(shop_sar,mk_reg(reg_sr_T),rs1,mk_imm(31));
				Emit(shop_and,mk_reg(reg_sr_T),mk_reg(reg_sr_T),mk_imm(1<<8));
				Emit(shop_or,mk_reg(reg_sr_status),mk_reg(reg_sr_status),mk_reg(reg_sr_T));

				//sr.M=r[m]>>31;
				Emit(shop_sar,mk_reg(reg_sr_T),rs2,mk_imm(31));
				Emit(shop_and,mk_reg(reg_sr_T),mk_reg(reg_sr_T),mk_imm(1<<9));
				Emit(shop_or,mk_reg(reg_sr_status),mk_reg(reg_sr_status),mk_reg(reg_sr_T));

				//sr.T=sr.M^sr.Q;
				Emit(shop_xor,mk_reg(reg_sr_T),rs1,rs2);
				Emit(shop_shr,mk_reg(reg_sr_T),mk_reg(reg_sr_T),mk_imm(31));
			}
		}
		break;

	case DM_ADC:
		{
			Emit(natop,rs1,rs1,rs2,0,mk_reg(reg_sr_T),mk_reg(reg_sr_T));
		}
		break;

	default:
		verify(false);
	}

	return true;
}