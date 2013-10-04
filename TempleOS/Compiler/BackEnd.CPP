/*Intermediate Code
To
Machine Code

RAX,RBX,RCX and RDX can be clobbered by
each intermediate code's output code.
However, intermediate codes must be
coupled together based on the arg and
result type specifications in the
$LK,"CICArg","MN:CICArg"$.  RAX is the most common register
for coupling intermediate codes.

Internal calculations take place on
64-bit vals, so anything which has
found it's way into a register has been
sign or zero extended to 64-bits.

*/

U0 ICU8(CIntermediateCode *tempi,U8 b)
{
  if (tempi->ic_cnt>=IC_BODY_SIZE-1|| tempi->ic_ext_body) {
    if (!tempi->ic_ext_body) {
      tempi->ic_ext_body=MAlloc(256);
      MemCpy(tempi->ic_ext_body,tempi->ic_body,tempi->ic_cnt);
    }
    tempi->ic_ext_body[tempi->ic_cnt++]=b;
  } else
    tempi->ic_body[tempi->ic_cnt++]=b;
}

U0 ICRex(CIntermediateCode *tempi,U8 b)
{
  if (b) {
    if (tempi->ic_cnt>=IC_BODY_SIZE-1|| tempi->ic_ext_body) {
      if (!tempi->ic_ext_body) {
	tempi->ic_ext_body=MAlloc(256);
	MemCpy(tempi->ic_ext_body,tempi->ic_body,tempi->ic_cnt);
      }
      tempi->ic_ext_body[tempi->ic_cnt++]=b;
    } else
      tempi->ic_body[tempi->ic_cnt++]=b;
  }
}

U0 ICOpSizeRex(CIntermediateCode *tempi,U8 b)
{
  if (!b) {
    if (tempi->ic_cnt>=IC_BODY_SIZE-1|| tempi->ic_ext_body) {
      if (!tempi->ic_ext_body) {
	tempi->ic_ext_body=MAlloc(256);
	MemCpy(tempi->ic_ext_body,tempi->ic_body,tempi->ic_cnt);
      }
      tempi->ic_ext_body[tempi->ic_cnt++]=OC_OP_SIZE_PREFIX;
    } else
      tempi->ic_body[tempi->ic_cnt++]=OC_OP_SIZE_PREFIX;
  } else {
    if (tempi->ic_cnt>=IC_BODY_SIZE-2|| tempi->ic_ext_body) {
      if (!tempi->ic_ext_body) {
	tempi->ic_ext_body=MAlloc(256);
	MemCpy(tempi->ic_ext_body,tempi->ic_body,tempi->ic_cnt);
      }
      tempi->ic_ext_body[tempi->ic_cnt++]=OC_OP_SIZE_PREFIX;
      tempi->ic_ext_body[tempi->ic_cnt++]=b;
    } else {
      tempi->ic_body[tempi->ic_cnt++]=OC_OP_SIZE_PREFIX;
      tempi->ic_body[tempi->ic_cnt++]=b;
    }
  }
}

U0 ICU16(CIntermediateCode *tempi,U16 w)
{
  if (tempi->ic_cnt>=IC_BODY_SIZE-2|| tempi->ic_ext_body) {
    if (!tempi->ic_ext_body) {
      tempi->ic_ext_body=MAlloc(256);
      MemCpy(tempi->ic_ext_body,tempi->ic_body,tempi->ic_cnt);
    }
    tempi->ic_ext_body[tempi->ic_cnt++]=w.u8[0];
    tempi->ic_ext_body[tempi->ic_cnt++]=w.u8[1];
  } else {
    tempi->ic_body[tempi->ic_cnt++]=w.u8[0];
    tempi->ic_body[tempi->ic_cnt++]=w.u8[1];
  }
}

U0 ICU24(CIntermediateCode *tempi,U32 d)
{
  if (tempi->ic_cnt>=IC_BODY_SIZE-3|| tempi->ic_ext_body) {
    if (!tempi->ic_ext_body) {
      tempi->ic_ext_body=MAlloc(256);
      MemCpy(tempi->ic_ext_body,tempi->ic_body,tempi->ic_cnt);
    }
    tempi->ic_ext_body[tempi->ic_cnt++]=d.u8[0];
    tempi->ic_ext_body[tempi->ic_cnt++]=d.u8[1];
    tempi->ic_ext_body[tempi->ic_cnt++]=d.u8[2];
  } else {
    tempi->ic_body[tempi->ic_cnt++]=d.u8[0];
    tempi->ic_body[tempi->ic_cnt++]=d.u8[1];
    tempi->ic_body[tempi->ic_cnt++]=d.u8[2];
  }
}

U0 ICU32(CIntermediateCode *tempi,U32 d)
{
  U32 *_d;
  if (tempi->ic_cnt>=IC_BODY_SIZE-4|| tempi->ic_ext_body) {
    if (!tempi->ic_ext_body) {
      tempi->ic_ext_body=MAlloc(256);
      MemCpy(tempi->ic_ext_body,tempi->ic_body,tempi->ic_cnt);
    }
    _d=&tempi->ic_ext_body[tempi->ic_cnt];
  } else {
    _d=&tempi->ic_body[tempi->ic_cnt];
  }
  *_d=d;
  tempi->ic_cnt+=4;
}

U0 ICU64(CIntermediateCode *tempi,I64 q)
{
  I64 *qq;
  if (tempi->ic_cnt>=IC_BODY_SIZE-8 || tempi->ic_ext_body) {
    if (!tempi->ic_ext_body) {
      tempi->ic_ext_body=MAlloc(256);
      MemCpy(tempi->ic_ext_body,tempi->ic_body,tempi->ic_cnt);
    }
    qq=&tempi->ic_ext_body[tempi->ic_cnt];
  } else
    qq=&tempi->ic_body[tempi->ic_cnt];
  *qq=q;
  tempi->ic_cnt+=8;
}

Bool ICLastIns(CIntermediateCode *tempi,CIntermediateCode **_tempi1,U8 **_body)
{
  CIntermediateCode *tempil1;
  if (tempi->ic_last_cnt>=0) {
    if (tempi->ic_ext_body)
      *_body=tempi->ic_ext_body;
    else
      *_body=tempi->ic_body;
    *_tempi1=tempi;
    return TRUE;
  } else {
    if (!tempi->ic_cnt &&
      (tempil1=OptLag1(tempi)) && tempil1->ic_last_cnt>=0) {
      if (tempil1->ic_ext_body)
	*_body=tempil1->ic_ext_body;
      else
	*_body=tempil1->ic_body;
      *_tempi1=tempil1;
      return TRUE;
    } else {
      *_body=NULL;
      *_tempi1=NULL;
      return FALSE;
    }
  }
}


U0 ICAddRSP(CIntermediateCode *tempi,I64 i,Bool optimize=TRUE)
{
  I64 j,last_cnt;
  U8 *body;
  CIntermediateCode *tempil1;
  if (optimize && ICLastIns(tempi,&tempil1,&body)) {
    j=tempil1->ic_cnt;
    if (tempil1->ic_last_cnt==j-4 && body[j-3]==0x83 && body[j-4]==0x48) {
      if (body[j-2]==0xEC)
	j=-body[j-1](I8);
      else if (body[j-2]==0xC4)
	j=body[j-1](I8);
      else
	j=0;
    } else if (tempil1->ic_last_cnt==j-7 && body[j-6]==0x81 && body[j-7]==0x48) {
      if (body[j-5]==0xEC)
	j=-body[j-4](I32);
      else if (body[j-5]==0xC4)
	j=body[j-4](I32);
      else
	j=0;
    } else
      j=0;
    if (j) {
      if (tempi==tempil1) {
	tempi->ic_cnt=tempi->ic_last_cnt;
	i+=j;
      } else if (!(tempi->ic_flags&ICF_PREVIOUS_DELETED)) {
	tempil1->ic_flags|=ICF_DEL_PREVIOUS_INS;
	tempi->ic_flags=tempi->ic_flags&~ICF_CODE_FINAL|ICF_PREVIOUS_DELETED;
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
      throw(EXCEPT_COMPILER,20);
  } else if (i<0) {
    i=-i;
    if (i<=MAX_I8)
      ICU32(tempi,0xEC8348+i<<24);
    else if (i<=MAX_I32) {
      ICU24(tempi,0xEC8148);
      ICU32(tempi,i);
    } else
      throw(EXCEPT_COMPILER,20);
  }
  if (optimize && tempi->ic_cnt>last_cnt)
    tempi->ic_last_cnt=last_cnt;
}

#define CN_MAIN	0
#define CN_P1	1
#define CN_P2	2
#define CN_R	3

U0 CmpNoteFloatOp(CLex *lx,CIntermediateCode *tempi,Bool dont_pushable,Bool dont_popable,I64 pos)
{
  Bool link=FALSE;
  if (lx->pass==7 && lx->last_float_op_ic && lx->last_dont_popable && dont_pushable) {
    nobound_switch (pos) {
      case CN_P2:
	if (lx->last_float_op_ic!=tempi && lx->dont_push_float)
	  link=TRUE;
	break;
      case CN_P1:
	if (lx->last_float_op_ic!=tempi && lx->dont_push_float)
	  link=TRUE;
	break;
      case CN_MAIN:
	if (lx->last_float_op_ic!=tempi) {
	  if (lx->dont_push_float) {
	    if (!(tempi->ic_flags & ICF_P1_FIRST))
	      tempi->ic_flags|=ICF_ALTERNATE_TEMPLATE;
	    else
	      tempi->ic_flags&=~ICF_ALTERNATE_TEMPLATE;
	    link=TRUE;
	  }
	} else {
	  if (lx->last_float_op_pos==CN_P1 ^^ !(tempi->ic_flags & ICF_P1_FIRST))
	    tempi->ic_flags|=ICF_ALTERNATE_TEMPLATE;
	  else
	    tempi->ic_flags&=~ICF_ALTERNATE_TEMPLATE;
	  link=TRUE;
	}
	break;
      case CN_R:
	if (lx->last_float_op_ic==tempi && lx->last_float_op_pos==CN_MAIN)
	  link=TRUE;
	break;
    }
    if (link) {
      if (!Bts(&lx->last_float_op_ic->ic_flags,ICf_DONT_POP_FLOAT0+lx->last_ic_float_op_num))
	lx->last_float_op_ic->ic_flags&=~ICF_CODE_FINAL;
      if (!Bts(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0+lx->cur_ic_float_op_num))
	tempi->ic_flags&=~ICF_CODE_FINAL;
    }
  }
  lx->last_float_op_ic=tempi;
  lx->last_dont_pushable=dont_pushable;
  lx->last_dont_popable=dont_popable;
  lx->last_ic_float_op_num=lx->cur_ic_float_op_num++;
  lx->last_float_op_pos=pos;
  if (lx->cur_ic_float_op_num>4)
    throw(EXCEPT_COMPILER,21);
}

U0 CmpSetFloatOpPushPop(CLex *lx,CIntermediateCode *tempi,Bool *dont_push_float,Bool *dont_pop_float)
{
  if (lx->pass==7) {
    *dont_push_float=FALSE;
    *dont_pop_float =FALSE;
    tempi->ic_flags&=~ICF_CODE_FINAL;
  } else {
    *dont_push_float=Bt(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0+lx->cur_ic_float_op_num);
    *dont_pop_float=Bt(&tempi->ic_flags,ICf_DONT_POP_FLOAT0+lx->cur_ic_float_op_num);
  }
}

U0 ICCopyTemplate(CLex *lx,CIntermediateCode *tempi,I64 op,
  Bool off_the_record,Bool dont_pushable,Bool dont_popable,I64 pos)
{
  Bool dont_push_float,dont_pop_float,alt;
  U8 *ptr;
  I64 i=0;
  if (!off_the_record) {
    if (tempi->ic_flags&ICF_ALTERNATE_TEMPLATE)
      alt=TRUE;
    else
      alt=FALSE;
    CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  } else {
    dont_push_float=FALSE;
    dont_pop_float=FALSE;
    alt=FALSE;
  }
  if (alt && dont_push_float && !dont_pop_float) {
    ptr=cmp_templates_dont_push2[op];
    i=cmp_templates_dont_push2[op+1]-ptr;
  }
  if (!i) {
    if (dont_push_float) {
      if (dont_pop_float) {
	ptr=cmp_templates_dont_push_pop[op];
	i=cmp_templates_dont_push_pop[op+1]-ptr;
      } else {
	ptr=cmp_templates_dont_push[op];
	i=cmp_templates_dont_push[op+1]-ptr;
      }
    } else {
      if (dont_pop_float) {
	ptr=cmp_templates_dont_pop[op];
	i=cmp_templates_dont_pop[op+1]-ptr;
      } else {
	ptr=cmp_templates[op];
	i=cmp_templates[op+1]-ptr;
      }
    }
  }
  if (tempi->ic_cnt+i>=IC_BODY_SIZE) {
    if (!tempi->ic_ext_body) {
      tempi->ic_ext_body=MAlloc(256);
      MemCpy(tempi->ic_ext_body,tempi->ic_body,tempi->ic_cnt);
    }
    MemCpy(&tempi->ic_ext_body[tempi->ic_cnt],ptr,i);
  } else
    MemCpy(&tempi->ic_body[tempi->ic_cnt],ptr,i);
  if (!off_the_record)
    CmpNoteFloatOp(lx,tempi,dont_pushable,dont_popable,pos);
  tempi->ic_cnt+=i;
}

extern U0 ICMov(CIntermediateCode *tempi,I64 t1,I64 r1,I64 d1,I64 t2,I64 r2,I64 d2,I64 ip);

#define MDR_REG				0
#define MDR_INDIRECT_REG		1
#define MDR_D8_INDIRECT_REG		2
#define MDR_D32_INDIRECT_REG		3
#define MDR_SIB_INDIRECT_REG		4
#define MDR_SIB_D8_INDIRECT_REG		5
#define MDR_SIB_D32_INDIRECT_REG	6
#define MDR_RIP_REL			7

I64 ICModr1(I64 r,I64 t2,I64 r2,I64 d2)
{
  //result.u8[0] is type
  //result.u8[1] is REX
  //result.u8[2] is ModR
  //result.u8[3] is SIB
  I64 result=0;
  if (t2&TY_MASK>TY_RIP_DISP32) {
    "Callers:%P;%P\n",Caller,Caller(2);
    throw(EXCEPT_COMPILER,22);
  }
  if (t2&IT_MASK<IT_I64)
    result.u8[1]=0x40;
  else
    result.u8[1]=0x48;
  if (r>7) {
    result.u8[1]+=4;
    r&=7;
  }
  switch (t2>>4) {
    case TY_REG>>4:
      if (r2>7) {
	result.u8[1]++;
	r2&=7;
      }
      result.u8[2]=0xC0+r<<3+r2;
      result.u8[0]=MDR_REG;
      if (result.u8[1]==0x40 &&
	  (t2&IT_MASK>=IT_I16 ||
	   r<4 && r2<4))
	result.u8[1]=0;
      break;
    case TY_DISP>>4:
      if (r2>7) {
	result.u8[1]++;
	r2&=7;
      }
      if (!d2 && r2!=CREG_RBP) {
	result.u8[2]=r<<3+r2;
	result.u8[0]=MDR_INDIRECT_REG;
      } else if (MIN_I8<=d2<=MAX_I8) {
	result.u8[2]=0x40+r<<3+r2;
	result.u8[0]=MDR_D8_INDIRECT_REG;
      } else {
	result.u8[2]=0x80+r<<3+r2;
	result.u8[0]=MDR_D32_INDIRECT_REG;
      }
      if (result.u8[1]==0x40 &&
	  (t2&IT_MASK>=IT_I16 ||
	   r<4))
	result.u8[1]=0;
      break;
    case TY_SIB>>4:
      if (7<r2.u8[0]<CREG_NONE)
	result.u8[1]++;
      if (r2.u8[1]&15>7)
	result.u8[1]+=2;
      if (r2.u8[0]==CREG_NONE) {
	result.u8[3]=5+(r2.u8[1]&7)<<3+r2.u8[1]&0xC0;
	result.u8[2]=4+r<<3;
	result.u8[0]=MDR_SIB_D32_INDIRECT_REG;
      } else {
	result.u8[3]=r2.u8[0]&7+(r2.u8[1]&7)<<3+r2.u8[1]&0xC0;
	if (!d2 && r2.u8[0]&7!=CREG_RBP) {
	  result.u8[2]=4+r<<3;
	  result.u8[0]=MDR_SIB_INDIRECT_REG;
	} else if (MIN_I8<=d2<=MAX_I8) {
	  result.u8[2]=0x44+r<<3;
	  result.u8[0]=MDR_SIB_D8_INDIRECT_REG;
	} else {
	  result.u8[2]=0x84+r<<3;
	  result.u8[0]=MDR_SIB_D32_INDIRECT_REG;
	}
      }
      if (result.u8[1]==0x40 &&
	  (t2&IT_MASK>=IT_I16 ||
	   r<4))
	result.u8[1]=0;
      break;
    case TY_RIP_DISP32>>4:
      result.u8[2]=0x05+r<<3;
      result.u8[0]=MDR_RIP_REL;
      if (result.u8[1]==0x40 &&
	  (t2&IT_MASK>=IT_I16 ||
	   r<4))
	result.u8[1]=0;
      break;
  }
  return result;
}

U0 ICModr2(CIntermediateCode *tempi,I64 i,I64 d,I64 ip=0)
{//t=0 means no SIB
  nobound_switch (i.u8[0]) {
    case MDR_REG:
      break;
    case MDR_INDIRECT_REG:
      break;
    case MDR_D8_INDIRECT_REG:
      ICU8(tempi,d);
      break;
    case MDR_D32_INDIRECT_REG:
      ICU32(tempi,d);
      break;
    case MDR_SIB_INDIRECT_REG:
      ICU8(tempi,i.u8[3]);
      break;
    case MDR_SIB_D8_INDIRECT_REG:
      ICU8(tempi,i.u8[3]);
      ICU8(tempi,d);
      break;
    case MDR_SIB_D32_INDIRECT_REG:
      ICU8(tempi,i.u8[3]);
      ICU32(tempi,d);
      break;
    case MDR_RIP_REL:
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

U0 ICSlashOp(CIntermediateCode *tempi,I64 t1,I64 r1,I64 d1,I64 op,I64 ip)
{
  I64 i;
  if (t1&TY_MASK==TY_REG && !op.u8[3])
    t1=t1&(TY_MASK|1)+IT_I64; //Set to 64 bit, preserving signed/unsigned
  i=ICModr1(op.u8[0],t1,r1,d1);

  if (tempi->ic_flags&ICF_LOCK && op!=SLASH_OP_MOV && t1&TY_MASK!=TY_REG)
    ICU8(tempi,OC_LOCK_PREFIX);
  switch (t1&IT_MASK) {
    case IT_I8:
    case IT_U8:
      ICRex(tempi,i.u8[1]);
      ICU16(tempi,i.u8[2]<<8+op.u8[1]);
      break;
    case IT_I16:
    case IT_U16:
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

U0 ICPush(CIntermediateCode *tempi,I64 t1,I64 r1,I64 d1,I64 ip)
{
  switch (t1>>4) {
    case TY_REG>>4:
      if (r1>7)
	ICU16(tempi,0x5049+(r1&7)<<8);
      else
	ICU8(tempi,0x50+r1);
      return;
    case TY_IMM>>4:
      if (MIN_I8<=d1<=MAX_I8)
	ICU16(tempi,0x6A+d1<<8);
      else if (MIN_I32<=d1<=MAX_I32) {
	ICU8(tempi,0x68);
	ICU32(tempi,d1);
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,t1,r1,d1,ip);
	ICU8(tempi,0x50+CREG_RBX);
      }
      return;
    case TY_STK>>4:
      return;
    case TY_DISP>>4:
    case TY_SIB>>4:
    case TY_RIP_DISP32>>4:
      switch (t1&IT_MASK) {
	case IT_I64:
	case IT_U64:
	case IT_F64:
	  ICSlashOp(tempi,t1,r1,d1,SLASH_OP_PUSH,ip);
	  return;
      }
      break;
  }
  ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,t1,r1,d1,ip);
  ICU16(tempi,0x5048+CREG_RBX<<8);
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

U0 ICPop(CIntermediateCode *tempi,I64 t1,I64 r1,I64 d1,I64 ip)
{
  switch (t1>>4) {
    case TY_NULL>>4:
      ICAddRSP(tempi,8);
      break;
    case TY_REG>>4:
      if (r1>7)
	ICU16(tempi,0x5849+(r1&7)<<8);
      else
	ICU8(tempi,0x58+r1);
      break;
    case TY_DISP>>4:
    case TY_RIP_DISP32>>4:
    case TY_SIB>>4:
      if (t1&IT_MASK<IT_I64) {
	ICU8(tempi,0x58+CREG_RBX);
	ICMov(tempi,t1,r1,d1,TY_REG+IT_I64,CREG_RBX,0,ip);
      } else
	ICSlashOp(tempi,t1,r1,d1,SLASH_OP_POP,ip);
      break;
    default:
      ICU8(tempi,0x58+CREG_RBX);
      ICMov(tempi,t1,r1,d1,TY_REG+IT_I64,CREG_RBX,0,ip);
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
{ //Returns 2-byte opcode for FLD const or zero
  if (!d)
    return 0xEED9;
  else if (d==1.0)
    return 0xE8D9;
  else if (GetOpt(OPTf_NO_BUILTIN_CONST))
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


U0 ICMov(CIntermediateCode *tempi,I64 t1,I64 r1,I64 d1,I64 t2,I64 r2,I64 d2,I64 ip)
{
  I64 i,j,last_cnt=tempi->ic_cnt;
  CIntermediateCode *tempil1;
  U8 *body;
  Bool old_lock=Btr(&tempi->ic_flags,ICf_LOCK);
  switch (t1>>4) {
    case TY_REG>>4:
      if (t2&TY_MASK==TY_IMM) {
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
      } else if (t2&TY_MASK==TY_STK)
	ICPop(tempi,t1,r1,d1,ip);
      else {
	if (r1==r2 && t2&TY_MASK==TY_REG)
	  goto move_done;
	if (t2&TY_MASK==TY_REG) {
	  t2=TY_REG+IT_I64;
	  if (ICLastIns(tempi,&tempil1,&body)) {
	    i=tempil1->ic_cnt;
	    j=0x48+(r1>7);
	    if (r2>7) j+=4;
	    if (tempil1->ic_last_cnt==i-3 &&
		body[i-2]==0x8B &&
		body[i-1]==0xC0+r1&7+(r2&7)<<3 &&
		body[i-3]==j ) {
	      goto move_done;
	    }
	  }
	}
	i=ICModr1(r1,t2,r2,d2);
	if (t2&IT_MASK!=IT_U32)
	  i|=0x4800;
	ICRex(tempi,i.u8[1]);
	switch (t2&IT_MASK) {
	  case IT_I8:
	    ICU24(tempi,i.u8[2]<<16+0xBE0F);
	    break;
	  case IT_I16:
	    ICU24(tempi,i.u8[2]<<16+0xBF0F);
	    break;
	  case IT_I32:
	    ICU16(tempi,i.u8[2]<<8+0x63);
	    break;

	  case IT_U8:
	    ICU24(tempi,i.u8[2]<<16+0xB60F);
	    break;
	  case IT_U16:
	    ICU24(tempi,i.u8[2]<<16+0xB70F);
	    break;
	  default:
	    ICU16(tempi,i.u8[2]<<8+0x8B);
	}
	ICModr2(tempi,i,d2,ip+4);
      }
      break;
    case TY_STK>>4:
      if (t1&IT_MASK<t2&IT_MASK)
	ICPush(tempi,t2&TY_MASK+t1&IT_MASK,r2,d2,ip);
      else
	ICPush(tempi,t2,r2,d2,ip);
      break;
    case TY_DISP>>4:
    case TY_RIP_DISP32>>4:
    case TY_SIB>>4:
      if (t2&TY_MASK==TY_IMM &&
	  (t1&IT_MASK<IT_I64 || (MIN_I32<=d2<=MAX_I32))) {
	i=ICModr1(0,t1,r1,d1);
	switch (t1&IT_MASK) {
	  case IT_I8:
	  case IT_U8:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xC6);
	    break;
	  case IT_U16:
	  case IT_I16:
	    ICOpSizeRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xC7);
	    break;
	  default:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xC7);
	}
	if (i.u8[0]!=MDR_RIP_REL)
	  ICModr2(tempi,i,d1,);
	else {
	  i=d1-(ip+tempi->ic_cnt+4);
	  switch (t1&IT_MASK) {
	    case IT_I8:
	    case IT_U8:
	      i--;
	      break;
	    case IT_I16:
	    case IT_U16:
	      i-=2;
	      break;
	    default:
	      i-=4;
	  }
	  ICU32(tempi,i);
	  tempi->ic_flags&=~ICF_CODE_FINAL;
	}

	switch (t1&IT_MASK) {
	  case IT_I8:
	  case IT_U8:
	    ICU8(tempi,d2);
	    break;
	  case IT_I16:
	  case IT_U16:
	    ICU16(tempi,d2);
	    break;
	  default:
	    ICU32(tempi,d2);
	}
      } else {
	if (t2&TY_MASK==TY_REG)
	  ICSlashOp(tempi,t1,r1,d1,r2+SLASH_OP_MOV,ip);
	else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,t2,r2,d2,ip);
	  ICMov(tempi,t1,r1,d1,TY_REG+IT_I64,CREG_RBX,0,ip);
	}
      }
      break;
  }
move_done:
  BEqu(&tempi->ic_flags,ICf_LOCK,old_lock);
  if (tempi->ic_cnt>last_cnt>tempi->ic_last_cnt)
    tempi->ic_last_cnt=last_cnt;
}

U0 ICLea(CIntermediateCode *tempi,I64 t1,I64 r1,I64 d1,I64 t2,I64 r2,I64 d2,CLex *lx,U8 *buf,I64 ip)
{
  I64 i;
  CExeAbsAddr *tempa;
  switch (t1>>4) {
    case TY_REG>>4:
      i=ICModr1(r1,t2,r2,d2);
      i.u8[1]|=0x48;
      ICU24(tempi,i.u8[2]<<16+0x8D00+i.u8[1]);
      ICModr2(tempi,i,d2,ip+4);
      break;
    case TY_STK>>4:
      if (t2&TY_MASK==TY_RIP_DISP32) {
	ICU8(tempi,0x68);
	ICU32(tempi,d2);
	if (lx->flags&LF_AOT_COMPILE && buf && !(lx->flags&LF_NO_ABSS)) {
	  tempa=CAlloc(sizeof(CExeAbsAddr));
	  tempa->next=lx->a->abss;
	  tempa->type=EAA_ADD_U32;
	  lx->a->abss=tempa;
	  tempa->ip=ip+tempi->ic_cnt-4;
	}
	tempi->ic_flags&=~ICF_CODE_FINAL;
      } else {
	ICLea(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,lx,buf,ip);
	ICMov(tempi,t1,r1,d1,TY_REG+IT_I64,CREG_RCX,0,ip);
      }
      break;
    default:
      ICLea(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,lx,buf,ip);
      ICMov(tempi,t1,r1,d1,TY_REG+IT_I64,CREG_RCX,0,ip);
  }
}

U0 ICDeref(CIntermediateCode *tempi,I64 ip)
{
  I64 t;
  t=tempi->r.type&IT_MASK;
  if (t>tempi->p1.ptype)
    t=tempi->p1.ptype;

  if (tempi->p1.type&TY_MASK==TY_REG)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  TY_DISP+t,tempi->p1.reg,tempi->p1.disp,ip);
  else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,
      tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  TY_DISP+t,CREG_RCX,0,ip);
  }
}

U0 ICAddEct(CIntermediateCode *tempi,I64 t1,I64 r1,I64 d1,
	   I64 t2,I64 r2,I64 d2,I64 t3,I64 r3,I64 d3,I64 op,I64 ip)
{
  I64 i,temp,result_reg=CREG_RAX;
  Bool swap=FALSE;
  if (r3!=result_reg) {
    swap^=TRUE;
    SwapU64(&t2,&t3);
    SwapU64(&r2,&r3);
    SwapU64(&d2,&d3);
  }
  if (t2&IT_MASK>=IT_I64 && r2!=result_reg &&
      TY_REG<=t2&TY_MASK<=TY_RIP_DISP32) {
    if (t1&TY_MASK==TY_REG && !(r2==r1 && TY_REG<=t2&TY_MASK<=TY_SIB))
      result_reg=r1;
    ICMov(tempi,TY_REG+IT_I64,result_reg,0,t3,r3,d3,ip);
    i=ICModr1(result_reg,t2,r2,d2);
    if (tempi->ic_flags&ICF_LOCK)
      ICU8(tempi,OC_LOCK_PREFIX);
    ICRex(tempi,i.u8[1]);
    ICU16(tempi,i.u8[2]<<8+op);
    ICModr2(tempi,i,d2,ip+4);
  } else {
    if (t2&TY_MASK==TY_REG)
      temp=r2;
    else
      temp=CREG_RCX;

    if (t1&TY_MASK==TY_REG)
      result_reg=r1;

    if (temp==result_reg)
      result_reg=CREG_RDX;
    if (swap ^^ tempi->ic_flags & ICF_P1_FIRST) {
      if (r3==temp && TY_REG<=t3&TY_MASK<=TY_SIB)
	temp=CREG_RCX;
      ICMov(tempi,TY_REG+IT_I64,temp,0,t2,r2,d2,ip);
      ICMov(tempi,TY_REG+IT_I64,result_reg,0,t3,r3,d3,ip);
    } else {
      if (r2==result_reg && TY_REG<=t2&TY_MASK<=TY_SIB)
	result_reg=CREG_RDX;
      ICMov(tempi,TY_REG+IT_I64,result_reg,0,t3,r3,d3,ip);
      ICMov(tempi,TY_REG+IT_I64,temp,0,t2,r2,d2,ip);
    }
    i=0x48;
    if (result_reg>7)
      i+=4;
    if (temp>7)
      i++;
    if (tempi->ic_flags&ICF_LOCK)
      ICU8(tempi,OC_LOCK_PREFIX);
    ICU24(tempi,0xC00000+i+(temp&7)<<16+(result_reg&7)<<19+op<<8);
  }
  ICMov(tempi,t1,r1,d1,TY_REG+IT_I64,result_reg,0,ip);
}

U0 ICAddSubEctImm(CIntermediateCode *tempi,I64 t1,I64 r1,I64 d1,
	   I64 t2,I64 r2,I64 d2,I64 d,I64 op,I64 ip)
{
  I64 i,r;
  if (op.u8[0]==0x2B) {
    op=0x0003;
    d=-d;
  }
  if (t1&TY_MASK==TY_REG) {
    if (t2&TY_MASK!=TY_REG) {
      ICMov(tempi,t1,r1,d1,t2,r2,d2,ip);
      t2=t1;
      r2=r1;
      d2=d1;
    }
    if (r1==r2) {
      if (r1>7)
	i=0x49;
      else
	i=0x48;
      if (!d &&
	(op.u8[0]==0x03||op.u8[0]==0x2B||
	 op.u8[0]==0x33||op.u8[0]==0x0B))
	return;
      else if (d==1 && op.u8[0]==0x03) {
	ICU24(tempi,0xC0FF00+op.u8[1]<<19+i+(r1&7)<<16);
	return;
      } else if (d==-1 && op.u8[0]==0x03) {
	ICU24(tempi,0xC8FF00+i+(r1&7)<<16);
	return;
      } else if (MIN_I8<=d<=MAX_I8) {
	ICU24(tempi,0xC08300+op.u8[1]<<19+i+(r1&7)<<16);
	ICU8(tempi,d);
	return;
      } else if (MIN_I32<=d<=MAX_I32) {
	ICU24(tempi,0xC08100+op.u8[1]<<19+i+(r1&7)<<16);
	ICU32(tempi,d);
	return;
      }
    }
    if (op.u8[0]==0x03 && MIN_I32<=d<=MAX_I32 &&
      !Bt(&non_ptr_vars_mask,r2)) {
      i=ICModr1(r1,TY_DISP+IT_I64,r2,d);
      i.u8[1]|=0x48;
      ICU24(tempi,i.u8[2]<<16+0x8D00+i.u8[1]);
      ICModr2(tempi,i,d,ip+4);
      return;
    }
  }
  switch (t1>>4) {
    case TY_REG>>4:
    case TY_DISP>>4:
    case TY_SIB>>4:
    case TY_RIP_DISP32>>4:
      if (t1!=t2 || r1!=r2 || d1!=d2) {
	ICMov(tempi,t1,r1,d1,t2,r2,d2,ip);
	t2=t1;
	r2=r1;
	d2=d1;
      }

      if (!d &&
	(op.u8[0]==0x03||op.u8[0]==0x2B||
	 op.u8[0]==0x33||op.u8[0]==0x0B))
	return;
      r=op.u8[1]; //not a register but a slash val
      if (op.u8[0]==0x03 && d==-1) //add -1
	r=1; //Decrement slash val

      if (op.u8[0]==0x03 && (d==1 || d==-1)) {	//Add
	i=ICModr1(r,t1,r1,d1);
	if (t1!=TY_REG && tempi->ic_flags&ICF_LOCK)
	  ICU8(tempi,OC_LOCK_PREFIX);
	switch (t1&IT_MASK) {
	  case IT_I8:
	  case IT_U8:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xFE);
	    break;
	  case IT_I16:
	  case IT_U16:
	    ICOpSizeRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xFF);
	    break;
	  default:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xFF);
	}
	ICModr2(tempi,i,d1,ip+4);
	return;
      }
      if (MIN_I8<=d<=MAX_I8 ||
	  t1&(IT_MASK-1)==IT_I8) {
	i=ICModr1(r,t1,r1,d1);
	if (tempi->ic_flags&ICF_LOCK)
	  ICU8(tempi,OC_LOCK_PREFIX);
	switch (t1&IT_MASK) {
	  case IT_I8:
	  case IT_U8:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0x80);
	    break;
	  case IT_I16:
	  case IT_U16:
	    ICOpSizeRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0x83);
	    break;
	  default:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0x83);
	}
	ICModr2(tempi,i,d1,ip+5);
	ICU8(tempi,d);
	return;
      }
      if (MIN_I32<=d<=MAX_I32 ||
	  t1&IT_MASK<IT_I64) {
	i=ICModr1(r,t1,r1,d1);
	if (tempi->ic_flags&ICF_LOCK)
	  ICU8(tempi,OC_LOCK_PREFIX);
	switch (t1&IT_MASK) {
	  case IT_I16:
	  case IT_U16:
	    ICOpSizeRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0x81);
	    break;
	  default:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0x81);
	}
	if (i.u8[0]!=MDR_RIP_REL)
	  ICModr2(tempi,i,d1,);
	else {
	  if (t1&(IT_MASK-1)==IT_I16)
	    ICU32(tempi,d1-(ip+tempi->ic_cnt+6));
	  else
	    ICU32(tempi,d1-(ip+tempi->ic_cnt+8));
	  tempi->ic_flags&=~ICF_CODE_FINAL;
	}
	if (t1&(IT_MASK-1)==IT_I16)
	  ICU16(tempi,d);
	else
	  ICU32(tempi,d);
	return;
      }
      break;
    case TY_STK>>4:
      ICAddSubEctImm(tempi,TY_REG+IT_I64,CREG_RAX,0,t2,r2,d2,d,op,ip);
      ICPushRegs(tempi,1<<CREG_RAX);
      return;
  }
  ICAddEct(tempi,t1,r1,d1,TY_IMM+IT_I64,0,d,t2,r2,d2,op.u8[0],ip);
}

U0 ICSub(CIntermediateCode *tempi,I64 t1,I64 r1,I64 d1,
	   I64 t2,I64 r2,I64 d2,I64 t3,I64 r3,I64 d3,I64 ip)
{
  I64 i=0x48,op=0x2B;
  Bool swap=FALSE;
  if (r3!=CREG_RAX) {
    swap=TRUE;
    SwapU64(&t2,&t3);
    SwapU64(&r2,&r3);
    SwapU64(&d2,&d3);
  }
  if (t2&IT_MASK>=IT_I64 && r2.u8[0]!=CREG_RAX && (t2&TY_MASK!=TY_SIB || r2.u8[1]&15!=CREG_RAX) &&
      TY_REG<=t2&TY_MASK<=TY_RIP_DISP32) {
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t3,r3,d3,ip);
    if (!swap) {
      op=0x03;
      ICU24(tempi,0xD8F748);
    }
    i=ICModr1(CREG_RAX,t2,r2,d2);
    if (tempi->ic_flags&ICF_LOCK)
      ICU8(tempi,OC_LOCK_PREFIX);
    ICRex(tempi,i.u8[1]);
    ICU16(tempi,i.u8[2]<<8+op);
    ICModr2(tempi,i,d2,ip+4);
    ICMov(tempi,t1,r1,d1,TY_REG+IT_I64,CREG_RAX,0,ip);
  } else {
    if (tempi->ic_flags & ICF_P1_FIRST) {
      if (t3&TY_MASK!=TY_REG || t3&IT_MASK<IT_I64) {
	if (swap) {
	  swap=FALSE;
	  ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
	  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t3,r3,d3,ip);
	  r3=CREG_RCX;
	  r2=CREG_RAX;
	} else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
	  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t3,r3,d3,ip);
	  r2=CREG_RCX;
	  r3=CREG_RAX;
	}
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
	r2=CREG_RCX;
      }
    } else {
      if (t3&TY_MASK!=TY_REG || t3&IT_MASK<IT_I64) {
	if (swap) {
	  swap=FALSE;
	  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t3,r3,d3,ip);
	  ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
	  r2=CREG_RAX;
	  r3=CREG_RCX;
	} else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t3,r3,d3,ip);
	  ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
	  r3=CREG_RAX;
	  r2=CREG_RCX;
	}
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
	r2=CREG_RCX;
      }
    }
    if (swap) {
      op=0x03;
      ICU24(tempi,0xD9F748);
    }
    if (r3>7)
      i++;
    if (r2>7)
      i+=4;
    if (tempi->ic_flags&ICF_LOCK)
      ICU8(tempi,OC_LOCK_PREFIX);
    ICU24(tempi,0xC00000+i+(r3&7)<<16+(r2&7)<<19+op<<8);
    ICMov(tempi,t1,r1,d1,TY_REG+IT_I64,r2,0,ip);
  }
}

U0 ICFCvt(CLex *lx,CIntermediateCode *tempi,I64 r1,I64 t2,I64 r2,I64 d2,Bool to_int,I64 pos,I64 ip)
{
  I64 rsp_size=0,op1,op2;
  Bool dont_push_float,dont_pop_float;

  if (to_int) {
    op1=SLASH_OP_FLD;
    op2=SLASH_OP_FISTTP;
  } else {
    op1=SLASH_OP_FILD;
    op2=SLASH_OP_FSTP;
  }

  CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  if (!dont_push_float) {
    if (!(t2&IT_MASK>=IT_I64 &&
	(t2&TY_MASK==TY_DISP ||
	t2&TY_MASK==TY_RIP_DISP32 ||
	t2&TY_MASK==TY_SIB))) {
      ICPush(tempi,t2,r2,d2,ip);
      t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
      rsp_size=8;
    } else {
      if (!dont_pop_float) {
	rsp_size=8;
	ICAddRSP(tempi,-8);
      }
    }
    ICSlashOp(tempi,t2,r2,d2,op1,ip);
  } else {
    if (!dont_pop_float) {
      rsp_size=8;
      ICAddRSP(tempi,-8);
    }
  }
  if (to_int)
    CmpNoteFloatOp(lx,tempi,TRUE,FALSE,pos);
  else
    CmpNoteFloatOp(lx,tempi,FALSE,TRUE,pos);
  if (dont_pop_float) {
    if (rsp_size)
      ICAddRSP(tempi,rsp_size);
  } else {
    ICSlashOp(tempi,TY_SIB+IT_I64,CREG_RSP+CREG_RSP<<8,0,op2,ip);
    ICPop(tempi,TY_REG+IT_I64,r1,0,ip);
  }
}

U0 ICFCvt2(CLex *lx,CIntermediateCode *tempi,I64 r1,I64 t2,I64 r2,I64 d2,Bool to_int,I64 ip)
{
  I64 rsp_size=0,op1,op2;
  if (to_int) {
    op1=SLASH_OP_FLD;
    op2=SLASH_OP_FISTTP;
  } else {
    op1=SLASH_OP_FILD;
    op2=SLASH_OP_FSTP;
  }
  if (!(t2&IT_MASK>=IT_I64 &&
      (t2&TY_MASK==TY_DISP ||
      t2&TY_MASK==TY_RIP_DISP32 ||
      t2&TY_MASK==TY_SIB))) {
    ICPush(tempi,t2,r2,d2,ip);
    t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
    rsp_size=8;
  } else {
    rsp_size=8;
    ICAddRSP(tempi,-8);
  }
  ICSlashOp(tempi,t2,r2,d2,op1,ip);
  ICSlashOp(tempi,TY_SIB+IT_I64,CREG_RSP+CREG_RSP<<8,0,op2,ip);
  ICPop(tempi,TY_REG+IT_I64,r1,0,ip);
  lx->last_dont_pushable=lx->last_dont_popable=FALSE; //TODO: might improve this
}

U0 ICFUnaryMinus(CLex *lx,CIntermediateCode *tempi,U8 *buf2,I64 ip)
{
  CICArg *p1=&tempi->p1;
  I64 rsp_size=0,builtin1=0,t1,r1,d1;
  Bool dont_push_float,dont_pop_float;

  if (lx->flags&LF_AOT_COMPILE)
    buf2=lx->a->ip;

  CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  if (!dont_push_float) {
    if (p1->type&IT_MASK>=IT_I64 &&
	(p1->type&TY_MASK==TY_DISP ||
	p1->type&TY_MASK==TY_RIP_DISP32 ||
	p1->type&TY_MASK==TY_SIB)) {
      t1=p1->type;
      r1=p1->reg;
      d1=p1->disp;
    } else {
      if (p1->type&TY_MASK==TY_IMM) {
	if (!(builtin1=ICBuiltInFloatConst(p1->disp(F64)))) {
	  t1=TY_RIP_DISP32+IT_I64;
	  r1=CREG_RIP;
	  d1=LCCFloatConstFind(lx,p1->disp(F64))+buf2;
	}
      } else {
	ICPush(tempi,p1->type,p1->reg,p1->disp,ip);
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	rsp_size+=8;
      }
    }
    if (builtin1)
      ICU16(tempi,builtin1);
    else
      ICSlashOp(tempi,t1,r1,d1,SLASH_OP_FLD,ip);
  }
  if (!dont_pop_float && !rsp_size) {
    rsp_size=8;
    ICAddRSP(tempi,-8);
  }
  ICU16(tempi,0xE0D9); //FCHS
  CmpNoteFloatOp(lx,tempi,TRUE,TRUE,CN_MAIN);
  if (dont_pop_float) {
    if (rsp_size)
      ICAddRSP(tempi,rsp_size);
  } else {
    ICSlashOp(tempi,TY_SIB+IT_I64,CREG_RSP+CREG_RSP<<8,0,SLASH_OP_FSTP,ip);
    ICPop(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,ip);
  }
}

U0 ICFMod(CLex *lx,CIntermediateCode *tempi,I64 ip)
{ //for MOD
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (tempi->ic_flags&ICF_ALTERNATE_TEMPLATE)
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    else
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
  } else {
    if (tempi->ic_flags & ICF_P1_FIRST) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    } else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    }
  }
  ICCopyTemplate(lx,tempi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_MAIN); //TODO: unpushable,unpop?  Not sure
  if (!dont_pop_float)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	TY_REG+IT_I64,CREG_RAX,0,ip);
}

U0 ICFPow(CLex *lx,CIntermediateCode *tempi,U8 *buf,I64 ip)
{ //for POW
  I64 i;
  CExeImportExport *tempe;
  CHashExport *tempex=HashFind("SYS_POW",lx->htc.hash_table_lst,HTT_EXPORT_SYS_SYM);

  if (tempi->ic_flags & ICF_P1_FIRST) {
    ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
  }
  if (lx->flags&LF_AOT_COMPILE) {
    if (!tempex) {
      tempex=CAlloc(sizeof(CHashExport));
      tempex->str=StrNew("SYS_POW");
      tempex->type=HTT_EXPORT_SYS_SYM|HTF_UNRESOLVED|HTF_IMPORT;
      HashAdd(tempex,lx->htc.glbl_hash_table);
    }
    if (tempex->type&HTF_IMPORT) {
      if (GetOpt(OPTf_USE_IMM64)) {
	ICU16(tempi,0xBB48);
	ICU64(tempi,0);
	if (buf) {
	  tempe=CAlloc(sizeof(CExeImportExport));
	  tempe->type=EIE_IMM_U64;
	  tempe->ip=ip+tempi->ic_cnt-8;
	  tempe->next=tempex->ie_lst;
	  tempex->ie_lst=tempe;
	}
	ICU16(tempi,0xD3FF);
      } else {
	ICU8(tempi,0xE8);
	ICU32(tempi,-(ip+tempi->ic_cnt+4));
	if (buf) {
	  tempe=CAlloc(sizeof(CExeImportExport));
	  tempe->type=EIE_REL_U32;
	  tempe->ip=ip+tempi->ic_cnt-4;
	  tempe->next=tempex->ie_lst;
	  tempex->ie_lst=tempe;
	}
      }
    } else { //Kernel
      if (tempex->type&HTF_UNRESOLVED)
	throw(EXCEPT_COMPILER,23);
      else {
	i=tempex->val-(ip+tempi->ic_cnt+5);
	if (!(MIN_I32<=i<=MAX_I32)) {
	  throw(EXCEPT_COMPILER,23);
//	    ICU16(tempi,0xBB48);
//	    ICU64(tempi,tempex->val);
//	    ICU16(tempi,0xD3FF);
	} else {
	  ICU8(tempi,0xE8);
	  ICU32(tempi,i);
	}
      }
    }
  } else {
    i=tempex->val-(ip+tempi->ic_cnt+5);
    if (!(MIN_I32<=i<=MAX_I32)) {
      ICU16(tempi,0xBB48);
      ICU64(tempi,tempex->val);
      ICU16(tempi,0xD3FF);
    } else {
      ICU8(tempi,0xE8);
      ICU32(tempi,i);
    }
  }
  tempi->ic_flags&=~ICF_CODE_FINAL;
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
      TY_REG+IT_I64,CREG_RAX,0,ip);
}

U0 ICFOp(CLex *lx,CIntermediateCode *tempi,I64 op,U8 *buf2,I64 ip)
{ //for ADD,SUB,DIV,MUL
  CICArg *p1,*p2;
  Bool dont_push_float,dont_pop_float,alt;
  I64 rsp_size=0,builtin1=0,builtin2=0,
      t1,r1,d1,t2,r2,d2;

  if (tempi->ic_flags&ICF_ALTERNATE_TEMPLATE) {
    p1=&tempi->p2;
    p2=&tempi->p1;
    alt=TRUE;
  } else {
    p1=&tempi->p1;
    p2=&tempi->p2;
    alt=FALSE;
  }

  if (lx->flags&LF_AOT_COMPILE)
    buf2=lx->a->ip;

  CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (p2->type&IT_MASK>=IT_I64 &&
	(p2->type&TY_MASK==TY_DISP ||
	p2->type&TY_MASK==TY_RIP_DISP32 ||
	p2->type&TY_MASK==TY_SIB)) {
      t2=p2->type;
      r2=p2->reg;
      d2=p2->disp;
    } else {
      if (p2->type&TY_MASK==TY_IMM) {
	if (!(builtin2=ICBuiltInFloatConst(p2->disp(F64)))) {
	  t2=TY_RIP_DISP32+IT_I64;
	  r2=CREG_RIP;
	  d2=LCCFloatConstFind(lx,p2->disp(F64))+buf2;
	}
      } else {
	ICPush(tempi,p2->type,p2->reg,p2->disp,ip);
	t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
	rsp_size+=8;
      }
    }
  } else {
    if (tempi->ic_flags & ICF_P1_FIRST^^alt) {
      if (p2->type&TY_MASK!=TY_STK) {
	if (p1->type&IT_MASK>=IT_I64 &&
	    (p1->type&TY_MASK==TY_DISP ||
	    p1->type&TY_MASK==TY_RIP_DISP32 ||
	    p1->type&TY_MASK==TY_SIB)) {
	  t1=p1->type;
	  r1=p1->reg;
	  d1=p1->disp;
	} else {
	  if (p1->type&TY_MASK==TY_IMM) {
	    if (!(builtin1=ICBuiltInFloatConst(p1->disp(F64)))) {
	      t1=TY_RIP_DISP32+IT_I64;
	      r1=CREG_RIP;
	      d1=LCCFloatConstFind(lx,p1->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,p1->type,p1->reg,p1->disp,ip);
	    t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	    rsp_size+=8;
	  }
	}
	if (p2->type&IT_MASK>=IT_I64 &&
	    (p2->type&TY_MASK==TY_DISP ||
	    p2->type&TY_MASK==TY_RIP_DISP32 ||
	    p2->type&TY_MASK==TY_SIB)) {
	  t2=p2->type;
	  r2=p2->reg;
	  d2=p2->disp;
	} else {
	  if (p2->type&TY_MASK==TY_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(p2->disp(F64)))) {
	      t2=TY_RIP_DISP32+IT_I64;
	      r2=CREG_RIP;
	      d2=LCCFloatConstFind(lx,p2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,p2->type,p2->reg,p2->disp,ip);
	    t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
	    rsp_size+=8;
	    if (r1==CREG_RSP+CREG_RSP<<8)
	      d1+=8;
	  }
	}
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	    p1->type,p1->reg,p1->disp,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	    p2->type,p2->reg,p2->disp,ip);
	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=8;
	t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
      }
    } else {
      if (p1->type&TY_MASK!=TY_STK) {
	if (p2->type&IT_MASK>=IT_I64 &&
	    (p2->type&TY_MASK==TY_DISP ||
	    p2->type&TY_MASK==TY_RIP_DISP32 ||
	    p2->type&TY_MASK==TY_SIB)) {
	  t2=p2->type;
	  r2=p2->reg;
	  d2=p2->disp;
	} else {
	  if (p2->type&TY_MASK==TY_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(p2->disp(F64)))) {
	      t2=TY_RIP_DISP32+IT_I64;
	      r2=CREG_RIP;
	      d2=LCCFloatConstFind(lx,p2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,p2->type,p2->reg,p2->disp,ip);
	    t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
	    rsp_size+=8;
	  }
	}
	if (p1->type&IT_MASK>=IT_I64 &&
	    (p1->type&TY_MASK==TY_DISP ||
	    p1->type&TY_MASK==TY_RIP_DISP32 ||
	    p1->type&TY_MASK==TY_SIB)) {
	  t1=p1->type;
	  r1=p1->reg;
	  d1=p1->disp;
	} else {
	  if (p1->type&TY_MASK==TY_IMM) {
	    if (!(builtin1=ICBuiltInFloatConst(p1->disp(F64)))) {
	      t1=TY_RIP_DISP32+IT_I64;
	      r1=CREG_RIP;
	      d1=LCCFloatConstFind(lx,p1->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,p1->type,p1->reg,p1->disp,ip);
	    t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	    rsp_size+=8;
	    if (r2==CREG_RSP+CREG_RSP<<8)
	      d2+=8;
	  }
	}
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	    p2->type,p2->reg,p2->disp,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	    p1->type,p1->reg,p1->disp,ip);
	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=8;
	t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
      }
    }
  }
  if (!dont_pop_float && !rsp_size) {
    rsp_size=8;
    ICAddRSP(tempi,-8);
  }
  if (!dont_push_float) {
    if (builtin2 && !builtin1) {
      alt=!alt;
      SwapU64(&t1,&t2);
      SwapU64(&r1,&r2);
      SwapU64(&d1,&d2);
      SwapU64(&builtin1,&builtin2);
    }
    if (builtin1)
      ICU16(tempi,builtin1);
    else
      ICSlashOp(tempi,t1,r1,d1,SLASH_OP_FLD,ip);
  }
  if (alt)
    switch (op.u8[0]) {
      case 4: //FSUB
	op=SLASH_OP_FSUBR;
	break;
      case 6: //FDIV
	op=SLASH_OP_FDIVR;
	break;
    }
  if (builtin2) {
    ICU16(tempi,builtin2);
    ICU16(tempi,op.u16[2]);
  } else
    ICSlashOp(tempi,t2,r2,d2,op,ip);
  CmpNoteFloatOp(lx,tempi,TRUE,TRUE,CN_MAIN);
  if (dont_pop_float) {
    if (rsp_size)
      ICAddRSP(tempi,rsp_size);
  } else {
    if (rsp_size==8)
      ICSlashOp(tempi,TY_SIB+IT_I64,CREG_RSP+CREG_RSP<<8,0,SLASH_OP_FSTP,ip);
    else if (rsp_size>8) {
      ICSlashOp(tempi,TY_SIB+IT_I64,CREG_RSP+CREG_RSP<<8,rsp_size-8,SLASH_OP_FSTP,ip);
      ICAddRSP(tempi,rsp_size-8);
    }
    ICPop(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,ip);
  }
}

U0 ICFCmp(CLex *lx,CIntermediateCode *tempi,I64 op,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (tempi->ic_flags&ICF_ALTERNATE_TEMPLATE) {
      if (tempi->ic_flags&ICF_POP_CMP)
	ICPopRegs(tempi,1<<CREG_RAX);
      else
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    } else
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
  } else {
    if (tempi->ic_flags & ICF_P1_FIRST) {
      if (tempi->ic_flags&ICF_POP_CMP)
	ICPopRegs(tempi,1<<CREG_RDX);
      else
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	    tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    } else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      if (tempi->ic_flags&ICF_POP_CMP)
	ICPopRegs(tempi,1<<CREG_RDX);
      else
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	    tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    }
  }
  if (tempi->ic_flags&ICF_PUSH_CMP)
    ICPushRegs(tempi,1<<CREG_RAX);
  ICCopyTemplate(lx,tempi,op,FALSE,TRUE,FALSE,CN_MAIN);
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
      TY_REG+IT_I64,CREG_RAX,0,ip);
}

U0 ICFModEqual(CLex *lx,CIntermediateCode *tempi,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (dont_push_float) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
      if (tempi->p1.ptype!=IT_F64)
	ICFCvt2(lx,tempi,CREG_RAX,TY_REG+IT_I64,CREG_RAX,0,FALSE,ip);
    } else if (tempi->ic_flags & ICF_P1_FIRST) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	  tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
      if (tempi->p1.ptype!=IT_F64)
	ICFCvt2(lx,tempi,CREG_RDX,TY_REG+IT_I64,CREG_RDX,0,FALSE,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    } else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	  tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
      if (tempi->p1.ptype!=IT_F64)
	ICFCvt2(lx,tempi,CREG_RDX,TY_REG+IT_I64,CREG_RDX,0,FALSE,ip);
    }
    ICCopyTemplate(lx,tempi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_MAIN); //TODO: unpushable,unpop?  Not sure
    if (tempi->p1.ptype!=IT_F64)
      ICFCvt2(lx,tempi,CREG_RAX,TY_REG+IT_I64,CREG_RAX,0,TRUE,ip);
    ICMov(tempi,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	TY_REG+IT_I64,CREG_RAX,0,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
  } else {
    if (dont_push_float) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  TY_DISP+tempi->p1.ptype,CREG_RCX,0,ip);
      if (tempi->p1.ptype!=IT_F64)
	ICFCvt2(lx,tempi,CREG_RAX,TY_REG+IT_I64,CREG_RAX,0,FALSE,ip);
    } else if (tempi->ic_flags & ICF_P1_FIRST) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	  TY_DISP+tempi->p1.ptype,CREG_RCX,0,ip);
      if (tempi->p1.ptype!=IT_F64)
	ICFCvt2(lx,tempi,CREG_RDX,TY_REG+IT_I64,CREG_RDX,0,FALSE,ip);
    } else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	  TY_DISP+tempi->p1.ptype,CREG_RCX,0,ip);
      if (tempi->p1.ptype!=IT_F64)
	ICFCvt2(lx,tempi,CREG_RDX,TY_REG+IT_I64,CREG_RDX,0,FALSE,ip);
    }
    ICCopyTemplate(lx,tempi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_MAIN); //TODO: unpushable,unpop?  Not sure
    if (tempi->p1.ptype!=IT_F64)
      ICFCvt2(lx,tempi,CREG_RAX,TY_REG+IT_I64,CREG_RAX,0,TRUE,ip);
    ICMov(tempi,TY_DISP+tempi->p1.ptype,CREG_RCX,0,TY_REG+IT_I64,CREG_RAX,0,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  TY_REG+IT_I64,CREG_RAX,0,ip);
  }
}

U0 ICFOpEqual(CLex *lx,CIntermediateCode *tempi,I64 op,U8 *buf2,I64 ip)
{ //for ADD,SUB,DIV,MUL
  CICArg *p1=&tempi->p1,
      *p2=&tempi->p2;
  Bool dont_push_float,dont_pop_float,p1_mem;
  I64 rsp_size=0,builtin2=0,
     t1,r1,d1,t2,r2,d2;

  if (lx->flags&LF_AOT_COMPILE)
    buf2=lx->a->ip;

  CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  if (dont_pop_float)
    throw(EXCEPT_COMPILER,24);

  if (tempi->ic_flags & ICF_NOT_ADDR) {
    p1_mem=FALSE;
    if (dont_push_float) {
      if (p1->ptype!=IT_F64) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	    p1->type&TY_MASK+p1->ptype,p1->reg,p1->disp,ip);
	ICFCvt2(lx,tempi,CREG_RAX,TY_REG+IT_I64,CREG_RAX,0,FALSE,ip);
	ICPush(tempi,TY_REG+IT_I64,CREG_RAX,0,ip);
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	rsp_size+=8;
      } else {
	if (p1->ptype>=IT_I64 &&
	    (p1->type&TY_MASK==TY_DISP ||
	    p1->type&TY_MASK==TY_RIP_DISP32 ||
	    p1->type&TY_MASK==TY_SIB)) {
	  t1=p1->ptype+p1->type&TY_MASK;
	  r1=p1->reg;
	  d1=p1->disp;
	  p1_mem=TRUE;
	} else {
	  ICPush(tempi,p1->ptype+p1->type&TY_MASK,p1->reg,p1->disp,ip);
	  t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	  rsp_size+=8;
	}
      }
    } else if (tempi->ic_flags & ICF_P1_FIRST) {
      if (p1->ptype!=IT_F64 || p2->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	    p1->type&TY_MASK+p1->ptype,p1->reg,p1->disp,ip);
	if (p1->ptype!=IT_F64)
	  ICFCvt2(lx,tempi,CREG_RDX,TY_REG+IT_I64,CREG_RDX,0,FALSE,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	    p2->type,p2->reg,p2->disp,ip);

	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=8;
	t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
      } else {
	if (p1->ptype>=IT_I64 &&
	    (p1->type&TY_MASK==TY_DISP ||
	    p1->type&TY_MASK==TY_RIP_DISP32 ||
	    p1->type&TY_MASK==TY_SIB)) {
	  t1=p1->ptype+p1->type&TY_MASK;
	  r1=p1->reg;
	  d1=p1->disp;
	  p1_mem=TRUE;
	} else {
	  ICPush(tempi,p1->ptype+p1->type&TY_MASK,p1->reg,p1->disp,ip);
	  t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	  rsp_size+=8;
	}
	if (p2->type&IT_MASK>=IT_I64 &&
	    (p2->type&TY_MASK==TY_DISP ||
	    p2->type&TY_MASK==TY_RIP_DISP32 ||
	    p2->type&TY_MASK==TY_SIB)) {
	  t2=p2->type;
	  r2=p2->reg;
	  d2=p2->disp;
	} else {
	  if (p2->type&TY_MASK==TY_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(p2->disp(F64)))) {
	      t2=TY_RIP_DISP32+IT_I64;
	      r2=CREG_RIP;
	      d2=LCCFloatConstFind(lx,p2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,p2->type,p2->reg,p2->disp,ip);
	    t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
	    rsp_size+=8;
	    if (r1==CREG_RSP+CREG_RSP<<8)
	      d1+=8;
	  }
	}
      }
    } else {
      if (p1->ptype!=IT_F64 || p1->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	    p2->type,p2->reg,p2->disp,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	    p1->type&TY_MASK+p1->ptype,p1->reg,p1->disp,ip);
	if (p1->ptype!=IT_F64)
	  ICFCvt2(lx,tempi,CREG_RDX,TY_REG+IT_I64,CREG_RDX,0,FALSE,ip);
	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=8;
	t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
      } else {
	if (p2->type&IT_MASK>=IT_I64 &&
	    (p2->type&TY_MASK==TY_DISP ||
	    p2->type&TY_MASK==TY_RIP_DISP32 ||
	    p2->type&TY_MASK==TY_SIB)) {
	  t2=p2->type;
	  r2=p2->reg;
	  d2=p2->disp;
	} else {
	  if (p2->type&TY_MASK==TY_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(p2->disp(F64)))) {
	      t2=TY_RIP_DISP32+IT_I64;
	      r2=CREG_RIP;
	      d2=LCCFloatConstFind(lx,p2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,p2->type,p2->reg,p2->disp,ip);
	    t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
	    rsp_size+=8;
	  }
	}
	if (p1->ptype>=IT_I64 &&
	    (p1->type&TY_MASK==TY_DISP ||
	    p1->type&TY_MASK==TY_RIP_DISP32 ||
	    p1->type&TY_MASK==TY_SIB)) {
	  t1=p1->ptype+p1->type&TY_MASK;
	  r1=p1->reg;
	  d1=p1->disp;
	  p1_mem=TRUE;
	} else {
	  ICPush(tempi,p1->ptype+p1->type&TY_MASK,p1->reg,p1->disp,ip);
	  t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	  rsp_size+=8;
	  if (r2==CREG_RSP+CREG_RSP<<8)
	    d2+=8;
	}
      }
    }
    if (!rsp_size && !(p1_mem && p1->ptype==IT_F64)) {
      rsp_size=8;
      ICAddRSP(tempi,-8);
    }
    if (!dont_push_float) {
      if (builtin2)
	ICU16(tempi,builtin2);
      else
	ICSlashOp(tempi,t2,r2,d2,SLASH_OP_FLD,ip);
    }
    switch (op.u8[0]) {
      case 4: //FSUB
	op=SLASH_OP_FSUBR;
	break;
      case 6: //FDIV
	op=SLASH_OP_FDIVR;
	break;
    }
    ICSlashOp(tempi,t1,r1,d1,op,ip);
    CmpNoteFloatOp(lx,tempi,TRUE,FALSE,CN_MAIN);
    if (p1_mem && p1->ptype==IT_F64) {
      ICSlashOp(tempi,t1,r1,d1,SLASH_OP_FSTP,ip);
      if (rsp_size)
	ICAddRSP(tempi,rsp_size);
    } else {
      if (rsp_size==8)
	ICSlashOp(tempi,TY_SIB+IT_I64,CREG_RSP+CREG_RSP<<8,0,SLASH_OP_FSTP,ip);
      else if (rsp_size>8) {
	ICSlashOp(tempi,TY_SIB+IT_I64,CREG_RSP+CREG_RSP<<8,rsp_size-8,SLASH_OP_FSTP,ip);
	ICAddRSP(tempi,rsp_size-8);
      }
      if (p1->ptype!=IT_F64) {
	ICPop(tempi,TY_REG+IT_I64,CREG_RAX,0,ip);
	ICFCvt2(lx,tempi,CREG_RAX,TY_REG+IT_I64,CREG_RAX,0,TRUE,ip);
	ICMov(tempi,p1->type&TY_MASK+p1->ptype,p1->reg,p1->disp,
	    TY_REG+IT_I64,CREG_RAX,0,ip);
      } else
	ICPop(tempi,p1->ptype+p1->type&TY_MASK,p1->reg,p1->disp,ip);
    }
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  p1->type&TY_MASK+p1->ptype,p1->reg,p1->disp,ip);
  } else {
    if (p1->ptype>=IT_I64)
      p1_mem=TRUE;
    else
      p1_mem=FALSE;
    if (dont_push_float) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p1->type,p1->reg,p1->disp,ip);
      if (p1->ptype!=IT_F64) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	    TY_DISP+p1->ptype,CREG_RCX,0,ip);
	ICFCvt2(lx,tempi,CREG_RAX,TY_REG+IT_I64,CREG_RAX,0,FALSE,ip);
	ICPush(tempi,TY_REG+IT_I64,CREG_RAX,0,ip);
      } else
	ICPush(tempi,p1->ptype+TY_DISP,CREG_RCX,0,ip);
      t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
      rsp_size+=8;
    } else if (tempi->ic_flags & ICF_P1_FIRST) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p1->type,p1->reg,p1->disp,ip);
      if (p1->ptype!=IT_F64 || p2->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	    TY_DISP+p1->ptype,CREG_RCX,0,ip);
	if (p1->ptype!=IT_F64)
	  ICFCvt2(lx,tempi,CREG_RDX,TY_REG+IT_I64,CREG_RDX,0,FALSE,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	    p2->type,p2->reg,p2->disp,ip);

	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=8;
	t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
      } else {
	ICPush(tempi,TY_DISP+p1->ptype,CREG_RCX,0,ip);
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	rsp_size+=8;
	if (p2->type&IT_MASK>=IT_I64 &&
	    (p2->type&TY_MASK==TY_DISP ||
	    p2->type&TY_MASK==TY_RIP_DISP32 ||
	    p2->type&TY_MASK==TY_SIB)) {
	  t2=p2->type;
	  r2=p2->reg;
	  d2=p2->disp;
	} else {
	  if (p2->type&TY_MASK==TY_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(p2->disp(F64)))) {
	      t2=TY_RIP_DISP32+IT_I64;
	      r2=CREG_RIP;
	      d2=LCCFloatConstFind(lx,p2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,p2->type,p2->reg,p2->disp,ip);
	    t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
	    rsp_size+=8;
	    if (r1==CREG_RSP+CREG_RSP<<8)
	      d1+=8;
	  }
	}
      }
    } else {
      if (p1->ptype!=IT_F64 || p1->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	    p2->type,p2->reg,p2->disp,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p1->type,p1->reg,p1->disp,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	    TY_DISP+p1->ptype,CREG_RCX,0,ip);
	if (p1->ptype!=IT_F64)
	  ICFCvt2(lx,tempi,CREG_RDX,TY_REG+IT_I64,CREG_RDX,0,FALSE,ip);
	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=8;
	t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
      } else {
	if (p2->type&IT_MASK>=IT_I64 &&
	    (p2->type&TY_MASK==TY_DISP ||
	    p2->type&TY_MASK==TY_RIP_DISP32 ||
	    p2->type&TY_MASK==TY_SIB)) {
	  t2=p2->type;
	  r2=p2->reg;
	  d2=p2->disp;
	} else {
	  if (p2->type&TY_MASK==TY_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(p2->disp(F64)))) {
	      t2=TY_RIP_DISP32+IT_I64;
	      r2=CREG_RIP;
	      d2=LCCFloatConstFind(lx,p2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,p2->type,p2->reg,p2->disp,ip);
	    t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
	    rsp_size+=8;
	  }
	}
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p1->type,p1->reg,p1->disp,ip);
	ICPush(tempi,TY_DISP+p1->ptype,CREG_RCX,0,ip);
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	rsp_size+=8;
	if (r2==CREG_RSP+CREG_RSP<<8)
	  d2+=8;
      }
    }
    if (!rsp_size && !(p1_mem && p1->ptype==IT_F64)) {
      rsp_size=8;
      ICAddRSP(tempi,-8);
    }
    if (!dont_push_float) {
      if (builtin2)
	ICU16(tempi,builtin2);
      else
	ICSlashOp(tempi,t2,r2,d2,SLASH_OP_FLD,ip);
    }

    switch (op.u8[0]) {
      case 4: //FSUB
	op=SLASH_OP_FSUBR;
	break;
      case 6: //FDIV
	op=SLASH_OP_FDIVR;
	break;
    }
    ICSlashOp(tempi,t1,r1,d1,op,ip);
    CmpNoteFloatOp(lx,tempi,TRUE,FALSE,CN_MAIN);
    if (p1_mem && p1->ptype==IT_F64) {
      ICSlashOp(tempi,TY_DISP+p1->ptype,CREG_RCX,0,SLASH_OP_FSTP,ip);
      if (rsp_size)
	ICAddRSP(tempi,rsp_size);
    } else {
      if (rsp_size==8)
	ICSlashOp(tempi,TY_SIB+IT_I64,CREG_RSP+CREG_RSP<<8,0,SLASH_OP_FSTP,ip);
      else if (rsp_size>8) {
	ICSlashOp(tempi,TY_SIB+IT_I64,CREG_RSP+CREG_RSP<<8,rsp_size-8,SLASH_OP_FSTP,ip);
	ICAddRSP(tempi,rsp_size-8);
      }
      ICPop(tempi,TY_REG+IT_I64,CREG_RAX,0,ip);
      if (p1->ptype!=IT_F64)
	ICFCvt2(lx,tempi,CREG_RAX,TY_REG+IT_I64,CREG_RAX,0,TRUE,ip);
      ICMov(tempi,TY_DISP+p1->ptype,CREG_RCX,0,TY_REG+IT_I64,CREG_RAX,0,ip);
    }
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  TY_REG+IT_I64,CREG_RAX,0,ip);
  }
}
 
U0 ICFCmpAndBranch(CLex *lx,CIntermediateCode *tempi,I64 ip,
   I64 us,I64 not_us,U8 *buf,I64 ip2)
{
  CICArg *p1,*p2;
  Bool dont_push_float,dont_pop_float,alt,short_jmp;
  I64 i,rsp_size=0,builtin1=0,builtin2=0,
	t1,r1,d1,t2,r2,d2;
  CLexCodeMisc *lb;
  U8 *buf2;

  if (tempi->ic_flags&ICF_ALTERNATE_TEMPLATE) {
    p1=&tempi->p2;
    p2=&tempi->p1;
    alt=TRUE;
  } else {
    p1=&tempi->p1;
    p2=&tempi->p2;
    alt=FALSE;
  }

  if (lx->flags&LF_AOT_COMPILE)
    buf2=lx->a->ip;
  else
    buf2=buf;

  CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (tempi->ic_flags&ICF_POP_CMP && alt) {
      t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
      rsp_size+=8;
    } else {
      if (p2->type&IT_MASK>=IT_I64 &&
	    (p2->type&TY_MASK==TY_DISP ||
	    p2->type&TY_MASK==TY_RIP_DISP32 ||
	    p2->type&TY_MASK==TY_SIB)) {
	t2=p2->type;
	r2=p2->reg;
	d2=p2->disp;
      } else {
	if (p2->type&TY_MASK==TY_IMM) {
	  if (!(builtin2=ICBuiltInFloatConst(p2->disp(F64))) ||
		tempi->ic_flags&ICF_PUSH_CMP) {
	    t2=TY_RIP_DISP32+IT_I64;
	    r2=CREG_RIP;
	    d2=LCCFloatConstFind(lx,p2->disp(F64))+buf2;
	  }
	} else {
	  ICPush(tempi,p2->type,p2->reg,p2->disp,ip2);
	  t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
	  rsp_size+=8;
	}
      }
    }
  } else {
    if (tempi->ic_flags & ICF_P1_FIRST^^alt) {
      if (p2->type&TY_MASK!=TY_STK) {
	if (tempi->ic_flags&ICF_POP_CMP) {
	  t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	  rsp_size+=8;
	} else {
	  if (p1->type&IT_MASK>=IT_I64 &&
		(p1->type&TY_MASK==TY_DISP ||
		p1->type&TY_MASK==TY_RIP_DISP32 ||
		p1->type&TY_MASK==TY_SIB)) {
	    t1=p1->type;
	    r1=p1->reg;
	    d1=p1->disp;
	  } else {
	    if (p1->type&TY_MASK==TY_IMM) {
	      if (!(builtin1=ICBuiltInFloatConst(p1->disp(F64)))) {
		t1=TY_RIP_DISP32+IT_I64;
		r1=CREG_RIP;
		d1=LCCFloatConstFind(lx,p1->disp(F64))+buf2;
	      }
	    } else {
	      ICPush(tempi,p1->type,p1->reg,p1->disp,ip2);
	      t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	      rsp_size+=8;
	    }
	  }
	}
	if (p2->type&IT_MASK>=IT_I64 &&
	      (p2->type&TY_MASK==TY_DISP ||
	      p2->type&TY_MASK==TY_RIP_DISP32 ||
	      p2->type&TY_MASK==TY_SIB)) {
	  t2=p2->type;
	  r2=p2->reg;
	  d2=p2->disp;
	} else {
	  if (p2->type&TY_MASK==TY_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(p2->disp(F64))) ||
		  tempi->ic_flags&ICF_PUSH_CMP) {
	      t2=TY_RIP_DISP32+IT_I64;
	      r2=CREG_RIP;
	      d2=LCCFloatConstFind(lx,p2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,p2->type,p2->reg,p2->disp,ip2);
	    t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
	    rsp_size+=8;
	    if (r1==CREG_RSP+CREG_RSP<<8)
	      d1+=8;
	  }
	}
      } else {
	if (tempi->ic_flags&ICF_POP_CMP)
	  ICPopRegs(tempi,1<<CREG_RDX);
	else
	  ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
		p1->type,p1->reg,p1->disp,ip2);
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	      p2->type,p2->reg,p2->disp,ip2);
	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=8;
	t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
      }
    } else {
      if (p1->type&TY_MASK!=TY_STK) {
	if (p2->type&IT_MASK>=IT_I64 &&
	      (p2->type&TY_MASK==TY_DISP ||
	      p2->type&TY_MASK==TY_RIP_DISP32 ||
	      p2->type&TY_MASK==TY_SIB)) {
	  t2=p2->type;
	  r2=p2->reg;
	  d2=p2->disp;
	} else {
	  if (p2->type&TY_MASK==TY_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(p2->disp(F64))) ||
		  tempi->ic_flags&ICF_PUSH_CMP) {
	      t2=TY_RIP_DISP32+IT_I64;
	      r2=CREG_RIP;
	      d2=LCCFloatConstFind(lx,p2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,p2->type,p2->reg,p2->disp,ip2);
	    t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
	    rsp_size+=8;
	  }
	}
	if (tempi->ic_flags&ICF_POP_CMP) {
	  t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	  rsp_size+=8;
	  if (r2==CREG_RSP+CREG_RSP<<8)
	    d2+=8;
	} else {
	  if (p1->type&IT_MASK>=IT_I64 &&
		(p1->type&TY_MASK==TY_DISP ||
		p1->type&TY_MASK==TY_RIP_DISP32 ||
		p1->type&TY_MASK==TY_SIB)) {
	    t1=p1->type;
	    r1=p1->reg;
	    d1=p1->disp;
	  } else {
	    if (p1->type&TY_MASK==TY_IMM) {
	      if (!(builtin1=ICBuiltInFloatConst(p1->disp(F64)))) {
		t1=TY_RIP_DISP32+IT_I64;
		r1=CREG_RIP;
		d1=LCCFloatConstFind(lx,p1->disp(F64))+buf2;
	      }
	    } else {
	      ICPush(tempi,p1->type,p1->reg,p1->disp,ip2);
	      t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=0;
	      rsp_size+=8;
	      if (r2==CREG_RSP+CREG_RSP<<8)
		d2+=8;
	    }
	  }
	}
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	      p2->type,p2->reg,p2->disp,ip2);
	if (tempi->ic_flags&ICF_POP_CMP)
	  ICPopRegs(tempi,1<<CREG_RDX);
	else
	  ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
		p1->type,p1->reg,p1->disp,ip2);
	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=TY_SIB+IT_I64; r1=CREG_RSP+CREG_RSP<<8; d1=8;
	t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
      }
    }
  }
  if (!dont_push_float) {
    if (builtin1)
      ICU16(tempi,builtin1);
    else
      ICSlashOp(tempi,t1,r1,d1,SLASH_OP_FLD,ip2);
  }
  if (!alt)
    us=not_us;
  if (builtin2)
    ICU16(tempi,builtin2);
  else
    ICSlashOp(tempi,t2,r2,d2,SLASH_OP_FLD,ip2);
  if (tempi->ic_flags&ICF_PUSH_CMP) {
    t2=TY_SIB+IT_I64; r2=CREG_RSP+CREG_RSP<<8; d2=0;
    if (!rsp_size) {
      rsp_size=8;
      ICAddRSP(tempi,-8);
    } else if (rsp_size==16)
      d2=8;
  }
  if (tempi->ic_flags&ICF_PUSH_CMP) {
    if (alt) {
      ICU16(tempi,0xF1DF);	  //FCOMIP
      ICSlashOp(tempi,t2,r2,d2,SLASH_OP_FSTP,ip2);
    } else {
      ICU16(tempi,0xF1DB);	  //FCOMI
      ICSlashOp(tempi,t2,r2,d2,SLASH_OP_FSTP,ip2);
      ICU32(tempi,0xF7D9C0DD); //FFREE,FINCSTP
    }
  } else {
    ICU16(tempi,0xF1DF);	  //FCOMIP
    ICU32(tempi,0xF7D9C0DD); //FFREE,FINCSTP
  }
  CmpNoteFloatOp(lx,tempi,TRUE,FALSE,CN_MAIN);

  if (tempi->ic_flags&ICF_PUSH_CMP) {
    if (r2.u8[0]==CREG_RSP) {
      while (d2 && rsp_size) {
	ICU8(tempi,0x5B); //POP RBX (Dont change flags)
	rsp_size-=8;
	d2-=8;
      }
    } else {
      while (rsp_size) {
	ICU8(tempi,0x5B); //POP RBX (Dont change flags)
	rsp_size-=8;
      }
      ICPush(tempi,t2,r2,d2,ip2);
    }
  } else {
    while (rsp_size) {
      ICU8(tempi,0x5B); //POP RBX (Dont change flags)
      rsp_size-=8;
    }
  }

  ip+=tempi->ic_cnt;
  lb=OptLabelFwd(tempi->ic_data);
  short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP) {
      if(tempi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tempi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP) {
      if(tempi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    ICU16(tempi,i<<8+us.u8[2]);
  } else {
    tempi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP) {
      if(tempi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    ICU16(tempi,us.u16[0]);
    ICU32(tempi,i);
  }
}

U0 ICFPreIncDec(CLex *lx,CIntermediateCode *tempi,I64 op,I64 ip)
{
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
      tempi->p1.type&TY_MASK+IT_I64,tempi->p1.reg,tempi->p1.disp,ip);
    ICCopyTemplate(lx,tempi,op,FALSE,TRUE,TRUE,CN_MAIN);
    ICMov(tempi,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,
      TY_REG+IT_I64,CREG_RAX,0,ip);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,
      tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
      TY_DISP+IT_I64,CREG_RCX,0,ip);
    ICCopyTemplate(lx,tempi,op,FALSE,TRUE,TRUE,CN_MAIN);
    ICMov(tempi,TY_DISP+IT_I64,CREG_RCX,0,
      TY_REG+IT_I64,CREG_RAX,0,ip);
  }
  if (tempi->r.type&TY_MASK)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
      TY_REG+IT_I64,CREG_RAX,0,ip);
}

U0 ICFPostIncDec(CLex *lx,CIntermediateCode *tempi,I64 op,I64 ip)
{
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
      tempi->p1.type&TY_MASK+IT_I64,tempi->p1.reg,tempi->p1.disp,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	TY_REG+IT_I64,CREG_RAX,0,ip);
    ICCopyTemplate(lx,tempi,op,FALSE,TRUE,TRUE,CN_MAIN);
    ICMov(tempi,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,
      TY_REG+IT_I64,CREG_RAX,0,ip);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,
      tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
      TY_DISP+IT_I64,CREG_RCX,0,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	TY_REG+IT_I64,CREG_RAX,0,ip);
    ICCopyTemplate(lx,tempi,op,FALSE,TRUE,TRUE,CN_MAIN);
    ICMov(tempi,TY_DISP+IT_I64,CREG_RCX,0,
      TY_REG+IT_I64,CREG_RAX,0,ip);
  }
  if (tempi->r.type&TY_MASK)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
      TY_REG+IT_I64,CREG_RDX,0,ip);
}

U0 ICMul(CIntermediateCode *tempi,I64 ip)
{
  I64 i,r2,r=CREG_RAX,j;
  CICArg *p1,*p2;
  if (tempi->p1.type&TY_MASK==TY_IMM) {
    p1=&tempi->p2;
    p2=&tempi->p1;
    tempi->ic_flags|=ICF_P1_FIRST;
  } else {
    p1=&tempi->p1;
    p2=&tempi->p2;
  }
  i=p2->disp;
  if (!(tempi->ic_class->sub_type&1) &&
      p2->type&TY_MASK==TY_IMM && MIN_I32<=i<=MAX_I32) {
    if (tempi->r.type==TY_REG+IT_I64) {
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,p1->type,p1->reg,p1->disp,ip);
      r=tempi->r.reg;
    } else
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,p1->type,p1->reg,p1->disp,ip);
    if (r>7)
      j=0xC0004D;
    else
      j=0xC00048;
    if (MIN_I8<=i<=MAX_I8)
      ICU32(tempi,i<<24+0x6B00+j+(r&7)<<16+(r&7)<<19);
    else {
      ICU24(tempi,0x6900+j+(r&7)<<16+(r&7)<<19);
      ICU32(tempi,i);
    }
  } else {
    if (tempi->ic_class->sub_type&1)
      i=0xE0F748;
    else
      i=0xE8F748;
    if (tempi->ic_flags & ICF_P1_FIRST) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p1->type,p1->reg,p1->disp,ip);
      r2=CREG_RCX;
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,p2->type,p2->reg,p2->disp,ip);
    } else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,p2->type,p2->reg,p2->disp,ip);
      if (p1->type&TY_MASK!=TY_REG ||
	  p1->type&IT_MASK<IT_I64) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p1->type,p1->reg,p1->disp,ip);
	r2=CREG_RCX;
      } else
	r2=p1->reg;
    }
    if (r2>7) {
      i++;
      r2&=7;
    }
    ICU24(tempi,i+r2<<16);
  }
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+IT_I64,r,0,ip);
}


U0 ICFMul(CLex *lx,CIntermediateCode *tempi,U8 *buf,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  if (tempi->p1.type&TY_MASK==TY_IMM &&
      tempi->p1.ptype&IT_MASK==IT_F64 &&
      tempi->p1.disp(F64)==1.0) {
    if (dont_push_float) {
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  TY_REG+IT_I64,CREG_RAX,0,ip);
    } else if (dont_pop_float) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_MAIN);
    } else {
      CmpNoteFloatOp(lx,tempi,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    }
  } else if (tempi->p2.type&TY_MASK==TY_IMM &&
      tempi->p2.ptype&IT_MASK==IT_F64 &&
      tempi->p2.disp(F64)==1.0) {
    if (dont_push_float) {
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  TY_REG+IT_I64,CREG_RAX,0,ip);
    } else if (dont_pop_float) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_MAIN);
    } else {
      CmpNoteFloatOp(lx,tempi,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    }
  } else
    ICFOp(lx,tempi,SLASH_OP_FMUL,buf,ip);
}

U0 ICFDiv(CLex *lx,CIntermediateCode *tempi,U8 *buf,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  if (tempi->p2.type&TY_MASK==TY_IMM &&
      tempi->p2.ptype&IT_MASK==IT_F64 &&
      tempi->p2.disp(F64)==1.0) {
    CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
    if (dont_push_float) {
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  TY_REG+IT_I64,CREG_RAX,0,ip);
    } else if (dont_pop_float) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_MAIN);
    } else {
      CmpNoteFloatOp(lx,tempi,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    }
  } else
    ICFOp(lx,tempi,SLASH_OP_FDIV,buf,ip);
}

U0 ICFAdd(CLex *lx,CIntermediateCode *tempi,U8 *buf,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  if (tempi->p1.type&TY_MASK==TY_IMM &&
      !tempi->p1.disp) {
    if (dont_push_float) {
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  TY_REG+IT_I64,CREG_RAX,0,ip);
    } else if (dont_pop_float) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_MAIN);
    } else {
      CmpNoteFloatOp(lx,tempi,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    }
  } else if (tempi->p2.type&TY_MASK==TY_IMM &&
      !tempi->p2.disp) {
    if (dont_push_float) {
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  TY_REG+IT_I64,CREG_RAX,0,ip);
    } else if (dont_pop_float) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_MAIN);
    } else {
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      CmpNoteFloatOp(lx,tempi,TRUE,TRUE,CN_MAIN);
    }
  } else
    ICFOp(lx,tempi,SLASH_OP_FADD,buf,ip);
}

U0 ICFSub(CLex *lx,CIntermediateCode *tempi,U8 *buf,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  if (tempi->p2.type&TY_MASK==TY_IMM &&
      !tempi->p2.disp) {
    CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
    if (dont_push_float) {
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  TY_REG+IT_I64,CREG_RAX,0,ip);
    } else if (dont_pop_float) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICCopyTemplate(lx,tempi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_MAIN);
    } else {
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      CmpNoteFloatOp(lx,tempi,TRUE,TRUE,CN_MAIN);
    }
  } else
    ICFOp(lx,tempi,SLASH_OP_FSUB,buf,ip);
}

U0 ICMulEqual(CIntermediateCode *tempi,I64 ip)
{
  I64 i=tempi->p2.disp,r=CREG_RAX,j;
  if (!(tempi->ic_class->sub_type&1) &&
      tempi->p2.type&TY_MASK==TY_IMM && MIN_I32<=i<=MAX_I32) {
    if (tempi->ic_flags & ICF_NOT_ADDR) {
      if (tempi->p1.type==TY_REG+IT_I64)
	r=tempi->p1.reg;
      else
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
      if (r>7)
	j=0xC0004D;
      else
	j=0xC00048;
      if (MIN_I8<=i<=MAX_I8)
	ICU32(tempi,i<<24+0x6B00+j+(r&7)<<16+(r&7)<<19);
      else {
	ICU24(tempi,0x6900+j+(r&7)<<16+(r&7)<<19);
	ICU32(tempi,i);
      }
      ICMov(tempi,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	TY_REG+IT_I64,r,0,ip);
    } else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,TY_DISP+tempi->p1.ptype,CREG_RCX,0,ip);
      r=CREG_RBX;
      if (MIN_I8<=i<=MAX_I8)
	ICU32(tempi,i<<24+0xDB6B48);
      else {
	ICU24(tempi,0xDB6948);
	ICU32(tempi,i);
      }
      ICMov(tempi,TY_DISP+tempi->p1.ptype,CREG_RCX,0,TY_REG+IT_I64,CREG_RBX,0,ip);
    }
  } else {
    if (tempi->ic_class->sub_type&1)
      i=0xE3F748;
    else
      i=0xEBF748;
    if (tempi->ic_flags & ICF_NOT_ADDR) {
      if (tempi->ic_flags & ICF_P1_FIRST) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
      }
      ICU24(tempi,i);
      ICMov(tempi,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	TY_REG+IT_I64,CREG_RAX,0,ip);
    } else {
      if (tempi->ic_flags & ICF_P1_FIRST) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      }
      ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,TY_DISP+tempi->p1.ptype,CREG_RCX,0,ip);
      ICU24(tempi,i);
      ICMov(tempi,TY_DISP+tempi->p1.ptype,CREG_RCX,0,TY_REG+IT_I64,CREG_RAX,0,ip);
    }
  }
  if (tempi->r.type&TY_MASK)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+IT_I64,r,0,ip);
}


U0 ICDiv(CIntermediateCode *tempi,I64 ip)
{
  if (tempi->ic_flags & ICF_P1_FIRST) {
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
  }
  if (tempi->ic_class->sub_type&1) {
    ICZero(tempi,CREG_RDX);
    ICU24(tempi,0xF1F748);
  } else {
    ICU16(tempi,0x9948);
    ICU24(tempi,0xF9F748);
  }
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+IT_I64,CREG_RAX,0,ip);
}

U0 ICDivEqual(CIntermediateCode *tempi,Bool is_mod,I64 ip)
{
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempi->ic_flags & ICF_P1_FIRST) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    } else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
    }
    if (tempi->ic_class->sub_type&1) {
      ICZero(tempi,CREG_RDX);
      ICU24(tempi,0xF1F748);
    } else {
      ICU16(tempi,0x9948);
      ICU24(tempi,0xF9F748);
    }
    if (is_mod)
      ICMov(tempi,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	TY_REG+IT_I64,CREG_RDX,0,ip);
    else
      ICMov(tempi,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	TY_REG+IT_I64,CREG_RAX,0,ip);
  } else {
    if (tempi->ic_flags & ICF_P1_FIRST) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      ICU24(tempi,0xD98748); //xchg bx,cx
    } else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    }
    //dangerous might clobber RBX in Mov, but it doesn't
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,TY_DISP+tempi->p1.ptype,CREG_RBX,0,ip);
    if (tempi->ic_class->sub_type&1) {
      ICZero(tempi,CREG_RDX);
      ICU24(tempi,0xF1F748);
    } else {
      ICU16(tempi,0x9948);
      ICU24(tempi,0xF9F748);
    }
    if (is_mod)
      ICMov(tempi,TY_DISP+tempi->p1.ptype,CREG_RBX,0,TY_REG+IT_I64,CREG_RDX,0,ip);
    else
      ICMov(tempi,TY_DISP+tempi->p1.ptype,CREG_RBX,0,TY_REG+IT_I64,CREG_RAX,0,ip);
  }
  if (tempi->r.type&TY_MASK) {
    if (is_mod)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+IT_I64,CREG_RDX,0,ip);
    else
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+IT_I64,CREG_RAX,0,ip);
  }
}

U0 ICMod(CIntermediateCode *tempi,I64 ip)
{
  if (tempi->ic_flags & ICF_P1_FIRST) {
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
  }
  if (tempi->ic_class->sub_type&1) {
    ICZero(tempi,CREG_RDX);
    ICU24(tempi,0xF1F748);
  } else {
    ICU16(tempi,0x9948);
    ICU24(tempi,0xF9F748);
  }
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+IT_I64,CREG_RDX,0,ip);
}

U0 ICAddSubEctEqual(CIntermediateCode *tempi,I64 ptype,I64 t1,I64 r1,I64 d1,
	   I64 t2,I64 r2,I64 d2,I64 t3,I64 r3,I64 d3,I64 op,I64 ip)
{
  Bool done;
  I64 result_reg,temp,i;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (t3&TY_MASK==TY_IMM) {
      ICAddSubEctImm(tempi,
	t2&TY_MASK+ptype,r2,d2,t2&TY_MASK+ptype,r2,d2,d3,op,ip);
      if (t1&TY_MASK)
	ICMov(tempi,t1,r1,d1,t2&TY_MASK+ptype,r2,d2,ip);
      return;
    } else {
      done=FALSE;
      if (ptype>=IT_I64) {
	if (!(t1&TY_MASK) &&
	    TY_REG<=t2&TY_MASK<=TY_RIP_DISP32) {
	  if (t3&TY_MASK==TY_REG)
	    temp=r3;
	  else {
	    temp=CREG_RCX;
	    ICMov(tempi,TY_REG+IT_I64,temp,0,t3,r3,d3,ip);
	  }
	  i=ICModr1(temp,t2&TY_MASK+ptype,r2,d2);
	  if (tempi->ic_flags&ICF_LOCK)
	    ICU8(tempi,OC_LOCK_PREFIX);
	  ICRex(tempi,i.u8[1]);
	  ICU16(tempi,i.u8[2]<<8+op.u8[5]);
	  ICModr2(tempi,i,d2,ip+4);
	  return;
	}
	if (t3&IT_MASK>=IT_I64 &&
	    TY_REG<=t3&TY_MASK<=TY_RIP_DISP32) {
	  if (t2&TY_MASK==TY_REG)
	    result_reg=r2;
	  else {
	    result_reg=CREG_RCX;
	    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
	  }
	  i=ICModr1(result_reg,t3&TY_MASK+ptype,r3,d3);
	  if (tempi->ic_flags&ICF_LOCK)
	    ICU8(tempi,OC_LOCK_PREFIX);
	  ICRex(tempi,i.u8[1]);
	  ICU16(tempi,i.u8[2]<<8+op.u8[0]);
	  ICModr2(tempi,i,d3,ip+4);
	  ICMov(tempi,t2&TY_MASK+ptype,r2,d2,TY_REG+IT_I64,result_reg,0,ip);
	  done=TRUE;
	}
      }
      if (!done) {
	if (tempi->ic_flags & ICF_P1_FIRST) {
	  if (t2&TY_MASK==TY_REG && r2!=CREG_RAX)
	    result_reg=r2;
	  else {
	    result_reg=CREG_RCX;
	    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2&TY_MASK+ptype,r2,d2,ip);
	  }
	  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t3,r3,d3,ip);
	} else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t3,r3,d3,ip);
	  if (t2&TY_MASK==TY_REG && r2!=CREG_RAX)
	    result_reg=r2;
	  else {
	    result_reg=CREG_RCX;
	    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2&TY_MASK+ptype,r2,d2,ip);
	  }
	}
	if (tempi->ic_flags&ICF_LOCK)
	  ICU8(tempi,OC_LOCK_PREFIX);
	if (result_reg>7)
	  ICU8(tempi,0x4C);
	else
	  ICU8(tempi,0x48);
	ICU16(tempi,0xC000+op.u8[0]+(result_reg&7)<<11);
	ICMov(tempi,t2&TY_MASK+ptype,r2,d2,TY_REG+IT_I64,result_reg,0,ip);
      }
    }
  } else {
    done=FALSE;
    if (t3&TY_MASK==TY_IMM && op.u8[2]) {
      if (!d3.u32[1]) {
	if (tempi->ic_flags&ICF_NO_DEPEND_RESULT &&
	    t2&TY_MASK==TY_REG && d3(U64)<=MAX_I8) {
	  ICSlashOp(tempi,TY_DISP+ptype,r2,0,0x838000+op.u8[4],ip);
	  ICU8(tempi,d3);
	  done=TRUE;
	} else if (op.u8[2]==0x24) { //AND
	  ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
	  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,TY_DISP+ptype,CREG_RCX,0,ip);
	  result_reg=CREG_RAX;
	  if (tempi->ic_flags&ICF_LOCK)
	    ICU8(tempi,OC_LOCK_PREFIX);
	  ICU16(tempi,op.u8[3]<<8+0x40);
	  ICU32(tempi,d3);
	  ICMov(tempi,TY_DISP+ptype,CREG_RCX,0,TY_REG+IT_I64,result_reg,0,ip);
	  done=TRUE;
	} else if (ptype&IT_MASK<IT_I64) { //OR/XOR
	  ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
	  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,TY_DISP+ptype,CREG_RCX,0,ip);
	  result_reg=CREG_RAX;
	  if (tempi->ic_flags&ICF_LOCK)
	    ICU8(tempi,OC_LOCK_PREFIX);
	  if (d3.u16[1]) {
	    ICU16(tempi,op.u8[3]<<8+0x40);
	    ICU32(tempi,d3);
	  } else if (d3.u8[1]) {
	    ICU24(tempi,op.u8[3]<<16+0x4000+OC_OP_SIZE_PREFIX);
	    ICU16(tempi,d3);
	  } else {
	    ICU16(tempi,op.u8[2]<<8+0x40);
	    ICU8(tempi,d3);
	  }
	  ICMov(tempi,TY_DISP+ptype,CREG_RCX,0,TY_REG+IT_I64,result_reg,0,ip);
	  done=TRUE;
	}
      }
    }
    if (!done) {
      if (tempi->ic_flags & ICF_P1_FIRST) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t3,r3,d3,ip);
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t3,r3,d3,ip);
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
      }
      ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,TY_DISP+ptype,CREG_RCX,0,ip);
      result_reg=CREG_RBX;
      if (tempi->ic_flags&ICF_LOCK)
	ICU8(tempi,OC_LOCK_PREFIX);
      ICU8(tempi,0x48);
      ICU16(tempi,0xC000+op.u8[0]+(result_reg&7)<<11);
      ICMov(tempi,TY_DISP+ptype,CREG_RCX,0,TY_REG+IT_I64,result_reg,0,ip);
    }
  }
  if (t1&TY_MASK)
    ICMov(tempi,t1,r1,d1,TY_REG+IT_I64,result_reg,0,ip);
}

U0 ICShift(CIntermediateCode *tempi,I64 t1,I64 r1,I64 d1,
	   I64 t2,I64 r2,I64 d2,I64 t3,I64 r3,I64 d3,I64 us,I64 is,I64 ip)
{
  I64 i=0x48,result_reg;
  if (tempi->ic_class->sub_type&1 || tempi->ic_flags & ICF_USE_UNSIGNED)
    is=us;
  if (t3&TY_MASK==TY_IMM) {
    if (t1&TY_MASK==TY_REG) {
      result_reg=r1;
      if (result_reg>7)
	i++;
    } else
      result_reg=CREG_RAX;
    ICMov(tempi,TY_REG+IT_I64,result_reg,0,t2,r2,d2,ip);
    if (d3==1)
      ICU24(tempi,i+is.u16[2]<<8+(result_reg&7)<<16);
    else {
      ICU24(tempi,i+is.u16[0]<<8+(result_reg&7)<<16);
      ICU8(tempi,d3);
    }
  } else {
    if (tempi->ic_flags & ICF_P1_FIRST) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t2,r2,d2,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t3,r3,d3,ip);
    } else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t3,r3,d3,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t2,r2,d2,ip);
    }
    result_reg=CREG_RAX;
    ICU24(tempi,i+is.u16[1]<<8);
  }
  ICMov(tempi,t1,r1,d1,TY_REG+IT_I64,result_reg,0,ip);
}

U0 ICShiftEqual(CIntermediateCode *tempi,I64 ptype,I64 t1,I64 r1,I64 d1,
	   I64 t2,I64 r2,I64 d2,I64 t3,I64 r3,I64 d3,I64 us,I64 is,I64 ip)
{
  I64 result_reg;
  if (tempi->ic_class->sub_type&1 || tempi->ic_flags & ICF_USE_UNSIGNED)
    is=us;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempi->ic_flags & ICF_P1_FIRST) {
      if (t2&TY_MASK==TY_REG)
	result_reg=r2;
      else {
	result_reg=CREG_RAX;
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t2&TY_MASK+ptype,r2,d2,ip);
      }
      if (t3&TY_MASK!=TY_IMM)
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t3,r3,d3,ip);
    } else {
      if (t3&TY_MASK!=TY_IMM)
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t3,r3,d3,ip);
      if (t2&TY_MASK==TY_REG)
	result_reg=r2;
      else {
	result_reg=CREG_RAX;
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,t2&TY_MASK+ptype,r2,d2,ip);
      }
    }
    if (result_reg>7)
      ICU8(tempi,0x49);
    else
      ICU8(tempi,0x48);
    if (t3&TY_MASK==TY_IMM) {
      if (d3==1)
	ICU16(tempi,is.u16[2]+(result_reg&7)<<8);
      else {
	ICU16(tempi,is.u16[0]+(result_reg&7)<<8);
	ICU8(tempi,d3);
      }
    } else
      ICU16(tempi,is.u16[1]+(result_reg&7)<<8);
    ICMov(tempi,t2&TY_MASK+ptype,r2,d2,TY_REG+IT_I64,result_reg,0,ip);
  } else {
    if (tempi->ic_flags & ICF_P1_FIRST) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,t2,r2,d2,ip);
      if (t3&TY_MASK!=TY_IMM)
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t3,r3,d3,ip);
    } else {
      if (t3&TY_MASK!=TY_IMM)
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t3,r3,d3,ip);
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,t2,r2,d2,ip);
    }
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,TY_DISP+ptype,CREG_RDX,0,ip);
    result_reg=CREG_RAX;
    ICU8(tempi,0x48);
    if (t3&TY_MASK==TY_IMM) {
      if (d3==1)
	ICU16(tempi,is.u16[2]+(result_reg&7)<<8);
      else {
	ICU16(tempi,is.u16[0]+(result_reg&7)<<8);
	ICU8(tempi,d3);
      }
    } else
      ICU16(tempi,is.u16[1]+(result_reg&7)<<8);
    ICMov(tempi,TY_DISP+ptype,CREG_RDX,0,TY_REG+IT_I64,result_reg,0,ip);
  }
  if (t1&TY_MASK)
    ICMov(tempi,t1,r1,d1,TY_REG+IT_I64,result_reg,0,ip);
}

U0 ICUnaries(CIntermediateCode *tempi,I64 op,I64 ip)
{
  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
    tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
  ICSlashOp(tempi,TY_REG+IT_I64,CREG_RAX,0,op,ip);
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
    TY_REG+IT_I64,CREG_RAX,0,ip);
}

U0 ICNot(CIntermediateCode *tempi,I64 ip)
{
  I64 i;
  if (tempi->p1.type&IT_MASK<=IT_U8 &&
    TY_DISP<=tempi->p1.type&TY_MASK<=TY_RIP_DISP32) {
    i=ICModr1(tempi,tempi->p1.type,tempi->p1.reg,tempi->p1.disp);
    ICRex(tempi,i.u8[1]);
    ICU16(tempi,i.u8[2]<<8+0xF6); //TEST ?,0xFF
    ICModr2(tempi,i,tempi->p1.disp,ip+5);
    ICU8(tempi,0xFF);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    ICTest(tempi,CREG_RAX);
  }
  ICU24(tempi,0xC0940F);   //SETZ AL
  ICU32(tempi,0xC0B60F48); //MOVZX RAX,AL
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
    TY_REG+IT_U64,CREG_RAX,0,ip);
}

U0 ICAndAnd(CIntermediateCode *tempi,I64 ip)
{
  I64 r2;
  if (tempi->ic_flags & ICF_P1_FIRST) {
    if (tempi->p1.type&TY_MASK!=TY_REG ||
	tempi->p1.reg==CREG_RAX) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      r2=CREG_RDX;
    } else
      r2=tempi->p1.reg;
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    if (tempi->p1.type&TY_MASK!=TY_REG ||
	tempi->p1.reg==CREG_RAX) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      r2=CREG_RDX;
    } else
      r2=tempi->p1.reg;
  }
  ICZero(tempi,CREG_RAX);
  ICTest(tempi,r2);
  ICU16(tempi,0x0874);
  ICTest(tempi,CREG_RCX);
  ICU16(tempi,0x0374);
  ICU24(tempi,0xC0FF48);
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+IT_I64,CREG_RAX,0,ip);
}

U0 ICOrOr(CIntermediateCode *tempi,I64 ip)
{
  I64 i=0x48,r2;
  if (tempi->ic_flags & ICF_P1_FIRST) {
    if (tempi->p1.type&TY_MASK!=TY_REG ||
	tempi->p1.reg==CREG_RAX) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      r2=CREG_RDX;
    } else
      r2=tempi->p1.reg;
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    if (tempi->p1.type&TY_MASK!=TY_REG ||
	tempi->p1.reg==CREG_RAX) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      r2=CREG_RDX;
    } else
      r2=tempi->p1.reg;
  }
 
  if (r2>7) {
    i++;
    r2&=7;
  }
  ICZero(tempi,CREG_RAX);
  ICU24(tempi,0xC80B00+i+r2<<16);
  ICU16(tempi,0x0374);
  ICU24(tempi,0xC0FF48);
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+IT_I64,CREG_RAX,0,ip);
}

U0 ICXorXor(CIntermediateCode *tempi,I64 ip)
{
  I64 r2;
  if (tempi->ic_flags & ICF_P1_FIRST) {
    if (tempi->p1.type&TY_MASK!=TY_REG) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      r2=CREG_RDX;
    } else
      r2=tempi->p1.reg;
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    if (tempi->p1.type&TY_MASK!=TY_REG) {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      r2=CREG_RDX;
    } else
      r2=tempi->p1.reg;
  }
  ICZero(tempi,CREG_RBX);
  ICTest(tempi,r2);
  ICU16(tempi,0x0374);
  ICU24(tempi,0xC3FF48);

  ICZero(tempi,CREG_RAX);
  ICTest(tempi,CREG_RCX);
  ICU16(tempi,0x0374);
  ICU24(tempi,0xC0FF48);
 
  ICU24(tempi,0xC33348);

  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+IT_I64,CREG_RAX,0,ip);
}

U0 ICCmp(CIntermediateCode *tempi,I64 us,I64 is,I64 ip)
{
  I64 r1,d1,r2,i=0x48,j=tempi->p2.disp;
  if (tempi->p2.type&TY_MASK==TY_IMM && MIN_I32<=j<=MAX_I32) {
    if (!(tempi->ic_flags&(ICF_POP_CMP|ICF_PUSH_CMP)) &&
	tempi->p1.type&TY_MASK==TY_DISP && //TODO
	tempi->p1.type&IT_MASK>=IT_I64 && tempi->p1.reg!=CREG_RAX) {
      r1=tempi->p1.reg;
      d1=tempi->p1.disp;
      ICZero(tempi,CREG_RAX);
      if (r1>7)
	i++;
      if (MIN_I8<=j<=MAX_I8)
	i+=0x388300;
      else
	i+=0x388100;
      if (!d1) {
	ICU24(tempi,0x000000+i+(r1&7)<<16);
      } else if (MIN_I8<=d1<=MAX_I8) {
	ICU24(tempi,0x400000+i+(r1&7)<<16);
	ICU8(tempi,d1);
      } else {
	ICU24(tempi,0x800000+i+(r1&7)<<16);
	ICU32(tempi,d1);
      }
      if (MIN_I8<=j<=MAX_I8)
	ICU8(tempi,j);
      else
	ICU32(tempi,j);
    } else {
      if (tempi->ic_flags & ICF_POP_CMP) {
	ICPopRegs(tempi,1<<CREG_RCX);
	r1=CREG_RCX;
      } else {
	if (tempi->p1.type&TY_MASK==TY_REG &&
	    tempi->p1.reg!=CREG_RAX)
	  r1=tempi->p1.reg;
	else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
	  r1=CREG_RCX;
	}
      }
      ICZero(tempi,CREG_RAX);
      if (r1>7)
	i++;
      if (MIN_I8<=j<=MAX_I8) {
	ICU24(tempi,0xF88300+i+(r1&7)<<16);
	ICU8(tempi,j);
      } else {
	ICU24(tempi,0xF88100+i+(r1&7)<<16);
	ICU32(tempi,j);
      }
    }
    if (tempi->ic_flags & ICF_PUSH_CMP)
      ICPush(tempi,TY_IMM+IT_I64,0,j,ip);
    if (tempi->ic_class->sub_type&1 || tempi->ic_flags & ICF_USE_UNSIGNED)
      is=us;
    ICU16(tempi,0x300+is);
    ICU24(tempi,0xC0FF48);
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
      TY_REG+IT_I64,CREG_RAX,0,ip);
  } else {
    if (tempi->ic_flags & ICF_P1_FIRST) {
      if (tempi->ic_flags & ICF_POP_CMP) {
	ICPopRegs(tempi,1<<CREG_RCX);
	r1=CREG_RCX;
      } else {
	if (tempi->p1.type&TY_MASK==TY_REG &&
	    tempi->p1.reg!=CREG_RAX)
	  r1=tempi->p1.reg;
	else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
	  r1=CREG_RCX;
	}
      }
      if (tempi->p2.type&TY_MASK==TY_REG &&
	  tempi->p2.reg!=CREG_RAX)
	r2=tempi->p2.reg;
      else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
	r2=CREG_RDX;
      }
    } else {
      if (tempi->p2.type&TY_MASK==TY_REG &&
	  tempi->p2.reg!=CREG_RAX)
	r2=tempi->p2.reg;
      else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
	r2=CREG_RCX;
      }
      if (tempi->ic_flags & ICF_POP_CMP) {
	ICPopRegs(tempi,1<<CREG_RDX);
	r1=CREG_RDX;
      } else {
	if (tempi->p1.type&TY_MASK==TY_REG &&
	    tempi->p1.reg!=CREG_RAX)
	  r1=tempi->p1.reg;
	else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
	  r1=CREG_RDX;
	}
      }
    }
    ICZero(tempi,CREG_RAX);
    if (r2>7)
      i++;
    if (r1>7)
      i+=4;
    if (tempi->ic_flags & ICF_PUSH_CMP)
      ICPushRegs(tempi,1<<r2);
    ICU24(tempi,0xC03B00+i+(r2&7)<<16+(r1&7)<<19);
    if (tempi->ic_class->sub_type&1 || tempi->ic_flags & ICF_USE_UNSIGNED)
      is=us;
    ICU16(tempi,0x300+is);
    ICU24(tempi,0xC0FF48);
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
      TY_REG+IT_I64,CREG_RAX,0,ip);
  }
}

U0 ICBitOps(CIntermediateCode *tempi,CICArg *p1,CICArg *p2,CIntermediateCode *tempi2,
	      I64 op,I64 op_imm,I64 ip)
{ //TODO:not fully utilizing Modr
  Bool no_depend=ToBool(tempi2->ic_flags&ICF_NO_DEPEND_RESULT);
  I64 r1,t2,r2,d2,i=0x48;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    t2=p2->type&TY_MASK+IT_I64; //TODO: check overflow
    r2=p2->reg;
    d2=p2->disp;
    if (tempi->ic_flags & ICF_P1_FIRST) {
      if (p1->type&TY_MASK==TY_REG &&
	  (p1->reg!=CREG_RAX||no_depend))
	r1=p1->reg;
      else if (p1->type&TY_MASK!=TY_IMM || p1->disp>63) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p1->type,p1->reg,p1->disp,ip);
	r1=CREG_RCX;
      } else
	r1=0;
      if (!(TY_REG<=t2&TY_MASK<=TY_RIP_DISP32) ||
	  !(r2.u8[0]!=CREG_RAX&&(t2&TY_MASK!=TY_SIB||r2.u8[1]&15!=CREG_RAX)||no_depend)) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,t2,r2,d2,ip);
	t2=TY_REG+IT_I64;
	r2=CREG_RDX;
	d2=0;
      }
    } else {
      if (!(TY_REG<=t2&TY_MASK<=TY_RIP_DISP32) ||
	  !(r2.u8[0]!=CREG_RAX&&(t2&TY_MASK!=TY_SIB||r2.u8[1]&15!=CREG_RAX)||no_depend)) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,t2,r2,d2,ip);
	t2=TY_REG+IT_I64;
	r2=CREG_RCX;
	d2=0;
      }
      if (p1->type&TY_MASK==TY_REG &&
	  (p1->reg!=CREG_RAX||no_depend))
	r1=p1->reg;
      else if (p1->type&TY_MASK!=TY_IMM || p1->disp>63) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,p1->type,p1->reg,p1->disp,ip);
	r1=CREG_RDX;
      } else
	r1=0;
    }
  } else {
    t2=TY_DISP+IT_I64;
    d2=0;
    if (tempi->ic_flags & ICF_P1_FIRST) {
      if (p1->type&TY_MASK==TY_REG &&
	  (p1->reg!=CREG_RAX||no_depend))
	r1=p1->reg;
      else if (p1->type&TY_MASK!=TY_IMM || p1->disp>63) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p1->type,p1->reg,p1->disp,ip);
	r1=CREG_RCX;
      } else
	r1=0;
      if (p2->type&TY_MASK==TY_REG &&
	  (p2->reg!=CREG_RAX||no_depend))
	r2=p2->reg;
      else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,p2->type,p2->reg,p2->disp,ip);
	r2=CREG_RDX;
      }
    } else {
      if (p2->type&TY_MASK==TY_REG &&
	  (p2->reg!=CREG_RAX||no_depend))
	r2=p2->reg;
      else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p2->type,p2->reg,p2->disp,ip);
	r2=CREG_RCX;
      }
      if (p1->type&TY_MASK==TY_REG &&
	  (p1->reg!=CREG_RAX||no_depend))
	r1=p1->reg;
      else if (p1->type&TY_MASK!=TY_IMM || p1->disp>63) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,p1->type,p1->reg,p1->disp,ip);
	r1=CREG_RDX;
      } else
	r1=0;
    }
  }
  if (!no_depend)
    ICZero(tempi,CREG_RAX);
  if (tempi->ic_flags&ICF_LOCK) {
    if (op!=0xA30F)
      ICU8(tempi,OC_LOCK_PREFIX);
  }
  if (p1->type&TY_MASK==TY_IMM && p1->disp<32)
    t2=t2&TY_MASK+IT_U32;
  i=ICModr1(r1,t2,r2,d2);
  ICRex(tempi,i.u8[1]);
  if (p1->type&TY_MASK==TY_IMM && p1->disp<64) {
    ICU24(tempi,i.u8[2]<<16+op_imm);
    ICModr2(tempi,i,d2,ip+5);
    ICU8(tempi,p1->disp);
  } else {
    ICU24(tempi,i.u8[2]<<16+op);
    ICModr2(tempi,i,d2,ip+4);
  }
  if (!no_depend) {
    ICU32(tempi,0xD08348);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+IT_I64,CREG_RAX,0,ip);
  }
}

U0 ICPreempt(CIntermediateCode *tempi,I64 ip)
{
  CIntermediateCode *tempi2=tempi->next;
  if (tempi->p1.type&TY_MASK==TY_IMM) {
    ICZero(tempi,CREG_RBX);
    ICU32(tempi,0x1B8B4864);
    if (!(tempi2->ic_flags&ICF_NO_DEPEND_RESULT))
      ICZero(tempi,CREG_RAX);
    if (tempi->p1.disp)
      ICU24(tempi,0x6BBA0F);
    else
      ICU24(tempi,0x73BA0F);
    ICU16(tempi,CTask.task_flags+TASKf_PREEMPT<<8);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    if (!(tempi2->ic_flags&ICF_NO_DEPEND_RESULT))
      ICZero(tempi,CREG_RAX);
    ICZero(tempi,CREG_RBX);
    ICU32(tempi,0x1B8B4864);
    ICTest(tempi,CREG_RCX);
    ICU16(tempi,0x0774);
    ICU24(tempi,0x6BBA0F);
    ICU16(tempi,CTask.task_flags+TASKf_PREEMPT<<8);
    ICU16(tempi,0x05EB);
    ICU24(tempi,0x73BA0F);
    ICU16(tempi,CTask.task_flags+TASKf_PREEMPT<<8);
  }
  if (!(tempi2->ic_flags&ICF_NO_DEPEND_RESULT))
    ICU32(tempi,0xD08348);
}

U0 ICToUpper(CIntermediateCode *tempi,I64 ip)
{
  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
    tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
  ICU32(tempi,0x61F88348);
  ICU16(tempi,0x0A7C);
  ICU32(tempi,0x7AF88348);
  ICU16(tempi,0x047F);
  ICU32(tempi,0xE0C08348);
}

U0 ICToI64(CLex *lx,CIntermediateCode *tempi,I64 ip)
{
  ICFCvt(lx,tempi,CREG_RAX,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,TRUE,CN_MAIN,ip);
}

U0 ICToF64(CLex *lx,CIntermediateCode *tempi,I64 ip)
{
  ICFCvt(lx,tempi,CREG_RAX,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,FALSE,CN_MAIN,ip);
}

U0 ICToBool(CLex *,CIntermediateCode *tempi,I64 ip)
{
  I64 r;
  if (tempi->p1.type&TY_MASK==TY_REG)
    r=tempi->p1.reg;
  else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
      tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    r=CREG_RAX;
  }
  ICTest(tempi,r);
  ICU24(tempi,0xC0950F);   //SETNZ AL
  ICU32(tempi,0xC0B60F48); //MOVZX RAX,AL
}

U0 ICFTemplateFun(CLex *lx,CIntermediateCode *tempi,I64 op,I64 ip)
{
  Bool dont_push_float,dont_pop_float;

  CmpSetFloatOpPushPop(lx,tempi,&dont_push_float,&dont_pop_float);
  if (!dont_push_float)
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
      tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);

  ICCopyTemplate(lx,tempi,op,FALSE,TRUE,TRUE,CN_MAIN);
  if (tempi->r.type&TY_MASK && !(tempi->ic_flags & ICF_R_TO_F64) &&
    !(tempi->ic_flags & ICF_R_TO_INT))
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
      TY_REG+IT_I64,CREG_RAX,0,ip);
}

U0 ICPreIncDec(CIntermediateCode *tempi,I64 op,I64 ip)
{
  I64 r;
  CHashClass *tempc=tempi->ic_class,*tempc1=tempc-1;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempc->ptr_cnt && tempc1->size!=1) {
      ICAddSubEctImm(tempi,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,
	tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,op,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
  } else {
    if (tempi->p1.type&TY_MASK==TY_REG)
      r=tempi->p1.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      r=CREG_RCX;
    }
    if (tempc->ptr_cnt &&
	tempc1->size!=1) {
      ICAddSubEctImm(tempi,TY_DISP+tempi->p1.ptype,r,0,
	TY_DISP+tempi->p1.ptype,r,0,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,
	TY_DISP+tempi->p1.ptype,r,0,op,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	TY_DISP+tempi->p1.ptype,r,0,ip);
  }
}


U0 ICPostIncDec(CIntermediateCode *tempi,I64 op,I64 ip)
{
  I64 r;
  CHashClass *tempc=tempi->ic_class,*tempc1=tempc-1;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempi->r.type&TY_MASK) {
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
    }
    if (tempc->ptr_cnt &&
	tempc1->size!=1) {
      ICAddSubEctImm(tempi,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,
	tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,op,ip);
  } else {
    if (tempi->p1.type&TY_MASK==TY_REG &&
	!(tempi->r.type&TY_MASK==TY_REG && tempi->r.reg==tempi->p1.reg))
      r=tempi->p1.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      r=CREG_RCX;
    }
    if (tempi->r.type&TY_MASK) {
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	TY_DISP+tempi->p1.ptype,r,0,ip);
    }
    if (tempc->ptr_cnt &&
	tempc1->size!=1) {
      ICAddSubEctImm(tempi,TY_DISP+tempi->p1.ptype,r,0,
	TY_DISP+tempi->p1.ptype,r,0,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,
	TY_DISP+tempi->p1.ptype,r,0,op,ip);
  }
}

U0 ICDerefPostIncDec(CIntermediateCode *tempi,I64 op,I64 ip)
{
  I64 t,r;
  CHashClass *tempc1=tempi->ic_class;
  t=tempi->r.type&IT_MASK;
  if (t>tempi->p1.ptype)
    t=tempi->p1.ptype;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempi->p1.type&TY_MASK==TY_REG)
      r=tempi->p1.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,tempi->p1.type&TY_MASK+IT_I64,tempi->p1.reg,tempi->p1.disp,ip);
      r=CREG_RDX;
    }
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
       TY_DISP+t,r,0,ip);
    if (tempc1->size!=1) {
      ICAddSubEctImm(tempi,tempi->p1.type&TY_MASK+IT_I64,tempi->p1.reg,tempi->p1.disp,
	tempi->p1.type&TY_MASK+IT_I64,tempi->p1.reg,tempi->p1.disp,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,
	tempi->p1.type&TY_MASK+IT_I64,tempi->p1.reg,tempi->p1.disp,op,ip);
  } else {
    if (tempi->p1.type&TY_MASK==TY_REG)
      r=tempi->p1.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,tempi->p1.type&TY_MASK+IT_I64,tempi->p1.reg,tempi->p1.disp,ip);
      r=CREG_RCX;
    }
    ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
      TY_DISP+IT_I64,r,0,ip);
    if (tempc1->size!=1) {
      ICAddSubEctImm(tempi,TY_DISP+IT_I64,r,0,
	TY_DISP+IT_I64,r,0,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,TY_DISP+IT_I64,r,0,op,ip);
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
       TY_DISP+t,CREG_RDX,0,ip);
  }
}

U0 ICAssignPostIncDec(CIntermediateCode *tempi,I64 op,I64 ip)
{
  CHashClass *tempc1=tempi->ic_class2-1;
  I64 r;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempi->p1.type&TY_MASK==TY_REG)
      r=tempi->p1.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,tempi->p1.type&TY_MASK+IT_I64,tempi->p1.reg,tempi->p1.disp,ip);
      r=CREG_RDX;
    }
    ICMov(tempi,TY_DISP+tempi->p1.ptype,r,0,
      tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    if (tempc1->size!=1 || tempi->p1.type&TY_MASK==TY_STK) {
      ICAddSubEctImm(tempi,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,
	TY_REG+IT_I64,r,0,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,op,ip);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,
	  TY_DISP+IT_I64,CREG_RDX,0,ip);
    ICMov(tempi,TY_DISP+tempi->p1.ptype,CREG_RCX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    if (tempc1->size!=1) {
      ICAddSubEctImm(tempi,TY_DISP+IT_I64,CREG_RDX,0,
	TY_REG+IT_I64,CREG_RCX,0,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,TY_DISP+IT_I64,CREG_RDX,0,op,ip);
  }
}


U0 ICCmpAndBranch(CIntermediateCode *tempi,Bool has_result,I64 ip,
   I64 us,I64 is,I64 not_us,I64 not_is,
   U8 *buf,I64 ip2)
{
  I64 r1,r2,i=0x48,j,t1,t2,result_reg;
  Bool short_jmp,done;
  CLexCodeMisc *lb;
  CICArg *p1=&tempi->p1,*p2=&tempi->p2,*swap;

  j=p1->disp;
  if (p1->type&TY_MASK==TY_IMM && MIN_I32<=j<=MAX_I32) {
    swap=p1; p1=p2; p2=swap;
    us=not_us;
    is=not_is;
  }
  if (tempi->ic_class->sub_type&1 || tempi->ic_flags & ICF_USE_UNSIGNED)
    is=us;

  j=p2->disp;
  if (p2->type&TY_MASK==TY_IMM && MIN_I32<=j<=MAX_I32) {
    if (!has_result && TY_REG<=p1->type&TY_MASK<=TY_RIP_DISP32)
      ICAddSubEctImm(tempi,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,
			   tempi->p1.type,tempi->p1.reg,tempi->p1.disp,
		     j,0x073B,ip2);
    else {
      if (p1->type&TY_MASK==TY_REG)
	r1=p1->reg;
      else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,p1->type,p1->reg,p1->disp,ip2);
	r1=CREG_RDX;
      }
      if (!j) {
	if (is.u8[2]==0x7C) {
	  ICTest(tempi,r1);
	  is=0x78880F;
	} else if (is.u8[2]==0x7D) {
	  ICTest(tempi,r1);
	  is=0x79890F;
	} else if (is.u8[2]==0x74 || is.u8[2]==0x75)
	  ICTest(tempi,r1);
	else {
	  if (r1>7)
	    i++;
	  ICU24(tempi,0xF88300+i+(r1&7)<<16);
	  ICU8(tempi,j);
	}
      } else {
	if (r1>7)
	  i++;
	if (MIN_I8<=j<=MAX_I8) {
	  ICU24(tempi,0xF88300+i+(r1&7)<<16);
	  ICU8(tempi,j);
	} else {
	  ICU24(tempi,0xF88100+i+(r1&7)<<16);
	  ICU32(tempi,j);
	}
      }
    }
    if (has_result) {
      if (!swap) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,TY_IMM+IT_I64,0,j,ip2);
	result_reg=CREG_RCX;
      } else
	result_reg=r1;
    }
  } else {
    done=FALSE;
    t1=p1->type;
    r1=p1->reg;
    r2=p2->reg;
    t2=p2->type;
    if (t2&IT_MASK>=IT_I64 && !has_result &&
	  TY_DISP<=t2&TY_MASK<=TY_RIP_DISP32) {
      if (t1&TY_MASK!=TY_REG || t1&IT_MASK<IT_I64) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,p1->type,p1->reg,p1->disp,ip2);
	r1=CREG_RAX;
      }
      i=ICModr1(r1,t2,r2,p2->disp);
      ICRex(tempi,i.u8[1]);
      ICU16(tempi,i.u8[2]<<8+0x3B);
      ICModr2(tempi,i,p2->disp,ip2+4);
      done=TRUE;
    } else if (t1&IT_MASK>=IT_I64 &&
	  TY_REG<=t1&TY_MASK<=TY_RIP_DISP32) {
      if (t2&TY_MASK!=TY_REG || t2&IT_MASK<IT_I64) {
	if (t1&TY_MASK==TY_REG && r1==CREG_RAX) {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p2->type,p2->reg,p2->disp,ip2);
	  r2=CREG_RCX;
	} else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,p2->type,p2->reg,p2->disp,ip2);
	  r2=CREG_RAX;
	}
      }
      i=ICModr1(r2,t1,r1,p1->disp);
      ICRex(tempi,i.u8[1]);
      ICU16(tempi,i.u8[2]<<8+0x39);
      ICModr2(tempi,i,p1->disp,ip2+4);
      if (has_result)
	result_reg=r2;
      done=TRUE;
    }
    if (!done) {
      if (tempi->ic_flags & ICF_P1_FIRST) {
	if (p1->type&TY_MASK==TY_REG)
	  r1=p1->reg;
	else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,p1->type,p1->reg,p1->disp,ip2);
	  r1=CREG_RAX;
	}
	if (p2->type&TY_MASK==TY_REG)
	  r2=p2->reg;
	else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p2->type,p2->reg,p2->disp,ip2);
	  r2=CREG_RCX;
	}
      } else {
	if (p2->type&TY_MASK==TY_REG)
	  r2=p2->reg;
	else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,p2->type,p2->reg,p2->disp,ip2);
	  r2=CREG_RAX;
	}
	if (p1->type&TY_MASK==TY_REG)
	  r1=p1->reg;
	else {
	  ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p1->type,p1->reg,p1->disp,ip2);
	  r1=CREG_RCX;
	}
      }
      if (r2>7)
	i++;
      if (r1>7)
	i+=4;
      ICU24(tempi,0xC03B00+i+(r2&7)<<16+(r1&7)<<19);
      if (has_result)
	result_reg=r2;
    }
  }

  ip+=tempi->ic_cnt;
  lb=OptLabelFwd(tempi->ic_data);
  short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP) {
      if(tempi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tempi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP) {
      if(tempi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    ICU16(tempi,i<<8+is.u8[2]);
  } else {
    tempi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP) {
      if(tempi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    ICU16(tempi,is.u16[0]);
    ICU32(tempi,i);
  }
  if (has_result)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+IT_I64,result_reg,0,ip2);
}

U0 ICTestAndBranch(CIntermediateCode *tempi,I64 ip,I64 is,U8 *buf,I64 ip2)
{
  I64 i;
  Bool short_jmp;
  CLexCodeMisc *lb;

  if (tempi->p1.type&TY_MASK!=TY_REG) {
    if (tempi->p1.type&IT_MASK<=IT_U8 &&
	  TY_DISP<=tempi->p1.type&TY_MASK<=TY_RIP_DISP32) {
      i=ICModr1(tempi,tempi->p1.type,tempi->p1.reg,tempi->p1.disp);
      ICRex(tempi,i.u8[1]);
      ICU16(tempi,i.u8[2]<<8+0xF6);
      ICModr2(tempi,i,tempi->p1.disp,ip2+5);
      ICU8(tempi,0xFF);
    } else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
      ICTest(tempi,CREG_RAX);
    }
  } else
    ICTest(tempi,tempi->p1.reg);

  ip+=tempi->ic_cnt;
  lb=OptLabelFwd(tempi->ic_data);
  short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tempi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,i<<8+is.u8[2]);
  } else {
    tempi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,is.u16[0]);
    ICU32(tempi,i);
  }
}

U0 ICZFlagBranch(CIntermediateCode *tempi,I64 ip,I64 is,U8 *buf)
{
  I64 i;
  Bool short_jmp;
  CLexCodeMisc *lb;

  ip+=tempi->ic_cnt;
  lb=OptLabelFwd(tempi->ic_data);
  short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tempi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,i<<8+is.u8[2]);
  } else {
    tempi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,is.u16[0]);
    ICU32(tempi,i);
  }
}

U0 ICAndBranch(CIntermediateCode *tempi,I64 ip,I64 is,U8 *buf,I64 ip2)
{
  U64 i;
  I64 it,t1,r1,d1,r2;
  Bool short_jmp,swap,override;
  CLexCodeMisc *lb;
  CICArg *p1,*p2;

  if (tempi->p1.type&TY_MASK==TY_IMM) {
    swap=TRUE;
    p1=&tempi->p2;
    p2=&tempi->p1;
  } else {
    swap=FALSE;
    p1=&tempi->p1;
    p2=&tempi->p2;
  }

  if (p2->type&TY_MASK==TY_IMM && p2->disp>MAX_U32)
    override=TRUE;
  else
    override=FALSE;

  if (p1->type&IT_MASK<p2->type&IT_MASK)
    it=p1->type&IT_MASK;
  else
    it=p2->type&IT_MASK;

  i=p2->disp;
  if (p2->type&TY_MASK==TY_IMM && i<=MAX_U32) {
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,p1->type,p1->reg,p1->disp,ip2);
    if (i<=MAX_U8)
      ICU16(tempi,i<<8+0xA8);
    else if (i<=MAX_U16)
      ICU32(tempi,i<<16+0xA900+OC_OP_SIZE_PREFIX);
    else {
      ICU8(tempi,0xA9);
      ICU32(tempi,i);
    }
  } else {
    t1=TY_REG+it;
    d1=0;
    if ((tempi->ic_flags&ICF_P1_FIRST ^^swap)&&!override) {
      if (p1->type&TY_MASK==TY_REG) {
	r1=p1->reg;
	swap=TRUE;
      } else {
	r1=CREG_RCX;
	swap=FALSE;
      }
      if (p2->type&TY_MASK==TY_REG) {
	r2=p2->reg;
	swap=FALSE;
      } else
	r2=CREG_RDX;
      if (swap) {
	if (p1->type&TY_MASK!=TY_REG || r1!=p1->reg)
	  ICMov(tempi,TY_REG+IT_I64,r1,0,p1->type,p1->reg,p1->disp,ip2);
	if (TY_REG<=p2->type&TY_MASK<=TY_RIP_DISP32) {
	  t1=p2->type&TY_MASK+it;
	  r2=p2->reg;
	  d1=p2->disp;
	} else
	  ICMov(tempi,TY_REG+IT_I64,r2,0,p2->type,p2->reg,p2->disp,ip2);
	i=ICModr1(r1,t1,r2,d1);
      } else {
	if (TY_REG<=p1->type&TY_MASK<=TY_RIP_DISP32) {
	  t1=p1->type&TY_MASK+it;
	  r1=p1->reg;
	  d1=p1->disp;
	} else
	  ICMov(tempi,TY_REG+IT_I64,r1,0,p1->type,p1->reg,p1->disp,ip2);
	if (p2->type&TY_MASK!=TY_REG || r2!=p2->reg)
	  ICMov(tempi,TY_REG+IT_I64,r2,0,p2->type,p2->reg,p2->disp,ip2);
	i=ICModr1(r2,t1,r1,d1);
      }
    } else {
      if (p2->type&TY_MASK==TY_REG) {
	r2=p2->reg;
	swap=FALSE;
      } else {
	r2=CREG_RDX;
	swap=TRUE;
      }
      if (p1->type&TY_MASK==TY_REG) {
	r1=p1->reg;
	swap=TRUE;
      } else
	r1=CREG_RCX;
      if (override)
	swap=FALSE;
      if (swap) {
	if (TY_REG<=p2->type&TY_MASK<=TY_RIP_DISP32) {
	  t1=p2->type&TY_MASK+it;
	  r2=p2->reg;
	  d1=p2->disp;
	} else
	  ICMov(tempi,TY_REG+IT_I64,r2,0,p2->type,p2->reg,p2->disp,ip2);
	if (p1->type&TY_MASK!=TY_REG || r1!=p1->reg)
	  ICMov(tempi,TY_REG+IT_I64,r1,0,p1->type,p1->reg,p1->disp,ip2);
	i=ICModr1(r1,t1,r2,d1);
      } else {
	if (p2->type&TY_MASK!=TY_REG || r2!=p2->reg)
	  ICMov(tempi,TY_REG+IT_I64,r2,0,p2->type,p2->reg,p2->disp,ip2);
	if (TY_REG<=p1->type&TY_MASK<=TY_RIP_DISP32) {
	  t1=p1->type&TY_MASK+it;
	  r1=p1->reg;
	  d1=p1->disp;
	} else
	  ICMov(tempi,TY_REG+IT_I64,r1,0,p1->type,p1->reg,p1->disp,ip2);
	i=ICModr1(r2,t1,r1,d1);
      }
    }
    switch (it) {
      case IT_I8:
      case IT_U8:
	ICRex(tempi,i.u8[1]);
	ICU16(tempi,i.u8[2]<<8+0x84);
	break;
      case IT_U16:
      case IT_I16:
	ICOpSizeRex(tempi,i.u8[1]);
	ICU16(tempi,i.u8[2]<<8+0x85);
	break;
      default:
	ICRex(tempi,i.u8[1]);
	ICU16(tempi,i.u8[2]<<8+0x85);
    }
    ICModr2(tempi,i,d1,ip2+4);
  }

  ip+=tempi->ic_cnt;
  lb=OptLabelFwd(tempi->ic_data);
  short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tempi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,i<<8+is.u8[2]);
  } else {
    tempi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,is.u16[0]);
    ICU32(tempi,i);
  }
}

U0 ICAssign(CIntermediateCode *tempi,I64 ip)
{
  CIntermediateCode *tempi1;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    ICMov(tempi,tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    if (tempi->r.type&TY_MASK) {
      if (tempi->p1.type&TY_MASK==TY_REG)
	ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->p1.type&TY_MASK+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,ip);
      else
	ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    }
  } else {
    if (tempi->p1.type&TY_MASK==TY_REG) {
      if (!(tempi1=OptLag1(tempi)) ||
	  tempi1->ic_opcode!=IC_ADD_CONST ||
	  tempi1->r.type!=TY_REG+IT_I64 || tempi1->r.reg!=tempi->p1.reg ||
	  tempi1->p1.type!=TY_REG+IT_I64 || tempi1->p1.reg!=tempi->p1.reg ||
	  (tempi->p2.type&TY_MASK==TY_REG||tempi->p2.type&TY_MASK==TY_DISP) &&
	  tempi->p2.reg==tempi->p1.reg ||
	  tempi->r.type&TY_MASK ||
	  tempi1->ic_flags&~ICG_NO_CVT_MASK) {
	ICMov(tempi,TY_DISP+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	      tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
	if (tempi->r.type&TY_MASK)
	  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      } else {
	tempi->ic_flags=(tempi->ic_flags|tempi1->ic_flags)&~ICF_CODE_FINAL|ICF_DONT_RESTORE;
	tempi->p1.disp=tempi1->ic_data;
	OptSetNOP1(tempi1); //This better not be last pass!
	ICMov(tempi,TY_DISP+tempi->p1.ptype,tempi->p1.reg,tempi->p1.disp,
	      tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      }
    } else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,
	    tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip);
      ICMov(tempi,TY_DISP+tempi->p1.ptype,CREG_RCX,0,
	    tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
      if (tempi->r.type&TY_MASK)
	ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip);
    }
  }
}


U0 ICBrBitOps(CIntermediateCode *tempi,I64 ip,I64 op,I64 op_imm,I64 is,U8 *buf,I64 ip2)
{
  I64 i,t,r1,r2,d1,d2,t1,t2;
  CICArg *p1=&tempi->p1,*p2=&tempi->p2;
  Bool short_jmp;
  CLexCodeMisc *lb;

  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempi->ic_flags & ICF_P1_FIRST) {
      if (!(tempi->ic_flags&ICF_SWAP) && p1->type&TY_MASK!=TY_REG && (p1->type&TY_MASK!=TY_IMM || p1->disp>63) ||
	    tempi->ic_flags&ICF_SWAP && p1->type&TY_MASK==TY_IMM && p1->disp<64 ||
	    p1->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p1->type,p1->reg,p1->disp,ip2);
	t1=TY_REG+IT_I64;
	r1=CREG_RCX;
	d1=0;
      } else {
	t1=p1->type;
	if (t1&TY_MASK==TY_IMM && p1->disp<64)
	  r1=0;
	else
	  r1=p1->reg;
	d1=p1->disp;
      }
      if (tempi->ic_flags&ICF_SWAP && p2->type&TY_MASK!=TY_REG && (p2->type&TY_MASK!=TY_IMM || p2->disp>63) ||
	    !(tempi->ic_flags&ICF_SWAP) && p2->type&TY_MASK==TY_IMM && p2->disp<64 ||
	    p2->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,p2->type,p2->reg,p2->disp,ip2);
	t2=TY_REG+IT_I64;
	r2=CREG_RDX;
	d2=0;
      } else {
	t2=p2->type;
	if (t2&TY_MASK==TY_IMM && p2->disp<64)
	  r2=0;
	else
	  r2=p2->reg;
	d2=p2->disp;
      }
    } else {
      if (tempi->ic_flags&ICF_SWAP && p2->type&TY_MASK!=TY_REG && (p2->type&TY_MASK!=TY_IMM ||p2->disp>63)||
	    !(tempi->ic_flags&ICF_SWAP) && p2->type&TY_MASK==TY_IMM && p2->disp<64 ||
	    p2->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p2->type,p2->reg,p2->disp,ip2);
	t2=TY_REG+IT_I64;
	r2=CREG_RCX;
	d2=0;
      } else {
	t2=p2->type;
	if (t2&TY_MASK==TY_IMM && p2->disp<64)
	  r2=0;
	else
	  r2=p2->reg;
	d2=p2->disp;
      }
      if (!(tempi->ic_flags&ICF_SWAP) && p1->type&TY_MASK!=TY_REG && (p1->type&TY_MASK!=TY_IMM || p1->disp>63) ||
	    tempi->ic_flags&ICF_SWAP && p1->type&TY_MASK==TY_IMM && p1->disp<64||
	    p1->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,p1->type,p1->reg,p1->disp,ip2);
	t1=TY_REG+IT_I64;
	r1=CREG_RDX;
	d1=0;
      } else {
	t1=p1->type;
	if (t1&TY_MASK==TY_IMM && p1->disp<64)
	  r1=0;
	else
	  r1=p1->reg;
	d1=p1->disp;
      }
    }
  } else {
    t1=TY_DISP+IT_I64;
    t2=TY_DISP+IT_I64;
    d1=0;
    d2=0;
    if (tempi->ic_flags & ICF_P1_FIRST) {
      if (p1->type&TY_MASK==TY_REG)
	r1=p1->reg;
      else if (tempi->ic_flags&ICF_SWAP || p1->type&TY_MASK!=TY_IMM || p1->disp>63) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p1->type,p1->reg,p1->disp,ip2);
	r1=CREG_RCX;
      } else
	r1=0;
      if (p2->type&TY_MASK==TY_REG)
	r2=p2->reg;
      else if (!(tempi->ic_flags&ICF_SWAP) || p2->type&TY_MASK!=TY_IMM || p2->disp>63) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,p2->type,p2->reg,p2->disp,ip2);
	r2=CREG_RDX;
      } else
	r2=0;
    } else {
      if (p2->type&TY_MASK==TY_REG)
	r2=p2->reg;
      else if (!(tempi->ic_flags&ICF_SWAP) || p2->type&TY_MASK!=TY_IMM || p2->disp>63) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,p2->type,p2->reg,p2->disp,ip2);
	r2=CREG_RCX;
      } else
	r2=0;
      if (p1->type&TY_MASK==TY_REG)
	r1=p1->reg;
      else if (tempi->ic_flags&ICF_SWAP || p1->type&TY_MASK!=TY_IMM || p1->disp>63) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,p1->type,p1->reg,p1->disp,ip2);
	r1=CREG_RDX;
      } else
	r1=0;
    }
  }
  if (tempi->ic_flags&ICF_LOCK) {
    if (op!=0xA30F)
      ICU8(tempi,OC_LOCK_PREFIX);
  }
  if (tempi->ic_flags&ICF_SWAP) {
    if (p2->type&TY_MASK==TY_IMM && p2->disp<32)
      t=t1&TY_MASK+IT_U32;
    else
      t=t1;
    i=ICModr1(r2,t,r1,d1);
    SwapU64(&p1,&p2);
  } else {
    if (p1->type&TY_MASK==TY_IMM && p1->disp<32)
      t=t2&TY_MASK+IT_U32;
    else
      t=t2;
    i=ICModr1(r1,t,r2,d2);
  }
  ICRex(tempi,i.u8[1]);
  if (p1->type&TY_MASK==TY_IMM && p1->disp<64) {
    ICU24(tempi,i.u8[2]<<16+op_imm);
    ICModr2(tempi,i,p2->disp,ip2+5);
    ICU8(tempi,p1->disp);
  } else {
    ICU24(tempi,i.u8[2]<<16+op);
    ICModr2(tempi,i,p2->disp,ip2+4);
  }

  ip+=tempi->ic_cnt;
  lb=OptLabelFwd(tempi->ic_data);
  short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tempi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,i<<8+is.u8[2]);
  } else {
    tempi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,is.u16[0]);
    ICU32(tempi,i);
  }
}

U0 ICQueInit(CIntermediateCode *tempi,I64 ip2)
{
  I64 r1;
  if (tempi->p1.type==TY_REG+IT_I64)
    r1=tempi->p1.reg;
  else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
      tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
    r1=CREG_RAX;
  }
  ICMov(tempi,TY_DISP+IT_I64,r1,0,TY_REG+IT_I64,r1,0,ip2);
  ICMov(tempi,TY_DISP+IT_I64,r1,sizeof(U8 *),TY_REG+IT_I64,r1,0,ip2);
}

U0 ICQueIns(CIntermediateCode *tempi,I64 ip2)
{
  I64 r1,r2;
  if (tempi->ic_flags & ICF_P1_FIRST) {
    if (tempi->p1.type==TY_REG+IT_I64 && tempi->p1.reg!=CREG_RAX)
      r1=tempi->p1.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
      r1=CREG_RDX;
    }
    if (tempi->p2.type==TY_REG+IT_I64)
      r2=tempi->p2.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
      r2=CREG_RAX;
    }
  } else {
    if (tempi->p2.type==TY_REG+IT_I64 && tempi->p2.reg!=CREG_RDX)
      r2=tempi->p2.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
      r2=CREG_RAX;
    }
    if (tempi->p1.type==TY_REG+IT_I64)
      r1=tempi->p1.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
      r1=CREG_RDX;
    }
  }
  ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,TY_DISP+IT_I64,r1,0,ip2);
  ICMov(tempi,TY_DISP+IT_I64,r1,0,TY_REG+IT_I64,r2,0,ip2);
  ICMov(tempi,TY_DISP+IT_I64,r2,0,TY_REG+IT_I64,CREG_RBX,0,ip2);
  ICMov(tempi,TY_DISP+IT_I64,r2,sizeof(U8 *),TY_REG+IT_I64,r1,0,ip2);
  ICMov(tempi,TY_DISP+IT_I64,CREG_RBX,sizeof(U8 *),TY_REG+IT_I64,r2,0,ip2);
}

U0 ICQueInsRev(CIntermediateCode *tempi,I64 ip2)
{
  I64 r1,r2;
  if (tempi->ic_flags & ICF_P1_FIRST) {
    if (tempi->p1.type==TY_REG+IT_I64 && tempi->p1.reg!=CREG_RAX)
      r1=tempi->p1.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
      r1=CREG_RDX;
    }
    if (tempi->p2.type==TY_REG+IT_I64)
      r2=tempi->p2.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
      r2=CREG_RAX;
    }
  } else {
    if (tempi->p2.type==TY_REG+IT_I64 && tempi->p2.reg!=CREG_RDX)
      r2=tempi->p2.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
      r2=CREG_RAX;
    }
    if (tempi->p1.type==TY_REG+IT_I64)
      r1=tempi->p1.reg;
    else {
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
      r1=CREG_RDX;
    }
  }
  ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,TY_DISP+IT_I64,r1,sizeof(U8 *),ip2);
  ICMov(tempi,TY_DISP+IT_I64,CREG_RBX,0,TY_REG+IT_I64,r2,0,ip2);
  ICMov(tempi,TY_DISP+IT_I64,r2,0,TY_REG+IT_I64,r1,0,ip2);
  ICMov(tempi,TY_DISP+IT_I64,r2,sizeof(U8 *),TY_REG+IT_I64,CREG_RBX,0,ip2);
  ICMov(tempi,TY_DISP+IT_I64,r1,sizeof(U8 *),TY_REG+IT_I64,r2,0,ip2);
}

U0 ICQueRem(CIntermediateCode *tempi,I64 ip2)
{
  I64 r1;
  if (tempi->p1.type==TY_REG+IT_I64)
    r1=tempi->p1.reg;
  else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
      tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
    r1=CREG_RAX;
  }
  ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,TY_DISP+IT_I64,r1,0,ip2);
  ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,TY_DISP+IT_I64,r1,sizeof(U8 *),ip2);
  ICU24(tempi,0x1A8948);
  ICU32(tempi,sizeof(U8 *)<<24+0x538948);
}

U0 ICMinMax(CIntermediateCode *tempi,I64 op,I64 ip2)
{
  I64 r1,i1=0x48;
  if (tempi->ic_flags & ICF_P1_FIRST) {
    if (tempi->p1.type==TY_REG+IT_I64 && tempi->p1.reg!=CREG_RAX) {
      r1=tempi->p1.reg;
      if (r1>7) {
	i1++;
	r1&=7;
      }
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
    } else {
      if (tempi->p2.reg==CREG_RAX && TY_REG<=tempi->p2.type&TY_MASK<=TY_SIB) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
	r1=CREG_RDX;
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
	if (tempi->p2.type==TY_REG+IT_I64) {
	  r1=tempi->p2.reg;
	  if (r1>7) {
	    i1++;
	    r1&=7;
	  }
	} else {
	  r1=CREG_RDX;
	  ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	    tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
	}
      }
    }
  } else {
    if (tempi->p2.type==TY_REG+IT_I64 && tempi->p2.reg!=CREG_RAX) {
      r1=tempi->p2.reg;
      if (r1>7) {
	i1++;
	r1&=7;
      }
      ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
    } else {
      if (tempi->p1.reg==CREG_RAX && TY_REG<=tempi->p1.type&TY_MASK<=TY_SIB) {
	ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
	r1=CREG_RDX;
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	  tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
	if (tempi->p1.type==TY_REG+IT_I64) {
	  r1=tempi->p1.reg;
	  if (r1>7) {
	    i1++;
	    r1&=7;
	  }
	} else {
	  r1=CREG_RDX;
	  ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,
	    tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
	}
      }
    }
  }
  ICU24(tempi,0xC03B00+r1<<16+i1);
  ICU32(tempi,0xC0000F00+op<<16+r1<<24+i1);
}

U0 ICSqr(CIntermediateCode *tempi,I64 op,I64 ip2)
{
  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
  ICSlashOp(tempi,TY_REG+IT_I64,CREG_RAX,0,op,ip2);
}

U0 ICModU64(CIntermediateCode *tempi,I64 ip2)
{
  I64 t1,r1,d1;
  if (tempi->ic_flags & ICF_P1_FIRST) {
    if (tempi->p1.reg!=CREG_RAX && tempi->p1.reg!=CREG_RDX && TY_REG<=tempi->p1.type&TY_MASK<=TY_SIB &&
	tempi->p1.type&IT_MASK>=IT_I64) {
      t1=tempi->p1.type;
      r1=tempi->p1.reg;
      d1=tempi->p1.disp;
    } else {
      t1=TY_REG+IT_I64;
      r1=CREG_RBX;
      d1=0;
      ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
    }
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
  } else {
    ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
    if (tempi->p1.reg!=CREG_RAX && tempi->p1.reg!=CREG_RDX && TY_REG<=tempi->p1.type&TY_MASK<=TY_SIB &&
	tempi->p1.type&IT_MASK>=IT_I64) {
      t1=tempi->p1.type;
      r1=tempi->p1.reg;
      d1=tempi->p1.disp;
    } else {
      t1=TY_REG+IT_I64;
      r1=CREG_RBX;
      d1=0;
      ICMov(tempi,TY_REG+IT_I64,CREG_RBX,0,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
    }
  }
  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,TY_DISP+IT_I64,CREG_RCX,0,ip2);
  ICZero(tempi,CREG_RDX);
  ICSlashOp(tempi,t1,r1,d1,SLASH_OP_DIV,ip2);
  ICMov(tempi,TY_DISP+IT_I64,CREG_RCX,0,TY_REG+IT_I64,CREG_RAX,0,ip2);
  ICMov(tempi,TY_REG+IT_I64,CREG_RAX,0,
    TY_REG+IT_I64,CREG_RDX,0,ip2);
}

U0 ICSwap(CIntermediateCode *tempi,I64 ip2)
{
  I64 r1,r2;
  if (tempi->p1.type&TY_MASK==TY_REG)
    r1=tempi->p1.reg;
  else
    r1=CREG_RAX;
  if (tempi->p2.type&TY_MASK==TY_REG)
    r2=tempi->p2.reg;
  else
    r2=CREG_RAX;
  if (tempi->ic_flags & ICF_P1_FIRST) {
    if (r1==r2) {
      if (r2==CREG_RAX)
	r2=CREG_RBX;
      else
	r1=CREG_RAX;
    }
    ICMov(tempi,TY_REG+IT_I64,r1,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
    ICMov(tempi,TY_REG+IT_I64,r2,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
  } else {
    if (r1==r2) {
      if (r1==CREG_RAX)
	r1=CREG_RBX;
      else
	r2=CREG_RAX;
    }
    ICMov(tempi,TY_REG+IT_I64,r2,0,tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
    ICMov(tempi,TY_REG+IT_I64,r1,0,tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
  }
  switch (tempi->ic_opcode) {
    case IC_SWAP_U8:
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,TY_DISP+IT_U8,r1,0,ip2);
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,TY_DISP+IT_U8,r2,0,ip2);
      ICMov(tempi,TY_DISP+IT_U8,r2,0,TY_REG+IT_I64,CREG_RCX,0,ip2);
      ICMov(tempi,TY_DISP+IT_U8,r1,0,TY_REG+IT_I64,CREG_RDX,0,ip2);
      break;
    case IC_SWAP_U16:
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,TY_DISP+IT_U16,r1,0,ip2);
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,TY_DISP+IT_U16,r2,0,ip2);
      ICMov(tempi,TY_DISP+IT_U16,r2,0,TY_REG+IT_I64,CREG_RCX,0,ip2);
      ICMov(tempi,TY_DISP+IT_U16,r1,0,TY_REG+IT_I64,CREG_RDX,0,ip2);
      break;
    case IC_SWAP_U32:
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,TY_DISP+IT_U32,r1,0,ip2);
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,TY_DISP+IT_U32,r2,0,ip2);
      ICMov(tempi,TY_DISP+IT_U32,r2,0,TY_REG+IT_I64,CREG_RCX,0,ip2);
      ICMov(tempi,TY_DISP+IT_U32,r1,0,TY_REG+IT_I64,CREG_RDX,0,ip2);
      break;
    case IC_SWAP_U64:
      ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,TY_DISP+IT_U64,r1,0,ip2);
      ICMov(tempi,TY_REG+IT_I64,CREG_RDX,0,TY_DISP+IT_U64,r2,0,ip2);
      ICMov(tempi,TY_DISP+IT_U64,r2,0,TY_REG+IT_I64,CREG_RCX,0,ip2);
      ICMov(tempi,TY_DISP+IT_U64,r1,0,TY_REG+IT_I64,CREG_RDX,0,ip2);
      break;
  }
}

U0 ICAndEqual(CIntermediateCode *tempi,I64 ip2)
{
  I64 i,bit;
  if (tempi->p2.type&TY_MASK==TY_IMM &&
      tempi->p1.type&TY_MASK!=TY_STK &&
      tempi->ic_flags&ICF_NO_DEPEND_RESULT) {
    i=~tempi->p2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i)) {
      tempi->p2.disp=bit;
      tempi->p2.reg=0;
      tempi->p1.type=tempi->p1.type&TY_MASK+IT_I64;
      ICBitOps(tempi,&tempi->p2,&tempi->p1,tempi,
	0xB30F,0x30BA0F,ip2);
      return;
    }
  }
  ICAddSubEctEqual(tempi,tempi->p1.ptype,
	tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,0x210425240423,ip2);
}

U0 ICOrEqual(CIntermediateCode *tempi,I64 ip2)
{
  I64 i,bit;
  if (tempi->p2.type&TY_MASK==TY_IMM &&
      tempi->p1.type&TY_MASK!=TY_STK &&
      tempi->ic_flags&ICF_NO_DEPEND_RESULT) {
    i=tempi->p2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i) && i>MAX_I8) {
      tempi->p2.disp=bit;
      tempi->p2.reg=0;
      tempi->p1.type=tempi->p1.type&TY_MASK+IT_I64;
      ICBitOps(tempi,&tempi->p2,&tempi->p1,tempi,
	0xAB0F,0x28BA0F,ip2);
      return;
    }
  }
  ICAddSubEctEqual(tempi,tempi->p1.ptype,
	tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,0x09010D0C010B,ip2);
}

U0 ICXorEqual(CIntermediateCode *tempi,I64 ip2)
{
  I64 i,bit;
  if (tempi->p2.type&TY_MASK==TY_IMM &&
      tempi->p1.type&TY_MASK!=TY_STK &&
      tempi->ic_flags&ICF_NO_DEPEND_RESULT) {
    i=tempi->p2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i)) {
      tempi->p2.disp=bit;
      tempi->p2.reg=0;
      tempi->p1.type=tempi->p1.type&TY_MASK+IT_I64;
      ICBitOps(tempi,&tempi->p2,&tempi->p1,tempi,
	0xBB0F,0x38BA0F,ip2);
      return;
    }
  }
  ICAddSubEctEqual(tempi,tempi->p1.ptype,
	tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->p1.type,tempi->p1.reg,tempi->p1.disp,
	tempi->p2.type,tempi->p2.reg,tempi->p2.disp,0x310635340633,ip2);
}

U0 ICSwitch(CIntermediateCode *tempi,I64 ip,Bool fast,CLex *lx,U8 *buf,I64 ip2)
{
  I64 i,j,r;
  CLexCodeMisc *lb;
  Bool short_jmp;
  CExeAbsAddr *tempa;
  if (tempi->p1.type&TY_MASK==TY_REG)
    r=tempi->p1.reg;
  else
    r=CREG_RDX;
  if (fast)
    ICMov(tempi,TY_REG+IT_I64,r,0,
	  tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
  else {
    if (tempi->p2.type&TY_MASK==TY_IMM) {
      j=tempi->p2.disp;
      ICMov(tempi,TY_REG+IT_I64,r,0,
	    tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
      if (MIN_I8<=j<=MAX_I8) {
	i=0xF88348+(r&7)<<16;
	if (r>7) i++;
	ICU24(tempi,i);
	ICU8(tempi,j);
      } else if (MIN_I32<=j<=MAX_I32) {
	i=0xF88148+(r&7)<<16;
	if (r>7) i++;
	ICU24(tempi,i);
	ICU32(tempi,j);
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,
	      tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
	i=0xC13B48+(r&7)<<19;
	if (r>7) i+=4;
	ICU24(tempi,i);
      }
    } else {
      if (tempi->ic_flags & ICF_P1_FIRST) {
	ICMov(tempi,TY_REG+IT_I64,r,0,
	      tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,
	      tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
      } else {
	ICMov(tempi,TY_REG+IT_I64,CREG_RCX,0,
	      tempi->p2.type,tempi->p2.reg,tempi->p2.disp,ip2);
	ICMov(tempi,TY_REG+IT_I64,r,0,
	      tempi->p1.type,tempi->p1.reg,tempi->p1.disp,ip2);
      }
      i=0xC13B48+(r&7)<<19;
      if (r>7) i+=4;
      ICU24(tempi,i);
    }

    ip+=tempi->ic_cnt;
    lb=OptLabelFwd(tempi->ic_data(CLexCodeMisc *)->dft);
    short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
    if (!buf && lb->addr!=INVALID_PTR) {
      i=lb->addr-(ip+2);
      if (MIN_I8<=i<=MAX_I8)
	short_jmp=TRUE;
    }

    if (short_jmp) {
      tempi->ic_flags|=ICF_SHORT_JMP;
      i=lb->addr-(ip+2);
      ICU16(tempi,i<<8+0x73);
    } else {
      tempi->ic_flags&=~ICF_SHORT_JMP;
      i=lb->addr-(ip+6);
      ICU16(tempi,0x830F);
      ICU32(tempi,i);
    }
  }

  lb=tempi->ic_data;
  i=0xC524FF48+(r&7)<<27;
  if (r>7) i+=2;
  ICU32(tempi,i);
  if (lx->flags&LF_AOT_COMPILE)
    i=lb->addr+lx->a->ip;
  else
    i=lb->addr+buf;
  if (lx->flags&LF_AOT_COMPILE)
    j=ip2+tempi->ic_cnt;
  ICU32(tempi,i);
  if (buf && lx->flags&LF_AOT_COMPILE) {
    tempa=CAlloc(sizeof(CExeAbsAddr));
    tempa->next=lx->a->abss;
    tempa->type=EAA_ADD_U32;
    lx->a->abss=tempa;
    tempa->ip=j;
  }
  tempi->ic_flags&=~ICF_CODE_FINAL;
}

U0 ICLocalVarInit(CIntermediateCode *tempi)
{
  ICU24(tempi,0xC48B48);
  ICU16(tempi,0x5748);
  ICU24(tempi,0xF88B48);
  ICU24(tempi,0xC1C748);
  ICU32(tempi,tempi->ic_data);
  ICU16(tempi,sys_var_init_val<<8+0xB0);
  ICU16(tempi,0xAAF3);
  ICU16(tempi,0x5F48);
}
