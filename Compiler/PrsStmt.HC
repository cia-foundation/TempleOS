CHashClass *PrsClass(CCmpCtrl *cc,I64 keyword,I64 fsp_flags,Bool is_extern)
{
  CHashClass *tmpc,*base_class;
  if (cc->token!=TK_IDENT)
    LexExcept(cc,"Expecting identifier at ");
  if (is_extern) {
    tmpc=PrsClassNew;
    tmpc->str=cc->cur_str;
    cc->cur_str=NULL;
    HashAdd(tmpc,cc->htc.glbl_hash_table);
    LBts(&tmpc->flags,Cf_EXTERN);
    HashSrcFileSet(cc,tmpc);
    Lex(cc);
  } else {
    if (cc->flags&CCF_AOT_COMPILE)
      tmpc=HashFind(cc->cur_str,cc->htc.glbl_hash_table,HTT_CLASS);
    else
      tmpc=HashSingleTableFind(cc->cur_str,cc->htc.glbl_hash_table,HTT_CLASS);
    if (tmpc) {
      if (!Bt(&tmpc->flags,Cf_EXTERN))
	tmpc=NULL;
      else if (tmpc->use_cnt<3)
	UnusedExternWarning(cc,tmpc);
    }
    if (tmpc) {
      Free(tmpc->src_link);
      tmpc->src_link=NULL;
      Free(tmpc->idx);
      tmpc->idx=NULL;
    } else {
      tmpc=PrsClassNew;
      tmpc->str=cc->cur_str;
      cc->cur_str=NULL;
      HashAdd(tmpc,cc->htc.glbl_hash_table);
    }
    LBtr(&tmpc->flags,Cf_EXTERN);
    if (fsp_flags&FSF_PUBLIC)
      tmpc->type|=HTF_PUBLIC;
    tmpc->use_cnt=0;
    if (cc->last_U16=='\n')
      HashSrcFileSet(cc,tmpc,-1);
    else
      HashSrcFileSet(cc,tmpc,0);
    if (Lex(cc)==':') {
      if (Lex(cc)!=TK_IDENT || !(base_class=cc->hash_entry) ||
	    !(base_class->type&HTT_CLASS))
	LexExcept(cc,"Invalid class at ");
      if (Lex(cc)==',')
	LexExcept(cc,"Only one base class allowed at this time at ");
      tmpc->base_class=base_class;
      tmpc->size+=base_class->size;
    }
    if (keyword==KW_UNION)
      PrsVarLst(cc,tmpc,PRS0_NULL|PRS1_CLASS|PRSF_UNION);
    else
      PrsVarLst(cc,tmpc,PRS0_NULL|PRS1_CLASS);
    tmpc->size+=tmpc->neg_offset;
  }
  return tmpc;
}

CHashFun *PrsFunJoin(CCmpCtrl *cc,CHashClass *tmp_return,
	U8 *name,I64 fsp_flags)
{
  CMemberLst *tmpm,*header_lst;
  CAOTCtrl *aotc=cc->aotc;
  CHashClass *header_return;
  CHashFun *tmpf;
  I64 header_arg_cnt;
  if (name) {//if not fun_ptr
    if (cc->flags&CCF_AOT_COMPILE) {
      if ((tmpf=HashFind(name,cc->htc.glbl_hash_table,HTT_FUN)) &&
	    tmpf->type & HTF_IMPORT)
	tmpf=NULL;
    } else
      if ((tmpf=HashSingleTableFind(name,cc->htc.glbl_hash_table,HTT_FUN)) &&
	    !Bt(&tmpf->flags,Cf_EXTERN))
	tmpf=NULL;
    if (tmpf && tmpf->use_cnt<3)
      UnusedExternWarning(cc,tmpf);
  } else
    tmpf=NULL;
  if (tmpf) {
    tmpf->used_reg_mask=REGG_CLOBBERED+REGG_SAVED+REGG_STK_TMP;
    Free(tmpf->src_link);
    tmpf->src_link=NULL;
    Free(tmpf->idx);
    tmpf->idx=NULL;
    Free(name);
    header_arg_cnt=tmpf->arg_cnt;
    header_lst=tmpf->member_lst_and_root;
    header_return=tmpf->return_class;
    tmpf->member_lst_and_root=NULL;
    ClassMemberLstDel(tmpf);
  } else {
    tmpf=PrsFunNew;
    header_return=NULL;
    tmpf->used_reg_mask=REGG_CLOBBERED+REGG_SAVED+REGG_STK_TMP;
    tmpf->clobbered_reg_mask=REGG_CLOBBERED+REGG_STK_TMP;
    tmpf->str=name;
    if (cc->flags&CCF_AOT_COMPILE)
      tmpf->exe_addr=aotc->rip;
    else
      tmpf->exe_addr=&UndefinedExtern;
    LBts(&tmpf->flags,Cf_EXTERN);
    tmpf->flags|=fsp_flags&FSG_FUN_FLAGS1;
    if (name) //if not fun_ptr
      HashAdd(tmpf,cc->htc.glbl_hash_table);
  }
  BEqu(&tmpf->type,HTf_PUBLIC,fsp_flags&FSF_PUBLIC);
  tmpf->return_class=tmp_return;
  tmpf->use_cnt=0;
  HashSrcFileSet(cc,tmpf);
  PrsVarLst(cc,tmpf,PRS0_NULL|PRS1_FUN_ARG);
  tmpf->arg_cnt=tmpf->member_cnt;
  if (0<tmpf->arg_cnt<<3<=I16_MAX && !Bt(&tmpf->flags,Ff_DOT_DOT_DOT))
    LBts(&tmpf->flags,Ff_RET1);
  tmpm=tmpf->member_lst_and_root;
  while (tmpm) {
    tmpm->offset+=16; //RBP+RETURN
    tmpm=tmpm->next;
  }
  tmpf->size=0;
  if (header_return) {
    if (GetOption(OPTf_WARN_HEADER_MISMATCH)) {
      if (tmpf->return_class!=header_return) {
	PrintWarn("Fun Header return mismatch '%s'\n",tmpf->str);
	cc->warning_cnt++;
      }
      if (!MemberLstCmp(tmpf->member_lst_and_root,header_lst,header_arg_cnt)) {
	PrintWarn("Fun header args mismatch '%s'\n",tmpf->str);
	cc->warning_cnt++;
      }
    }
    MemberLstDel(header_lst);
  }
  return tmpf;
}

U0 PrsFun(CCmpCtrl *cc,CHashClass *tmp_return,U8 *name,I64 fsp_flags)
{
  CMemberLst *tmpm;
  CCodeMisc *saved_leave_label;
  I64 i,j,size,*r;
  Bool old_trace;

  cc->fun_lex_file=cc->lex_include_stk;
  cc->min_line=cc->max_line=cc->lex_include_stk->line_num;

  cc->flags&=~CCF_NO_REG_OPT;
  cc->htc.local_var_lst=cc->htc.fun=PrsFunJoin(cc,tmp_return,name,fsp_flags);

  COCPush(cc);
  Btr(&cc->flags,CCf_PASS_TRACE_PRESENT);
  COCInit(cc);
  ICAdd(cc,IC_ENTER,0,0);
  saved_leave_label=cc->lb_leave;
  cc->lb_leave=COCMiscNew(cc,CMT_LABEL);
  cc->flags&=~CCF_HAS_RETURN;
  PrsStmt(cc,,,0);

  if (cc->max_line<cc->min_line)
    cc->max_line=cc->min_line;

  if (cc->htc.fun->return_class->size && !(cc->flags&CCF_HAS_RETURN))
    LexWarn(cc,"Function should return val ");
  ICAdd(cc,IC_LABEL,cc->lb_leave,0);
  cc->lb_leave=saved_leave_label;
  ICAdd(cc,IC_LEAVE,0,cc->htc.fun->return_class);
  cc->htc.fun->size&=~7;
  if (cc->flags&CCF_AOT_COMPILE) {
    cc->htc.fun->exe_addr=cc->aotc->rip;
    cc->htc.fun->type|=HTF_EXPORT|HTF_RESOLVE;
    r=COCCompile(cc,&size,&cc->htc.fun->dbg_info,NULL);
    if (r) {
      j=(size+7)>>3;
      for (i=0;i<j;i++)
	AOTStoreCodeU64(cc,r[i]);
      Free(r);
    }
  } else {
    old_trace=Btr(&cc->opts,OPTf_TRACE);
    cc->htc.fun->exe_addr=COCCompile(
	  cc,&size,&cc->htc.fun->dbg_info,NULL);
    if (old_trace) {
      Bts(&cc->opts,OPTf_TRACE);
      Un(cc->htc.fun->exe_addr,size,64);
    }
    SysSymImportsResolve(cc->htc.fun->str);
  }
  LBtr(&cc->htc.fun->flags,Cf_EXTERN);
  COCPop(cc);
  tmpm=cc->htc.fun->member_lst_and_root;
  while (tmpm) {
    if (tmpm->flags & MLF_NO_UNUSED_WARN) {
      if (tmpm->use_cnt>1&&StrCmp(tmpm->str,"_anon_"))
	PrintWarn("Unneeded no_warn\n $$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
	      cc->lex_include_stk->full_name,cc->lex_include_stk->line_num,
	      tmpm->str,cc->htc.fun->str);
    } else if (!tmpm->use_cnt && GetOption(OPTf_WARN_UNUSED_VAR))
      PrintWarn("Unused var\n $$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
	    cc->lex_include_stk->full_name,cc->lex_include_stk->line_num,
	    tmpm->str,cc->htc.fun->str);
    tmpm=tmpm->next;
  }
  cc->htc.local_var_lst=cc->htc.fun=cc->fun_lex_file=NULL;
}

U0 PrsGlblVarLst(CCmpCtrl *cc,I64 saved_mode,CHashClass *saved_tmpc,
  I64 saved_val,I64 fsp_flags)
{
  I64 i,j,mode,k,val;
  U8 *st;
  CHashExport *tmpex;
  CHashGlblVar *tmpg;
  CAOTCtrl *aotc=cc->aotc;
  CAOTHeapGlbl *tmphg;
  CHashClass *tmpc;
  CHashFun *tmpf,*tmpf_fun_ptr;
  CArrayDim tmpad;
  Bool has_alias,undef_array_size,is_array;
  while (TRUE) {
    tmpc=PrsType(cc,&saved_tmpc,&saved_mode,NULL,&st,
	  &tmpf_fun_ptr,&tmpex,&tmpad,fsp_flags);

    if (!st) return;
    if (tmpad.next)
      is_array=TRUE;
    else if (tmpad.total_cnt<0) {
      is_array=TRUE;
      tmpc--;
    } else
      is_array=FALSE;

    val=saved_val;
    mode=saved_mode;
    if (tmpex && mode&255==PRS0_EXTERN && !(cc->flags&CCF_AOT_COMPILE) &&
	  tmpex->type&HTT_EXPORT_SYS_SYM) {
      val=tmpex->val;
      mode=PRS0__EXTERN|PRS1_NOT_REALLY__EXTERN;
    }
    if (cc->token=='(') {
      switch (mode&255) {
	case PRS0__INTERN:
	  tmpf=PrsFunJoin(cc,tmpc,st,fsp_flags);
	  tmpf->exe_addr=val;
	  Bts(&tmpf->flags,Ff_INTERNAL);
	  LBtr(&tmpf->flags,Cf_EXTERN);
	  return;
	case PRS0__EXTERN:
	  if (!(fsp_flags&FSF__) && !(mode&PRS1_NOT_REALLY__EXTERN))
	    LexExcept(cc,"Expecting label with underscore at ");
	  tmpf=PrsFunJoin(cc,tmpc,st,fsp_flags);
	  tmpf->exe_addr=val;
	  SysSymImportsResolve(tmpf->str);
	  LBtr(&tmpf->flags,Cf_EXTERN);
	  if (saved_mode&255==PRS0__EXTERN)
	    LBts(&tmpf->flags,Ff__EXTERN);
	  if (cc->flags&CCF_AOT_COMPILE)
	    tmpf->type|=HTF_RESOLVE;
	  return;
	case PRS0_EXTERN:
	  PrsFunJoin(cc,tmpc,st,fsp_flags);
	  return;
	case PRS0__IMPORT:
	  if (!(fsp_flags&FSF__))
	    LexExcept(cc,"Expecting label with underscore at ");
	case PRS0_IMPORT:
	  if (!(cc->flags&CCF_AOT_COMPILE))
	    LexExcept(cc,"import not needed at ");
	  else {
	    tmpf=PrsFunJoin(cc,tmpc,st,fsp_flags);
	    tmpf->type|=HTF_IMPORT;
	    if (mode&255==PRS0__IMPORT)
	      tmpf->import_name=StrNew(val);
	    else
	      tmpf->import_name=StrNew(st);
	  }
	  return;
	default:
	  PrsFun(cc,tmpc,st,fsp_flags);
	  return;
      }
    } else {
      if (tmpad.total_cnt<0) {
	i=0;
	undef_array_size=TRUE;
      } else {
	i=tmpad.total_cnt;
	undef_array_size=FALSE;
      }
      if (tmpf_fun_ptr)
	j=sizeof(U8 *);
      else
	j=tmpc->size;
      j*=i;
      has_alias=FALSE;
      tmphg=NULL;
      switch (mode&255) {
	case PRS0__EXTERN:
	  if (cc->flags&CCF_AOT_COMPILE) {
	    tmpg=CAlloc(sizeof(CHashGlblVar));
	    tmpg->data_addr_rip=val;
	    tmpg->type=HTT_GLBL_VAR | HTF_EXPORT;
	  } else {
	    tmpg=CAlloc(sizeof(CHashGlblVar),Fs->code_heap);
	    tmpg->data_addr=val;
	    tmpg->type=HTT_GLBL_VAR;
	  }
	  tmpg->flags|=GVF_ALIAS;
	  break;
	case PRS0__IMPORT:
	case PRS0_IMPORT:
	  if (!(cc->flags&CCF_AOT_COMPILE))
	    LexExcept(cc,"import not needed at ");
	  else {
	    tmpg=CAlloc(sizeof(CHashGlblVar));
	    tmpg->type=HTT_GLBL_VAR | HTF_IMPORT;
	    if (mode&255==PRS0__IMPORT)
	      tmpg->import_name=StrNew(val);
	    else
	      tmpg->import_name=StrNew(st);
	  }
	  break;
	case PRS0_EXTERN:
	  if (cc->flags&CCF_AOT_COMPILE) {
	    tmpg=CAlloc(sizeof(CHashGlblVar));
	    tmpg->type=HTT_GLBL_VAR;
	  } else {
	    tmpg=CAlloc(sizeof(CHashGlblVar),Fs->code_heap);
	    tmpg->type=HTT_GLBL_VAR|HTF_UNRESOLVED;
	  }
	  break;
	default:
	  if (cc->flags&CCF_AOT_COMPILE) {
	    if (Bt(&cc->opts,OPTf_GLBLS_ON_DATA_HEAP)) {
	      if (cc->token=='=')
		LexExcept(cc,"Can't init glbl var on data heap in AOT module ");
	      tmpg=CAlloc(sizeof(CHashGlblVar));
	      tmphg=tmpg->heap_glbl=CAlloc(sizeof(CAOTHeapGlbl));
	      tmphg->size=j;
	      tmphg->str=StrNew(st);
	      tmphg->next=aotc->heap_glbls;
	      aotc->heap_glbls=tmphg;
	      tmpg->flags=GVF_DATA_HEAP;
	      tmpg->type=HTT_GLBL_VAR; //TODO: HTF_EXPORT
	      if (tmpex && tmpex->type & HTT_GLBL_VAR) //TODO!! extern
		LexExcept(cc,"Feature not implemented ");
	    } else {
	      tmpg=CAlloc(sizeof(CHashGlblVar));
	      if (cc->token=='=')
		tmpg->data_addr=CAlloc(j);
	      if (tmpc->size>=8) //align
		while (aotc->rip&7)
		  AOTStoreCodeU8(cc,0);
	      else if (tmpc->size==4)
		while (aotc->rip&3)
		  AOTStoreCodeU8(cc,0);
	      else if (tmpc->size==2)
		while (aotc->rip&1)
		  AOTStoreCodeU8(cc,0);
	      tmpg->data_addr_rip=aotc->rip;
	      tmpg->type=HTT_GLBL_VAR | HTF_EXPORT;
	      if (tmpex && tmpex->type & HTT_GLBL_VAR)
		has_alias=TRUE;
	      for (k=0;k<j;k++)
		AOTStoreCodeU8(cc,0); //Init AOT glbl to zero.
	    }
	  } else {
	    if (Bt(&cc->opts,OPTf_GLBLS_ON_DATA_HEAP)) {
	      tmpg=CAlloc(sizeof(CHashGlblVar),Fs->code_heap);
	      tmpg->data_addr=MAlloc(j);
	      tmpg->flags=GVF_DATA_HEAP;
	    } else {
	      tmpg=CAlloc(sizeof(CHashGlblVar),Fs->code_heap);
	      tmpg->data_addr=MAlloc(j,Fs->code_heap);
	    }
	    tmpg->type=HTT_GLBL_VAR;
	    if (tmpex && tmpex->type&HTT_GLBL_VAR &&
		  tmpex->type&HTF_UNRESOLVED &&
		  MHeapCtrl(tmpex)==MHeapCtrl(tmpg))
	      has_alias=TRUE;
	    if (sys_var_init_flag)
	      MemSet(tmpg->data_addr,sys_var_init_val,j);
	  }
      }
      tmpg->dim.next=tmpad.next;
      if (fsp_flags&FSF_PUBLIC)
	tmpg->type|=HTF_PUBLIC;
      tmpg->var_class=tmpc;
      tmpg->str=st;
      tmpg->size=j;
      tmpg->dim.total_cnt=i;
      tmpg->use_cnt=0;
      if (cc->last_U16=='\n')
	HashSrcFileSet(cc,tmpg,-1);
      else
	HashSrcFileSet(cc,tmpg,0);
      if (mode&255==PRS0_IMPORT || mode&255==PRS0__IMPORT)
	tmpg->flags|=GVF_IMPORT;
      if (mode&255==PRS0_EXTERN)
	tmpg->flags|=GVF_EXTERN;
      if (tmpf_fun_ptr) {
	tmpg->fun_ptr=tmpf_fun_ptr;
	tmpg->flags|=GVF_FUN;
      }
      if (is_array)
	tmpg->flags|=GVF_ARRAY;
      HashAdd(tmpg,cc->htc.glbl_hash_table);
      if (!(cc->flags&CCF_AOT_COMPILE) && !(tmpg->flags&GVF_EXTERN))
	SysSymImportsResolve(tmpg->str);
      if (cc->token=='=') {
	if (undef_array_size) {
	  LexPush(cc);
	  LexPush(cc);
	  Lex(cc);
	  PrsGlblInit(cc,tmpg,1);
	  LexPopNoRestore(cc);
	  tmpg->size=tmpg->dim.total_cnt*tmpc->size;
	  if (tmphg)
	    tmphg->size=tmpg->size;
	  if (cc->flags&CCF_AOT_COMPILE)
	    for (k=0;k<tmpg->size;k++)
	      AOTStoreCodeU8(cc,0);
	  else
	    if (sys_var_init_flag)
	      MemSet(tmpg->data_addr,sys_var_init_val,k);
	  LexPopRestore(cc);
	}
	LexPush(cc);
	Lex(cc);
	PrsGlblInit(cc,tmpg,2);
	if (cc->flags&CCF_AOT_COMPILE)
	  for (k=0;k<tmpg->size;k++)
	    AOTStoreCodeU8At(cc,tmpg->data_addr_rip+k,tmpg->data_addr[k]);
	LexPopNoRestore(cc);
      }
      if (has_alias) {
	if (tmpex(CHashGlblVar *)->use_cnt<2) {
	  PrintWarn("Unused extern '%s'\n",tmpex(CHashGlblVar *)->str);
	  cc->warning_cnt++;
	}
	tmpex(CHashGlblVar *)->flags|=GVF_ALIAS;
	tmpex(CHashGlblVar *)->data_addr=tmpg->data_addr;
	tmpex(CHashGlblVar *)->data_addr_rip=tmpg->data_addr_rip;
      }
      if (cc->token==',')
	Lex(cc);
      else {
	if (cc->token!=';')
	  LexExcept(cc,"Missing ';' at");
	Lex(cc);
	return;
      }
    }
  }
}

U0 PrsIf(CCmpCtrl *cc,I64 try_cnt,CCodeMisc *lb_break)
{
  CCodeMisc *lb,*lb1;
  I64 k;
  if (cc->token!='(')
    LexExcept(cc,"Expecting '(' at ");
  Lex(cc);
  if (!PrsExpression(cc,NULL,FALSE))
    throw('Compiler');
  if (cc->token!=')')
    LexExcept(cc,"Missing ')' at ");
  Lex(cc);
  lb=COCMiscNew(cc,CMT_LABEL);
  ICAdd(cc,IC_BR_ZERO,lb,0);
  PrsStmt(cc,try_cnt,lb_break);
  k=PrsKeyWord(cc);
  if (k==KW_ELSE) {
    Lex(cc);
    lb1=COCMiscNew(cc,CMT_LABEL);
    ICAdd(cc,IC_JMP,lb1,0);
    ICAdd(cc,IC_LABEL,lb,0);
    PrsStmt(cc,try_cnt,lb_break);
    ICAdd(cc,IC_LABEL,lb1,0);
  } else
    ICAdd(cc,IC_LABEL,lb,0);
}

U0 PrsWhile(CCmpCtrl *cc,I64 try_cnt)
{
  CCodeMisc *lb,*lb_done;
  if (cc->token!='(')
    LexExcept(cc,"Expecting '(' at ");
  Lex(cc);
  lb=COCMiscNew(cc,CMT_LABEL);
  ICAdd(cc,IC_LABEL,lb,0);
  if (!PrsExpression(cc,NULL,FALSE))
    throw('Compiler');
  if (cc->token!=')')
    LexExcept(cc,"Missing ')' at ");
  Lex(cc);
  lb_done=COCMiscNew(cc,CMT_LABEL);
  ICAdd(cc,IC_BR_ZERO,lb_done,0);
  PrsStmt(cc,try_cnt,lb_done);
  ICAdd(cc,IC_JMP,lb,0);
  ICAdd(cc,IC_LABEL,lb_done,0);
}

U0 PrsDoWhile(CCmpCtrl *cc,I64 try_cnt)
{
  CCodeMisc *lb,*lb_done;
  lb=COCMiscNew(cc,CMT_LABEL);
  lb_done=COCMiscNew(cc,CMT_LABEL);
  ICAdd(cc,IC_LABEL,lb,0);
  PrsStmt(cc,try_cnt,lb_done);
  if (PrsKeyWord(cc)!=KW_WHILE)
    LexExcept(cc,"Missing 'while' at");
  if (Lex(cc)!='(')
    LexExcept(cc,"Expecting '(' at ");
  Lex(cc);
  if (!PrsExpression(cc,NULL,FALSE))
    throw('Compiler');
  if (cc->token!=')')
    LexExcept(cc,"Missing ')' at ");
  ICAdd(cc,IC_BR_NOT_ZERO,lb,0);
  ICAdd(cc,IC_LABEL,lb_done,0);
  if (Lex(cc)!=';')
    LexExcept(cc,"Missing ';' at");
  Lex(cc);
}

U0 PrsFor(CCmpCtrl *cc,I64 try_cnt)
{
  CCodeCtrl *tmpcbh;
  CCodeMisc *lb,*lb_done;

  if (cc->token!='(')
    LexExcept(cc,"Expecting '(' at ");
  Lex(cc);
  PrsStmt(cc,try_cnt);

  lb=COCMiscNew(cc,CMT_LABEL);
  ICAdd(cc,IC_LABEL,lb,0);
  if (!PrsExpression(cc,NULL,FALSE))
    throw('Compiler');
  lb_done=COCMiscNew(cc,CMT_LABEL);
  ICAdd(cc,IC_BR_ZERO,lb_done,0);
  if (cc->token!=';')
    LexExcept(cc,"Missing ';' at");
  Lex(cc);

  COCPush(cc);
  COCInit(cc);
  if (cc->token!=')')
    PrsStmt(cc,try_cnt,NULL,0);
  COCPush(cc);
  tmpcbh=COCPopNoFree(cc);
  COCPop(cc);
  if (cc->token!=')')
    LexExcept(cc,"Missing ')' at ");
  Lex(cc);

  PrsStmt(cc,try_cnt,lb_done);
  COCAppend(cc,tmpcbh);
  ICAdd(cc,IC_JMP,lb,0);
  ICAdd(cc,IC_LABEL,lb_done,0);
}

class CSubSwitch {
  CSubSwitch *next,*last;
  CCodeMisc *lb_start,*lb_break;
};

class CSwitchCase {
  CSwitchCase *next;
  CCodeMisc *label;
  I64 val;
  CSubSwitch *ss;
};

U0 PrsSwitch(CCmpCtrl *cc,I64 try_cnt)
{
  CSwitchCase *header=NULL,*tmps,*tmps1;	//Leaks on except
  CSubSwitch head,*tmpss;			//Leaks on except
  CCodeMisc *lb_dft,*lb_fwd_case,*mc_jt,*lb_entry,**jmp_table;
  CIntermediateCode *tmpi_sub,*tmpi_cmp,*tmpi_jmp,*tmpi_start;
  Bool dft_found=FALSE,nobound;
  I64 i,k_start=I64_MIN,k_end,lo=I64_MAX,hi=I64_MIN,range;

  if (cc->token=='(')
    nobound=FALSE;
  else if (cc->token=='[')
    nobound=TRUE;
  else
    LexExcept(cc,"Expecting '(' or '[' at ");
  Lex(cc);
  QueInit(&head);

  head.last->lb_break=COCMiscNew(cc,CMT_LABEL);
  head.last->lb_break->use_cnt++;
  lb_dft=COCMiscNew(cc,CMT_LABEL);
  lb_dft->use_cnt++;
  mc_jt=COCMiscNew(cc,CMT_JMP_TABLE);
  mc_jt->begin=COCMiscNew(cc,CMT_LABEL);
  mc_jt->begin->use_cnt++;
  if (!PrsExpression(cc,NULL,FALSE))
    throw('Compiler');
  tmpi_sub=ICAdd(cc,IC_IMM_I64,0,cmp.internal_types[RT_I64]);
  ICAdd(cc,IC_SUB,0,cmp.internal_types[RT_I64]);
  tmpi_cmp=ICAdd(cc,IC_IMM_I64,0,cmp.internal_types[RT_I64]);
  if (nobound) {
    ICAdd(cc,IC_NOBOUND_SWITCH,mc_jt,0);
    if (cc->token!=']')
      LexExcept(cc,"Missing ']' at ");
  } else {
    ICAdd(cc,IC_SWITCH,mc_jt,0);
    if (cc->token!=')')
      LexExcept(cc,"Missing ')' at ");
  }
  if (Lex(cc)!='{')
    LexExcept(cc,"Expecting '{' at ");
  Lex(cc);
  ICAdd(cc,IC_LABEL,mc_jt->begin,0);
  while (TRUE) {
    while (cc->token && cc->token!='}') {
sw_cont:
      switch (PrsKeyWord(cc)) {
	case KW_END:
	  goto sw_sub_end;
	case KW_START:
	  if (Lex(cc)==':')
	    Lex(cc);
	  else
	    LexExcept(cc,"Expecting ':' at ");
	  tmpss=MAlloc(sizeof(CSubSwitch));
	  QueIns(tmpss,head.last);
	  head.last->lb_break=COCMiscNew(cc,CMT_LABEL);
	  head.last->lb_break->use_cnt++;
	  lb_fwd_case=COCMiscNew(cc,CMT_LABEL);
	  tmpi_jmp=ICAdd(cc,IC_JMP,lb_fwd_case,0);

	  tmpss->lb_start=COCMiscNew(cc,CMT_LABEL);
	  tmpi_start=ICAdd(cc,IC_LABEL,tmpss->lb_start,0);
	  while (cc->token && cc->token!='}') {
	    switch (PrsKeyWord(cc)) {
	      case KW_END:
		OptFree(tmpi_jmp);
		goto sw_sub_end;
	      case KW_START:
	      case KW_CASE:
	      case KW_DFT:
		if (cc->coc.coc_head.last==tmpi_start) {
		  OptFree(tmpi_jmp);
		  tmpss->lb_start=NULL;
		} else {
		  ICAdd(cc,IC_RET,0,0);
		  ICAdd(cc,IC_LABEL,lb_fwd_case,0);
		  ICAdd(cc,IC_SUB_CALL,tmpss->lb_start,0);//In case fall-thru
		}
		goto sw_cont;
	      default:
		PrsStmt(cc,try_cnt);
	    }
	  }
	  break;
	case KW_CASE:
	  if (head.next!=&head) {
	    lb_fwd_case=COCMiscNew(cc,CMT_LABEL);
	    tmpi_jmp=ICAdd(cc,IC_JMP,lb_fwd_case,0);//In case fall-thru
	  }
	  Lex(cc);
	  lb_entry=COCMiscNew(cc,CMT_LABEL);
	  ICAdd(cc,IC_LABEL,lb_entry,0);
	  lb_entry->use_cnt++;
	  if (head.next!=&head) {
	    tmpss=head.next;
	    while (tmpss!=&head) {
	      if (tmpss->lb_start)
		ICAdd(cc,IC_SUB_CALL,tmpss->lb_start,0);
	      tmpss=tmpss->next;
	    }
	    ICAdd(cc,IC_LABEL,lb_fwd_case,0);
	  }
	  if (cc->token==':') {
	    if (k_start==I64_MIN)
	      k_start=0;
	    else
	      k_start++;
	  } else
	    k_start=LexExpressionI64(cc);
	  if (k_start<lo) lo=k_start;
	  if (k_start>hi) hi=k_start;
	  if (cc->token==':') {
	    Lex(cc);
	    tmps=MAlloc(sizeof(CSwitchCase));
	    tmps->label=lb_entry;
	    tmps->val=k_start;
	    tmps->next=header;
	    header=tmps;
	  } else if (cc->token==TK_ELLIPSIS) {
	    Lex(cc);
	    k_end=LexExpressionI64(cc);
	    if (cc->token==':') {
	      Lex(cc);
	      if (k_end<lo) lo=k_end;
	      if (k_end>hi) hi=k_end;
	      if (k_start>k_end)
		SwapI64(&k_start,&k_end);
	      for (i=k_start;i<=k_end;i++) {
		tmps=MAlloc(sizeof(CSwitchCase));
		tmps->label=lb_entry;
		tmps->val=i;
		tmps->next=header;
		header=tmps;
	      }
	      k_start=k_end;
	    } else
	      LexExcept(cc,"Expecting ':' at ");
	  } else
	    LexExcept(cc,"Expecting ':' at ");
	  break;
	case KW_DFT:
	  if (head.next!=&head) {
	    lb_fwd_case=COCMiscNew(cc,CMT_LABEL);
	    tmpi_jmp=ICAdd(cc,IC_JMP,lb_fwd_case,0);//In case fall-thru
	  }
	  Lex(cc);
	  ICAdd(cc,IC_LABEL,lb_dft,0);
	  if (cc->token==':')
	    Lex(cc);
	  else
	    LexExcept(cc,"Expecting ':' at ");
	  if (head.next!=&head) {
	    tmpss=head.next;
	    while (tmpss!=&head) {
	      if (tmpss->lb_start)
		ICAdd(cc,IC_SUB_CALL,tmpss->lb_start,0);
	      tmpss=tmpss->next;
	    }
	    ICAdd(cc,IC_LABEL,lb_fwd_case,0);
	  }
	  dft_found=TRUE;
	  break;
	default:
	  PrsStmt(cc,try_cnt,head.last->lb_break);
      }
    }
sw_sub_end:
    tmpss=head.last;
    ICAdd(cc,IC_LABEL,tmpss->lb_break,0);
    if (tmpss==&head) {
      if (cc->token!='}')
	LexExcept(cc,"Missing '}' at ");
      Lex(cc);
      break;
    } else {
      QueRem(tmpss);
      Free(tmpss);
      if (PrsKeyWord(cc)!=KW_END)
	LexExcept(cc,"Missing 'end' at ");
      if (Lex(cc)==':')
	Lex(cc);
      else
	LexExcept(cc,"Expecting ':' at ");
    }
  }
  if (!dft_found)
    ICAdd(cc,IC_LABEL,lb_dft,0);

  if (0<lo<=16)
    lo=0;
  range=hi-lo+1;
  if (lo>hi || !(0<range<=0xFFFF))
    LexExcept(cc,"switch range error at ");
  jmp_table=MAlloc((sizeof(CCodeMisc *)*range+0x1FF)&~0x1FF);
  MemSetI64(jmp_table,lb_dft,range);
  tmpi_sub->ic_data=lo;
  tmpi_cmp->ic_data=range;
  tmps=header;
  while (tmps) {
    tmps1=tmps->next;
    if (jmp_table[tmps->val-lo]!=lb_dft)
      LexExcept(cc,"Duplicate case at ");
    else
      jmp_table[tmps->val-lo]=tmps->label;
    Free(tmps);
    tmps=tmps1;
  }
  mc_jt->dft=lb_dft;
  mc_jt->jmp_table=jmp_table;
  mc_jt->range=range;
}

U0 PrsNoWarn(CCmpCtrl *cc)
{
  CMemberLst *tmpm;
  while (cc->token==TK_IDENT) {
    if (!(tmpm=cc->local_var_entry))
      LexExcept(cc,"Expecting local var at ");
    tmpm->flags|=MLF_NO_UNUSED_WARN;
    if (Lex(cc)==',')
      Lex(cc);
    else if (cc->token!=';')
      LexExcept(cc,"Expecting ',' at ");
  }
}

U0 PrsStreamBlk(CCmpCtrl *cc)
{
  CLexHashTableContext *htc=MAlloc(sizeof(CLexHashTableContext));
  CStreamBlk *tmpe=MAlloc(sizeof(CStreamBlk));
  tmpe->body=StrNew("");
  QueIns(tmpe,cc->last_stream_blk);
  COCPush(cc);
  QueInit(&cc->coc.coc_next_misc);

  MemCpy(htc,&cc->htc,sizeof(CLexHashTableContext));
  htc->old_flags=cc->flags;
  cc->htc.next=htc;
  cc->htc.fun=cc->htc.local_var_lst=NULL;
  cc->htc.define_hash_table=cc->htc.hash_table_lst=
	cc->htc.glbl_hash_table=cc->htc.local_hash_table=Fs->hash_table;
  cc->flags=cc->flags & ~(CCF_ASM_EXPRESSIONS|CCF_AOT_COMPILE) | CCF_EXE_BLK;
  if (cc->token=='{')
    Lex(cc);
  else
    LexExcept(cc,"Missing '}' at ");
  while (cc->token && cc->token!='}')
    ExeCmdLine(cc);

  MemCpy(&cc->htc,htc,sizeof(CLexHashTableContext));
  cc->flags=cc->flags&~CCF_EXE_BLK |
	htc->old_flags & (CCF_ASM_EXPRESSIONS|CCF_EXE_BLK|CCF_AOT_COMPILE);
  Free(htc);
  COCPop(cc);
  QueRem(tmpe);
  if (*tmpe->body)
    LexIncludeStr(cc,"StreamBlk",tmpe->body,FALSE);
  else
    Free(tmpe->body);
  Free(tmpe);
  Lex(cc); //Skip '}'
}

U0 PrsTryBlk(CCmpCtrl *cc,I64 try_cnt)
{
  CCodeMisc	*lb_catch,*lb_done,*lb_untry;
  CHashClass	*tmpc=cmp.internal_types[RT_PTR];
  CHashFun	*tmp_try=HashFind("SysTry",cc->htc.hash_table_lst,HTT_FUN),
	*tmp_untry=HashFind("SysUntry",cc->htc.hash_table_lst,HTT_FUN);

  if (!tmp_try || !tmp_untry)
    LexExcept(cc,"Missing header for SysTry() and SysUntry() at ");

  cc->flags|=CCF_NO_REG_OPT; //TODO:Currently no reg vars in funs with try/catch

  lb_catch=COCMiscNew(cc,CMT_LABEL);
  lb_done =COCMiscNew(cc,CMT_LABEL);
  lb_untry=COCMiscNew(cc,CMT_LABEL);

  ICAdd(cc,IC_CALL_START,0,0);
  ICAdd(cc,IC_GET_LABEL,lb_untry,tmpc,ICF_PUSH_RES);
  ICAdd(cc,IC_GET_LABEL,lb_catch,tmpc,ICF_PUSH_RES);
  if (Bt(&tmp_try->flags,Cf_EXTERN)) {
    cc->abs_cnts.externs++;
    if (cc->flags&CCF_AOT_COMPILE)
      ICAdd(cc,IC_CALL_IMPORT,tmp_try,tmpc);
    else
      ICAdd(cc,IC_CALL_INDIRECT2,&tmp_try->exe_addr,tmpc);
  } else
    ICAdd(cc,IC_CALL,tmp_try->exe_addr,tmpc);
  if ((Bt(&tmp_try->flags,Ff_RET1) ||
	Bt(&tmp_try->flags,Ff_ARGPOP)) && !Bt(&tmp_try->flags,Ff_NOARGPOP))
    ICAdd(cc,IC_ADD_RSP1,16,tmpc);
  else
    ICAdd(cc,IC_ADD_RSP,16,tmpc);
  ICAdd(cc,IC_CALL_END,0,tmpc);
  ICAdd(cc,IC_END_EXP,0,0,ICF_RES_NOT_USED);

  PrsStmt(cc,try_cnt+1);

  ICAdd(cc,IC_LABEL,lb_untry,0);
  ICAdd(cc,IC_CALL_START,0,0);
  if (Bt(&tmp_untry->flags,Cf_EXTERN)) {
    cc->abs_cnts.externs++;
    if (cc->flags&CCF_AOT_COMPILE)
      ICAdd(cc,IC_CALL_IMPORT,tmp_untry,tmpc);
    else
      ICAdd(cc,IC_CALL_INDIRECT2,&tmp_untry->exe_addr,tmpc);
  } else
    ICAdd(cc,IC_CALL,tmp_untry->exe_addr,tmpc);
  ICAdd(cc,IC_CALL_END,0,tmpc);
  ICAdd(cc,IC_END_EXP,0,0,ICF_RES_NOT_USED);

  ICAdd(cc,IC_JMP,lb_done,0);

  if (PrsKeyWord(cc)!=KW_CATCH)
    LexExcept(cc,"Missing 'catch' at");

  Lex(cc);
  ICAdd(cc,IC_LABEL,lb_catch,0);
  PrsStmt(cc,try_cnt+1);
  ICAdd(cc,IC_RET,0,tmpc);
  ICAdd(cc,IC_LABEL,lb_done,0);
}

Bool PrsStmt(CCmpCtrl *cc,I64 try_cnt=0,
  CCodeMisc *lb_break=NULL,I64 cmp_flags=CMPF_PRS_SEMICOLON)
{
  I64 i,fsp_flags=0;
  CHashExport *tmpex;
  CCodeMisc *g_lb;
  U8 *import_name;
  CHashFun *tmp_untry;
  CAOT *tmpaot;
  if (cmp_flags&CMPF_ONE_ASM_INS) {
    if (cc->flags&CCF_AOT_COMPILE || cc->aot_depth)
      PrsAsmBlk(cc,CMPF_ONE_ASM_INS);
    else if (tmpaot=CmpJoin(cc,CMPF_ASM_BLK|CMPF_ONE_ASM_INS))
      CmpFixUpJITAsm(cc,tmpaot);
    fsp_flags=FSF_ASM;
  } else
    while (TRUE) {
      while (cc->token==',')
	Lex(cc);
      if (cc->token=='{') {
	Lex(cc);
	while (cc->token!='}' && cc->token!=TK_EOF)
	  PrsStmt(cc,try_cnt,lb_break);
	if (cc->lex_include_stk==cc->fun_lex_file)
	  cc->max_line=cc->lex_include_stk->line_num;
	if (Lex(cc)!=',') goto sm_done;
      } else if (cc->token==';') {
	if (cmp_flags&CMPF_PRS_SEMICOLON)
	  Lex(cc);
	if (cc->token!=',') goto sm_done;
      } else {
	if (cc->token==TK_IDENT) {
	  if (tmpex=cc->hash_entry) {
	    if (tmpex->type & HTT_KEYWORD) {
	      i=tmpex(CHashGeneric *)->user_data0;
	      switch [i] {
		case KW_KWS_NUM-1: //nobound switch
		default: //A keyword that is not valid here is just a symbol.
		  goto sm_not_keyword_afterall;
		start:
		  case KW_ASM:
		    if (cc->htc.fun) {
		      if (tmpaot=CmpJoin(cc,CMPF_ASM_BLK))
			ICAdd(cc,IC_ASM,tmpaot,0);
		      Lex(cc); //Skip '}' of asm{}
		    } else {
		      if (cc->flags&CCF_AOT_COMPILE || cc->aot_depth) {
			Lex(cc);
			PrsAsmBlk(cc,0);
			if (cc->flags&CCF_AOT_COMPILE && cc->aot_depth==1)
			  Lex(cc); //Skip '}' of asm{}
		      } else {
			if (tmpaot=CmpJoin(cc,CMPF_ASM_BLK))
			  CmpFixUpJITAsm(cc,tmpaot);
			Lex(cc); //Skip '}' of asm{}
		      }
		      fsp_flags=FSF_ASM;
		    }
		    break;
		  start:
		    Lex(cc);
		    case KW_LOCK:
		      cc->lock_cnt++;
		      PrsStmt(cc,try_cnt);
		      cc->lock_cnt--;
		      break;
		    case KW_TRY:
		      PrsTryBlk(cc,try_cnt);
		      break;
		    case KW_IF:
		      PrsIf(cc,try_cnt,lb_break);
		      break;
		    case KW_FOR:
		      PrsFor(cc,try_cnt);
		      break;
		    case KW_WHILE:
		      PrsWhile(cc,try_cnt);
		      break;
		    case KW_DO:
		      PrsDoWhile(cc,try_cnt);
		      break;
		    case KW_SWITCH:
		      PrsSwitch(cc,try_cnt);
		      break;
		  end:
		end:
		  if (cc->token!=',') goto sm_done;
		  break;
		start:
		  if (cc->htc.fun)
		    LexExcept(cc,"Not allowed in fun");
		  Lex(cc);
		  case KW__EXTERN:
		    if (Bt(&cc->opts,OPTf_EXTERNS_TO_IMPORTS))
		      goto sm_underscore_import;
		    if (cc->token!=TK_IDENT || !(tmpex=cc->hash_entry) ||
			  !(tmpex->type & HTT_EXPORT_SYS_SYM))
		      LexExcept(cc,"Expecting system sym at ");
		    if (*cc->cur_str=='_')
		      fsp_flags|=FSF__;
		    i=tmpex->val;
		    Lex(cc);
		    if (cc->token!=TK_IDENT || !(tmpex=cc->hash_entry) ||
			  !(tmpex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(cc,"Expecting type at ");
		    Lex(cc);
		    PrsGlblVarLst(cc,PRS0__EXTERN|PRS1_NULL,tmpex,i,fsp_flags);
		    break;
		  case KW__IMPORT:
sm_underscore_import:
		    if (cc->token!=TK_IDENT)
		      LexExcept(cc,"Expecting system sym at ");
		    if (*cc->cur_str=='_')
		      fsp_flags|=FSF__;
		    import_name=cc->cur_str;
		    cc->cur_str=0;
		    if (Lex(cc)!=TK_IDENT || !(tmpex=cc->hash_entry) ||
			  !(tmpex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(cc,"Expecting type at ");
		    Lex(cc);
		    PrsGlblVarLst(cc,PRS0__IMPORT|PRS1_NULL,tmpex,
			  import_name,fsp_flags);
		    Free(import_name);
		    break;
		  case KW_EXTERN:
		    if (cc->token!=TK_IDENT)
		      LexExcept(cc,"Expecting type at ");
		    tmpex=cc->hash_entry;
		    i=PrsKeyWord(cc);
		    if (i==KW_CLASS||i==KW_UNION) {
		      Lex(cc);
		      PrsClass(cc,i,fsp_flags,TRUE);
		      fsp_flags&=FSF_ASM;
		      goto sm_semicolon;
		    }
		    if (!tmpex ||
			  !(tmpex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(cc,"Expecting type at ");
		    if (Bt(&cc->opts,OPTf_EXTERNS_TO_IMPORTS))
		      goto sm_import;
		    Lex(cc);
		    PrsGlblVarLst(cc,PRS0_EXTERN|PRS1_NULL,tmpex,0,fsp_flags);
		    break;
		  case KW_IMPORT:
		    if (cc->token!=TK_IDENT || !(tmpex=cc->hash_entry) ||
			  !(tmpex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(cc,"Expecting type at ");
sm_import:
		    Lex(cc);
		    PrsGlblVarLst(cc,PRS0_IMPORT|PRS1_NULL,tmpex,0,fsp_flags);
		    break;
		  case KW__INTERN:
		    i=LexExpressionI64(cc);
		    if (cc->token!=TK_IDENT || !(tmpex=cc->hash_entry) ||
			  !(tmpex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(cc,"Expecting type at ");
		    Lex(cc);
		    PrsGlblVarLst(cc,PRS0__INTERN|PRS1_NULL,tmpex,i,fsp_flags);
		    break;
		end:
		  fsp_flags&=FSF_ASM;
		  break;
		start:
		  case KW_STATIC:
		    fsp_flags=FSF_STATIC|fsp_flags&FSF_ASM;
		    break;
		  case KW_INTERRUPT:
		    fsp_flags=FSF_INTERRUPT|FSF_NOARGPOP|
			  fsp_flags&(FSG_FUN_FLAGS2|FSF_ASM);
		    break;
		  case KW_HASERRCODE:
		    fsp_flags=FSF_HASERRCODE|fsp_flags&(FSG_FUN_FLAGS2|FSF_ASM);
		    break;
		  case KW_ARGPOP:
		    fsp_flags=FSF_ARGPOP|fsp_flags&(FSG_FUN_FLAGS2|FSF_ASM);
		    break;
		  case KW_NOARGPOP:
		    fsp_flags=FSF_NOARGPOP|fsp_flags&(FSG_FUN_FLAGS2|FSF_ASM);
		    break;
		  case KW_PUBLIC:
		    fsp_flags=FSF_PUBLIC|fsp_flags&(FSG_FUN_FLAGS2|FSF_ASM);
		    break;
		end:
		  Lex(cc);
		  break;
		case KW_RETURN:
		  if (!cc->htc.fun)
		    LexExcept(cc,"Not in fun.  Can't return a val ");
		  if (try_cnt) {
		    tmp_untry=HashFind("SysUntry",
			  cc->htc.hash_table_lst,HTT_FUN);
		    for (i=0;i<try_cnt;i++) {
		      if (Bt(&tmp_untry->flags,Cf_EXTERN)) {
			cc->abs_cnts.externs++;
			if (cc->flags&CCF_AOT_COMPILE)
			  ICAdd(cc,IC_CALL_IMPORT,
				tmp_untry,cmp.internal_types[RT_PTR]);
			else
			  ICAdd(cc,IC_CALL_INDIRECT2,
				&tmp_untry->exe_addr,
				cmp.internal_types[RT_PTR]);
		      } else
			ICAdd(cc,IC_CALL,tmp_untry->exe_addr,
			      cmp.internal_types[RT_PTR]);
		    }
		  }
		  if (Lex(cc)!=';') {
		    if (!cc->htc.fun->return_class->size)
		      LexWarn(cc,"Function should NOT return val ");
		    if (!PrsExpression(cc,NULL,FALSE))
		      throw('Compiler');
		    ICAdd(cc,IC_RETURN_VAL,0,cc->htc.fun->return_class);
		    cc->flags|=CCF_HAS_RETURN;
		  } else if (cc->htc.fun->return_class->size)
		    LexWarn(cc,"Function should return val ");
		  ICAdd(cc,IC_JMP,cc->lb_leave,0);
		  goto sm_semicolon;
		case KW_GOTO:
		  if (Lex(cc)!=TK_IDENT)
		    LexExcept(cc,"Expecting identifier at ");
		  if (!(g_lb=COCGoToLabelFind(cc,cc->cur_str))) {
		    g_lb=COCMiscNew(cc,CMT_GOTO_LABEL);
		    g_lb->str=cc->cur_str;
		    cc->cur_str=NULL;
		  }
		  g_lb->use_cnt++;
		  ICAdd(cc,IC_JMP,g_lb,0);
		  Lex(cc);
		  goto sm_semicolon;
		case KW_BREAK:
		  Lex(cc);
		  if (!lb_break)
		    LexExcept(cc,"'break' not allowed\n");
		  ICAdd(cc,IC_JMP,lb_break,0);
		  goto sm_semicolon;
		case KW_NO_WARN:
		  Lex(cc);
		  PrsNoWarn(cc);
		  goto sm_semicolon;
		case KW_UNION:
		case KW_CLASS:
		  Lex(cc);
		  tmpex=PrsClass(cc,i,fsp_flags,FALSE);
		  if (!cc->htc.fun && cc->token!=';') {
		    PrsGlblVarLst(cc,PRS0_NULL|PRS1_NULL,tmpex,0,fsp_flags);
		    fsp_flags&=FSF_ASM;
		    break;
		  } else {
		    fsp_flags&=FSF_ASM;
		    goto sm_semicolon;
		  }
	      }
	    } else {//Ident, found in hash table, not keyword
sm_not_keyword_afterall:
	      if (tmpex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)) {
		if (cc->htc.fun) {
		  if (fsp_flags&FSF_STATIC)
		    PrsVarLst(cc,cc->htc.fun,PRS0_NULL|PRS1_STATIC_LOCAL_VAR);
		  else
		    PrsVarLst(cc,cc->htc.fun,PRS0_NULL|PRS1_LOCAL_VAR);
		  if (cc->token=='}') goto sm_done;
		} else {
		  Lex(cc);
		  PrsGlblVarLst(cc,PRS0_NULL|PRS1_NULL,tmpex,0,fsp_flags);
		}
	      } else {
		if (tmpex->type & (HTT_OPCODE|HTT_ASM_KEYWORD)) {
		  if (cc->htc.fun) {
		    if (tmpaot=CmpJoin(cc,CMPF_ASM_BLK|CMPF_ONE_ASM_INS))
		      ICAdd(cc,IC_ASM,tmpaot,0);
		  } else
		    LexExcept(cc,"Use Asm Blk at ");
		  if (cc->token!=',') goto sm_done;
		} else
		  goto sm_prs_exp;
	      }
	      fsp_flags&=FSF_ASM;
	    }
	  } else {//Ident, not in hash table
	    if (cc->local_var_entry)
	      goto sm_prs_exp;
	    if (!(g_lb=COCGoToLabelFind(cc,cc->cur_str))) {
	      g_lb=COCMiscNew(cc,CMT_GOTO_LABEL);
	      g_lb->str=cc->cur_str;
	      cc->cur_str=NULL;
	    } else if (g_lb->flags&CMF_DEFINED)
	      LexExcept(cc,"Duplicate goto label at ");
	    g_lb->flags|=CMF_DEFINED;
	    ICAdd(cc,IC_LABEL,g_lb,0);
	    if (Lex(cc)==':') //skip cur_str
	      Lex(cc); //skip colon
	    else
	      LexExcept(cc,"Undefined identifier at ");
	    if (!cc->htc.fun)
	      LexExcept(cc,"No global labels at ");
	    if (cc->token!=',') goto sm_done;
	  }
	} else if (cc->token==TK_STR||cc->token==TK_CHAR_CONST) {
	  PrsFunCall(cc,NULL,FALSE,NULL);
	  goto sm_semicolon;
	} else if (cc->token!=TK_EOF) {//Non-cur_str symbol, num or something
sm_prs_exp:
	  if (!PrsExpression(cc,NULL,TRUE))
	    throw('Compiler');
sm_semicolon:
	  if (cmp_flags&CMPF_PRS_SEMICOLON) {
	    if (cc->token==';')
	      Lex(cc);
	    else if (cc->token!=',')
	      LexExcept(cc,"Missing ';' at");
	  }
	  if (cc->token!=',') goto sm_done;
	} else
	  goto sm_done; //TK_EOF
      }
    }
sm_done:
  return fsp_flags&FSF_ASM;
}
