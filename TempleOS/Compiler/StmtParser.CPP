CHashClass *PrsClassNew()
{
/*
Ptrs to classes are handled by
allocating 5 structures for each
new class and representing a pointer
to a class by advancing 1 struct fwd
for one * and two fwd for two **.
*/

  CHashClass *result=CAlloc(sizeof(CHashClass)*5,Fs->code_heap),
      *tempc=result;
  tempc->last_in_member_lst=&tempc->member_lst;
  tempc->type=HTT_CLASS;
  tempc->sub_type=IT_PTR;
  tempc->ptr_cnt=0;

  tempc++;
  tempc->type=HTT_CLASS;
  tempc->sub_type=IT_PTR;
  tempc->size=sizeof(U8 *);
  tempc->ptr_cnt=1;

  tempc++;
  tempc->type=HTT_CLASS;
  tempc->sub_type=IT_PTR;
  tempc->size=sizeof(U8 *);
  tempc->ptr_cnt=2;

  tempc++;
  tempc->type=HTT_CLASS;
  tempc->sub_type=IT_PTR;
  tempc->size=sizeof(U8 *);
  tempc->ptr_cnt=3;

  tempc++;
  tempc->type=HTT_CLASS;
  tempc->sub_type=IT_PTR;
  tempc->size=sizeof(U8 *);
  tempc->ptr_cnt=4;

  return result;
}

CHashFun *PrsFunNew()
{
  CHashFun *result=CAlloc(sizeof(CHashFun)*5,Fs->code_heap),
	    *tempf=result;
  tempf->last_in_member_lst=&tempf->member_lst;
  tempf->type=HTT_FUN;
  tempf->sub_type=IT_PTR;
  tempf->ptr_cnt=0;

  tempf++;
  tempf->type=HTT_FUN;
  tempf->sub_type=IT_PTR;
  tempf->size=sizeof(U8 *);
  tempf->ptr_cnt=1;

  tempf++;
  tempf->type=HTT_FUN;
  tempf->sub_type=IT_PTR;
  tempf->size=sizeof(U8 *);
  tempf->ptr_cnt=2;

  tempf++;
  tempf->type=HTT_FUN;
  tempf->sub_type=IT_PTR;
  tempf->size=sizeof(U8 *);
  tempf->ptr_cnt=3;

  tempf++;
  tempf->type=HTT_FUN;
  tempf->sub_type=IT_PTR;
  tempf->size=sizeof(U8 *);
  tempf->ptr_cnt=4;

  return result;
}

CMemberLst *MemberLstNew(I64 register)
{
  CMemberLst *result=CAlloc(sizeof(CMemberLst));
  result->register=register;
  return result;
}

I64 PrsKeyWord(CLex *lx)
{
  CHashGeneric *temph;
  if (lx->token==TK_IDENT &&
      (temph=lx->hash_entry) &&
      temph->type&HTT_KEYWORD)
    return temph->user_data0;
  else
    return -1;
}

U0 PrsVarInit(CLex *lx,U8 **_dst,CHashClass *tempc,CArrayDim *tempad,U8 *data_addr_ip,U8 **_base,Bool data_heap,I64 pass)
{
  U8 *dst=*_dst,*ex;
  I64 i,j,r,old_flags,type,size;
  CMemberLst *tempm;
  CIntermediateCode *tempi;
  CAsmCtrl *a=lx->a;
  CExeAbsAddr *tempa;
  CExeImportExport *tempe;
  Bool is_str;

  tempc=OptClassFwd(tempc);
  if (tempm=tempc->member_lst) {
    if (lx->token!='{')
      LexExcept(lx,"ST_EXPECTING_LEFT_BRACE");
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
      LexExcept(lx,"ST_MISSING_RIGHT_BRACE");
    Lex(lx);
  } else {
    if (tempc->ptr_cnt==1 && ((tempc-1)->sub_type==IT_I8 || (tempc-1)->sub_type==IT_U8) &&
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
	tempa->type=EAA_ADD_U64;
	a->abss=tempa;
	tempa->ip=data_addr_ip+dst-*_base;
	*dst(I64 *)=a->ip;
	for (j=0;j<i;j++)
	  AsmStoreCodeU8(lx,ex[j]);
      }
      Free(ex);
    } else {
      old_flags=lx->flags;
      lx->flags=LF_NO_ABSS | lx->flags & ~(LF_AOT_COMPILE|LF_HAS_MISC_DATA|LF_NOT_CONSTANT);
      ex=CmpExpression(lx,&type);
      if (old_flags&LF_AOT_COMPILE &&
	lx->flags&LF_NOT_CONSTANT &&
	 !Bt(&lx->opts,OPTf_GLBLS_ON_DATA_HEAP)) {
	lx->flags=lx->flags&~LF_NO_ABSS|LF_AOT_COMPILE;
	ExpressionDel(ex);
	if (pass==2) {
	  MemSet(dst,0,tempc->size);
	  LexPop(lx);
	  Lex(lx);
	  CBPush(lx);
	  CBOutPtrsInit(lx);
	  CBICAdd(lx,IC_START,0,0); //So we can scan back an instruction
	  CBICAdd(lx,IC_ABS_ADDR,data_addr_ip,tempc+1);
	  CBICAdd(lx,IC_IMM_U64,dst-*_base,tempc+1);
	  CBICAdd(lx,IC_ADD,0,tempc+1);
	  if (!PrsExpression(lx,NULL,TRUE))
	    throw(EXCEPT_COMPILER,10);
	  tempi=lx->cbh.cb_last_out_ptr;
	  if (tempi->ic_opcode==IC_END_EXP)
	    tempi->ic_opcode=IC_NOP1;
	  CBICAdd(lx,IC_ASSIGN,0,tempc);
	  CBICAdd(lx,IC_END_EXP,0,tempc);
	  CBICAdd(lx,IC_RET,0,0);
	  CBICAdd(lx,IC_END,0,0);
	  if (ex=CBCompile(lx,NULL,&size,NULL)) {
	    tempe=CAlloc(sizeof(CExeImportExport));
	    tempe->type=EIE_MAIN;
	    tempe->ip=lx->a->ip;
	    QueIns(tempe,lx->exe_struct->last_ie);
	    for (i=0;i<size;i++)
	      AsmStoreCodeU8(lx,ex[i]);
	    ExpressionDel(ex);
	  }
	  CBPop(lx);
	} else
	  lx->stk_ptr--;  //grows up
      } else {
	lx->stk_ptr--;	//grows up
	if (!ex)
	  throw(EXCEPT_COMPILER,10);
	r=Call(ex);
	if (!(lx->flags & LF_HAS_MISC_DATA)||pass==1)
	  ExpressionDel(ex);

	if (type==IT_F64 &&
	    tempc->sub_type!=IT_F64)
	  r=r(F64);
	else if (type!=IT_F64 &&
	    tempc->sub_type==IT_F64)
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
	(tempc->sub_type==IT_I8 || tempc->sub_type==IT_U8) &&
	lx->token==TK_STR) {
      lx->stk_ptr--;	//grows up
      st=LexExtStr(lx,&i);
      if (tempad1->cnt<0) { //[]
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
      if (tempad1->cnt<0) { //[]
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
  CIntermediateCode *tempi;
  CExeImportExport *tempe;

  if (lx->flags&LF_AOT_COMPILE && pass==2) {
    CBPush(lx);
    CBOutPtrsInit(lx);
    CBICAdd(lx,IC_START,0,0); //So we can scan back an instruction
    tempi=lx->cbh.cb_last_out_ptr;
  }
  PrsVarInit2(lx,&dst,tempc,&tempm->dim,tempm->static_data_ip,&tempm->static_data,Bt(&lx->flags,Lf_AOT_COMPILE),pass);
  if (lx->flags&LF_AOT_COMPILE && pass==2) {
    if (tempi!=lx->cbh.cb_last_out_ptr) {
      CBICAdd(lx,IC_RET,0,0);
      CBICAdd(lx,IC_END,0,0);
      if (ex=CBCompile(lx,NULL,&size,NULL)) {
	if (pass==2) {
	  tempe=CAlloc(sizeof(CExeImportExport));
	  tempe->type=EIE_MAIN;
	  tempe->ip=lx->a->ip;
	  QueIns(tempe,lx->exe_struct->last_ie);
	  for (i=0;i<size;i++)
	    AsmStoreCodeU8(lx,ex[i]);
	}
	ExpressionDel(ex);
      }
    } else
      LinkedLstDel(lx->cbh.cb_out_first);
    CBPop(lx);
  }
}


Bool PrsArrayDims(CLex *lx,CArrayDim *dim)
{ //returns false on empty []
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
      if ((j=Expression(lx))<0)
	LexExcept(lx,"ST_INVALID_ARRAY_SIZE");
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
      LexExcept(lx,"ST_MISSING_RIGHT_BRACKET");
    Lex(lx);
  }
  return result;
}


U0 PrsDotDotDot(CLex *lx,CHashFun *tempf,I64 register)
{
  CMemberLst *tempm;
  CArrayDim *tempad;

  Bts(&tempf->flags,Ff_DOT_DOT_DOT);

  Lex(lx);
  tempm=MemberLstNew(register);
  tempm->flags=MLF_DOT_DOT_DOT;
  tempm->member_class=sys_internal_types[IT_I64];
  tempm->str=StrNew("argc");
  tempm->offset=tempf->size;
  tempf->size+=8;
  MemberAdd(lx,tempm,tempf,PRS1B_FUN_ARG);

  tempm=MemberLstNew(register);
  tempm->flags=MLF_DOT_DOT_DOT;
  tempm->member_class=sys_internal_types[IT_I64];
  tempm->str=StrNew("argv");
  tempm->dim.total_cnt=127; //arbitrary
  tempm->dim.next=tempad=MAlloc(sizeof(CArrayDim));
  tempad->next=NULL;
  tempad->cnt=127; //arbitrary
  tempad->total_cnt=1;
  tempm->offset=tempf->size;
  tempf->size+=8;
  MemberAdd(lx,tempm,tempf,PRS1B_FUN_ARG);

  if (lx->token==')')
    Lex(lx);
}

CHashClass *PrsType(CLex *lx,CHashClass **_tempc1,
  I64 *_mode,CMemberLst *tempm,U8 **_ident,CHashFun **_fun_ptr,
  CHashExport **_tempess,CArrayDim *tempad,I64 cps_flags)
{
  I64 fun_ptr_cnt,mode=*_mode;
  CHashClass *tempc1=*_tempc1,*tempc2;
  CHashFun *fun_ptr=NULL;
  CHashExport *tempess=NULL;
  CArrayDim *tempad2;

start:
  if (!tempc1 || !(tempc1->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
    LexExcept(lx,"ST_INVALID_CLASS");

  while (lx->token=='*') {
    if (mode.u8[1]) {
      lx->stk_ptr--;	//grows up
      LexPush(lx);
    }
    Lex(lx);
    tempc1++;
  }

  if (PrsKeyWord(lx)==KW_UNION) {
    Lex(lx);
    tempc2=PrsClass(lx,TRUE,cps_flags,mode&255==PRS0_EXTERN);
    tempc2->fwd_class=tempc1;
    tempc1=tempc2;
    if (_tempc1) *_tempc1=tempc1;
    mode=PRS0_NULL|PRS1_NULL;
    goto start;
  } else if (PrsKeyWord(lx)==KW_CLASS) {
    Lex(lx);
    tempc2=PrsClass(lx,FALSE,cps_flags,mode&255==PRS0_EXTERN);
    tempc2->fwd_class=tempc1;
    tempc1=tempc2;
    if (_tempc1) *_tempc1=tempc1;
    mode=PRS0_NULL|PRS1_NULL;
    goto start;
  }

  if (lx->token=='(') {
    if (Lex(lx)!='*')
      LexExcept(lx,"ST_EXPECTING_STAR");
    fun_ptr_cnt=1;
    while (Lex(lx)=='*')
      fun_ptr_cnt++;
  } else
    fun_ptr_cnt=-1;

  if (_ident) {
    if (lx->token==TK_IDENT) {
      tempess=lx->hash_entry;
      *_ident=lx->ident;
      lx->ident=NULL;
      Lex(lx);
    } else {
      if (!mode.u8[1])
	*_ident=NULL;
      else if (lx->token==',' || lx->token==';' || lx->token==')') {
	tempess=NULL;
	*_ident=StrNew("_anon_");
	tempm->flags|=MLF_NO_UNUSED_WARN;
      } else
	LexExcept(lx,"ST_EXPECTING_IDENTIFIER");
    }
  }

  if (fun_ptr_cnt>=0) {
    if (lx->token!=')')
      LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
    if (Lex(lx)!='(')
      LexExcept(lx,"ST_EXPECTING_LEFT_PAREN");
    fun_ptr=PrsFunJoin(lx,tempc1,NULL,cps_flags)+fun_ptr_cnt;
    tempc1=sys_internal_types[IT_PTR]+fun_ptr_cnt;
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
      LexPutWarn(lx,"ST_PTR_TO_U0");
  }

  if (_mode)	    *_mode=mode;
  if (_fun_ptr)     *_fun_ptr=fun_ptr;
  if (_tempess)     *_tempess=tempess;
  return tempc1;
}

U0 PrsVarLst(CLex *lx,CHashClass *tempc,
		     I64 mode,I64 union_base=0)
{
  I64 i,k,old_flags=lx->flags,old_flags2,type,register;
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
    while (lx->token==';') Lex(lx);
    if (mode&PRSF_UNION)
      lx->class_ip=union_base;
    else
      lx->class_ip=tempc->size;
    while (lx->token=='$$') {
      if (Lex(lx)!='=') //skip $$
	LexExcept(lx,"ST_EXPECTING_EQUAL");
      Lex(lx); //skip =
      lx->class_ip=Expression(lx);
      if (mode&PRSF_UNION)
	union_base=lx->class_ip;
      else
	tempc->size=lx->class_ip;
      if (lx->token!=';')
	LexExcept(lx,"ST_MISSING_SEMICOLON");
      Lex(lx); //skip ;
    }
    if (lx->token==')' || lx->token=='}') {
      Lex(lx);
      goto pvl_done;
    }
    register=CREG_UNDEF;
    k=PrsKeyWord(lx);
    if (k==KW_REG) {
      register=CREG_ALLOC;
      if (Lex(lx)==TK_IDENT) {
	k=DefineMatch(lx->ident,"ST_U64_REGS");
	if (k>=0) {
	  register=k;
	  Lex(lx);
	}
      }
    } else if (k==KW_NOREG) {
      register=CREG_NONE;
      Lex(lx);
    }

    if (lx->token==TK_DOT_DOT_DOT && mode.u8[1]==PRS1B_FUN_ARG) {
      PrsDotDotDot(lx,tempc,register);
      goto pvl_done;
    }
    temph=NULL;
    if (lx->token==TK_IDENT) {
      temph=lx->hash_entry;
      k=PrsKeyWord(lx);
    }
    if (!temph)
      LexExcept(lx,"ST_EXPECTING_TYPE");
    if (k==KW_UNION) {
      Lex(lx);
      PrsVarLst(lx,tempc,mode|PRSF_UNION,tempc->size);
    } else {
      if (!(temph->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
	LexExcept(lx,"ST_EXPECTING_TYPE");
      first=TRUE;
restart:
      tempc1=temph;
      LexPush(lx);
      if (lx->token!=',' && mode.u8[1]==PRS1B_LOCAL_VAR)
	Lex(lx); //skip type
      else
	Lex(lx); //skip type or ','
      tempm=MemberLstNew(register);
      if (mode.u8[1]==PRS1B_STATIC_LOCAL_VAR) {
	tempm->flags=MLF_STATIC;
	tempm->register=CREG_NONE;
      }
      if (mode.u8[1]==PRS1B_FUN_ARG || mode.u8[1]==PRS1B_LOCAL_VAR) {
	k=PrsKeyWord(lx);
	if (k==KW_REG) {
	  tempm->register=CREG_ALLOC;
	  lx->stk_ptr--;  //grows up
	  LexPush(lx);
	  if (Lex(lx)==TK_IDENT) {
	    k=DefineMatch(lx->ident,"ST_U64_REGS");
	    if (k>=0) {
	      tempm->register=k;
	      lx->stk_ptr--;  //grows up
	      LexPush(lx);
	      Lex(lx);
	    }
	  }
	} else if (k==KW_NOREG) {
	  tempm->register=CREG_NONE;
	  lx->stk_ptr--;  //grows up
	  LexPush(lx);
	  Lex(lx);
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
	    LexExcept(lx,"ST_STATIC_UNION");
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
		AsmStoreCodeU8At(lx,tempm->static_data[k],tempm->static_data_ip+k);
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
	  if (lx->token=='=') {
	    lx->flags=lx->flags&~LF_DONT_MAKE_RESULT|old_flags&LF_DONT_MAKE_RESULT;
	    LexPop(lx);
	    Lex(lx);
	    if (!PrsExpression(lx,NULL,TRUE))
	      throw(EXCEPT_COMPILER,7);
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
	  if (lx->token=='=') {
	    Lex(lx);
	    old_flags2=lx->flags;
	    lx->flags&=~LF_HAS_MISC_DATA;
	    ex=CmpExpression(lx,&type);
	    if (!ex)
	      throw(EXCEPT_COMPILER,8);
	    tempm->dft_val=Call(ex);
	    tempc2=OptClassFwd(tempc2);
	    if (tempc2->sub_type==IT_F64) {
	      if (type!=IT_F64)
		tempm->dft_val(F64)=tempm->dft_val;
	    } else {
	      if (type==IT_F64)
		tempm->dft_val=tempm->dft_val(F64);
	    }
	    if (lx->flags & LF_HAS_MISC_DATA) {
	      tempm->dft_val=StrNew(tempm->dft_val);
	      tempm->flags|=MLF_STR_DFT_AVAILABLE;
	    }
	    ExpressionDel(ex);
	    tempm->flags|=MLF_DFT_AVAILABLE;
	    lx->flags|=old_flags2&LF_HAS_MISC_DATA;
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
	  while (lx->token==TK_IDENT) {
	    temp_meta=MAlloc(sizeof(CMemberLstMeta));
	    temp_meta->next=tempm->meta;
	    tempm->meta=temp_meta;
	    temp_meta->str=lx->ident;
	    temp_meta->flags=0;
	    lx->ident=NULL;
	    if (Lex(lx)==TK_STR) {
	      temp_meta->user_data=LexExtStr(lx);
	      temp_meta->flags|=MLMF_IS_STR;
	    } else
	      temp_meta->user_data=Expression(lx);
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
	    goto restart;
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
	  LexExcept(lx,"ST_MISSING_SEMICOLON");
      }
    }
  }
pvl_done:
  lx->flags=lx->flags&~(LF_CLASS_IP|LF_DONT_MAKE_RESULT)|
	    old_flags& (LF_CLASS_IP|LF_DONT_MAKE_RESULT);
}

U0 UndefinedExtern()
{
  PutDefineErr("ST_UNDEFINED_EXTERN");
  "at %P\n",Caller;
  throw(EXCEPT_UNDEF_EXTERN);
}

U0 UnusedExternWarning(CLex *lx,CHashClass *tempc)
{
  PrintFWarn("%S '%s'\n","ST_UNUSED_EXTERN",tempc->str);
  lx->warning_cnt++;
}

CHashClass *PrsClass(CLex *lx,Bool is_union,I64 cps_flags,Bool is_extern)
{
  CHashClass *tempc,*base_class;
  if (lx->token!=TK_IDENT)
    LexExcept(lx,"ST_EXPECTING_IDENTIFIER");
  if (is_extern) {
    tempc=PrsClassNew;
    tempc->str=lx->ident;
    lx->ident=NULL;
    if (IsReplaceSyms)
      HashReplace(tempc,lx->htc.glbl_hash_table);
    else
      HashAdd(tempc,lx->htc.glbl_hash_table);
    LBts(tempc->flags,Cf_EXTERN);
    SetSrcFileInfo(lx,tempc);
    Lex(lx);
  } else {
    if (lx->flags&LF_AOT_COMPILE) {
      if (tempc=HashFind(lx->ident,lx->htc.glbl_hash_table,HTT_CLASS)) {
	if (!Bt(tempc->flags,Cf_EXTERN))
	  tempc=NULL;
	else if (tempc->use_cnt<3)
	  UnusedExternWarning(lx,tempc);
      }
    } else {
      if (tempc=HashSingleTableFind(lx->ident,lx->htc.glbl_hash_table,HTT_CLASS)) {
	if (!Bt(tempc->flags,Cf_EXTERN))
	  tempc=NULL;
	else if (tempc->use_cnt<3)
	  UnusedExternWarning(lx,tempc);
      }
    }
    if (!tempc) {
      tempc=PrsClassNew;
      tempc->str=lx->ident;
      lx->ident=NULL;
      if (IsReplaceSyms)
	HashReplace(tempc,lx->htc.glbl_hash_table);
      else
	HashAdd(tempc,lx->htc.glbl_hash_table);
    } else {
      Free(tempc->src_link);
      Free(tempc->index);
    }
    LBtr(tempc->flags,Cf_EXTERN);
    if (cps_flags&CPS_IS_PUBLIC)
      tempc->type|=HTF_PUBLIC;
    SetSrcFileInfo(lx,tempc);
    tempc->use_cnt=0;
    if (Lex(lx)==':') {
      if (Lex(lx)!=TK_IDENT || !(base_class=lx->hash_entry) ||
	!(base_class->type&HTT_CLASS))
	LexExcept(lx,"ST_INVALID_CLASS");
      if (Lex(lx)==',')
	LexExcept(lx,"ST_ONE_BASE_CLASS");
      tempc->base_class=base_class;
      tempc->size+=base_class->size;
    }
    if (is_union)
      PrsVarLst(lx,tempc,PRS0_NULL|PRS1_CLASS|PRSF_UNION);
    else
      PrsVarLst(lx,tempc,PRS0_NULL|PRS1_CLASS);
  }
  return tempc;
}

CHashFun *PrsFunJoin(CLex *lx,
	CHashClass *temp_return,U8 *name,I64 cps_flags)
{
  CMemberLst *tempm;
  CAsmCtrl *a=lx->a;
  CHashFun *tempf;
  if (name) { //if not fun_ptr
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
	if (!Bt(tempf->flags,Cf_EXTERN))
	  tempf=NULL;
	else if (tempf->use_cnt<3)
	  UnusedExternWarning(lx,tempf);
      }
    }
  } else
    tempf=NULL;
  if (!tempf) {
    tempf=PrsFunNew;
    tempf->used_reg_mask=CALWAYS_CLOBBERED_MASK+CSTK_TEMPS_MASK;
    tempf->clobbered_reg_mask=CALWAYS_CLOBBERED_MASK+CSTK_TEMPS_MASK;
    tempf->str=name;
    if (lx->flags&LF_AOT_COMPILE)
      tempf->executable_addr=a->ip;
    else
      tempf->executable_addr=&UndefinedExtern;
    LBts(tempf->flags,Cf_EXTERN);
    if (cps_flags&CPS_IS_INTERRUPT)
      Bts(tempf->flags,Ff_INTERRUPT);
    if (cps_flags&CPS_IS_ARGPOP)
      Bts(tempf->flags,Ff_ARGPOP);
    if (cps_flags&CPS_IS_NOARGPOP)
      Bts(tempf->flags,Ff_NOARGPOP);
    if (name) { //if not fun_ptr
      if (IsReplaceSyms)
	HashReplace(tempf,lx->htc.glbl_hash_table);
      else
	HashAdd(tempf,lx->htc.glbl_hash_table);
    }
  } else {
    tempf->used_reg_mask=CALWAYS_CLOBBERED_MASK+CSTK_TEMPS_MASK;
    Free(tempf->src_link);
    Free(tempf->index);
    Free(name);
    MemberLstDel(tempf);
  }
  BEqu(&tempf->type,HTf_PUBLIC,cps_flags&CPS_IS_PUBLIC);
  tempf->return_class=temp_return;
  tempf->use_cnt=0;
  SetSrcFileInfo(lx,tempf);
  PrsVarLst(lx,tempf,PRS0_NULL|PRS1_FUN_ARG);
  tempf->arg_cnt=tempf->member_cnt;
  if (0<tempf->arg_cnt<<3<=MAX_I16 && !Bt(tempf->flags,Ff_DOT_DOT_DOT))
    LBts(tempf->flags,Ff_RET1);
  tempm=tempf->member_lst;
  while (tempm) {
    tempm->offset+=16; //RBP+RETURN
    tempm=tempm->next;
  }
  tempf->size=0;
  return tempf;
}

U0 PrsFun(CLex *lx,
	CHashClass *temp_return,U8 *name,I64 cps_flags)
{
  CHashFun *tempf;
  CMemberLst *tempm;
  CCodeBlkMisc *saved_leave_label;
  I64 i,j,size,*r;
  Bool old_ctrace;

  lx->fun_lfn=lx->cur_lfn;
  lx->min_line=lx->max_line=lx->cur_lfn->line_num;

  lx->flags&=~LF_NO_REG_OPT;
  tempf=PrsFunJoin(lx,temp_return,name,cps_flags);
  lx->htc.local_var_lst=tempf;

  CBPush(lx);
  Btr(&lx->flags,Lf_OPT_TRACE_PRESENT);
  CBOutPtrsInit(lx);
  CBICAdd(lx,IC_START,0,0); //So we can scan back an instruction

  CBICAdd(lx,IC_ENTER,0,0);
  saved_leave_label=lx->lb_leave;
  lx->lb_leave=CBMiscNew(lx,CBMT_LABEL);
  lx->flags&=~LF_HAS_RETURN;
  PrsStatement(lx,tempf,0,NULL,FALSE);
  if (tempf->return_class->size && !(lx->flags&LF_HAS_RETURN))
    LexPutWarn(lx,"ST_SHOULD_RETURN_VAL");
  CBICAdd(lx,IC_LABEL,lx->lb_leave,0);
  lx->lb_leave=saved_leave_label;
  CBICAdd(lx,IC_LEAVE,0,tempf->return_class);
  CBICAdd(lx,IC_END,0,0);
  tempf->size&=~7;
  if (lx->flags&LF_AOT_COMPILE) {
    tempf->executable_addr=lx->a->ip;
    tempf->type|=HTF_EXPORT;
    r=CBCompile(lx,tempf,&size,&tempf->debug);
    if (r) {
      j=(size+7)>>3;
      for (i=0;i<j;i++)
	AsmStoreCodeU64(lx,r[i]);
      Free(r);
    }
  } else {
    old_ctrace=Btr(&lx->opts,OPTf_COMPILE_TRACE);
    tempf->executable_addr=CBCompile(lx,tempf,&size,&tempf->debug);
    if (old_ctrace) {
      Bts(&lx->opts,OPTf_COMPILE_TRACE);
      Un(tempf->executable_addr,size,64);
    }
    SysSymImportsResolve(tempf->str);
  }
  LBtr(tempf->flags,Cf_EXTERN);
  CBPop(lx);
  lx->htc.local_var_lst=NULL;
  tempm=tempf->member_lst;
  while (tempm) {
    if (tempm->flags & MLF_NO_UNUSED_WARN) {
      if (tempm->use_cnt>1&&StrCmp(tempm->str,"_anon_"))
	PrintFWarn("%S\n $$LK-A,\"FL:%s,%d\"$$ '%s' in '%s'\n","ST_UNNEEDED_NOUNUSEDWARN",
	  lx->cur_lfn->name,lx->cur_lfn->line_num,tempm->str,tempf->str);
    } else if (!tempm->use_cnt && GetOpt(OPTf_WARN_UNUSED_VAR))
      PrintFWarn("%S\n $$LK-A,\"FL:%s,%d\"$$ '%s' in '%s'\n","ST_UNUSED_VAR",
	lx->cur_lfn->name,lx->cur_lfn->line_num,tempm->str,tempf->str);
    tempm=tempm->next;
  }
  lx->fun_lfn=NULL;
}

U0 PrsGlblVarLst(CLex *lx,I64 saved_mode,CHashClass *saved_tempc,
  I64 saved_val,I64 cps_flags)
{
  I64 i,j,mode,k,val;
  U8 *st;
  CHashExport *tempess;
  CHashGlblVar *tempg;
  CAsmCtrl *a=lx->a;
  CExeHeapGlbl *temphg;
  CHashClass *tempc;
  CHashFun *tempf,*tempf_fun_ptr;
  CArrayDim tempad;
  Bool has_alias,undef_array_size,is_array;
  while (TRUE) {
    tempc=PrsType(lx,&saved_tempc,&saved_mode,NULL,&st,
	&tempf_fun_ptr,&tempess,&tempad,cps_flags);
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
    if (tempess && mode&255==PRS0_EXTERN && !(lx->flags&LF_AOT_COMPILE)) {
      if (tempess->type & HTT_EXPORT_SYS_SYM) {
	val=tempess->val;
	mode=PRS0__EXTERN|PRS1_NULL;
      }
    }
    if (lx->token=='(') {
      switch (mode&255) {
	case PRS0__INTERN:
	  tempf=PrsFunJoin(lx,tempc,st,cps_flags);
	  tempf->executable_addr=val;
	  Bts(tempf->flags,Ff_INTERNAL);
	  LBtr(tempf->flags,Cf_EXTERN);
	  return;
	case PRS0__EXTERN:
	  tempf=PrsFunJoin(lx,tempc,st,cps_flags);
	  tempf->executable_addr=val;
	  LBtr(tempf->flags,Cf_EXTERN);
	  SysSymImportsResolve(tempf->str);
	  return;
	case PRS0_EXTERN:
	  PrsFunJoin(lx,tempc,st,cps_flags);
	  return;
	case PRS0_IMPORT:
	case PRS0__IMPORT:
	  if (!(lx->flags&LF_AOT_COMPILE))
	    LexExcept(lx,"ST_IMPORT_NOT_NEEDED");
	  else {
	    tempf=PrsFunJoin(lx,tempc,st,cps_flags);
	    tempf->type|=HTF_IMPORT;
	    if (mode&255==PRS0__IMPORT)
	      tempf->import_name=StrNew(val);
	    else
	      tempf->import_name=StrNew(st);
	  }
	  return;
	default:
	  PrsFun(lx,tempc,st,cps_flags);
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
	  tempg->flags|=GVAF_ALIAS;
	  break;
	case PRS0__IMPORT:
	case PRS0_IMPORT:
	  if (!(lx->flags&LF_AOT_COMPILE))
	    LexExcept(lx,"ST_IMPORT_NOT_NEEDED");
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
		LexExcept(lx,"ST_CANT_INIT_GLBL_ON_HEAP");
	      tempg=CAlloc(sizeof(CHashGlblVar));
	      temphg=tempg->heap_glbl=CAlloc(sizeof(CExeHeapGlbl));
	      temphg->size=j;
	      temphg->str=StrNew(st);
	      temphg->next=a->heap_glbls;
	      a->heap_glbls=temphg;
	      tempg->flags=GVAF_DATA_HEAP;
	      tempg->type=HTT_GLBL_VAR; //TODO: HTF_EXPORT
	      if (tempess && tempess->type & HTT_GLBL_VAR) //TODO!! extern
		LexExcept(lx,"ST_NOT_IMPLEMENTED");
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
	      if (tempess && tempess->type & HTT_GLBL_VAR)
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
	      tempg->flags=GVAF_DATA_HEAP;
	    } else {
	      tempg=CAlloc(sizeof(CHashGlblVar),Fs->code_heap);
	      tempg->data_addr=MAlloc(j,Fs->code_heap);
	    }
	    tempg->type=HTT_GLBL_VAR;
	    if (tempess && tempess->type&HTT_GLBL_VAR &&
		tempess->type&HTF_UNRESOLVED &&
		MHeapCtrl(tempess)==MHeapCtrl(tempg))
	      has_alias=TRUE;
	    if (sys_var_init_flag&1)
	      MemSet(tempg->data_addr,sys_var_init_val,j);
	  }
      }
      tempg->dim.next=tempad.next;
      if (cps_flags&CPS_IS_PUBLIC)
	tempg->type|=HTF_PUBLIC;
      tempg->var_class=tempc;
      tempg->str=st;
      tempg->size=j;
      tempg->dim.total_cnt=i;
      tempg->use_cnt=0;
      SetSrcFileInfo(lx,tempg);
      if (mode&255==PRS0_IMPORT || mode&255==PRS0__IMPORT)
	tempg->flags|=GVAF_IMPORT;
      if (mode&255==PRS0_EXTERN)
	tempg->flags|=GVAF_EXTERN;
      if (tempf_fun_ptr) {
	tempg->fun_ptr=tempf_fun_ptr;
	tempg->flags|=GVAF_FUN;
      }
      if (is_array)
	tempg->flags|=GVAF_ARRAY;
      if (IsReplaceSyms)
	HashReplace(tempg,lx->htc.glbl_hash_table);
      else
	HashAdd(tempg,lx->htc.glbl_hash_table);
      if (!(lx->flags&LF_AOT_COMPILE) && !(tempg->flags&GVAF_EXTERN))
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
	    AsmStoreCodeU8At(lx,tempg->data_addr[k],tempg->data_addr_ip+k);
	  lx->stk_ptr--;	//grows up
      }
      if (has_alias) {
	tempess(CHashGlblVar *)->flags|=GVAF_ALIAS;
	tempess(CHashGlblVar *)->data_addr=tempg->data_addr;
	tempess(CHashGlblVar *)->data_addr_ip=tempg->data_addr_ip;
      }
      if (lx->token==',')
	Lex(lx);
      else {
	if (lx->token!=';')
	  LexExcept(lx,"ST_MISSING_SEMICOLON");
	Lex(lx);
	return;
      }
    }
  }
}

U0 PrsIf(CLex *lx,CHashFun *locals,I64 try_cnt,
  CCodeBlkMisc *lb_break)
{
  CCodeBlkMisc *lb,*lb1;
  I64 k;

  if (lx->token!='(')
    LexExcept(lx,"ST_EXPECTING_LEFT_PAREN");
  Lex(lx);
  if (!PrsExpression(lx,NULL,FALSE))
    throw(EXCEPT_COMPILER,11);
  if (lx->token!=')')
    LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
  Lex(lx);
  lb=CBMiscNew(lx,CBMT_LABEL);
  CBICAdd(lx,IC_BR_ZERO,lb,0);
  PrsStatement(lx,locals,try_cnt,lb_break);
  k=PrsKeyWord(lx);
  if (k==KW_ELSE) {
    Lex(lx);
    lb1=CBMiscNew(lx,CBMT_LABEL);
    CBICAdd(lx,IC_JMP,lb1,0);
    CBICAdd(lx,IC_LABEL,lb,0);
    PrsStatement(lx,locals,try_cnt,lb_break,TRUE);
    CBICAdd(lx,IC_LABEL,lb1,0);
  } else
    CBICAdd(lx,IC_LABEL,lb,0);
}

U0 PrsWhile(CLex *lx,CHashFun *locals,I64 try_cnt)
{
  CCodeBlkMisc *lb,*lb_done;

  if (lx->token!='(')
    LexExcept(lx,"ST_EXPECTING_LEFT_PAREN");
  Lex(lx);
  lb=CBMiscNew(lx,CBMT_LABEL);
  CBICAdd(lx,IC_LABEL,lb,0);
  if (!PrsExpression(lx,NULL,FALSE))
    throw(EXCEPT_COMPILER,12);
  if (lx->token!=')')
    LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
  Lex(lx);

  lb_done=CBMiscNew(lx,CBMT_LABEL);
  CBICAdd(lx,IC_BR_ZERO,lb_done,0);
  PrsStatement(lx,locals,try_cnt,lb_done);
  CBICAdd(lx,IC_JMP,lb,0);
  CBICAdd(lx,IC_LABEL,lb_done,0);
}

U0 PrsDoWhile(CLex *lx,CHashFun *locals,I64 try_cnt)
{
  CCodeBlkMisc *lb,*lb_done;

  lb=CBMiscNew(lx,CBMT_LABEL);
  lb_done=CBMiscNew(lx,CBMT_LABEL);
  CBICAdd(lx,IC_LABEL,lb,0);
  PrsStatement(lx,locals,try_cnt,lb_done);
  if (PrsKeyWord(lx)!=KW_WHILE)
    LexExcept(lx,"ST_MISSING_WHILE");
  if (Lex(lx)!='(')
    LexExcept(lx,"ST_EXPECTING_LEFT_PAREN");
  Lex(lx);
  if (!PrsExpression(lx,NULL,FALSE))
    throw(EXCEPT_COMPILER,13);
  if (lx->token!=')')
    LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
  CBICAdd(lx,IC_BR_NOT_ZERO,lb,0);
  CBICAdd(lx,IC_LABEL,lb_done,0);
  if (Lex(lx)!=';')
    LexExcept(lx,"ST_MISSING_SEMICOLON");
  Lex(lx);
}

U0 PrsFor(CLex *lx,CHashFun *locals,I64 try_cnt)
{
  CLexCodeBlkCtrl *tempcbh;
  CCodeBlkMisc *lb,*lb_done;

  if (lx->token!='(')
    LexExcept(lx,"ST_EXPECTING_LEFT_PAREN");
  Lex(lx);
  PrsStatement(lx,locals,try_cnt);

  lb=CBMiscNew(lx,CBMT_LABEL);
  CBICAdd(lx,IC_LABEL,lb,0);
  if (!PrsExpression(lx,NULL,FALSE))
    throw(EXCEPT_COMPILER,14);
  lb_done=CBMiscNew(lx,CBMT_LABEL);
  CBICAdd(lx,IC_BR_ZERO,lb_done,0);
  if (lx->token!=';')
    LexExcept(lx,"ST_MISSING_SEMICOLON");
  Lex(lx);

  CBPush(lx);
  CBOutPtrsInit(lx);
  if (lx->token!=')')
    PrsStatement(lx,locals,try_cnt,NULL,FALSE);
  CBICAdd(lx,IC_END,0,0);
  CBPush(lx);
  tempcbh=CBPopNoFree(lx);
  CBPop(lx);
  if (lx->token!=')')
    LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
  Lex(lx);

  PrsStatement(lx,locals,try_cnt,lb_done);
  CBAppend(lx,tempcbh);
  CBICAdd(lx,IC_JMP,lb,0);
  CBICAdd(lx,IC_LABEL,lb_done,0);
}

class CSwitchCase {
  CSwitchCase *next;
  CCodeBlkMisc *label;
  I64 val;
};

U0 PrsSwitch(CLex *lx,Bool nobound,CHashFun *locals,I64 try_cnt)
{
  CSwitchCase *header=NULL,*temps,*temps1;
  CCodeBlkMisc *lb_break,*lb_dft,*mc_jt,*lb_entry,**jmp_table;
  CIntermediateCode *tempi_sub,*tempi_cmp;
  Bool dft_found=FALSE;
  I64 i,k,k2,lo=MAX_I64,hi=MIN_I64,range;

  if (lx->token!='(')
    LexExcept(lx,"ST_EXPECTING_LEFT_PAREN");
  Lex(lx);
  lb_break=CBMiscNew(lx,CBMT_LABEL);
  lb_break->flags|=CBMF_USED;
  lb_dft=CBMiscNew(lx,CBMT_LABEL);
  lb_dft->flags|=CBMF_USED;
  mc_jt=CBMiscNew(lx,CBMT_JMP_TABLE);
  if (!PrsExpression(lx,NULL,FALSE))
    throw(EXCEPT_COMPILER,15);
  tempi_sub=CBICAdd(lx,IC_IMM_U64,0,sys_internal_types[IT_I64]);
  CBICAdd(lx,IC_SUB,0,sys_internal_types[IT_I64]);
  tempi_cmp=CBICAdd(lx,IC_IMM_U64,0,sys_internal_types[IT_I64]);
  if (nobound)
    CBICAdd(lx,IC_NOBOUND_SWITCH,mc_jt,0);
  else
    CBICAdd(lx,IC_SWITCH,mc_jt,0);
  if (lx->token!=')')
    LexExcept(lx,"ST_MISSING_RIGHT_PAREN");
  if (Lex(lx)!='{')
    LexExcept(lx,"ST_EXPECTING_LEFT_BRACE");
  Lex(lx);
  while (lx->token && lx->token!='}') {
    k=PrsKeyWord(lx);
    if (k==KW_CASE) {
      Lex(lx);
      lb_entry=CBMiscNew(lx,CBMT_LABEL);
      CBICAdd(lx,IC_LABEL,lb_entry,0);
      lb_entry->flags|=CBMF_USED;
      k=Expression(lx);
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
	k2=Expression(lx);
	if (lx->token==':') {
	  Lex(lx);
	  if (k2<lo) lo=k2;
	  if (k2>hi) hi=k2;
	  if (k>k2)
	    SwapU64(&k,&k2);
	  for (i=k;i<=k2;i++) {
	    temps=MAlloc(sizeof(CSwitchCase));
	    temps->label=lb_entry;
	    temps->val=i;
	    temps->next=header;
	    header=temps;
	  }
	  } else
	  LexExcept(lx,"ST_EXPECTING_COLON");
      } else
	LexExcept(lx,"ST_EXPECTING_COLON");
    } else if (k==KW_DFT) {
      Lex(lx);
      CBICAdd(lx,IC_LABEL,lb_dft,0);
      if (lx->token==':')
	Lex(lx);
      else
	LexExcept(lx,"ST_EXPECTING_COLON");
      dft_found=TRUE;
    } else
      PrsStatement(lx,locals,try_cnt,lb_break);
  }

  if (!dft_found)
    CBICAdd(lx,IC_LABEL,lb_dft,0);
  CBICAdd(lx,IC_LABEL,lb_break,0);
  if (lx->token!='}')
    LexExcept(lx,"ST_MISSING_RIGHT_BRACE");
  Lex(lx);

  if (0<lo<=16)
    lo=0;
  range=hi-lo+1;
  if (lo>hi || !(0<range<=0x4000))
    LexExcept(lx,"ST_CASE_RANGE_ERROR");
  jmp_table=MAlloc((sizeof(CCodeBlkMisc *)*range+0x1FF)&~0x1FF);
  MemSetU64(jmp_table,lb_dft,range);
  tempi_sub->ic_data=lo;
  tempi_cmp->ic_data=range;
  temps=header;
  while (temps) {
    temps1=temps->next;
    if (jmp_table[temps->val-lo]!=lb_dft)
      LexExcept(lx,"ST_DUP_CASE");
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
      LexExcept(lx,"ST_EXPECTING_LOCAL_VAR");
    tempm->flags|=MLF_NO_UNUSED_WARN;
    if (Lex(lx)==',')
      Lex(lx);
    else {
      if (lx->token!=';')
	LexExcept(lx,"ST_EXPECTING_COMMA");
    }
  }
}

U0 PrsStreamBlk(CLex *lx)
{
  CLexHashTableContext *htc=MAlloc(sizeof(CLexHashTableContext));
  CStreamBlk *tempe=MAlloc(sizeof(CStreamBlk));

  tempe->body=StrNew("");
  QueIns(tempe,lx->last_stream_blk);
  CBPush(lx);
  lx->cbh.cb_misc_lst=NULL;
  lx->cbh.cb_misc_lst_end=&lx->cbh.cb_misc_lst;

  MemCpy(htc,&lx->htc,sizeof(CLexHashTableContext));
  htc->old_flags=lx->flags;
  lx->htc.next=htc;
  lx->htc.local_var_lst=NULL;
  lx->htc.hash_mask=HTG_ALL-HTT_IMPORT_SYS_SYM-HTT_ASM_KEYWORD;
  lx->htc.define_hash_table=lx->htc.hash_table_lst=lx->htc.glbl_hash_table=lx->htc.local_hash_table=Fs->hash_table;
  lx->flags=lx->flags & ~(LF_ASM_EXPRESSIONS|LF_AOT_COMPILE) | LF_EXE_BLK;
  if (lx->token=='{')
    Lex(lx);
  else
    LexExcept(lx,"ST_MISSING_RIGHT_BRACE");
  while (lx->token && lx->token!='}')
    ExeCmdLine(lx);

  MemCpy(&lx->htc,htc,sizeof(CLexHashTableContext));
  lx->flags=lx->flags&~LF_EXE_BLK |
    htc->old_flags & (LF_ASM_EXPRESSIONS|LF_EXE_BLK|LF_AOT_COMPILE);
  Free(htc);

  CBPop(lx);
  QueRem(tempe);
  if (*tempe->body)
    LexIncludeStr(lx,"StreamBlk",tempe->body,FALSE);
  else
    Free(tempe->body);
  Free(tempe);
  Lex(lx); //Skip "}"
}

U0 PrsTryBlk(CLex *lx,CHashFun *locals,I64 try_cnt)
{
  CCodeBlkMisc *lb_catch,*lb_done,*lb_untry;
  CHashClass *tempc=sys_internal_types[IT_PTR];
  CHashFun *temp_try=HashFind("SysTry",lx->htc.hash_table_lst,HTT_FUN),
	    *temp_untry=HashFind("SysUntry",lx->htc.hash_table_lst,HTT_FUN);

  if (!temp_try || !temp_untry)
    LexExcept(lx,"ST_HEADER_FOR_SYSTRY");

  lx->flags|=LF_NO_REG_OPT; //TODO (Currently no reg vars in funs with try/catch)

  lb_catch=CBMiscNew(lx,CBMT_LABEL);
  lb_done =CBMiscNew(lx,CBMT_LABEL);
  lb_untry=CBMiscNew(lx,CBMT_LABEL);

  CBICAdd(lx,IC_CALL_START,0,0);
  CBICAdd(lx,IC_GET_LABEL,lb_untry,tempc,ICF_PUSH_RESULT);
  CBICAdd(lx,IC_GET_LABEL,lb_catch,tempc,ICF_PUSH_RESULT);
  if (Bt(temp_try->flags,Cf_EXTERN)) {
    lx->abs_cnts.externs++;
    if (lx->flags&LF_AOT_COMPILE)
      CBICAdd(lx,IC_CALL_IMPORT,temp_try,tempc);
    else
      CBICAdd(lx,IC_CALL_INDIRECT2,&temp_try->executable_addr,tempc);
  } else
    CBICAdd(lx,IC_CALL,temp_try->executable_addr,tempc);
  if ((Bt(temp_try->flags,Ff_RET1) || Bt(temp_try->flags,Ff_ARGPOP)) && !Bt(temp_try->flags,Ff_NOARGPOP))
    CBICAdd(lx,IC_ADD_RSP1,16,tempc);
  else
    CBICAdd(lx,IC_ADD_RSP,16,tempc);
  CBICAdd(lx,IC_CALL_END,0,tempc);
  CBICAdd(lx,IC_END_EXP,0,0);

  PrsStatement(lx,locals,try_cnt+1);

  CBICAdd(lx,IC_LABEL,lb_untry,0);
  CBICAdd(lx,IC_CALL_START,0,0);
  if (Bt(temp_untry->flags,Cf_EXTERN)) {
    lx->abs_cnts.externs++;
    if (lx->flags&LF_AOT_COMPILE)
      CBICAdd(lx,IC_CALL_IMPORT,temp_untry,tempc);
    else
      CBICAdd(lx,IC_CALL_INDIRECT2,&temp_untry->executable_addr,tempc);
  } else
    CBICAdd(lx,IC_CALL,temp_untry->executable_addr,tempc);
  CBICAdd(lx,IC_CALL_END,0,tempc);
  CBICAdd(lx,IC_END_EXP,0,0);

  CBICAdd(lx,IC_JMP,lb_done,0);

  if (PrsKeyWord(lx)!=KW_CATCH)
    LexExcept(lx,"ST_MISSING_CATCH");

  Lex(lx);
  CBICAdd(lx,IC_LABEL,lb_catch,0);
  PrsStatement(lx,locals,try_cnt+1);
  CBICAdd(lx,IC_RET,0,tempc);
  CBICAdd(lx,IC_LABEL,lb_done,0);
}

Bool PrsStatement(CLex *lx,CHashFun *locals,I64 try_cnt=0,
  CCodeBlkMisc *lb_break=NULL,Bool saved_prs_semicolon=TRUE)
{
  Bool prs_exp,cont,prs_semicolon;
  I64 i,cps_flags=0,last_cps_flags=0;
  CHashExport *tempess;
  CCodeBlkMisc *g_lb;
  U8 *import_name;
  CHashFun *temp_untry;
  CExe *tempex;

  do {
    cps_flags&=~CPS_FUN_FLAGS;
    cont=FALSE;
    prs_exp=FALSE;
    prs_semicolon=saved_prs_semicolon;
    lx->statement_cnt++;
    while (lx->token==',')
      Lex(lx);
    if (lx->token=='{') {
      Lex(lx);
      lx->braces_cnt++;
      while (lx->token!='}')
	PrsStatement(lx,locals,try_cnt,lb_break);
      lx->braces_cnt--;
      lx->statement_cnt--;
      Lex(lx);
    } else if (lx->token==TK_EOF) ;
    else if (lx->token==';') {
      if (prs_semicolon) {
	lx->statement_cnt--;
	Lex(lx);
      }
    } else {
      if (lx->token==TK_IDENT) {
	if (tempess=lx->hash_entry) {
	  if (tempess->type & HTT_KEYWORD) {
	    i=tempess(CHashGeneric *)->user_data0;
	    nobound_switch (i) {
	      case KW_IF:
		Lex(lx);
		PrsIf(lx,locals,try_cnt,lb_break);
		prs_semicolon=FALSE;
		break;
	      case KW_FOR:
		Lex(lx);
		PrsFor(lx,locals,try_cnt);
		prs_semicolon=FALSE;
		break;
	      case KW_WHILE:
		Lex(lx);
		PrsWhile(lx,locals,try_cnt);
		prs_semicolon=FALSE;
		break;
	      case KW_DO:
		Lex(lx);
		PrsDoWhile(lx,locals,try_cnt);
		prs_semicolon=FALSE;
		break;
	      case KW_NOBOUND_SWITCH:
		Lex(lx);
		PrsSwitch(lx,TRUE,locals,try_cnt);
		prs_semicolon=FALSE;
		break;
	      case KW_SWITCH:
		Lex(lx);
		PrsSwitch(lx,FALSE,locals,try_cnt);
		prs_semicolon=FALSE;
		break;
	      case KW_LOCK:
		Lex(lx);
		lx->lock_cnt++;
		PrsStatement(lx,locals,try_cnt);
		lx->lock_cnt--;
		prs_semicolon=FALSE;
		break;
	      case KW__EXTERN:
		if (locals)
		  LexExcept(lx,"ST_NOT_ALLOWED_IN_FUN");
		Lex(lx);
		if (Bt(&lx->opts,OPTf_EXTERNS_TO_IMPORTS))
		  goto keyword_LTimport;
		if (lx->flags&LF_AOT_COMPILE) {
		  if (lx->token!=TK_IDENT || !(tempess=lx->hash_entry) ||
		      !(tempess->type & HTT_EXPORT_SYS_SYM))
		    LexExcept(lx,"ST_EXPECTING_SYS_SYM");
		  i=tempess->val;
		  Lex(lx);
		} else
		  i=Expression(lx);
		if (lx->token!=TK_IDENT || !(tempess=lx->hash_entry) ||
		    !(tempess->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		  LexExcept(lx,"ST_EXPECTING_TYPE");
		Lex(lx);
		PrsGlblVarLst(lx,PRS0__EXTERN|PRS1_NULL,tempess,i,last_cps_flags);
		prs_semicolon=FALSE;
		cont=TRUE;
		break;
	      case KW__IMPORT:
		if (locals)
		  LexExcept(lx,"ST_NOT_ALLOWED_IN_FUN");
		Lex(lx);
keyword_LTimport:
		if (lx->token!=TK_IDENT)
		  LexExcept(lx,"ST_EXPECTING_SYS_SYM");
		import_name=lx->ident;
		lx->ident=0;
		if (Lex(lx)!=TK_IDENT || !(tempess=lx->hash_entry) ||
		   !(tempess->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		  LexExcept(lx,"ST_EXPECTING_TYPE");
		Lex(lx);
		PrsGlblVarLst(lx,PRS0__IMPORT|PRS1_NULL,tempess,import_name,FALSE);
		Free(import_name);
		prs_semicolon=FALSE;
		cont=TRUE;
		break;
	      case KW__INTERN:
		if (locals)
		  LexExcept(lx,"ST_NOT_ALLOWED_IN_FUN");
		Lex(lx);
		i=Expression(lx);
		if (lx->token!=TK_IDENT || !(tempess=lx->hash_entry) ||
		    !(tempess->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		  LexExcept(lx,"ST_EXPECTING_TYPE");
		Lex(lx);
		PrsGlblVarLst(lx,PRS0__INTERN|PRS1_NULL,tempess,i,last_cps_flags);
		prs_semicolon=FALSE;
		cont=TRUE;
		break;
	      case KW_EXTERN:
		if (locals)
		  LexExcept(lx,"ST_NOT_ALLOWED_IN_FUN");
		Lex(lx);
		if (lx->token!=TK_IDENT)
		  LexExcept(lx,"ST_EXPECTING_TYPE");
		tempess=lx->hash_entry;
		if (PrsKeyWord(lx)==KW_CLASS) {
		  Lex(lx);
		  PrsClass(lx,FALSE,last_cps_flags,TRUE);
		} else if (PrsKeyWord(lx)==KW_UNION) {
		  Lex(lx);
		  PrsClass(lx,TRUE,last_cps_flags,TRUE);
		} else {
		  if (!tempess || !(tempess->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		    LexExcept(lx,"ST_EXPECTING_TYPE");
		  if (Bt(&lx->opts,OPTf_EXTERNS_TO_IMPORTS))
		    goto keyword_import;
		  Lex(lx);
		  PrsGlblVarLst(lx,PRS0_EXTERN|PRS1_NULL,tempess,0,last_cps_flags);
		  prs_semicolon=FALSE;
		  cont=TRUE;
		}
		break;
	      case KW_IMPORT:
		if (locals)
		  LexExcept(lx,"ST_NOT_ALLOWED_IN_FUN");
		Lex(lx);
		if (lx->token!=TK_IDENT || !(tempess=lx->hash_entry) ||
		  !(tempess->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
		  LexExcept(lx,"ST_EXPECTING_TYPE");
keyword_import:
		Lex(lx);
		PrsGlblVarLst(lx,PRS0_IMPORT|PRS1_NULL,tempess,0,FALSE);
		prs_semicolon=FALSE;
		cont=TRUE;
		break;
	      case KW_ASM:
		if (locals) {
		  if (tempex=CmpJoin(lx,locals,FALSE,TRUE))
		    CBICAdd(lx,IC_ASM,tempex,0);
		  Lex(lx); //Skip "}" of asm{}
		} else {
		  if (lx->flags&LF_AOT_COMPILE || lx->a_depth) {
		    Lex(lx);
		    PrsAsmBlk(lx);
		    if (lx->flags&LF_AOT_COMPILE && lx->a_depth==1)
		      Lex(lx); //Skip "}" of asm{}
		  } else {
		    if (tempex=CmpJoin(lx,NULL,FALSE,TRUE))
		      CmpFixUpJITAsm(lx,tempex,tempex->code);
		    Lex(lx); //Skip "}" of asm{}
		  }
		  cps_flags|=CPS_IS_ASM;
		}
		prs_semicolon=FALSE;
		break;
	      case KW_TRY:
		Lex(lx);
		PrsTryBlk(lx,locals,try_cnt);
		prs_semicolon=FALSE;
		break;
	      case KW_BREAK:
		Lex(lx);
		if (!lb_break)
		  LexExcept(lx,"ST_BREAK_NOT_ALLOWED");
		CBICAdd(lx,IC_JMP,lb_break,0);
		break;
	      case KW_RETURN:
		if (!locals)
		  LexExcept(lx,"ST_CANT_RETURN_VAL");
		if (try_cnt) {
		  temp_untry=HashFind("SysUntry",
		    lx->htc.hash_table_lst,HTT_FUN);
		  for (i=0;i<try_cnt;i++) {
		    if (Bt(temp_untry->flags,Cf_EXTERN)) {
		      lx->abs_cnts.externs++;
		      if (lx->flags&LF_AOT_COMPILE)
			CBICAdd(lx,IC_CALL_IMPORT,
			  temp_untry,sys_internal_types[IT_PTR]);
		      else
			CBICAdd(lx,IC_CALL_INDIRECT2,
			  &temp_untry->executable_addr,sys_internal_types[IT_PTR]);
		    } else
		      CBICAdd(lx,IC_CALL,temp_untry->executable_addr,sys_internal_types[IT_PTR]);
		  }
		}
		if (Lex(lx)!=';') {
		  if (!locals->return_class->size)
		    LexPutWarn(lx,"ST_SHOULD_NOT_RETURN_VAL");
		  if (!PrsExpression(lx,NULL,FALSE))
		    throw(EXCEPT_COMPILER,17);
		  CBICAdd(lx,IC_RETURN_VAL,0,locals->return_class);
		  lx->flags|=LF_HAS_RETURN;
		} else if (locals->return_class->size)
		  LexPutWarn(lx,"ST_SHOULD_RETURN_VAL");
		CBICAdd(lx,IC_JMP,lx->lb_leave,0);
		break;
	      case KW_GOTO:
		Lex(lx);
		if (lx->token!=TK_IDENT)
		  LexExcept(lx,"ST_EXPECTING_IDENTIFIER");
		if (!(g_lb=CBGoToLabelFind(lx,lx->ident))) {
		  g_lb=CBMiscNew(lx,CBMT_GOTO_LABEL);
		  g_lb->str=lx->ident;
		  lx->ident=NULL;
		}
		g_lb->flags|=CBMF_USED;
		CBICAdd(lx,IC_JMP,g_lb,0);
		Lex(lx);
		break;
	      case KW_NOUNUSEDWARN:
		Lex(lx);
		PrsNoUnusedWarn(lx);
		break;
	      case KW_CLASS:
		Lex(lx);
		tempess=PrsClass(lx,FALSE,last_cps_flags,FALSE);
		if (!locals && lx->token!=';') {
		  PrsGlblVarLst(lx,PRS0_NULL|PRS1_NULL,tempess,0,last_cps_flags);
		  cont=TRUE;
		  prs_semicolon=FALSE;
		}
		break;
	      case KW_UNION:
		Lex(lx);
		tempess=PrsClass(lx,TRUE,last_cps_flags,FALSE);
		if (!locals && lx->token!=';') {
		  PrsGlblVarLst(lx,PRS0_NULL|PRS1_NULL,tempess,0,last_cps_flags);
		  cont=TRUE;
		  prs_semicolon=FALSE;
		}
		break;
	      case KW_PUBLIC:
		Lex(lx);
		cps_flags|=CPS_IS_PUBLIC|last_cps_flags&CPS_FUN_FLAGS;
		cont=TRUE;
		prs_semicolon=FALSE;
		break;
	      case KW_INTERRUPT:
		Lex(lx);
		cps_flags|=CPS_IS_INTERRUPT|CPS_IS_NOARGPOP|last_cps_flags&CPS_FUN_FLAGS;
		cont=TRUE;
		prs_semicolon=FALSE;
		break;
	      case KW_ARGPOP:
		Lex(lx);
		cps_flags|=CPS_IS_ARGPOP|last_cps_flags&CPS_FUN_FLAGS;
		cont=TRUE;
		prs_semicolon=FALSE;
		break;
	      case KW_NOARGPOP:
		Lex(lx);
		cps_flags|=CPS_IS_NOARGPOP|last_cps_flags&CPS_FUN_FLAGS;
		cont=TRUE;
		prs_semicolon=FALSE;
		break;
	      case KW_STATIC:
		Lex(lx);
		cps_flags|=CPS_IS_STATIC;
		cont=TRUE;
		prs_semicolon=FALSE;
		break;
	      case KW_NUM_KEYWORDS: //nobound_switch
	      default:
		prs_exp=TRUE;
	    }
	  } else if (tempess->type & (HTT_CLASS|HTT_INTERNAL_TYPE)) {
	    if (locals) {
	      if (cps_flags&CPS_IS_STATIC) {
		PrsVarLst(lx,locals,PRS0_NULL|PRS1_STATIC_LOCAL_VAR);
		cps_flags&=~CPS_IS_STATIC;
	      } else
		PrsVarLst(lx,locals,PRS0_NULL|PRS1_LOCAL_VAR);
	      if (lx->token=='}')
		cont=FALSE;
	      else
		cont=TRUE;
	    } else {
	      Lex(lx);
	      PrsGlblVarLst(lx,PRS0_NULL|PRS1_NULL,tempess,0,last_cps_flags);
	      cont=TRUE;
	    }
	    prs_semicolon=FALSE;
	  } else prs_exp=TRUE;
	} else {
	  if (lx->local_var_entry)
	    prs_exp=TRUE;
	  else {
	    if (!(g_lb=CBGoToLabelFind(lx,lx->ident))) {
	      g_lb=CBMiscNew(lx,CBMT_GOTO_LABEL);
	      g_lb->str=lx->ident;
	      lx->ident=NULL;
	    } else if (g_lb->flags&CBMF_DEFINED)
	      LexExcept(lx,"ST_DUP_GOTO_LABEL");
	    g_lb->flags|=CBMF_DEFINED;
	    Lex(lx); //skip ident
	    CBICAdd(lx,IC_LABEL,g_lb,0);
	    if (lx->token==':') //skip ident
	      Lex(lx); //skip colon
	    else
	      LexExcept(lx,"ST_UNDEFINED_IDENTIFIER");
	    if (!locals)
	      LexExcept(lx,"ST_NO_GLBL_LABELS");
	    prs_semicolon=FALSE;
	  }
	}
      } else prs_exp=TRUE;
      if (prs_exp) {
	if (lx->token==TK_STR||lx->token==TK_U8)
	  PrsFunCall(lx,NULL,FALSE,NULL);
	else if (!PrsExpression(lx,NULL,TRUE))
	  throw(EXCEPT_COMPILER,18);
      }
      if (lx->token!=',')
	lx->statement_cnt--;
      if (prs_semicolon && lx->token!=',') {
	if (lx->token==';')
	  Lex(lx);
	else
	  LexExcept(lx,"ST_MISSING_SEMICOLON");
      }
    }
    last_cps_flags=cps_flags;
  } while (lx->token==',' || cont);
  return cps_flags&CPS_IS_ASM;
}
