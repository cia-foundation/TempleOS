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

CIntermediateCode *PrsAddOp(CCmpCtrl *cc,I64 stk_op,CHashClass *tempc)
{
  CIntermediateCode *tempi=cc->coc.coc_root.last;
  Bool div_sizeof=FALSE;
  switch (stk_op.u16[0]) {
    case IC_ADD:
      if (tempc->ptr_stars_cnt && !tempi->ic_class->ptr_stars_cnt &&
	    tempi->ic_class->raw_type!=RT_F64) {
	ICAdd(cc,IC_SIZEOF,1,cmp.internal_types[RT_I64]);
	ICAdd(cc,IC_MUL,0,cmp.internal_types[RT_I64]);
      }
      break;
    case IC_SUB:
      if (tempc->ptr_stars_cnt && tempi->ic_class->raw_type!=RT_F64) {
	if (!tempi->ic_class->ptr_stars_cnt) {
	  ICAdd(cc,IC_SIZEOF,1,cmp.internal_types[RT_I64]);
	  ICAdd(cc,IC_MUL,0,cmp.internal_types[RT_I64]);
	} else
	  div_sizeof=TRUE;
      }
      break;
    case IC_AND_AND:
    case IC_OR_OR:
      ICAdd(cc,IC_NOP1,0,cmp.internal_types[RT_I64]);
      break;
    case IC_ADD_EQUAL:
    case IC_SUB_EQUAL:
      if (tempc->ptr_stars_cnt) {
	ICAdd(cc,IC_SIZEOF,1,cmp.internal_types[RT_I64]);
	ICAdd(cc,IC_MUL,0,cmp.internal_types[RT_I64]);
      }
      break;
  }
  tempi=ICAdd(cc,stk_op,0,tempc);
  if (stk_op.u8[3]&ECF_HAS_PUSH_CMP) {
    tempi->ic_flags|=ICF_POP_CMP;
    ICAdd(cc,IC_NOP1,0,cmp.internal_types[RT_I64]);
    ICAdd(cc,IC_AND_AND,0,cmp.internal_types[RT_I64],ICF_POP_CMP);
  }
  if (div_sizeof) {
    tempc--;
    if (tempc->size!=1) {
      ICAdd(cc,IC_IMM_I64,tempc->size,cmp.internal_types[RT_I64]);
      ICAdd(cc,IC_DIV,0,cmp.internal_types[RT_I64]);
      tempc=cmp.internal_types[RT_I64];
    }
  }
  return tempi;
}

U0 PrsExpression2(CCmpCtrl *cc,I64 *_max_prec,CPrsStk *ps)
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
	cc->flags&=~(CCF_PAREN+CCF_PREINC+CCF_PREDEC+CCF_POSTINC+
	      CCF_POSTDEC+CCF_FUN_EXP);
      case PE_UNARY_TERM2:
	state=PrsUnaryTerm(cc,ps,&local_var,&tempad,
	      &max_prec,&unary_pre_prec,&paren_prec);
	break;
      case PE_UNARY_MODIFIERS:
	state=PrsUnaryModifier(cc,ps,&local_var,&tempad,&unary_post_prec);
	break;
      case PE_MAYBE_MODIFIERS:
	if (cc->token=='(') {
	  cc->flags|=CCF_RAX;
	  state=PrsUnaryModifier(cc,ps,&local_var,&tempad,&unary_post_prec);
	} else
	  goto pe_check_binary_ops1;
	break;
      case PE_DEREFERENCE:
	if (!(cc->flags&(CCF_PREINC|CCF_PREDEC|CCF_POSTINC|CCF_POSTDEC)))
	  i=IC_DEREF+PREC_UNARY_PRE<<16;
	else {
	  if (cc->flags & CCF_POSTINC)
	    i=IC__PP+PREC_UNARY_POST<<16;
	  else if (cc->flags & CCF_POSTDEC)
	    i=IC__MM+PREC_UNARY_POST<<16;
	  else if (cc->flags & CCF_PREDEC)
	    i=IC_MM_+PREC_UNARY_PRE<<16;
	  else
	    i=IC_PP_+PREC_UNARY_PRE<<16;
	  cc->flags&=~(CCF_PREINC|CCF_PREDEC|CCF_POSTINC|CCF_POSTDEC);
	}
	tempi=cc->coc.coc_root.last;
	if (cc->flags & (CCF_RAX|CCF_ARRAY)) {
	  if (tempi->ic_code==IC_DEREF)
	    tempi->ic_code=i;
	} else {
	  tempc=OptClassFwd(tempi->ic_class-1);
	  ICAdd(cc,i,0,tempc);
	}
      case PE_CHECK_BINARY_OPS1:
pe_check_binary_ops1:
	if (paren_prec) {
	  if (unary_pre_prec || unary_post_prec) {
	    if (paren_prec<=unary_pre_prec && !unary_post_prec)
	      ParenWarning(cc);
	    paren_prec=PREC_NULL;
	  } else if (paren_prec<=PREC_UNARY_PRE+ASSOC_MASK)
	    ParenWarning(cc);
	}
	cur_op=cmp.binary_ops[cc->token];

      case PE_CHECK_BINARY_OPS2:
pe_check_binary_ops2:
	stk_op=PrsPop(ps);
	tempc=PrsPop(ps);
	if (!(0<stk_op.u8[2]<=PREC_UNARY_PRE+ASSOC_MASK))
	  goto pe_do_binary_op;

      case PE_DO_UNARY_OP:
	if (cur_op.u16[0]==IC_POWER &&
	      stk_op.u16[0]==IC_UNARY_MINUS) {
	  Lex(cc); //skip ` op
	  left_prec=cur_op.i8[2];
	  PrsPush(ps,tempc);
	  PrsPush(ps,stk_op);
	  PrsPush(ps,cc->coc.coc_root.last->ic_class);
	  PrsPush(ps,cur_op);
	  goto pe_unary_term1;
	} else {
	  tempi=cc->coc.coc_root.last;
	  tempc=tempi->ic_class;
	  if (stk_op.u16[0]==IC_DEREF && tempc->ptr_stars_cnt)
	    tempc--;
	  else if (stk_op.u16[0]==IC_ADDR) {
	    cc->abs_cnts.c_addres++;
	    if (intermediate_code_table[tempi->ic_code].type==IST_DEREF)
	      OptFree(tempi);
	    tempc++;
	  }
	  tempc=OptClassFwd(tempc);
	  if (stk_op)
	    ICAdd(cc,stk_op,0,tempc);
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
	    tempi=cc->coc.coc_root.last;
	    if (!tempi->ic_class->ptr_stars_cnt &&
		  tempi->ic_class->raw_type!=RT_F64) {
	      ICAdd(cc,IC_SIZEOF,1,cmp.internal_types[RT_I64]);
	      ICAdd(cc,IC_MUL,0,cmp.internal_types[RT_I64]);
	    }
	    break;
	  case IC_AND_AND:
	  case IC_OR_OR:
	    ICAdd(cc,IC_NOP1,0,cmp.internal_types[RT_I64]);
	    break;
	}
	if (cc->flags & CCF_FUN_EXP) {
	  ps->ptr2--;
	  cc->flags&=~CCF_FUN_EXP;
	}
	Lex(cc); //skip op
	if (paren_prec>PREC_UNARY_PRE+ASSOC_MASK &&
	      paren_prec&~ASSOC_MASK<left_prec&~ASSOC_MASK+
	      paren_prec&ASSOCF_RIGHT &&
	      paren_prec&~ASSOC_MASK<cur_op.u8[2]&~ASSOC_MASK+
	      !(paren_prec&ASSOCF_RIGHT))
	  ParenWarning(cc);
	if (cur_op.u8[2]>max_prec)
	  max_prec=cur_op.u8[2];
	left_prec=cur_op.u8[2];
	if (intermediate_code_table[cur_op.u16[0]].type==IST_ASSIGN) {
	  tempi=cc->coc.coc_root.last;
	  tempc=OptClassFwd(tempi->ic_class);
	  if (intermediate_code_table[tempi->ic_code].type!=IST_DEREF ||
		!tempc->ptr_stars_cnt && !Bt(&tempc->flags,Cf_INTERNAL_TYPE))
	    LexExcept(cc,"invalid lval at ");
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

	tempi=PrsAddOp(cc,stk_op,tempc);

	if (intermediate_code_table[cur_op.u16[0]].type==IST_CMP &&
	      intermediate_code_table[stk_op.u16[0]].type==IST_CMP) {
	  tempi->ic_flags|=ICF_PUSH_CMP;
	  ICAdd(cc,IC_NOP1,0,cmp.internal_types[RT_I64]);
	  ICAdd(cc,IC_PUSH_CMP,0,tempc);
	  cur_op.u8[3]|=ECF_HAS_PUSH_CMP;
	} else if (cur_op.u16[0]==IC_AND_AND || cur_op.u16[0]==IC_OR_OR)
	  ICAdd(cc,IC_NOP1,0,cmp.internal_types[RT_I64]);
	goto pe_pop_higher;

      case PE_PUSH_LOWER:
pe_push_lower:
	PrsPush(ps,tempc);
	PrsPush(ps,stk_op);
	PrsPush(ps,cc->coc.coc_root.last->ic_class);
	PrsPush(ps,cur_op);
	goto pe_unary_term1;

      case PE_POP_ALL1:
pe_pop_all1:
	if (paren_prec>PREC_UNARY_PRE+ASSOC_MASK &&
	      paren_prec&~ASSOC_MASK<=left_prec&~ASSOC_MASK-
	      paren_prec&ASSOCF_LEFT-left_prec&ASSOCF_LEFT)
	  ParenWarning(cc);
      case PE_POP_ALL2:
pe_pop_all2:
	stk_op=PrsPop(ps);
	tempc=PrsPop(ps);
	if (!stk_op.u16[0])
	  goto pe_done;
	PrsAddOp(cc,stk_op,tempc);
	goto pe_pop_all2;
    }
  }
pe_done:
  if (_max_prec)
    *_max_prec=max_prec;
}

Bool PrsExpression(CCmpCtrl *cc,I64 *_max_prec,Bool end_exp,CPrsStk *_ps=NULL)
{
  Bool result=TRUE;
  I64 old_flags=cc->flags;
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
//try catch causes noreg vars in function
	  PrsExpression2(cc,_max_prec,ps);
  catch {
    if (Fs->except_ch=='Compiler') {
      result=FALSE;
      Fs->catch_except=TRUE;
    }
  }
  if (!_ps) {
    if (ps->ptr)
      LexExcept(cc,"Compiler Parse Error at ");
    Free(ps);
  }
  if (end_exp && result)
    ICAdd(cc,IC_END_EXP,0,0,ICF_NO_DEPEND_RESULT);
  cc->flags|=old_flags&(CCF_PREINC|CCF_PREDEC);
  return result;
}

U0 PrsSizeOf(CCmpCtrl *cc)
{
  CHashClass *tempc;
  CMemberLst *tempm;
  CDbgInfo *dbg_info;
  I64 i;
  if (cc->token!=TK_IDENT)
    LexExcept(cc,"invalid class at ");
  if (tempm=cc->local_var_entry) {
    tempc=tempm->member_class;
    i=tempc->size*tempm->dim.total_cnt;
    if (Lex(cc)=='.')
      goto pu_sizeof_member;
  } else {
    if (!(tempc=cc->hash_entry) || !(tempc->type &
	  (HTT_CLASS|HTT_INTERNAL_TYPE|HTT_GLBL_VAR|
	  HTT_FUN|HTT_EXPORT_SYS_SYM)))
      LexExcept(cc,"invalid class at ");
    if (tempc->type&(HTT_FUN|HTT_EXPORT_SYS_SYM)) {
      if (!(dbg_info=tempc(CHashFun *)->dbg_info))
	LexExcept(cc,"Size not defined at ");
      i=dbg_info->body[dbg_info->max_line+1-dbg_info->min_line]
	    -dbg_info->body[0];
      Lex(cc);
    } else {
      i=tempc->size;
      while (Lex(cc)=='.') {
pu_sizeof_member:
	if (!(tempc->type & (HTT_CLASS|HTT_GLBL_VAR)))
	  LexExcept(cc,"invalid class at ");
	else if (tempc->type & HTT_GLBL_VAR)
	  tempc=tempc(CHashGlblVar *)->var_class;
	if (Lex(cc)!=TK_IDENT || !(tempm=MemberFind(cc->cur_str,tempc)))
	  LexExcept(cc,"invalid member at ");
	else if (cc->local_var_entry)
	  cc->local_var_entry->use_cnt--;
	tempc=tempm->member_class;
//Probably others like this:
	#assert offset(CHashClass.size)==offset(CHashGlblVar.size)
	i=tempc->size*tempm->dim.total_cnt;
      }
    }
  }
  if (cc->token=='*') {
    while (Lex(cc)=='*');
    i=sizeof(U8 *);
  }
  ICAdd(cc,IC_IMM_I64,i,cmp.internal_types[RT_I64]);
}

U0 PrsOffsetOf(CCmpCtrl *cc)
{
  CHashClass *tempc;
  CMemberLst *tempm;
  I64 i;
  if (cc->token!=TK_IDENT)
    LexExcept(cc,"invalid class at ");
  if (tempm=cc->local_var_entry)
    tempc=tempm->member_class;
  else {
    tempc=cc->hash_entry;
    if (!tempc || !(tempc->type & (HTT_CLASS|HTT_GLBL_VAR)))
      LexExcept(cc,"invalid class at ");
    else if (tempc->type & HTT_GLBL_VAR)
      tempc=tempc(CHashGlblVar *)->var_class;
  }
  if (Lex(cc)!='.')
    LexExcept(cc,"expecting '.' at ");
  i=0;
  do {
    if (Lex(cc)!=TK_IDENT || !(tempm=MemberFind(cc->cur_str,tempc)))
      LexExcept(cc,"invalid member at ");
    else if (cc->local_var_entry)
      cc->local_var_entry->use_cnt--;
    i+=tempm->offset;
    tempc=tempm->member_class;
  } while (Lex(cc)=='.');
  ICAdd(cc,IC_IMM_I64,i,cmp.internal_types[RT_I64]);
}

I64 PrsFunCall(CCmpCtrl *cc,CPrsStk *ps,Bool indirect,CHashFun *tempf)
{
  I64 i,argc_cnt,dft_val;
  Bool is_first_arg=TRUE,needs_right_paren,is_print,is_putchars,
	is_template_fun;
  CHashClass *tempc2,*last_class=NULL;
  CMemberLst *tempm;
  CCodeCtrl *tempcbh,*tempcbh1;
  CCodeMisc *cm;
  CIntermediateCode *tempi;

  if (!tempf) {
    if (cc->token==TK_CHAR_CONST) {
      if (!(tempf=HashFind("PutChars",cc->htc.hash_table_lst,HTT_FUN)))
	LexExcept(cc,"Missing header for Print() and PutChars() at ");
      if (!cc->cur_i64) //empty char signals PutChars with variable
	Lex(cc);
      is_print=FALSE;
      is_putchars=TRUE;
    } else {
      if (!(tempf=HashFind("Print",cc->htc.hash_table_lst,HTT_FUN)))
	LexExcept(cc,"Missing header for Print() and PutChars() at ");
      if (!*cc->cur_str) //empty string signals Print with variable fmt_str
	Lex(cc);
      is_putchars=FALSE;
      is_print=TRUE;
    }
  } else {
    is_print=FALSE;
    is_putchars=FALSE;
  }

  if (Bt(&tempf->flags,Ff_INTERNAL)&& IC_SQR<=tempf->executable_addr<=IC_ATAN)
    is_template_fun=TRUE;
  else
    is_template_fun=FALSE;

  if (indirect) {
    if (!(cc->flags & (CCF_RAX | CCF_ARRAY)))
      ICAdd(cc,IC_DEREF,0,cmp.internal_types[RT_PTR]);
    cc->coc.coc_root.last->ic_class=cmp.internal_types[RT_PTR];
    ICAdd(cc,IC_SET_RAX,0,cmp.internal_types[RT_PTR]);
    ICAdd(cc,IC_NOP2,1,cmp.internal_types[RT_PTR]); //balance the books
  }
  COCPush(cc);
  tempcbh=cc->coc.coc_next;
  cc->coc.coc_next=NULL;
  i=tempf->arg_cnt;
  tempm=tempf->member_lst_and_root;
  argc_cnt=0;
  if (cc->token=='(') {
    Lex(cc);
    needs_right_paren=TRUE;
  } else
    needs_right_paren=FALSE;
  while (i--) {
    COCInit(cc);
    if (!is_first_arg) {
      if (is_print) {
	if (cc->token==',')
	  Lex(cc);
	else if (cc->token!=';')
	  LexExcept(cc,"expecting ',' at ");
      } else {
	if (needs_right_paren) {
	  if (cc->token==',')
	    Lex(cc);
	  else if (cc->token!=')')
	    LexExcept(cc,"expecting ',' at ");
	}
      }
    }
    if (tempm->flags & MLF_DFT_AVAILABLE &&
	  (cc->token==')' || cc->token==',' || !needs_right_paren)) {
      dft_val=tempm->dft_val;
      if (tempm->flags & MLF_LASTCLASS && last_class)
	dft_val=(last_class-last_class->ptr_stars_cnt)->str;
      if (tempm->flags & (MLF_STR_DFT_AVAILABLE|MLF_LASTCLASS) &&
	    cc->flags&CCF_AOT_COMPILE) {
	cm=COCMiscNew(cc,CMT_STR_CONST);
	ICAdd(cc,IC_STR_CONST,cm,cmp.internal_types[RT_U8]+1);
	cm->st_len=StrLen(dft_val)+1;
	cm->str=StrNew(dft_val);
	cc->flags|=CCF_HAS_MISC_DATA;
      } else
	ICAdd(cc,IC_IMM_I64,dft_val,tempm->member_class);
    } else {
      if (!PrsExpression(cc,NULL,FALSE,ps))
	throw('Compiler');
      else {
	COCPush(cc);
	cc->pass=0;	  OptPass012(cc);
	cc->pass=1; tempi=OptPass012(cc);
	COCPop(cc);
	last_class=OptClassFwd(tempi->ic_class);
	tempc2=OptClassFwd(tempm->member_class);
	if (tempc2->raw_type==RT_F64 && !tempm->dim.next &&
	      last_class->raw_type!=RT_F64)
	  tempi->ic_flags|=ICF_R_TO_F64;
	else
	  if (tempc2->raw_type!=RT_F64 && last_class->raw_type==RT_F64)
	    tempi->ic_flags|=ICF_R_TO_INT;
      }
    }
    COCPush(cc);
    is_first_arg=FALSE;
    tempm=tempm->next;
  }
  if (tempm && tempm->flags & MLF_DOT_DOT_DOT) {
    COCInit(cc);
    tempi=ICAdd(cc,IC_IMM_I64,0,tempm->member_class);
    COCPush(cc);
    if (is_print) {
      if (cc->token!=';') {
	do {
	  if (!is_first_arg) {
	    if (cc->token==',')
	      Lex(cc);
	    else
	      LexExcept(cc,"expecting ',' at ");
	  }
	  COCInit(cc);
	  if (!PrsExpression(cc,NULL,FALSE,ps))
	    throw('Compiler');
	  COCPush(cc);
	  is_first_arg=FALSE;
	  argc_cnt++;
	} while (cc->token==',');
      }
    } else if (needs_right_paren) {
      if (cc->token!=')') {
	do {
	  if (!is_first_arg) {
	    if (cc->token==',')
	      Lex(cc);
	    else
	      LexExcept(cc,"expecting ',' at ");
	  }
	  COCInit(cc);
	  if (!PrsExpression(cc,NULL,FALSE,ps))
	    throw('Compiler');
	  COCPush(cc);
	  is_first_arg=FALSE;
	  argc_cnt++;
	} while (cc->token==',');
      }
    }
    tempi->ic_data=argc_cnt++; //++ so add_esp latter works
  }
  if (needs_right_paren) {
    if (cc->token==')')
      Lex(cc);
    else
      LexExcept(cc,"missing ')' at ");
  }
  tempcbh1=tempcbh->coc_next;
  tempcbh->coc_next=cc->coc.coc_next;
  cc->coc.coc_next=tempcbh;
  COCPop(cc);
  tempcbh=cc->coc.coc_next;
  cc->coc.coc_next=tempcbh1;
  if (!is_template_fun)
    ICAdd(cc,IC_CALL_START,tempf,0);
  if (indirect)
    ICAdd(cc,IC_PUSH_REGS,1<<REG_RAX,tempf->return_class);
  while (tempcbh) {
    tempcbh1=tempcbh->coc_next;
    COCAppend(cc,tempcbh);
    if (!Bt(&tempf->flags,Ff_INTERNAL))
      cc->coc.coc_root.last->ic_flags|=ICF_PUSH_RESULT;
    tempcbh=tempcbh1;
  }
  if (Bt(&tempf->flags,Ff_INTERNAL))
    ICAdd(cc,tempf->executable_addr,0,tempf->return_class);
  else {
    if (indirect)
      ICAdd(cc,IC_CALL_INDIRECT,
	    (argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
    else if (Bt(&tempf->flags,Cf_EXTERN)) {
      cc->abs_cnts.externs++;
      if (cc->flags&CCF_AOT_COMPILE) {
	if (tempf->type&HTF_IMPORT)
	  ICAdd(cc,IC_CALL_IMPORT,tempf,tempf->return_class);
	else
	  ICAdd(cc,IC_CALL_EXTERN,tempf,tempf->return_class);
      } else
	ICAdd(cc,IC_CALL_INDIRECT2,&tempf->executable_addr,tempf->return_class);
    } else
      ICAdd(cc,IC_CALL,tempf->executable_addr,tempf->return_class);
    if ((Bt(&tempf->flags,Ff_RET1) || Bt(&tempf->flags,Ff_ARGPOP)) &&
	  !Bt(&tempf->flags,Ff_NOARGPOP)) {
      if (indirect) {
	ICAdd(cc,IC_ADD_RSP1,(argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
	ICAdd(cc,IC_ADD_RSP,8,tempf->return_class);
      } else
	ICAdd(cc,IC_ADD_RSP1,(argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
    } else {
      if (indirect)
	argc_cnt++;
      ICAdd(cc,IC_ADD_RSP,(argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
    }
  }
  if (!is_template_fun)
    ICAdd(cc,IC_CALL_END,tempf,tempf->return_class);
  if (is_print||is_putchars)
    ICAdd(cc,IC_END_EXP,0,0,ICF_NO_DEPEND_RESULT);
  cc->flags=(cc->flags|CCF_RAX) & ~(CCF_ARRAY|CCF_FUN_EXP);
  return PE_UNARY_MODIFIERS;
}

I64 PrsUnaryTerm(CCmpCtrl *cc,CPrsStk *ps,CMemberLst **_local_var,
	CArrayDim **_tempad,I64 *max_prec,I64 *unary_pre_prec,I64 *paren_prec)
{
  I64 i,j;
  CHashExport *tempex;
  CHashClass *tempc;
  CHashFun *tempf;
  CHashGlblVar *tempg;
  CMemberLst *tempm;
  CAsmUndefHash *tempauh;
  CCodeMisc *cm;
  Bool paren_warn;

  *_local_var=NULL;
  *paren_prec=PREC_NULL;
  switch (cc->token) {
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
	Lex(cc); //Skip op
	break;
      case '&':
	if (Lex(cc)==TK_IDENT) {
	  if (tempc=cc->hash_entry) {
	    if (tempc->type & HTT_FUN) {
	      tempf=tempc;
	      if (!Bt(&tempf->flags,Ff_INTERNAL)) {
		if (Bt(&tempf->flags,Cf_EXTERN)) {
		  if (cc->flags&CCF_AOT_COMPILE) {
		    if (cc->flags&CCF_ASM_EXPRESSIONS) {
		      if (tempex=HashFind(
			    tempf->str,cc->htc.hash_table_lst,
			    HTT_EXPORT_SYS_SYM))
			goto pu_export_sys_sym;
		      else
			goto pu_new_sys_sym;
		    }
		    LexExcept(cc,"Can't take addr of extern fun");
		  }
		  cc->abs_cnts.externs++;
		  ICAdd(cc,IC_IMM_I64,
			&tempf->executable_addr,cmp.internal_types[RT_PTR]);
		  ICAdd(cc,IC_DEREF,0,
			cmp.internal_types[RT_PTR]);
		} else {
		  if (cc->flags&CCF_AOT_COMPILE) {
		    ICAdd(cc,IC_ABS_ADDR,
			  tempf->executable_addr,cmp.internal_types[RT_PTR]);
		    if (cc->flags&CCF_ASM_EXPRESSIONS)
		      cc->abs_cnts.abs_addres++;
		  } else
		    ICAdd(cc,IC_IMM_I64,
			  tempf->executable_addr,cmp.internal_types[RT_PTR]);
		}
		cc->abs_cnts.c_addres++;
		Lex(cc);
		return PE_MAYBE_MODIFIERS;
	      }
	    } else if (tempc->type & HTT_EXPORT_SYS_SYM) {
	      tempex=tempc;
	      if (cc->flags&CCF_ASM_EXPRESSIONS &&
		    !(cc->flags&CCF_AOT_COMPILE) && tempex->type&HTF_IMM) {
		cc->abs_cnts.c_addres++;
		ICAdd(cc,IC_IMM_I64,
		      tempex->val,cmp.internal_types[RT_PTR]);
		Lex(cc);
		return PE_MAYBE_MODIFIERS;
	      } else
		goto pu_export_sys_sym;
	    }
	  } else if (cc->flags&CCF_ASM_EXPRESSIONS && !cc->local_var_entry)
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
	if (cc->cur_i64<0)
	  ICAdd(cc,IC_IMM_I64,cc->cur_i64,cmp.internal_types[RT_U64]);
	else
	  ICAdd(cc,IC_IMM_I64,cc->cur_i64,cmp.internal_types[RT_I64]);
	Lex(cc);
	break;
      case TK_F64:
	ICAdd(cc,IC_IMM_F64,cc->cur_f64(I64),cmp.internal_types[RT_F64]);
	Lex(cc);
	break;
      case TK_STR:
	cm=COCMiscNew(cc,CMT_STR_CONST);
	ICAdd(cc,IC_STR_CONST,cm,cmp.internal_types[RT_U8]+1);
	cm->str=LexExtStr(cc,&cm->st_len);
	cc->flags|=CCF_HAS_MISC_DATA;
	break;
      case TK_INS_BINARY:
	cm=COCMiscNew(cc,CMT_STR_CONST);
	ICAdd(cc,IC_STR_CONST,cm,cmp.internal_types[RT_U8]+1);
	cm->str=cc->cur_str;
	cm->st_len=cc->cur_str_len;
	cc->cur_str=NULL;
	cc->flags|=CCF_HAS_MISC_DATA;
	Lex(cc);
	break;
      case '$$':
	if (cc->flags & CCF_ASM_EXPRESSIONS) {
	  cc->abs_cnts.abs_addres++;
	  if (cc->flags&CCF_AOT_COMPILE)
	    ICAdd(cc,IC_ABS_ADDR,cc->aotc->ip,cmp.internal_types[RT_PTR]);
	  else
	    ICAdd(cc,IC_IMM_I64,cc->aotc->ip,cmp.internal_types[RT_PTR]);
	} else {
	  if (cc->flags&CCF_CLASS_IP)
	    ICAdd(cc,IC_IMM_I64,cc->class_ip,cmp.internal_types[RT_I64]);
	  else
	    ICAdd(cc,IC_RIP,0,cmp.internal_types[RT_PTR]);
	}
	Lex(cc);
	break;
    sub_switch_end:
      if (PREC_TERM>*max_prec)
	*max_prec=PREC_TERM;
      return PE_MAYBE_MODIFIERS;

    case '(':
      if (Lex(cc)==TK_IDENT && cc->hash_entry &&
	    cc->hash_entry->type & (HTT_CLASS|HTT_INTERNAL_TYPE))
	LexExcept(cc,"Use TempleOS postfix typecasting at ");
      else {
	if (PREC_TERM>*max_prec)
	  *max_prec=PREC_TERM;
	if (cc->lex_include_stk->flags&LFSF_DEFINE)
	  paren_warn=FALSE;
	else
	  paren_warn=TRUE;
	if (!PrsExpression(cc,paren_prec,FALSE,ps))
	  throw('Compiler');
	if (!paren_warn)
	  *paren_prec=PREC_NULL;
	if (cc->token!=')')
	  LexExcept(cc,"missing ')' at ");
	Lex(cc); //skip )
	cc->flags= cc->flags & ~CCF_ARRAY | CCF_RAX | CCF_PAREN;
	return PE_UNARY_MODIFIERS;
      }

    sub_switch_start:
      case '+': 				  break;
      case TK_PLUS_PLUS:   cc->flags|=CCF_PREINC; break;
      case TK_MINUS_MINUS: cc->flags|=CCF_PREDEC; break;
    sub_switch_end:
      if (PREC_UNARY_PRE>*max_prec)
	*max_prec=PREC_UNARY_PRE;
      *unary_pre_prec=PREC_UNARY_PRE;
      Lex(cc);
      return PE_UNARY_TERM2;

    case TK_IDENT:
      if (tempm=cc->local_var_entry) {
	if (PREC_TERM>*max_prec)
	  *max_prec=PREC_TERM;
	cc->flags&=~(CCF_RAX|CCF_ARRAY|CCF_FUN_EXP);
	tempc=tempm->member_class+1;
	if (tempm->flags & MLF_FUN && !(cc->flags&CCF_ASM_EXPRESSIONS)) {
	  PrsPopDeref(ps);
	  cc->flags|=CCF_FUN_EXP;
	  PrsPush2(ps,tempm->fun_ptr-tempm->fun_ptr->ptr_stars_cnt);
	}
	if (tempm->dim.next) {
	  *_tempad=tempm->dim.next;
	  cc->flags|=CCF_ARRAY;
	}
	if (tempm->flags&MLF_STATIC) {
	  if (cc->flags&CCF_AOT_COMPILE) {
//	if (tempg->flags&GVF_DATA_HEAP) //TODO
	    //	  ICAdd(cc,IC_HEAP_GLBL,tempm->static_data,tempc);
	    //	else
	    ICAdd(cc,IC_ABS_ADDR,tempm->static_data_ip,tempc);
	  } else
	    ICAdd(cc,IC_IMM_I64,tempm->static_data,tempc);
	} else {
	  if (cc->flags&CCF_ASM_EXPRESSIONS) {
	    i=PrsPop(ps);
	    ps->ptr--;
	    if (i.u16[0]!=IC_ADDR)
	      LexExcept(cc,"expecting '&' at ");
	    ICAdd(cc,IC_IMM_I64,tempm->offset,cmp.internal_types[RT_PTR]);
	    *_local_var=tempm;
	    Lex(cc);
	    return PE_MAYBE_MODIFIERS;
	  } else {
	    if (tempm->dim.next && tempm->offset>0 &&
		  StrCmp(tempm->str,"argv")) {
	      tempc++;
	      cc->flags&=~CCF_ARRAY;
	    }
	    ICAdd(cc,IC_RBP,0,tempc);
	    ICAdd(cc,IC_IMM_I64,tempm->offset,tempc);
	    ICAdd(cc,IC_ADD,0,tempc);
	  }
	}
	Lex(cc); //skip var name
	*_local_var=tempm;
	return PE_UNARY_MODIFIERS;
      }
pu_ident_but_not_local_var:
      if (!(tempex=cc->hash_entry)) {
	if (!(cc->flags & CCF_ASM_EXPRESSIONS))
	  LexExcept(cc,"invalid lval at ");
	tempc=NULL;
pu_new_sys_sym:
	tempex=CAlloc(sizeof(CHashExport),Fs->code_heap);
	tempex->str=cc->cur_str;
	cc->cur_str=NULL;
	if (!cc->htc.local_var_lst &&
	      *tempex->str=='@' && tempex->str[1]=='@') {
	  tempex->type=HTT_EXPORT_SYS_SYM | HTF_UNRESOLVED | HTF_LOCAL;
	  HashAdd(tempex,cc->htc.local_hash_table);
	} else {
	  tempex->type=HTT_EXPORT_SYS_SYM | HTF_UNRESOLVED;
	  if (tempc)
	    HashAddAfter(tempex,tempc,cc->htc.glbl_hash_table);
	  else
	    HashAdd(tempex,cc->htc.glbl_hash_table);
	}
      }
      switch (Bsf(tempex->type)) {
	case HTt_EXPORT_SYS_SYM:
pu_export_sys_sym:
	  if (PREC_TERM>*max_prec)
	    *max_prec=PREC_TERM;
	  if (!(tempex->type & (HTF_IMM|HTF_IMPORT)))
	    cc->abs_cnts.abs_addres++;
	  if (tempex->type & HTF_UNRESOLVED) {
	    if (!(cc->flags&CCF_ASM_EXPRESSIONS))
	      LexExcept(cc,"Illegal fwd ref at ");
	    tempauh=MAlloc(sizeof(CAsmUndefHash));
	    tempauh->hash=tempex;
	    tempauh->next=cc->asm_undef_hash;
	    cc->asm_undef_hash=tempauh;
	    if (tempex->type & HTF_LOCAL)
	      cc->flags|=CCF_UNRESOLVED|CCF_LOCAL;
	    else
	      cc->flags|=CCF_UNRESOLVED;
	    ICAdd(cc,IC_IMM_I64,
		  &tempex->val,cmp.internal_types[RT_PTR],ICF_NO_RIP);
	    ICAdd(cc,IC_DEREF,0,cmp.internal_types[RT_PTR]);
	  } else {
	    if (cc->flags&CCF_AOT_COMPILE && !(tempex->type & HTF_IMM))
	      ICAdd(cc,IC_ABS_ADDR,tempex->val,cmp.internal_types[RT_PTR]);
	    else {
	      if (tempex->type&HTF_IMM)
		cc->abs_cnts.c_addres++;
	      ICAdd(cc,IC_IMM_I64,tempex->val,cmp.internal_types[RT_PTR]);
	    }
	  }
	  Lex(cc);
	  return PE_MAYBE_MODIFIERS;
	case HTt_FUN:
	  if (PREC_TERM>*max_prec)
	    *max_prec=PREC_TERM;
	  Lex(cc);  //skip fun name
	  return PrsFunCall(cc,ps,FALSE,tempex);
	case HTt_GLBL_VAR:
	  if (PREC_TERM>*max_prec)
	    *max_prec=PREC_TERM;
	  tempg=tempex;
	  tempc=tempg->var_class+1;
	  cc->flags&=~(CCF_RAX|CCF_ARRAY|CCF_FUN_EXP);
	  if (tempg->flags&GVF_ARRAY) {
	    *_tempad=tempg->dim.next;
	    cc->flags|=CCF_ARRAY;
	  }
	  if (cc->flags&CCF_AOT_COMPILE) {
	    if (tempg->flags & GVF_EXTERN) //TODO
	      LexExcept(cc,"Feature not implemented ");
	    else {
	      if (tempg->flags & GVF_IMPORT)
		ICAdd(cc,IC_ADDR_IMPORT,tempg,tempc);
	      else {
		if (tempg->flags&GVF_DATA_HEAP)
		  ICAdd(cc,IC_HEAP_GLBL,tempg->heap_glbl,tempc);
		else
		  ICAdd(cc,IC_ABS_ADDR,tempg->data_addr_ip,tempc);
	      }
	    }
	  } else {
	    if (tempg->flags & GVF_EXTERN) {
	      cc->abs_cnts.externs++;
	      ICAdd(cc,IC_IMM_I64,&tempg->data_addr,tempc);
	      ICAdd(cc,IC_DEREF,0,tempc);
	    } else
	      ICAdd(cc,IC_IMM_I64,tempg->data_addr,tempc);
	  }
	  Lex(cc);
	  if (tempg->flags & GVF_FUN) {
	    PrsPopDeref(ps);
	    cc->flags|=CCF_FUN_EXP;
	    PrsPush2(ps,tempg->fun_ptr-tempg->fun_ptr->ptr_stars_cnt);
	  }
	  return PE_UNARY_MODIFIERS;
	case HTt_CLASS:
	  PrsOffsetOf(cc);
	  return PE_MAYBE_MODIFIERS;
	case HTt_KEYWORD:
	  switch (tempex(CHashGeneric *)->user_data0) {
	    case KW_SIZEOF:
	      if (PREC_TERM>*max_prec)
		*max_prec=PREC_TERM;
	      j=0;
	      while (Lex(cc)=='(')
		j++;
	      PrsSizeOf(cc);
	      while (j--) {
		if (cc->token!=')')
		  LexExcept(cc,"missing ')' at ");
		Lex(cc);
	      }
	      return PE_MAYBE_MODIFIERS;
	    case KW_OFFSET:
	      if (PREC_TERM>*max_prec)
		*max_prec=PREC_TERM;
	      j=0;
	      while (Lex(cc)=='(')
		j++;
	      PrsOffsetOf(cc);
	      while (j--) {
		if (cc->token!=')')
		  LexExcept(cc,"missing ')' at ");
		Lex(cc);
	      }
	      return PE_MAYBE_MODIFIERS;
	    case KW_DEFINED:
	      if (PREC_TERM>*max_prec)
		*max_prec=PREC_TERM;
	      j=0;
	      while (Lex(cc)=='(')
		j++;
	      if (cc->token==TK_IDENT &&
		    (cc->hash_entry || cc->local_var_entry))
		ICAdd(cc,IC_IMM_I64,TRUE,cmp.internal_types[RT_I64]);
	      else
		ICAdd(cc,IC_IMM_I64,FALSE,cmp.internal_types[RT_I64]);
	      Lex(cc);
	      while (j--) {
		if (cc->token!=')')
		  LexExcept(cc,"missing ')' at ");
		Lex(cc);
	      }
	      return PE_MAYBE_MODIFIERS;
	  }
      }
  }
  LexExcept(cc,"missing expression at ");
}

I64 PrsUnaryModifier(CCmpCtrl *cc,CPrsStk *ps,CMemberLst **_local_var,
	CArrayDim **_tempad,I64 *unary_post_prec)
{
  CHashClass *tempc,*tempc1;
  CHashFun *fun_ptr;
  CMemberLst *tempm=*_local_var;
  CIntermediateCode *tempi,*tempi1;
  CArrayDim *tempad1,tempad2;
  CCodeMisc *cm;
  I64 mode,old_flags;
  Bool was_paren=Btr(&cc->flags,CCf_PAREN);
  *_local_var=NULL;
  switch (cc->token) {
    case '.':
      if (tempm)
	tempm->reg=REG_NONE;
      goto um_join;

    case TK_DEREFERENCE:
      tempi=cc->coc.coc_root.last;
      if (!(cc->flags & (CCF_RAX | CCF_ARRAY)))
	ICAdd(cc,IC_DEREF+PREC_UNARY_PRE<<16,0,tempi->ic_class-1);
      else
	tempi->ic_class--;

	um_join:
      if (!*unary_post_prec)
	*unary_post_prec=PREC_TERM;
      tempc=cc->coc.coc_root.last->ic_class;
      if (!tempc->ptr_stars_cnt && cc->token=='.')
	LexExcept(cc,"Must be address, not value ");
      if (!(cc->flags & CCF_RAX))
	tempc--;
      if (!(tempc->type & HTT_CLASS))
	LexExcept(cc,"invalid class at ");
      if (Lex(cc)!=TK_IDENT ||
	    !(tempm=MemberFind(cc->cur_str,tempc)))
	LexExcept(cc,"invalid member at ");
      else if (cc->local_var_entry)
	cc->local_var_entry->use_cnt--;
      Lex(cc);	//skip member name
      tempc1=tempm->member_class+1;
      ICAdd(cc,IC_IMM_I64,tempm->offset,tempc1);
      cc->flags&=~(CCF_RAX|CCF_ARRAY|CCF_FUN_EXP);
      if (tempm->dim.next) {
	*_tempad=tempm->dim.next;
	cc->flags|=CCF_ARRAY;
      }
      if(tempm->flags & MLF_FUN) {
	PrsPopDeref(ps);
	PrsPush2(ps,tempm->fun_ptr-tempm->fun_ptr->ptr_stars_cnt);
	cc->flags|=CCF_FUN_EXP;
      }
      ICAdd(cc,IC_ADD,0,tempc1);
      return PE_UNARY_MODIFIERS;
    case '(':
      if (cc->flags & CCF_FUN_EXP) {
	if (!*unary_post_prec)
	  *unary_post_prec=PREC_TERM;
	return PrsFunCall(cc,ps,TRUE,PrsPop2(ps));
      }
      if (!*unary_post_prec)
	*unary_post_prec=PREC_TERM;
      if (Lex(cc)!=TK_IDENT)
	LexExcept(cc,"invalid class at ");
      if (Btr(&cc->flags,CCf_FUN_EXP))
	ps->ptr2--;
      cc->flags&=~CCF_ARRAY;
      tempc=cc->hash_entry;
      Lex(cc);
      mode=PRS0_TYPECAST|PRS1_NULL;
      tempc=PrsType(cc,&tempc,&mode,NULL,NULL,&fun_ptr,NULL,&tempad2,0);
      if (fun_ptr) {
	PrsPopDeref(ps);
	Bts(&cc->flags,CCf_FUN_EXP);
	PrsPush2(ps,fun_ptr);
	cm=COCMiscNew(cc,CMT_HASH_ENTRY);
	cm->h=fun_ptr;
      }
      if (*_tempad=tempad2.next) {
	cc->flags|=CCF_ARRAY;
	tempc++;
	cm=COCMiscNew(cc,CMT_ARRAY_DIM);
	cm->dim=*_tempad;
      }
      if (!(cc->flags&(CCF_RAX|CCF_ARRAY)))
	tempc++;
      tempi=cc->coc.coc_root.last;
      tempi->ic_class=tempc;
      ICAdd(cc,IC_HOLYC_TYPECAST,was_paren,tempc);
      if (cc->token!=')')
	LexExcept(cc,"missing ')' at ");
      Lex(cc);
      return PE_UNARY_MODIFIERS;
    case '[':
      if (!*unary_post_prec)
	*unary_post_prec=PREC_TERM;
      Lex(cc);
      tempc=OptClassFwd(cc->coc.coc_root.last->ic_class);
      if (!tempc->ptr_stars_cnt)
	LexExcept(cc,"not array or ptr ");
      if (!(cc->flags & (CCF_ARRAY | CCF_RAX))) {
	tempc=OptClassFwd(tempc-1);
	if (!tempc->ptr_stars_cnt)
	  LexExcept(cc,"not array or ptr ");
	ICAdd(cc,IC_DEREF+PREC_UNARY_PRE<<16,0,tempc);
      }
      tempc1=tempc-1;
      if (tempad1=*_tempad) {
	ICAdd(cc,IC_IMM_I64,tempad1->total_cnt*tempc1->size,tempc);
	if (*_tempad=tempad1->next) {
	  old_flags=cc->flags;
	  if (!PrsExpression(cc,NULL,FALSE,ps))
	    throw('Compiler');
	  cc->flags=cc->flags&~CCF_FUN_EXP|old_flags&CCF_FUN_EXP;
	  if (cc->token!=']')
	    LexExcept(cc,"missing ']' at ");
	  Lex(cc); //skip ]
	  tempi1=cc->coc.coc_root.last;
	  tempi1->ic_flags|=ICF_R_TO_INT;
	  ICAdd(cc,IC_MUL,0,tempc);
	  ICAdd(cc,IC_ADD,0,tempc);
	  cc->flags|=CCF_RAX;
	  return PE_UNARY_MODIFIERS;
	}
      } else
	ICAdd(cc,IC_IMM_I64,tempc1->size,tempc);
      old_flags=cc->flags;
      if (!PrsExpression(cc,NULL,FALSE,ps))
	throw('Compiler');
      cc->flags=cc->flags&~CCF_FUN_EXP|old_flags&CCF_FUN_EXP;
      if (cc->token!=']')
	LexExcept(cc,"missing ']' at ");
      Lex(cc); //skip ]
      tempi1=cc->coc.coc_root.last;
      tempi1->ic_flags|=ICF_R_TO_INT;
      ICAdd(cc,IC_MUL,0,tempc);
      ICAdd(cc,IC_ADD,0,tempc);
      cc->flags&=~(CCF_RAX|CCF_ARRAY);
      return PE_UNARY_MODIFIERS;
    sub_switch_start:
      case TK_PLUS_PLUS:
	cc->flags|=CCF_POSTINC;
	break;
      case TK_MINUS_MINUS:
	cc->flags|=CCF_POSTDEC;
	break;
    sub_switch_end:
      if (!*unary_post_prec)
	*unary_post_prec=PREC_UNARY_POST;
      Lex(cc);
      return PE_DEREFERENCE;
  }
  return PE_DEREFERENCE;
}

U8 *LexExpression2Bin(CCmpCtrl *cc,I64 *_type=NULL)
{//Compile cc expression. You call the code.
  U8 *result;
  I64 size;
  Bool old_trace=Btr(&cc->flags,CCf_PASS_TRACE_PRESENT);
  COCPush(cc);
  COCInit(cc);
  if (PrsExpression(cc,NULL,FALSE)) {
    ICAdd(cc,IC_RETURN_VAL,0,0);
    ICAdd(cc,IC_RET,0,0);
    result=COCCompile(cc,&size,NULL,_type);
  } else
    result=NULL;
  COCPop(cc);
  BEqu(&cc->flags,CCf_PASS_TRACE_PRESENT,old_trace);
  return result;
}

Bool IsLexExpression2Bin(CCmpCtrl *cc,U8 **_machine_code)
{//Compile cc expression to bin. Return err status.
  return ToBool(*_machine_code=LexExpression2Bin(cc));
}

I64 LexExpressionI64(CCmpCtrl *cc)
{//Compile cc expression, forcing to I64 and eval.
  U8 *machine_code;
  I64 result,type;
  if (machine_code=LexExpression2Bin(cc,&type)) {
    result=Call(machine_code);
    Free(machine_code);
    if (type==RT_F64)
      result=ToI64(result(F64));
  } else
    result=0;
  return result;
}

F64 LexExpressionF64(CCmpCtrl *cc)
{//Compile cc expression, forcing to F64 and eval.
  U8 *machine_code;
  I64 result,type;
  if (machine_code=LexExpression2Bin(cc,&type)) {
    result=Call(machine_code);
    Free(machine_code);
    if (type!=RT_F64)
      result(F64)=ToF64(result);
  } else
    result=0;
  return result(F64);
}

I64 LexExpression(CCmpCtrl *cc)
{//Compile cc expression and eval.  Might be I64 or F64.
  U8 *machine_code;
  I64 result;
  if (machine_code=LexExpression2Bin(cc)) {
    result=Call(machine_code);
    Free(machine_code);
  } else
    result=0;
  return result;
}
