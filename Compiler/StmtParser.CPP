CHashClass *PrsClassNew()
{/*Ptrs to classes are handled by
allocating 5 structures for each
new class and representing a pointer
to a class by advancing 1 struct fwd
for one * and two fwd for two **.
*/
  I64 i;
  CHashClass *result=CAlloc(sizeof(CHashClass)*(MAX_PTR_STARS+1),Fs->code_heap),*tempc=result;
  for (i=0;i<=MAX_PTR_STARS;i++) {
    tempc->type=HTT_CLASS;
    tempc->raw_type=RT_PTR;
    tempc->size=sizeof(U8 *);
    tempc->ptr_cnt=i;
    tempc++;
  }
  result->last_in_member_lst=&result->member_lst;
  result->size=0;
  return result;
}

CHashFun *PrsFunNew()
{
  I64 i;
  CHashFun *result=CAlloc(sizeof(CHashFun)*(MAX_PTR_STARS+1),Fs->code_heap),*tempf=result;
  for (i=0;i<=MAX_PTR_STARS;i++) {
    tempf->type=HTT_FUN;
    tempf->raw_type=RT_PTR;
    tempf->size=sizeof(U8 *);
    tempf->ptr_cnt=i;
    tempf++;
  }
  result->last_in_member_lst=&result->member_lst;
  result->size=0;
  return result;
}

CMemberLst *MemberLstNew(I64 _reg)
{
  CMemberLst *result=CAlloc(sizeof(CMemberLst));
  result->reg=_reg;
  return result;
}

I64 PrsKeyWord(CLex *lx)
{//Cvt cur token to $LK,"KEYWORD",A="FF:::/Compiler/OpCodes.TXT.Z,KEYWORD"$ or -1.
  CHashGeneric *temph;
  if (lx->token==TK_IDENT &&(temph=lx->hash_entry) && temph->type&HTT_KEYWORD)
    return temph->user_data0;
  else
    return -1;
}

U0 UndefinedExtern()
{
  PrintErr("Undefined Extern\nat %P\n",Caller);
  throw('UndefExt');
}

U0 UnusedExternWarning(CLex *lx,CHashClass *tempc)
{
  PrintWarn("Unused extern '%s'\n",tempc->str);
  lx->warning_cnt++;
}

U0 PrsVarInit(CLex *lx,U8 **_dst,CHashClass *tempc,CArrayDim *tempad,U8 *data_addr_ip,U8 **_base,Bool data_heap,I64 pass)
{
  U8 *dst=*_dst,*ex;
  I64 i,j,r,old_flags,type,size;
  CMemberLst *tempm;
  CIntermediateCode *tempi;
  CAOTCtrl *a=lx->a;
  CExeAbsAddr *tempa;
  CExeImportExport *tempe;
  Bool is_str;

  tempc=OptClassFwd(tempc);
  if (tempm=tempc->member_lst) {
    if (lx->token!='{')
      LexExcept(lx,"expecting '{' at ");
    lx->stk_ptr--;  //grows up
    LexPush(lx);
    Lex(lx);
    while (tempm) {
      PrsVarInit2(lx,&dst,tempm->member_class,&tempm->dim,data_addr_ip,_base,data_heap,pass);
      if (lx->token==',')
	Lex(lx);
      tempm=tempm->next;
    }
    lx->stk_ptr--;  //grows up
    if (lx->token!='}')
      LexExcept(lx,"missing '}' at ");
    Lex(lx);
  } else {
    if (tempc->ptr_cnt==1 && ((tempc-1)->raw_type==RT_I8 || (tempc-1)->raw_type==RT_U8) &&
      !tempad && lx->token==TK_STR)
      is_str=TRUE;
    else
      is_str=FALSE;
    if (lx->flags&LF_AOT_COMPILE && is_str) {
      lx->stk_ptr--;	//grows up
      ex=LexExtStr(lx,&i);
      if (pass==2) {
	tempa=CAlloc(sizeof(CExeAbsAddr));
	tempa->next=a->abss;
	tempa->type=EAT_ADD_U64;
	a->abss=tempa;
	tempa->ip=data_addr_ip+dst-*_base;
	*dst(I64 *)=a->ip;
	for (j=0;j<i;j++)
	  AsmStoreCodeU8(lx,ex[j]);
      }
      Free(ex);
    } else {
      old_flags=lx->flags;
      lx->flags=LF_NO_ABSS | lx->flags & ~(LF_AOT_COMPILE|LF_HAS_MISC_DATA|LF_NOT_CONST);
      ex=LexExpression2Bin(lx,&type);
      if (old_flags&LF_AOT_COMPILE &&
	lx->flags&LF_NOT_CONST &&
	 !Bt(&lx->opts,OPTf_GLBLS_ON_DATA_HEAP)) {
	lx->flags=lx->flags&~LF_NO_ABSS|LF_AOT_COMPILE;
	Free(ex);
	if (pass==2) {
	  MemSet(dst,0,tempc->size);
	  LexPop(lx);
	  Lex(lx);
	  LCCPush(lx);
	  LCCInit(lx);
	  LCCAdd(lx,IC_ABS_ADDR,data_addr_ip,tempc+1);
	  LCCAdd(lx,IC_IMM_U64,dst-*_base,tempc+1);
	  LCCAdd(lx,IC_ADD,0,tempc+1);
	  if (!PrsExpression(lx,NULL,TRUE))
	    throw('Compiler');
	  tempi=lx->lcc.lcc_root.last;
	  if (tempi->ic_code==IC_END_EXP)
	    tempi->ic_code=IC_NOP1;
	  LCCAdd(lx,IC_ASSIGN,0,tempc);
	  LCCAdd(lx,IC_END_EXP,0,tempc);
	  LCCAdd(lx,IC_RET,0,0);
	  if (ex=LCCCompile(lx,&size,NULL,NULL)) {
	    tempe=CAlloc(sizeof(CExeImportExport));
	    tempe->type=EIT_MAIN;
	    tempe->ip=lx->a->ip;
	    QueIns(tempe,lx->exe->last_ie);
	    for (i=0;i<size;i++)
	      AsmStoreCodeU8(lx,ex[i]);
	    Free(ex);
	  }
	  LCCPop(lx);
	} else
	  lx->stk_ptr--;  //grows up
      } else {
	lx->stk_ptr--;	//grows up
	if (!ex)
	  throw('Compiler');
	r=Call(ex);
	if (!(lx->flags & LF_HAS_MISC_DATA)||pass==1)
	  Free(ex);

	if (type==RT_F64 &&
	    tempc->raw_type!=RT_F64)
	  r=r(F64);
	else if (type!=RT_F64 &&
	    tempc->raw_type==RT_F64)
	  r(F64)=r;
	MemCpy(dst,&r,tempc->size);
      }
    }
    dst+=tempc->size;
    lx->flags=lx->flags&~LF_NO_ABSS|old_flags&(LF_HAS_MISC_DATA|LF_AOT_COMPILE);
  }
  *_dst=dst;
}

class CVI2
{
  CVI2 *next,*last;
  U0 base;
};

U0 PrsVarInit2(CLex *lx,U8 **_dst,CHashClass *tempc,CArrayDim *tempad,U8 *data_addr_ip,U8 **_base,Bool data_heap,I64 pass)
{
  I64 i,j,cnt;
  U8 *st,*_b;
  CVI2 root,*tempvi,*tempvi1;
  CArrayDim *tempad1;
  tempc=OptClassFwd(tempc);
  if (tempad1=tempad->next) {
    if (!tempc->ptr_cnt &&
	(tempc->raw_type==RT_I8 || tempc->raw_type==RT_U8) &&
	lx->token==TK_STR) {
      lx->stk_ptr--;	//grows up
      st=LexExtStr(lx,&i);
      if (tempad1->cnt<0) {//[]
	tempad1->cnt=i;
	tempad->total_cnt=i*tempad1->total_cnt;
	Free(*_base);
	if (data_heap)
	  *_base=MAlloc(i);
	else
	  *_base=MAlloc(i,Fs->code_heap);
	MemCpy(*_base,st,i);
	*_dst=*_base+i;
      } else {
	MemCpy(*_dst,st,tempad1->cnt);
	*_dst+=tempad1->cnt;
      }
      Free(st);
      LexPush(lx);
    } else {
      if (lx->token=='{') {
	lx->stk_ptr--;	//grows up
	LexPush(lx);
	Lex(lx);
      }
      if (tempad1->cnt<0) {//[]
	QueInit(&root);
	cnt=0;
	while (lx->token!='}') {
	  tempvi=MAlloc(offset(CVI2.base)+tempad1->total_cnt*tempc->size);
	  _b=&tempvi->base;
	  PrsVarInit2(lx,&_b,tempc,tempad1,data_addr_ip,_base,data_heap,pass);
	  QueIns(tempvi,root.last);
	  if (lx->token==',')
	    Lex(lx);
	  cnt++;
	}
	Lex(lx); //skip '}'
	tempad1->cnt=cnt;
	tempad->total_cnt=cnt*tempad1->total_cnt;
	j=tempad1->total_cnt*tempc->size;
	i=cnt*j;
	Free(*_base);
	if (data_heap)
	  *_base=_b=MAlloc(i);
	else
	  *_base=_b=MAlloc(i,Fs->code_heap);
	tempvi=root.next;
	while (tempvi!=&root) {
	  tempvi1=tempvi->next;
	  MemCpy(_b,&tempvi->base,j);
	  _b+=j;
	  Free(tempvi);
	  tempvi=tempvi1;
	}
	*_dst=_b;
      } else {
	for (i=0;i<tempad1->cnt;i++) {
	  PrsVarInit2(lx,_dst,tempc,tempad1,data_addr_ip,_base,data_heap,pass);
	  if (tempad1->cnt>1 && lx->token==',')
	    Lex(lx);
	}
	if (lx->token=='}')
	  Lex(lx);
      }
    }
  } else {
    PrsVarInit(lx,_dst,tempc,tempad1,data_addr_ip,_base,data_heap,pass);
    LexPush(lx);
  }
}

U0 PrsGlblInit(CLex *lx,CHashGlblVar *tempg,I64 pass)
{
  U8 *dst=tempg->data_addr;
  PrsVarInit2(lx,&dst,tempg->var_class,&tempg->dim,
    tempg->data_addr_ip,&tempg->data_addr,
    Bt(&lx->opts,OPTf_GLBLS_ON_DATA_HEAP)||
    Bt(&lx->flags,Lf_AOT_COMPILE),pass);
}

U0 PrsStaticInit(CLex *lx,CMemberLst *tempm,I64 pass)
{
  U8 *ex,*dst=tempm->static_data;
  CHashClass *tempc=tempm->member_class;
  I64 i,size;
  CExeImportExport *tempe;

  if (lx->flags&LF_AOT_COMPILE && pass==2) {
    LCCPush(lx);
    LCCInit(lx);
  }
  PrsVarInit2(lx,&dst,tempc,&tempm->dim,tempm->static_data_ip,&tempm->static_data,Bt(&lx->flags,Lf_AOT_COMPILE),pass);
  if (lx->flags&LF_AOT_COMPILE && pass==2) {
    if (lx->lcc.lcc_root.next!=&lx->lcc.lcc_root) {
      LCCAdd(lx,IC_RET,0,0);
      if (ex=LCCCompile(lx,&size,NULL,NULL)) {
	if (pass==2) {
	  tempe=CAlloc(sizeof(CExeImportExport));
	  tempe->type=EIT_MAIN;
	  tempe->ip=lx->a->ip;
	  QueIns(tempe,lx->exe->last_ie);
	  for (i=0;i<size;i++)
	    AsmStoreCodeU8(lx,ex[i]);
	}
	Free(ex);
      }
    } //TODO: else del misc?
    LCCPop(lx);
  }
}

Bool PrsArrayDims(CLex *lx,CArrayDim *dim)
{//Returns false on empty []
//dim->next!=0 for array
  CArrayDim *tempad,*tempad1;
  I64 j;
  Bool result=TRUE;
  dim->next=NULL;
  dim->cnt=0;
  dim->total_cnt=1;
  tempad1=&dim->next;
  while (lx->token=='[') {
    if (Lex(lx)==']' && !dim->next) {
      j=-1;
      result=FALSE;
    } else {
      if ((j=LexExpressionI64(lx))<0)
	LexExcept(lx,"Invalid array size at ");
    }
    tempad1=&dim->next;
    while (tempad1->next) {
      tempad1->next->total_cnt*=j;
      tempad1=tempad1->next;
    }
    tempad=MAlloc(sizeof(CArrayDim));
    tempad1->next=tempad;
    tempad1=tempad;
    tempad->next=NULL;
    tempad->cnt=j;
    tempad->total_cnt=1;
    dim->total_cnt*=j;
    if (lx->token!=']')
      LexExcept(lx,"missing ']' at ");
    Lex(lx);
  }
  return result;
}

U0 PrsDotDotDot(CLex *lx,CHashFun *tempf,I64 _reg)
{
  CMemberLst *tempm;
  CArrayDim *tempad;

  Bts(&tempf->flags,Ff_DOT_DOT_DOT);

  Lex(lx);
  tempm=MemberLstNew(_reg);
  tempm->flags=MLF_DOT_DOT_DOT;
  tempm->member_class=cmp.internal_types[RT_I64];
  tempm->str=StrNew("argc");
  tempm->offset=tempf->size;
  tempm->size=8;
  tempf->size+=8;
  MemberAdd(lx,tempm,tempf,PRS1B_FUN_ARG);

  tempm=MemberLstNew(_reg);
  tempm->flags=MLF_DOT_DOT_DOT;
  tempm->member_class=cmp.internal_types[RT_I64];
  tempm->str=StrNew("argv");
  tempm->dim.total_cnt=127; //arbitrary
  tempm->dim.next=tempad=MAlloc(sizeof(CArrayDim));
  tempad->next=NULL;
  tempad->cnt=127; //arbitrary
  tempad->total_cnt=1;
  tempm->offset=tempf->size;
  tempm->size=8; //Close enough
  tempf->size+=8;//Close enough
  MemberAdd(lx,tempm,tempf,PRS1B_FUN_ARG);

  if (lx->token==')')
    Lex(lx);
}

CHashClass *PrsType(CLex *lx,CHashClass **_tempc1,
  I64 *_mode,CMemberLst *tempm,U8 **_ident,CHashFun **_fun_ptr,
  CHashExport **_tempex,CArrayDim *tempad,I64 fsp_flags)
{
  I64 k,ptr_cnt,mode=*_mode;
  CHashClass *tempc1=*_tempc1,*tempc2;
  CHashFun *fun_ptr=NULL;
  CHashExport *tempex=NULL;
  CArrayDim *tempad2;

pt_start:
  if (!tempc1 || !(tempc1->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
    LexExcept(lx,"invalid class at ");
 
  ptr_cnt=0;
  while (lx->token=='*') {
    if (mode.u8[1]) {
      lx->stk_ptr--;	//grows up
      LexPush(lx);
    }
    Lex(lx);
    tempc1++;
    if (++ptr_cnt>MAX_PTR_STARS)
      LexExcept(lx,"Too many *'s at ");
  }

  k=PrsKeyWord(lx);
  if (k==KW_UNION || k==KW_CLASS) {
    Lex(lx);
    tempc2=PrsClass(lx,k,fsp_flags,mode&255==PRS0_EXTERN);
    tempc2->fwd_class=tempc1;
    tempc1=tempc2;
    if (_tempc1) *_tempc1=tempc1;
    mode=PRS0_NULL|PRS1_NULL;
    goto pt_start;
  }

  if (lx->token=='(') {
    if (Lex(lx)!='*')
      LexExcept(lx,"expecting '*' at ");
    ptr_cnt=1; //fun_ptr
    while (Lex(lx)=='*')
      ptr_cnt++; //fun_ptr
    if (ptr_cnt>MAX_PTR_STARS)
      LexExcept(lx,"Too many *'s at ");
  } else
    ptr_cnt=-1; //fun_ptr

  if (_ident) {
    if (lx->token==TK_IDENT) {
      tempex=lx->hash_entry;
      *_ident=lx->cur_str;
      lx->cur_str=NULL;
      Lex(lx);
    } else {
      if (!mode.u8[1])
	*_ident=NULL;
      else if (lx->token==',' || lx->token==';' || lx->token==')') {
	tempex=NULL;
	*_ident=StrNew("_anon_");
	tempm->flags|=MLF_NO_UNUSED_WARN;
      } else
	LexExcept(lx,"expecting identifier at ");
    }
  }

  if (ptr_cnt>=0) { //fun_ptr
    if (lx->token!=')')
      LexExcept(lx,"missing ')' at ");
    if (Lex(lx)!='(')
      LexExcept(lx,"expecting '(' at ");
    fun_ptr=PrsFunJoin(lx,tempc1,NULL,fsp_flags)+ptr_cnt;
    tempc1=cmp.internal_types[RT_PTR]+ptr_cnt;
  }
  if (!PrsArrayDims(lx,tempad) && lx->token!='=') {
    tempc1++;
    tempad2=tempad->next->next;
    Free(tempad->next);
    tempad->next=tempad2;
  }

  tempc2=OptClassFwd(tempc1);
  if (tempc2->ptr_cnt) {
    tempc2-=tempc2->ptr_cnt;
    if (tempc2->type&HTT_INTERNAL_TYPE && !tempc2->size)
      LexWarn(lx,"use \"U8 *\" instead of \"U0 *\" at ");
  }

  if (_mode)	*_mode=mode;
  if (_fun_ptr)	*_fun_ptr=fun_ptr;
  if (_tempex)	*_tempex=tempex;
  return tempc1;
}

U0 PrsVarLst(CLex *lx,CHashClass *tempc,I64 mode,I64 union_base=0)
{
  I64 i,k,old_flags=lx->flags,old_flags2,type,_reg;
  CHashClass *tempc1,*tempc2;
  CHash *temph;
  CMemberLst *tempm;
  CMemberLstMeta *temp_meta;
  U8 *ex;
  Bool undef_array_size,first;
  lx->flags|=LF_DONT_MAKE_RESULT;
  if (mode.u8[1]==PRS1B_CLASS)
    lx->flags|=LF_CLASS_IP;
  if (mode.u8[1]!=PRS1B_LOCAL_VAR && mode.u8[1]!=PRS1B_STATIC_LOCAL_VAR || mode&PRSF_UNION) {
    if (lx->token=='(' || lx->token=='{')
      Lex(lx);
  }
  while (TRUE) {
    while (lx->token==';')
      Lex(lx);
    if (mode&PRSF_UNION)
      lx->class_ip=union_base;
    else
      lx->class_ip=tempc->size;
    while (lx->token=='$$') {
      if (Lex(lx)!='=') //skip $$
	LexExcept(lx,"expecting '=' at ");
      Lex(lx); //skip =
      lx->class_ip=LexExpression(lx);
      if (-lx->class_ip>tempc->neg_offset)
	tempc->neg_offset=-lx->class_ip;
      if (mode&PRSF_UNION)
	union_base=lx->class_ip;
      else
	tempc->size=lx->class_ip;
      if (lx->token!=';')
	LexExcept(lx,"missing ';' at");
      Lex(lx); //skip ;
    }
    if (lx->token==')' || lx->token=='}') {
      Lex(lx);
      goto pvl_done;
    }
    _reg=REG_UNDEF;
pvl_restart1:
    switch (PrsKeyWord(lx)) {
      case KW_REG:
	_reg=REG_ALLOC;
	if (Lex(lx)==TK_IDENT) {
	  k=DefineMatch(lx->cur_str,"ST_U64_REGS");
	  if (k>=0) {
	    _reg=k;
	    Lex(lx);
	  }
	}
	goto pvl_restart1;
      case KW_NOREG:
	_reg=REG_NONE;
	Lex(lx);
	goto pvl_restart1;
    }

    if (lx->token==TK_DOT_DOT_DOT && mode.u8[1]==PRS1B_FUN_ARG) {
      PrsDotDotDot(lx,tempc,_reg);
      goto pvl_done;
    }
    if (lx->token==TK_IDENT)
      temph=lx->hash_entry;
    else
      temph=NULL;
    if (!temph)
      LexExcept(lx,"expecting type at ");
    k=PrsKeyWord(lx);
    if (k==KW_UNION) {
      Lex(lx);
      PrsVarLst(lx,tempc,mode|PRSF_UNION,tempc->size);
    } else {
      if (!(temph->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
	LexExcept(lx,"expecting type at ");
      first=TRUE;
pvl_restart2:
      tempc1=temph;
      LexPush(lx);
      Lex(lx); //skip type or ','
      tempm=MemberLstNew(_reg);
      _reg=REG_UNDEF;
      if (mode.u8[1]==PRS1B_STATIC_LOCAL_VAR) {
	tempm->flags|=MLF_STATIC;
	tempm->reg=REG_NONE;
      }
      if (mode.u8[1]==PRS1B_FUN_ARG || mode.u8[1]==PRS1B_LOCAL_VAR) {
pvl_restart3:
	switch (PrsKeyWord(lx)) {
	  case KW_REG:
	    tempm->reg=REG_ALLOC;
	    lx->stk_ptr--;  //grows up
	    LexPush(lx);
	    if (Lex(lx)==TK_IDENT) {
	      k=DefineMatch(lx->cur_str,"ST_U64_REGS");
	      if (k>=0) {
		tempm->reg=k;
		lx->stk_ptr--;	//grows up
		LexPush(lx);
		Lex(lx);
	      }
	    }
	    goto pvl_restart3;
	  case KW_NOREG:
	    tempm->reg=REG_NONE;
	    lx->stk_ptr--;  //grows up
	    LexPush(lx);
	    Lex(lx);
	    goto pvl_restart3;
	}
      }
      tempm->member_class=PrsType(lx,&tempc1,&mode,tempm,&tempm->str,
	  &tempm->fun_ptr,NULL,&tempm->dim,0);
      if (tempm->fun_ptr)
	tempm->flags|=MLF_FUN;
      if (first)
	MemberAdd(lx,tempm,tempc,mode.u8[1]);
      else
	MemberAdd(lx,tempm,tempc,PRS1B_NULL);
      tempc->member_cnt++;

      tempc2=tempm->member_class;
      i=tempc2->size*tempm->dim.total_cnt;
      switch (mode.u8[1]) {
	case PRS1B_STATIC_LOCAL_VAR:
	  if (i<0) {
	    i=0;
	    undef_array_size=TRUE;
	  } else
	    undef_array_size=FALSE;
	  if (mode&PRSF_UNION)
	    LexExcept(lx,"Static unions are not implemented ");
	  k=(i+7)&~7;
	  if (lx->flags&LF_AOT_COMPILE)
	    tempm->static_data=MAlloc(k);
	  else
	    tempm->static_data=MAlloc(k,Fs->code_heap);
	  if (lx->flags&LF_AOT_COMPILE)	{
	    tempm->static_data_ip=lx->a->ip;
	    k>>=3;
	    if (sys_var_init_flag&1) {
	      k<<=3;
	      while (k--)
		AsmStoreCodeU8(lx,sys_var_init_val);
	    } else
	      while (k--)
		AsmStoreCodeU64(lx,0);
	  } else
	  if (sys_var_init_flag&1)
	    MemSet(tempm->static_data,sys_var_init_val,k);
	  lx->stk_ptr--;
	  if (lx->token=='=') {
	    lx->flags=lx->flags&~LF_DONT_MAKE_RESULT|old_flags&LF_DONT_MAKE_RESULT;
	    if (undef_array_size) {
	      LexPush(lx);
	      LexPush(lx);
	      Lex(lx); //skip =
	      PrsStaticInit(lx,tempm,1);
	      lx->stk_ptr--;
	      i=tempc2->size*tempm->dim.total_cnt;
	      k=(i+7)&~7;
	      if (lx->flags&LF_AOT_COMPILE)	{
		k>>=3;
		if (sys_var_init_flag&1) {
		  k<<=3;
		  while (k--)
		    AsmStoreCodeU8(lx,sys_var_init_val);
		} else
		  while (k--)
		    AsmStoreCodeU64(lx,0);
	      } else
	      if (sys_var_init_flag&1)
		MemSet(tempm->static_data,sys_var_init_val,k);
	      LexPop(lx);
	    }
	    LexPush(lx);
	    Lex(lx); //skip =
	    PrsStaticInit(lx,tempm,2);
	    lx->stk_ptr--;
	    if (lx->flags&LF_AOT_COMPILE)
	      for (k=0;k<i;k++)
		AsmStoreCodeU8At(lx,tempm->static_data_ip+k,tempm->static_data[k]);
	      tempm->use_cnt=0;
	    lx->flags|=LF_DONT_MAKE_RESULT;
	  }
	  if (lx->flags&LF_AOT_COMPILE)
	    Free(tempm->static_data);
	  break;
	case PRS1B_LOCAL_VAR:
	  if (mode&PRSF_UNION) {
	    if (union_base-tempc->size<i)
	      i=union_base-i-tempc->size;
	    else
	      i=0;
	  }
	  if (i>=8)
	    tempc->size=(tempc->size-i)&~7;
	  else if (i>=4)
	    tempc->size=(tempc->size-i)&~3;
	  else if (i>=2)
	    tempc->size=(tempc->size-i)&~1;
	  else
	    tempc->size-=i;
	  tempm->offset=tempc->size;
	  tempm->size=i;
	  if (lx->token=='=') {
	    lx->flags=lx->flags&~LF_DONT_MAKE_RESULT|old_flags&LF_DONT_MAKE_RESULT;
	    LexPop(lx);
	    Lex(lx);
	    if (!PrsExpression(lx,NULL,TRUE))
	      throw('Compiler');
	    tempm->use_cnt=0;
	    lx->flags|=LF_DONT_MAKE_RESULT;
	  } else
	    lx->stk_ptr--;
	  break;
	case PRS1B_FUN_ARG:
	  if (mode&PRSF_UNION) {
	    tempm->offset=union_base;
	    if (tempc->size-union_base<8)
	      tempc->size=8+union_base;
	  } else {
	    tempm->offset=tempc->size;
	    tempc->size+=8;
	  }
	  tempm->size=8;
	  if (lx->token=='=') {
	    Lex(lx);
	    if (PrsKeyWord(lx)==KW_LASTCLASS) {
	      tempm->flags|=MLF_LASTCLASS;
	      Lex(lx);
	    } else {
	      old_flags2=lx->flags;
	      lx->flags&=~LF_HAS_MISC_DATA;
	      ex=LexExpression2Bin(lx,&type);
	      if (!ex)
		throw('Compiler');
	      tempm->dft_val=Call(ex);
	      tempc2=OptClassFwd(tempc2);
	      if (tempc2->raw_type==RT_F64) {
		if (type!=RT_F64)
		  tempm->dft_val(F64)=tempm->dft_val;
	      } else {
		if (type==RT_F64)
		  tempm->dft_val=tempm->dft_val(F64);
	      }
	      if (lx->flags & LF_HAS_MISC_DATA) {
		tempm->dft_val=StrNew(tempm->dft_val);
		tempm->flags|=MLF_STR_DFT_AVAILABLE;
	      }
	      Free(ex);
	      lx->flags|=old_flags2&LF_HAS_MISC_DATA;
	    }
	    tempm->flags|=MLF_DFT_AVAILABLE;
	  }
	  lx->stk_ptr--;
	  break;
	case PRS1B_CLASS:
	  if (mode&PRSF_UNION) {
	    tempm->offset=union_base;
	    if (tempc->size-union_base<i)
	      tempc->size=i+union_base;
	  } else {
	    tempm->offset=tempc->size;
	    tempc->size+=i;
	  }
	  tempm->size=i;
	  while (lx->token==TK_IDENT) {
	    temp_meta=MAlloc(sizeof(CMemberLstMeta));
	    temp_meta->next=tempm->meta;
	    tempm->meta=temp_meta;
	    temp_meta->str=lx->cur_str;
	    temp_meta->flags=0;
	    lx->cur_str=NULL;
	    if (Lex(lx)==TK_STR) {
	      temp_meta->user_data=LexExtStr(lx);
	      temp_meta->flags|=MLMF_IS_STR;
	    } else
	      temp_meta->user_data=LexExpression(lx);
	  }
	  lx->stk_ptr--;
	  break;
      }
      switch (lx->token) {
	case ',':
	  if (mode.u8[1]==PRS1B_FUN_ARG && !(mode&PRSF_UNION))
	    Lex(lx);
	  else {
	    first=FALSE;
	    goto pvl_restart2;
	  }
	  break;
	case ')':
	case '}':
	  Lex(lx);
	  goto pvl_done;
	case ';':
	  lx->flags=lx->flags&~LF_DONT_MAKE_RESULT|old_flags&LF_DONT_MAKE_RESULT;
	  Lex(lx);
	  lx->flags|=LF_DONT_MAKE_RESULT;
	  if ((mode.u8[1]==PRS1B_LOCAL_VAR||mode.u8[1]==PRS1B_STATIC_LOCAL_VAR) && !(mode&PRSF_UNION))
	    goto pvl_done;
	  break;
	default:
	  LexExcept(lx,"missing ';' at");
      }
    }
  }
pvl_done:
  lx->flags=lx->flags&~(LF_CLASS_IP|LF_DONT_MAKE_RESULT)|
	    old_flags& (LF_CLASS_IP|LF_DONT_MAKE_RESULT);
}

CHashClass *PrsClass(CLex *lx,I64 keyword,I64 fsp_flags,Bool is_extern)
{
  CHashClass *tempc,*base_class;
  if (lx->token!=TK_IDENT)
    LexExcept(lx,"expecting identifier at ");
  if (is_extern) {
    tempc=PrsClassNew;
    tempc->str=lx->cur_str;
    lx->cur_str=NULL;
    HashAdd(tempc,lx->htc.glbl_hash_table);
    LBts(&tempc->flags,Cf_EXTERN);
    SetSrcFileInfo(lx,tempc);
    Lex(lx);
  } else {
    if (lx->flags&LF_AOT_COMPILE) {
      if (tempc=HashFind(lx->cur_str,lx->htc.glbl_hash_table,HTT_CLASS)) {
	if (!Bt(&tempc->flags,Cf_EXTERN))
	  tempc=NULL;
	else if (tempc->use_cnt<3)
	  UnusedExternWarning(lx,tempc);
      }
    } else {
      if (tempc=HashSingleTableFind(lx->cur_str,lx->htc.glbl_hash_table,HTT_CLASS)) {
	if (!Bt(&tempc->flags,Cf_EXTERN))
	  tempc=NULL;
	else if (tempc->use_cnt<3)
	  UnusedExternWarning(lx,tempc);
      }
    }
    if (tempc) {
      Free(tempc->src_link);
      tempc->src_link=NULL;
      Free(tempc->index);
      tempc->index=NULL;
    } else {
      tempc=PrsClassNew;
      tempc->str=lx->cur_str;
      lx->cur_str=NULL;
      HashAdd(tempc,lx->htc.glbl_hash_table);
    }
    LBtr(&tempc->flags,Cf_EXTERN);
    if (fsp_flags&FSF_PUBLIC)
      tempc->type|=HTF_PUBLIC;
    tempc->use_cnt=0;
    if (lx->last_U16=='\n')
      SetSrcFileInfo(lx,tempc,-1);
    else
      SetSrcFileInfo(lx,tempc,0);
    if (Lex(lx)==':') {
      if (Lex(lx)!=TK_IDENT || !(base_class=lx->hash_entry) ||
	!(base_class->type&HTT_CLASS))
	LexExcept(lx,"invalid class at ");
      if (Lex(lx)==',')
	LexExcept(lx,"Only one base class allowed at this time at ");
      tempc->base_class=base_class;
      tempc->size+=base_class->size;
    }
    if (keyword==KW_UNION)
      PrsVarLst(lx,tempc,PRS0_NULL|PRS1_CLASS|PRSF_UNION);
    else
      PrsVarLst(lx,tempc,PRS0_NULL|PRS1_CLASS);
    tempc->size+=tempc->neg_offset;
  }
  return tempc;
}

CHashFun *PrsFunJoin(CLex *lx,CHashClass *temp_return,U8 *name,I64 fsp_flags)
{
  CMemberLst *tempm;
  CAOTCtrl *a=lx->a;
  CHashFun *tempf;
  if (name) {//if not fun_ptr
    if (lx->flags&LF_AOT_COMPILE) {
      if (tempf=HashFind(name,lx->htc.glbl_hash_table,HTT_FUN)) {
	if (tempf->type & HTF_IMPORT)
	  tempf=NULL;
	else
	  if (tempf->use_cnt<3)
	    UnusedExternWarning(lx,tempf);
      }
    } else {
      if (tempf=HashSingleTableFind(name,lx->htc.glbl_hash_table,HTT_FUN)) {
	if (!Bt(&tempf->flags,Cf_EXTERN))
	  tempf=NULL;
	else if (tempf->use_cnt<3)
	  UnusedExternWarning(lx,tempf);
      }
    }
  } else
    tempf=NULL;
  if (tempf) {
    tempf->used_reg_mask=REG_ALWAYS_CLOBBERED_MASK+REG_STK_TEMPS_MASK;
    Free(tempf->src_link);
    tempf->src_link=NULL;
    Free(tempf->index);
    tempf->index=NULL;
    Free(name);
    MemberLstDel(tempf);
  } else {
    tempf=PrsFunNew;
    tempf->used_reg_mask=REG_ALWAYS_CLOBBERED_MASK+REG_STK_TEMPS_MASK;
    tempf->clobbered_reg_mask=REG_ALWAYS_CLOBBERED_MASK+REG_STK_TEMPS_MASK;
    tempf->str=name;
    if (lx->flags&LF_AOT_COMPILE)
      tempf->executable_addr=a->ip;
    else
      tempf->executable_addr=&UndefinedExtern;
    LBts(&tempf->flags,Cf_EXTERN);
    tempf->flags|=fsp_flags&FSG_FUN_FLAGS1;
    if (name) //if not fun_ptr
      HashAdd(tempf,lx->htc.glbl_hash_table);
  }
  BEqu(&tempf->type,HTf_PUBLIC,fsp_flags&FSF_PUBLIC);
  tempf->return_class=temp_return;
  tempf->use_cnt=0;
  SetSrcFileInfo(lx,tempf);
  PrsVarLst(lx,tempf,PRS0_NULL|PRS1_FUN_ARG);
  tempf->arg_cnt=tempf->member_cnt;
  if (0<tempf->arg_cnt<<3<=MAX_I16 && !Bt(&tempf->flags,Ff_DOT_DOT_DOT))
    LBts(&tempf->flags,Ff_RET1);
  tempm=tempf->member_lst;
  while (tempm) {
    tempm->offset+=16; //RBP+RETURN
    tempm=tempm->next;
  }
  tempf->size=0;
  return tempf;
}

U0 PrsFun(CLex *lx,CHashClass *temp_return,U8 *name,I64 fsp_flags)
{
  CMemberLst *tempm;
  CLexCodeMisc *saved_leave_label;
  I64 i,j,size,*r;
  Bool old_ctrace;

  lx->fun_lfn=lx->cur_lfn;
  lx->min_line=lx->max_line=lx->cur_lfn->line_num;

  lx->flags&=~LF_NO_REG_OPT;
  lx->htc.local_var_lst=lx->htc.fun=PrsFunJoin(lx,temp_return,name,fsp_flags);

  LCCPush(lx);
  Btr(&lx->flags,Lf_OPT_TRACE_PRESENT);
  LCCInit(lx);
  LCCAdd(lx,IC_ENTER,0,0);
  saved_leave_label=lx->lb_leave;
  lx->lb_leave=LCCMiscNew(lx,CMT_LABEL);
  lx->flags&=~LF_HAS_RETURN;
  PrsStatement(lx,,,0);
  if (lx->htc.fun->return_class->size && !(lx->flags&LF_HAS_RETURN))
    LexWarn(lx,"Function should return val ");
  LCCAdd(lx,IC_LABEL,lx->lb_leave,0);
  lx->lb_leave=saved_leave_label;
  LCCAdd(lx,IC_LEAVE,0,lx->htc.fun->return_class);
  lx->htc.fun->size&=~7;
  if (lx->flags&LF_AOT_COMPILE) {
    lx->htc.fun->executable_addr=lx->a->ip;
    lx->htc.fun->type|=HTF_EXPORT|HTF_RESOLVE;
    r=LCCCompile(lx,&size,&lx->htc.fun->dbg_info,NULL);
    if (r) {
      j=(size+7)>>3;
      for (i=0;i<j;i++)
	AsmStoreCodeU64(lx,r[i]);
      Free(r);
    }
  } else {
    old_ctrace=Btr(&lx->opts,OPTf_COMPILE_TRACE);
    lx->htc.fun->executable_addr=LCCCompile(lx,&size,&lx->htc.fun->dbg_info,NULL);
    if (old_ctrace) {
      Bts(&lx->opts,OPTf_COMPILE_TRACE);
      Un(lx->htc.fun->executable_addr,size,64);
    }
    SysSymImportsResolve(lx->htc.fun->str);
  }
  LBtr(&lx->htc.fun->flags,Cf_EXTERN);
  LCCPop(lx);
  tempm=lx->htc.fun->member_lst;
  while (tempm) {
    if (tempm->flags & MLF_NO_UNUSED_WARN) {
      if (tempm->use_cnt>1&&StrCmp(tempm->str,"_anon_"))
	PrintWarn("Unneeded nounusedwarn\n $$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
	  lx->cur_lfn->name,lx->cur_lfn->line_num,tempm->str,lx->htc.fun->str);
    } else if (!tempm->use_cnt && GetOpt(OPTf_WARN_UNUSED_VAR))
      PrintWarn("Unused var\n $$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
	lx->cur_lfn->name,lx->cur_lfn->line_num,tempm->str,lx->htc.fun->str);
    tempm=tempm->next;
  }
  lx->htc.local_var_lst=lx->htc.fun=lx->fun_lfn=NULL;
}

U0 PrsGlblVarLst(CLex *lx,I64 saved_mode,CHashClass *saved_tempc,
  I64 saved_val,I64 fsp_flags)
{
  I64 i,j,mode,k,val;
  U8 *st;
  CHashExport *tempex;
  CHashGlblVar *tempg;
  CAOTCtrl *a=lx->a;
  CExeHeapGlbl *temphg;
  CHashClass *tempc;
  CHashFun *tempf,*tempf_fun_ptr;
  CArrayDim tempad;
  Bool has_alias,undef_array_size,is_array;
  while (TRUE) {
    tempc=PrsType(lx,&saved_tempc,&saved_mode,NULL,&st,
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
    if (tempex && mode&255==PRS0_EXTERN && !(lx->flags&LF_AOT_COMPILE) &&
	  tempex->type & HTT_EXPORT_SYS_SYM) {
      val=tempex->val;
      mode=PRS0__EXTERN|PRS1_NULL;
    }
    if (lx->token=='(') {
      switch (mode&255) {
	case PRS0__INTERN:
	  tempf=PrsFunJoin(lx,tempc,st,fsp_flags);
	  tempf->executable_addr=val;
	  Bts(&tempf->flags,Ff_INTERNAL);
	  LBtr(&tempf->flags,Cf_EXTERN);
	  return;
	case PRS0__EXTERN:
	  tempf=PrsFunJoin(lx,tempc,st,fsp_flags);
	  tempf->executable_addr=val;
	  SysSymImportsResolve(tempf->str);
	  LBtr(&tempf->flags,Cf_EXTERN);
	  if (saved_mode&255==PRS0__EXTERN)
	    LBts(&tempf->flags,Ff__EXTERN);
	  if (lx->flags&LF_AOT_COMPILE)
	    tempf->type|=HTF_RESOLVE;
	  return;
	case PRS0_EXTERN:
	  PrsFunJoin(lx,tempc,st,fsp_flags);
	  return;
	case PRS0_IMPORT:
	case PRS0__IMPORT:
	  if (!(lx->flags&LF_AOT_COMPILE))
	    LexExcept(lx,"import not needed at ");
	  else {
	    tempf=PrsFunJoin(lx,tempc,st,fsp_flags);
	    tempf->type|=HTF_IMPORT;
	    if (mode&255==PRS0__IMPORT)
	      tempf->import_name=StrNew(val);
	    else
	      tempf->import_name=StrNew(st);
	  }
	  return;
	default:
	  PrsFun(lx,tempc,st,fsp_flags);
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
	  if (lx->flags&LF_AOT_COMPILE) {
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
	  if (!(lx->flags&LF_AOT_COMPILE))
	    LexExcept(lx,"import not needed at ");
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
	  if (lx->flags&LF_AOT_COMPILE) {
	    tempg=CAlloc(sizeof(CHashGlblVar));
	    tempg->type=HTT_GLBL_VAR;
	  } else {
	    tempg=CAlloc(sizeof(CHashGlblVar),Fs->code_heap);
	    tempg->type=HTT_GLBL_VAR|HTF_UNRESOLVED;
	  }
	  break;
	default:
	  if (lx->flags&LF_AOT_COMPILE) {
	    if (Bt(&lx->opts,OPTf_GLBLS_ON_DATA_HEAP)) {
	      if (lx->token=='=')
		LexExcept(lx,"Can't init glbl var on data heap in AOT module ");
	      tempg=CAlloc(sizeof(CHashGlblVar));
	      temphg=tempg->heap_glbl=CAlloc(sizeof(CExeHeapGlbl));
	      temphg->size=j;
	      temphg->str=StrNew(st);
	      temphg->next=a->heap_glbls;
	      a->heap_glbls=temphg;
	      tempg->flags=GVF_DATA_HEAP;
	      tempg->type=HTT_GLBL_VAR; //TODO: HTF_EXPORT
	      if (tempex && tempex->type & HTT_GLBL_VAR) //TODO!! extern
		LexExcept(lx,"Feature not implemented ");
	    } else {
	      tempg=CAlloc(sizeof(CHashGlblVar));
	      if (lx->token=='=')
		tempg->data_addr=CAlloc(j);
	      if (tempc->size>=8) //align
		while (a->ip&7)
		  AsmStoreCodeU8(lx,0);
	      else if (tempc->size==4)
		while (a->ip&3)
		  AsmStoreCodeU8(lx,0);
	      else if (tempc->size==2)
		while (a->ip&1)
		  AsmStoreCodeU8(lx,0);
	      tempg->data_addr_ip=a->ip;
	      tempg->type=HTT_GLBL_VAR | HTF_EXPORT;
	      if (tempex && tempex->type & HTT_GLBL_VAR)
		has_alias=TRUE;
	      if (sys_var_init_flag&1)
		for (k=0;k<j;k++)
		  AsmStoreCodeU8(lx,sys_var_init_val);
	      else
		for (k=0;k<j;k++)
		  AsmStoreCodeU8(lx,0);
	    }
	  } else {
	    if (Bt(&lx->opts,OPTf_GLBLS_ON_DATA_HEAP)) {
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
      if (lx->last_U16=='\n')
	SetSrcFileInfo(lx,tempg,-1);
      else
	SetSrcFileInfo(lx,tempg,0);
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
      HashAdd(tempg,lx->htc.glbl_hash_table);
      if (!(lx->flags&LF_AOT_COMPILE) && !(tempg->flags&GVF_EXTERN))
	SysSymImportsResolve(tempg->str);
      if (lx->token=='=') {
	if (undef_array_size) {
	  LexPush(lx);
	  LexPush(lx);
	  Lex(lx);
	  PrsGlblInit(lx,tempg,1);
	  lx->stk_ptr--;	//grows up
	  tempg->size=tempg->dim.total_cnt*tempc->size;
	  if (temphg)
	    temphg->size=tempg->size;
	  if (lx->flags&LF_AOT_COMPILE) {
	    if (sys_var_init_flag&1)
	      for (k=0;k<tempg->size;k++)
		AsmStoreCodeU8(lx,sys_var_init_val);
	    else
	      for (k=0;k<tempg->size;k++)
		AsmStoreCodeU8(lx,0);
	  } else
	    if (sys_var_init_flag&1)
	      MemSet(tempg->data_addr,sys_var_init_val,k);
	  LexPop(lx);
	}
	LexPush(lx);
	Lex(lx);
	PrsGlblInit(lx,tempg,2);
	if (lx->flags&LF_AOT_COMPILE)
	  for (k=0;k<tempg->size;k++)
	    AsmStoreCodeU8At(lx,tempg->data_addr_ip+k,tempg->data_addr[k]);
	lx->stk_ptr--;	//grows up
      }
      if (has_alias) {
	if (tempex(CHashGlblVar *)->use_cnt<2) {
	  PrintWarn("Unused extern '%s'\n",tempex(CHashGlblVar *)->str);
	  lx->warning_cnt++;
	}
	tempex(CHashGlblVar *)->flags|=GVF_ALIAS;
	tempex(CHashGlblVar *)->data_addr=tempg->data_addr;
	tempex(CHashGlblVar *)->data_addr_ip=tempg->data_addr_ip;
      }
      if (lx->token==',')
	Lex(lx);
      else {
	if (lx->token!=';')
	  LexExcept(lx,"missing ';' at");
	Lex(lx);
	return;
      }
    }
  }
}

U0 PrsIf(CLex *lx,I64 try_cnt,CLexCodeMisc *lb_break)
{
  CLexCodeMisc *lb,*lb1;
  I64 k;

  if (lx->token!='(')
    LexExcept(lx,"expecting '(' at ");
  Lex(lx);
  if (!PrsExpression(lx,NULL,FALSE))
    throw('Compiler');
  if (lx->token!=')')
    LexExcept(lx,"missing ')' at ");
  Lex(lx);
  lb=LCCMiscNew(lx,CMT_LABEL);
  LCCAdd(lx,IC_BR_ZERO,lb,0);
  PrsStatement(lx,try_cnt,lb_break);
  k=PrsKeyWord(lx);
  if (k==KW_ELSE) {
    Lex(lx);
    lb1=LCCMiscNew(lx,CMT_LABEL);
    LCCAdd(lx,IC_JMP,lb1,0);
    LCCAdd(lx,IC_LABEL,lb,0);
    PrsStatement(lx,try_cnt,lb_break);
    LCCAdd(lx,IC_LABEL,lb1,0);
  } else
    LCCAdd(lx,IC_LABEL,lb,0);
}

U0 PrsWhile(CLex *lx,I64 try_cnt)
{
  CLexCodeMisc *lb,*lb_done;

  if (lx->token!='(')
    LexExcept(lx,"expecting '(' at ");
  Lex(lx);
  lb=LCCMiscNew(lx,CMT_LABEL);
  LCCAdd(lx,IC_LABEL,lb,0);
  if (!PrsExpression(lx,NULL,FALSE))
    throw('Compiler');
  if (lx->token!=')')
    LexExcept(lx,"missing ')' at ");
  Lex(lx);

  lb_done=LCCMiscNew(lx,CMT_LABEL);
  LCCAdd(lx,IC_BR_ZERO,lb_done,0);
  PrsStatement(lx,try_cnt,lb_done);
  LCCAdd(lx,IC_JMP,lb,0);
  LCCAdd(lx,IC_LABEL,lb_done,0);
}

U0 PrsDoWhile(CLex *lx,I64 try_cnt)
{
  CLexCodeMisc *lb,*lb_done;

  lb=LCCMiscNew(lx,CMT_LABEL);
  lb_done=LCCMiscNew(lx,CMT_LABEL);
  LCCAdd(lx,IC_LABEL,lb,0);
  PrsStatement(lx,try_cnt,lb_done);
  if (PrsKeyWord(lx)!=KW_WHILE)
    LexExcept(lx,"missing 'while' at");
  if (Lex(lx)!='(')
    LexExcept(lx,"expecting '(' at ");
  Lex(lx);
  if (!PrsExpression(lx,NULL,FALSE))
    throw('Compiler');
  if (lx->token!=')')
    LexExcept(lx,"missing ')' at ");
  LCCAdd(lx,IC_BR_NOT_ZERO,lb,0);
  LCCAdd(lx,IC_LABEL,lb_done,0);
  if (Lex(lx)!=';')
    LexExcept(lx,"missing ';' at");
  Lex(lx);
}

U0 PrsFor(CLex *lx,I64 try_cnt)
{
  CLexCodeCtrl *tempcbh;
  CLexCodeMisc *lb,*lb_done;

  if (lx->token!='(')
    LexExcept(lx,"expecting '(' at ");
  Lex(lx);
  PrsStatement(lx,try_cnt);

  lb=LCCMiscNew(lx,CMT_LABEL);
  LCCAdd(lx,IC_LABEL,lb,0);
  if (!PrsExpression(lx,NULL,FALSE))
    throw('Compiler');
  lb_done=LCCMiscNew(lx,CMT_LABEL);
  LCCAdd(lx,IC_BR_ZERO,lb_done,0);
  if (lx->token!=';')
    LexExcept(lx,"missing ';' at");
  Lex(lx);

  LCCPush(lx);
  LCCInit(lx);
  if (lx->token!=')')
    PrsStatement(lx,try_cnt,NULL,0);
  LCCPush(lx);
  tempcbh=LCCPopNoFree(lx);
  LCCPop(lx);
  if (lx->token!=')')
    LexExcept(lx,"missing ')' at ");
  Lex(lx);

  PrsStatement(lx,try_cnt,lb_done);
  LCCAppend(lx,tempcbh);
  LCCAdd(lx,IC_JMP,lb,0);
  LCCAdd(lx,IC_LABEL,lb_done,0);
}

class CSubSwitch {
  CSubSwitch *next,*last;
  CLexCodeMisc *lb_start,*lb_break;
};

class CSwitchCase {
  CSwitchCase *next;
  CLexCodeMisc *label;
  I64 val;
  CSubSwitch *ss;
};

U0 PrsSwitch(CLex *lx,Bool nobound,I64 try_cnt)
{
  CSwitchCase *header=NULL,*temps,*temps1;	//Leaks on except
  CSubSwitch root,*tempss;			//Leaks on except
  CLexCodeMisc *lb_dft,*lb_fwd_case,*mc_jt,*lb_entry,**jmp_table;
  CIntermediateCode *tempi_sub,*tempi_cmp,*tempi_jmp,*tempi_start;
  Bool dft_found=FALSE;
  I64 i,k,k2,lo=MAX_I64,hi=MIN_I64,range;

  if (lx->token!='(')
    LexExcept(lx,"expecting '(' at ");
  Lex(lx);
  QueInit(&root);

  root.last->lb_break=LCCMiscNew(lx,CMT_LABEL);
  root.last->lb_break->use_cnt++;
  lb_dft=LCCMiscNew(lx,CMT_LABEL);
  lb_dft->use_cnt++;
  mc_jt=LCCMiscNew(lx,CMT_JMP_TABLE);
  mc_jt->begin=LCCMiscNew(lx,CMT_LABEL);
  mc_jt->begin->use_cnt++;
  if (!PrsExpression(lx,NULL,FALSE))
    throw('Compiler');
  tempi_sub=LCCAdd(lx,IC_IMM_U64,0,cmp.internal_types[RT_I64]);
  LCCAdd(lx,IC_SUB,0,cmp.internal_types[RT_I64]);
  tempi_cmp=LCCAdd(lx,IC_IMM_U64,0,cmp.internal_types[RT_I64]);
  if (nobound)
    LCCAdd(lx,IC_NOBOUND_SWITCH,mc_jt,0);
  else
    LCCAdd(lx,IC_SWITCH,mc_jt,0);
  if (lx->token!=')')
    LexExcept(lx,"missing ')' at ");
  if (Lex(lx)!='{')
    LexExcept(lx,"expecting '{' at ");
  Lex(lx);
  LCCAdd(lx,IC_LABEL,mc_jt->begin,0);
  while (TRUE) {
    while (lx->token && lx->token!='}') {
sw_cont:
      switch (PrsKeyWord(lx)) {
	case KW_SUB_SWITCH_END:
	  goto sw_sub_end;
	case KW_SUB_SWITCH_START:
	  if (Lex(lx)==':')
	    Lex(lx);
	  else
	    LexExcept(lx,"expecting ':' at ");
	  tempss=MAlloc(sizeof(CSubSwitch));
	  QueIns(tempss,root.last);
	  root.last->lb_break=LCCMiscNew(lx,CMT_LABEL);
	  root.last->lb_break->use_cnt++;
	  lb_fwd_case=LCCMiscNew(lx,CMT_LABEL);
	  tempi_jmp=LCCAdd(lx,IC_JMP,lb_fwd_case,0);

	  tempss->lb_start=LCCMiscNew(lx,CMT_LABEL);
	  tempi_start=LCCAdd(lx,IC_LABEL,tempss->lb_start,0);
	  while (lx->token && lx->token!='}') {
	    switch (PrsKeyWord(lx)) {
	      case KW_SUB_SWITCH_END:
		OptFree(tempi_jmp);
		goto sw_sub_end;
	      case KW_SUB_SWITCH_START:
	      case KW_CASE:
	      case KW_DFT:
		if (lx->lcc.lcc_root.last==tempi_start) {
		  OptFree(tempi_jmp);
		  tempss->lb_start=NULL;
		} else {
		  LCCAdd(lx,IC_RET,0,0);
		  LCCAdd(lx,IC_LABEL,lb_fwd_case,0);
		  LCCAdd(lx,IC_SUB_CALL,tempss->lb_start,0);//In case fall-through
		}
		goto sw_cont;
	      default:
		PrsStatement(lx,try_cnt,root.last->last->lb_break);
	    }
	  }
	  break;
	case KW_CASE:
	  if (root.next!=&root) {
	    lb_fwd_case=LCCMiscNew(lx,CMT_LABEL);
	    tempi_jmp=LCCAdd(lx,IC_JMP,lb_fwd_case,0);//In case fall-through
	  }
	  Lex(lx);
	  lb_entry=LCCMiscNew(lx,CMT_LABEL);
	  LCCAdd(lx,IC_LABEL,lb_entry,0);
	  lb_entry->use_cnt++;
	  if (root.next!=&root) {
	    tempss=root.next;
	    while (tempss!=&root) {
	      if (tempss->lb_start)
		LCCAdd(lx,IC_SUB_CALL,tempss->lb_start,0);
	      tempss=tempss->next;
	    }
	    LCCAdd(lx,IC_LABEL,lb_fwd_case,0);
	  }
	  k=LexExpressionI64(lx);
	  if (k<lo) lo=k;
	  if (k>hi) hi=k;
	  if (lx->token==':') {
	    Lex(lx);
	    temps=MAlloc(sizeof(CSwitchCase));
	    temps->label=lb_entry;
	    temps->val=k;
	    temps->next=header;
	    header=temps;
	  } else if (lx->token==TK_DOT_DOT_DOT) {
	    Lex(lx);
	    k2=LexExpressionI64(lx);
	    if (lx->token==':') {
	      Lex(lx);
	      if (k2<lo) lo=k2;
	      if (k2>hi) hi=k2;
	      if (k>k2)
		SwapI64(&k,&k2);
	      for (i=k;i<=k2;i++) {
		temps=MAlloc(sizeof(CSwitchCase));
		temps->label=lb_entry;
		temps->val=i;
		temps->next=header;
		header=temps;
	      }
	    } else
	      LexExcept(lx,"expecting ':' at ");
	  } else
	    LexExcept(lx,"expecting ':' at ");
	  break;
	case KW_DFT:
	  if (root.next!=&root) {
	    lb_fwd_case=LCCMiscNew(lx,CMT_LABEL);
	    tempi_jmp=LCCAdd(lx,IC_JMP,lb_fwd_case,0);//In case fall-through
	  }
	  Lex(lx);
	  LCCAdd(lx,IC_LABEL,lb_dft,0);
	  if (lx->token==':')
	    Lex(lx);
	  else
	    LexExcept(lx,"expecting ':' at ");
	  if (root.next!=&root) {
	    tempss=root.next;
	    while (tempss!=&root) {
	      if (tempss->lb_start)
		LCCAdd(lx,IC_SUB_CALL,tempss->lb_start,0);
	      tempss=tempss->next;
	    }
	    LCCAdd(lx,IC_LABEL,lb_fwd_case,0);
	  }
	  dft_found=TRUE;
	  break;
	default:
	  PrsStatement(lx,try_cnt,root.last->lb_break);
      }
    }
sw_sub_end:
    tempss=root.last;
    LCCAdd(lx,IC_LABEL,tempss->lb_break,0);
    if (tempss==&root) {
      if (lx->token!='}')
	LexExcept(lx,"missing '}' at ");
      Lex(lx);
      break;
    } else {
      QueRem(tempss);
      Free(tempss);
      if (PrsKeyWord(lx)!=KW_SUB_SWITCH_END)
	LexExcept(lx,"missing 'sub_switch_end' at ");
      if (Lex(lx)==':')
	Lex(lx);
      else
	LexExcept(lx,"expecting ':' at ");
    }
  }
  if (!dft_found)
    LCCAdd(lx,IC_LABEL,lb_dft,0);

  if (0<lo<=16)
    lo=0;
  range=hi-lo+1;
  if (lo>hi || !(0<range<=0xFFFF))
    LexExcept(lx,"switch range error at ");
  jmp_table=MAlloc((sizeof(CLexCodeMisc *)*range+0x1FF)&~0x1FF);
  MemSetI64(jmp_table,lb_dft,range);
  tempi_sub->ic_data=lo;
  tempi_cmp->ic_data=range;
  temps=header;
  while (temps) {
    temps1=temps->next;
    if (jmp_table[temps->val-lo]!=lb_dft)
      LexExcept(lx,"Duplicate case at ");
    else
      jmp_table[temps->val-lo]=temps->label;
    Free(temps);
    temps=temps1;
  }
  mc_jt->dft=lb_dft;
  mc_jt->jmp_table=jmp_table;
  mc_jt->range=range;
}

U0 PrsNoUnusedWarn(CLex *lx)
{
  CMemberLst *tempm;
  while (lx->token==TK_IDENT) {
    if (!(tempm=lx->local_var_entry))
      LexExcept(lx,"expecting local var at ");
    tempm->flags|=MLF_NO_UNUSED_WARN;
    if (Lex(lx)==',')
      Lex(lx);
    else if (lx->token!=';')
      LexExcept(lx,"expecting ',' at ");
  }
}

U0 PrsStreamBlk(CLex *lx)
{
  CLexHashTableContext *htc=MAlloc(sizeof(CLexHashTableContext));
  CStreamBlk *tempe=MAlloc(sizeof(CStreamBlk));
  tempe->body=StrNew("");
  QueIns(tempe,lx->last_stream_blk);
  LCCPush(lx);
  QueInit(&lx->lcc.lcc_next_misc);

  MemCpy(htc,&lx->htc,sizeof(CLexHashTableContext));
  htc->old_flags=lx->flags;
  lx->htc.next=htc;
  lx->htc.fun=lx->htc.local_var_lst=NULL;
  lx->htc.define_hash_table=lx->htc.hash_table_lst=lx->htc.glbl_hash_table=lx->htc.local_hash_table=Fs->hash_table;
  lx->flags=lx->flags & ~(LF_ASM_EXPRESSIONS|LF_AOT_COMPILE) | LF_EXE_BLK;
  if (lx->token=='{')
    Lex(lx);
  else
    LexExcept(lx,"missing '}' at ");
  while (lx->token && lx->token!='}')
    ExeCmdLine(lx);

  MemCpy(&lx->htc,htc,sizeof(CLexHashTableContext));
  lx->flags=lx->flags&~LF_EXE_BLK |
    htc->old_flags & (LF_ASM_EXPRESSIONS|LF_EXE_BLK|LF_AOT_COMPILE);
  Free(htc);
  LCCPop(lx);
  QueRem(tempe);
  if (*tempe->body)
    LexIncludeStr(lx,"StreamBlk",tempe->body,FALSE);
  else
    Free(tempe->body);
  Free(tempe);
  Lex(lx); //Skip '}'
}

U0 PrsTryBlk(CLex *lx,I64 try_cnt)
{
  CLexCodeMisc	*lb_catch,*lb_done,*lb_untry;
  CHashClass	*tempc=cmp.internal_types[RT_PTR];
  CHashFun	*temp_try=HashFind("SysTry",lx->htc.hash_table_lst,HTT_FUN),
		*temp_untry=HashFind("SysUntry",lx->htc.hash_table_lst,HTT_FUN);

  if (!temp_try || !temp_untry)
    LexExcept(lx,"Missing header for SysTry() and SysUntry() at ");

  lx->flags|=LF_NO_REG_OPT; //TODO (Currently no reg vars in funs with try/catch)

  lb_catch=LCCMiscNew(lx,CMT_LABEL);
  lb_done =LCCMiscNew(lx,CMT_LABEL);
  lb_untry=LCCMiscNew(lx,CMT_LABEL);

  LCCAdd(lx,IC_CALL_START,0,0);
  LCCAdd(lx,IC_GET_LABEL,lb_untry,tempc,ICF_PUSH_RESULT);
  LCCAdd(lx,IC_GET_LABEL,lb_catch,tempc,ICF_PUSH_RESULT);
  if (Bt(&temp_try->flags,Cf_EXTERN)) {
    lx->abs_cnts.externs++;
    if (lx->flags&LF_AOT_COMPILE)
      LCCAdd(lx,IC_CALL_IMPORT,temp_try,tempc);
    else
      LCCAdd(lx,IC_CALL_INDIRECT2,&temp_try->executable_addr,tempc);
  } else
    LCCAdd(lx,IC_CALL,temp_try->executable_addr,tempc);
  if ((Bt(&temp_try->flags,Ff_RET1) || Bt(&temp_try->flags,Ff_ARGPOP)) && !Bt(&temp_try->flags,Ff_NOARGPOP))
    LCCAdd(lx,IC_ADD_RSP1,16,tempc);
  else
    LCCAdd(lx,IC_ADD_RSP,16,tempc);
  LCCAdd(lx,IC_CALL_END,0,tempc);
  LCCAdd(lx,IC_END_EXP,0,0);

  PrsStatement(lx,try_cnt+1);

  LCCAdd(lx,IC_LABEL,lb_untry,0);
  LCCAdd(lx,IC_CALL_START,0,0);
  if (Bt(&temp_untry->flags,Cf_EXTERN)) {
    lx->abs_cnts.externs++;
    if (lx->flags&LF_AOT_COMPILE)
      LCCAdd(lx,IC_CALL_IMPORT,temp_untry,tempc);
    else
      LCCAdd(lx,IC_CALL_INDIRECT2,&temp_untry->executable_addr,tempc);
  } else
    LCCAdd(lx,IC_CALL,temp_untry->executable_addr,tempc);
  LCCAdd(lx,IC_CALL_END,0,tempc);
  LCCAdd(lx,IC_END_EXP,0,0);

  LCCAdd(lx,IC_JMP,lb_done,0);

  if (PrsKeyWord(lx)!=KW_CATCH)
    LexExcept(lx,"missing 'catch' at");

  Lex(lx);
  LCCAdd(lx,IC_LABEL,lb_catch,0);
  PrsStatement(lx,try_cnt+1);
  LCCAdd(lx,IC_RET,0,tempc);
  LCCAdd(lx,IC_LABEL,lb_done,0);
}

Bool PrsStatement(CLex *lx,I64 try_cnt=0,
  CLexCodeMisc *lb_break=NULL,I64 cmp_flags=CMPF_PRS_SEMICOLON)
{
  I64 i,fsp_flags=0;
  CHashExport *tempex;
  CLexCodeMisc *g_lb;
  U8 *import_name;
  CHashFun *temp_untry;
  CExe *tempx;
  if (cmp_flags&CMPF_ONE_ASM_INS) {
    if (lx->flags&LF_AOT_COMPILE || lx->a_depth)
      PrsAsmBlk(lx,CMPF_ONE_ASM_INS);
    else if (tempx=CmpJoin(lx,CMPF_ASM_BLK|CMPF_ONE_ASM_INS))
      CmpFixUpJITAsm(lx,tempx);
    fsp_flags=FSF_ASM;
  } else
    while (TRUE) {
      while (lx->token==',')
	Lex(lx);
      if (lx->token=='{') {
	Lex(lx);
	while (lx->token!='}' && lx->token!=TK_EOF)
	  PrsStatement(lx,try_cnt,lb_break);
	if (Lex(lx)!=',') goto sm_done;
      } else if (lx->token==';') {
	if (cmp_flags&CMPF_PRS_SEMICOLON)
	  Lex(lx);
	if (lx->token!=',') goto sm_done;
      } else {
	if (lx->token==TK_IDENT) {
	  if (tempex=lx->hash_entry) {
	    if (tempex->type & HTT_KEYWORD) {
	      i=tempex(CHashGeneric *)->user_data0;
	      nobound_switch (i) {
		case KW_NUM_KEYWORDS-1: //nobound_switch
		default: //A keyword that is not valid here is just a symbol.
		  goto sm_not_keyword_afterall;
		sub_switch_start:
		  case KW_ASM:
		    if (lx->htc.fun) {
		      if (tempx=CmpJoin(lx,CMPF_ASM_BLK))
			LCCAdd(lx,IC_ASM,tempx,0);
		      Lex(lx); //Skip '}' of asm{}
		    } else {
		      if (lx->flags&LF_AOT_COMPILE || lx->a_depth) {
			Lex(lx);
			PrsAsmBlk(lx,0);
			if (lx->flags&LF_AOT_COMPILE && lx->a_depth==1)
			  Lex(lx); //Skip '}' of asm{}
		      } else {
			if (tempx=CmpJoin(lx,CMPF_ASM_BLK))
			  CmpFixUpJITAsm(lx,tempx);
			Lex(lx); //Skip '}' of asm{}
		      }
		      fsp_flags=FSF_ASM;
		    }
		    break;
		  sub_switch_start:
		    Lex(lx);
		    case KW_LOCK:
		      lx->lock_cnt++;
		      PrsStatement(lx,try_cnt);
		      lx->lock_cnt--;
		      break;
		    case KW_TRY:
		      PrsTryBlk(lx,try_cnt);
		      break;
		    case KW_IF:
		      PrsIf(lx,try_cnt,lb_break);
		      break;
		    case KW_FOR:
		      PrsFor(lx,try_cnt);
		      break;
		    case KW_WHILE:
		      PrsWhile(lx,try_cnt);
		      break;
		    case KW_DO:
		      PrsDoWhile(lx,try_cnt);
		      break;
		    case KW_NOBOUND_SWITCH:
		      PrsSwitch(lx,TRUE,try_cnt);
		      break;
		    case KW_SWITCH:
		      PrsSwitch(lx,FALSE,try_cnt);
		      break;
		  sub_switch_end:
		sub_switch_end:
		  if (lx->token!=',') goto sm_done;
		  break;
		sub_switch_start:
		  if (lx->htc.fun)
		    LexExcept(lx,"Not allowed in fun");
		  Lex(lx);
		  case KW__EXTERN:
		    if (Bt(&lx->opts,OPTf_EXTERNS_TO_IMPORTS))
		      goto sm_underscore_import;
		    if (lx->flags&LF_AOT_COMPILE) {
		      if (lx->token!=TK_IDENT || !(tempex=lx->hash_entry) ||
			    !(tempex->type & HTT_EXPORT_SYS_SYM))
			LexExcept(lx,"expecting System sym at ");
		      i=tempex->val;
		      Lex(lx);
		    } else
		      i=LexExpressionI64(lx);
		    if (lx->token!=TK_IDENT || !(tempex=lx->hash_entry) ||
			  !(tempex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(lx,"expecting type at ");
		    Lex(lx);
		    PrsGlblVarLst(lx,PRS0__EXTERN|PRS1_NULL,tempex,i,fsp_flags);
		    break;
		  case KW__IMPORT:
sm_underscore_import:
		    if (lx->token!=TK_IDENT)
		      LexExcept(lx,"expecting System sym at ");
		    import_name=lx->cur_str;
		    lx->cur_str=0;
		    if (Lex(lx)!=TK_IDENT || !(tempex=lx->hash_entry) ||
			  !(tempex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(lx,"expecting type at ");
		    Lex(lx);
		    PrsGlblVarLst(lx,PRS0__IMPORT|PRS1_NULL,tempex,import_name,FALSE);
		    Free(import_name);
		    break;
		  case KW_EXTERN:
		    if (lx->token!=TK_IDENT)
		      LexExcept(lx,"expecting type at ");
		    tempex=lx->hash_entry;
		    i=PrsKeyWord(lx);
		    if (i==KW_CLASS||i==KW_UNION) {
		      Lex(lx);
		      PrsClass(lx,i,fsp_flags,TRUE);
		      fsp_flags&=FSF_ASM;
		      goto sm_semicolon;
		    }
		    if (!tempex || !(tempex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(lx,"expecting type at ");
		    if (Bt(&lx->opts,OPTf_EXTERNS_TO_IMPORTS))
		      goto sm_import;
		    Lex(lx);
		    PrsGlblVarLst(lx,PRS0_EXTERN|PRS1_NULL,tempex,0,fsp_flags);
		    break;
		  case KW_IMPORT:
		    if (lx->token!=TK_IDENT || !(tempex=lx->hash_entry) ||
			  !(tempex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(lx,"expecting type at ");
sm_import:
		    Lex(lx);
		    PrsGlblVarLst(lx,PRS0_IMPORT|PRS1_NULL,tempex,0,FALSE);
		    break;
		  case KW__INTERN:
		    i=LexExpressionI64(lx);
		    if (lx->token!=TK_IDENT || !(tempex=lx->hash_entry) ||
			  !(tempex->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		      LexExcept(lx,"expecting type at ");
		    Lex(lx);
		    PrsGlblVarLst(lx,PRS0__INTERN|PRS1_NULL,tempex,i,fsp_flags);
		    break;
		sub_switch_end:
		  fsp_flags&=FSF_ASM;
		  break;
		sub_switch_start:
		  case KW_STATIC:
		    fsp_flags=FSF_STATIC|fsp_flags&FSF_ASM;
		    break;
		  case KW_INTERRUPT:
		    fsp_flags=FSF_INTERRUPT|FSF_NOARGPOP|fsp_flags&(FSG_FUN_FLAGS2|FSF_ASM);
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
		  Lex(lx);
		  break;
		case KW_RETURN:
		  if (!lx->htc.fun)
		    LexExcept(lx,"Not in fun.  Can't return a val ");
		  if (try_cnt) {
		    temp_untry=HashFind("SysUntry",
			  lx->htc.hash_table_lst,HTT_FUN);
		    for (i=0;i<try_cnt;i++) {
		      if (Bt(&temp_untry->flags,Cf_EXTERN)) {
			lx->abs_cnts.externs++;
			if (lx->flags&LF_AOT_COMPILE)
			  LCCAdd(lx,IC_CALL_IMPORT,
				temp_untry,cmp.internal_types[RT_PTR]);
			else
			  LCCAdd(lx,IC_CALL_INDIRECT2,
				&temp_untry->executable_addr,cmp.internal_types[RT_PTR]);
		      } else
			LCCAdd(lx,IC_CALL,temp_untry->executable_addr,cmp.internal_types[RT_PTR]);
		    }
		  }
		  if (Lex(lx)!=';') {
		    if (!lx->htc.fun->return_class->size)
		      LexWarn(lx,"Function should NOT return val ");
		    if (!PrsExpression(lx,NULL,FALSE))
		      throw('Compiler');
		    LCCAdd(lx,IC_RETURN_VAL,0,lx->htc.fun->return_class);
		    lx->flags|=LF_HAS_RETURN;
		  } else if (lx->htc.fun->return_class->size)
		    LexWarn(lx,"Function should return val ");
		  LCCAdd(lx,IC_JMP,lx->lb_leave,0);
		  goto sm_semicolon;
		case KW_GOTO:
		  if (Lex(lx)!=TK_IDENT)
		    LexExcept(lx,"expecting identifier at ");
		  if (!(g_lb=LCCGoToLabelFind(lx,lx->cur_str))) {
		    g_lb=LCCMiscNew(lx,CMT_GOTO_LABEL);
		    g_lb->str=lx->cur_str;
		    lx->cur_str=NULL;
		  }
		  g_lb->use_cnt++;
		  LCCAdd(lx,IC_JMP,g_lb,0);
		  Lex(lx);
		  goto sm_semicolon;
		case KW_BREAK:
		  Lex(lx);
		  if (!lb_break)
		    LexExcept(lx,"'break' not allowed\n");
		  LCCAdd(lx,IC_JMP,lb_break,0);
		  goto sm_semicolon;
		case KW_NOUNUSEDWARN:
		  Lex(lx);
		  PrsNoUnusedWarn(lx);
		  goto sm_semicolon;
		case KW_UNION:
		case KW_CLASS:
		  Lex(lx);
		  tempex=PrsClass(lx,i,fsp_flags,FALSE);
		  if (!lx->htc.fun && lx->token!=';') {
		    PrsGlblVarLst(lx,PRS0_NULL|PRS1_NULL,tempex,0,fsp_flags);
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
		if (lx->htc.fun) {
		  if (fsp_flags&FSF_STATIC)
		    PrsVarLst(lx,lx->htc.fun,PRS0_NULL|PRS1_STATIC_LOCAL_VAR);
		  else
		    PrsVarLst(lx,lx->htc.fun,PRS0_NULL|PRS1_LOCAL_VAR);
		  if (lx->token=='}') goto sm_done;
		} else {
		  Lex(lx);
		  PrsGlblVarLst(lx,PRS0_NULL|PRS1_NULL,tempex,0,fsp_flags);
		}
	      } else {
		if (tempex->type & HTT_OPCODE) {
		  if (lx->htc.fun) {
		    if (tempx=CmpJoin(lx,CMPF_ASM_BLK|CMPF_ONE_ASM_INS))
		      LCCAdd(lx,IC_ASM,tempx,0);
		  } else
		    LexExcept(lx,"Use Asm Blk at ");
		  if (lx->token!=',') goto sm_done;
		} else
		  goto sm_prs_exp;
	      }
	      fsp_flags&=FSF_ASM;
	    }
	  } else {//Ident, not in hash table
	    if (lx->local_var_entry)
	      goto sm_prs_exp;
	    if (!(g_lb=LCCGoToLabelFind(lx,lx->cur_str))) {
	      g_lb=LCCMiscNew(lx,CMT_GOTO_LABEL);
	      g_lb->str=lx->cur_str;
	      lx->cur_str=NULL;
	    } else if (g_lb->flags&CMF_DEFINED)
	      LexExcept(lx,"Duplicate goto label at ");
	    g_lb->flags|=CMF_DEFINED;
	    LCCAdd(lx,IC_LABEL,g_lb,0);
	    if (Lex(lx)==':') //skip cur_str
	      Lex(lx); //skip colon
	    else
	      LexExcept(lx,"undefined identifier at ");
	    if (!lx->htc.fun)
	      LexExcept(lx,"No global labels at ");
	    if (lx->token!=',') goto sm_done;
	  }
	} else if (lx->token==TK_STR||lx->token==TK_CHAR_CONST) {
	  PrsFunCall(lx,NULL,FALSE,NULL);
	  goto sm_semicolon;
	} else if (lx->token!=TK_EOF) {//Non-cur_str symbol, number or something
sm_prs_exp:
	  if (!PrsExpression(lx,NULL,TRUE))
	    throw('Compiler');
sm_semicolon:
	  if (cmp_flags&CMPF_PRS_SEMICOLON) {
	    if (lx->token==';')
	      Lex(lx);
	    else if (lx->token!=',')
	      LexExcept(lx,"missing ';' at");
	  }
	  if (lx->token!=',') goto sm_done;
	} else
	  goto sm_done; //TK_EOF
      }
    }
sm_done:
  return fsp_flags&FSF_ASM;
}
