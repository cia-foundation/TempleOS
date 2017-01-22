CCodeMisc *OptLabelFwd(CCodeMisc *lb)
{
  CCodeMisc *lb1;
  while (lb1=lb->fwd)
    lb=lb1;
  return lb;
}

CHashClass *OptClassFwd(CHashClass *tmpc)
{//Class forwarding for unions and subclasses.
  CHashClass *tmpc1;
  while (tmpc1=tmpc->fwd_class)
    tmpc=tmpc1;
  return tmpc;
}

U0 OptSetNOP1(CIntermediateCode *tmpi)
{
  tmpi->ic_code=IC_NOP1;
  tmpi->ic_flags=0;
  tmpi->arg1.type=MDF_NULL+tmpi->arg1.type.raw_type;
  tmpi->res.type =MDF_NULL+tmpi->res.type.raw_type;
}

U0 OptSetNOP2(CIntermediateCode *tmpi,I64 stk_delta=1)
{
  tmpi->ic_code=IC_NOP2;
  tmpi->ic_data=stk_delta;
  tmpi->arg1.type=MDF_NULL+tmpi->arg1.type.raw_type;
  tmpi->res.type =MDF_NULL+tmpi->res.type.raw_type;
}

CIntermediateCode *OptFree(CIntermediateCode *tmpi)
{//We might access freed entries in CICTreeLinks
  QueRem(tmpi);
  Free(tmpi);
  return NULL;
}

CIntermediateCode *OptLag(CIntermediateCode *tmpi)
{
  do {
    if (!tmpi->ic_code)
      return NULL;
    else
      tmpi=tmpi->last;
  } while (tmpi->ic_code<=IC_END_EXP);
  return tmpi;
}

CIntermediateCode *OptLag1(CIntermediateCode *tmpi)
{
  do {
    if (!tmpi->ic_code)
      return NULL;
    else
      tmpi=tmpi->last;
  } while (tmpi->ic_code==IC_NOP1||tmpi->ic_code==IC_NOP2);
  return tmpi;
}

CIntermediateCode *OptLag2(CIntermediateCode *tmpi)
{
  do {
    if (!tmpi->ic_code)
      return NULL;
    else
      tmpi=tmpi->last;
  } while (tmpi->ic_code<IC_END_EXP);
  return tmpi;
}

CIntermediateCode *OptLead1(CIntermediateCode *tmpi)
{
  do {
    tmpi=tmpi->next;
    if (!tmpi->ic_code)
      return NULL;
  } while (tmpi->ic_code==IC_NOP1||tmpi->ic_code==IC_NOP2);
  return tmpi;
}

I64 CmpOffset2Reg(I64 offset,COptReg *reg_offsets)
{
  I64 i;
  for (i=0;i<REG_REGS_NUM;i++)
    if (offset==reg_offsets[i].offset)
      return i;
  return -1;
}

#define FBO1_NOT_CONST	0
#define FBO1_INT	1
#define FBO1_F64	2

Bool OptFixupBinaryOp1(CIntermediateCode *tmpi,
     CIntermediateCode *tmpi1,CIntermediateCode *tmpi2,
     Bool *is_unsigned)
{
  CIntermediateCode *tmpii;
  CHashClass *tmpc=tmpi->ic_class,*tmpc1,*tmpc2;

  if (tmpi1->ic_flags&ICF_RES_TO_INT)
    tmpc1=cmp.internal_types[RT_I64];
  else if (tmpi1->ic_flags&ICF_RES_TO_F64)
    tmpc1=cmp.internal_types[RT_F64];
  else {
    tmpc1=OptClassFwd(tmpi1->ic_class);
  }

  if (tmpi2->ic_flags&ICF_RES_TO_INT)
    tmpc2=cmp.internal_types[RT_I64];
  else if (tmpi2->ic_flags&ICF_RES_TO_F64)
    tmpc2=cmp.internal_types[RT_F64];
  else {
    tmpc2=OptClassFwd(tmpi2->ic_class);
  }

  if (tmpc1->raw_type>tmpc2->raw_type)
    tmpc=tmpi->ic_class=tmpc1;
  else
    tmpc=tmpi->ic_class=tmpc2;

  if (tmpc->raw_type==RT_F64) {
    if (tmpi1->ic_code==IC_IMM_I64) {
      tmpi1->ic_data(F64)=tmpi1->ic_data;
      tmpi1->ic_class=cmp.internal_types[RT_F64];
      tmpi1->ic_code=IC_IMM_F64;
      tmpi1->ic_flags&=~ICF_RES_TO_F64;
    } else
      if (tmpc1->raw_type!=RT_F64)
	tmpi1->ic_flags|=ICF_RES_TO_F64;
    if (tmpi2->ic_code==IC_IMM_I64) {
      tmpi2->ic_data(F64)=tmpi2->ic_data;
      tmpi2->ic_class=cmp.internal_types[RT_F64];
      tmpi2->ic_code=IC_IMM_F64;
      tmpi2->ic_flags&=~ICF_RES_TO_F64;
    } else
      if (tmpc2->raw_type!=RT_F64)
	tmpi2->ic_flags|=ICF_RES_TO_F64;
    if (IC_LESS<=tmpi->ic_code<=IC_GREATER_EQU && (tmpii=OptLead1(tmpi)) &&
	  tmpii->ic_code!=IC_PUSH_CMP && tmpii->ic_code!=IC_AND_AND) {
//We are looking for float comparisons to zero to convert to int.
      if (tmpi1->ic_code==IC_IMM_F64 && !tmpi1->ic_data &&
	    tmpi2->ic_code==IC_DEREF && tmpc2==cmp.internal_types[RT_F64]) {
	tmpi1->ic_code==IC_IMM_I64;
	goto fb_here1;
      } else if (tmpi2->ic_code==IC_IMM_F64 && !tmpi2->ic_data &&
	    tmpi1->ic_code==IC_DEREF && tmpc1==cmp.internal_types[RT_F64]) {
	tmpi2->ic_code==IC_IMM_I64;
fb_here1:
	tmpi1->ic_flags&=~ICF_RES_TO_F64;
	tmpi->ic_class=tmpi1->ic_class=tmpi2->ic_class=
	      cmp.internal_types[RT_I64];
	*is_unsigned=FALSE;
	return FBO1_NOT_CONST;
      }
      goto fb_here2;
    } else {
fb_here2:
      if (tmpi1->ic_code==IC_IMM_F64 && tmpi2->ic_code==IC_IMM_F64 &&
	    !(tmpi->ic_flags&(ICF_PUSH_CMP|ICF_POP_CMP))) {
	tmpi->ic_flags|=tmpi1->ic_flags|tmpi2->ic_flags;
	OptSetNOP1(tmpi1);
	OptSetNOP1(tmpi2);
	return FBO1_F64;
      } else
	return FBO1_NOT_CONST;
    }
  }
  *is_unsigned=tmpc1->raw_type&RTF_UNSIGNED || tmpc2->raw_type&RTF_UNSIGNED;
  if (tmpi1->ic_code==IC_IMM_I64 && tmpi2->ic_code==IC_IMM_I64 &&
	!(tmpi->ic_flags&(ICF_PUSH_CMP|ICF_POP_CMP))) {
    tmpi->ic_flags|=tmpi1->ic_flags|tmpi2->ic_flags;
    OptSetNOP1(tmpi1);
    OptSetNOP1(tmpi2);
    return FBO1_INT;
  } else
    return FBO1_NOT_CONST;
}

Bool OptFixupBinaryOp2(CIntermediateCode **tmpi1,CIntermediateCode **tmpi2)
{
  CIntermediateCode *tmpii1=*tmpi1,
	*tmpii2=*tmpi2;
  if (tmpii1->ic_code==IC_IMM_I64 && !(tmpii1->ic_flags & ICF_RES_TO_F64))
    return TRUE;
  if (tmpii2->ic_code==IC_IMM_I64 && !(tmpii2->ic_flags & ICF_RES_TO_F64)) {
    *tmpi1=tmpii2;
    *tmpi2=tmpii1;
    return TRUE;
  }
  return FALSE;
}

Bool OptFixupUnaryOp(CIntermediateCode *tmpi, CIntermediateCode *tmpi1,
     Bool *is_unsigned)
{
  CHashClass *tmpc,*tmpc1;
  tmpc1=OptClassFwd(tmpi1->ic_class);
  tmpi->ic_class=tmpc1;
  tmpc=tmpi->ic_class;
  if (tmpc->raw_type==RT_F64) {
    if (tmpi1->ic_code==IC_IMM_I64) {
      tmpi1->ic_data(F64)=tmpi1->ic_data;
      tmpi1->ic_class=cmp.internal_types[RT_F64];
      tmpi1->ic_code=IC_IMM_F64;
      tmpi1->ic_flags&=~ICF_RES_TO_F64;
    } else
      if (tmpc1->raw_type!=RT_F64)
	tmpi1->ic_flags|=ICF_RES_TO_F64;
    if (tmpi1->ic_code==IC_IMM_F64) {
      tmpi->ic_flags|=tmpi1->ic_flags;
      OptSetNOP1(tmpi1);
      return FBO1_F64;
    } else
      return FBO1_NOT_CONST;
  }
  *is_unsigned=tmpc1->raw_type&RTF_UNSIGNED;
  if (tmpi1->ic_code==IC_IMM_I64) {
    tmpi->ic_flags|=tmpi1->ic_flags;
    OptSetNOP1(tmpi1);
    return FBO1_INT;
  } else
    return FBO1_NOT_CONST;
}

extern CIntermediateCode *OptBrNotZero(CCmpCtrl *cc,CIntermediateCode *tmpi);

CIntermediateCode *OptBrZero(CCmpCtrl *cc,CIntermediateCode *tmpi)
{
  CCodeMisc *lb_true,*lb_false;
  CIntermediateCode *tmpii=OptLag(tmpi),*tmpii2;
  switch (tmpii->ic_code) {
    case IC_NOT:
      tmpi->ic_code=IC_BR_NOT_ZERO;
      tmpi->ic_class=tmpii->ic_class;
      tmpi->ic_flags|=tmpii->ic_flags;
      tmpi->t.arg1_class=tmpii->t.arg1_class;
      tmpi->t.arg1_tree=tmpii->t.arg1_tree;
      OptFree(tmpii);
      return OptBrNotZero(cc,tmpi);
    case IC_EQU_EQU...IC_LESS_EQU:
      tmpi->ic_code=(tmpii->ic_code-IC_EQU_EQU)^1+IC_BR_EQU_EQU;
      break;
    case IC_OR_OR:
      tmpi->ic_code=IC_BR_OR_OR_ZERO;
      break;
    case IC_AND_AND:
      tmpi->ic_code=IC_BR_AND_AND_ZERO;
      break;
    case IC_AND:
      tmpi->ic_code=IC_BR_AND_ZERO;
      break;
    case IC_MM_:
      if (cc->pass==2 && !(tmpii->ic_flags&ICF_RES_TO_F64) &&
	    tmpii->ic_class->raw_type!=RT_F64)
	tmpi->ic_code=IC_BR_MM_ZERO;
      break;
    case IC_CALL_END:
      tmpii2=OptLag(tmpii);
      switch (tmpii2->ic_code) {
	start:
	  case IC_CARRY:
	    tmpii2->ic_code=IC_BR_NOT_CARRY;
	    break;
	  case IC_BT:
	    tmpii2->ic_code=IC_BR_NOT_BT;
	    break;
	  case IC_LBTS:
	    tmpii2->ic_flags|=ICF_LOCK;
	  case IC_BTS:
	    tmpii2->ic_code=IC_BR_NOT_BTS;
	    break;
	  case IC_LBTR:
	    tmpii2->ic_flags|=ICF_LOCK;
	  case IC_BTR:
	    tmpii2->ic_code=IC_BR_NOT_BTR;
	    break;
	  case IC_LBTC:
	    tmpii2->ic_flags|=ICF_LOCK;
	  case IC_BTC:
	    tmpii2->ic_code=IC_BR_NOT_BTC;
	    break;
	end:
	  tmpii2->ic_data=tmpi->ic_data;
	  tmpii->ic_code=IC_CALL_END2;
	  OptSetNOP1(tmpi);
	  return tmpii;
      }
      break;
  }
  if (tmpi->ic_code!=IC_BR_ZERO) {
    tmpi->ic_class=tmpii->ic_class;
    tmpi->ic_flags|=tmpii->ic_flags;
    tmpi->t.arg1_class=tmpii->t.arg1_class;
    tmpi->t.arg2_class=tmpii->t.arg2_class;
    tmpi->t.arg1_tree=tmpii->t.arg1_tree;
    tmpi->t.arg2_tree=tmpii->t.arg2_tree;
    OptFree(tmpii);

    if (tmpi->ic_flags&ICF_PUSH_CMP &&
	  IC_BR_NOT_EQU<=tmpi->ic_code<=IC_BR_LESS_EQU &&
	  !(tmpi->ic_flags&ICF_USE_F64)) {
      tmpi->ic_code+=IC_BR_EQU_EQU2-IC_BR_EQU_EQU;
      tmpi->ic_flags&=~ICF_PUSH_CMP;
      tmpii=tmpi->next; //IC_PUSH_CMP inst
      while (tmpii->ic_code!=IC_PUSH_CMP)
	tmpii=tmpii->next;
      tmpii->t.arg1_tree=tmpi;
      OptSetNOP1(tmpii);
    }

    lb_true=tmpi->ic_data;
    if (tmpi->ic_code==IC_BR_AND_AND_ZERO) {
      tmpii=tmpi->t.arg1_tree->next;
      tmpii->ic_data=lb_true;
      tmpii->t.arg1_tree=tmpi->t.arg1_tree;
      tmpii->t.arg1_class=tmpi->t.arg1_class;
      tmpii->ic_code=IC_BR_ZERO;
      OptBrZero(cc,tmpii);
      tmpii=tmpi->t.arg2_tree->next;
      tmpii->t.arg1_tree=tmpi->t.arg2_tree;
      tmpii->t.arg1_class=tmpi->t.arg2_class;
      tmpii->ic_data=lb_true;
      tmpii->ic_code=IC_BR_ZERO;
      tmpii=OptBrZero(cc,tmpii);
      OptSetNOP1(tmpi);
    } else if (tmpi->ic_code==IC_BR_OR_OR_ZERO) {
      lb_false=COCMiscNew(cc,CMT_LABEL);
      tmpi->ic_code=IC_LABEL;
      tmpi->ic_flags=0;
      tmpi->ic_data=lb_false;
      tmpii=tmpi->t.arg1_tree->next;
      tmpii->t.arg1_tree=tmpi->t.arg1_tree;
      tmpii->t.arg1_class=tmpi->t.arg1_class;
      tmpii->ic_data=lb_false;
      tmpii->ic_code=IC_BR_NOT_ZERO;
      OptBrNotZero(cc,tmpii);
      tmpii=tmpi->t.arg2_tree->next;
      tmpii->t.arg1_tree=tmpi->t.arg2_tree;
      tmpii->t.arg1_class=tmpi->t.arg2_class;
      tmpii->ic_data=lb_true;
      tmpii->ic_code=IC_BR_ZERO;
      tmpii=OptBrZero(cc,tmpii);
    } else
      tmpii=tmpi;
    if (tmpi->ic_flags&ICF_POP_CMP && tmpi->t.arg1_tree->ic_code==IC_NOP1) {
      tmpi->t.arg1_tree=tmpi->t.arg1_tree->t.arg1_tree;
      tmpi->ic_flags&=~ICF_POP_CMP;
    }
    return tmpii;
  }
  return tmpi;
}

CIntermediateCode *OptBrNotZero(CCmpCtrl *cc,CIntermediateCode *tmpi)
{
  CCodeMisc *lb_true,*lb_false;
  CIntermediateCode *tmpii=OptLag(tmpi),*tmpii2;
  switch (tmpii->ic_code) {
    case IC_NOT:
      tmpi->ic_code=IC_BR_ZERO;
      tmpi->ic_class=tmpii->ic_class;
      tmpi->ic_flags|=tmpii->ic_flags;
      tmpi->t.arg1_class=tmpii->t.arg1_class;
      tmpi->t.arg1_tree=tmpii->t.arg1_tree;
      OptFree(tmpii);
      return OptBrZero(cc,tmpi);
    case IC_EQU_EQU...IC_LESS_EQU:
      tmpi->ic_code=tmpii->ic_code+IC_BR_EQU_EQU-IC_EQU_EQU;
      break;
    case IC_OR_OR:
      tmpi->ic_code=IC_BR_OR_OR_NOT_ZERO;
      break;
    case IC_AND_AND:
      tmpi->ic_code=IC_BR_AND_AND_NOT_ZERO;
      break;
    case IC_AND:
      tmpi->ic_code=IC_BR_AND_NOT_ZERO;
      break;
    case IC_MM_:
      if (cc->pass==2 && !(tmpii->ic_flags&ICF_RES_TO_F64) &&
	    tmpii->ic_class->raw_type!=RT_F64)
	tmpi->ic_code=IC_BR_MM_NOT_ZERO;
      break;
    case IC_CALL_END:
      tmpii2=OptLag(tmpii);
      switch (tmpii2->ic_code) {
	start:
	  case IC_CARRY:
	    tmpii2->ic_code=IC_BR_CARRY;
	    break;
	  case IC_BT:
	    tmpii2->ic_code=IC_BR_BT;
	    break;
	  case IC_LBTS:
	    tmpii2->ic_flags|=ICF_LOCK;
	  case IC_BTS:
	    tmpii2->ic_code=IC_BR_BTS;
	    break;
	  case IC_LBTR:
	    tmpii2->ic_flags|=ICF_LOCK;
	  case IC_BTR:
	    tmpii2->ic_code=IC_BR_BTR;
	    break;
	  case IC_LBTC:
	    tmpii2->ic_flags|=ICF_LOCK;
	  case IC_BTC:
	    tmpii2->ic_code=IC_BR_BTC;
	    break;
	end:
	  tmpii2->ic_data=tmpi->ic_data;
	  tmpii->ic_code=IC_CALL_END2;
	  OptSetNOP1(tmpi);
	  return tmpii;
      }
      break;
  }
  if (tmpi->ic_code!=IC_BR_NOT_ZERO) {
    tmpi->ic_class=tmpii->ic_class;
    tmpi->ic_flags|=tmpii->ic_flags;
    tmpi->t.arg1_class=tmpii->t.arg1_class;
    tmpi->t.arg2_class=tmpii->t.arg2_class;
    tmpi->t.arg1_tree=tmpii->t.arg1_tree;
    tmpi->t.arg2_tree=tmpii->t.arg2_tree;
    OptFree(tmpii);

    if (tmpi->ic_flags&ICF_PUSH_CMP &&
	  IC_BR_NOT_EQU<=tmpi->ic_code<=IC_BR_LESS_EQU &&
	  !(tmpi->ic_flags&ICF_USE_F64)) {
      tmpi->ic_code+=IC_BR_EQU_EQU2-IC_BR_EQU_EQU;
      tmpi->ic_flags&=~ICF_PUSH_CMP;
      tmpii=tmpi->next; //IC_PUSH_CMP inst
      while (tmpii->ic_code!=IC_PUSH_CMP)
	tmpii=tmpii->next;
      tmpii->t.arg1_tree=tmpi;
      OptSetNOP1(tmpii);
    }

    lb_true=tmpi->ic_data;
    if (tmpi->ic_code==IC_BR_OR_OR_NOT_ZERO) {
      tmpii=tmpi->t.arg1_tree->next;
      tmpii->t.arg1_tree=tmpi->t.arg1_tree;
      tmpii->t.arg1_class=tmpi->t.arg1_class;
      tmpii->ic_data=lb_true;
      tmpii->ic_code=IC_BR_NOT_ZERO;
      OptBrNotZero(cc,tmpii);
      tmpii=tmpi->t.arg2_tree->next;
      tmpii->t.arg1_tree=tmpi->t.arg2_tree;
      tmpii->t.arg1_class=tmpi->t.arg2_class;
      tmpii->ic_data=lb_true;
      tmpii->ic_code=IC_BR_NOT_ZERO;
      tmpii=OptBrNotZero(cc,tmpii);
      OptSetNOP1(tmpi);
    } else if (tmpi->ic_code==IC_BR_AND_AND_NOT_ZERO) {
      lb_false=COCMiscNew(cc,CMT_LABEL);
      tmpi->ic_code=IC_LABEL;
      tmpi->ic_flags=0;
      tmpi->ic_data=lb_false;
      tmpii=tmpi->t.arg1_tree->next;
      tmpii->t.arg1_tree=tmpi->t.arg1_tree;
      tmpii->t.arg1_class=tmpi->t.arg1_class;
      tmpii->ic_data=lb_false;
      tmpii->ic_code=IC_BR_ZERO;
      OptBrZero(cc,tmpii);
      tmpii=tmpi->t.arg2_tree->next;
      tmpii->t.arg1_tree=tmpi->t.arg2_tree;
      tmpii->t.arg1_class=tmpi->t.arg2_class;
      tmpii->ic_data=lb_true;
      tmpii->ic_code=IC_BR_NOT_ZERO;
      tmpii=OptBrNotZero(cc,tmpii);
    } else
      tmpii=tmpi;
    if (tmpi->ic_flags&ICF_POP_CMP && tmpi->t.arg1_tree->ic_code==IC_NOP1) {
      tmpi->t.arg1_tree=tmpi->t.arg1_tree->t.arg1_tree;
      tmpi->ic_flags&=~ICF_POP_CMP;
    }
    return tmpii;
  }
  return tmpi;
}

U0 OptFixSizeOf(CIntermediateCode *tmpi1,
	CIntermediateCode *tmpi_push,CHashClass *tmpcc)
{
  if (tmpi1->ic_code==IC_MUL && tmpi1->t.arg2_tree->ic_code==IC_SIZEOF) {
    tmpi1->t.arg2_tree->ic_code=IC_IMM_I64;
    tmpi1->t.arg2_tree->ic_class=tmpcc;
    tmpi_push->ic_class=tmpcc;
    if (tmpcc->ptr_stars_cnt) {
      tmpcc--;
      if (tmpcc->size==1)
	goto here;
      tmpi1->t.arg2_tree->ic_data=tmpcc->size;
    } else {
here:
      if (tmpi_push==tmpi1)
	tmpi1->t.arg2_tree->ic_data=1;
      else {
	OptSetNOP1(tmpi1->t.arg2_tree);
	OptSetNOP1(tmpi1);
      }
    }
  }
}

I64 CmpRawType(CHashClass *tmpc)
{
  if (tmpc) {
    tmpc=OptClassFwd(tmpc);
    return tmpc->raw_type;
  }
  return 0;
}

I64 CmpRawTypePointed(CHashClass *tmpc)
{
  if (tmpc) {
    if (tmpc->ptr_stars_cnt)
      tmpc--;
    tmpc=OptClassFwd(tmpc);
    return tmpc->raw_type;
  }
  return 0;
}

U0 CmpMinTypePointed(CIntermediateCode *tmpi,I64 pt1)
{
  I64 pt;
  if ((pt=tmpi->arg1_type_pointed_to) && pt!=RT_F64 && 0<pt1<pt)
    tmpi->arg1_type_pointed_to=pt;
}

U0 CmpF1PushPop(CIntermediateCode *tmpi,CIntermediateCode *tmpi2)
{
  if (intermediate_code_table[tmpi2->ic_code].fpop||
	tmpi2->ic_flags&ICF_RES_TO_F64)
    Bts(&tmpi->ic_flags,ICf_DONT_PUSH_FLOAT0);
}

U0 CmpF2PushPop(CIntermediateCode *tmpi,
	CIntermediateCode *tmpi1,CIntermediateCode *tmpi2)
{
  if ((tmpi2->ic_code==IC_MOV || tmpi2->ic_code==IC_IMM_F64) &&
	!(tmpi2->ic_flags&ICF_RES_TO_F64) &&
	(intermediate_code_table[tmpi1->ic_code].fpop ||
	tmpi1->ic_flags&ICF_RES_TO_F64))
    Bts(&tmpi->ic_flags,ICf_DONT_PUSH_FLOAT0);
  else if ((intermediate_code_table[tmpi2->ic_code].fpop ||
	tmpi2->ic_flags&ICF_RES_TO_F64)&&
	!(tmpi1->ic_flags&ICF_RES_TO_F64))
    Bts(&tmpi->ic_flags,ICf_DONT_PUSH_FLOAT0);
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
