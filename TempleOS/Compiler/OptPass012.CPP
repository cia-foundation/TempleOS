U0 OptMissing(CLex *lx,CIntermediateCode *tempi)
{
  "Pass:%d Missing IC handler\n",lx->pass;
  CBPutIC(tempi);
  LexExcept(lx,"ST_OPT_INTERNAL_ERROR");
}

CIntermediateCode opt_nop;

CPrsStk *OptStart(CLex *lx,CIntermediateCode **temp_lag=NULL)
{
  I64 i;
  CPrsStk *ps=MAlloc(sizeof(CPrsStk));
  CBInPtrsInit(&lx->cbh);
  ps->ptr=0;
  ps->ptr2=0;
  if (temp_lag)
    for (i=0;i<4;i++)
      temp_lag[i]=&opt_nop;
  return ps;
}

U0 OptEnd(CLex *lx,CPrsStk *ps)
{
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",lx->pass,ps->ptr;
    LexExcept(lx,"ST_OPT_INTERNAL_ERROR");
  }
  Free(ps);
}

CCodeBlkMisc *OptLabelFwd(CCodeBlkMisc *lb)
{
  CCodeBlkMisc *lb1;
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
  tempi->ic_opcode=IC_NOP1;
  tempi->ic_flags=0;
  tempi->p1.type=TY_NULL+tempi->p1.type&IT_MASK;
  tempi->r.type =TY_NULL+tempi->r.type &IT_MASK;
}

U0 OptSetNOP2(CIntermediateCode *tempi,I64 stk_delta=1)
{
  tempi->ic_opcode=IC_NOP2;
  tempi->ic_data=stk_delta;
  tempi->p1.type=TY_NULL+tempi->p1.type&IT_MASK;
  tempi->r.type =TY_NULL+tempi->r.type &IT_MASK;
}

CIntermediateCode *OptLag(CIntermediateCode *tempi)
{
  do {
    if (tempi->ic_opcode==IC_START)
      return NULL;
    else
      tempi--;
  } while (tempi->ic_opcode<=IC_END_EXP);
  return tempi;
}

CIntermediateCode *OptLag1(CIntermediateCode *tempi)
{
  do {
    if (tempi->ic_opcode==IC_START)
      return NULL;
    else
      tempi--;
  } while (tempi->ic_opcode==IC_NOP1||tempi->ic_opcode==IC_NOP2);
  return tempi;
}

CIntermediateCode *OptLead1(CLex *,CIntermediateCode *tempi)
{
  do {
    tempi++;
    if (tempi->ic_opcode==IC_END)
      return NULL;
  } while (tempi->ic_opcode==IC_NOP1||tempi->ic_opcode==IC_NOP2);
  return tempi;
}

I64 CmpOffset2Reg(I64 offset,COptReg *register_offsets)
{
  I64 i;
  for (i=0;i<NUM_REGS;i++)
    if (offset==register_offsets[i].offset)
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
    tempc1=sys_internal_types[IT_I64];
  else if (tempi1->ic_flags&ICF_R_TO_F64)
    tempc1=sys_internal_types[IT_F64];
  else {
    tempc1=OptClassFwd(tempi1->ic_class);
  }

  if (tempi2->ic_flags&ICF_R_TO_INT)
    tempc2=sys_internal_types[IT_I64];
  else if (tempi2->ic_flags&ICF_R_TO_F64)
    tempc2=sys_internal_types[IT_F64];
  else {
    tempc2=OptClassFwd(tempi2->ic_class);
  }

  if (tempc1->sub_type>tempc2->sub_type)
    tempc=tempi->ic_class=tempc1;
  else
    tempc=tempi->ic_class=tempc2;

  if (tempc->sub_type==IT_F64) {
    if (tempi1->ic_opcode==IC_IMM_U64) {
      tempi1->ic_data(F64)=tempi1->ic_data;
      tempi1->ic_class=sys_internal_types[IT_F64];
      tempi1->ic_opcode=IC_IMM_F64;
      tempi1->ic_flags&=~ICF_R_TO_F64;
    } else
      if (tempc1->sub_type!=IT_F64)
      tempi1->ic_flags|=ICF_R_TO_F64;
    if (tempi2->ic_opcode==IC_IMM_U64) {
      tempi2->ic_data(F64)=tempi2->ic_data;
      tempi2->ic_class=sys_internal_types[IT_F64];
      tempi2->ic_opcode=IC_IMM_F64;
      tempi2->ic_flags&=~ICF_R_TO_F64;
    } else
      if (tempc2->sub_type!=IT_F64)
      tempi2->ic_flags|=ICF_R_TO_F64;
    if (tempi1->ic_opcode==IC_IMM_F64 &&
	tempi2->ic_opcode==IC_IMM_F64 &&
	!(tempi->ic_flags&(ICF_PUSH_CMP|ICF_POP_CMP))) {
      tempi->ic_flags|=tempi1->ic_flags|tempi2->ic_flags;
      OptSetNOP1(tempi1);
      OptSetNOP1(tempi2);
      return FBO1_F64;
    } else
      return FBO1_NOT_CONST;
  }
  *is_unsigned=tempc1->sub_type&1 || tempc2->sub_type&1;
  if (tempi1->ic_opcode==IC_IMM_U64 &&
      tempi2->ic_opcode==IC_IMM_U64 &&
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
  if (tempii1->ic_opcode==IC_IMM_U64 &&
      !(tempii1->ic_flags & ICF_R_TO_F64))
    return TRUE;
  if (tempii2->ic_opcode==IC_IMM_U64 &&
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
  if (tempc->sub_type==IT_F64) {
    if (tempi1->ic_opcode==IC_IMM_U64) {
      tempi1->ic_data(F64)=tempi1->ic_data;
      tempi1->ic_class=sys_internal_types[IT_F64];
      tempi1->ic_opcode=IC_IMM_F64;
      tempi1->ic_flags&=~ICF_R_TO_F64;
    } else
      if (tempc1->sub_type!=IT_F64)
      tempi1->ic_flags|=ICF_R_TO_F64;
    if (tempi1->ic_opcode==IC_IMM_F64) {
      tempi->ic_flags|=tempi1->ic_flags;
      OptSetNOP1(tempi1);
      return FBO1_F64;
    } else
      return FBO1_NOT_CONST;
  }
  *is_unsigned=tempc1->sub_type&1;
  if (tempi1->ic_opcode==IC_IMM_U64) {
    tempi->ic_flags|=tempi1->ic_flags;
    OptSetNOP1(tempi1);
    return FBO1_INT;
  } else
    return FBO1_NOT_CONST;
}

extern U0 OptBrNotZero(CLex *lx,CIntermediateCode *tempi);

CIntermediateCode *OptBrZero(CLex *lx,CIntermediateCode *tempi)
{
  CCodeBlkMisc *lb_true,*lb_false;
  CIntermediateCode *tempii=OptLag(tempi),*tempii2;
  switch (tempii->ic_opcode) {
    case IC_NOT:
      tempi->ic_opcode=IC_BR_NOT_ZERO;
      tempi->ic_class=tempii->ic_class;
      tempi->ic_flags|=tempii->ic_flags;
      tempi->t.p1c=tempii->t.p1c;
      tempi->t.p1t=tempii->t.p1t;
      OptSetNOP1(tempii);
      return OptBrNotZero(lx,tempi);
    case IC_EQUAL_EQUAL:
      tempi->ic_opcode=IC_BR_NOT_EQUAL;
      break;
    case IC_NOT_EQUAL:
      tempi->ic_opcode=IC_BR_EQUAL_EQUAL;
      break;
    case IC_LESS:
      tempi->ic_opcode=IC_BR_GREATER_EQUAL;
      break;
    case IC_GREATER:
      tempi->ic_opcode=IC_BR_LESS_EQUAL;
      break;
    case IC_LESS_EQUAL:
      tempi->ic_opcode=IC_BR_GREATER;
      break;
    case IC_GREATER_EQUAL:
      tempi->ic_opcode=IC_BR_LESS;
      break;
    case IC_OR_OR:
      tempi->ic_opcode=IC_BR_OR_OR_ZERO;
      break;
    case IC_AND_AND:
      tempi->ic_opcode=IC_BR_AND_AND_ZERO;
      break;
    case IC_AND:
      tempi->ic_opcode=IC_BR_AND_ZERO;
      break;
    case IC_MM_:
      if (lx->pass==2 && !(tempii->ic_flags&ICF_R_TO_F64) &&
	  tempii->ic_class->sub_type!=IT_F64)
	tempi->ic_opcode=IC_BR_MM_ZERO;
      break;
    case IC_CALL_END:
      tempii2=OptLag(tempii);
      switch (tempii2->ic_opcode) {
	case IC_BT:
	  tempii2->ic_opcode=IC_BR_NOT_BT;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_BTS:
	  tempii2->ic_opcode=IC_BR_NOT_BTS;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_BTR:
	  tempii2->ic_opcode=IC_BR_NOT_BTR;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_BTC:
	  tempii2->ic_opcode=IC_BR_NOT_BTC;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_LBTS:
	  tempii2->ic_opcode=IC_BR_NOT_LBTS;
	  tempii2->ic_flags|=ICF_LOCK;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_LBTR:
	  tempii2->ic_opcode=IC_BR_NOT_LBTR;
	  tempii2->ic_flags|=ICF_LOCK;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_LBTC:
	  tempii2->ic_opcode=IC_BR_NOT_LBTC;
	  tempii2->ic_flags|=ICF_LOCK;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
      }
      break;
  }
  if (tempi->ic_opcode!=IC_BR_ZERO) {
    tempi->ic_class=tempii->ic_class;
    tempi->ic_flags|=tempii->ic_flags;
    tempi->t.p1c=tempii->t.p1c;
    tempi->t.p2c=tempii->t.p2c;
    tempi->t.p1t=tempii->t.p1t;
    tempi->t.p2t=tempii->t.p2t;
    OptSetNOP1(tempii);

    if (tempi->ic_flags&ICF_PUSH_CMP &&
	IC_BR_NOT_EQUAL<=tempi->ic_opcode<=IC_BR_LESS &&
	!(tempi->ic_flags&ICF_USE_F64)) {
      tempi->ic_opcode+=IC_BR_NOT_EQUAL2-IC_BR_NOT_EQUAL;
      tempi->ic_flags&=~ICF_PUSH_CMP;
      tempii =tempi+1; //IC_PUSH_CMP instruction
      while (tempii->ic_opcode!=IC_PUSH_CMP)
	tempii++;
      tempii->t.p1t=tempi;
      OptSetNOP1(tempii);
    }

    lb_true=tempi->ic_data;
    if (tempi->ic_opcode==IC_BR_AND_AND_ZERO) {
      tempii=tempi->t.p1t+1;
      tempii->ic_data=lb_true;
      tempii->t.p1t=tempi->t.p1t;
      tempii->t.p1c=tempi->t.p1c;
      tempii->ic_opcode=IC_BR_ZERO;
      OptBrZero(lx,tempii);
      tempii=tempi->t.p2t+1;
      tempii->t.p1t=tempi->t.p2t;
      tempii->t.p1c=tempi->t.p2c;
      tempii->ic_data=lb_true;
      tempii->ic_opcode=IC_BR_ZERO;
      tempii=OptBrZero(lx,tempii);
      OptSetNOP1(tempi);;
    } else if (tempi->ic_opcode==IC_BR_OR_OR_ZERO) {
      lb_false=CBMiscNew(lx,CBMT_LABEL);
      tempi->ic_opcode=IC_LABEL;
      tempi->ic_flags=0;
      tempi->ic_data=lb_false;
      tempii=tempi->t.p1t+1;
      tempii->t.p1t=tempi->t.p1t;
      tempii->t.p1c=tempi->t.p1c;
      tempii->ic_data=lb_false;
      tempii->ic_opcode=IC_BR_NOT_ZERO;
      OptBrNotZero(lx,tempii);
      tempii=tempi->t.p2t+1;
      tempii->t.p1t=tempi->t.p2t;
      tempii->t.p1c=tempi->t.p2c;
      tempii->ic_data=lb_true;
      tempii->ic_opcode=IC_BR_ZERO;
      tempii=OptBrZero(lx,tempii);
    } else
      tempii=tempi;
    if (tempi->ic_flags&ICF_POP_CMP && tempi->t.p1t->ic_opcode==IC_NOP1) {
      tempi->t.p1t=tempi->t.p1t->t.p1t;
      tempi->ic_flags&=~ICF_POP_CMP;
    }
    return tempii;
  }
  return tempi;
}


CIntermediateCode *OptBrNotZero(CLex *lx,CIntermediateCode *tempi)
{
  CCodeBlkMisc *lb_true,*lb_false;
  CIntermediateCode *tempii=OptLag(tempi),*tempii2;
  switch (tempii->ic_opcode) {
    case IC_NOT:
      tempi->ic_opcode=IC_BR_ZERO;
      tempi->ic_class=tempii->ic_class;
      tempi->ic_flags|=tempii->ic_flags;
      tempi->t.p1c=tempii->t.p1c;
      tempi->t.p1t=tempii->t.p1t;
      OptSetNOP1(tempii);
      return OptBrZero(lx,tempi);
    case IC_EQUAL_EQUAL:
      tempi->ic_opcode=IC_BR_EQUAL_EQUAL;
      break;
    case IC_NOT_EQUAL:
      tempi->ic_opcode=IC_BR_NOT_EQUAL;
      break;
    case IC_LESS:
      tempi->ic_opcode=IC_BR_LESS;
      break;
    case IC_GREATER:
      tempi->ic_opcode=IC_BR_GREATER;
      break;
    case IC_LESS_EQUAL:
      tempi->ic_opcode=IC_BR_LESS_EQUAL;
      break;
    case IC_GREATER_EQUAL:
      tempi->ic_opcode=IC_BR_GREATER_EQUAL;
      break;
    case IC_OR_OR:
      tempi->ic_opcode=IC_BR_OR_OR_NOT_ZERO;
      break;
    case IC_AND_AND:
      tempi->ic_opcode=IC_BR_AND_AND_NOT_ZERO;
      break;
    case IC_AND:
      tempi->ic_opcode=IC_BR_AND_NOT_ZERO;
      break;
    case IC_MM_:
      if (lx->pass==2 && !(tempii->ic_flags&ICF_R_TO_F64) &&
	  tempii->ic_class->sub_type!=IT_F64)
	tempi->ic_opcode=IC_BR_MM_NOT_ZERO;
      break;
    case IC_CALL_END:
      tempii2=OptLag(tempii);
      switch (tempii2->ic_opcode) {
	case IC_BT:
	  tempii2->ic_opcode=IC_BR_BT;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_BTS:
	  tempii2->ic_opcode=IC_BR_BTS;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_BTR:
	  tempii2->ic_opcode=IC_BR_BTR;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_BTC:
	  tempii2->ic_opcode=IC_BR_BTC;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_LBTS:
	  tempii2->ic_opcode=IC_BR_LBTS;
	  tempii2->ic_flags|=ICF_LOCK;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_LBTR:
	  tempii2->ic_opcode=IC_BR_LBTR;
	  tempii2->ic_flags|=ICF_LOCK;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
	case IC_LBTC:
	  tempii2->ic_opcode=IC_BR_LBTC;
	  tempii2->ic_flags|=ICF_LOCK;
	  tempii2->ic_data=tempi->ic_data;
	  tempii->ic_opcode=IC_CALL_END2;
	  OptSetNOP1(tempi);
	  return tempii;
      }
      break;
  }
  if (tempi->ic_opcode!=IC_BR_NOT_ZERO) {
    tempi->ic_class=tempii->ic_class;
    tempi->ic_flags|=tempii->ic_flags;
    tempi->t.p1c=tempii->t.p1c;
    tempi->t.p2c=tempii->t.p2c;
    tempi->t.p1t=tempii->t.p1t;
    tempi->t.p2t=tempii->t.p2t;
    OptSetNOP1(tempii);

    if (tempi->ic_flags&ICF_PUSH_CMP &&
	IC_BR_NOT_EQUAL<=tempi->ic_opcode<=IC_BR_LESS &&
	!(tempi->ic_flags&ICF_USE_F64)) {
      tempi->ic_opcode+=IC_BR_NOT_EQUAL2-IC_BR_NOT_EQUAL;
      tempi->ic_flags&=~ICF_PUSH_CMP;
      tempii =tempi+1; //IC_PUSH_CMP instruction
      while (tempii->ic_opcode!=IC_PUSH_CMP)
	tempii++;
      tempii->t.p1t=tempi;
      OptSetNOP1(tempii);
    }

    lb_true=tempi->ic_data;
    if (tempi->ic_opcode==IC_BR_OR_OR_NOT_ZERO) {
      tempii=tempi->t.p1t+1;
      tempii->t.p1t=tempi->t.p1t;
      tempii->t.p1c=tempi->t.p1c;
      tempii->ic_data=lb_true;
      tempii->ic_opcode=IC_BR_NOT_ZERO;
      OptBrNotZero(lx,tempii);
      tempii=tempi->t.p2t+1;
      tempii->t.p1t=tempi->t.p2t;
      tempii->t.p1c=tempi->t.p2c;
      tempii->ic_data=lb_true;
      tempii->ic_opcode=IC_BR_NOT_ZERO;
      tempii=OptBrNotZero(lx,tempii);
      OptSetNOP1(tempi);;
    } else if (tempi->ic_opcode==IC_BR_AND_AND_NOT_ZERO) {
      lb_false=CBMiscNew(lx,CBMT_LABEL);
      tempi->ic_opcode=IC_LABEL;
      tempi->ic_flags=0;
      tempi->ic_data=lb_false;
      tempii=tempi->t.p1t+1;
      tempii->t.p1t=tempi->t.p1t;
      tempii->t.p1c=tempi->t.p1c;
      tempii->ic_data=lb_false;
      tempii->ic_opcode=IC_BR_ZERO;
      OptBrZero(lx,tempii);
      tempii=tempi->t.p2t+1;
      tempii->t.p1t=tempi->t.p2t;
      tempii->t.p1c=tempi->t.p2c;
      tempii->ic_data=lb_true;
      tempii->ic_opcode=IC_BR_NOT_ZERO;
      tempii=OptBrNotZero(lx,tempii);
    } else
      tempii=tempi;
    if (tempi->ic_flags&ICF_POP_CMP && tempi->t.p1t->ic_opcode==IC_NOP1) {
      tempi->t.p1t=tempi->t.p1t->t.p1t;
      tempi->ic_flags&=~ICF_POP_CMP;
    }
    return tempii;
  }
  return tempi;
}

U0 OptFixSizeOf(CIntermediateCode *tempi1,CIntermediateCode *tempi_push,CHashClass *tempcc)
{
  if (tempi1->ic_opcode==IC_MUL &&
      tempi1->t.p2t->ic_opcode==IC_SIZEOF) {
    tempi1->t.p2t->ic_opcode=IC_IMM_U64;
    tempi1->t.p2t->ic_class=tempcc;
    tempi_push->ic_class=tempcc;
    if (tempcc->ptr_cnt) {
      tempcc--;
      if (tempcc->size==1)
	goto here;
      tempi1->t.p2t->ic_data=tempcc->size;
    } else {
here:
      if (tempi_push==tempi1)
	tempi1->t.p2t->ic_data=1;
      else {
	OptSetNOP1(tempi1);
	OptSetNOP1(tempi1->t.p2t);
      }
    }
  }
}

CIntermediateCode *OptPass012(CLex *lx,CHashFun *fun_class,Bool combined)
{
/*
Simplify CONST arithmetic.
Sets the class throughout Expression trees.
Returns the type of an Expression for use
in int<-->F64 conversions of fun
args.
*/
  I64 code,i;
  Bool is_unsigned;
  CHashClass *tempc,*tempc1,*tempc2;
  CIntermediateCode *tempi,*tempi1,*tempi2,*tempi3,*tempi_push,
	*temp_lag[4],*last_with_class=NULL,*tempii;
  CPrsStk *ps=OptStart(lx,temp_lag);
  CCodeBlkMisc *lb,*lb1,*lb2;
  if (combined)
    tempi=lx->cbh.cb_in_ptr;
  else
    tempi=CBGetIC1(&lx->cbh);
  if (tempi->ic_opcode==IC_START) {
    if (combined)
      tempi++;
    else
      tempi=CBGetIC1(&lx->cbh);
  }
#assert !IC_END
  while (code=tempi->ic_opcode) {
    OptNext(tempi,temp_lag);
    tempc=tempi->ic_class;
    tempi->ic_class2=tempc;
    tempi_push=tempi;
    MemSet(&tempi->p1,0,sizeof(CICArg)*4);
    nobound_switch (cmp_arg_cnt[code]) {
      case 9:
	ps->ptr-=tempi->ic_data>>2;
	break;
      case 2:
	tempi2=PrsPop(ps);
	tempc2=tempi2->ic_class;
	tempi->t.p2t=tempi2;
	tempi->t.p2c=PrsPop(ps);
      case 1:
	tempi1=PrsPop(ps);
	tempc1=tempi1->ic_class;
	tempi->t.p1t=tempi1;
	tempi->t.p1c=PrsPop(ps);
	break;
      case 0: //nobound_switch
	break;
    }
    if (Bt(not_constant_bitmap,code))
      lx->flags|=LF_NOT_CONSTANT;
    nobound_switch (code) {
      case IC_IMM_F64:
	tempi->ic_flags&=~ICF_R_TO_F64;
	if (lx->pass==2 && tempi->ic_flags&ICF_R_TO_INT) {
	  tempi->ic_data=ToI64(tempi->ic_data(F64));
	  tempi->ic_flags&=~ICF_R_TO_INT;
	  tempi->ic_opcode=IC_IMM_U64;
	  tempi->ic_class=sys_internal_types[IT_I64];
	}
	break;
      case IC_IMM_U64:
	tempi->ic_flags&=~ICF_R_TO_INT;
	if (lx->pass==2 && tempi->ic_flags&ICF_R_TO_F64) {
	  tempi->ic_data(F64)=ToF64(tempi->ic_data);
	  tempi->ic_flags&=~ICF_R_TO_F64;
	  tempi->ic_opcode=IC_IMM_F64;
	  tempi->ic_class=sys_internal_types[IT_F64];
	}
	break;
      case IC_HOLYC_TYPECAST:
	if (tempi1->ic_opcode==IC_IMM_U64 || tempi1->ic_opcode==IC_IMM_F64) {
	  if (tempi->ic_class->sub_type==IT_F64)
	    tempi1->ic_opcode=IC_IMM_F64;
	  else
	    tempi1->ic_opcode=IC_IMM_U64;
	  tempi1->ic_class=tempi->ic_class;
	  tempi1->ic_flags|=tempi->ic_flags;
	  tempi_push=tempi1;
	  OptSetNOP1(tempi);;
	} else {
	  if (tempi->ic_data) { //was paren
	    if (!tempi_push->ic_class->ptr_cnt) {
	      if (tempi_push->ic_class->sub_type==IT_F64)
		tempi_push->ic_class2=sys_internal_types[IT_F64];
	      else
		tempi_push->ic_class2=sys_internal_types[IT_I64];
	    }
	  } else {
	    tempi1->ic_class=tempi->ic_class;
	    tempi1->ic_flags|=tempi->ic_flags;
	    tempi_push=tempi1;
	    OptSetNOP1(tempi);;
	  }
	}
	break;
      case IC_FS:
      case IC_GS:
//CALL,FS/GS,CALL_END,IMM,ADD,DEREF-->MOV_FS/GS
	if (combined) {
	  tempi1=tempi+2; //IMM
	  tempi2=tempi+3; //ADD
	  tempi3=tempi+4; //DEREF
	  if (tempi1->ic_opcode==IC_IMM_U64 && tempi2->ic_opcode==IC_ADD &&
		tempi3->ic_opcode==IC_DEREF && !(tempi3->ic_flags&~ICF_NO_CVT_MASK)) {
	    tempi->ic_flags|=tempi1->ic_flags|tempi2->ic_flags|tempi3->ic_flags;
	    if (tempi->ic_opcode==IC_FS)
	      tempi->ic_opcode=IC_MOV_FS;
	    else
	      tempi->ic_opcode=IC_MOV_GS;
	    tempi->ic_data=tempi1->ic_data;
	    tempi->ic_class =tempi3->ic_class;
	    tempi->ic_class2=tempi3->ic_class2;
	    OptSetNOP1(tempi1);
	    OptSetNOP1(tempi2);
	    OptSetNOP1(tempi3);

	    tempi1=tempi-1; //CALL
	    tempi2=tempi+1; //CALL_END
	    tempi->ic_flags|=tempi1->ic_flags|tempi2->ic_flags;
	    OptSetNOP1(tempi1);
	    OptSetNOP1(tempi2);
	  }
	}
	break;
      case IC_STR_CONST:
      case IC_RBP:
      case IC_MOV_FS:
      case IC_MOV_GS:
      case IC_RIP:
      case IC_SIZEOF:
	break;
      case IC_PUSH_CMP:
	tempi1=temp_lag[1];
	if (tempi1->ic_opcode==IC_AND_AND)
	  tempi1=temp_lag[2];
	tempi->ic_class=tempi1->ic_class;
	tempi->ic_class2=tempi->ic_class;
	tempi->ic_data=0;
	if (tempi->ic_class->sub_type==IT_F64)
	  tempi->ic_flags|=ICF_USE_F64;
	break;
      case IC_COM:
	if (tempi1->ic_opcode==IC_IMM_U64) {
	  tempi->ic_data=~tempi1->ic_data;
	  tempi->ic_opcode=IC_IMM_U64;
	  tempi->ic_flags|=tempi1->ic_flags;
	  OptSetNOP1(tempi1);
	}
	tempi_push->ic_class2=sys_internal_types[IT_I64];
	break;
      case IC_NOT:
	if (tempc->sub_type==IT_F64) {
	  if (tempi1->ic_opcode==IC_IMM_F64) {
	    tempi->ic_data(F64)=!tempi1->ic_data(F64);
	    tempi->ic_opcode=IC_IMM_F64;
	    tempi->ic_flags|=tempi1->ic_flags;
	    OptSetNOP1(tempi1);
	  }
	  break;
	}
	if (tempi1->ic_opcode==IC_IMM_U64) {
	  tempi->ic_data=!tempi1->ic_data;
	  tempi->ic_opcode=IC_IMM_U64;
	  tempi->ic_flags|=tempi1->ic_flags;
	  OptSetNOP1(tempi1);
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_UNARY_MINUS:
	if (i=OptFixupUnaryOp(tempi,tempi1,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    tempi->ic_data=-tempi1->ic_data(I64);
	    tempi->ic_opcode=IC_IMM_U64;
	  } else {
	    tempi->ic_data(F64)=-tempi1->ic_data(F64);
	    tempi->ic_opcode=IC_IMM_F64;
	  }
	}
	if (tempc1->type&HTT_INTERNAL_TYPE && tempc1->sub_type& 1)
	  tempi->ic_class=sys_internal_types[tempc1->sub_type-1];
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_DEREF:
	if (lx->pass==2) {
	  if (!tempc->size)
	    LexPutWarn(lx,"ST_DEREF_U0",fun_class);
	  if (tempi1->ic_class->sub_type!=IT_F64) {
	    if (tempi1->ic_opcode==IC__PP) {
	      tempi->ic_opcode=IC_DEREF_PP;
	      tempi->ic_flags|=tempi1->ic_flags;
	      OptSetNOP1(tempi1);
	    } else if (tempi1->ic_opcode==IC__MM) {
	      tempi->ic_opcode=IC_DEREF_MM;
	      tempi->ic_flags|=tempi1->ic_flags;
	      OptSetNOP1(tempi1);
	    }
	  }
	}
	break;
      case IC__PP:
      case IC__MM:
      case IC_PP_:
      case IC_MM_:
	if (lx->pass==2 && !tempc->size)
	  LexPutWarn(lx,"ST_DEREF_U0",fun_class);
	break;
      case IC_SHL_CONST:
	if (i=OptFixupUnaryOp(tempi,tempi1,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    if (is_unsigned)
	      tempi->ic_data=tempi1->ic_data(U64)<<tempi->ic_data(U64);
	    else
	      tempi->ic_data=tempi1->ic_data(I64)<<tempi->ic_data(I64);
	    tempi->ic_opcode=IC_IMM_U64;
	  } else {
	    tempi->ic_data(F64)=tempi1->ic_data(F64)<<tempi->ic_data;
	    tempi->ic_opcode=IC_IMM_F64;
	  }
	} else if (tempi1->ic_opcode==IC_SHL_CONST) {
	  tempi->ic_flags|=tempi1->ic_flags;
	  tempi->ic_data+=tempi1->ic_data;
	  OptSetNOP1(tempi1);
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_SHR_CONST:
	if (i=OptFixupUnaryOp(tempi,tempi1,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    if (is_unsigned)
	      tempi->ic_data=tempi1->ic_data(U64)>>tempi->ic_data(U64);
	    else
	      tempi->ic_data=tempi1->ic_data(I64)>>tempi->ic_data(I64);
	    tempi->ic_opcode=IC_IMM_U64;
	  } else {
	    tempi->ic_data(F64)=tempi1->ic_data(F64)>>tempi->ic_data;
	    tempi->ic_opcode=IC_IMM_F64;
	  }
	} else if (tempi1->ic_opcode==IC_SHR_CONST) {
	  tempi->ic_flags|=tempi1->ic_flags;
	  tempi->ic_data+=tempi1->ic_data;
	  OptSetNOP1(tempi1);
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;

      case IC_POWER:
	if (tempc1->sub_type>tempc2->sub_type)
	  tempi->ic_class=tempc1;
	else
	  tempi->ic_class=tempc2;
	tempc=tempi->ic_class;
	if (tempc1->sub_type!=IT_F64)
	  tempi1->ic_flags|=ICF_R_TO_F64;
	if (tempc2->sub_type!=IT_F64)
	  tempi2->ic_flags|=ICF_R_TO_F64;
	if (tempc->sub_type!=IT_F64)
	  tempi->ic_flags|=ICF_R_TO_INT|ICF_USE_F64;
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_SHL:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    if (is_unsigned)
	      tempi->ic_data=tempi1->ic_data(U64)<<tempi2->ic_data(U64);
	    else
	      tempi->ic_data=tempi1->ic_data(I64)<<tempi2->ic_data(I64);
	    tempi->ic_opcode=IC_IMM_U64;
	  } else {
	    tempi->ic_data(F64)=tempi1->ic_data(F64) << tempi2->ic_data(F64);
	    tempi->ic_opcode=IC_IMM_F64;
	  }
	} else if (tempi2->ic_opcode==IC_IMM_U64) {
	  tempi->ic_flags|=tempi2->ic_flags;
	  tempi->ic_data=tempi2->ic_data;
	  tempi->ic_opcode=IC_SHL_CONST;
	  OptSetNOP1(tempi2);
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_SHR:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    if (is_unsigned)
	      tempi->ic_data=tempi1->ic_data(U64)>>tempi2->ic_data(U64);
	    else
	      tempi->ic_data=tempi1->ic_data(I64)>>tempi2->ic_data(I64);
	    tempi->ic_opcode=IC_IMM_U64;
	  } else {
	    tempi->ic_data(F64)=tempi1->ic_data(F64) >> tempi2->ic_data(F64);
	    tempi->ic_opcode=IC_IMM_F64;
	  }
	} else if (tempi2->ic_opcode==IC_IMM_U64) {
	  tempi->ic_flags|=tempi2->ic_flags;
	  tempi->ic_data=tempi2->ic_data;
	  tempi->ic_opcode=IC_SHR_CONST;
	  OptSetNOP1(tempi2);
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_MUL:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    if (is_unsigned)
	      tempi->ic_data=tempi1->ic_data(U64)*tempi2->ic_data(U64);
	    else
	      tempi->ic_data=tempi1->ic_data(I64)*tempi2->ic_data(I64);
	    tempi->ic_opcode=IC_IMM_U64;
	  } else {
	    tempi->ic_data(F64)=tempi1->ic_data(F64)*
		  tempi2->ic_data(F64);
	    tempi->ic_opcode=IC_IMM_F64;
	  }
	} else {
	  if (tempi1->ic_opcode==IC_IMM_U64 && lx->pass==2) {
	    switch (i=tempi1->ic_data) {
	      case 0:
		break;
	      case 1:
		tempi2->ic_flags|=tempi->ic_flags|tempi1->ic_flags;
		tempi2->ic_class2=tempi->ic_class2;
		tempi_push=tempi2;
		OptSetNOP1(tempi1);
		OptSetNOP1(tempi);;
		break;
	      default:
		if (Bsf(i)==Bsr(i)) {
		  tempi->ic_flags|=tempi1->ic_flags;
		  tempi->t.p1c=tempi->t.p2c;
		  tempi->ic_data=Bsf(i);
		  tempi->ic_opcode=IC_SHL_CONST;
		  OptSetNOP1(tempi1);
		}
	    }
	  } else if (tempi2->ic_opcode==IC_IMM_U64) {
	    switch (i=tempi2->ic_data) {
	      case 0:
		break;
	      case 1:
		tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
		tempi1->ic_class2=tempi->ic_class2;
		tempi_push=tempi1;
		OptSetNOP1(tempi2);
		OptSetNOP1(tempi);;
		break;
	      default:
		if (Bsf(i)==Bsr(i)) {
		  tempi->ic_flags|=tempi2->ic_flags;
		  tempi->ic_data=Bsf(i);
		  tempi->ic_opcode=IC_SHL_CONST;
		  OptSetNOP1(tempi2);
		} else if (tempi1->ic_opcode==IC_MUL && lx->pass==2) {
		  if (tempi1->t.p1t->ic_opcode==IC_IMM_U64) {
		    tempi1->ic_flags|=tempi->ic_flags;
		    tempi1->t.p1t->ic_data*=tempi2->ic_data;
		    tempi1->ic_class2=tempi->ic_class2;
		    tempi_push=tempi1;
		    OptSetNOP1(tempi2);
		    OptSetNOP1(tempi);;
		  } else if (tempi1->t.p2t->ic_opcode==IC_IMM_U64) {
		    tempi1->ic_flags|=tempi->ic_flags;
		    tempi1->t.p2t->ic_data*=tempi2->ic_data;
		    tempi1->ic_class2=tempi->ic_class2;
		    tempi_push=tempi1;
		    OptSetNOP1(tempi2);
		    OptSetNOP1(tempi);;
		  }
		}
	    }
	  } else if (tempi2->ic_opcode==IC_IMM_F64 && lx->pass==2) {
	    if (tempi2->ic_data(F64)==1.0) {
	      tempi1->ic_flags|=tempi2->ic_flags|tempi->ic_flags;
	      tempi1->ic_class2=tempi->ic_class2;
	      tempi_push=tempi1;
	      OptSetNOP1(tempi2);
	      OptSetNOP1(tempi);;
	    } else if (tempi1->ic_opcode==IC_MUL) {
	      if (tempi1->t.p1t->ic_opcode==IC_IMM_F64) {
		tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
		tempi1->t.p1t->ic_data(F64)*=tempi2->ic_data(F64);
		tempi1->ic_class2=tempi->ic_class2;
		tempi_push=tempi1;
		OptSetNOP1(tempi2);
		OptSetNOP1(tempi);;
	      } else if (tempi1->t.p2t->ic_opcode==IC_IMM_F64) {
		tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
		tempi1->t.p2t->ic_data(F64)*=tempi2->ic_data(F64);
		tempi1->ic_class2=tempi->ic_class2;
		tempi_push=tempi1;
		OptSetNOP1(tempi2);
		OptSetNOP1(tempi);;
	      }
	    }
	  }
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_DIV:
	if ((tempi2->ic_data || tempi2->ic_opcode!=IC_IMM_U64 && tempi2->ic_opcode!=IC_IMM_F64) &&
	      (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned))) {
	  if (i==FBO1_INT) {
	    if (is_unsigned)
	      tempi->ic_data=tempi1->ic_data(U64)/tempi2->ic_data(U64);
	    else
	      tempi->ic_data=tempi1->ic_data(I64)/tempi2->ic_data(I64);
	    tempi->ic_opcode=IC_IMM_U64;
	  } else {
	    tempi->ic_data(F64)=tempi1->ic_data(F64)/
		  tempi2->ic_data(F64);
	    tempi->ic_opcode=IC_IMM_F64;
	  }
	} else {
	  if (tempi2->ic_opcode==IC_IMM_U64 && (i=tempi2->ic_data)) {
	    if (i==1) {
	      tempi1->ic_flags|=tempi2->ic_flags|tempi->ic_flags;
	      tempi1->ic_class2=tempi->ic_class2;
	      tempi_push=tempi1;
	      OptSetNOP1(tempi2);
	      OptSetNOP1(tempi);;
	    } else if (Bsf(i)==Bsr(i)) {
	      tempi->ic_flags|=tempi2->ic_flags;
	      tempi->ic_data=Bsf(i);
	      tempi->ic_opcode=IC_SHR_CONST;
	      OptSetNOP1(tempi2);
	    }
	  }
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_MOD:
	if ((tempi2->ic_data || tempi2->ic_opcode!=IC_IMM_U64 && tempi2->ic_opcode!=IC_IMM_F64) &&
	      (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned))) {
	  if (i==FBO1_INT) {
	    if (is_unsigned)
	      tempi->ic_data=tempi1->ic_data(U64)%tempi2->ic_data(U64);
	    else
	      tempi->ic_data=tempi1->ic_data(I64)%tempi2->ic_data(I64);
	    tempi->ic_opcode=IC_IMM_U64;
	  } else {
	    tempi->ic_data(F64)=tempi1->ic_data(F64)%
		  tempi2->ic_data(F64);
	    tempi->ic_opcode=IC_IMM_F64;
	  }
	} else if (lx->pass==2 && tempi2->ic_opcode==IC_IMM_U64 &&
	      (i=tempi2->ic_data) && Bsf(i)==Bsr(i) && tempi_push->ic_class->sub_type!=IT_F64 &&
	      tempi_push->ic_class->sub_type&1) { //do only unsigned
	  tempi2->ic_data=i-1;
	  tempi->ic_opcode=IC_AND;
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_AND:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  tempi->ic_data=tempi1->ic_data&tempi2->ic_data;
	  if (i==FBO1_INT)
	    tempi->ic_opcode=IC_IMM_U64;
	  else
	    tempi->ic_opcode=IC_IMM_F64;
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_OR:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  tempi->ic_data=tempi1->ic_data|tempi2->ic_data;
	  if (i==FBO1_INT)
	    tempi->ic_opcode=IC_IMM_U64;
	  else
	    tempi->ic_opcode=IC_IMM_F64;
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_XOR:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  tempi->ic_data=tempi1->ic_data^tempi2->ic_data;
	  if (i==FBO1_INT)
	    tempi->ic_opcode=IC_IMM_U64;
	  else
	    tempi->ic_opcode=IC_IMM_F64;
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_ADD:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    tempi->ic_data=tempi1->ic_data+tempi2->ic_data;
	    tempi->ic_opcode=IC_IMM_U64;
	  } else {
	    tempi->ic_data(F64)=tempi1->ic_data(F64)+
		  tempi2->ic_data(F64);
	    tempi->ic_opcode=IC_IMM_F64;
	  }
	} else {
	  if (tempi1->ic_opcode==IC_ABS_ADDR&&tempi2->ic_opcode==IC_IMM_U64) {
	    tempi->ic_flags|=tempi1->ic_flags|tempi2->ic_flags;
	    tempi->ic_data=tempi1->ic_data+tempi2->ic_data;
	    tempi->ic_opcode=IC_ABS_ADDR;
	    OptSetNOP1(tempi1);
	    OptSetNOP1(tempi2);
	  } else if (lx->pass==2) {
	    if (tempi1->ic_opcode==IC_IMM_U64) {
	      if (!tempi1->ic_data) {
		tempi2->ic_flags|=tempi1->ic_flags|tempi->ic_flags;
		tempi2->ic_class2=tempi->ic_class2;
		tempi_push=tempi2;
		OptSetNOP1(tempi1);
		OptSetNOP1(tempi);;
	      } else if (tempi2->ic_opcode==IC_ADD||
		    tempi2->ic_opcode==IC_SUB) {
		if (tempi2->t.p1t->ic_opcode==IC_IMM_U64) {
		  tempi2->ic_flags|=tempi->ic_flags;
		  tempi2->t.p1t->ic_data+=tempi1->ic_data;
		  tempi2->ic_class2=tempi->ic_class2;
		  tempi_push=tempi2;
		  OptSetNOP1(tempi1);
		  OptSetNOP1(tempi);;
		} else if (tempi2->t.p2t->ic_opcode==IC_IMM_U64) {
		  tempi2->ic_flags|=tempi->ic_flags;
		  if (tempi2->ic_opcode==IC_ADD)
		    tempi2->t.p2t->ic_data+=tempi1->ic_data;
		  else
		    tempi2->t.p2t->ic_data-=tempi1->ic_data;
		  tempi2->ic_class2=tempi->ic_class2;
		  tempi_push=tempi2;
		  OptSetNOP1(tempi1);
		  OptSetNOP1(tempi);;
		}
	      }
	    } else if (tempi2->ic_opcode==IC_IMM_U64) {
	      if (!tempi2->ic_data) {
		tempi1->ic_flags|=tempi2->ic_flags|tempi->ic_flags;
		tempi1->ic_class2=tempi->ic_class2;
		tempi_push=tempi1;
		OptSetNOP1(tempi2);
		OptSetNOP1(tempi);;
	      } else if (tempi1->ic_opcode==IC_ADD ||
		    tempi1->ic_opcode==IC_SUB) {
		if (tempi1->t.p1t->ic_opcode==IC_IMM_U64) {
		  tempi1->ic_flags|=tempi->ic_flags;
		  tempi1->t.p1t->ic_data+=tempi2->ic_data;
		  tempi1->ic_class2=tempi->ic_class2;
		  tempi_push=tempi1;
		  OptSetNOP1(tempi2);
		  OptSetNOP1(tempi);;
		} else if (tempi1->t.p2t->ic_opcode==IC_IMM_U64) {
		  tempi1->ic_flags|=tempi->ic_flags;
		  if (tempi1->ic_opcode==IC_ADD)
		    tempi1->t.p2t->ic_data+=tempi2->ic_data;
		  else
		    tempi1->t.p2t->ic_data-=tempi2->ic_data;
		  tempi1->ic_class2=tempi->ic_class2;
		  tempi_push=tempi1;
		  OptSetNOP1(tempi2);
		  OptSetNOP1(tempi);;
		}
	      }
	    } else if (tempi1->ic_opcode==IC_IMM_F64) {
	      if (!tempi1->ic_data) {
		tempi2->ic_flags|=tempi1->ic_flags|tempi->ic_flags;
		tempi2->ic_class2=tempi->ic_class2;
		tempi_push=tempi2;
		OptSetNOP1(tempi1);
		OptSetNOP1(tempi);;
	      } else if (tempi2->ic_opcode==IC_ADD||
		    tempi2->ic_opcode==IC_SUB) {
		if (tempi2->t.p1t->ic_opcode==IC_IMM_F64) {
		  tempi2->ic_flags|=tempi->ic_flags;
		  tempi2->t.p1t->ic_data(F64)+=tempi1->ic_data(F64);
		  tempi2->ic_class2=tempi->ic_class2;
		  tempi_push=tempi2;
		  OptSetNOP1(tempi1);
		  OptSetNOP1(tempi);;
		} else if (tempi2->t.p2t->ic_opcode==IC_IMM_F64) {
		  tempi2->ic_flags|=tempi->ic_flags;
		  if (tempi2->ic_opcode==IC_ADD)
		    tempi2->t.p2t->ic_data(F64)+=tempi1->ic_data(F64);
		  else
		    tempi2->t.p2t->ic_data(F64)-=tempi1->ic_data(F64);
		  tempi2->ic_class2=tempi->ic_class2;
		  tempi_push=tempi2;
		  OptSetNOP1(tempi1);
		  OptSetNOP1(tempi);;
		}
	      }
	    } else if (tempi2->ic_opcode==IC_IMM_F64) {
	      if (!tempi2->ic_data) {
		tempi1->ic_flags|=tempi2->ic_flags|tempi->ic_flags;
		tempi1->ic_class2=tempi->ic_class2;
		tempi_push=tempi1;
		OptSetNOP1(tempi2);
		OptSetNOP1(tempi);;
	      } else if (tempi1->ic_opcode==IC_ADD ||
		    tempi1->ic_opcode==IC_SUB) {
		if (tempi1->t.p1t->ic_opcode==IC_IMM_F64) {
		  tempi1->ic_flags|=tempi->ic_flags;
		  tempi1->t.p1t->ic_data(F64)+=tempi2->ic_data(F64);
		  tempi1->ic_class2=tempi->ic_class2;
		  tempi_push=tempi1;
		  OptSetNOP1(tempi2);
		  OptSetNOP1(tempi);;
		} else if (tempi1->t.p2t->ic_opcode==IC_IMM_F64) {
		  tempi1->ic_flags|=tempi->ic_flags;
		  if (tempi1->ic_opcode==IC_ADD)
		    tempi1->t.p2t->ic_data(F64)+=tempi2->ic_data(F64);
		  else
		    tempi1->t.p2t->ic_data(F64)-=tempi2->ic_data(F64);
		  tempi1->ic_class2=tempi->ic_class2;
		  tempi_push=tempi1;
		  OptSetNOP1(tempi2);
		  OptSetNOP1(tempi);;
		}
	      }
	    }
	  }
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_SUB:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    tempi->ic_data=tempi1->ic_data-tempi2->ic_data;
	    tempi->ic_opcode=IC_IMM_U64;
	  } else {
	    tempi->ic_data(F64)=tempi1->ic_data(F64)-
		  tempi2->ic_data(F64);
	    tempi->ic_opcode=IC_IMM_F64;
	  }
	} else {
	  if (lx->pass==2) {
	    if (tempi2->ic_opcode==IC_IMM_U64) {
	      if (!tempi2->ic_data) {
		tempi1->ic_flags|=tempi2->ic_flags|tempi->ic_flags;
		tempi1->ic_class2=tempi->ic_class2;
		tempi_push=tempi1;
		OptSetNOP1(tempi2);
		OptSetNOP1(tempi);;
	      } else
		if (tempi1->ic_opcode==IC_ADD ||
		      tempi1->ic_opcode==IC_SUB) {
		  if (tempi1->t.p1t->ic_opcode==IC_IMM_U64) {
		    tempi1->ic_flags|=tempi->ic_flags;
		    tempi1->t.p1t->ic_data-=tempi2->ic_data;
		    tempi1->ic_class2=tempi->ic_class2;
		    tempi_push=tempi1;
		    OptSetNOP1(tempi2);
		    OptSetNOP1(tempi);;
		  } else if (tempi1->t.p2t->ic_opcode==IC_IMM_U64) {
		    tempi1->ic_flags|=tempi->ic_flags;
		    if (tempi1->ic_opcode==IC_ADD)
		      tempi1->t.p2t->ic_data-=tempi2->ic_data;
		    else
		      tempi1->t.p2t->ic_data+=tempi2->ic_data;
		    tempi1->ic_class2=tempi->ic_class2;
		    tempi_push=tempi1;
		    OptSetNOP1(tempi2);
		    OptSetNOP1(tempi);;
		  }
		}
	    } else
	      if (tempi2->ic_opcode==IC_IMM_F64) {
		if (!tempi2->ic_data) {
		  tempi1->ic_flags|=tempi2->ic_flags|tempi->ic_flags;
		  tempi1->ic_class2=tempi->ic_class2;
		  tempi_push=tempi1;
		  OptSetNOP1(tempi2);
		  OptSetNOP1(tempi);;
		} else
		  if (tempi1->ic_opcode==IC_ADD ||
			tempi1->ic_opcode==IC_SUB) {
		    if (tempi1->t.p1t->ic_opcode==IC_IMM_F64) {
		      tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
		      tempi1->t.p1t->ic_data(F64)-=tempi2->ic_data(F64);
		      tempi1->ic_class2=tempi->ic_class2;
		      tempi_push=tempi1;
		      OptSetNOP1(tempi2);
		      OptSetNOP1(tempi);;
		    } else if (tempi1->t.p2t->ic_opcode==IC_IMM_F64) {
		      tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
		      if (tempi1->ic_opcode==IC_ADD)
			tempi1->t.p2t->ic_data(F64)-=tempi2->ic_data(F64);
		      else
			tempi1->t.p2t->ic_data(F64)+=tempi2->ic_data(F64);
		      tempi1->ic_class2=tempi->ic_class2;
		      tempi_push=tempi1;
		      OptSetNOP1(tempi2);
		      OptSetNOP1(tempi);;
		    }
		  }
	      }
	  }

	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	break;
      case IC_EQUAL_EQUAL:
	if (OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  tempi->ic_data=tempi1->ic_data==tempi2->ic_data;
	  tempi->ic_opcode=IC_IMM_U64;
	} else
	  if (tempi->ic_class->sub_type==IT_F64)
	    tempi->ic_flags|=ICF_USE_F64;
	tempi->ic_flags&=~ICF_R_TO_INT;
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	if (tempi_push->ic_flags & ICF_PUSH_CMP)
	  tempi->ic_class=tempi->ic_class2;
	else
	  tempi->ic_class=sys_internal_types[IT_I64];
	break;
      case IC_NOT_EQUAL:
	if (OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  tempi->ic_data=tempi1->ic_data!=tempi2->ic_data;
	  tempi->ic_opcode=IC_IMM_U64;
	} else
	  if (tempi->ic_class->sub_type==IT_F64)
	    tempi->ic_flags|=ICF_USE_F64;
	tempi->ic_flags&=~ICF_R_TO_INT;
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	if (tempi_push->ic_flags & ICF_PUSH_CMP)
	  tempi->ic_class=tempi->ic_class2;
	else
	  tempi->ic_class=sys_internal_types[IT_I64];
	break;
      case IC_LESS:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    if (is_unsigned)
	      tempi->ic_data=tempi1->ic_data(U64)<tempi2->ic_data(U64);
	    else
	      tempi->ic_data=tempi1->ic_data(I64)<tempi2->ic_data(I64);
	  } else {
	    tempi->ic_data=tempi1->ic_data(F64)<
		  tempi2->ic_data(F64);
	  }
	  tempi->ic_opcode=IC_IMM_U64;
	} else {
	  if (is_unsigned)
	    tempi->ic_flags|=ICF_USE_UNSIGNED;
	  if (tempi->ic_class->sub_type==IT_F64)
	    tempi->ic_flags|=ICF_USE_F64;
	}
	tempi->ic_flags&=~ICF_R_TO_INT;
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	if (tempi_push->ic_flags & ICF_PUSH_CMP)
	  tempi->ic_class=tempi->ic_class2;
	else
	  tempi->ic_class=sys_internal_types[IT_I64];
	break;
      case IC_GREATER:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    if (is_unsigned)
	      tempi->ic_data=tempi1->ic_data(U64)>tempi2->ic_data(U64);
	    else
	      tempi->ic_data=tempi1->ic_data(I64)>tempi2->ic_data(I64);
	  } else {
	    tempi->ic_data=tempi1->ic_data(F64)>
		  tempi2->ic_data(F64);
	  }
	  tempi->ic_opcode=IC_IMM_U64;
	} else {
	  if (is_unsigned)
	    tempi->ic_flags|=ICF_USE_UNSIGNED;
	  if (tempi->ic_class->sub_type==IT_F64)
	    tempi->ic_flags|=ICF_USE_F64;
	}
	tempi->ic_flags&=~ICF_R_TO_INT;
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	if (tempi_push->ic_flags & ICF_PUSH_CMP)
	  tempi->ic_class=tempi->ic_class2;
	else
	  tempi->ic_class=sys_internal_types[IT_I64];
	break;
      case IC_LESS_EQUAL:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    if (is_unsigned)
	      tempi->ic_data=tempi1->ic_data(U64)<=tempi2->ic_data(U64);
	    else
	      tempi->ic_data=tempi1->ic_data(I64)<=tempi2->ic_data(I64);
	  } else {
	    tempi->ic_data=tempi1->ic_data(F64)<=
		  tempi2->ic_data(F64);
	  }
	  tempi->ic_opcode=IC_IMM_U64;
	} else {
	  if (is_unsigned)
	    tempi->ic_flags|=ICF_USE_UNSIGNED;
	  if (tempi->ic_class->sub_type==IT_F64)
	    tempi->ic_flags|=ICF_USE_F64;
	}
	tempi->ic_flags&=~ICF_R_TO_INT;
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	if (tempi_push->ic_flags & ICF_PUSH_CMP)
	  tempi->ic_class=tempi->ic_class2;
	else
	  tempi->ic_class=sys_internal_types[IT_I64];
	break;
      case IC_GREATER_EQUAL:
	if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  if (i==FBO1_INT) {
	    if (is_unsigned)
	      tempi->ic_data=tempi1->ic_data(U64)>=tempi2->ic_data(U64);
	    else
	      tempi->ic_data=tempi1->ic_data(I64)>=tempi2->ic_data(I64);
	  } else {
	    tempi->ic_data=tempi1->ic_data(F64)>=
		  tempi2->ic_data(F64);
	  }
	  tempi->ic_opcode=IC_IMM_U64;
	} else {
	  if (is_unsigned)
	    tempi->ic_flags|=ICF_USE_UNSIGNED;
	  if (tempi->ic_class->sub_type==IT_F64)
	    tempi->ic_flags|=ICF_USE_F64;
	}
	tempi->ic_flags&=~ICF_R_TO_INT;
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	if (tempi_push->ic_flags & ICF_PUSH_CMP)
	  tempi->ic_class=tempi->ic_class2;
	else
	  tempi->ic_class=sys_internal_types[IT_I64];
	break;
      case IC_AND_AND:
	if (OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  tempi->ic_data=tempi1->ic_data&&tempi2->ic_data;
	  tempi->ic_opcode=IC_IMM_U64;
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	tempi->ic_class=sys_internal_types[IT_I64];
	break;
      case IC_OR_OR:
	if (OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  tempi->ic_data=tempi1->ic_data||tempi2->ic_data;
	  tempi->ic_opcode=IC_IMM_U64;
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	tempi->ic_class=sys_internal_types[IT_I64];
	break;
      case IC_XOR_XOR:
	if (OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	  tempi->ic_data=tempi1->ic_data^^tempi2->ic_data;
	  tempi->ic_opcode=IC_IMM_U64;
	}
	if (!tempi_push->ic_class->ptr_cnt) {
	  if (tempi_push->ic_class->sub_type==IT_F64)
	    tempi_push->ic_class2=sys_internal_types[IT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=sys_internal_types[IT_U64];
	  else
	    tempi_push->ic_class2=sys_internal_types[IT_I64];
	}
	tempi->ic_class=sys_internal_types[IT_I64];
	break;
      case IC_MUL_EQUAL:
	if (lx->pass==2 && (!tempc->size||!tempc2->size)) LexPutWarn(lx,"ST_ASSIGN_U0",fun_class);

	if (tempi2->ic_opcode==IC_IMM_U64 &&
	      tempc->sub_type!=IT_F64 &&
	      tempc2->sub_type!=IT_F64) {
	  if (i=tempi2->ic_data) {
	    if (Bsf(i)==Bsr(i)) {
	      tempi2->ic_data=Bsf(i);
	      tempi->ic_opcode=IC_SHL_EQUAL;
	    }
	  }
	}
	if (tempi2->ic_class->sub_type==IT_F64)
	  tempi->ic_flags=tempi->ic_flags|ICF_USE_F64;
	if (tempc->sub_type==IT_F64) {
	  if (tempc2->sub_type!=IT_F64)
	    tempi2->ic_flags|=ICF_R_TO_F64;
	}
	break;
      case IC_DIV_EQUAL:
	if (lx->pass==2 && (!tempc->size||!tempc2->size)) LexPutWarn(lx,"ST_ASSIGN_U0",fun_class);

	if (tempi2->ic_opcode==IC_IMM_U64 &&
	      tempc->sub_type!=IT_F64 &&
	      tempc2->sub_type!=IT_F64 &&
	      (i=tempi2->ic_data) && Bsf(i)==Bsr(i)) {
	  tempi2->ic_data=Bsf(i);
	  tempi->ic_opcode=IC_SHR_EQUAL;
	}
	if (tempi2->ic_class->sub_type==IT_F64)
	  tempi->ic_flags=tempi->ic_flags|ICF_USE_F64;
	if (tempc->sub_type==IT_F64) {
	  if (tempc2->sub_type!=IT_F64)
	    tempi2->ic_flags|=ICF_R_TO_F64;
	}
	break;
      case IC_MOD_EQUAL:
	if (lx->pass==2 && (!tempc->size||!tempc2->size)) LexPutWarn(lx,"ST_ASSIGN_U0",fun_class);

	if (tempi2->ic_opcode==IC_IMM_U64 &&
	      tempc->sub_type!=IT_F64 &&
	      tempc2->sub_type!=IT_F64 &&
	      (i=tempi2->ic_data) && Bsf(i)==Bsr(i)) {
	  tempi2->ic_data=i-1;
	  tempi->ic_opcode=IC_AND_EQUAL;
	}
	if (tempi2->ic_class->sub_type==IT_F64)
	  tempi->ic_flags=tempi->ic_flags|ICF_USE_F64;
	if (tempc->sub_type==IT_F64) {
	  if (tempc2->sub_type!=IT_F64)
	    tempi2->ic_flags|=ICF_R_TO_F64;
	}
	break;
      case IC_ADD_EQUAL:
      case IC_SUB_EQUAL:
	if (lx->pass==2 && (!tempc->size||!tempc2->size)) LexPutWarn(lx,"ST_ASSIGN_U0",fun_class);

	if (tempi2->ic_class->sub_type==IT_F64)
	  tempi->ic_flags=tempi->ic_flags|ICF_USE_F64;
	if (tempc->sub_type==IT_F64) {
	  if (tempc2->sub_type!=IT_F64)
	    tempi2->ic_flags|=ICF_R_TO_F64;
	}
	break;
      case IC_ASSIGN:
	if (lx->pass==2 && (!tempc->size||!tempc2->size)) LexPutWarn(lx,"ST_ASSIGN_U0",fun_class);

	if (tempc->sub_type==IT_F64) {
	  if (tempc2->sub_type!=IT_F64)
	    tempi2->ic_flags|=ICF_R_TO_F64;
	} else {
	  if (tempc2->sub_type==IT_F64)
	    tempi2->ic_flags|=ICF_R_TO_INT;
	}
	if (lx->pass==2 && tempi1->ic_class->sub_type!=IT_F64) {
	  if (tempi1->ic_opcode==IC__PP) {
	    tempi->ic_opcode=IC_ASSIGN_PP;
	    tempi->ic_flags|=tempi1->ic_flags;
	    tempi->t.class2=tempi1->ic_class;
	    OptSetNOP1(tempi1);
	  } else if (tempi1->ic_opcode==IC__MM) {
	    tempi->ic_opcode=IC_ASSIGN_MM;
	    tempi->ic_flags|=tempi1->ic_flags;
	    tempi->t.class2=tempi1->ic_class;
	    OptSetNOP1(tempi1);
	  }
	}
	break;
      case IC_SHL_EQUAL:
      case IC_SHR_EQUAL:
      case IC_AND_EQUAL:
      case IC_OR_EQUAL:
      case IC_XOR_EQUAL:
	if (lx->pass==2 && (!tempc->size||!tempc2->size)) LexPutWarn(lx,"ST_ASSIGN_U0",fun_class);

	if (tempc2->sub_type==IT_F64)
	  tempi2->ic_flags|=ICF_R_TO_INT;
	break;
      case IC_ENTER:
      case IC_LEAVE:
	tempi->ic_data=-fun_class->size;
	break;
      case IC_ADD_RSP:
	if (temp_lag[1]->ic_opcode==IC_ADD_RSP) {
	  tempi->ic_data+=temp_lag[1]->ic_data;
	  tempi->ic_flags|=temp_lag[1]->ic_flags;
	  OptSetNOP1(temp_lag[1]);
	}
      case IC_ADD_RSP1:
	break;

      case IC_CALL:
      case IC_CALL_INDIRECT:
      case IC_CALL_INDIRECT2:
      case IC_CALL_EXTERN:
      case IC_CALL_IMPORT:
      case IC_CLI:
      case IC_STI:
      case IC_HLT:
      case IC_PUSHFD:
      case IC_POPFD:
      case IC_PUSH:
      case IC_POP:
      case IC_CLD:
      case IC_STD:
      case IC_INVLPG:
      case IC_CLFLUSH:
      case IC_PAUSE:
      case IC_GET_FLAGS:
      case IC_CARRY:
      case IC_GET_BASE_PTR:
      case IC_GET_STK_PTR:
      case IC_GET_RAX:
      case IC_RETURN_VAL:
      case IC_RETURN_VAL2:
      case IC_ABS_ADDR:
      case IC_HEAP_GLBL:
      case IC_ADDR_IMPORT:
      case IC_GET_LABEL:
      case IC_TYPE:
      case IC_RDTSC:
      case IC_HPET:
      case IC_SET_FLAGS:
      case IC_SET_BASE_PTR:
      case IC_SET_STK_PTR:
      case IC_SET_RAX:
      case IC_BSF:
      case IC_BSR:
      case IC_PREEMPT:
      case IC_SIGN_I64:
      case IC_TOUPPER:
      case IC_ABS_I64:
      case IC_MIN_I64:
      case IC_MAX_I64:
      case IC_MIN_U64:
      case IC_MAX_U64:
      case IC_MOD_U64:
      case IC_SQR_I64:
      case IC_SQR_U64:
      case IC_SWAP_U8:
      case IC_SWAP_U16:
      case IC_SWAP_U32:
      case IC_SWAP_U64:
      case IC_IN_U32:
      case IC_IN_U16:
      case IC_IN_U8:
      case IC_STRLEN:
      case IC_BT:
      case IC_BTS:
      case IC_BTR:
      case IC_BTC:
      case IC_QUE_INIT:
      case IC_QUE_REM:
      case IC_QUE_INS:
      case IC_QUE_INS_REV:
      case IC_OUT_U32:
      case IC_OUT_U16:
      case IC_OUT_U8:
      case IC_NOBOUND_SWITCH:
      case IC_SWITCH:
      case IC_ASM:
	break;
      case IC_LBTS:
      case IC_LBTR:
      case IC_LBTC:
	tempi->ic_flags|=ICF_LOCK;
	break;
      case IC_TO_I64:
	if (combined) {
	  if (tempi1->ic_opcode==IC_IMM_F64) {
	    tempi->ic_flags|=tempi1->ic_flags&~ICF_R_TO_INT;
	    tempi->ic_opcode=IC_IMM_U64;
	    tempi->ic_data=ToI64(tempi1->ic_data(F64));
	    tempi->ic_class=sys_internal_types[IT_I64];
	    tempi->ic_class2=sys_internal_types[IT_I64];
	    OptSetNOP1(tempi1);

	    tempii=tempi1-1;
	    while (tempii->ic_opcode!=IC_CALL_START)
	      tempii--;
	    OptSetNOP1(tempii);

	    tempii=tempi+1;
	    while (tempii->ic_opcode!=IC_CALL_END)
	      tempii++;
	    OptSetNOP1(tempii);
	  } else if (tempi1->ic_opcode==IC_IMM_U64) {
	    tempi->ic_flags|=tempi1->ic_flags&~ICF_R_TO_F64;
	    tempi->ic_opcode=IC_IMM_U64;
	    tempi->ic_data=tempi1->ic_data;
	    tempi->ic_class=sys_internal_types[IT_I64];
	    tempi->ic_class2=sys_internal_types[IT_I64];
	    OptSetNOP1(tempi1);

	    tempii=tempi1-1;
	    while (tempii->ic_opcode!=IC_CALL_START)
	      tempii--;
	    OptSetNOP1(tempii);

	    tempii=tempi+1;
	    while (tempii->ic_opcode!=IC_CALL_END)
	      tempii++;
	    OptSetNOP1(tempii);
	  }
	  if (tempi1->ic_flags&ICF_R_TO_F64) {
	    tempi1->ic_flags=tempi->ic_flags|tempi1->ic_flags&~(ICF_R_TO_F64|ICF_PUSH_RESULT);
	    tempi1->ic_class=sys_internal_types[IT_I64];
	    tempi1->ic_class2=sys_internal_types[IT_I64];
	    tempi_push=tempi1;
	    OptSetNOP1(tempi);;

	    i=0;
	    tempii=tempi1-1;
	    while (TRUE) {
	      if (tempii->ic_opcode==IC_CALL_START) {
		if (!i) break;
		i--;
	      } else if (tempii->ic_opcode==IC_CALL_END)
		i++;
	      tempii--;
	    }
	    OptSetNOP1(tempii);

	    i=0;
	    tempii=tempi1+1;
	    while (TRUE) {
	      if (tempii->ic_opcode==IC_CALL_END) {
		if (!i) break;
		i--;
	      } else if (tempii->ic_opcode==IC_CALL_START)
		i++;
	      tempii++;
	    }
	    OptSetNOP1(tempii);
	  }
	}
	break;
      case IC_TO_F64:
	if (combined) {
	  if (tempi1->ic_opcode==IC_IMM_U64) {
	    tempi->ic_flags|=tempi1->ic_flags&~ICF_R_TO_F64;
	    tempi->ic_opcode=IC_IMM_F64;
	    tempi->ic_data(F64)=ToF64(tempi1->ic_data);
	    tempi->ic_class=sys_internal_types[IT_F64];
	    tempi->ic_class2=sys_internal_types[IT_F64];
	    OptSetNOP1(tempi1);

	    tempii=tempi1-1;
	    while (tempii->ic_opcode!=IC_CALL_START)
	      tempii--;
	    OptSetNOP1(tempii);

	    tempii=tempi+1;
	    while (tempii->ic_opcode!=IC_CALL_END)
	      tempii++;
	    OptSetNOP1(tempii);
	  } else if (tempi1->ic_opcode==IC_IMM_F64) {
	    tempi->ic_flags|=tempi1->ic_flags&~ICF_R_TO_INT;
	    tempi->ic_opcode=IC_IMM_F64;
	    tempi->ic_data=tempi1->ic_data;
	    tempi->ic_class=sys_internal_types[IT_F64];
	    tempi->ic_class2=sys_internal_types[IT_F64];
	    OptSetNOP1(tempi1);

	    tempii=tempi1-1;
	    while (tempii->ic_opcode!=IC_CALL_START)
	      tempii--;
	    OptSetNOP1(tempii);

	    tempii=tempi+1;
	    while (tempii->ic_opcode!=IC_CALL_END)
	      tempii++;
	    OptSetNOP1(tempii);
	  }
	  if (tempi1->ic_flags&ICF_R_TO_INT) {
	    tempi1->ic_flags=tempi->ic_flags|tempi1->ic_flags&~(ICF_R_TO_INT|ICF_PUSH_RESULT);
	    OptSetNOP1(tempi);;
	    tempi1->ic_class=sys_internal_types[IT_F64];
	    tempi1->ic_class2=sys_internal_types[IT_F64];
	    tempi_push=tempi1;

	    i=0;
	    tempii=tempi1-1;
	    while (TRUE) {
	      if (tempii->ic_opcode==IC_CALL_START) {
		if (!i) break;
		i--;
	      } else if (tempii->ic_opcode==IC_CALL_END)
		i++;
	      tempii--;
	    }
	    OptSetNOP1(tempii);

	    i=0;
	    tempii=tempi1+1;
	    while (TRUE) {
	      if (tempii->ic_opcode==IC_CALL_END) {
		if (!i) break;
		i--;
	      } else if (tempii->ic_opcode==IC_CALL_START)
		i++;
	      tempii++;
	    }
	    OptSetNOP1(tempii);
	  }
	}
	break;
      case IC_TO_BOOL:
	if (combined) {
	  if (tempi1->ic_opcode==IC_IMM_U64 || tempi1->ic_opcode==IC_IMM_F64) {
	    tempi->ic_flags|=tempi1->ic_flags&~ICF_R_TO_F64;
	    tempi->ic_opcode=IC_IMM_U64;
	    tempi->ic_data=ToBool(tempi1->ic_data);
	    tempi->ic_class=sys_internal_types[IT_I64];
	    tempi->ic_class2=sys_internal_types[IT_I64];
	    OptSetNOP1(tempi1);

	    tempii=tempi1-1;
	    while (tempii->ic_opcode!=IC_CALL_START)
	      tempii--;
	    OptSetNOP1(tempii);

	    tempii=tempi+1;
	    while (tempii->ic_opcode!=IC_CALL_END)
	      tempii++;
	    OptSetNOP1(tempii);
	  }
	}
	break;

      case IC_SQR:
      case IC_ABS:
      case IC_SQRT:
      case IC_SIN:
      case IC_COS:
      case IC_TAN:
      case IC_ATAN:
	break;

      case IC_BR_ZERO:
	if (combined)
	  tempi_push=OptBrZero(lx,tempi);
	break;
      case IC_BR_NOT_ZERO:
	if (combined)
	  tempi_push=OptBrNotZero(lx,tempi);
	break;
      case IC_BR_NOT_EQUAL:
      case IC_BR_EQUAL_EQUAL:
      case IC_BR_GREATER_EQUAL:
      case IC_BR_LESS_EQUAL:
      case IC_BR_GREATER:
      case IC_BR_LESS:
      case IC_BR_NOT_EQUAL2:
      case IC_BR_EQUAL_EQUAL2:
      case IC_BR_GREATER_EQUAL2:
      case IC_BR_LESS_EQUAL2:
      case IC_BR_GREATER2:
      case IC_BR_LESS2:
      case IC_BR_OR_OR_NOT_ZERO:
      case IC_BR_OR_OR_ZERO:
      case IC_BR_AND_AND_NOT_ZERO:
      case IC_BR_AND_AND_ZERO:
      case IC_BR_AND_NOT_ZERO:
      case IC_BR_AND_ZERO:
      case IC_BR_MM_NOT_ZERO:
      case IC_BR_MM_ZERO:
      case IC_BR_BT:
      case IC_BR_BTS:
      case IC_BR_BTR:
      case IC_BR_BTC:
      case IC_BR_NOT_BT:
      case IC_BR_NOT_BTS:
      case IC_BR_NOT_BTR:
      case IC_BR_NOT_BTC:
	break;
      case IC_BR_LBTS:
      case IC_BR_LBTR:
      case IC_BR_LBTC:
      case IC_BR_NOT_LBTS:
      case IC_BR_NOT_LBTR:
      case IC_BR_NOT_LBTC:
	tempi->ic_flags|=ICF_LOCK;
	break;

      case IC_NOP1:
	if (combined && tempi->ic_flags&ICF_PUSH_RESULT) {
	  tempii=tempi;
	  do tempii--;
	  while (tempii->ic_opcode==IC_NOP1);
	  tempii->ic_flags|=ICF_PUSH_RESULT;
	  tempi->ic_flags&=~ICF_PUSH_RESULT;
	}
	break;
      case IC_NOP2:
	ps->ptr+=tempi->ic_data<<1;
	break;
      case IC_LABEL:
	lb=OptLabelFwd(tempi->ic_data);
	lb1=tempi->ic_data;
	while (lb2=lb1->fwd) {
	  lb1->fwd=lb;
	  lb1=lb2;
	}
	tempii=temp_lag[1];
	if (tempii->ic_opcode==IC_JMP) {
	  lb1=tempii->ic_data;
	  while (lb1->fwd)
	    lb1=lb1->fwd;
	  if (lb1==lb) {
	    tempi->ic_flags|=tempii->ic_flags;
	    OptSetNOP1(tempii);
	  }
	} else if (tempii->ic_opcode==IC_LABEL) {
	  lb1=tempii->ic_data;
	  if (!lb1->fwd)
	    lb1->fwd=lb;
	  tempii=temp_lag[2];
	  if (tempii->ic_opcode==IC_JMP) {
	    lb1=tempii->ic_data;
	    while (lb1->fwd)
	      lb1=lb1->fwd;
	    if (lb1==lb) {
	      tempi->ic_flags|=tempii->ic_flags;
	      OptSetNOP1(tempii);
	    }
	  }
	}
	break;
      case IC_END:
      case IC_ADDR:
      case IC_RET:
      case IC_END_EXP:
      case IC_CALL_START:
      case IC_CALL_END:
      case IC_CALL_END2:
      case IC_PUSH_REGS:
      case IC_POP_REGS:
      case IC_IRQ:
	break;
      case IC_JMP:
	if (temp_lag[1]->ic_opcode==IC_LABEL) {
	  lb=OptLabelFwd(tempi->ic_data);
	  lb1=OptLabelFwd(temp_lag[1]->ic_data);
	  if (lb!=lb1)
	    lb1->fwd=lb;
	}
	break;
      default:
	OptMissing(lx,tempi);
    }
    if (cmp_arg_cnt[code]==2) {
      if (tempi_push->ic_precedence.u8[0]&~ASSOC_MASK==PREC_ASSIGN)
	OptFixSizeOf(tempi2,tempi_push,tempi1->ic_class-1);
      else {
	OptFixSizeOf(tempi1,tempi_push,tempi2->ic_class);
	OptFixSizeOf(tempi2,tempi_push,tempi1->ic_class);
      }
    }
    if (cmp_result_cnt[tempi_push->ic_opcode]) {
      PrsPush(ps,tempi->ic_class2);
      PrsPush(ps,tempi_push);
    }
    if (tempi->ic_class) {
      if (tempi->ic_class->sub_type==IT_F64)
	tempi->ic_flags&=~ICF_R_TO_F64;
      else
	tempi->ic_flags&=~ICF_R_TO_INT;
      if (code>IC_END_EXP)
	last_with_class=tempi;
    }
    if (combined)
      tempi++;
    else
      tempi=CBGetIC1(&lx->cbh);
  }
  OptEnd(lx,ps);

  /*
This is for determining type conversions
for passing args to funs.
*/
  return last_with_class;
}

