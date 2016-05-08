/*Intermediate Code to Machine Code

RAX,RBX,RCX and RDX can be clobbered by
each intermediate code's output code.
However, intermediate codes must be
coupled together based on the arg and
res type specifications in the
$LK,"CICArg",A="MN:CICArg"$.  RAX is the most common reg
for coupling intermediate codes.

Internal calculations take place on
64-bit vals, so anything which has
found it's way into a reg has been
sign or zero extended to 64-bits.
*/

U0 ICU8(CIntermediateCode *tempi,U8 b)
{
  tempi->ic_body[tempi->ic_cnt++]=b;
}

U0 ICRex(CIntermediateCode *tempi,U8 b)
{
  if (b)
    tempi->ic_body[tempi->ic_cnt++]=b;
}

U0 ICOpSizeRex(CIntermediateCode *tempi,U8 b)
{
  tempi->ic_body[tempi->ic_cnt++]=OC_OP_SIZE_PREFIX;
  if (b)
    tempi->ic_body[tempi->ic_cnt++]=b;
}

U0 ICU16(CIntermediateCode *tempi,U16 w)
{
  *(&tempi->ic_body[tempi->ic_cnt])(U16)=w;
  tempi->ic_cnt+=2;
}

U0 ICU24(CIntermediateCode *tempi,U32 d)
{//Writes extra harmless overhanging byte.
  *(&tempi->ic_body[tempi->ic_cnt])(U32)=d;
  tempi->ic_cnt+=3;
}

U0 ICU32(CIntermediateCode *tempi,U32 d)
{
  *(&tempi->ic_body[tempi->ic_cnt])(U32)=d;
  tempi->ic_cnt+=4;
}

U0 ICU64(CIntermediateCode *tempi,U64 q)
{
  *(&tempi->ic_body[tempi->ic_cnt])(U64)=q;
  tempi->ic_cnt+=8;
}

CIntermediateCode *ICLastIns(CIntermediateCode *tempi)
{
  if (tempi->ic_last_cnt>=0)
    return tempi;
  else if (!tempi->ic_cnt &&(tempi=OptLag1(tempi)) && tempi->ic_last_cnt>=0)
    return tempi;
  else
    return NULL;
}

U0 ICAddRSP(CIntermediateCode *tempi,I64 i,Bool optimize=TRUE)
{
  I64 j,last_cnt;
  CIntermediateCode *tempil1;
  if (optimize && (tempil1=ICLastIns(tempi))) {
    j=tempil1->ic_cnt;
    if (tempil1->ic_last_cnt==j-4 && tempil1->ic_body[j-3]==0x83 &&
	  tempil1->ic_body[j-4]==0x48) {
      if (tempil1->ic_body[j-2]==0xEC)
	j=-tempil1->ic_body[j-1](I8);
      else if (tempil1->ic_body[j-2]==0xC4)
	j=tempil1->ic_body[j-1](I8);
      else
	j=0;
    } else if (tempil1->ic_last_cnt==j-7 && tempil1->ic_body[j-6]==0x81 &&
	  tempil1->ic_body[j-7]==0x48) {
      if (tempil1->ic_body[j-5]==0xEC)
	j=-tempil1->ic_body[j-4](I32);
      else if (tempil1->ic_body[j-5]==0xC4)
	j=tempil1->ic_body[j-4](I32);
      else
	j=0;
    } else
      j=0;
    if (j) {
      if (tempi==tempil1) {
	tempi->ic_cnt=tempi->ic_last_cnt;
	i+=j;
      } else if (!(tempi->ic_flags&ICF_PREV_DELETED)) {
	tempil1->ic_flags|=ICF_DEL_PREV_INS;
	tempi->ic_flags=tempi->ic_flags&~ICF_CODE_FINAL|ICF_PREV_DELETED;
	i+=j;
      }
    }
  }
  last_cnt=tempi->ic_cnt;
  if (i>0) {
    if (i<=MAX_I8)
      ICU32(tempi,0xC48348+i<<24);
    else if (i<=MAX_I32) {
      ICU24(tempi,0xC48148);
      ICU32(tempi,i);
    } else
      throw('Compiler');
  } else if (i<0) {
    i=-i;
    if (i<=MAX_I8)
      ICU32(tempi,0xEC8348+i<<24);
    else if (i<=MAX_I32) {
      ICU24(tempi,0xEC8148);
      ICU32(tempi,i);
    } else
      throw('Compiler');
  }
  if (optimize && tempi->ic_cnt>last_cnt)
    tempi->ic_last_cnt=last_cnt;
}

extern U0 ICMov(CIntermediateCode *tempi,
	CICType t1,I64 r1,I64 d1,CICType t2,I64 r2,I64 d2,I64 ip);

#define MODR_REG			0
#define MODR_INDIRECT_REG		1
#define MODR_D8_INDIRECT_REG		2
#define MODR_D32_INDIRECT_REG		3
#define MODR_SIB_INDIRECT_REG		4
#define MODR_SIB_D8_INDIRECT_REG	5
#define MODR_SIB_D32_INDIRECT_REG	6
#define MODR_RIP_REL			7

I64 ICModr1(I64 r,CICType t2,I64 r2,I64 d2)
{//res.u8[0] is type
//res.u8[1] is REX
  //res.u8[2] is ModR
  //res.u8[3] is SIB
  I64 res=0;
  if (t2.raw_type<RT_I64)
    res.u8[1]=0x40;
  else
    res.u8[1]=0x48;
  if (r>7) {
    res.u8[1]+=4;
    r&=7;
  }
  switch (Bsr(t2)) {
    case MDf_REG:
      if (r2>7) {
	res.u8[1]++;
	r2&=7;
      }
      res.u8[2]=0xC0+r<<3+r2;
      res.u8[0]=MODR_REG;
      if (res.u8[1]==0x40 && (t2.raw_type>=RT_I16 || r<4 && r2<4))
	res.u8[1]=0;
      break;
    case MDf_DISP:
      if (r2>7) {
	res.u8[1]++;
	r2&=7;
      }
      if (!d2 && r2!=REG_RBP) {
	res.u8[2]=r<<3+r2;
	res.u8[0]=MODR_INDIRECT_REG;
      } else if (MIN_I8<=d2<=MAX_I8) {
	res.u8[2]=0x40+r<<3+r2;
	res.u8[0]=MODR_D8_INDIRECT_REG;
      } else {
	res.u8[2]=0x80+r<<3+r2;
	res.u8[0]=MODR_D32_INDIRECT_REG;
      }
      if (res.u8[1]==0x40 && (t2.raw_type>=RT_I16 || r<4))
	res.u8[1]=0;
      break;
    case MDf_SIB:
      if (7<r2.u8[0]<REG_NONE)
	res.u8[1]++;
      if (r2.u8[1]&15>7)
	res.u8[1]+=2;
      if (r2.u8[0]==REG_NONE) {
	res.u8[3]=5+(r2.u8[1]&7)<<3+r2.u8[1]&0xC0;
	res.u8[2]=4+r<<3;
	res.u8[0]=MODR_SIB_D32_INDIRECT_REG;
      } else {
	res.u8[3]=r2.u8[0]&7+(r2.u8[1]&7)<<3+r2.u8[1]&0xC0;
	if (!d2 && r2.u8[0]&7!=REG_RBP) {
	  res.u8[2]=4+r<<3;
	  res.u8[0]=MODR_SIB_INDIRECT_REG;
	} else if (MIN_I8<=d2<=MAX_I8) {
	  res.u8[2]=0x44+r<<3;
	  res.u8[0]=MODR_SIB_D8_INDIRECT_REG;
	} else {
	  res.u8[2]=0x84+r<<3;
	  res.u8[0]=MODR_SIB_D32_INDIRECT_REG;
	}
      }
      if (res.u8[1]==0x40 && (t2.raw_type>=RT_I16 || r<4))
	res.u8[1]=0;
      break;
    case MDf_RIP_DISP32:
      res.u8[2]=0x05+r<<3;
      res.u8[0]=MODR_RIP_REL;
      if (res.u8[1]==0x40 && (t2.raw_type>=RT_I16 || r<4))
	res.u8[1]=0;
      break;
  }
  return res;
}

U0 ICModr2(CIntermediateCode *tempi,I64 i,I64 d,I64 ip=0)
{//t=0 means no SIB
  switch [i.u8[0]] {
    case MODR_REG:
      break;
    case MODR_INDIRECT_REG:
      break;
    case MODR_D8_INDIRECT_REG:
      ICU8(tempi,d);
      break;
    case MODR_D32_INDIRECT_REG:
      ICU32(tempi,d);
      break;
    case MODR_SIB_INDIRECT_REG:
      ICU8(tempi,i.u8[3]);
      break;
    case MODR_SIB_D8_INDIRECT_REG:
      ICU8(tempi,i.u8[3]);
      ICU8(tempi,d);
      break;
    case MODR_SIB_D32_INDIRECT_REG:
      ICU8(tempi,i.u8[3]);
      ICU32(tempi,d);
      break;
    case MODR_RIP_REL:
      ICU32(tempi,d-(ip+tempi->ic_cnt));
      tempi->ic_flags&=~ICF_CODE_FINAL;
      break;
  }
}

#define SLASH_OP_INC	0x0003000000FFFE00
#define SLASH_OP_DEC	0x052B000000FFFE01
#define SLASH_OP_NOT	0x0000000000F7F602
#define SLASH_OP_NEG	0x0000000000F7F603
#define SLASH_OP_MUL	0x0000000000F7F604
#define SLASH_OP_IMUL	0x0000000000F7F605
#define SLASH_OP_DIV	0x0000000000F7F606
#define SLASH_OP_MOV	0x0000000000898800
#define SLASH_OP_PUSH	0x0000000000FFFF06
#define SLASH_OP_POP	0x00000000008F8F00
#define SLASH_OP_FADD	0x0000C1DE01DCDC00
#define SLASH_OP_FSUB	0x0000E9DE01DCDC04
#define SLASH_OP_FSUBR	0x0000E1DE01DCDC05
#define SLASH_OP_FMUL	0x0000C9DE01DCDC01
#define SLASH_OP_FDIV	0x0000F9DE01DCDC06
#define SLASH_OP_FDIVR	0x0000F1DE01DCDC07
#define SLASH_OP_FLD	0x0000000001DDDD00
#define SLASH_OP_FSTP	0x0000000001DDDD03
#define SLASH_OP_FISTTP	0x0000000001DDDD01
#define SLASH_OP_FILD	0x0000000001DFDF05

U0 ICSlashOp(CIntermediateCode *tempi,CICType t1,I64 r1,I64 d1,I64 op,I64 ip)
{
  I64 i;
  if (t1&MDF_REG && !op.u8[3])
    t1=t1&(MDG_MASK|RTF_UNSIGNED)+RT_I64; //Set to 64 bit,preserving unsigned
  i=ICModr1(op.u8[0],t1,r1,d1);

  if (tempi->ic_flags&ICF_LOCK && op!=SLASH_OP_MOV && !(t1&MDF_REG))
    ICU8(tempi,OC_LOCK_PREFIX);
  switch (t1.raw_type) {
    case RT_I8:
    case RT_U8:
      ICRex(tempi,i.u8[1]);
      ICU16(tempi,i.u8[2]<<8+op.u8[1]);
      break;
    case RT_I16:
    case RT_U16:
      ICOpSizeRex(tempi,i.u8[1]);
      ICU16(tempi,i.u8[2]<<8+op.u8[2]);
      break;
    default:
      if (i.u8[1]!=0x48 || !op.u8[3])
	ICRex(tempi,i.u8[1]);
      ICU16(tempi,i.u8[2]<<8+op.u8[2]);
  }
  ICModr2(tempi,i,d1,ip+4);
}

U0 ICPush(CIntermediateCode *tempi,CICType t1,I64 r1,I64 d1,I64 ip)
{
  switch (Bsr(t1)) {
    case MDf_REG:
      if (r1>7)
	ICU16(tempi,0x5049+(r1&7)<<8);
      else
	ICU8(tempi,0x50+r1);
      return;
    case MDf_IMM:
      if (MIN_I8<=d1<=MAX_I8)
	ICU16(tempi,0x6A+d1<<8);
      else if (MIN_I32<=d1<=MAX_I32) {
	ICU8(tempi,0x68);
	ICU32(tempi,d1);
      } else {
	ICMov(tempi,MDF_REG+RT_I64,REG_RBX,0,t1,r1,d1,ip);
	ICU8(tempi,0x50+REG_RBX);
      }
      return;
    case MDf_STK:
      return;
    case MDf_DISP:
    case MDf_SIB:
    case MDf_RIP_DISP32:
      switch (t1.raw_type) {
	case RT_I64:
	case RT_U64:
	case RT_F64:
	  ICSlashOp(tempi,t1,r1,d1,SLASH_OP_PUSH,ip);
	  return;
      }
      break;
  }
  ICMov(tempi,MDF_REG+RT_I64,REG_RBX,0,t1,r1,d1,ip);
  ICU16(tempi,0x5048+REG_RBX<<8);
}

U0 ICPushRegs(CIntermediateCode *tempi,I64 mask)
{
  I64 i;
  for (i=0;i<NUM_REGS;i++) {
    if (Bt(&mask,i)) {
      if (i>7)
	ICU16(tempi,0x5049+(i&7)<<8);
      else
	ICU8(tempi,0x50+i);
    }
  }
}

U0 ICPop(CIntermediateCode *tempi,CICType t1,I64 r1,I64 d1,I64 ip)
{
  switch (Bsr(t1)) {
    case MDf_REG:
      if (r1>7)
	ICU16(tempi,0x5849+(r1&7)<<8);
      else
	ICU8(tempi,0x58+r1);
      break;
    case MDf_DISP:
    case MDf_RIP_DISP32:
    case MDf_SIB:
      if (t1.raw_type<RT_I64) {
	ICU8(tempi,0x58+REG_RBX);
	ICMov(tempi,t1,r1,d1,MDF_REG+RT_I64,REG_RBX,0,ip);
      } else
	ICSlashOp(tempi,t1,r1,d1,SLASH_OP_POP,ip);
      break;
    case MDf_STK:
    case MDf_IMM:
      ICU8(tempi,0x58+REG_RBX);
      ICMov(tempi,t1,r1,d1,MDF_REG+RT_I64,REG_RBX,0,ip);
      break;
    default:
      ICAddRSP(tempi,8);
  }
}

U0 ICPopRegs(CIntermediateCode *tempi,I64 mask)
{
  I64 i;
  for (i=NUM_REGS-1;i>=0;i--) {
    if (Bt(&mask,i)) {
      if (i>7)
	ICU16(tempi,0x5849+(i&7)<<8);
      else
	ICU8(tempi,0x58+i);
    }
  }
}

U0 ICZero(CIntermediateCode *tempi,I64 r)
{
  if (r>7) {
    r&=7;
    ICU24(tempi,0xC0334D+r<<16+r<<19);
  } else
    ICU16(tempi,0xC033+r<<8+r<<11);
}

U0 ICTest(CIntermediateCode *tempi,I64 r)
{
  I64 i=0xC08548; //TEST R,R
  if (r>7) {
    i+=5;
    r&=7;
  }
  ICU24(tempi,i+r<<16+r<<19);
}

I64 ICBuiltInFloatConst(F64 d)
{//Returns 2-byte opcode for FLD const or zero
  if (!d)
    return 0xEED9;
  else if (d==1.0)
    return 0xE8D9;
  else if (GetOption(OPTf_NO_BUILTIN_CONST))
    return 0;
  else if (d==ã)
    return 0xEBD9;
  else if (d==log2_10)
    return 0xE9D9;
  else if (d==log2_e)
    return 0xEAD9;
  else if (d==log10_2)
    return 0xECD9;
  else if (d==loge_2)
    return 0xEDD9;
  else
    return 0;
}

U0 ICMov(CIntermediateCode *tempi,
	CICType t1,I64 r1,I64 d1,CICType t2,I64 r2,I64 d2,I64 ip)
{
  I64 i,j,last_cnt=tempi->ic_cnt;
  CIntermediateCode *tempil1;
  Bool old_lock=Btr(&tempi->ic_flags,ICf_LOCK);
  switch (Bsr(t1)) {
    case MDf_REG:
      if (t2&MDF_IMM) {
	if (!d2)
	  ICZero(tempi,r1);
	else if (0<=d2<=MAX_U8) {
	  ICZero(tempi,r1);
	  if (r1>7)
	    ICU24(tempi,d2<<16+(0xB0+r1&7)<<8+0x41);
	  else if (r1>3)
	    ICU24(tempi,d2<<16+(0xB0+r1)<<8+0x40);
	  else
	    ICU16(tempi,d2<<8+0xB0+r1);
	} else if (MIN_I8<=d2<0) {
	  if (r1>7) {
	    r1&=7;
	    ICU24(tempi,d2<<16+(0xB0+r1)<<8+0x41);
	    ICU32(tempi,0xC0BE0F4D+r1<<24+r1<<27);
	  } else {
	    if (r1>3)
	      ICU24(tempi,d2<<16+(0xB0+r1)<<8+0x40);
	    else
	      ICU16(tempi,d2<<8+0xB0+r1);
	    ICU32(tempi,0xC0BE0F48+r1<<24+r1<<27);
	  }
	} else if (0<=d2<=MAX_U32) {
	  if (r1>7) {
	    r1&=7;
	    ICU16(tempi,(0xB8+r1)<<8+0x41);
	    ICU32(tempi,d2);
	  } else {
	    ICU8(tempi,0xB8+r1);
	    ICU32(tempi,d2);
	  }
	} else if (MIN_I32<=d2<0) {
	  if (r1>7) {
	    r1&=7;
	    ICU16(tempi,(0xB8+r1)<<8+0x41);
	    ICU32(tempi,d2);
	    ICU24(tempi,0xC0634D+r1<<16+r1<<19);
	  } else {
	    ICU8(tempi,0xB8+r1);
	    ICU32(tempi,d2);
	    ICU24(tempi,0xC06348+r1<<16+r1<<19);
	  }
	} else {
	  i=0xB848;
	  if (r1>7) {
	    i++;
	    r1&=7;
	  }
	  ICU16(tempi,i+r1<<8);
	  ICU64(tempi,d2);
	}
      } else if (t2&MDF_STK)
	ICPop(tempi,t1,r1,d1,ip);
      else {
	if (r1==r2 && t2&MDF_REG)
	  goto move_done;
	if (t2&MDF_REG) {
	  t2=MDF_REG+RT_I64;
	  if (tempil1=ICLastIns(tempi)) {
	    i=tempil1->ic_cnt;
	    j=0x48+(r1>7);
	    if (r2>7) j+=4;
	    if (tempil1->ic_last_cnt==i-3 && tempil1->ic_body[i-2]==0x8B &&
		  tempil1->ic_body[i-1]==0xC0+r1&7+(r2&7)<<3 &&
		  tempil1->ic_body[i-3]==j) {
	      goto move_done;
	    }
	  }
	}
	i=ICModr1(r1,t2,r2,d2);
	if (t2.raw_type!=RT_U32)
	  i|=0x4800;
	ICRex(tempi,i.u8[1]);
	switch (t2.raw_type) {
	  case RT_I8:
	    ICU24(tempi,i.u8[2]<<16+0xBE0F);
	    break;
	  case RT_I16:
	    ICU24(tempi,i.u8[2]<<16+0xBF0F);
	    break;
	  case RT_I32:
	    ICU16(tempi,i.u8[2]<<8+0x63);
	    break;

	  case RT_U8:
	    ICU24(tempi,i.u8[2]<<16+0xB60F);
	    break;
	  case RT_U16:
	    ICU24(tempi,i.u8[2]<<16+0xB70F);
	    break;
	  default:
	    ICU16(tempi,i.u8[2]<<8+0x8B);
	}
	ICModr2(tempi,i,d2,ip+4);
      }
      break;
    case MDf_STK:
      if (tempi->ic_flags&ICF_PUSH_CMP)
	ICPopRegs(tempi,1<<REG_RBX);
      if (t1.raw_type<t2.raw_type)
	ICPush(tempi,t2&MDG_MASK+t1.raw_type,r2,d2,ip);
      else
	ICPush(tempi,t2,r2,d2,ip);
      if (tempi->ic_flags&ICF_PUSH_CMP)
	ICPushRegs(tempi,1<<REG_RBX);
      break;
    case MDf_DISP:
    case MDf_RIP_DISP32:
    case MDf_SIB:
      if (t2&MDF_IMM && (t1.raw_type<RT_I64 || (MIN_I32<=d2<=MAX_I32))) {
	i=ICModr1(0,t1,r1,d1);
	switch (t1.raw_type) {
	  case RT_I8:
	  case RT_U8:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xC6);
	    break;
	  case RT_U16:
	  case RT_I16:
	    ICOpSizeRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xC7);
	    break;
	  default:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xC7);
	}
	if (i.u8[0]!=MODR_RIP_REL)
	  ICModr2(tempi,i,d1,);
	else {
	  i=d1-(ip+tempi->ic_cnt+4);
	  switch (t1.raw_type) {
	    case RT_I8:
	    case RT_U8:
	      i--;
	      break;
	    case RT_I16:
	    case RT_U16:
	      i-=2;
	      break;
	    default:
	      i-=4;
	  }
	  ICU32(tempi,i);
	  tempi->ic_flags&=~ICF_CODE_FINAL;
	}

	switch (t1.raw_type) {
	  case RT_I8:
	  case RT_U8:
	    ICU8(tempi,d2);
	    break;
	  case RT_I16:
	  case RT_U16:
	    ICU16(tempi,d2);
	    break;
	  default:
	    ICU32(tempi,d2);
	}
      } else {
	if (t2&MDF_REG)
	  ICSlashOp(tempi,t1,r1,d1,r2+SLASH_OP_MOV,ip);
	else {
	  ICMov(tempi,MDF_REG+RT_I64,REG_RBX,0,t2,r2,d2,ip);
	  ICMov(tempi,t1,r1,d1,MDF_REG+RT_I64,REG_RBX,0,ip);
	}
      }
      break;
  }
move_done:
  BEqu(&tempi->ic_flags,ICf_LOCK,old_lock);
  if (tempi->ic_cnt>last_cnt>tempi->ic_last_cnt)
    tempi->ic_last_cnt=last_cnt;
}

U0 ICLea(CIntermediateCode *tempi,CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,CCmpCtrl *cc,U8 *buf,I64 ip)
{
  I64 i;
  CAOTAbsAddr *tempa;
  switch (Bsr(t1)) {
    case MDf_REG:
      i=ICModr1(r1,t2,r2,d2);
      i.u8[1]|=0x48;
      ICU24(tempi,i.u8[2]<<16+0x8D00+i.u8[1]);
      ICModr2(tempi,i,d2,ip+4);
      break;
    case MDf_STK:
      if (t2&MDF_RIP_DISP32) {
	ICU8(tempi,0x68);
	ICU32(tempi,d2);
	if (cc->flags&CCF_AOT_COMPILE && buf && !(cc->flags&CCF_NO_ABSS)) {
	  tempa=CAlloc(sizeof(CAOTAbsAddr));
	  tempa->next=cc->aotc->abss;
	  tempa->type=AAT_ADD_U32;
	  cc->aotc->abss=tempa;
	  tempa->ip=ip+tempi->ic_cnt-4;
	}
	tempi->ic_flags&=~ICF_CODE_FINAL;
	break;
      } // Fall thru
    default:
      ICLea(tempi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,cc,buf,ip);
      ICMov(tempi,t1,r1,d1,MDF_REG+RT_I64,REG_RCX,0,ip);
  }
}

U0 ICDeref(CIntermediateCode *tempi,I64 ip)
{
  CICType t;
  t=tempi->r.type.raw_type;
  if (t>tempi->a1.type_pointed_to)
    t=tempi->a1.type_pointed_to;
  if (tempi->a1.type&MDF_REG)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  MDF_DISP+t,tempi->a1.reg,tempi->a1.disp,ip);
  else {
    ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  MDF_DISP+t,REG_RCX,0,ip);
  }
}
