U0 ParenWarning(CLex *lx)
{
  if (Bt(&lx->opts,OPTf_WARN_PAREN) &&
      !(lx->cur_lfn->flags&LFSF_DEFINE))
    LexWarn(lx,"Unnecessary parenthesis ");
}

U0 PrsPush(CPrsStk *ps,I64 val)
{
  ps->stk[++ps->ptr]=val;
}

I64 PrsPop(CPrsStk *ps)
{
  return ps->stk[ps->ptr--];
}

U0 PrsPush2(CPrsStk *ps,I64 val)
{
  ps->stk2[++ps->ptr2]=val;
}

I64 PrsPop2(CPrsStk *ps)
{
  return ps->stk2[ps->ptr2--];
}

U0 PrsPopDeref(CPrsStk *ps)
{
  I64 i=PrsPop(ps);
  CHashClass *tempc=PrsPop(ps);
  if (i.u16[0]!=IC_DEREF) {
    PrsPush(ps,tempc);
    PrsPush(ps,i);
  }
}

#define PE_UNARY_TERM1		0
#define PE_UNARY_TERM2		1
#define PE_MAYBE_MODIFIERS	2
#define PE_UNARY_MODIFIERS	3
#define PE_DEREFERENCE		4
#define PE_CHECK_BINARY_OPS1	5
#define PE_CHECK_BINARY_OPS2	6
#define PE_DO_UNARY_OP		7
#define PE_DO_BINARY_OP		8
#define PE_POP_HIGHER		9
#define PE_PUSH_LOWER		10
#define PE_POP_ALL1		11
#define PE_POP_ALL2		12

CIntermediateCode *PrsAddOp(CLex *lx,I64 stk_op,CHashClass *tempc)
{
  CIntermediateCode *tempi=lx->lcc.lcc_root.last;
  Bool div_sizeof=FALSE;
  switch (stk_op.u16[0]) {
    case IC_ADD:
      if (tempc->ptr_cnt && !tempi->ic_class->ptr_cnt &&
	    tempi->ic_class->raw_type!=RT_F64) {
	LCCAdd(lx,IC_SIZEOF,1,cmp.internal_types[RT_I64]);
	LCCAdd(lx,IC_MUL,0,cmp.internal_types[RT_I64]);
      }
      break;
    case IC_SUB:
      if (tempc->ptr_cnt && tempi->ic_class->raw_type!=RT_F64) {
	if (!tempi->ic_class->ptr_cnt) {
	  LCCAdd(lx,IC_SIZEOF,1,cmp.internal_types[RT_I64]);
	  LCCAdd(lx,IC_MUL,0,cmp.internal_types[RT_I64]);
	} else
	  div_sizeof=TRUE;
      }
      break;
    case IC_AND_AND:
    case IC_OR_OR:
      LCCAdd(lx,IC_NOP1,0,cmp.internal_types[RT_I64]);
      break;
    case IC_ADD_EQUAL:
    case IC_SUB_EQUAL:
      if (tempc->ptr_cnt) {
	LCCAdd(lx,IC_SIZEOF,1,cmp.internal_types[RT_I64]);
	LCCAdd(lx,IC_MUL,0,cmp.internal_types[RT_I64]);
      }
      break;
  }
  tempi=LCCAdd(lx,stk_op,0,tempc);
  if (stk_op.u8[3]&ECF_HAS_PUSH_CMP) {
    tempi->ic_flags|=ICF_POP_CMP;
    LCCAdd(lx,IC_NOP1,0,cmp.internal_types[RT_I64]);
    LCCAdd(lx,IC_AND_AND,0,cmp.internal_types[RT_I64],ICF_POP_CMP);
  }
  if (div_sizeof) {
    tempc--;
    if (tempc->size!=1) {
      LCCAdd(lx,IC_IMM_U64,tempc->size,cmp.internal_types[RT_I64]);
      LCCAdd(lx,IC_DIV,0,cmp.internal_types[RT_I64]);
      tempc=cmp.internal_types[RT_I64];
    }
  }
  return tempi;
}

U0 PrsExpression2(CLex *lx,I64 *_max_prec,CPrsStk *ps)
{
  I64 i,cur_op,stk_op,state,max_prec=PREC_NULL,unary_pre_prec,paren_prec,
	unary_post_prec,left_prec=PREC_MAX;
  CIntermediateCode *tempi;
  CHashClass *tempc;
  CMemberLst *local_var;
  CArrayDim *tempad=NULL;

  goto pe_unary_term1;
  while (TRUE) {
    nobound_switch (state) {
      case PE_UNARY_TERM1:
pe_unary_term1:
	unary_pre_prec=PREC_NULL;
	unary_post_prec=PREC_NULL;
	lx->flags&=~(LF_PAREN+LF_PREINC+LF_PREDEC+LF_POSTINC+LF_POSTDEC+LF_FUN_EXP);
      case PE_UNARY_TERM2:
	state=PrsUnaryTerm(lx,ps,&local_var,&tempad,&max_prec,&unary_pre_prec,&paren_prec);
	break;
      case PE_UNARY_MODIFIERS:
	state=PrsUnaryModifier(lx,ps,&local_var,&tempad,&unary_post_prec);
	break;
      case PE_MAYBE_MODIFIERS:
	if (lx->token=='(') {
	  lx->flags|=LF_RAX;
	  state=PrsUnaryModifier(lx,ps,&local_var,&tempad,&unary_post_prec);
	} else
	  goto pe_check_binary_ops1;
	break;
      case PE_DEREFERENCE:
	if (!(lx->flags&(LF_PREINC|LF_PREDEC|LF_POSTINC|LF_POSTDEC)))
	  i=IC_DEREF+PREC_UNARY_PRE<<16;
	else {
	  if (lx->flags & LF_POSTINC)
	    i=IC__PP+PREC_UNARY_POST<<16;
	  else if (lx->flags & LF_POSTDEC)
	    i=IC__MM+PREC_UNARY_POST<<16;
	  else if (lx->flags & LF_PREDEC)
	    i=IC_MM_+PREC_UNARY_PRE<<16;
	  else
	    i=IC_PP_+PREC_UNARY_PRE<<16;
	  lx->flags&=~(LF_PREINC|LF_PREDEC|LF_POSTINC|LF_POSTDEC);
	}
	tempi=lx->lcc.lcc_root.last;
	if (lx->flags & (LF_RAX|LF_ARRAY)) {
	  if (tempi->ic_code==IC_DEREF)
	    tempi->ic_code=i;
	} else {
	  tempc=OptClassFwd(tempi->ic_class-1);
	  LCCAdd(lx,i,0,tempc);
	}
      case PE_CHECK_BINARY_OPS1:
pe_check_binary_ops1:
	if (paren_prec) {
	  if (unary_pre_prec || unary_post_prec) {
	    if (paren_prec<=unary_pre_prec && !unary_post_prec)
	      ParenWarning(lx);
	    paren_prec=PREC_NULL;
	  } else if (paren_prec<=PREC_UNARY_PRE+ASSOC_MASK)
	    ParenWarning(lx);
	}
	cur_op=cmp.binary_ops[lx->token];

      case PE_CHECK_BINARY_OPS2:
pe_check_binary_ops2:
	stk_op=PrsPop(ps);
	tempc=PrsPop(ps);
	if (!(0<stk_op.u8[2]<=PREC_UNARY_PRE+ASSOC_MASK))
	  goto pe_do_binary_op;

      case PE_DO_UNARY_OP:
	if (cur_op.u16[0]==IC_POWER &&
	      stk_op.u16[0]==IC_UNARY_MINUS) {
	  Lex(lx); //skip ` op
	  left_prec=cur_op.i8[2];
	  PrsPush(ps,tempc);
	  PrsPush(ps,stk_op);
	  PrsPush(ps,lx->lcc.lcc_root.last->ic_class);
	  PrsPush(ps,cur_op);
	  goto pe_unary_term1;
	} else {
	  tempi=lx->lcc.lcc_root.last;
	  tempc=tempi->ic_class;
	  if (stk_op.u16[0]==IC_DEREF && tempc->ptr_cnt)
	    tempc--;
	  else if (stk_op.u16[0]==IC_ADDR) {
	    lx->abs_cnts.c_addres++;
	    if (Bt(cmp.ic_bitmap_deref,tempi->ic_code))
	      OptFree(tempi);
	    tempc++;
	  }
	  tempc=OptClassFwd(tempc);
	  if (stk_op)
	    LCCAdd(lx,stk_op,0,tempc);
	  goto pe_check_binary_ops2;
	}

      case PE_DO_BINARY_OP:
pe_do_binary_op:
	PrsPush(ps,tempc);
	PrsPush(ps,stk_op);
	if (!cur_op)
	  goto pe_pop_all1;

	switch (cur_op.u16[0]) {
	  case IC_ADD:
	  case IC_SUB:
	    tempi=lx->lcc.lcc_root.last;
	    if (!tempi->ic_class->ptr_cnt &&
		  tempi->ic_class->raw_type!=RT_F64) {
	      LCCAdd(lx,IC_SIZEOF,1,cmp.internal_types[RT_I64]);
	      LCCAdd(lx,IC_MUL,0,cmp.internal_types[RT_I64]);
	    }
	    break;
	  case IC_AND_AND:
	  case IC_OR_OR:
	    LCCAdd(lx,IC_NOP1,0,cmp.internal_types[RT_I64]);
	    break;
	}
	if (lx->flags & LF_FUN_EXP) {
	  ps->ptr2--;
	  lx->flags&=~LF_FUN_EXP;
	}
	Lex(lx); //skip op
	if (paren_prec>PREC_UNARY_PRE+ASSOC_MASK &&
	      paren_prec&~ASSOC_MASK<left_prec&~ASSOC_MASK+
	      paren_prec&ASSOCF_RIGHT &&
	      paren_prec&~ASSOC_MASK<cur_op.u8[2]&~ASSOC_MASK+
	      !(paren_prec&ASSOCF_RIGHT))
	  ParenWarning(lx);
	if (cur_op.u8[2]>max_prec)
	  max_prec=cur_op.u8[2];
	left_prec=cur_op.u8[2];
	if (Bt(cmp.ic_bitmap_assign,cur_op.u16[0])) {
	  tempi=lx->lcc.lcc_root.last;
	  tempc=OptClassFwd(tempi->ic_class);
	  if (!Bt(cmp.ic_bitmap_deref,tempi->ic_code) ||
		!tempc->ptr_cnt && !Bt(&tempc->flags,Cf_INTERNAL_TYPE))
	    LexExcept(lx,"invalid lval at ");
	  tempi->ic_code=IC_NOP1; //Important for setting class (pretty sure)
	  cur_op.u8[2]=PREC_ASSIGN|ASSOCF_RIGHT;
	}

      case PE_POP_HIGHER:
pe_pop_higher:
	stk_op=PrsPop(ps); //pop ops of higher prec
	tempc=PrsPop(ps);
	if (!stk_op)
	  goto pe_push_lower;
	else if (cur_op.u8[2]&~ASSOC_MASK==stk_op.u8[2]&~ASSOC_MASK) {
	  if (cur_op.u8[2]&ASSOCF_RIGHT)
	    goto pe_push_lower;
	} else if (cur_op.u8[2]&~ASSOC_MASK<=stk_op.u8[2]&~ASSOC_MASK)
	  goto pe_push_lower;

	tempi=PrsAddOp(lx,stk_op,tempc);

	if (Bt(cmp.ic_bitmap_cmp,cur_op.u16[0]) && Bt(cmp.ic_bitmap_cmp,stk_op.u16[0])) {
	  tempi->ic_flags|=ICF_PUSH_CMP;
	  LCCAdd(lx,IC_NOP1,0,cmp.internal_types[RT_I64]);
	  LCCAdd(lx,IC_PUSH_CMP,0,tempc);
	  cur_op.u8[3]|=ECF_HAS_PUSH_CMP;
	} else if (cur_op.u16[0]==IC_AND_AND || cur_op.u16[0]==IC_OR_OR)
	  LCCAdd(lx,IC_NOP1,0,cmp.internal_types[RT_I64]);
	goto pe_pop_higher;

      case PE_PUSH_LOWER:
pe_push_lower:
	PrsPush(ps,tempc);
	PrsPush(ps,stk_op);
	PrsPush(ps,lx->lcc.lcc_root.last->ic_class);
	PrsPush(ps,cur_op);
	goto pe_unary_term1;

      case PE_POP_ALL1:
pe_pop_all1:
	if (paren_prec>PREC_UNARY_PRE+ASSOC_MASK &&
	      paren_prec&~ASSOC_MASK<=left_prec&~ASSOC_MASK-
	      paren_prec&ASSOCF_LEFT-left_prec&ASSOCF_LEFT)
	  ParenWarning(lx);
      case PE_POP_ALL2:
pe_pop_all2:
	stk_op=PrsPop(ps);
	tempc=PrsPop(ps);
	if (!stk_op.u16[0])
	  goto pe_done;
	PrsAddOp(lx,stk_op,tempc);
	goto pe_pop_all2;
    }
  }
pe_done:
  if (_max_prec)
    *_max_prec=max_prec;
}

Bool PrsExpression(CLex *lx,I64 *_max_prec,Bool end_exp,CPrsStk *_ps=NULL)
{
  Bool result=TRUE;
  I64 old_flags=lx->flags;
  CPrsStk *ps;
  if (_ps)
    ps=_ps;
  else {
    ps=MAlloc(sizeof(CPrsStk));
    ps->ptr=0;
    ps->ptr2=0;
  }
  PrsPush(ps,0); //terminate
  PrsPush(ps,0); //terminate
  try
    PrsExpression2(lx,_max_prec,ps); //try catch causes noreg vars in function
  catch {
    if (Fs->except_ch=='Compiler') {
      result=FALSE;
      Fs->catch_except=TRUE;
    }
  }
  if (!_ps) {
    if (ps->ptr)
      LexExcept(lx,"Compiler Parse Error at ");
    Free(ps);
  }
  if (end_exp && result)
    LCCAdd(lx,IC_END_EXP,0,0);
  lx->flags|=old_flags&(LF_PREINC|LF_PREDEC);
  return result;
}

U0 PrsSizeOf(CLex *lx)
{
  CHashClass *tempc;
  CMemberLst *tempm;
  CDbgInfo *dbg_info;
  I64 i;
  if (lx->token!=TK_IDENT)
    LexExcept(lx,"invalid class at ");
  if (tempm=lx->local_var_entry) {
    tempc=tempm->member_class;
    i=tempc->size*tempm->dim.total_cnt;
    if (Lex(lx)=='.')
      goto pu_sizeof_member;
  } else {
    if (!(tempc=lx->hash_entry) || !(tempc->type &
	(HTT_CLASS|HTT_INTERNAL_TYPE|HTT_GLBL_VAR|HTT_FUN|HTT_EXPORT_SYS_SYM)))
      LexExcept(lx,"invalid class at ");
    if (tempc->type&(HTT_FUN|HTT_EXPORT_SYS_SYM)) {
      if (!(dbg_info=tempc(CHashFun *)->dbg_info))
	LexExcept(lx,"Size not defined at ");
      i=dbg_info->body[dbg_info->max_line+1-dbg_info->min_line]-dbg_info->body[0];
      Lex(lx);
    } else {
      i=tempc->size;
      while (Lex(lx)=='.') {
pu_sizeof_member:
	if (!(tempc->type & (HTT_CLASS|HTT_GLBL_VAR)))
	  LexExcept(lx,"invalid class at ");
	else if (tempc->type & HTT_GLBL_VAR)
	  tempc=tempc(CHashGlblVar *)->var_class;
	if (Lex(lx)!=TK_IDENT || !(tempm=MemberFind(lx->cur_str,tempc)))
	  LexExcept(lx,"invalid member at ");
	else if (lx->local_var_entry)
	  lx->local_var_entry->use_cnt--;
	tempc=tempm->member_class;
//Probably others like this:
#assert offset(CHashClass.size)==offset(CHashGlblVar.size)
	i=tempc->size*tempm->dim.total_cnt;
      }
    }
  }
  if (lx->token=='*') {
    while (Lex(lx)=='*');
    i=sizeof(U8 *);
  }
  LCCAdd(lx,IC_IMM_U64,i,cmp.internal_types[RT_I64]);
}

U0 PrsOffsetOf(CLex *lx)
{
  CHashClass *tempc;
  CMemberLst *tempm;
  I64 i;
  if (lx->token!=TK_IDENT)
    LexExcept(lx,"invalid class at ");
  if (tempm=lx->local_var_entry)
    tempc=tempm->member_class;
  else {
    tempc=lx->hash_entry;
    if (!tempc || !(tempc->type & (HTT_CLASS|HTT_GLBL_VAR)))
      LexExcept(lx,"invalid class at ");
    else if (tempc->type & HTT_GLBL_VAR)
      tempc=tempc(CHashGlblVar *)->var_class;
  }
  if (Lex(lx)!='.')
    LexExcept(lx,"expecting '.' at ");
  i=0;
  do {
    if (Lex(lx)!=TK_IDENT || !(tempm=MemberFind(lx->cur_str,tempc)))
      LexExcept(lx,"invalid member at ");
    else if (lx->local_var_entry)
      lx->local_var_entry->use_cnt--;
    i+=tempm->offset;
    tempc=tempm->member_class;
  } while (Lex(lx)=='.');
  LCCAdd(lx,IC_IMM_U64,i,cmp.internal_types[RT_I64]);
}

I64 PrsFunCall(CLex *lx,CPrsStk *ps,Bool indirect,CHashFun *tempf)
{
  I64 i,argc_cnt,dft_val;
  Bool is_first_arg=TRUE,needs_right_paren,is_str_out,is_putchars,is_template_fun;
  CHashClass *tempc2,*last_class=NULL;
  CMemberLst *tempm;
  CLexCodeCtrl *tempcbh,*tempcbh1;
  CLexCodeMisc *mc;
  CIntermediateCode *tempi;

  if (!tempf) {
    if (lx->token==TK_CHAR_CONST) {
      if (!(tempf=HashFind("PutChars",lx->htc.hash_table_lst,HTT_FUN)))
	LexExcept(lx,"Missing header for Print() and PutChars() at ");
      if (!lx->cur_i64) //empty char signals PutChars with variable
	Lex(lx);
      is_str_out=FALSE;
      is_putchars=TRUE;
    } else {
      if (!(tempf=HashFind("Print",lx->htc.hash_table_lst,HTT_FUN)))
	LexExcept(lx,"Missing header for Print() and PutChars() at ");
      if (!*lx->cur_str) //empty string signals Print with variable fmt_str
	Lex(lx);
      is_putchars=FALSE;
      is_str_out=TRUE;
    }
  } else {
    is_str_out=FALSE;
    is_putchars=FALSE;
  }

  if (Bt(&tempf->flags,Ff_INTERNAL)&& IC_SQR<=tempf->executable_addr<=IC_ATAN)
    is_template_fun=TRUE;
  else
    is_template_fun=FALSE;

  if (indirect) {
    if (!(lx->flags & (LF_RAX | LF_ARRAY)))
      LCCAdd(lx,IC_DEREF,0,cmp.internal_types[RT_PTR]);
    lx->lcc.lcc_root.last->ic_class=cmp.internal_types[RT_PTR];
    LCCAdd(lx,IC_SET_RAX,0,cmp.internal_types[RT_PTR]);
    LCCAdd(lx,IC_NOP2,1,cmp.internal_types[RT_PTR]); //balance the books
  }
  LCCPush(lx);
  tempcbh=lx->lcc.lcc_next;
  lx->lcc.lcc_next=NULL;
  i=tempf->arg_cnt;
  tempm=tempf->member_lst;
  argc_cnt=0;
  if (lx->token=='(') {
    Lex(lx);
    needs_right_paren=TRUE;
  } else
    needs_right_paren=FALSE;
  while (i--) {
    LCCInit(lx);
    if (!is_first_arg) {
      if (is_str_out) {
	if (lx->token==',')
	  Lex(lx);
	else if (lx->token!=';')
	  LexExcept(lx,"expecting ',' at ");
      } else {
	if (needs_right_paren) {
	  if (lx->token==',')
	    Lex(lx);
	  else if (lx->token!=')')
	    LexExcept(lx,"expecting ',' at ");
	}
      }
    }
    if (tempm->flags & MLF_DFT_AVAILABLE &&
	(lx->token==')' || lx->token==',' || !needs_right_paren)) {
      dft_val=tempm->dft_val;
      if (tempm->flags & MLF_LASTCLASS && last_class)
	dft_val=(last_class-last_class->ptr_cnt)->str;
      if (tempm->flags & (MLF_STR_DFT_AVAILABLE|MLF_LASTCLASS) &&
	  lx->flags&LF_AOT_COMPILE) {
	mc=LCCMiscNew(lx,CMT_STR_CONST);
	LCCAdd(lx,IC_STR_CONST,mc,cmp.internal_types[RT_U8]+1);
	mc->st_len=StrLen(dft_val)+1;
	mc->str=StrNew(dft_val);
	lx->flags|=LF_HAS_MISC_DATA;
      } else
	LCCAdd(lx,IC_IMM_U64,dft_val,tempm->member_class);
    } else {
      if (!PrsExpression(lx,NULL,FALSE,ps))
	throw('Compiler');
      else {
	LCCPush(lx);
	lx->pass=0;	  OptPass012(lx);
	lx->pass=1; tempi=OptPass012(lx);
	LCCPop(lx);
	last_class=OptClassFwd(tempi->ic_class);
	tempc2=OptClassFwd(tempm->member_class);
	if (tempc2->raw_type==RT_F64 && !tempm->dim.next &&
	    last_class->raw_type!=RT_F64)
	  tempi->ic_flags|=ICF_R_TO_F64;
	else
	if (tempc2->raw_type!=RT_F64 &&
	    last_class->raw_type==RT_F64)
	  tempi->ic_flags|=ICF_R_TO_INT;
      }
    }
    LCCPush(lx);
    is_first_arg=FALSE;
    tempm=tempm->next;
  }
  if (tempm && tempm->flags & MLF_DOT_DOT_DOT) {
    LCCInit(lx);
    tempi=LCCAdd(lx,IC_IMM_U64,0,tempm->member_class);
    LCCPush(lx);
    if (is_str_out) {
      if (lx->token!=';') {
	do {
	  if (!is_first_arg) {
	    if (lx->token==',')
	      Lex(lx);
	    else
	      LexExcept(lx,"expecting ',' at ");
	  }
	  LCCInit(lx);
	  if (!PrsExpression(lx,NULL,FALSE,ps))
	    throw('Compiler');
	  LCCPush(lx);
	  is_first_arg=FALSE;
	  argc_cnt++;
	} while (lx->token==',');
      }
    } else if (needs_right_paren) {
      if (lx->token!=')') {
	do {
	  if (!is_first_arg) {
	    if (lx->token==',')
	      Lex(lx);
	    else
	      LexExcept(lx,"expecting ',' at ");
	  }
	  LCCInit(lx);
	  if (!PrsExpression(lx,NULL,FALSE,ps))
	    throw('Compiler');
	  LCCPush(lx);
	  is_first_arg=FALSE;
	  argc_cnt++;
	} while (lx->token==',');
      }
    }
    tempi->ic_data=argc_cnt++; //++ so add_esp latter works
  }
  if (needs_right_paren) {
    if (lx->token==')')
      Lex(lx);
    else
      LexExcept(lx,"missing ')' at ");
  }
  tempcbh1=tempcbh->lcc_next;
  tempcbh->lcc_next=lx->lcc.lcc_next;
  lx->lcc.lcc_next=tempcbh;
  LCCPop(lx);
  tempcbh=lx->lcc.lcc_next;
  lx->lcc.lcc_next=tempcbh1;
  if (!is_template_fun)
    LCCAdd(lx,IC_CALL_START,tempf,0);
  if (indirect)
    LCCAdd(lx,IC_PUSH_REGS,1<<REG_RAX,tempf->return_class);
  while (tempcbh) {
    tempcbh1=tempcbh->lcc_next;
    LCCAppend(lx,tempcbh);
    if (!Bt(&tempf->flags,Ff_INTERNAL))
      lx->lcc.lcc_root.last->ic_flags|=ICF_PUSH_RESULT;
    tempcbh=tempcbh1;
  }
  if (Bt(&tempf->flags,Ff_INTERNAL))
    LCCAdd(lx,tempf->executable_addr,0,tempf->return_class);
  else {
    if (indirect)
      LCCAdd(lx,IC_CALL_INDIRECT,(argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
    else if (Bt(&tempf->flags,Cf_EXTERN)) {
      lx->abs_cnts.externs++;
      if (lx->flags&LF_AOT_COMPILE) {
	if (tempf->type&HTF_IMPORT)
	  LCCAdd(lx,IC_CALL_IMPORT,tempf,tempf->return_class);
	else
	  LCCAdd(lx,IC_CALL_EXTERN,tempf,tempf->return_class);
      } else
	LCCAdd(lx,IC_CALL_INDIRECT2,&tempf->executable_addr,tempf->return_class);
    } else
      LCCAdd(lx,IC_CALL,tempf->executable_addr,tempf->return_class);
    if ((Bt(&tempf->flags,Ff_RET1) || Bt(&tempf->flags,Ff_ARGPOP)) && !Bt(&tempf->flags,Ff_NOARGPOP)) {
      if (indirect) {
	LCCAdd(lx,IC_ADD_RSP1,(argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
	LCCAdd(lx,IC_ADD_RSP,8,tempf->return_class);
      } else
	LCCAdd(lx,IC_ADD_RSP1,(argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
    } else {
      if (indirect)
	argc_cnt++;
      LCCAdd(lx,IC_ADD_RSP,(argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
    }
  }
  if (!is_template_fun)
    LCCAdd(lx,IC_CALL_END,tempf,tempf->return_class);
  if (is_str_out||is_putchars)
    LCCAdd(lx,IC_END_EXP,0,0);
  lx->flags=(lx->flags|LF_RAX) & ~(LF_ARRAY|LF_FUN_EXP);
  return PE_UNARY_MODIFIERS;
}

I64 PrsUnaryTerm(CLex *lx,CPrsStk *ps,CMemberLst **_local_var,CArrayDim **_tempad,
    I64 *max_prec,I64 *unary_pre_prec,I64 *paren_prec)
{
  I64 i,j;
  CHashExport *tempex;
  CHashClass *tempc;
  CHashFun *tempf;
  CHashGlblVar *tempg;
  CMemberLst *tempm;
  CLexUndef *templu;
  CLexCodeMisc *mc;
  Bool paren_warn;

  *_local_var=NULL;
  *paren_prec=PREC_NULL;
  switch (lx->token) {
    sub_switch_start:
      if (PREC_UNARY_PRE>*max_prec)
	*max_prec=PREC_UNARY_PRE;
      *unary_pre_prec=PREC_UNARY_PRE;
      sub_switch_start:
	case '~': i=IC_COM;		break;
	case '!': i=IC_NOT;		break;
	case '-': i=IC_UNARY_MINUS;	break;
	case '*': i=IC_DEREF;		break;
      sub_switch_end:
	Lex(lx); //Skip op
	break;
      case '&':
	if (Lex(lx)==TK_IDENT) {
	  if (tempc=lx->hash_entry) {
	    if (tempc->type & HTT_FUN) {
	      tempf=tempc;
	      if (!Bt(&tempf->flags,Ff_INTERNAL)) {
		if (Bt(&tempf->flags,Cf_EXTERN)) {
		  if (lx->flags&LF_AOT_COMPILE) {
		    if (lx->flags&LF_ASM_EXPRESSIONS) {
		      if (tempex=HashFind(tempf->str,lx->htc.hash_table_lst,HTT_EXPORT_SYS_SYM))
			goto pu_export_sys_sym;
		      else
			goto pu_new_sys_sym;
		    }
		    LexExcept(lx,"Can't take addr of extern fun");
		  }
		  lx->abs_cnts.externs++;
		  LCCAdd(lx,IC_IMM_U64,
			&tempf->executable_addr,cmp.internal_types[RT_PTR]);
		  LCCAdd(lx,IC_DEREF,0,
			cmp.internal_types[RT_PTR]);
		} else {
		  if (lx->flags&LF_AOT_COMPILE) {
		    LCCAdd(lx,IC_ABS_ADDR,
			  tempf->executable_addr,cmp.internal_types[RT_PTR]);
		    if (lx->flags&LF_ASM_EXPRESSIONS)
		      lx->abs_cnts.abs_addres++;
		  } else
		    LCCAdd(lx,IC_IMM_U64,
			  tempf->executable_addr,cmp.internal_types[RT_PTR]);
		}
		lx->abs_cnts.c_addres++;
		Lex(lx);
		return PE_MAYBE_MODIFIERS;
	      }
	    } else if (tempc->type & HTT_EXPORT_SYS_SYM) {
	      tempex=tempc;
	      if (lx->flags&LF_ASM_EXPRESSIONS && !(lx->flags&LF_AOT_COMPILE) &&
		    tempex->type&HTF_IMM) {
		lx->abs_cnts.c_addres++;
		LCCAdd(lx,IC_IMM_U64,
		      tempex->val,cmp.internal_types[RT_PTR]);
		Lex(lx);
		return PE_MAYBE_MODIFIERS;
	      } else
		goto pu_export_sys_sym;
	    }
	  } else if (lx->flags&LF_ASM_EXPRESSIONS && !lx->local_var_entry)
	    goto pu_ident_but_not_local_var;
	}
	i=IC_ADDR;
	break;
    sub_switch_end:
      PrsPush(ps,tempc);
      PrsPush(ps,PREC_UNARY_PRE<<16+i);
      return PE_UNARY_TERM2;

    sub_switch_start:
      case TK_INTEGER:
      case TK_CHAR_CONST:
      case TK_INS_BINARY_SIZE:
	if (lx->cur_i64<0)
	  LCCAdd(lx,IC_IMM_U64,lx->cur_i64,cmp.internal_types[RT_U64]);
	else
	  LCCAdd(lx,IC_IMM_U64,lx->cur_i64,cmp.internal_types[RT_I64]);
	Lex(lx);
	break;
      case TK_F64:
	LCCAdd(lx,IC_IMM_F64,lx->cur_f64(I64),cmp.internal_types[RT_F64]);
	Lex(lx);
	break;
      case TK_STR:
	mc=LCCMiscNew(lx,CMT_STR_CONST);
	LCCAdd(lx,IC_STR_CONST,mc,cmp.internal_types[RT_U8]+1);
	mc->str=LexExtStr(lx,&mc->st_len);
	lx->flags|=LF_HAS_MISC_DATA;
	break;
      case TK_INS_BINARY:
	mc=LCCMiscNew(lx,CMT_STR_CONST);
	LCCAdd(lx,IC_STR_CONST,mc,cmp.internal_types[RT_U8]+1);
	mc->str=lx->cur_str;
	mc->st_len=lx->cur_str_len;
	lx->cur_str=NULL;
	lx->flags|=LF_HAS_MISC_DATA;
	Lex(lx);
	break;
      case '$$':
	if (lx->flags & LF_ASM_EXPRESSIONS) {
	  lx->abs_cnts.abs_addres++;
	  if (lx->flags&LF_AOT_COMPILE)
	    LCCAdd(lx,IC_ABS_ADDR,lx->a->ip,cmp.internal_types[RT_PTR]);
	  else
	    LCCAdd(lx,IC_IMM_U64,lx->a->ip,cmp.internal_types[RT_PTR]);
	} else {
	  if (lx->flags&LF_CLASS_IP)
	    LCCAdd(lx,IC_IMM_U64,lx->class_ip,cmp.internal_types[RT_I64]);
	  else
	    LCCAdd(lx,IC_RIP,0,cmp.internal_types[RT_PTR]);
	}
	Lex(lx);
	break;
    sub_switch_end:
      if (PREC_TERM>*max_prec)
	*max_prec=PREC_TERM;
      return PE_MAYBE_MODIFIERS;

    case '(':
      if (Lex(lx)==TK_IDENT && lx->hash_entry && lx->hash_entry->type & (HTT_CLASS|HTT_INTERNAL_TYPE))
	LexExcept(lx,"Use TempleOS postfix typecasting at ");
      else {
	if (PREC_TERM>*max_prec)
	  *max_prec=PREC_TERM;
	if (lx->cur_lfn->flags&LFSF_DEFINE)
	  paren_warn=FALSE;
	else
	  paren_warn=TRUE;
	if (!PrsExpression(lx,paren_prec,FALSE,ps))
	  throw('Compiler');
	if (!paren_warn)
	  *paren_prec=PREC_NULL;
	if (lx->token!=')')
	  LexExcept(lx,"missing ')' at ");
	Lex(lx); //skip )
	lx->flags= lx->flags & ~LF_ARRAY | LF_RAX | LF_PAREN;
	return PE_UNARY_MODIFIERS;
      }

    sub_switch_start:
      case '+': 					break;
      case TK_PLUS_PLUS:   lx->flags|=LF_PREINC;	break;
      case TK_MINUS_MINUS: lx->flags|=LF_PREDEC;	break;
    sub_switch_end:
      if (PREC_UNARY_PRE>*max_prec)
	*max_prec=PREC_UNARY_PRE;
      *unary_pre_prec=PREC_UNARY_PRE;
      Lex(lx);
      return PE_UNARY_TERM2;

    case TK_IDENT:
      if (tempm=lx->local_var_entry) {
	if (PREC_TERM>*max_prec)
	  *max_prec=PREC_TERM;
	lx->flags&=~(LF_RAX|LF_ARRAY|LF_FUN_EXP);
	tempc=tempm->member_class+1;
	if (tempm->flags & MLF_FUN && !(lx->flags&LF_ASM_EXPRESSIONS)) {
	  PrsPopDeref(ps);
	  lx->flags|=LF_FUN_EXP;
	  PrsPush2(ps,tempm->fun_ptr-tempm->fun_ptr->ptr_cnt);
	}
	if (tempm->dim.next) {
	  *_tempad=tempm->dim.next;
	  lx->flags|=LF_ARRAY;
	}
	if (tempm->flags&MLF_STATIC) {
	  if (lx->flags&LF_AOT_COMPILE) {
//	if (tempg->flags&GVF_DATA_HEAP) //TODO
	    //	  LCCAdd(lx,IC_HEAP_GLBL,tempm->static_data,tempc);
	    //	else
	    LCCAdd(lx,IC_ABS_ADDR,tempm->static_data_ip,tempc);
	  } else
	    LCCAdd(lx,IC_IMM_U64,tempm->static_data,tempc);
	} else {
	  if (lx->flags&LF_ASM_EXPRESSIONS) {
	    i=PrsPop(ps);
	    ps->ptr--;
	    if (i.u16[0]!=IC_ADDR)
	      LexExcept(lx,"expecting '&' at ");
	    LCCAdd(lx,IC_IMM_U64,tempm->offset,cmp.internal_types[RT_PTR]);
	    *_local_var=tempm;
	    Lex(lx);
	    return PE_MAYBE_MODIFIERS;
	  } else {
	    if (tempm->dim.next && tempm->offset>0 && StrCmp(tempm->str,"argv")) {
	      tempc++;
	      lx->flags&=~LF_ARRAY;
	    }
	    LCCAdd(lx,IC_RBP,0,tempc);
	    LCCAdd(lx,IC_IMM_U64,tempm->offset,tempc);
	    LCCAdd(lx,IC_ADD,0,tempc);
	  }
	}
	Lex(lx); //skip var name
	*_local_var=tempm;
	return PE_UNARY_MODIFIERS;
      }
pu_ident_but_not_local_var:
      if (!(tempex=lx->hash_entry)) {
	if (!(lx->flags & LF_ASM_EXPRESSIONS))
	  LexExcept(lx,"invalid lval at ");
	tempc=NULL;
pu_new_sys_sym:
	tempex=CAlloc(sizeof(CHashExport),Fs->code_heap);
	tempex->str=lx->cur_str;
	lx->cur_str=NULL;
	if (!lx->htc.local_var_lst &&
	    *tempex->str=='@' && tempex->str[1]=='@') {
	  tempex->type=HTT_EXPORT_SYS_SYM | HTF_UNRESOLVED | HTF_LOCAL;
	  HashAdd(tempex,lx->htc.local_hash_table);
	} else {
	  tempex->type=HTT_EXPORT_SYS_SYM | HTF_UNRESOLVED;
	  if (tempc)
	    HashAddAfter(tempex,tempc,lx->htc.glbl_hash_table);
	  else
	    HashAdd(tempex,lx->htc.glbl_hash_table);
	}
      }
      switch (Bsf(tempex->type)) {
	case HTt_EXPORT_SYS_SYM:
pu_export_sys_sym:
	  if (PREC_TERM>*max_prec)
	    *max_prec=PREC_TERM;
	  if (!(tempex->type & (HTF_IMM|HTF_IMPORT)))
	    lx->abs_cnts.abs_addres++;
	  if (tempex->type & HTF_UNRESOLVED) {
	    if (!(lx->flags&LF_ASM_EXPRESSIONS))
	      LexExcept(lx,"Illegal fwd ref at ");
	    templu=MAlloc(sizeof(CLexUndef));
	    templu->hash=tempex;
	    templu->next=lx->undef_hash_entry;
	    lx->undef_hash_entry=templu;
	    if (tempex->type & HTF_LOCAL)
	      lx->flags|=LF_UNRESOLVED|LF_LOCAL;
	    else
	      lx->flags|=LF_UNRESOLVED;
	    LCCAdd(lx,IC_IMM_U64,&tempex->val,cmp.internal_types[RT_PTR],ICF_NO_RIP);
	    LCCAdd(lx,IC_DEREF,0,cmp.internal_types[RT_PTR]);
	  } else {
	    if (lx->flags&LF_AOT_COMPILE && !(tempex->type & HTF_IMM))
	      LCCAdd(lx,IC_ABS_ADDR,tempex->val,cmp.internal_types[RT_PTR]);
	    else {
	      if (tempex->type&HTF_IMM)
		lx->abs_cnts.c_addres++;
	      LCCAdd(lx,IC_IMM_U64,tempex->val,cmp.internal_types[RT_PTR]);
	    }
	  }
	  Lex(lx);
	  return PE_MAYBE_MODIFIERS;
	case HTt_FUN:
	  if (PREC_TERM>*max_prec)
	    *max_prec=PREC_TERM;
	  Lex(lx);  //skip fun name
	  return PrsFunCall(lx,ps,FALSE,tempex);
	case HTt_GLBL_VAR:
	  if (PREC_TERM>*max_prec)
	    *max_prec=PREC_TERM;
	  tempg=tempex;
	  tempc=tempg->var_class+1;
	  lx->flags&=~(LF_RAX|LF_ARRAY|LF_FUN_EXP);
	  if (tempg->flags&GVF_ARRAY) {
	    *_tempad=tempg->dim.next;
	    lx->flags|=LF_ARRAY;
	  }
	  if (lx->flags&LF_AOT_COMPILE) {
	    if (tempg->flags & GVF_EXTERN) //TODO
	      LexExcept(lx,"Feature not implemented ");
	    else {
	      if (tempg->flags & GVF_IMPORT)
		LCCAdd(lx,IC_ADDR_IMPORT,tempg,tempc);
	      else {
		if (tempg->flags&GVF_DATA_HEAP)
		  LCCAdd(lx,IC_HEAP_GLBL,tempg->heap_glbl,tempc);
		else
		  LCCAdd(lx,IC_ABS_ADDR,tempg->data_addr_ip,tempc);
	      }
	    }
	  } else {
	    if (tempg->flags & GVF_EXTERN) {
	      lx->abs_cnts.externs++;
	      LCCAdd(lx,IC_IMM_U64,&tempg->data_addr,tempc);
	      LCCAdd(lx,IC_DEREF,0,tempc);
	    } else
	      LCCAdd(lx,IC_IMM_U64,tempg->data_addr,tempc);
	  }
	  Lex(lx);
	  if (tempg->flags & GVF_FUN) {
	    PrsPopDeref(ps);
	    lx->flags|=LF_FUN_EXP;
	    PrsPush2(ps,tempg->fun_ptr-tempg->fun_ptr->ptr_cnt);
	  }
	  return PE_UNARY_MODIFIERS;
	case HTt_CLASS:
	  PrsOffsetOf(lx);
	  return PE_MAYBE_MODIFIERS;
	case HTt_KEYWORD:
	  switch (tempex(CHashGeneric *)->user_data0) {
	    case KW_SIZEOF:
	      if (PREC_TERM>*max_prec)
		*max_prec=PREC_TERM;
	      j=0;
	      while (Lex(lx)=='(')
		j++;
	      PrsSizeOf(lx);
	      while (j--) {
		if (lx->token!=')')
		  LexExcept(lx,"missing ')' at ");
		Lex(lx);
	      }
	      return PE_MAYBE_MODIFIERS;
	    case KW_OFFSET:
	      if (PREC_TERM>*max_prec)
		*max_prec=PREC_TERM;
	      j=0;
	      while (Lex(lx)=='(')
		j++;
	      PrsOffsetOf(lx);
	      while (j--) {
		if (lx->token!=')')
		  LexExcept(lx,"missing ')' at ");
		Lex(lx);
	      }
	      return PE_MAYBE_MODIFIERS;
	    case KW_DEFINED:
	      if (PREC_TERM>*max_prec)
		*max_prec=PREC_TERM;
	      j=0;
	      while (Lex(lx)=='(')
		j++;
	      if (lx->token==TK_IDENT &&
		    (lx->hash_entry || lx->local_var_entry))
		LCCAdd(lx,IC_IMM_U64,TRUE,cmp.internal_types[RT_I64]);
	      else
		LCCAdd(lx,IC_IMM_U64,FALSE,cmp.internal_types[RT_I64]);
	      Lex(lx);
	      while (j--) {
		if (lx->token!=')')
		  LexExcept(lx,"missing ')' at ");
		Lex(lx);
	      }
	      return PE_MAYBE_MODIFIERS;
	  }
      }
  }
  LexExcept(lx,"missing expression at ");
}

I64 PrsUnaryModifier(CLex *lx,CPrsStk *ps,CMemberLst **_local_var,CArrayDim **_tempad,I64 *unary_post_prec)
{
  CHashClass *tempc,*tempc1;
  CHashFun *fun_ptr;
  CMemberLst *tempm=*_local_var;
  CIntermediateCode *tempi,*tempi1;
  CArrayDim *tempad1,tempad2;
  CLexCodeMisc *mc;
  I64 mode,old_flags;
  Bool was_paren=Btr(&lx->flags,Lf_PAREN);
  *_local_var=NULL;
  switch (lx->token) {
    case '.':
      if (tempm)
	tempm->reg=REG_NONE;
      goto um_join;

    case TK_DEREFERENCE:
      tempi=lx->lcc.lcc_root.last;
      if (!(lx->flags & (LF_RAX | LF_ARRAY)))
	LCCAdd(lx,IC_DEREF+PREC_UNARY_PRE<<16,0,tempi->ic_class-1);
      else
	tempi->ic_class--;

um_join:
      if (!*unary_post_prec)
	*unary_post_prec=PREC_TERM;
      tempc=lx->lcc.lcc_root.last->ic_class;
      if (!tempc->ptr_cnt && lx->token=='.')
	LexExcept(lx,"Must be address, not value ");
      if (!(lx->flags & LF_RAX))
	tempc--;
      if (!(tempc->type & HTT_CLASS))
	LexExcept(lx,"invalid class at ");
      if (Lex(lx)!=TK_IDENT ||
	    !(tempm=MemberFind(lx->cur_str,tempc)))
	LexExcept(lx,"invalid member at ");
      else if (lx->local_var_entry)
	lx->local_var_entry->use_cnt--;
      Lex(lx);	//skip member name
      tempc1=tempm->member_class+1;
      LCCAdd(lx,IC_IMM_U64,tempm->offset,tempc1);
      lx->flags&=~(LF_RAX|LF_ARRAY|LF_FUN_EXP);
      if (tempm->dim.next) {
	*_tempad=tempm->dim.next;
	lx->flags|=LF_ARRAY;
      }
      if(tempm->flags & MLF_FUN) {
	PrsPopDeref(ps);
	PrsPush2(ps,tempm->fun_ptr-tempm->fun_ptr->ptr_cnt);
	lx->flags|=LF_FUN_EXP;
      }
      LCCAdd(lx,IC_ADD,0,tempc1);
      return PE_UNARY_MODIFIERS;
    case '(':
      if (lx->flags & LF_FUN_EXP) {
	if (!*unary_post_prec)
	  *unary_post_prec=PREC_TERM;
	return PrsFunCall(lx,ps,TRUE,PrsPop2(ps));
      }
      if (!*unary_post_prec)
	*unary_post_prec=PREC_TERM;
      if (Lex(lx)!=TK_IDENT)
	LexExcept(lx,"invalid class at ");
      if (Btr(&lx->flags,Lf_FUN_EXP))
	ps->ptr2--;
      lx->flags&=~LF_ARRAY;
      tempc=lx->hash_entry;
      Lex(lx);
      mode=PRS0_TYPECAST|PRS1_NULL;
      tempc=PrsType(lx,&tempc,&mode,NULL,NULL,&fun_ptr,NULL,&tempad2,0);
      if (fun_ptr) {
	PrsPopDeref(ps);
	Bts(&lx->flags,Lf_FUN_EXP);
	PrsPush2(ps,fun_ptr);
	mc=LCCMiscNew(lx,CMT_HASH_ENTRY);
	mc->h=fun_ptr;
      }
      if (*_tempad=tempad2.next) {
	lx->flags|=LF_ARRAY;
	tempc++;
	mc=LCCMiscNew(lx,CMT_ARRAY_DIM);
	mc->dim=*_tempad;
      }
      if (!(lx->flags&(LF_RAX|LF_ARRAY)))
	tempc++;
      tempi=lx->lcc.lcc_root.last;
      tempi->ic_class=tempc;
      LCCAdd(lx,IC_HOLYC_TYPECAST,was_paren,tempc);
      if (lx->token!=')')
	LexExcept(lx,"missing ')' at ");
      Lex(lx);
      return PE_UNARY_MODIFIERS;
    case '[':
      if (!*unary_post_prec)
	*unary_post_prec=PREC_TERM;
      Lex(lx);
      tempc=OptClassFwd(lx->lcc.lcc_root.last->ic_class);
      if (!tempc->ptr_cnt)
	LexExcept(lx,"not array or ptr ");
      if (!(lx->flags & (LF_ARRAY | LF_RAX))) {
	tempc=OptClassFwd(tempc-1);
	if (!tempc->ptr_cnt)
	  LexExcept(lx,"not array or ptr ");
	LCCAdd(lx,IC_DEREF+PREC_UNARY_PRE<<16,0,tempc);
      }
      tempc1=tempc-1;
      if (tempad1=*_tempad) {
	LCCAdd(lx,IC_IMM_U64,tempad1->total_cnt*tempc1->size,tempc);
	if (*_tempad=tempad1->next) {
	  old_flags=lx->flags;
	  if (!PrsExpression(lx,NULL,FALSE,ps))
	    throw('Compiler');
	  lx->flags=lx->flags&~LF_FUN_EXP|old_flags&LF_FUN_EXP;
	  if (lx->token!=']')
	    LexExcept(lx,"missing ']' at ");
	  Lex(lx); //skip ]
	  tempi1=lx->lcc.lcc_root.last;
	  tempi1->ic_flags|=ICF_R_TO_INT;
	  LCCAdd(lx,IC_MUL,0,tempc);
	  LCCAdd(lx,IC_ADD,0,tempc);
	  lx->flags|=LF_RAX;
	  return PE_UNARY_MODIFIERS;
	}
      } else
	LCCAdd(lx,IC_IMM_U64,tempc1->size,tempc);
      old_flags=lx->flags;
      if (!PrsExpression(lx,NULL,FALSE,ps))
	throw('Compiler');
      lx->flags=lx->flags&~LF_FUN_EXP|old_flags&LF_FUN_EXP;
      if (lx->token!=']')
	LexExcept(lx,"missing ']' at ");
      Lex(lx); //skip ]
      tempi1=lx->lcc.lcc_root.last;
      tempi1->ic_flags|=ICF_R_TO_INT;
      LCCAdd(lx,IC_MUL,0,tempc);
      LCCAdd(lx,IC_ADD,0,tempc);
      lx->flags&=~(LF_RAX|LF_ARRAY);
      return PE_UNARY_MODIFIERS;
    sub_switch_start:
      case TK_PLUS_PLUS:
	lx->flags|=LF_POSTINC;
	break;
      case TK_MINUS_MINUS:
	lx->flags|=LF_POSTDEC;
	break;
    sub_switch_end:
      if (!*unary_post_prec)
	*unary_post_prec=PREC_UNARY_POST;
      Lex(lx);
      return PE_DEREFERENCE;
  }
  return PE_DEREFERENCE;
}

U8 *LexExpression2Bin(CLex *lx,I64 *_type=NULL)
{//Compile lx expression. You call code.
  U8 *result;
  I64 size;
  Bool old_trace=Btr(&lx->flags,Lf_OPT_TRACE_PRESENT);
  LCCPush(lx);
  LCCInit(lx);
  if (PrsExpression(lx,NULL,FALSE)) {
    LCCAdd(lx,IC_RETURN_VAL,0,NULL);
    LCCAdd(lx,IC_RET,0,0);
    result=LCCCompile(lx,&size,NULL,_type);
  } else
    result=NULL;
  LCCPop(lx);
  BEqu(&lx->flags,Lf_OPT_TRACE_PRESENT,old_trace);
  return result;
}

Bool IsLexExpression2Bin(CLex *lx,U8 **_ex)
{//Compile lx expression to bin. Return err status.
  return ToBool(*_ex=LexExpression2Bin(lx));
}

I64 LexExpressionI64(CLex *lx)
{//Compile lx expression, forcing to I64 and eval.
  U8 *ex;
  I64 result,type;
  if (ex=LexExpression2Bin(lx,&type)) {
    result=Call(ex);
    Free(ex);
    if (type==RT_F64)
      result=ToI64(result(F64));
  } else
    result=0;
  return result;
}

F64 LexExpressionF64(CLex *lx)
{//Compile lx expression, forcing to F64 and eval.
  U8 *ex;
  I64 result,type;
  if (ex=LexExpression2Bin(lx,&type)) {
    result=Call(ex);
    Free(ex);
    if (type!=RT_F64)
      result(F64)=ToF64(result);
  } else
    result=0;
  return result(F64);
}

I64 LexExpression(CLex *lx)
{//Compile lx expression and eval.  Might be I64 or F64.
  U8 *ex;
  I64 result;
  if (ex=LexExpression2Bin(lx)) {
    result=Call(ex);
    Free(ex);
  } else
    result=0;
  return result;
}

CD2I32 *LexD2I32(CLex *lx,CD2I32 *p)
{//Not HolyC. Sprite-like lex 2D point.
  if (lx->token!='(')
    LexExcept(lx,"expecting '(' at ");
  Lex(lx); //Skip (
  p->x=LexExpressionI64(lx);
  if (lx->token!=',')
    LexExcept(lx,"expecting ',' at ");
  Lex(lx); //Skip ,
  p->y=LexExpressionI64(lx);
  if (lx->token!=')')
    LexExcept(lx,"expecting ')' at ");
  Lex(lx); //Skip )
  return p;
}

CD3I32 *LexD3I32(CLex *lx,CD3I32 *p)
{//Not HolyC. Sprite-like lex 3D point.
  if (lx->token!='(')
    LexExcept(lx,"expecting '(' at ");
  Lex(lx); //Skip (
  p->x=LexExpressionI64(lx);
  if (lx->token!=',')
    LexExcept(lx,"expecting ',' at ");
  Lex(lx); //Skip ,
  p->y=LexExpressionI64(lx);
  if (lx->token!=',')
    LexExcept(lx,"expecting ',' at ");
  Lex(lx); //Skip ,
  p->z=LexExpressionI64(lx);
  if (lx->token!=')')
    LexExcept(lx,"expecting ')' at ");
  Lex(lx); //Skip )
  return p;
}
