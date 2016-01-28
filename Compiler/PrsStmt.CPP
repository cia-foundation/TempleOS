CHashClass *PrsClass(CCmpCtrl *cc,I64 keyword,I64 fsp_flags,Bool is_extern)
{
  CHashClass *tempc,*base_class;
  if (cc->token!=TK_IDENT)
    LexExcept(cc,"expecting identifier at ");
  if (is_extern) {
    tempc=PrsClassNew;
    tempc->str=cc->cur_str;
    cc->cur_str=NULL;
    HashAdd(tempc,cc->htc.glbl_hash_table);
    LBts(&tempc->flags,Cf_EXTERN);
    HashSrcFileSet(cc,tempc);
    Lex(cc);
  } else {
    if (cc->flags&CCF_AOT_COMPILE) {
      if (tempc=HashFind(cc->cur_str,cc->htc.glbl_hash_table,HTT_CLASS)) {
	if (!Bt(&tempc->flags,Cf_EXTERN))
	  tempc=NULL;
	else if (tempc->use_cnt<3)
	  UnusedExternWarning(cc,tempc);
      }
    } else {
      if (tempc=HashSingleTableFind(cc->cur_str,
	    cc->htc.glbl_hash_table,HTT_CLASS)) {
	if (!Bt(&tempc->flags,Cf_EXTERN))
	  tempc=NULL;
	else if (tempc->use_cnt<3)
	  UnusedExternWarning(cc,tempc);
      }
    }
    if (tempc) {
      Free(tempc->src_link);
      tempc->src_link=NULL;
      Free(tempc->index);
      tempc->index=NULL;
    } else {
      tempc=PrsClassNew;
      tempc->str=cc->cur_str;
      cc->cur_str=NULL;
      HashAdd(tempc,cc->htc.glbl_hash_table);
    }
    LBtr(&tempc->flags,Cf_EXTERN);
    if (fsp_flags&FSF_PUBLIC)
      tempc->type|=HTF_PUBLIC;
    tempc->use_cnt=0;
    if (cc->last_U16=='\n')
      HashSrcFileSet(cc,tempc,-1);
    else
      HashSrcFileSet(cc,tempc,0);
    if (Lex(cc)==':') {
      if (Lex(cc)!=TK_IDENT || !(base_class=cc->hash_entry) ||
	    !(base_class->type&HTT_CLASS))
	LexExcept(cc,"invalid class at ");
      if (Lex(cc)==',')
	LexExcept(cc,"Only one base class allowed at this time at ");
      tempc->base_class=base_class;
      tempc->size+=base_class->size;
    }
    if (keyword==KW_UNION)
      PrsVarLst(cc,tempc,PRS0_NULL|PRS1_CLASS|PRSF_UNION);
    else
      PrsVarLst(cc,tempc,PRS0_NULL|PRS1_CLASS);
    tempc->size+=tempc->neg_offset;
  }
  return tempc;
}

CHashFun *PrsFunJoin(CCmpCtrl *cc,
	CHashClass *temp_return,U8 *name,I64 fsp_flags)
{
  CMemberLst *tempm;
  CAOTCtrl *aotc=cc->aotc;
  CHashFun *tempf;
  if (name) {//if not fun_ptr
    if (cc->flags&CCF_AOT_COMPILE) {
      if (tempf=HashFind(name,cc->htc.glbl_hash_table,HTT_FUN)) {
	if (tempf->type & HTF_IMPORT)
	  tempf=NULL;
	else
	  if (tempf->use_cnt<3)
	    UnusedExternWarning(cc,tempf);
      }
    } else {
      if (tempf=HashSingleTableFind(name,cc->htc.glbl_hash_table,HTT_FUN)) {
	if (!Bt(&tempf->flags,Cf_EXTERN))
	  tempf=NULL;
	else if (tempf->use_cnt<3)
	  UnusedExternWarning(cc,tempf);
      }
    }
  } else
    tempf=NULL;
  if (tempf) {
    tempf->used_reg_mask=REGG_C_CLOBBERED+REGG_SAVED+REGG_C_TEMP_CLOBBERED;
    Free(tempf->src_link);
    tempf->src_link=NULL;
    Free(tempf->index);
    tempf->index=NULL;
    Free(name);
    MemberLstDel(tempf);
  } else {
    tempf=PrsFunNew;
    tempf->used_reg_mask=REGG_C_CLOBBERED+REGG_SAVED+REGG_C_TEMP_CLOBBERED;
    tempf->clobbered_reg_mask=REGG_C_CLOBBERED+REGG_C_TEMP_CLOBBERED;
    tempf->str=name;
    if (cc->flags&CCF_AOT_COMPILE)
      tempf->executable_addr=aotc->ip;
    else
      tempf->executable_addr=&UndefinedExtern;
    LBts(&tempf->flags,Cf_EXTERN);
    tempf->flags|=fsp_flags&FSG_FUN_FLAGS1;
    if (name) //if not fun_ptr
      HashAdd(tempf,cc->htc.glbl_hash_table);
  }
  BEqu(&tempf->type,HTf_PUBLIC,fsp_flags&FSF_PUBLIC);
  tempf->return_class=temp_return;
  tempf->use_cnt=0;
  HashSrcFileSet(cc,tempf);
  PrsVarLst(cc,tempf,PRS0_NULL|PRS1_FUN_ARG);
  tempf->arg_cnt=tempf->member_cnt;
  if (0<tempf->arg_cnt<<3<=MAX_I16 && !Bt(&tempf->flags,Ff_DOT_DOT_DOT))
    LBts(&tempf->flags,Ff_RET1);
  tempm=tempf->member_lst_and_root;
  while (tempm) {
    tempm->offset+=16; //RBP+RETURN
    tempm=tempm->next;
  }
  tempf->size=0;
  return tempf;
}

U0 PrsFun(CCmpCtrl *cc,CHashClass *temp_return,U8 *name,I64 fsp_flags)
{
  CMemberLst *tempm;
  CCodeMisc *saved_leave_label;
  I64 i,j,size,*r;
  Bool old_trace;

  cc->fun_lex_file=cc->lex_include_stk;
  cc->min_line=cc->max_line=cc->lex_include_stk->line_num;

  cc->flags&=~CCF_NO_REG_OPT;
  cc->htc.local_var_lst=cc->htc.fun=PrsFunJoin(cc,temp_return,name,fsp_flags);

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
    cc->htc.fun->executable_addr=cc->aotc->ip;
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
    cc->htc.fun->executable_addr=COCCompile(cc,&size,
	  &cc->htc.fun->dbg_info,NULL);
    if (old_trace) {
      Bts(&cc->opts,OPTf_TRACE);
      Un(cc->htc.fun->executable_addr,size,64);
    }
    SysSymImportsResolve(cc->htc.fun->str);
  }
  LBtr(&cc->htc.fun->flags,Cf_EXTERN);
  COCPop(cc);
  tempm=cc->htc.fun->member_lst_and_root;
  while (tempm) {
    if (tempm->flags & MLF_NO_UNUSED_WARN) {
      if (tempm->use_cnt>1&&StrCmp(tempm->str,"_anon_"))
	PrintWarn("Unneeded nounusedwarn\n $$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
	      cc->lex_include_stk->full_name,cc->lex_include_stk->line_num,
	      tempm->str,cc->htc.fun->str);
    } else if (!tempm->use_cnt && GetOption(OPTf_WARN_UNUSED_VAR))
      PrintWarn("Unused var\n $$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
	    cc->lex_include_stk->full_name,cc->lex_include_stk->line_num,
	    tempm->str,cc->htc.fun->str);
    tempm=tempm->next;
  }
  cc->htc.local_var_lst=cc->htc.fun=cc->fun_lex_file=NULL;
}

U0 PrsGlblVarLst(CCmpCtrl *cc,I64 saved_mode,CHashClass *saved_tempc,
  I64 saved_val,I64 fsp_flags)
{
  I64 i,j,mode,k,val;
  U8 *st;
  CHashExport *tempex;
  CHashGlblVar *tempg;
  CAOTCtrl *aotc=cc->aotc;
  CAOTHeapGlbl *temphg;
  CHashClass *tempc;
  CHashFun *tempf,*tempf_fun_ptr;
  CArrayDim tempad;
  Bool has_alias,undef_array_size,is_array;
  while (TRUE) {
    tempc=PrsType(cc,&saved_tempc,&saved_mode,NULL,&st,
	  &tempf_fun_ptr,&tempex,&tempad,fsp_flags);
    if (!st)
      return;

    if (tempad.next)
      is_array=TRUE;
    else if (tempad.total_cnt<0) {
      is_array=TRUE;
      tempc--;
    } else
      is_array=FALSE;

    val=saved_val;
    mode=saved_mode;
    if (tempex && mode&255==PRS0_EXTERN && !(cc->flags&CCF_AOT_COMPILE) &&
	  tempex->type & HTT_EXPORT_SYS_SYM) {
      val=tempex->val;
      mode=PRS0__EXTERN|PRS1_NULL;
    }
    if (cc->token=='(') {
      switch (mode&255) {
	case PRS0__INTERN:
	  tempf=PrsFunJoin(cc,tempc,st,fsp_flags);
	  tempf->executable_addr=val;
	  Bts(&tempf->flags,Ff_INTERNAL);
	  LBtr(&tempf->flags,Cf_EXTERN);
	  return;
	case PRS0__EXTERN:
	  tempf=PrsFunJoin(cc,tempc,st,fsp_flags);
	  tempf->executable_addr=val;
	  SysSymImportsResolve(tempf->str);
	  LBtr(&tempf->flags,Cf_EXTERN);
	  if (saved_mode&255==PRS0__EXTERN)
	    LBts(&tempf->flags,Ff__EXTERN);
	  if (cc->flags&CCF_AOT_COMPILE)
	    tempf->type|=HTF_RESOLVE;
	  return;
	case PRS0_EXTERN:
	  PrsFunJoin(cc,tempc,st,fsp_flags);
	  return;
	case PRS0_IMPORT:
	case PRS0__IMPORT:
	  if (!(cc->flags&CCF_AOT_COMPILE))
	    LexExcept(cc,"import not needed at ");
	  else {
	    tempf=PrsFunJoin(cc,tempc,st,fsp_flags);
	    tempf->type|=HTF_IMPORT;
	    if (mode&255==PRS0__IMPORT)
	      tempf->import_name=StrNew(val);
	    else
	      tempf->import_name=StrNew(st);
	  }
	  return;
	default:
	  PrsFun(cc,tempc,st,fsp_flags);
	  return;
      }
    } else {
      if (tempad.total_cnt<0) {
	i=0;
	undef_array_size=TRUE;
      } else {
	i=tempad.total_cnt;
	undef_array_size=FALSE;
      }
      if (tempf_fun_ptr)
	j=sizeof(U8 *);
      else
	j=tempc->size;
      j*=i;
      has_alias=FALSE;
      temphg=NULL;
      switch (mode&255) {
	case PRS0__EXTERN:
	  if (cc->flags&CCF_AOT_COMPILE) {
	    tempg=CAlloc(sizeof(CHashGlblVar));
	    tempg->data_addr_ip=val;
	    tempg->type=HTT_GLBL_VAR | HTF_EXPORT;
	  } else {
	    tempg=CAlloc(sizeof(CHashGlblVar),Fs->code_heap);
	    tempg->data_addr=val;
	    tempg->type=HTT_GLBL_VAR;
	  }
	  tempg->flags|=GVF_ALIAS;
	  break;
	case PRS0__IMPORT:
	case PRS0_IMPORT:
	  if (!(cc->flags&CCF_AOT_COMPILE))
	    LexExcept(cc,"import not needed at ");
	  else {
	    tempg=CAlloc(sizeof(CHashGlblVar));
	    tempg->type=HTT_GLBL_VAR | HTF_IMPORT;
	    if (mode&255==PRS0__IMPORT)
	      tempg->import_name=StrNew(val);
	    else
	      tempg->import_name=StrNew(st);
	  }
	  break;
	case PRS0_EXTERN:
	  if (cc->flags&CCF_AOT_COMPILE) {
	    tempg=CAlloc(sizeof(CHashGlblVar));
	    tempg->type=HTT_GLBL_VAR;
	  } else {
	    tempg=CAlloc(sizeof(CHashGlblVar),Fs->code_heap);
	    tempg->type=HTT_GLBL_VAR|HTF_UNRESOLVED;
	  }
	  break;
	default:
	  if (cc->flags&CCF_AOT_COMPILE) {
	    if (Bt(&cc->opts,OPTf_GLBLS_ON_DATA_HEAP)) {
	      if (cc->token=='=')
		LexExcept(cc,"Can't init glbl var on data heap in AOT module ");
	      tempg=CAlloc(sizeof(CHashGlblVar));
	      temphg=tempg->heap_glbl=CAlloc(sizeof(CAOTHeapGlbl));
	      temphg->size=j;
	      temphg->str=StrNew(st);
	      temphg->next=aotc->heap_glbls;
	      aotc->heap_glbls=temphg;
	      tempg->flags=GVF_DATA_HEAP;
	      tempg->type=HTT_GLBL_VAR; //TODO: HTF_EXPORT
	      if (tempex && tempex->type & HTT_GLBL_VAR) //TODO!! extern
		LexExcept(cc,"Feature not implemented ");
	    } else {
	      tempg=CAlloc(sizeof(CHashGlblVar));
	      if (cc->token=='=')
		tempg->data_addr=CAlloc(j);
	      if (tempc->size>=8) //align
		while (aotc->ip&7)
		  AOTStoreCodeU8(cc,0);
	      else if (tempc->size==4)
		while (aotc->ip&3)
		  AOTStoreCodeU8(cc,0);
	      else if (tempc->size==2)
		while (aotc->ip&1)
		  AOTStoreCodeU8(cc,0);
	      tempg->data_addr_ip=aotc->ip;
	      tempg->type=HTT_GLBL_VAR | HTF_EXPORT;
	      if (tempex && tempex->type & HTT_GLBL_VAR)
		has_alias=TRUE;
	      if (sys_var_init_flag&1)
		for (k=0;k<j;k++)
		  AOTStoreCodeU8(cc,sys_var_init_val);
	      else
		for (k=0;k<j;k++)
		  AOTStoreCodeU8(cc,0);
	    }
	  } else {
	    if (Bt(&cc->opts,OPTf_GLBLS_ON_DATA_HEAP)) {
	      tempg=CAlloc(sizeof(CHashGlblVar),Fs->code_heap);
	      tempg->data_addr=MAlloc(j);
	      tempg->flags=GVF_DATA_HEAP;
	    } else {
	      tempg=CAlloc(sizeof(CHashGlblVar),Fs->code_heap);
	      tempg->data_addr=MAlloc(j,Fs->code_heap);
	    }
	    tempg->type=HTT_GLBL_VAR;
	    if (tempex && tempex->type&HTT_GLBL_VAR &&
		  tempex->type&HTF_UNRESOLVED &&
		  MHeapCtrl(tempex)==MHeapCtrl(tempg))
	      has_alias=TRUE;
	    if (sys_var_init_flag&1)
	      MemSet(tempg->data_addr,sys_var_init_val,j);
	  }
      }
      tempg->dim.next=tempad.next;
      if (fsp_flags&FSF_PUBLIC)
	tempg->type|=HTF_PUBLIC;
      tempg->var_class=tempc;
      tempg->str=st;
      tempg->size=j;
      tempg->dim.total_cnt=i;
      tempg->use_cnt=0;
      if (cc->last_U16=='\n')
	HashSrcFileSet(cc,tempg,-1);
      else
	HashSrcFileSet(cc,tempg,0);
      if (mode&255==PRS0_IMPORT || mode&255==PRS0__IMPORT)
	tempg->flags|=GVF_IMPORT;
      if (mode&255==PRS0_EXTERN)
	tempg->flags|=GVF_EXTERN;
      if (tempf_fun_ptr) {
	tempg->fun_ptr=tempf_fun_ptr;
	tempg->flags|=GVF_FUN;
      }
      if (is_array)
	tempg->flags|=GVF_ARRAY;
      HashAdd(tempg,cc->htc.glbl_hash_table);
      if (!(cc->flags&CCF_AOT_COMPILE) && !(tempg->flags&GVF_EXTERN))
	SysSymImportsResolve(tempg->str);
      if (cc->token=='=') {
	if (undef_array_size) {
	  LexPush(cc);
	  LexPush(cc);
	  Lex(cc);
	  PrsGlblInit(cc,tempg,1);
	  LexPopNoRestore(cc);
	  tempg->size=tempg->dim.total_cnt*tempc->size;
	  if (temphg)
	    temphg->size=tempg->size;
	  if (cc->flags&CCF_AOT_COMPILE) {
	    if (sys_var_init_flag&1)
	      for (k=0;k<tempg->size;k++)
		AOTStoreCodeU8(cc,sys_var_init_val);
	    else
	      for (k=0;k<tempg->size;k++)
		AOTStoreCodeU8(cc,0);
	  } else
	    if (sys_var_init_flag&1)
	      MemSet(tempg->data_addr,sys_var_init_val,k);
	  LexPopRestore(cc);
	}
	LexPush(cc);
	Lex(cc);
	PrsGlblInit(cc,tempg,2);
	if (cc->flags&CCF_AOT_COMPILE)
	  for (k=0;k<tempg->size;k++)
	    AOTStoreCodeU8At(cc,tempg->data_addr_ip+k,tempg->data_addr[k]);
	LexPopNoRestore(cc);
      }
      if (has_alias) {
	if (tempex(CHashGlblVar *)->use_cnt<2) {
	  PrintWarn("Unused extern '%s'\n",tempex(CHashGlblVar *)->str);
	  cc->warning_cnt++;
	}
	tempex(CHashGlblVar *)->flags|=GVF_ALIAS;
	tempex(CHashGlblVar *)->data_addr=tempg->data_addr;
	tempex(CHashGlblVar *)->data_addr_ip=tempg->data_addr_ip;
      }
      if (cc->token==',')
	Lex(cc);
      else {
	if (cc->token!=';')
	  LexExcept(cc,"missing ';' at");
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
    LexExcept(cc,"expecting '(' at ");
  Lex(cc);
  if (!PrsExpression(cc,NULL,FALSE))
    throw('Compiler');
  if (cc->token!=')')
    LexExcept(cc,"missing ')' at ");
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
    LexExcept(cc,"expecting '(' at ");
  Lex(cc);
  lb=COCMiscNew(cc,CMT_LABEL);
  ICAdd(cc,IC_LABEL,lb,0);
  if (!PrsExpression(cc,NULL,FALSE))
    throw('Compiler');
  if (cc->token!=')')
    LexExcept(cc,"missing ')' at ");
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
    LexExcept(cc,"missing 'while' at");
  if (Lex(cc)!='(')
    LexExcept(cc,"expecting '(' at ");
  Lex(cc);
  if (!PrsExpression(cc,NULL,FALSE))
    throw('Compiler');
  if (cc->token!=')')
    LexExcept(cc,"missing ')' at ");
  ICAdd(cc,IC_BR_NOT_ZERO,lb,0);
  ICAdd(cc,IC_LABEL,lb_done,0);
  if (Lex(cc)!=';')
    LexExcept(cc,"missing ';' at");
  Lex(cc);
}

U0 PrsFor(CCmpCtrl *cc,I64 try_cnt)
{
  CCodeCtrl *tempcbh;
  CCodeMisc *lb,*lb_done;

  if (cc->token!='(')
    LexExcept(cc,"expecting '(' at ");
  Lex(cc);
  PrsStmt(cc,try_cnt);

  lb=COCMiscNew(cc,CMT_LABEL);
  ICAdd(cc,IC_LABEL,lb,0);
  if (!PrsExpression(cc,NULL,FALSE))
    throw('Compiler');
  lb_done=COCMiscNew(cc,CMT_LABEL);
  ICAdd(cc,IC_BR_ZERO,lb_done,0);
  if (cc->token!=';')
    LexExcept(cc,"missing ';' at");
  Lex(cc);

  COCPush(cc);
  COCInit(cc);
  if (cc->token!=')')
    PrsStmt(cc,try_cnt,NULL,0);
  COCPush(cc);
  tempcbh=COCPopNoFree(cc);
  COCPop(cc);
  if (cc->token!=')')
    LexExcept(cc,"missing ')' at ");
  Lex(cc);

  PrsStmt(cc,try_cnt,lb_done);
  COCAppend(cc,tempcbh);
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

U0 PrsSwitch(CCmpCtrl *cc,Bool nobound,I64 try_cnt)
{
  CSwitchCase *header=NULL,*temps,*temps1;	//Leaks on except
  CSubSwitch root,*tempss;			//Leaks on except
  CCodeMisc *lb_dft,*lb_fwd_case,*mc_jt,*lb_entry,**jmp_table;
  CIntermediateCode *tempi_sub,*tempi_cmp,*tempi_jmp,*tempi_start;
  Bool dft_found=FALSE;
  I64 i,k_start=MIN_I64,k_end,lo=MAX_I64,hi=MIN_I64,range;

  if (cc->token!='(')
    LexExcept(cc,"expecting '(' at ");
  Lex(cc);
  QueInit(&root);

  root.last->lb_break=COCMiscNew(cc,CMT_LABEL);
  root.last->lb_break->use_cnt++;
  lb_dft=COCMiscNew(cc,CMT_LABEL);
  lb_dft->use_cnt++;
  mc_jt=COCMiscNew(cc,CMT_JMP_TABLE);
  mc_jt->begin=COCMiscNew(cc,CMT_LABEL);
  mc_jt->begin->use_cnt++;
  if (!PrsExpression(cc,NULL,FALSE))
    throw('Compiler');
  tempi_sub=ICAdd(cc,IC_IMM_I64,0,cmp.internal_types[RT_I64]);
  ICAdd(cc,IC_SUB,0,cmp.internal_types[RT_I64]);
  tempi_cmp=ICAdd(cc,IC_IMM_I64,0,cmp.internal_types[RT_I64]);
  if (nobound)
    ICAdd(cc,IC_NOBOUND_SWITCH,mc_jt,0);
  else
    ICAdd(cc,IC_SWITCH,mc_jt,0);
  if (cc->token!=')')
    LexExcept(cc,"missing ')' at ");
  if (Lex(cc)!='{')
    LexExcept(cc,"expecting '{' at ");
  Lex(cc);
  ICAdd(cc,IC_LABEL,mc_jt->begin,0);
  while (TRUE) {
    while (cc->token && cc->token!='}') {
sw_cont:
      switch (PrsKeyWord(cc)) {
	case KW_SUB_SWITCH_END:
	  goto sw_sub_end;
	case KW_SUB_SWITCH_START:
	  if (Lex(cc)==':')
	    Lex(cc);
	  else
	    LexExcept(cc,"expecting ':' at ");
	  tempss=MAlloc(sizeof(CSubSwitch));
	  QueIns(tempss,root.last);
	  root.last->lb_break=COCMiscNew(cc,CMT_LABEL);
	  root.last->lb_break->use_cnt++;
	  lb_fwd_case=COCMiscNew(cc,CMT_LABEL);
	  tempi_jmp=ICAdd(cc,IC_JMP,lb_fwd_case,0);

	  tempss->lb_start=COCMiscNew(cc,CMT_LABEL);
	  tempi_start=ICAdd(cc,IC_LABEL,tempss->lb_start,0);
	  while (cc->token && cc->token!='}') {
	    switch (PrsKeyWord(cc)) {
	      case KW_SUB_SWITCH_END:
		OptFree(tempi_jmp);
		goto sw_sub_end;
	      case KW_SUB_SWITCH_START:
	      case KW_CASE:
	      case KW_DFT:
		if (cc->coc.coc_root.last==tempi_start) {
		  OptFree(tempi_jmp);
		  tempss->lb_start=NULL;
		} else {
		  ICAdd(cc,IC_RET,0,0);
		  ICAdd(cc,IC_LABEL,lb_fwd_case,0);
		  ICAdd(cc,IC_SUB_CALL,tempss->lb_start,0);//In case fall-thru
		}
		goto sw_cont;
	      default:
		PrsStmt(cc,try_cnt);
	    }
	  }
	  break;
	case KW_CASE:
	  if (root.next!=&root) {
	    lb_fwd_case=COCMiscNew(cc,CMT_LABEL);
	    tempi_jmp=ICAdd(cc,IC_JMP,lb_fwd_case,0);//In case fall-thru
	  }
	  Lex(cc);
	  lb_entry=COCMiscNew(cc,CMT_LABEL);
	  ICAdd(cc,IC_LABEL,lb_entry,0);
	  lb_entry->use_cnt++;
	  if (root.next!=&root) {
	    tempss=root.next;
	    while (tempss!=&root) {
	      if (tempss->lb_start)
		ICAdd(cc,IC_SUB_CALL,tempss->lb_start,0);
	      tempss=tempss->next;
	    }
	    ICAdd(cc,IC_LABEL,lb_fwd_case,0);
	  }
	  if (cc->token==':') {
	    if (k_start==MIN_I64)
	      k_start=0;
	    else
	      k_start++;
	  } else
	    k_start=LexExpressionI64(cc);
	  if (k_start<lo) lo=k_start;
	  if (k_start>hi) hi=k_start;
	  if (cc->token==':') {
	    Lex(cc);
	    temps=MAlloc(sizeof(CSwitchCase));
	    temps->label=lb_entry;
	    temps->val=k_start;
	    temps->next=header;
	    header=temps;
	  } else if (cc->token==TK_DOT_DOT_DOT) {
	    Lex(cc);
	    k_end=LexExpressionI64(cc);
	    if (cc->token==':') {
	      Lex(cc);
	      if (k_end<lo) lo=k_end;
	      if (k_end>hi) hi=k_end;
	      if (k_start>k_end)
		SwapI64(&k_start,&k_end);
	      for (i=k_start;i<=k_end;i++) {
		temps=MAlloc(sizeof(CSwitchCase));
		temps->label=lb_entry;
		temps->val=i;
		temps->next=header;
		header=temps;
	      }
	    } else
	      LexExcept(cc,"expecting ':' at ");
	  } else
	    LexExcept(cc,"expecting ':' at ");
	  break;
	case KW_DFT:
	  if (root.next!=&root) {
	    lb_fwd_case=COCMiscNew(cc,CMT_LABEL);
	    tempi_jmp=ICAdd(cc,IC_JMP,lb_fwd_case,0);//In case fall-thru
	  }
	  Lex(cc);
	  ICAdd(cc,IC_LABEL,lb_dft,0);
	  if (cc->token==':')
	    Lex(cc);
	  else
	    LexExcept(cc,"expecting ':' at ");
	  if (root.next!=&root) {
	    tempss=root.next;
	    while (tempss!=&root) {
	      if (tempss->lb_start)
		ICAdd(cc,IC_SUB_CALL,tempss->lb_start,0);
	      tempss=tempss->next;
	    }
	    ICAdd(cc,IC_LABEL,lb_fwd_case,0);
	  }
	  dft_found=TRUE;
	  break;
	default:
	  PrsStmt(cc,try_cnt,root.last->lb_break);
      }
    }
sw_sub_end:
    tempss=root.last;
    ICAdd(cc,IC_LABEL,tempss->lb_break,0);
    if (tempss==&root) {
      if (cc->token!='}')
	LexExcept(cc,"missing '}' at ");
      Lex(cc);
      break;
    } else {
      QueRem(tempss);
      Free(tempss);
      if (PrsKeyWord(cc)!=KW_SUB_SWITCH_END)
	LexExcept(cc,"missing 'sub_switch_end' at ");
      if (Lex(cc)==':')
	Lex(cc);
      else
	LexExcept(cc,"expecting ':' at ");
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
  tempi_sub->ic_data=lo;
  tempi_cmp->ic_data=range;
  temps=header;
  while (temps) {
    temps1=temps->next;
    if (jmp_table[temps->val-lo]!=lb_dft)
      LexExcept(cc,"Duplicate case at ");
    else
      jmp_table[temps->val-lo]=temps->label;
    Free(temps);
    temps=temps1;
  }
  mc_jt->dft=lb_dft;
  mc_jt->jmp_table=jmp_table;
  mc_jt->range=range;
}

U0 PrsNoUnusedWarn(CCmpCtrl *cc)
{
  CMemberLst *tempm;
  while (cc->token==TK_IDENT) {
    if (!(tempm=cc->local_var_entry))
      LexExcept(cc,"expecting local var at ");
    tempm->flags|=MLF_NO_UNUSED_WARN;
    if (Lex(cc)==',')
      Lex(cc);
    else if (cc->token!=';')
      LexExcept(cc,"expecting ',' at ");
  }
}

U0 PrsStreamBlk(CCmpCtrl *cc)
{
  CLexHashTableContext *htc=MAlloc(sizeof(CLexHashTableContext));
  CStreamBlk *tempe=MAlloc(sizeof(CStreamBlk));
  tempe->body=StrNew("");
  QueIns(tempe,cc->last_stream_blk);
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
    LexExcept(cc,"missing '}' at ");
  while (cc->token && cc->token!='}')
    ExeCmdLine(cc);

  MemCpy(&cc->htc,htc,sizeof(CLexHashTableContext));
  cc->flags=cc->flags&~CCF_EXE_BLK |
	htc->old_flags & (CCF_ASM_EXPRESSIONS|CCF_EXE_BLK|CCF_AOT_COMPILE);
  Free(htc);
  COCPop(cc);
  QueRem(tempe);
  if (*tempe->body)
    LexIncludeStr(cc,"StreamBlk",tempe->body,FALSE);
  else
    Free(tempe->body);
  Free(tempe);
  Lex(cc); //Skip '}'
}

U0 PrsTryBlk(CCmpCtrl *cc,I64 try_cnt)
{
  CCodeMisc	*lb_catch,*lb_done,*lb_untry;
  CHashClass	*tempc=cmp.internal_types[RT_PTR];
  CHashFun	*temp_try=HashFind("SysTry",cc->htc.hash_table_lst,HTT_FUN),
	*temp_untry=HashFind("SysUntry",cc->htc.hash_table_lst,HTT_FUN);

  if (!temp_try || !temp_untry)
    LexExcept(cc,"Missing header for SysTry() and SysUntry() at ");

  cc->flags|=CCF_NO_REG_OPT; //TODO:Currently no reg vars in funs with try/catch

  lb_catch=COCMiscNew(cc,CMT_LABEL);
  lb_done =COCMiscNew(cc,CMT_LABEL);
  lb_untry=COCMiscNew(cc,CMT_LABEL);

  ICAdd(cc,IC_CALL_START,0,0);
  ICAdd(cc,IC_GET_LABEL,lb_untry,tempc,ICF_PUSH_RESULT);
  ICAdd(cc,IC_GET_LABEL,lb_catch,tempc,ICF_PUSH_RESULT);
  if (Bt(&temp_try->flags,Cf_EXTERN)) {
    cc->abs_cnts.externs++;
    if (cc->flags&CCF_AOT_COMPILE)
      ICAdd(cc,IC_CALL_IMPORT,temp_try,tempc);
    else
      ICAdd(cc,IC_CALL_INDIRECT2,&temp_try->executable_addr,tempc);
  } else
    ICAdd(cc,IC_CALL,temp_try->executable_addr,tempc);
  if ((Bt(&temp_try->flags,Ff_RET1) ||
	Bt(&temp_try->flags,Ff_ARGPOP)) && !Bt(&temp_try->flags,Ff_NOARGPOP))
    ICAdd(cc,IC_ADD_RSP1,16,tempc);
  else
    ICAdd(cc,IC_ADD_RSP,16,tempc);
  ICAdd(cc,IC_CALL_END,0,tempc);
  ICAdd(cc,IC_END_EXP,0,0,ICF_NO_DEPEND_RESULT);

  PrsStmt(cc,try_cnt+1);

  ICAdd(cc,IC_LABEL,lb_untry,0);
  ICAdd(cc,IC_CALL_START,0,0);
  if (Bt(&temp_untry->flags,Cf_EXTERN)) {
    cc->abs_cnts.externs++;
    if (cc->flags&CCF_AOT_COMPILE)
      ICAdd(cc,IC_CALL_IMPORT,temp_untry,tempc);
    else
      ICAdd(cc,IC_CALL_INDIRECT2,&temp_untry->executable_addr,tempc);
  } else
    ICAdd(cc,IC_CALL,temp_untry->executable_addr,tempc);
  ICAdd(cc,IC_CALL_END,0,tempc);
  ICAdd(cc,IC_END_EXP,0,0,ICF_NO_DEPEND_RESULT);

  ICAdd(cc,IC_JMP,lb_done,0);

  if (PrsKeyWord(cc)!=KW_CATCH)
    LexExcept(cc,"missing 'catch' at");

  Lex(cc);
  ICAdd(cc,IC_LABEL,lb_catch,0);
  PrsStmt(cc,try_cnt+1);
  ICAdd(cc,IC_RET,0,tempc);
  ICAdd(cc,IC_LABEL,lb_done,0);
}

Bool PrsStmt(CCmpCtrl *cc,I64 try_cnt=0,
  CCodeMisc *lb_break=NULL,I64 cmp_flags=CMPF_PRS_SEMICOLON)
{
  I64 i,fsp_flags=0;
  CHashExport *tempex;
  CCodeMisc *g_lb;
  U8 *import_name;
  CHashFun *temp_untry;
  CAOT *tempaot;
  if (cmp_flags&CMPF_ONE_ASM_INS) {
    if (cc->flags&CCF_AOT_COMPILE || cc->aot_depth)
      PrsAsmBlk(cc,CMPF_ONE_ASM_INS);
    else if (tempaot=CmpJoin(cc,CMPF_ASM_BLK|CMPF_ONE_ASM_INS))
      CmpFixUpJITAsm(cc,tempaot);
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
	  if (tempex=cc->hash_entry) {
	    if (tempex->type & HTT_KEYWORD) {
	      i=tempex(CHashGeneric *)->user_data0;
	      nobound_switch (i) {
		case KW_NUM_KEYWORDS-1: //nobound_switch
		default: //A keyword that is not valid here is just a symbol.
		  goto sm_not_keyword_afterall;
		sub_switch_start:
		  case KW_ASM:
		    if (cc->htc.fun) {
		      if (tempaot=CmpJoin(cc,CMPF_ASM_BLK))
			ICAdd(cc,IC_ASM,tempaot,0);
		      Lex(cc); //Skip '}' of asm{}
		    } else {
		      if (cc->flags&CCF_AOT_COMPILE || cc->aot_depth) {
			Lex(cc);
			PrsAsmBlk(cc,0);
			if (cc->flags&CCF_AOT_COMPILE && cc->aot_depth==1)
			  Lex(cc); //Skip '}' of asm{}
		      } else {
			if (tempaot=CmpJoin(cc,CMPF_ASM_BLK))
			  CmpFixUpJITAsm(cc,tempaot);
			Lex(cc); //Skip '}' of asm{}
		      }
		      fsp_flags=FSF_ASM;
		    }
		    break;
		  sub_switch_start:
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
		    case KW_NOBOUND_SWITCH:
		      PrsSwitch(cc,TRUE,try_cnt);
		      break;
		    case KW_SWITCH:
		      PrsSwitch(cc,FALSE,try_cnt);
		      break;
		  sub_switch_end:
		sub_switch_end:
		  if (cc->token!=',') goto sm_done;
		  break;
		sub_switch_start:
		  if (cc->htc.fun)
		    LexExcept(cc,"Not allowed in fun");
		  Lex(cc);
		  case KW__EXTERN:
		    if (Bt(&cc->opts,OPTf_EXTERNS_TO_IMPORTS))
		      goto sm_underscore_import;
		    if (cc->flags&CCF_AOT_COMPILE) {
		      if (cc->token!=TK_IDENT || !(tempex=cc->hash_entry) ||
			    !(tempex->type & HTT_EXPORT_SYS_SYM))
			LexExcept(cc,"expecting System sym at ");
		      i=tempex->val;
		      Lex(cc);
		    } else
		      i=LexExpressionI64(cc);
		    if (cc->token!=TK_IDENT || !(tempex=cc->hash_entry) ||
			  !(tempex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(cc,"expecting type at ");
		    Lex(cc);
		    PrsGlblVarLst(cc,PRS0__EXTERN|PRS1_NULL,tempex,i,fsp_flags);
		    break;
		  case KW__IMPORT:
sm_underscore_import:
		    if (cc->token!=TK_IDENT)
		      LexExcept(cc,"expecting System sym at ");
		    import_name=cc->cur_str;
		    cc->cur_str=0;
		    if (Lex(cc)!=TK_IDENT || !(tempex=cc->hash_entry) ||
			  !(tempex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(cc,"expecting type at ");
		    Lex(cc);
		    PrsGlblVarLst(cc,PRS0__IMPORT|PRS1_NULL,tempex,
			  import_name,FALSE);
		    Free(import_name);
		    break;
		  case KW_EXTERN:
		    if (cc->token!=TK_IDENT)
		      LexExcept(cc,"expecting type at ");
		    tempex=cc->hash_entry;
		    i=PrsKeyWord(cc);
		    if (i==KW_CLASS||i==KW_UNION) {
		      Lex(cc);
		      PrsClass(cc,i,fsp_flags,TRUE);
		      fsp_flags&=FSF_ASM;
		      goto sm_semicolon;
		    }
		    if (!tempex ||
			  !(tempex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(cc,"expecting type at ");
		    if (Bt(&cc->opts,OPTf_EXTERNS_TO_IMPORTS))
		      goto sm_import;
		    Lex(cc);
		    PrsGlblVarLst(cc,PRS0_EXTERN|PRS1_NULL,tempex,0,fsp_flags);
		    break;
		  case KW_IMPORT:
		    if (cc->token!=TK_IDENT || !(tempex=cc->hash_entry) ||
			  !(tempex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(cc,"expecting type at ");
sm_import:
		    Lex(cc);
		    PrsGlblVarLst(cc,PRS0_IMPORT|PRS1_NULL,tempex,0,FALSE);
		    break;
		  case KW__INTERN:
		    i=LexExpressionI64(cc);
		    if (cc->token!=TK_IDENT || !(tempex=cc->hash_entry) ||
			  !(tempex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(cc,"expecting type at ");
		    Lex(cc);
		    PrsGlblVarLst(cc,PRS0__INTERN|PRS1_NULL,tempex,i,fsp_flags);
		    break;
		sub_switch_end:
		  fsp_flags&=FSF_ASM;
		  break;
		sub_switch_start:
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
		sub_switch_end:
		  Lex(cc);
		  break;
		case KW_RETURN:
		  if (!cc->htc.fun)
		    LexExcept(cc,"Not in fun.  Can't return a val ");
		  if (try_cnt) {
		    temp_untry=HashFind("SysUntry",
			  cc->htc.hash_table_lst,HTT_FUN);
		    for (i=0;i<try_cnt;i++) {
		      if (Bt(&temp_untry->flags,Cf_EXTERN)) {
			cc->abs_cnts.externs++;
			if (cc->flags&CCF_AOT_COMPILE)
			  ICAdd(cc,IC_CALL_IMPORT,
				temp_untry,cmp.internal_types[RT_PTR]);
			else
			  ICAdd(cc,IC_CALL_INDIRECT2,
				&temp_untry->executable_addr,
				cmp.internal_types[RT_PTR]);
		      } else
			ICAdd(cc,IC_CALL,temp_untry->executable_addr,
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
		    LexExcept(cc,"expecting identifier at ");
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
		case KW_NOUNUSEDWARN:
		  Lex(cc);
		  PrsNoUnusedWarn(cc);
		  goto sm_semicolon;
		case KW_UNION:
		case KW_CLASS:
		  Lex(cc);
		  tempex=PrsClass(cc,i,fsp_flags,FALSE);
		  if (!cc->htc.fun && cc->token!=';') {
		    PrsGlblVarLst(cc,PRS0_NULL|PRS1_NULL,tempex,0,fsp_flags);
		    fsp_flags&=FSF_ASM;
		    break;
		  } else {
		    fsp_flags&=FSF_ASM;
		    goto sm_semicolon;
		  }
	      }
	    } else {//Ident, found in hash table, not keyword
sm_not_keyword_afterall:
	      if (tempex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)) {
		if (cc->htc.fun) {
		  if (fsp_flags&FSF_STATIC)
		    PrsVarLst(cc,cc->htc.fun,PRS0_NULL|PRS1_STATIC_LOCAL_VAR);
		  else
		    PrsVarLst(cc,cc->htc.fun,PRS0_NULL|PRS1_LOCAL_VAR);
		  if (cc->token=='}') goto sm_done;
		} else {
		  Lex(cc);
		  PrsGlblVarLst(cc,PRS0_NULL|PRS1_NULL,tempex,0,fsp_flags);
		}
	      } else {
		if (tempex->type & (HTT_OPCODE|HTT_ASM_KEYWORD)) {
		  if (cc->htc.fun) {
		    if (tempaot=CmpJoin(cc,CMPF_ASM_BLK|CMPF_ONE_ASM_INS))
		      ICAdd(cc,IC_ASM,tempaot,0);
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
	      LexExcept(cc,"undefined identifier at ");
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
	      LexExcept(cc,"missing ';' at");
	  }
	  if (cc->token!=',') goto sm_done;
	} else
	  goto sm_done; //TK_EOF
      }
    }
sm_done:
  return fsp_flags&FSF_ASM;
}
