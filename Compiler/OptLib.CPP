CLexCodeMisc *OptLabelFwd(CLexCodeMisc *lb)
{
  CLexCodeMisc *lb1;
  while (lb1=lb->fwd)
    lb=lb1;
  return lb;
}

CHashClass *OptClassFwd(CHashClass *tempc)
{
  CHashClass *tempc1;
  while (tempc1=tempc->fwd_class)
    tempc=tempc1;
  return tempc;
}

U0 OptSetNOP1(CIntermediateCode *tempi)
{
  tempi->ic_code=IC_NOP1;
  tempi->ic_flags=0;
  tempi->a1.type=TY_NULL+tempi->a1.type&RT_MASK;
  tempi->r.type =TY_NULL+tempi->r.type &RT_MASK;
}

U0 OptSetNOP2(CIntermediateCode *tempi,I64 stk_delta=1)
{
  tempi->ic_code=IC_NOP2;
  tempi->ic_data=stk_delta;
  tempi->a1.type=TY_NULL+tempi->a1.type&RT_MASK;
  tempi->r.type =TY_NULL+tempi->r.type &RT_MASK;
}

CIntermediateCode *OptFree(CIntermediateCode *tempi)
{ //We might access freed entries in CICTreeLinks
  QueRem(tempi);
  Free(tempi);
  return NULL;
}

CIntermediateCode *OptLag(CIntermediateCode *tempi)
{
  do {
    if (!tempi->ic_code)
      return NULL;
    else
      tempi=tempi->last;
  } while (tempi->ic_code<=IC_END_EXP);
  return tempi;
}

CIntermediateCode *OptLag1(CIntermediateCode *tempi)
{
  do {
    if (!tempi->ic_code)
      return NULL;
    else
      tempi=tempi->last;
  } while (tempi->ic_code==IC_NOP1||tempi->ic_code==IC_NOP2);
  return tempi;
}

CIntermediateCode *OptLag2(CIntermediateCode *tempi)
{
  do {
    if (!tempi->ic_code)
      return NULL;
    else
      tempi=tempi->last;
  } while (tempi->ic_code<IC_END_EXP);
  return tempi;
}

CIntermediateCode *OptLead1(CIntermediateCode *tempi)
{
  do {
    tempi=tempi->next;
    if (!tempi->ic_code)
      return NULL;
  } while (tempi->ic_code==IC_NOP1||tempi->ic_code==IC_NOP2);
  return tempi;
}

I64 CmpOffset2Reg(I64 offset,COptReg *reg_offsets)
{
  I64 i;
  for (i=0;i<NUM_REGS;i++)
    if (offset==reg_offsets[i].offset)
      return i;
  return -1;
}

#define FBO1_NOT_CONST	0
#define FBO1_INT	1
#define FBO1_F64	2

Bool OptFixupBinaryOp1(CIntermediateCode *tempi,
     CIntermediateCode *tempi1,CIntermediateCode *tempi2,
     Bool *is_unsigned)
{
  CHashClass *tempc=tempi->ic_class,*tempc1,*tempc2;

  if (tempi1->ic_flags&ICF_R_TO_INT)
    tempc1=sys_internal_types[RT_I64];
  else if (tempi1->ic_flags&ICF_R_TO_F64)
    tempc1=sys_internal_types[RT_F64];
  else {
    tempc1=OptClassFwd(tempi1->ic_class);
  }

  if (tempi2->ic_flags&ICF_R_TO_INT)
    tempc2=sys_internal_types[RT_I64];
  else if (tempi2->ic_flags&ICF_R_TO_F64)
    tempc2=sys_internal_types[RT_F64];
  else {
    tempc2=OptClassFwd(tempi2->ic_class);
  }

  if (tempc1->raw_type>tempc2->raw_type)
    tempc=tempi->ic_class=tempc1;
  else
    tempc=tempi->ic_class=tempc2;

  if (tempc->raw_type==RT_F64) {
    if (tempi1->ic_code==IC_IMM_U64) {
      tempi1->ic_data(F64)=tempi1->ic_data;
      tempi1->ic_class=sys_internal_types[RT_F64];
      tempi1->ic_code=IC_IMM_F64;
      tempi1->ic_flags&=~ICF_R_TO_F64;
    } else
      if (tempc1->raw_type!=RT_F64)
      tempi1->ic_flags|=ICF_R_TO_F64;
    if (tempi2->ic_code==IC_IMM_U64) {
      tempi2->ic_data(F64)=tempi2->ic_data;
      tempi2->ic_class=sys_internal_types[RT_F64];
      tempi2->ic_code=IC_IMM_F64;
      tempi2->ic_flags&=~ICF_R_TO_F64;
    } else
      if (tempc2->raw_type!=RT_F64)
      tempi2->ic_flags|=ICF_R_TO_F64;
    if (tempi1->ic_code==IC_IMM_F64 &&
	tempi2->ic_code==IC_IMM_F64 &&
	!(tempi->ic_flags&(ICF_PUSH_CMP|ICF_POP_CMP))) {
      tempi->ic_flags|=tempi1->ic_flags|tempi2->ic_flags;
      OptSetNOP1(tempi1);
      OptSetNOP1(tempi2);
      return FBO1_F64;
    } else
      return FBO1_NOT_CONST;
  }
  *is_unsigned=tempc1->raw_type&1 || tempc2->raw_type&1;
  if (tempi1->ic_code==IC_IMM_U64 &&
      tempi2->ic_code==IC_IMM_U64 &&
      !(tempi->ic_flags&(ICF_PUSH_CMP|ICF_POP_CMP))) {
    tempi->ic_flags|=tempi1->ic_flags|tempi2->ic_flags;
    OptSetNOP1(tempi1);
    OptSetNOP1(tempi2);
    return FBO1_INT;
  } else
    return FBO1_NOT_CONST;
}

Bool OptFixupBinaryOp2(CIntermediateCode **tempi1,CIntermediateCode **tempi2)
{
  CIntermediateCode *tempii1=*tempi1,
		   *tempii2=*tempi2;
  if (tempii1->ic_code==IC_IMM_U64 &&
      !(tempii1->ic_flags & ICF_R_TO_F64))
    return TRUE;
  if (tempii2->ic_code==IC_IMM_U64 &&
      !(tempii2->ic_flags & ICF_R_TO_F64)) {
    *tempi1=tempii2;
    *tempi2=tempii1;
    return TRUE;
  }
  return FALSE;
}

Bool OptFixupUnaryOp(CIntermediateCode *tempi,
     CIntermediateCode *tempi1,
     Bool *is_unsigned)
{
  CHashClass *tempc,*tempc1;
  tempc1=OptClassFwd(tempi1->ic_class);
  tempi->ic_class=tempc1;
  tempc=tempi->ic_class;
  if (tempc->raw_type==RT_F64) {
    if (tempi1->ic_code==IC_IMM_U64) {
      tempi1->ic_data(F64)=tempi1->ic_data;
      tempi1->ic_class=sys_internal_types[RT_F64];
      tempi1->ic_code=IC_IMM_F64;
      tempi1->ic_flags&=~ICF_R_TO_F64;
    } else
      if (tempc1->raw_type!=RT_F64)
      tempi1->ic_flags|=ICF_R_TO_F64;
    if (tempi1->ic_code==IC_IMM_F64) {
      tempi->ic_flags|=tempi1->ic_flags;
      OptSetNOP1(tempi1);
      return FBO1_F64;
    } else
      return FBO1_NOT_CONST;
  }
  *is_unsigned=tempc1->raw_type&1;
  if (tempi1->ic_code==IC_IMM_U64) {
    tempi->ic_flags|=tempi1->ic_flags;
    OptSetNOP1(tempi1);
    return FBO1_INT;
  } else
    return FBO1_NOT_CONST;
}

extern U0 OptBrNotZero(CLex *lx,CIntermediateCode *tempi);

CIntermediateCode *OptBrZero(CLex *lx,CIntermediateCode *tempi)
{
  CLexCodeMisc *lb_true,*lb_false;
  CIntermediateCode *tempii=OptLag(tempi),*tempii2;
  switch (tempii->ic_code) {
    case IC_NOT:
      tempi->ic_code=IC_BR_NOT_ZERO;
      tempi->ic_class=tempii->ic_class;
      tempi->ic_flags|=tempii->ic_flags;
      tempi->t.a1c=tempii->t.a1c;
      tempi->t.a1t=tempii->t.a1t;
      OptFree(tempii);
      return OptBrNotZero(lx,tempi);
    case IC_EQUAL_EQUAL:
      tempi->ic_code=IC_BR_NOT_EQUAL;
      break;
    case IC_NOT_EQUAL:
      tempi->ic_code=IC_BR_EQUAL_EQUAL;
      break;
    case IC_LESS:
      tempi->ic_code=IC_BR_GREATER_EQUAL;
      break;
    case IC_GREATER:
      tempi->ic_code=IC_BR_LESS_EQUAL;
      break;
    case IC_LESS_EQUAL:
      tempi->ic_code=IC_BR_GREATER;
      break;
    case IC_GREATER_EQUAL:
      tempi->ic_code=IC_BR_LESS;
      break;
    case IC_OR_OR:
      tempi->ic_code=IC_BR_OR_OR_ZERO;
      break;
    case IC_AND_AND:
      tempi->ic_code=IC_BR_AND_AND_ZERO;
      break;
    case IC_AND:
      tempi->ic_code=IC_BR_AND_ZERO;
      break;
    case IC_MM_:
      if (lx->pass==2 && !(tempii->ic_flags&ICF_R_TO_F64) &&
	    tempii->ic_class->raw_type!=RT_F64)
	tempi->ic_code=IC_BR_MM_ZERO;
      break;
    case IC_CALL_END:
      tempii2=OptLag(tempii);
      switch (tempii2->ic_code) {
	sub_switch_start:
	  case IC_CARRY:
	    tempii2->ic_code=IC_BR_NOT_CARRY;
	    break;
	  case IC_BT:
	    tempii2->ic_code=IC_BR_NOT_BT;
	    break;
	  case IC_BTS:
	    tempii2->ic_code=IC_BR_NOT_BTS;
	    break;
	  case IC_BTR:
	    tempii2->ic_code=IC_BR_NOT_BTR;
	    break;
	  case IC_BTC:
	    tempii2->ic_code=IC_BR_NOT_BTC;
	    break;
	  case IC_LBTS:
	    tempii2->ic_code=IC_BR_NOT_LBTS;
	    tempii2->ic_flags|=ICF_LOCK;
	    break;
	  case IC_LBTR:
	    tempii2->ic_code=IC_BR_NOT_LBTR;
	    tempii2->ic_flags|=ICF_LOCK;
	    break;
	  case IC_LBTC:
	    tempii2->ic_code=IC_BR_NOT_LBTC;
	    tempii2->ic_flags|=ICF_LOCK;
	    break;
	sub_switch_end:
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_code=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
      }
      break;
  }
  if (tempi->ic_code!=IC_BR_ZERO) {
    tempi->ic_class=tempii->ic_class;
    tempi->ic_flags|=tempii->ic_flags;
    tempi->t.a1c=tempii->t.a1c;
    tempi->t.a2c=tempii->t.a2c;
    tempi->t.a1t=tempii->t.a1t;
    tempi->t.a2t=tempii->t.a2t;
    OptFree(tempii);

    if (tempi->ic_flags&ICF_PUSH_CMP &&
	  IC_BR_NOT_EQUAL<=tempi->ic_code<=IC_BR_LESS &&
	  !(tempi->ic_flags&ICF_USE_F64)) {
      tempi->ic_code+=IC_BR_NOT_EQUAL2-IC_BR_NOT_EQUAL;
      tempi->ic_flags&=~ICF_PUSH_CMP;
      tempii =tempi->next; //IC_PUSH_CMP instruction
      while (tempii->ic_code!=IC_PUSH_CMP)
	tempii=tempii->next;
      tempii->t.a1t=tempi;
      OptSetNOP1(tempii);
    }

    lb_true=tempi->ic_data;
    if (tempi->ic_code==IC_BR_AND_AND_ZERO) {
      tempii=tempi->t.a1t->next;
      tempii->ic_data=lb_true;
      tempii->t.a1t=tempi->t.a1t;
      tempii->t.a1c=tempi->t.a1c;
      tempii->ic_code=IC_BR_ZERO;
      OptBrZero(lx,tempii);
      tempii=tempi->t.a2t->next;
      tempii->t.a1t=tempi->t.a2t;
      tempii->t.a1c=tempi->t.a2c;
      tempii->ic_data=lb_true;
      tempii->ic_code=IC_BR_ZERO;
      tempii=OptBrZero(lx,tempii);
      OptSetNOP1(tempi);
    } else if (tempi->ic_code==IC_BR_OR_OR_ZERO) {
      lb_false=LCCMiscNew(lx,CMT_LABEL);
      tempi->ic_code=IC_LABEL;
      tempi->ic_flags=0;
      tempi->ic_data=lb_false;
      tempii=tempi->t.a1t->next;
      tempii->t.a1t=tempi->t.a1t;
      tempii->t.a1c=tempi->t.a1c;
      tempii->ic_data=lb_false;
      tempii->ic_code=IC_BR_NOT_ZERO;
      OptBrNotZero(lx,tempii);
      tempii=tempi->t.a2t->next;
      tempii->t.a1t=tempi->t.a2t;
      tempii->t.a1c=tempi->t.a2c;
      tempii->ic_data=lb_true;
      tempii->ic_code=IC_BR_ZERO;
      tempii=OptBrZero(lx,tempii);
    } else
      tempii=tempi;
    if (tempi->ic_flags&ICF_POP_CMP && tempi->t.a1t->ic_code==IC_NOP1) {
      tempi->t.a1t=tempi->t.a1t->t.a1t;
      tempi->ic_flags&=~ICF_POP_CMP;
    }
    return tempii;
  }
  return tempi;
}


CIntermediateCode *OptBrNotZero(CLex *lx,CIntermediateCode *tempi)
{
  CLexCodeMisc *lb_true,*lb_false;
  CIntermediateCode *tempii=OptLag(tempi),*tempii2;
  switch (tempii->ic_code) {
    case IC_NOT:
      tempi->ic_code=IC_BR_ZERO;
      tempi->ic_class=tempii->ic_class;
      tempi->ic_flags|=tempii->ic_flags;
      tempi->t.a1c=tempii->t.a1c;
      tempi->t.a1t=tempii->t.a1t;
      OptFree(tempii);
      return OptBrZero(lx,tempi);
    case IC_EQUAL_EQUAL:
      tempi->ic_code=IC_BR_EQUAL_EQUAL;
      break;
    case IC_NOT_EQUAL:
      tempi->ic_code=IC_BR_NOT_EQUAL;
      break;
    case IC_LESS:
      tempi->ic_code=IC_BR_LESS;
      break;
    case IC_GREATER:
      tempi->ic_code=IC_BR_GREATER;
      break;
    case IC_LESS_EQUAL:
      tempi->ic_code=IC_BR_LESS_EQUAL;
      break;
    case IC_GREATER_EQUAL:
      tempi->ic_code=IC_BR_GREATER_EQUAL;
      break;
    case IC_OR_OR:
      tempi->ic_code=IC_BR_OR_OR_NOT_ZERO;
      break;
    case IC_AND_AND:
      tempi->ic_code=IC_BR_AND_AND_NOT_ZERO;
      break;
    case IC_AND:
      tempi->ic_code=IC_BR_AND_NOT_ZERO;
      break;
    case IC_MM_:
      if (lx->pass==2 && !(tempii->ic_flags&ICF_R_TO_F64) &&
	    tempii->ic_class->raw_type!=RT_F64)
	tempi->ic_code=IC_BR_MM_NOT_ZERO;
      break;
    case IC_CALL_END:
      tempii2=OptLag(tempii);
      switch (tempii2->ic_code) {
	sub_switch_start:
	  case IC_CARRY:
	    tempii2->ic_code=IC_BR_CARRY;
	    break;
	  case IC_BT:
	    tempii2->ic_code=IC_BR_BT;
	    break;
	  case IC_BTS:
	    tempii2->ic_code=IC_BR_BTS;
	    break;
	  case IC_BTR:
	    tempii2->ic_code=IC_BR_BTR;
	    break;
	  case IC_BTC:
	    tempii2->ic_code=IC_BR_BTC;
	    break;
	  case IC_LBTS:
	    tempii2->ic_code=IC_BR_LBTS;
	    tempii2->ic_flags|=ICF_LOCK;
	    break;
	  case IC_LBTR:
	    tempii2->ic_code=IC_BR_LBTR;
	    tempii2->ic_flags|=ICF_LOCK;
	    break;
	  case IC_LBTC:
	    tempii2->ic_code=IC_BR_LBTC;
	    tempii2->ic_flags|=ICF_LOCK;
	    break;
	sub_switch_end:
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_code=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
      }
      break;
  }
  if (tempi->ic_code!=IC_BR_NOT_ZERO) {
    tempi->ic_class=tempii->ic_class;
    tempi->ic_flags|=tempii->ic_flags;
    tempi->t.a1c=tempii->t.a1c;
    tempi->t.a2c=tempii->t.a2c;
    tempi->t.a1t=tempii->t.a1t;
    tempi->t.a2t=tempii->t.a2t;
    OptFree(tempii);

    if (tempi->ic_flags&ICF_PUSH_CMP &&
	  IC_BR_NOT_EQUAL<=tempi->ic_code<=IC_BR_LESS &&
	  !(tempi->ic_flags&ICF_USE_F64)) {
      tempi->ic_code+=IC_BR_NOT_EQUAL2-IC_BR_NOT_EQUAL;
      tempi->ic_flags&=~ICF_PUSH_CMP;
      tempii =tempi->next; //IC_PUSH_CMP instruction
      while (tempii->ic_code!=IC_PUSH_CMP)
	tempii=tempii->next;
      tempii->t.a1t=tempi;
      OptSetNOP1(tempii);
    }

    lb_true=tempi->ic_data;
    if (tempi->ic_code==IC_BR_OR_OR_NOT_ZERO) {
      tempii=tempi->t.a1t->next;
      tempii->t.a1t=tempi->t.a1t;
      tempii->t.a1c=tempi->t.a1c;
      tempii->ic_data=lb_true;
      tempii->ic_code=IC_BR_NOT_ZERO;
      OptBrNotZero(lx,tempii);
      tempii=tempi->t.a2t->next;
      tempii->t.a1t=tempi->t.a2t;
      tempii->t.a1c=tempi->t.a2c;
      tempii->ic_data=lb_true;
      tempii->ic_code=IC_BR_NOT_ZERO;
      tempii=OptBrNotZero(lx,tempii);
      OptSetNOP1(tempi);
    } else if (tempi->ic_code==IC_BR_AND_AND_NOT_ZERO) {
      lb_false=LCCMiscNew(lx,CMT_LABEL);
      tempi->ic_code=IC_LABEL;
      tempi->ic_flags=0;
      tempi->ic_data=lb_false;
      tempii=tempi->t.a1t->next;
      tempii->t.a1t=tempi->t.a1t;
      tempii->t.a1c=tempi->t.a1c;
      tempii->ic_data=lb_false;
      tempii->ic_code=IC_BR_ZERO;
      OptBrZero(lx,tempii);
      tempii=tempi->t.a2t->next;
      tempii->t.a1t=tempi->t.a2t;
      tempii->t.a1c=tempi->t.a2c;
      tempii->ic_data=lb_true;
      tempii->ic_code=IC_BR_NOT_ZERO;
      tempii=OptBrNotZero(lx,tempii);
    } else
      tempii=tempi;
    if (tempi->ic_flags&ICF_POP_CMP && tempi->t.a1t->ic_code==IC_NOP1) {
      tempi->t.a1t=tempi->t.a1t->t.a1t;
      tempi->ic_flags&=~ICF_POP_CMP;
    }
    return tempii;
  }
  return tempi;
}

U0 OptFixSizeOf(CIntermediateCode *tempi1,CIntermediateCode *tempi_push,CHashClass *tempcc)
{
  if (tempi1->ic_code==IC_MUL &&
      tempi1->t.a2t->ic_code==IC_SIZEOF) {
    tempi1->t.a2t->ic_code=IC_IMM_U64;
    tempi1->t.a2t->ic_class=tempcc;
    tempi_push->ic_class=tempcc;
    if (tempcc->ptr_cnt) {
      tempcc--;
      if (tempcc->size==1)
	goto here;
      tempi1->t.a2t->ic_data=tempcc->size;
    } else {
here:
      if (tempi_push==tempi1)
	tempi1->t.a2t->ic_data=1;
      else {
	OptSetNOP1(tempi1->t.a2t);
	OptSetNOP1(tempi1);
      }
    }
  }
}

I64 CmpRawType(CHashClass *tempc)
{
  if (tempc) {
    tempc=OptClassFwd(tempc);
    return tempc->raw_type;
  }
  return 0;
}

I64 CmpRawTypePointed(CHashClass *tempc)
{
  if (tempc) {
    if (tempc->ptr_cnt)
      tempc--;
    tempc=OptClassFwd(tempc);
    return tempc->raw_type;
  }
  return 0;
}

U0 CmpF1PushPop(CIntermediateCode *tempi,CIntermediateCode *tempi2)
{
  if (Bt(ic_bitmap_fpop,tempi2->ic_code)|| tempi2->ic_flags&ICF_R_TO_F64)
    Bts(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0);
}

U0 CmpF2PushPop(CIntermediateCode *tempi,CIntermediateCode *tempi1,CIntermediateCode *tempi2)
{
  if ((tempi2->ic_code==IC_MOV || tempi2->ic_code==IC_IMM_F64) &&
       !(tempi2->ic_flags&ICF_R_TO_F64) &&
      (Bt(ic_bitmap_fpop,tempi1->ic_code) || tempi1->ic_flags&ICF_R_TO_F64))
    Bts(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0);
  else if ((Bt(ic_bitmap_fpop,tempi2->ic_code) || tempi2->ic_flags&ICF_R_TO_F64)&&
       !(tempi1->ic_flags&ICF_R_TO_F64))
    Bts(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0);
}

class COptMemberVar
{
  I64 score,offset_start,offset_end,lea_balance;
  CMemberLst *m;
};

I64 OptMVCompare(COptMemberVar *mv1,COptMemberVar *mv2)
{
  return mv2->score-mv1->score;
}
