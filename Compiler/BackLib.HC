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

U0 ICU8(CIntermediateCode *tmpi,U8 b)
{
  tmpi->ic_body[tmpi->ic_cnt++]=b;
}

U0 ICRex(CIntermediateCode *tmpi,U8 b)
{
  if (b)
    tmpi->ic_body[tmpi->ic_cnt++]=b;
}

U0 ICOpSizeRex(CIntermediateCode *tmpi,U8 b)
{
  tmpi->ic_body[tmpi->ic_cnt++]=OC_OP_SIZE_PREFIX;
  if (b)
    tmpi->ic_body[tmpi->ic_cnt++]=b;
}

U0 ICU16(CIntermediateCode *tmpi,U16 w)
{
  *(&tmpi->ic_body[tmpi->ic_cnt])(U16)=w;
  tmpi->ic_cnt+=2;
}

U0 ICU24(CIntermediateCode *tmpi,U32 d)
{//Writes extra harmless overhanging byte.
  *(&tmpi->ic_body[tmpi->ic_cnt])(U32)=d;
  tmpi->ic_cnt+=3;
}

U0 ICU32(CIntermediateCode *tmpi,U32 d)
{
  *(&tmpi->ic_body[tmpi->ic_cnt])(U32)=d;
  tmpi->ic_cnt+=4;
}

U0 ICU64(CIntermediateCode *tmpi,U64 q)
{
  *(&tmpi->ic_body[tmpi->ic_cnt])(U64)=q;
  tmpi->ic_cnt+=8;
}

U0 ICAddRSP(CIntermediateCode *tmpi,I64 i,Bool optimize=TRUE)
{
  I64 j,last_start;
  CIntermediateCode *tmpil1;
  if (optimize) {
    tmpil1=tmpi;
    if (tmpi->ic_last_start<0 && !tmpi->ic_cnt &&
	  (tmpil1=OptLag1(tmpi)) && tmpil1->ic_last_start<0)
      tmpil1=NULL;
    if (tmpil1) {
      j=tmpil1->ic_cnt;
      if (tmpil1->ic_last_start==j-4 && tmpil1->ic_body[j-3]==0x83 &&
	    tmpil1->ic_body[j-4]==0x48) {
	if (tmpil1->ic_body[j-2]==0xEC)
	  j=-tmpil1->ic_body[j-1](I8);
	else if (tmpil1->ic_body[j-2]==0xC4)
	  j=tmpil1->ic_body[j-1](I8);
	else
	  j=0;
      } else if (tmpil1->ic_last_start==j-7 && tmpil1->ic_body[j-6]==0x81 &&
	    tmpil1->ic_body[j-7]==0x48) {
	if (tmpil1->ic_body[j-5]==0xEC)
	  j=-tmpil1->ic_body[j-4](I32);
	else if (tmpil1->ic_body[j-5]==0xC4)
	  j=tmpil1->ic_body[j-4](I32);
	else
	  j=0;
      } else
	j=0;
      if (j) {
	if (tmpi==tmpil1) {
	  tmpi->ic_cnt=tmpi->ic_last_start;
	  i+=j;
	} else if (!(tmpi->ic_flags&ICF_PREV_DELETED)) {
	  tmpil1->ic_flags|=ICF_DEL_PREV_INS;
	  tmpi->ic_flags=tmpi->ic_flags&~ICF_CODE_FINAL|ICF_PREV_DELETED;
	  i+=j;
	}
      }
    }
  }
  last_start=tmpi->ic_cnt;
  if (i>0) {
    if (i<=I8_MAX)
      ICU32(tmpi,0xC48348+i<<24);
    else if (i<=I32_MAX) {
      ICU24(tmpi,0xC48148);
      ICU32(tmpi,i);
    } else
      throw('Compiler');
  } else if (i<0) {
    i=-i;
    if (i<=I8_MAX)
      ICU32(tmpi,0xEC8348+i<<24);
    else if (i<=I32_MAX) {
      ICU24(tmpi,0xEC8148);
      ICU32(tmpi,i);
    } else
      throw('Compiler');
  }
  if (optimize && tmpi->ic_cnt>last_start)
    tmpi->ic_last_start=last_start;
}

extern U0 ICMov(CIntermediateCode *tmpi,
	CICType t1,I64 r1,I64 d1,CICType t2,I64 r2,I64 d2,I64 rip);

#define MODR_REG			0
#define MODR_INDIRECT_REG		1
#define MODR_D8_INDIRECT_REG		2
#define MODR_D32_INDIRECT_REG		3
#define MODR_SIB_INDIRECT_REG		4
#define MODR_SIB_D8_INDIRECT_REG	5
#define MODR_SIB_D32_INDIRECT_REG	6
#define MODR_RIP_REL			7
#define MODR_RIP_REL_IMM_U32		8

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
      } else if (I8_MIN<=d2<=I8_MAX) {
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
	} else if (I8_MIN<=d2<=I8_MAX) {
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

U0 ICModr2(CIntermediateCode *tmpi,I64 i,CICType t=0,I64 d,I64 rip=0)
{
  switch [i.u8[0]] {
    case MODR_REG:
      break;
    case MODR_INDIRECT_REG:
      break;
    case MODR_D8_INDIRECT_REG:
      ICU8(tmpi,d);
      break;
    case MODR_D32_INDIRECT_REG:
      ICU32(tmpi,d);
      break;
    case MODR_SIB_INDIRECT_REG:
      ICU8(tmpi,i.u8[3]);
      break;
    case MODR_SIB_D8_INDIRECT_REG:
      ICU8(tmpi,i.u8[3]);
      ICU8(tmpi,d);
      break;
    case MODR_SIB_D32_INDIRECT_REG:
      ICU8(tmpi,i.u8[3]);
      ICU32(tmpi,d);
      break;
    case MODR_RIP_REL_IMM_U32:
      switch (t.raw_type) {
	case RT_I8:
	case RT_U8:
	  d--;
	  break;
	case RT_I16:
	case RT_U16:
	  d-=2;
	  break;
	default:
	  d-=4;
      }
    case MODR_RIP_REL:
      ICU32(tmpi,d-(rip+4+tmpi->ic_cnt));
      tmpi->ic_flags&=~ICF_CODE_FINAL;
      break;
  }
}

#define SLASH_OP_INC		0x0003000000FFFE00
#define SLASH_OP_DEC		0x052B000000FFFE01
#define SLASH_OP_NOT		0x0000000000F7F602
#define SLASH_OP_NEG		0x0000000000F7F603
#define SLASH_OP_IMM_U8		0x0000000000838000
#define SLASH_OP_IMM_U32	0x0000000000818300
#define SLASH_OP_MUL		0x0000000000F7F604
#define SLASH_OP_IMUL		0x0000000000F7F605
#define SLASH_OP_DIV		0x0000000000F7F606
#define SLASH_OP_MOV		0x0000000000898800
#define SLASH_OP_MOV_IMM	0x0000000000C7C600
#define SLASH_OP_PUSH		0x0000000000FFFF06
#define SLASH_OP_POP		0x00000000008F8F00
#define SLASH_OP_FADD		0x0000C1DE01DCDC00
#define SLASH_OP_FSUB		0x0000E9DE01DCDC04
#define SLASH_OP_FSUBR		0x0000E1DE01DCDC05
#define SLASH_OP_FMUL		0x0000C9DE01DCDC01
#define SLASH_OP_FDIV		0x0000F9DE01DCDC06
#define SLASH_OP_FDIVR		0x0000F1DE01DCDC07
#define SLASH_OP_FLD		0x0000000001DDDD00
#define SLASH_OP_FSTP		0x0000000001DDDD03
#define SLASH_OP_FISTTP		0x0000000001DDDD01
#define SLASH_OP_FILD		0x0000000001DFDF05

U0 ICSlashOp(CIntermediateCode *tmpi,CICType t1,I64 r1,I64 d1,I64 op,I64 rip)
{
  I64 i;
  if (t1&MDF_REG && !op.u8[3])
    t1=t1&(MDG_MASK|RTF_UNSIGNED)+RT_I64; //Set to 64 bit,preserving unsigned
  i=ICModr1(op.u8[0],t1,r1,d1);
  if (tmpi->ic_flags&ICF_LOCK && !(t1&MDF_REG) &&
	op&~7!=SLASH_OP_MOV && op!=SLASH_OP_MOV_IMM)
    ICU8(tmpi,OC_LOCK_PREFIX);
  switch (t1.raw_type) {
    case RT_I8:
    case RT_U8:
      ICRex(tmpi,i.u8[1]);
      ICU16(tmpi,i.u8[2]<<8+op.u8[1]);
      break;
    case RT_I16:
    case RT_U16:
      ICOpSizeRex(tmpi,i.u8[1]);
      ICU16(tmpi,i.u8[2]<<8+op.u8[2]);
      break;
    default:
      if (i.u8[1]!=0x48 || !op.u8[3])
	ICRex(tmpi,i.u8[1]);
      ICU16(tmpi,i.u8[2]<<8+op.u8[2]);
  }
  if (i.u8[0]==MODR_RIP_REL&& (op==SLASH_OP_MOV_IMM || op&~7==SLASH_OP_IMM_U32))
    i.u8[0]=MODR_RIP_REL_IMM_U32;
  ICModr2(tmpi,i,t1,d1,rip);
}

U0 ICPush(CIntermediateCode *tmpi,CICType t1,I64 r1,I64 d1,I64 rip)
{
  switch (Bsr(t1)) {
    case MDf_REG:
      if (r1>7)
	ICU16(tmpi,0x5049+(r1&7)<<8);
      else
	ICU8(tmpi,0x50+r1);
      return;
    case MDf_IMM:
      if (I8_MIN<=d1<=I8_MAX)
	ICU16(tmpi,0x6A+d1<<8);
      else if (I32_MIN<=d1<=I32_MAX) {
	ICU8(tmpi,0x68);
	ICU32(tmpi,d1);
      } else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,t1,r1,d1,rip);
	ICU8(tmpi,0x50+REG_RBX);
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
	  ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_PUSH,rip);
	  return;
      }
      break;
  }
  ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,t1,r1,d1,rip);
  ICU16(tmpi,0x5048+REG_RBX<<8);
}

U0 ICPushRegs(CIntermediateCode *tmpi,I64 mask)
{
  I64 i;
  for (i=0;i<REG_REGS_NUM;i++) {
    if (Bt(&mask,i)) {
      if (i>7)
	ICU16(tmpi,0x5049+(i&7)<<8);
      else
	ICU8(tmpi,0x50+i);
    }
  }
}

U0 ICPop(CIntermediateCode *tmpi,CICType t1,I64 r1,I64 d1,I64 rip)
{
  switch (Bsr(t1)) {
    case MDf_REG:
      if (r1>7)
	ICU16(tmpi,0x5849+(r1&7)<<8);
      else
	ICU8(tmpi,0x58+r1);
      break;
    case MDf_DISP:
    case MDf_RIP_DISP32:
    case MDf_SIB:
      if (t1.raw_type<RT_I64) {
	ICU8(tmpi,0x58+REG_RBX);
	ICMov(tmpi,t1,r1,d1,MDF_REG+RT_I64,REG_RBX,0,rip);
      } else
	ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_POP,rip);
      break;
    case MDf_STK:
    case MDf_IMM:
      ICU8(tmpi,0x58+REG_RBX);
      ICMov(tmpi,t1,r1,d1,MDF_REG+RT_I64,REG_RBX,0,rip);
      break;
    default:
      ICAddRSP(tmpi,8);
  }
}

U0 ICPopRegs(CIntermediateCode *tmpi,I64 mask)
{
  I64 i;
  for (i=REG_REGS_NUM-1;i>=0;i--) {
    if (Bt(&mask,i)) {
      if (i>7)
	ICU16(tmpi,0x5849+(i&7)<<8);
      else
	ICU8(tmpi,0x58+i);
    }
  }
}

U0 ICZero(CIntermediateCode *tmpi,I64 r)
{
  if (r>7) {
    r&=7;
    ICU24(tmpi,0xC0334D+r<<16+r<<19);
  } else
    ICU16(tmpi,0xC033+r<<8+r<<11);
}

U0 ICTest(CIntermediateCode *tmpi,I64 r)
{
  I64 i=0xC08548; //TEST R,R
  if (r>7) {
    i+=5;
    r&=7;
  }
  ICU24(tmpi,i+r<<16+r<<19);
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

U0 ICMov(CIntermediateCode *tmpi,
	CICType t1,I64 r1,I64 d1,CICType t2,I64 r2,I64 d2,I64 rip)
{
  I64 i,cnt1,cnt2,b1_rex,b2_rex,b1,b2,b1_modr,b2_modr,
	b1_r1,b1_r2,b2_r1,b2_r2,last_start=tmpi->ic_cnt;
  CIntermediateCode *tmpil1;
  Bool old_lock=Btr(&tmpi->ic_flags,ICf_LOCK);
  switch (Bsr(t1)) {
    case MDf_REG:
      if (t2&MDF_IMM) {
	if (!d2)
	  ICZero(tmpi,r1);
	else if (0<=d2<=U8_MAX) {
	  ICZero(tmpi,r1);
	  if (r1>7)
	    ICU24(tmpi,d2<<16+(0xB0+r1&7)<<8+0x41);
	  else if (r1>3)
	    ICU24(tmpi,d2<<16+(0xB0+r1)<<8+0x40);
	  else
	    ICU16(tmpi,d2<<8+0xB0+r1);
	} else if (I8_MIN<=d2<0) {
	  if (r1>7) {
	    r1&=7;
	    ICU24(tmpi,d2<<16+(0xB0+r1)<<8+0x41);
	    ICU32(tmpi,0xC0BE0F4D+r1<<24+r1<<27);
	  } else {
	    if (r1>3)
	      ICU24(tmpi,d2<<16+(0xB0+r1)<<8+0x40);
	    else
	      ICU16(tmpi,d2<<8+0xB0+r1);
	    ICU32(tmpi,0xC0BE0F48+r1<<24+r1<<27);
	  }
	} else if (0<=d2<=U32_MAX) {
	  if (r1>7) {
	    r1&=7;
	    ICU16(tmpi,(0xB8+r1)<<8+0x41);
	    ICU32(tmpi,d2);
	  } else {
	    ICU8(tmpi,0xB8+r1);
	    ICU32(tmpi,d2);
	  }
	} else if (I32_MIN<=d2<0) {
	  if (r1>7) {
	    r1&=7;
	    ICU16(tmpi,(0xB8+r1)<<8+0x41);
	    ICU32(tmpi,d2);
	    ICU24(tmpi,0xC0634D+r1<<16+r1<<19);
	  } else {
	    ICU8(tmpi,0xB8+r1);
	    ICU32(tmpi,d2);
	    ICU24(tmpi,0xC06348+r1<<16+r1<<19);
	  }
	} else {
	  i=0xB848;
	  if (r1>7) {
	    i++;
	    r1&=7;
	  }
	  ICU16(tmpi,i+r1<<8);
	  ICU64(tmpi,d2);
	}
      } else if (t2&MDF_STK)
	ICPop(tmpi,t1,r1,d1,rip);
      else {
	if (r1==r2 && t2&MDF_REG)
	  goto move_done;
	if (t2&MDF_REG)
	  t2=MDF_REG+RT_I64;
	i=ICModr1(r1,t2,r2,d2);
	if (t2.raw_type!=RT_U32)
	  i|=0x4800;
	ICRex(tmpi,i.u8[1]);
	switch (t2.raw_type) {
	  case RT_I8:
	    ICU24(tmpi,i.u8[2]<<16+0xBE0F);
	    break;
	  case RT_I16:
	    ICU24(tmpi,i.u8[2]<<16+0xBF0F);
	    break;
	  case RT_I32:
	    ICU16(tmpi,i.u8[2]<<8+0x63);
	    break;

	  case RT_U8:
	    ICU24(tmpi,i.u8[2]<<16+0xB60F);
	    break;
	  case RT_U16:
	    ICU24(tmpi,i.u8[2]<<16+0xB70F);
	    break;
	  default:
	    ICU16(tmpi,i.u8[2]<<8+0x8B);
	}
	ICModr2(tmpi,i,,d2,rip);
      }
      break;
    case MDf_STK:
      if (tmpi->ic_flags&ICF_PUSH_CMP)
	ICPopRegs(tmpi,1<<REG_RBX);
      if (t1.raw_type<t2.raw_type)
	ICPush(tmpi,t2&MDG_MASK+t1.raw_type,r2,d2,rip);
      else
	ICPush(tmpi,t2,r2,d2,rip);
      if (tmpi->ic_flags&ICF_PUSH_CMP)
	ICPushRegs(tmpi,1<<REG_RBX);
      break;
    case MDf_DISP:
    case MDf_RIP_DISP32:
    case MDf_SIB:
      if (t2&MDF_IMM && (t1.raw_type<RT_I64 || (I32_MIN<=d2<=I32_MAX))) {
	ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_MOV_IMM,rip);
	switch (t1.raw_type) {
	  case RT_I8:
	  case RT_U8:
	    ICU8(tmpi,d2);
	    break;
	  case RT_I16:
	  case RT_U16:
	    ICU16(tmpi,d2);
	    break;
	  default:
	    ICU32(tmpi,d2);
	}
      } else {
	if (t2&MDF_REG)
	  ICSlashOp(tmpi,t1,r1,d1,r2+SLASH_OP_MOV,rip);
	else {
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,t2,r2,d2,rip);
	  ICMov(tmpi,t1,r1,d1,MDF_REG+RT_I64,REG_RBX,0,rip);
	}
      }
      break;
  }
move_done:
  if (!((t1|t2)&(MDF_STK|MDF_RIP_DISP32))) {
    tmpil1=tmpi;
    if (tmpi->ic_last_start<0 && (tmpil1=OptLag1(tmpi)) &&
	  tmpil1->ic_last_start<0)
      tmpil1=NULL;
    if (tmpil1) {
      if (tmpil1==tmpi)
	cnt1=last_start-tmpil1->ic_last_start;
      else {
	if (!(tmpil1->ic_flags&ICF_CODE_FINAL))
	  tmpi->ic_flags&=~ICF_CODE_FINAL;
	if (last_start)
	  cnt1=0;
	else
	  cnt1=tmpil1->ic_cnt-tmpil1->ic_last_start;
      }
      cnt2=tmpi->ic_cnt-last_start;
      if (cnt1 && cnt1==cnt2) {
	b1_rex=tmpil1->ic_body[tmpil1->ic_last_start];
	b2_rex=tmpi->ic_body[last_start];
	if (b1_rex&0x48==0x48 && b2_rex&0x48==0x48) {
	  for (i=1;i<cnt1;i++)
	    if ((b1=tmpil1->ic_body[tmpil1->ic_last_start+i])==
		  (b2=tmpi->ic_body[last_start+i])) {
	      if (i==1 && (b2==0x89 || b2==0x8B)) {
		b1_modr=tmpil1->ic_body[tmpil1->ic_last_start+2];
		b1_r1=b1_modr&7	+Bt(&b1_rex,0)<<3;
		b1_r2=b1_modr>>3&7+Bt(&b1_rex,2)<<3;
		b2_modr=tmpi->ic_body[last_start+2];
		b2_r1=b2_modr&7	  +Bt(&b2_rex,0)<<3;
		b2_r2=b2_modr>>3&7+Bt(&b2_rex,2)<<3;
		if (cnt1==3 && b2_modr&0xC0==0xC0) {
		  if (b2_r1==b2_r2)
		    goto move_redundant;
		  if (b1_modr&0xC0==0xC0) {
		    if (b1_r1==b2_r2 && b2_r1==b1_r2)
		      goto move_redundant;
		  }
		} else if (b1_rex!=b2_rex || b1_r1==b1_r2 || (t1|t2)&MDF_SIB)
		  break;
	      } else if (b1_rex!=b2_rex)
		break;
	    } else if (i!=1)
	      break;
	    else if (b2!=0x89 && b2!=0x8B)
	      break;
	    else {
	      b1_modr=tmpil1->ic_body[tmpil1->ic_last_start+2];
	      b1_r1=b1_modr&7	+Bt(&b1_rex,0)<<3;
	      b1_r2=b1_modr>>3&7+Bt(&b1_rex,2)<<3;
	      b2_modr=tmpi->ic_body[last_start+2];
	      b2_r1=b2_modr&7	+Bt(&b2_rex,0)<<3;
	      b2_r2=b2_modr>>3&7+Bt(&b2_rex,2)<<3;
	      if (cnt1==3 && b2_modr&0xC0==0xC0) {
		if (b2_r1==b2_r2)
		  goto move_redundant;
		if (b1==0x89 && b2==0x8B || b1==0x8B && b2==0x89) {
		  if (b1_modr&0xC0==0xC0) {
		    if (b1_r1==b2_r1 && b1_r2==b2_r2 ||
			  b1_r1==b2_r2 && b2_r1==b1_r2)
		      goto move_redundant;
		  }
		  if (b1_rex!=b2_rex)
		    break;
		} else
		  break;
	      } else if (b1_r1==b1_r2 || (t1|t2)&MDF_SIB || b1_rex!=b2_rex ||
		    !(b1==0x89 && b2==0x8B || b1==0x8B && b2==0x89))
		break;
	    }
	  if (i==cnt1) {
move_redundant:
	    tmpi->ic_cnt=last_start;
	  }
	}
      }
    }
  }
  if (tmpi->ic_cnt>last_start>tmpi->ic_last_start)
    tmpi->ic_last_start=last_start;
  BEqu(&tmpi->ic_flags,ICf_LOCK,old_lock);
}

U0 ICLea(CIntermediateCode *tmpi,CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,CCmpCtrl *cc,U8 *buf,I64 rip)
{
  I64 i;
  CAOTAbsAddr *tmpa;
  switch (Bsr(t1)) {
    case MDf_REG:
      i=ICModr1(r1,t2,r2,d2);
      i.u8[1]|=0x48;
      ICU24(tmpi,i.u8[2]<<16+0x8D00+i.u8[1]);
      ICModr2(tmpi,i,,d2,rip);
      break;
    case MDf_STK:
      if (t2&MDF_RIP_DISP32) {
	ICU8(tmpi,0x68);
	ICU32(tmpi,d2);
	if (cc->flags&CCF_AOT_COMPILE && buf && !(cc->flags&CCF_NO_ABSS)) {
	  tmpa=CAlloc(sizeof(CAOTAbsAddr));
	  tmpa->next=cc->aotc->abss;
	  tmpa->type=AAT_ADD_U32;
	  cc->aotc->abss=tmpa;
	  tmpa->rip=rip+tmpi->ic_cnt-4;
	}
	tmpi->ic_flags&=~ICF_CODE_FINAL;
	break;
      } // Fall thru
    default:
      ICLea(tmpi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,cc,buf,rip);
      ICMov(tmpi,t1,r1,d1,MDF_REG+RT_I64,REG_RCX,0,rip);
  }
}

U0 ICDeref(CIntermediateCode *tmpi,I64 rip)
{
  CICType t;
  t=tmpi->res.type.raw_type;
  if (t>tmpi->arg1_type_pointed_to)
    t=tmpi->arg1_type_pointed_to;
  if (tmpi->arg1.type&MDF_REG)
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_DISP+t,tmpi->arg1.reg,tmpi->arg1.disp,rip);
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_DISP+t,REG_RCX,0,rip);
  }
}
