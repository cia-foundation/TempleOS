U0 ParenWarning(CLex *lx)
{
  if (Bt(&lx->opts,OPTf_WARN_PAREN) &&
      !(lx->cur_lfn->flags&LFSF_IS_DEFINE))
    LexPutWarn(lx,"ST_UNNECESSARY_PARENTHESIS");
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

#define PRST_UNARY_TERM		0
#define PRST_MAYBE_MODIFIERS	1
#define PRST_UNARY_MODIFIERS	2
#define PRST_DEREFERENCE	3
#define PRST_CHECK_BINARY_OPS	4

Bool PrsExpression(CLex *lx,I64 *_max_prec,Bool end_exp,CPrsStk *_ps=NULL)
{
  Bool result=TRUE,cont_unary,cont_check_binary_ops,is_lower,div_sizeof;
  I64 i,k,old_flags=lx->flags,old_expression_start_line_num=lx->expression_start_line_num,
      max_prec=PREC_NULL,unary_pre_prec,paren_prec,unary_post_prec,
      left_prec=PREC_MAX,quest=0;
  CIntermediateCode *tempi,*tempi2;
  CHashClass *tempc,*tempc1;
  CPrsStk *ps;
  CMemberLst *local_var;
  CArrayDim *tempad=NULL;
  if (!(lx->flags&LF_INSIDE_EXPRESSION)) {
    if (lx->fun_lfn)
      lx->expression_start_line_num=lx->fun_lfn->line_num;
    else
      lx->expression_start_line_num=1;
  }
  lx->flags|=LF_INSIDE_EXPRESSION;
  if (_ps)
    ps=_ps;
  else {
    ps=MAlloc(sizeof(CPrsStk));
    ps->ptr=0;
    ps->ptr2=0;
  }
  PrsPush(ps,0); //terminate
  PrsPush(ps,0); //terminate
  try {
    do {
      unary_pre_prec=PREC_NULL;
      lx->flags&=~(LF_PAREN+LF_PREINC+LF_PREDEC+LF_POSTINC+LF_POSTDEC+LF_FUN_EXP);
      do i=PrsUnaryTerm(lx,ps,&local_var,&tempad,&max_prec,&unary_pre_prec,&paren_prec);
      while (i==PRST_UNARY_TERM);

      unary_post_prec=PREC_NULL;
      while (TRUE) {
	if (i==PRST_UNARY_MODIFIERS)
	  i=PrsUnaryModifier(lx,ps,&local_var,&tempad,&unary_post_prec);
	else if (i==PRST_MAYBE_MODIFIERS) {
	  if (lx->token=='(') {
	    lx->flags|=LF_RAX;
	    i=PrsUnaryModifier(lx,ps,&local_var,&tempad,&unary_post_prec);
	  } else {
	    i=PRST_CHECK_BINARY_OPS;
	    break;
	  }
	} else
	  break;
      }

      if (paren_prec) {
	if (unary_pre_prec || unary_post_prec) {
	  if (paren_prec<=unary_pre_prec && !unary_post_prec)
	    ParenWarning(lx);
	  paren_prec=PREC_NULL;
	} else if (paren_prec<=PREC_UNARY_PRE+ASSOC_MASK)
	    ParenWarning(lx);
      }

      if (i==PRST_DEREFERENCE)
	PrsDereference(lx);

	//Check for binary Ops
      do {
	i=PrsPop(ps);
	tempc=PrsPop(ps);
	if (0<i.u8[2]<=PREC_UNARY_PRE+ASSOC_MASK) {
	  k=binary_ops_table[lx->token];
	  if (k.u16[0]==IC_POWER &&
	      i.u16[0]==IC_UNARY_MINUS) {
	    Lex(lx); //skip ` op
	    left_prec=k.i8[2];
	    PrsPush(ps,tempc);
	    PrsPush(ps,i);
	    PrsPush(ps,lx->cbh.cb_last_out_ptr->ic_class);
	    PrsPush(ps,k);
	    cont_check_binary_ops=FALSE;
	    cont_unary=TRUE;
	  } else {
	    tempi=lx->cbh.cb_last_out_ptr;
	    tempc=tempi->ic_class;
	    if (i.u16[0]==IC_DEREF && tempc->ptr_cnt)
	      tempc--;
	    else if (i.u16[0]==IC_ADDR) {
	      lx->abs_cnts.c_addres++;
	      if (Bt(deref_bitmap,tempi->ic_opcode))
		tempi->ic_opcode=IC_NOP;
	      tempc++;
	    }
	    while (tempc->fwd_class)
	      tempc=tempc->fwd_class;
	    if (i)
	      CBICAdd(lx,i,0,tempc);
	    cont_check_binary_ops=TRUE;
	  }
	} else {
	  PrsPush(ps,tempc);
	  PrsPush(ps,i);
	  i=lx->token;
	  k=binary_ops_table[i];
	  switch (k.u16[0]) {
	    case IC_ADD:
	    case IC_SUB:
	      tempi=lx->cbh.cb_last_out_ptr;
	      if (!tempi->ic_class->ptr_cnt &&
		  tempi->ic_class->sub_type!=IT_F64) {
		CBICAdd(lx,IC_SIZEOF,1,sys_internal_types[IT_I64]);
		CBICAdd(lx,IC_MUL,0,sys_internal_types[IT_I64]);
	      }
	      break;
	    case IC_AND_AND:
	    case IC_OR_OR:
	      CBICAdd(lx,IC_NOP,0,sys_internal_types[IT_I64]);
	      break;
	    case IC_QUESTION:
	      quest++;
	      break;
	    case IC_COLON:
	      if (!quest) //case statement colons or labels
		k=0;
	      else
		quest--;
	      break;
	  }
	  if (!k.u8[2]) {
	    if (paren_prec>PREC_UNARY_PRE+ASSOC_MASK &&
		paren_prec&~ASSOC_MASK<=left_prec&~ASSOC_MASK-
		paren_prec&ASSOCF_LEFT-left_prec&ASSOCF_LEFT)
	      ParenWarning(lx);
	    cont_check_binary_ops=FALSE;
	    cont_unary=FALSE; //DONE
	  } else {
	    if (lx->flags & LF_FUN_EXP) {
	      ps->ptr2--;
	      lx->flags&=~LF_FUN_EXP;
	    }
	    Lex(lx); //skip op
	    if (paren_prec>PREC_UNARY_PRE+ASSOC_MASK &&
		paren_prec&~ASSOC_MASK<left_prec&~ASSOC_MASK+
		   paren_prec&ASSOCF_RIGHT &&
		 paren_prec&~ASSOC_MASK<k.u8[2]&~ASSOC_MASK+
		   !(paren_prec&ASSOCF_RIGHT))
		ParenWarning(lx);
	    if (k.u8[2]>max_prec)
	      max_prec=k.u8[2];
	    left_prec=k.u8[2];
	    if (Bt(assign_bitmap,k.u16[0])) {
	      tempi=lx->cbh.cb_last_out_ptr;
	      tempc=tempi->ic_class;
	      while (tempc->fwd_class)
		tempc=tempc->fwd_class;
	      if (!Bt(deref_bitmap,tempi->ic_opcode) ||
		!tempc->ptr_cnt && !Bt(&tempc->flags,Cf_INTERNAL_TYPE))
		LexExcept(lx,"ST_INVALID_LVAL");
	      tempi->ic_opcode=IC_NOP;
	      k.u8[2]=PREC_ASSIGN|ASSOCF_RIGHT;
	    }
	    while (TRUE) {
	      i=PrsPop(ps); //pop ops of higher prec
	      tempc=PrsPop(ps);
	      is_lower=FALSE;
	      if (i) {
		if (k.u8[2]&~ASSOC_MASK==i.u8[2]&~ASSOC_MASK) {
		  if (!(k.u8[2]&ASSOCF_RIGHT))
		    is_lower=TRUE;
		} else if (k.u8[2]&~ASSOC_MASK>i.u8[2]&~ASSOC_MASK)
		  is_lower=TRUE;
	      }
	      div_sizeof=FALSE;
	      if (is_lower) {
		switch (i.u16[0]) {
		  case IC_ADD:
		    if (tempc->ptr_cnt) {
		      tempi=lx->cbh.cb_last_out_ptr;
		      if (!tempi->ic_class->ptr_cnt &&
			  tempi->ic_class->sub_type!=IT_F64) {
			CBICAdd(lx,IC_SIZEOF,1,sys_internal_types[IT_I64]);
			CBICAdd(lx,IC_MUL,0,sys_internal_types[IT_I64]);
		      }
		    }
		    break;
		  case IC_SUB:
		    if (tempc->ptr_cnt) {
		      tempi=lx->cbh.cb_last_out_ptr;
		      if (!tempi->ic_class->ptr_cnt &&
			  tempi->ic_class->sub_type!=IT_F64) {
			CBICAdd(lx,IC_SIZEOF,1,sys_internal_types[IT_I64]);
			CBICAdd(lx,IC_MUL,0,sys_internal_types[IT_I64]);
		      } else
			div_sizeof=TRUE;
		    }
		    break;
		  case IC_AND_AND:
		  case IC_OR_OR:
		    CBICAdd(lx,IC_NOP,0,sys_internal_types[IT_I64]);
		    break;
		  case IC_ADD_EQUAL:
		  case IC_SUB_EQUAL:
		    if (tempc->ptr_cnt) {
		      CBICAdd(lx,IC_SIZEOF,1,sys_internal_types[IT_I64]);
		      CBICAdd(lx,IC_MUL,0,sys_internal_types[IT_I64]);
		    }
		    break;
		}
		tempi2=CBICAdd(lx,i,0,tempc);
		if (i.u8[3]&ECF_HAS_PUSH_CMP) {
		  tempi2->ic_flags|=ICF_POP_CMP;
		  CBICAdd(lx,IC_NOP,0,sys_internal_types[IT_I64]);
		  CBICAdd(lx,IC_AND_AND,0,sys_internal_types[IT_I64],ICF_POP_CMP);
		}
		if (Bt(cmp_bitmap,k.u16[0]) && Bt(cmp_bitmap,i.u16[0])) {
		  tempi2->ic_flags|=ICF_PUSH_CMP;
		  CBICAdd(lx,IC_NOP,0,sys_internal_types[IT_I64]);
		  CBICAdd(lx,IC_PUSH_CMP,0,tempc);
		  k.u8[3]|=ECF_HAS_PUSH_CMP;
		} else if (k.u16[0]==IC_AND_AND || k.u16[0]==IC_OR_OR)
		  CBICAdd(lx,IC_NOP,0,sys_internal_types[IT_I64]);
		if (div_sizeof) {
		  tempc1=tempc-1;
		  if (tempc1->size!=1) {
		    CBICAdd(lx,IC_IMM,tempc1->size,sys_internal_types[IT_I64]);
		    CBICAdd(lx,IC_DIV,0,sys_internal_types[IT_I64]);
		    tempc=sys_internal_types[IT_I64];
		  }
		}
	      } else {
		PrsPush(ps,tempc);
		PrsPush(ps,i);
		PrsPush(ps,lx->cbh.cb_last_out_ptr->ic_class);
		PrsPush(ps,k);
		cont_check_binary_ops=FALSE;
		cont_unary=TRUE;
		break;
	      }
	    }
	  }
	}
      } while (cont_check_binary_ops);
    } while (cont_unary);
    while (TRUE) {
      i=PrsPop(ps);
      tempc=PrsPop(ps);
      div_sizeof=FALSE;
      tempi=lx->cbh.cb_last_out_ptr;
      switch (i.u16[0]) {
	case 0:
	  goto done3;
	case IC_ADD:
	  if (tempc->ptr_cnt && !tempi->ic_class->ptr_cnt &&
	      tempi->ic_class->sub_type!=IT_F64) {
	    CBICAdd(lx,IC_SIZEOF,1,sys_internal_types[IT_I64]);
	    CBICAdd(lx,IC_MUL,0,sys_internal_types[IT_I64]);
	  }
	  break;
	case IC_SUB:
	  if (tempc->ptr_cnt && tempi->ic_class->sub_type!=IT_F64) {
	    if (!tempi->ic_class->ptr_cnt) {
	      CBICAdd(lx,IC_SIZEOF,1,sys_internal_types[IT_I64]);
	      CBICAdd(lx,IC_MUL,0,sys_internal_types[IT_I64]);
	    } else
	      div_sizeof=TRUE;
	  }
	  break;
	case IC_AND_AND:
	case IC_OR_OR:
	  CBICAdd(lx,IC_NOP,0,sys_internal_types[IT_I64]);
	  break;
	case IC_ADD_EQUAL:
	case IC_SUB_EQUAL:
	  if (tempc->ptr_cnt) {
	    CBICAdd(lx,IC_SIZEOF,1,sys_internal_types[IT_I64]);
	    CBICAdd(lx,IC_MUL,0,sys_internal_types[IT_I64]);
	  }
	  break;
      }
      tempi=CBICAdd(lx,i,0,tempc);
      if (i.u8[3]&ECF_HAS_PUSH_CMP) {
	tempi->ic_flags|=ICF_POP_CMP;
	CBICAdd(lx,IC_NOP,0,sys_internal_types[IT_I64]);
	CBICAdd(lx,IC_AND_AND,0,sys_internal_types[IT_I64],ICF_POP_CMP);
      }
      if (div_sizeof) {
	tempc1=tempc-1;
	if (tempc1->size!=1) {
	  CBICAdd(lx,IC_IMM,tempc1->size,sys_internal_types[IT_I64]);
	  CBICAdd(lx,IC_DIV,0,sys_internal_types[IT_I64]);
	  tempc=sys_internal_types[IT_I64];
	}
      }
    }
done3:
  } catch {
    if (Fs->except_argc>0) {
      if (Fs->except_argv[0]==EXCEPT_COMPILER) {
	lx->statement_cnt=0;
	lx->braces_cnt=0;
	lx->temp_indent_level=lx->indent_level=0;
	QueVectU8Del(lx->indent_que_vect);
	lx->indent_que_vect=NULL;
	result=FALSE;
	Fs->catch_except=TRUE;
      }
    }
  }
  if (!_ps) {
    if (ps->ptr)
      LexExcept(lx,"ST_PRS_INTERNAL_ERROR");
    Free(ps);
  }
  if (end_exp && result)
    CBICAdd(lx,IC_END_EXP,0,0);
  if (quest>0)
    LexExcept(lx,"ST_MISSING_COLON");
  if (_max_prec)
    *_max_prec=max_prec;
  lx->flags=lx->flags&~LF_INSIDE_EXPRESSION |
      old_flags&(LF_INSIDE_EXPRESSION|LF_PREINC|LF_PREDEC);
  lx->expression_start_line_num=old_expression_start_line_num;
  return result;
}

U0 PrsSizeOf(CLex *lx)
{
  CHashClass *tempc;
  CMemberLst *tempm;
  CDbgInfo *dbg;
  I64 i;
  if (lx->token!=TK_IDENT)
    LexExcept(lx,"ST_INVALID_CLASS");
  if (tempm=lx->local_var_entry) {
    tempc=tempm->member_class;
    i=tempc->size*tempm->dim.total_cnt;
    if (Lex(lx)=='.')
      goto pu_sizeof_member;
  } else {
    tempc=lx->hash_entry;
    if (!tempc || !(tempc->type & (HTT_CLASS|HTT_INTERNAL_TYPE|HTT_GLBL_VAR|HTT_FUN)))
      LexExcept(lx,"ST_INVALID_CLASS");
    if (tempc->type&HTT_FUN) {
      if (!(dbg=tempc(CHashFun *)->debug))
	LexExcept(lx,"ST_FUN_SIZE_NOT_DEFINED");
      i=dbg->body[dbg->max_line+1-dbg->min_line]-tempc(CHashFun *)->executable_addr;
      Lex(lx);
    } else {
      i=tempc->size;
      while (Lex(lx)=='.') {
pu_sizeof_member:
	if (!(tempc->type & (HTT_CLASS|HTT_GLBL_VAR)))
	  LexExcept(lx,"ST_INVALID_CLASS");
	else if (tempc->type & HTT_GLBL_VAR)
	  tempc=tempc(CHashGlblVar *)->var_class;
	if (Lex(lx)!=TK_IDENT || !(tempm=MemberFind(lx->ident,tempc)))
	  LexExcept(lx,"ST_INVALID_MEMBER");
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
  CBICAdd(lx,IC_IMM,i,sys_internal_types[IT_I64]);
}

U0 PrsOffsetOf(CLex *lx)
{
  CHashClass *tempc;
  CMemberLst *tempm;
  I64 i;
  if (lx->token!=TK_IDENT)
    LexExcept(lx,"ST_INVALID_CLASS");
  if (tempm=lx->local_var_entry)
    tempc=tempm->member_class;
  else {
    tempc=lx->hash_entry;
    if (!tempc || !(tempc->type & (HTT_CLASS|HTT_GLBL_VAR)))
      LexExcept(lx,"ST_INVALID_CLASS");
    else if (tempc->type & HTT_GLBL_VAR)
      tempc=tempc(CHashGlblVar *)->var_class;
  }
  if (Lex(lx)!='.')
    LexExcept(lx,"ST_EXPECTING_DOT");

  i=0;
  do {
    if (Lex(lx)!=TK_IDENT || !(tempm=MemberFind(lx->ident,tempc)))
      LexExcept(lx,"ST_INVALID_MEMBER");
    else if (lx->local_var_entry)
      lx->local_var_entry->use_cnt--;
    i+=tempm->offset;
    tempc=tempm->member_class;
  } while (Lex(lx)=='.');
  CBICAdd(lx,IC_IMM,i,sys_internal_types[IT_I64]);
}

I64 PrsFunCall(CLex *lx,CPrsStk *ps,Bool indirect,CHashFun *tempf)
{
  I64 i,argc_cnt;
  Bool is_first_arg=TRUE,needs_right_paren,is_str_out,is_putchar,is_template_fun;
  CHashClass *tempc1,*tempc2;
  CMemberLst *tempm;
  CLexCodeBlkCtrl *tempcbh,*tempcbh1;
  CCodeBlkMisc *mc;
  CIntermediateCode *tempi;

  if (!tempf) {
    if (lx->token==TK_U8) {
      if (!(tempf=HashFind("PutChar",lx->hash_table_lst,HTT_FUN)))
	LexExcept(lx,"ST_HEADER_FOR_PRINTF");
      if (!lx->cur_i) //empty char signals PutChar with variable
	Lex(lx);
      is_str_out=FALSE;
      is_putchar=TRUE;
    } else {
      if (!(tempf=HashFind("PrintF",lx->hash_table_lst,HTT_FUN)))
	LexExcept(lx,"ST_HEADER_FOR_PRINTF");
      if (!*lx->ident) //empty string signals PrintF with variable fmt_str
	Lex(lx);
      is_putchar=FALSE;
      is_str_out=TRUE;
    }
  } else {
    is_str_out=FALSE;
    is_putchar=FALSE;
  }

  if (Bt(tempf->flags,Ff_INTERNAL)&& IC_SQR<=tempf->executable_addr<=IC_ATAN)
    is_template_fun=TRUE;
  else
    is_template_fun=FALSE;

  if (indirect) {
    if (!(lx->flags & (LF_RAX | LF_ARRAY)))
      CBICAdd(lx,IC_DEREF,0,sys_internal_types[IT_PTR]);
    lx->cbh.cb_last_out_ptr->ic_class=sys_internal_types[IT_PTR];
    CBICAdd(lx,IC_SET_RAX,0,sys_internal_types[IT_PTR]);
    CBICAdd(lx,IC_NOP1,1,sys_internal_types[IT_PTR]); //balance the books
  }
  CBPush(lx);
  tempcbh=lx->cbh.cb_next;
  lx->cbh.cb_next=NULL;
  i=tempf->arg_cnt;
  tempm=tempf->member_lst;
  argc_cnt=0;
  if (lx->token=='(') {
    Lex(lx);
    needs_right_paren=TRUE;
  } else
    needs_right_paren=FALSE;
  while (i--) {
    CBOutPtrsInit(lx);
    if (!is_first_arg) {
      if (is_str_out) {
	if (lx->token==',')
	  Lex(lx);
	else if (lx->token!=';')
	  LexExcept(lx,"ST_EXPECTING_COMMA");
      } else {
	if (needs_right_paren) {
	  if (lx->token==',')
	    Lex(lx);
	  else if (lx->token!=')')
	    LexExcept(lx,"ST_EXPECTING_COMMA");
	}
      }
    }
    if (tempm->flags & MLF_DFT_AVAILABLE &&
	(lx->token==')' || lx->token==',' || !needs_right_paren)) {
      if (tempm->flags & MLF_STR_DFT_AVAILABLE &&
	  lx->flags&LF_AOT_COMPILE) {
	mc=CBMiscNew(lx,CBMT_STR_CONST);
	CBICAdd(lx,IC_STR_CONST,mc,sys_internal_types[IT_U8]+1);
	mc->st_len=StrLen(tempm->dft_val)+1;
	mc->str=StrNew(tempm->dft_val);
	lx->flags|=LF_HAS_MISC_DATA;
      } else
	CBICAdd(lx,IC_IMM,tempm->dft_val,tempm->member_class);
    } else {
      if (!PrsExpression(lx,NULL,FALSE,ps))
	throw(EXCEPT_COMPILER,3);
      else {
	CBPush(lx);
	CBICAdd(lx,IC_END,0,0);
	lx->pass=0;
	      OptPass012(lx,NULL,FALSE);
	lx->pass=1;
	tempi=OptPass012(lx,NULL,FALSE);
	CBPop(lx);
	tempc1=tempi->ic_class;
	while (tempc1->fwd_class)
	  tempc1=tempc1->fwd_class;
	tempc2=tempm->member_class;
	while (tempc2->fwd_class)
	  tempc2=tempc2->fwd_class;
	if (tempc2->sub_type==IT_F64 &&
	    tempc1->sub_type!=IT_F64)
	  tempi->ic_flags|=ICF_R_TO_F64;
	else
	if (tempc2->sub_type!=IT_F64 &&
	    tempc1->sub_type==IT_F64)
	  tempi->ic_flags|=ICF_R_TO_INT;
      }
    }
    CBICAdd(lx,IC_END,0,0);
    CBPush(lx);
    is_first_arg=FALSE;
    tempm=tempm->next;
  }
  if (is_str_out && lx->token==';') {
    if (!(tempf=HashFind("PutS",lx->hash_table_lst,HTT_FUN)))
      LexExcept(lx,"ST_HEADER_FOR_PRINTF");
    tempm=NULL;
  }
  if (tempm && tempm->flags & MLF_DOT_DOT_DOT) {
    CBOutPtrsInit(lx);
    tempi=CBICAdd(lx,IC_IMM,0,tempm->member_class);
    CBICAdd(lx,IC_END,0,0);
    CBPush(lx);
    if (is_str_out) {
      if (lx->token!=';') {
	do {
	  if (!is_first_arg) {
	    if (lx->token==',')
	      Lex(lx);
	    else
	      LexExcept(lx,"ST_EXPECTING_COMMA");
	  }
	  CBOutPtrsInit(lx);
	  if (!PrsExpression(lx,NULL,FALSE,ps))
	    throw(EXCEPT_COMPILER,4);
	  CBICAdd(lx,IC_END,0,0);
	  CBPush(lx);
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
	      LexExcept(lx,"ST_EXPECTING_COMMA");
	  }
	  CBOutPtrsInit(lx);
	  if (!PrsExpression(lx,NULL,FALSE,ps))
	    throw(EXCEPT_COMPILER,4);
	  CBICAdd(lx,IC_END,0,0);
	  CBPush(lx);
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
      LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
  }
  tempcbh1=tempcbh->cb_next;
  tempcbh->cb_next=lx->cbh.cb_next;
  lx->cbh.cb_next=tempcbh;
  CBPop(lx);
  tempcbh=lx->cbh.cb_next;
  lx->cbh.cb_next=tempcbh1;
  if (!is_template_fun)
    CBICAdd(lx,IC_CALL_START,tempf,0);
  if (indirect)
    CBICAdd(lx,IC_PUSH_REGS,1<<CREG_RAX,tempf->return_class);
  while (tempcbh) {
    tempcbh1=tempcbh->cb_next;
    CBAppend(lx,tempcbh);
    if (!Bt(tempf->flags,Ff_INTERNAL))
      lx->cbh.cb_last_out_ptr->ic_flags|=ICF_PUSH_RESULT;
    tempcbh=tempcbh1;
  }
  if (Bt(tempf->flags,Ff_INTERNAL))
    CBICAdd(lx,tempf->executable_addr,0,tempf->return_class);
  else {
    if (indirect)
      CBICAdd(lx,IC_CALL_INDIRECT,(argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
    else if (Bt(tempf->flags,Cf_EXTERN)) {
      lx->abs_cnts.externs++;
      if (lx->flags&LF_AOT_COMPILE) {
	if (tempf->type&HTF_IMPORT)
	  CBICAdd(lx,IC_CALL_IMPORT,tempf,tempf->return_class);
	else
	  CBICAdd(lx,IC_CALL_EXTERN,tempf,tempf->return_class);
      } else
	CBICAdd(lx,IC_CALL_INDIRECT2,&tempf->executable_addr,tempf->return_class);
    } else {
      CBICAdd(lx,IC_CALL,tempf->executable_addr,tempf->return_class);
    }
    if ((Bt(tempf->flags,Ff_RET1) || Bt(tempf->flags,Ff_ARGPOP)) && !Bt(tempf->flags,Ff_NOARGPOP)) {
      if (indirect) {
	CBICAdd(lx,IC_ADD_RSP1,(argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
	CBICAdd(lx,IC_ADD_RSP,8,tempf->return_class);
      } else
	CBICAdd(lx,IC_ADD_RSP1,(argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
    } else {
      if (indirect)
	argc_cnt++;
      CBICAdd(lx,IC_ADD_RSP,(argc_cnt+tempf->arg_cnt)<<3,tempf->return_class);
    }
  }
  if (!is_template_fun)
    CBICAdd(lx,IC_CALL_END,tempf,tempf->return_class);
  if (is_str_out||is_putchar)
    CBICAdd(lx,IC_END_EXP,0,0);
  lx->flags=(lx->flags|LF_RAX) & ~(LF_ARRAY|LF_FUN_EXP);
  return PRST_UNARY_MODIFIERS;
}

I64 PrsUnaryTerm(CLex *lx,CPrsStk *ps,CMemberLst **_local_var,CArrayDim **_tempad,
    I64 *max_prec,I64 *unary_pre_prec,I64 *paren_prec)
{
  I64 i,*i_ptr,j,d,size;
  CHashExport *tempess;
  CHashClass *tempc;
  CHashFun *tempf;
  CHashGlblVar *tempg;
  CMemberLst *tempm;
  CLexUndef *templu;
  CCodeBlkMisc *mc;
  Bool paren_warn;

  *_local_var=NULL;
  *paren_prec=PREC_NULL;
  switch (lx->token) {
    case TK_INTEGER:
    case TK_U8:
      if (PREC_TERM>*max_prec)
	*max_prec=PREC_TERM;
      if (lx->cur_i<0)
	CBICAdd(lx,IC_IMM,lx->cur_i,sys_internal_types[IT_U64]);
      else
	CBICAdd(lx,IC_IMM,lx->cur_i,sys_internal_types[IT_I64]);
      Lex(lx);
      return PRST_MAYBE_MODIFIERS;
    case TK_F64:
      if (PREC_TERM>*max_prec)
	*max_prec=PREC_TERM;
      i_ptr=&lx->cur_f;
      CBICAdd(lx,IC_F64,*i_ptr,sys_internal_types[IT_F64]);
      Lex(lx);
      return PRST_MAYBE_MODIFIERS;
    case TK_STR:
      if (PREC_TERM>*max_prec)
	*max_prec=PREC_TERM;
      d=LexExtStr(lx,&size);
      mc=CBMiscNew(lx,CBMT_STR_CONST);
      CBICAdd(lx,IC_STR_CONST,mc,sys_internal_types[IT_U8]+1);
      mc->st_len=size;
      mc->str=d;
      lx->flags|=LF_HAS_MISC_DATA;
      return PRST_MAYBE_MODIFIERS;
    case TK_INS_BINARY:
      if (PREC_TERM>*max_prec)
	*max_prec=PREC_TERM;
      d=lx->ident;
      size=lx->ident_len;
      lx->ident=NULL;
      mc=CBMiscNew(lx,CBMT_STR_CONST);
      CBICAdd(lx,IC_STR_CONST,mc,sys_internal_types[IT_U8]+1);
      mc->st_len=size;
      mc->str=d;
      lx->flags|=LF_HAS_MISC_DATA;
      Lex(lx);
      return PRST_MAYBE_MODIFIERS;
    case TK_INS_BINARY_TYPE:
    case TK_INS_BINARY_SIZE:
      if (PREC_TERM>*max_prec)
	*max_prec=PREC_TERM;
      CBICAdd(lx,IC_IMM,lx->cur_i,sys_internal_types[IT_U64]);
      Lex(lx);
      return PRST_MAYBE_MODIFIERS;
    case '(':
      if (Lex(lx)==TK_IDENT && lx->hash_entry && lx->hash_entry->type & (HTT_CLASS|HTT_INTERNAL_TYPE))
	LexExcept(lx,"ST_USE_POSTFIX_CAST");
      else {
	if (PREC_TERM>*max_prec)
	  *max_prec=PREC_TERM;
	if (lx->cur_lfn->flags&LFSF_IS_DEFINE)
	  paren_warn=FALSE;
	else
	  paren_warn=TRUE;
	if (!PrsExpression(lx,paren_prec,FALSE,ps))
	  throw(EXCEPT_COMPILER,2);
	if (!paren_warn)
	  *paren_prec=PREC_NULL;
	if (lx->token!=')')
	  LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
	Lex(lx); //skip )
	lx->flags= lx->flags & ~LF_ARRAY | LF_RAX | LF_PAREN;
	return PRST_UNARY_MODIFIERS;
      }
  }
  i=unary_ops_table[lx->token];
  if (i.u8[2]) {
    if (i.u8[2]>*max_prec)
      *max_prec=i.u8[2];
    *unary_pre_prec=i.u8[2];
    Lex(lx); //Skip op
    if (i.u16[0]==IC_ADDR && lx->token==TK_IDENT) {
      if (tempc=lx->hash_entry) {
	if (tempc->type & HTT_FUN) {
	  tempf=tempc;
	  if (!Bt(tempf->flags,Ff_INTERNAL)) {
	    if (Bt(tempf->flags,Cf_EXTERN)) {
	      if (lx->flags&LF_AOT_COMPILE) {
		if (lx->flags&LF_ASM_EXPRESSIONS) {
		  if (tempess=HashFind(tempf->str,lx->hash_table_lst,HTT_EXPORT_SYS_SYM))
		    goto pu_export_sys_sym;
		  else
		    goto pu_new_sys_sym;
		}
		LexExcept(lx,"ST_ADDR_EXTERN");
	      }
	      lx->abs_cnts.externs++;
	      CBICAdd(lx,IC_IMM,
		  &tempf->executable_addr,sys_internal_types[IT_PTR]);
	      CBICAdd(lx,IC_DEREF,0,
		  sys_internal_types[IT_PTR]);
	    } else {
	      if (lx->flags&LF_AOT_COMPILE) {
		CBICAdd(lx,IC_ABS_ADDR,
		    tempf->executable_addr,sys_internal_types[IT_PTR]);
		if (lx->flags&LF_ASM_EXPRESSIONS)
		  lx->abs_cnts.abs_addres++;
	      } else
		CBICAdd(lx,IC_IMM,
		    tempf->executable_addr,sys_internal_types[IT_PTR]);
	    }
	    lx->abs_cnts.c_addres++;
	    Lex(lx);
	    return PRST_MAYBE_MODIFIERS;
	  }
	} else if (tempc->type & HTT_EXPORT_SYS_SYM) {
	  tempess=tempc;
	  if (lx->flags&LF_ASM_EXPRESSIONS && !(lx->flags&LF_AOT_COMPILE) &&
	      tempess->type&HTF_LIT) {
	    lx->abs_cnts.c_addres++;
	    CBICAdd(lx,IC_IMM,
		tempess->val,sys_internal_types[IT_PTR]);
	    Lex(lx);
	    return PRST_MAYBE_MODIFIERS;
	  } else
	    goto pu_export_sys_sym;
	}
      } else if (lx->flags&LF_ASM_EXPRESSIONS && !lx->local_var_entry)
	goto pu_ident_but_not_local_var;
    }
    PrsPush(ps,tempc);
    PrsPush(ps,i);
    return PRST_UNARY_TERM;
  }
  if (lx->token=='+') {
    if (PREC_UNARY_PRE>*max_prec)
      *max_prec=PREC_UNARY_PRE;
    *unary_pre_prec=PREC_UNARY_PRE;
    Lex(lx);
    return PRST_UNARY_TERM;
  }

  if (lx->token!=TK_IDENT) {
    if (lx->token==TK_PLUS_PLUS) {
      if (PREC_UNARY_PRE>*max_prec)
	*max_prec=PREC_UNARY_PRE;
      *unary_pre_prec=PREC_UNARY_PRE;
      Lex(lx);
      lx->flags|=LF_PREINC;
      return PRST_UNARY_TERM;
    }
    if (lx->token==TK_MINUS_MINUS) {
      if (PREC_UNARY_PRE>*max_prec)
	*max_prec=PREC_UNARY_PRE;
      *unary_pre_prec=PREC_UNARY_PRE;
      Lex(lx);
      lx->flags|=LF_PREDEC;
      return PRST_UNARY_TERM;
    }
    if (lx->token=='$$') {
      if (PREC_TERM>*max_prec)
	*max_prec=PREC_TERM;
      if (lx->flags & LF_ASM_EXPRESSIONS) {
	lx->abs_cnts.abs_addres++;
	if (lx->flags&LF_AOT_COMPILE)
	  CBICAdd(lx,IC_ABS_ADDR,lx->a->ip,sys_internal_types[IT_PTR]);
	else
	  CBICAdd(lx,IC_IMM,lx->a->ip,sys_internal_types[IT_PTR]);
      } else {
	if (lx->flags&LF_CLASS_IP)
	  CBICAdd(lx,IC_IMM,lx->class_ip,sys_internal_types[IT_I64]);
	else
	  CBICAdd(lx,IC_RIP,0,sys_internal_types[IT_PTR]);
      }
      Lex(lx);
      return PRST_MAYBE_MODIFIERS;
    }
    LexExcept(lx,"ST_MISSING_EXPRESSION");
  }

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
      //	if (tempg->flags&GVAF_DATA_HEAP) //TODO
      //	  CBICAdd(lx,IC_HEAP_GLBL,tempm->static_data,tempc);
      //	else
	CBICAdd(lx,IC_ABS_ADDR,tempm->static_data_ip,tempc);
      } else
	CBICAdd(lx,IC_IMM,tempm->static_data,tempc);
    } else {
      if (lx->flags&LF_ASM_EXPRESSIONS) {
	i=PrsPop(ps);
	ps->ptr--;
	if (i.u16[0]!=IC_ADDR)
	  LexExcept(lx,"ST_EXPECTING_AMPERSAND");
	CBICAdd(lx,IC_IMM,tempm->offset,sys_internal_types[IT_PTR]);
	*_local_var=tempm;
	Lex(lx);
	return PRST_MAYBE_MODIFIERS;
      } else {
	if (tempm->dim.next && tempm->offset>0 && StrCmp(tempm->str,"argv")) {
	  tempc++;
	  lx->flags&=~LF_ARRAY;
	}
	CBICAdd(lx,IC_RBP,0,tempc);
	CBICAdd(lx,IC_IMM,tempm->offset,tempc);
	CBICAdd(lx,IC_ADD,0,tempc);
      }
    }
    Lex(lx); //skip var name
    *_local_var=tempm;
    return PRST_UNARY_MODIFIERS;
  }

pu_ident_but_not_local_var:
  if (!(tempess=lx->hash_entry)) {
    if (!(lx->flags & LF_ASM_EXPRESSIONS))
      LexExcept(lx,"ST_INVALID_LVAL");
    tempc=NULL;
pu_new_sys_sym:
    tempess=CAlloc(sizeof(CHashExport),Fs->code_heap);
    tempess->str=lx->ident;
    lx->ident=NULL;
    if (*tempess->str=='@' &&
	tempess->str[1]=='@') {
      tempess->type=HTT_EXPORT_SYS_SYM | HTF_UNRESOLVED | HTF_LOCAL;
      if (IsReplaceSyms)
	HashReplace(tempess,lx->local_hash_table);
      else
	HashAdd(tempess,lx->local_hash_table);
    } else {
      tempess->type=HTT_EXPORT_SYS_SYM | HTF_UNRESOLVED;
      if (tempc)
	HashAddAfter(tempess,tempc,lx->glbl_hash_table);
      else if (IsReplaceSyms)
	HashReplace(tempess,lx->glbl_hash_table);
      else
	HashAdd(tempess,lx->glbl_hash_table);
    }
  }

  if (tempess->type & HTT_EXPORT_SYS_SYM) {
pu_export_sys_sym:
    if (PREC_TERM>*max_prec)
      *max_prec=PREC_TERM;
    if (!(tempess->type & (HTF_LIT|HTF_IMPORT)))
      lx->abs_cnts.abs_addres++;
    if (tempess->type & HTF_UNRESOLVED) {
      if (!(lx->flags&LF_ASM_EXPRESSIONS))
	LexExcept(lx,"ST_ILLEGAL_FWD_REF");
      templu=MAlloc(sizeof(CLexUndef));
      templu->hash=tempess;
      templu->next=lx->undef_hash_entry;
      lx->undef_hash_entry=templu;
      if (tempess->type & HTF_LOCAL)
	lx->flags|=LF_UNRESOLVED|LF_LOCAL;
      else
	lx->flags|=LF_UNRESOLVED;
      CBICAdd(lx,IC_IMM,&tempess->val,sys_internal_types[IT_PTR],ICF_NO_RIP);
      CBICAdd(lx,IC_DEREF,0,sys_internal_types[IT_PTR]);
    } else {
      if (lx->flags&LF_AOT_COMPILE && !(tempess->type & HTF_LIT))
	CBICAdd(lx,IC_ABS_ADDR,tempess->val,sys_internal_types[IT_PTR]);
      else {
	if (tempess->type&HTF_LIT)
	  lx->abs_cnts.c_addres++;
	CBICAdd(lx,IC_IMM,tempess->val,sys_internal_types[IT_PTR]);
      }
    }
    Lex(lx);
    return PRST_MAYBE_MODIFIERS;
  }

  if (tempess->type & HTT_FUN) {
    if (PREC_TERM>*max_prec)
      *max_prec=PREC_TERM;
    Lex(lx);  //skip fun name
    return PrsFunCall(lx,ps,FALSE,tempess);
  }
  if (tempess->type & HTT_GLBL_VAR) {
    if (PREC_TERM>*max_prec)
      *max_prec=PREC_TERM;
    tempg=tempess;
    tempc=tempg->var_class+1;
    lx->flags&=~(LF_RAX|LF_ARRAY|LF_FUN_EXP);
    if (tempg->flags&GVAF_ARRAY) {
      *_tempad=tempg->dim.next;
      lx->flags|=LF_ARRAY;
    }
    if (lx->flags&LF_AOT_COMPILE) {
      if (tempg->flags & GVAF_EXTERN) //TODO
	LexExcept(lx,"ST_NOT_IMPLEMENTED");
      else {
	if (tempg->flags & GVAF_IMPORT)
	  CBICAdd(lx,IC_ADDR_IMPORT,tempg,tempc);
	else {
	  if (tempg->flags&GVAF_DATA_HEAP)
	    CBICAdd(lx,IC_HEAP_GLBL,tempg->heap_glbl,tempc);
	  else
	    CBICAdd(lx,IC_ABS_ADDR,tempg->data_addr_ip,tempc);
	}
      }
    } else {
      if (tempg->flags & GVAF_EXTERN) {
	lx->abs_cnts.externs++;
	CBICAdd(lx,IC_IMM,&tempg->data_addr,tempc);
	CBICAdd(lx,IC_DEREF,0,tempc);
      } else
	CBICAdd(lx,IC_IMM,tempg->data_addr,tempc);
    }
    Lex(lx);
    if (tempg->flags & GVAF_FUN) {
      PrsPopDeref(ps);
      lx->flags|=LF_FUN_EXP;
      PrsPush2(ps,tempg->fun_ptr-tempg->fun_ptr->ptr_cnt);
    }
    return PRST_UNARY_MODIFIERS;
  } else if (tempess->type & HTT_CLASS) {
    PrsOffsetOf(lx);
    return PRST_MAYBE_MODIFIERS;
  }

  if (!(tempess->type & HTT_KEYWORD))
    LexExcept(lx,"ST_MISSING_EXPRESSION");
  switch (tempess(CHashGeneric *)->user_data0) {
    case KW_SIZEOF:
      if (PREC_TERM>*max_prec)
	*max_prec=PREC_TERM;
      Lex(lx); //skip sizeof
      j=0;
      while (lx->token=='(') {
	Lex(lx);
	j++;
      }
      PrsSizeOf(lx);
      while (j--) {
	if (lx->token!=')')
	  LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
	Lex(lx);
      }
      return PRST_MAYBE_MODIFIERS;
    case KW_OFFSET:
      if (PREC_TERM>*max_prec)
	*max_prec=PREC_TERM;
      Lex(lx); //skip offset
      j=0;
      while (lx->token=='(') {
	Lex(lx);
	j++;
      }
      PrsOffsetOf(lx);
      while (j--) {
	if (lx->token!=')')
	  LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
	Lex(lx);
      }
      return PRST_MAYBE_MODIFIERS;
    case KW_DEFINED:
      if (PREC_TERM>*max_prec)
	*max_prec=PREC_TERM;
      Lex(lx); //skip defined
      j=0;
      while (lx->token=='(') {
	Lex(lx);
	j++;
      }
      if (lx->token==TK_IDENT &&
	  (lx->hash_entry || lx->local_var_entry))
	CBICAdd(lx,IC_IMM,TRUE,sys_internal_types[IT_I64]);
      else
	CBICAdd(lx,IC_IMM,FALSE,sys_internal_types[IT_I64]);
      Lex(lx);
      while (j--) {
	if (lx->token!=')')
	  LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
	Lex(lx);
      }
      return PRST_MAYBE_MODIFIERS;
  }
  LexExcept(lx,"ST_MISSING_EXPRESSION");
}

I64 PrsUnaryModifier(CLex *lx,CPrsStk *ps,CMemberLst **_local_var,CArrayDim **_tempad,I64 *unary_post_prec)
{
  CHashClass *tempc,*tempc1;
  CHashFun *fun_ptr;
  CMemberLst *tempm=*_local_var;
  CIntermediateCode *tempi,*tempi1;
  CArrayDim *tempad1,tempad2;
  CCodeBlkMisc *mc;
  I64 mode,old_flags;
  Bool was_paren=Btr(&lx->flags,Lf_PAREN);
  if (lx->token==TK_DEREFERENCE) {
    if (!*unary_post_prec)
      *unary_post_prec=PREC_TERM;
    tempi=lx->cbh.cb_last_out_ptr;
    if (!(lx->flags & (LF_RAX | LF_ARRAY)))
      CBICAdd(lx,IC_DEREF+PREC_UNARY_PRE<<16,0,tempi->ic_class-1);
    else
      tempi->ic_class--;
  } else if (lx->token=='.') {
    if (!*unary_post_prec)
      *unary_post_prec=PREC_TERM;
    if (tempm)
      tempm->register=CREG_NONE;
  }
  *_local_var=NULL;
  switch (lx->token) {
    case '.':
    case TK_DEREFERENCE:
      if (!*unary_post_prec)
	*unary_post_prec=PREC_TERM;
      tempc=lx->cbh.cb_last_out_ptr->ic_class;
      if (!tempc->ptr_cnt && lx->token=='.')
	LexExcept(lx,"ST_ADD_NOT_VAL");
      if (!(lx->flags & LF_RAX))
	tempc--;
      if (!(tempc->type & HTT_CLASS))
	LexExcept(lx,"ST_INVALID_CLASS");
      if (Lex(lx)!=TK_IDENT ||
	 !(tempm=MemberFind(lx->ident,tempc)))
	LexExcept(lx,"ST_INVALID_MEMBER");
      else if (lx->local_var_entry)
	lx->local_var_entry->use_cnt--;
      Lex(lx);	//skip member name
      tempc1=tempm->member_class+1;
      CBICAdd(lx,IC_IMM,tempm->offset,tempc1);
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
      CBICAdd(lx,IC_ADD,0,tempc1);
      return PRST_UNARY_MODIFIERS;
    case '(':
      if (lx->flags & LF_FUN_EXP) {
	if (!*unary_post_prec)
	  *unary_post_prec=PREC_TERM;
	return PrsFunCall(lx,ps,TRUE,PrsPop2(ps));
      }
      if (!*unary_post_prec)
	*unary_post_prec=PREC_TERM;
      if (Lex(lx)!=TK_IDENT)
	LexExcept(lx,"ST_INVALID_CLASS");
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
	mc=CBMiscNew(lx,CBMT_HASH_ENTRY);
	mc->h=fun_ptr;
      }
      if (*_tempad=tempad2.next) {
	lx->flags|=LF_ARRAY;
	tempc++;
	mc=CBMiscNew(lx,CBMT_ARRAY_DIM);
	mc->dim=*_tempad;
      }
      if (!(lx->flags&(LF_RAX|LF_ARRAY)))
	tempc++;
      tempi=lx->cbh.cb_last_out_ptr;
      tempi->ic_class=tempc;
      CBICAdd(lx,IC_NATIVE_TYPECAST,was_paren,tempc);
      if (lx->token!=')')
	LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
      Lex(lx);
      return PRST_UNARY_MODIFIERS;
    case '[':
      if (!*unary_post_prec)
	*unary_post_prec=PREC_TERM;
      Lex(lx);
      tempc=lx->cbh.cb_last_out_ptr->ic_class;
      while (tempc->fwd_class)
	tempc=tempc->fwd_class;
      if (!tempc->ptr_cnt)
	LexExcept(lx,"ST_NOT_ARRAY_OR_PTR");
      if (!(lx->flags & (LF_ARRAY | LF_RAX))) {
	tempc--;
	while (tempc->fwd_class)
	  tempc=tempc->fwd_class;
	if (!tempc->ptr_cnt)
	  LexExcept(lx,"ST_NOT_ARRAY_OR_PTR");
	CBICAdd(lx,IC_DEREF+PREC_UNARY_PRE<<16,0,tempc);
      }
      tempc1=tempc-1;
      if (tempad1=*_tempad) {
	CBICAdd(lx,IC_IMM,tempad1->total_cnt*tempc1->size,tempc);
	if (*_tempad=tempad1->next) {
	  old_flags=lx->flags;
	  if (!PrsExpression(lx,NULL,FALSE,ps))
	    throw(EXCEPT_COMPILER,5);
	  lx->flags=lx->flags&~LF_FUN_EXP|old_flags&LF_FUN_EXP;
	  if (lx->token!=']')
	    LexExcept(lx,"ST_MISSING_RIGHT_BRACKET");
	  Lex(lx); //skip ]
	  tempi1=lx->cbh.cb_last_out_ptr;
	  tempi1->ic_flags|=ICF_R_TO_INT;
	  CBICAdd(lx,IC_MUL,0,tempc);
	  CBICAdd(lx,IC_ADD,0,tempc);
	  lx->flags|=LF_RAX;
	  return PRST_UNARY_MODIFIERS;
	}
      } else
	CBICAdd(lx,IC_IMM,tempc1->size,tempc);
      old_flags=lx->flags;
      if (!PrsExpression(lx,NULL,FALSE,ps))
	throw(EXCEPT_COMPILER,5);
      lx->flags=lx->flags&~LF_FUN_EXP|old_flags&LF_FUN_EXP;
      if (lx->token!=']')
	LexExcept(lx,"ST_MISSING_RIGHT_BRACKET");
      Lex(lx); //skip ]
      tempi1=lx->cbh.cb_last_out_ptr;
      tempi1->ic_flags|=ICF_R_TO_INT;
      CBICAdd(lx,IC_MUL,0,tempc);
      CBICAdd(lx,IC_ADD,0,tempc);
      lx->flags&=~(LF_RAX|LF_ARRAY);
      return PRST_UNARY_MODIFIERS;
    case TK_PLUS_PLUS:
      if (!*unary_post_prec)
	*unary_post_prec=PREC_UNARY_POST;
      Lex(lx);
      lx->flags|=LF_POSTINC;
      return PRST_DEREFERENCE;
    case TK_MINUS_MINUS:
      if (!*unary_post_prec)
	*unary_post_prec=PREC_UNARY_POST;
      Lex(lx);
      lx->flags|=LF_POSTDEC;
      return PRST_DEREFERENCE;
  }
  return PRST_DEREFERENCE;
}

U0 PrsDereference(CLex *lx)
{
  I64 i;
  CHashClass *tempc;
  CIntermediateCode *tempi;
  if (!(lx->flags&(LF_PREINC|LF_PREDEC|LF_POSTINC|LF_POSTDEC)))
    i=IC_DEREF+PREC_UNARY_PRE<<16;
  else {
    if (lx->flags & LF_PREINC)
      i=IC_PP_+PREC_UNARY_PRE<<16;
    else if (lx->flags & LF_PREDEC)
      i=IC_MM_+PREC_UNARY_PRE<<16;
    else if (lx->flags & LF_POSTINC)
      i=IC__PP+PREC_UNARY_POST<<16;
    else
      i=IC__MM+PREC_UNARY_POST<<16;
    lx->flags&=~(LF_PREINC|LF_PREDEC|LF_POSTINC|LF_POSTDEC);
  }
  tempi=lx->cbh.cb_last_out_ptr;
  if (lx->flags & (LF_RAX|LF_ARRAY)) {
    if (tempi->ic_opcode==IC_DEREF)
      tempi->ic_opcode=i;
  } else {
    tempc=tempi->ic_class-1;
    while (tempc->fwd_class)
      tempc=tempc->fwd_class;
    CBICAdd(lx,i,0,tempc);
  }
}

U8 *CmpExpression(CLex *lx,I64 *type=NULL)
{
  U8 *result;
  I64 size;
  Bool old_trace=Btr(&lx->flags,Lf_OPT_TRACE_PRESENT);
  CBPush(lx);
  CBOutPtrsInit(lx);
  CBICAdd(lx,IC_START,0,0); //So we can scan back an instruction
  if (PrsExpression(lx,NULL,FALSE)) {
    CBICAdd(lx,IC_RETURN_VAL,0,NULL);
    CBICAdd(lx,IC_RET,0,0);
    CBICAdd(lx,IC_END,0,0);
    result=CBCompile(lx,NULL,&size,NULL,NULL,type);
  } else
    result=NULL;
  CBPop(lx);
  BEqu(&lx->flags,Lf_OPT_TRACE_PRESENT,old_trace);
  return result;
}

Bool GetExpression(CLex *lx,U8 **_ex)
{
  return ToBool(*_ex=CmpExpression(lx));
}

I64 Expression(CLex *lx)
{
  I64 result;
  U8 *ex=CmpExpression(lx);
  if (ex) {
    result=Call(ex);
    ExpressionDel(ex);
  } else
    result=0;
  return result;
}
