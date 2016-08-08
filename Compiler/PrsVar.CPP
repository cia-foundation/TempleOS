U0 PrsVarInit(CCmpCtrl *cc,U8 **_dst,CHashClass *tempc,CArrayDim *tempad,
	U8 *data_addr_ip,U8 **_base,Bool data_heap,I64 pass)
{
  U8 *dst=*_dst,*machine_code;
  I64 i,j,r,old_flags,type,size;
  CMemberLst *tempm;
  CIntermediateCode *tempi;
  CAOTCtrl *aotc=cc->aotc;
  CAOTAbsAddr *tempa;
  CAOTImportExport *tempie;
  Bool is_str;

  tempc=OptClassFwd(tempc);
  if (tempm=tempc->member_lst_and_root) {
    if (cc->token!='{')
      LexExcept(cc,"Expecting '{' at ");
    LexPopNoRestore(cc);
    LexPush(cc);
    Lex(cc);
    while (tempm) {
      PrsVarInit2(cc,&dst,tempm->member_class,&tempm->dim,
	    data_addr_ip,_base,data_heap,pass);
      if (cc->token==',')
	Lex(cc);
      tempm=tempm->next;
    }
    LexPopNoRestore(cc);
    if (cc->token!='}')
      LexExcept(cc,"Missing '}' at ");
    Lex(cc);
  } else {
    if (tempc->ptr_stars_cnt==1 &&
	  ((tempc-1)->raw_type==RT_I8 || (tempc-1)->raw_type==RT_U8) &&
	  !tempad && cc->token==TK_STR)
      is_str=TRUE;
    else
      is_str=FALSE;
    if (cc->flags&CCF_AOT_COMPILE && is_str) {
      LexPopNoRestore(cc);
      machine_code=LexExtStr(cc,&i);
      if (pass==2) {
	tempa=CAlloc(sizeof(CAOTAbsAddr));
	tempa->next=aotc->abss;
	tempa->type=AAT_ADD_U64;
	aotc->abss=tempa;
	tempa->ip=data_addr_ip+dst-*_base;
	*dst(I64 *)=aotc->ip;
	for (j=0;j<i;j++)
	  AOTStoreCodeU8(cc,machine_code[j]);
      }
      Free(machine_code);
    } else {
      old_flags=cc->flags;
      cc->flags=CCF_NO_ABSS | cc->flags &
	    ~(CCF_AOT_COMPILE|CCF_HAS_MISC_DATA|CCF_NOT_CONST);
      machine_code=LexExpression2Bin(cc,&type);
      if (old_flags&CCF_AOT_COMPILE &&
	    cc->flags&CCF_NOT_CONST &&
	    !Bt(&cc->opts,OPTf_GLBLS_ON_DATA_HEAP)) {
	cc->flags=cc->flags&~CCF_NO_ABSS|CCF_AOT_COMPILE;
	Free(machine_code);
	if (pass==2) {
	  MemSet(dst,0,tempc->size);
	  LexPopRestore(cc);
	  Lex(cc);
	  COCPush(cc);
	  COCInit(cc);
	  ICAdd(cc,IC_ABS_ADDR,data_addr_ip,tempc+1);
	  ICAdd(cc,IC_IMM_I64,dst-*_base,tempc+1);
	  ICAdd(cc,IC_ADD,0,tempc+1);
	  if (!PrsExpression(cc,NULL,TRUE))
	    throw('Compiler');
	  tempi=cc->coc.coc_head.last;
	  if (tempi->ic_code==IC_END_EXP) {
	    tempi->ic_code=IC_NOP1;
	    tempi->ic_flags=0;
	  }
	  ICAdd(cc,IC_ASSIGN,0,tempc);
	  ICAdd(cc,IC_END_EXP,0,tempc,ICF_RES_NOT_USED);
	  ICAdd(cc,IC_RET,0,0);
	  if (machine_code=COCCompile(cc,&size,NULL,NULL)) {
	    tempie=CAlloc(sizeof(CAOTImportExport));
	    tempie->type=IET_MAIN;
	    tempie->ip=cc->aotc->ip;
	    QueIns(tempie,cc->aot->last_ie);
	    for (i=0;i<size;i++)
	      AOTStoreCodeU8(cc,machine_code[i]);
	    Free(machine_code);
	  }
	  COCPop(cc);
	} else
	  LexPopNoRestore(cc);
      } else {
	LexPopNoRestore(cc);
	if (!machine_code)
	  throw('Compiler');
	r=Call(machine_code);
	if (!(cc->flags & CCF_HAS_MISC_DATA)||pass==1)
	  Free(machine_code);

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
    cc->flags=cc->flags&
	  ~CCF_NO_ABSS|old_flags&(CCF_HAS_MISC_DATA|CCF_AOT_COMPILE);
  }
  *_dst=dst;
}

class CVI2
{
  CVI2 *next,*last;
  U0 base;
};

U0 PrsVarInit2(CCmpCtrl *cc,U8 **_dst,CHashClass *tempc,
	CArrayDim *tempad,U8 *data_addr_ip,U8 **_base,Bool data_heap,I64 pass)
{
  I64 i,j,cnt;
  U8 *st,*_b;
  CVI2 head,*tempvi,*tempvi1;
  CArrayDim *tempad1;
  tempc=OptClassFwd(tempc);
  if (tempad1=tempad->next) {
    if (!tempc->ptr_stars_cnt &&
	  (tempc->raw_type==RT_I8 || tempc->raw_type==RT_U8) &&
	  cc->token==TK_STR) {
      LexPopNoRestore(cc);
      st=LexExtStr(cc,&i);
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
      LexPush(cc);
    } else {
      if (cc->token=='{') {
	LexPopNoRestore(cc);
	LexPush(cc);
	Lex(cc);
      }
      if (tempad1->cnt<0) {//[]
	QueInit(&head);
	cnt=0;
	while (cc->token!='}') {
	  tempvi=MAlloc(offset(CVI2.base)+tempad1->total_cnt*tempc->size);
	  _b=&tempvi->base;
	  PrsVarInit2(cc,&_b,tempc,tempad1,data_addr_ip,_base,data_heap,pass);
	  QueIns(tempvi,head.last);
	  if (cc->token==',')
	    Lex(cc);
	  cnt++;
	}
	Lex(cc); //skip '}'
	tempad1->cnt=cnt;
	tempad->total_cnt=cnt*tempad1->total_cnt;
	j=tempad1->total_cnt*tempc->size;
	i=cnt*j;
	Free(*_base);
	if (data_heap)
	  *_base=_b=MAlloc(i);
	else
	  *_base=_b=MAlloc(i,Fs->code_heap);
	tempvi=head.next;
	while (tempvi!=&head) {
	  tempvi1=tempvi->next;
	  MemCpy(_b,&tempvi->base,j);
	  _b+=j;
	  Free(tempvi);
	  tempvi=tempvi1;
	}
	*_dst=_b;
      } else {
	for (i=0;i<tempad1->cnt;i++) {
	  PrsVarInit2(cc,_dst,tempc,tempad1,data_addr_ip,_base,data_heap,pass);
	  if (tempad1->cnt>1 && cc->token==',')
	    Lex(cc);
	}
	if (cc->token=='}')
	  Lex(cc);
      }
    }
  } else {
    PrsVarInit(cc,_dst,tempc,tempad1,data_addr_ip,_base,data_heap,pass);
    LexPush(cc);
  }
}

U0 PrsGlblInit(CCmpCtrl *cc,CHashGlblVar *tempg,I64 pass)
{
  U8 *dst=tempg->data_addr;
  PrsVarInit2(cc,&dst,tempg->var_class,&tempg->dim,
	tempg->data_addr_ip,&tempg->data_addr,
	Bt(&cc->opts,OPTf_GLBLS_ON_DATA_HEAP)||
	Bt(&cc->flags,CCf_AOT_COMPILE),pass);
}

U0 PrsStaticInit(CCmpCtrl *cc,CMemberLst *tempm,I64 pass)
{
  U8 *machine_code,*dst=tempm->static_data;
  CHashClass *tempc=tempm->member_class;
  I64 i,size;
  CAOTImportExport *tempie;

  if (cc->flags&CCF_AOT_COMPILE && pass==2) {
    COCPush(cc);
    COCInit(cc);
  }
  PrsVarInit2(cc,&dst,tempc,&tempm->dim,tempm->static_data_ip,
	&tempm->static_data,Bt(&cc->flags,CCf_AOT_COMPILE),pass);
  if (cc->flags&CCF_AOT_COMPILE && pass==2) {
    if (cc->coc.coc_head.next!=&cc->coc.coc_head) {
      ICAdd(cc,IC_RET,0,0);
      if (machine_code=COCCompile(cc,&size,NULL,NULL)) {
	if (pass==2) {
	  tempie=CAlloc(sizeof(CAOTImportExport));
	  tempie->type=IET_MAIN;
	  tempie->ip=cc->aotc->ip;
	  QueIns(tempie,cc->aot->last_ie);
	  for (i=0;i<size;i++)
	    AOTStoreCodeU8(cc,machine_code[i]);
	}
	Free(machine_code);
      }
    } //TODO: else del misc?
    COCPop(cc);
  }
}

Bool PrsArrayDims(CCmpCtrl *cc,I64 mode,CArrayDim *dim)
{//Returns false on empty []
//dim->next!=0 for array
  CArrayDim *tempad,*tempad1;
  I64 j;
  Bool res=TRUE;
  dim->next=NULL;
  dim->cnt=0;
  dim->total_cnt=1;
  tempad1=&dim->next;
  if (cc->token=='[') {
    if (mode.u8[1]==PRS1B_FUN_ARG)
      LexExcept(cc,"No arrays in fun args at ");
    do {
      if (Lex(cc)==']' && !dim->next) {
	j=-1;
	res=FALSE;
      } else {
	if ((j=LexExpressionI64(cc))<0)
	  LexExcept(cc,"Invalid array size at ");
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
      if (cc->token!=']')
	LexExcept(cc,"Missing ']' at ");
    } while (Lex(cc)=='[');
  }
  return res;
}

CHashClass *PrsType(CCmpCtrl *cc,CHashClass **_tempc1,
  I64 *_mode,CMemberLst *tempm,U8 **_ident,CHashFun **_fun_ptr,
  CHashExport **_tempex,CArrayDim *tempad,I64 fsp_flags)
{
  I64 k,ptr_stars_cnt,mode=*_mode;
  CHashClass *tempc1=*_tempc1,*tempc2;
  CHashFun *fun_ptr=NULL;
  CHashExport *tempex=NULL;
  CArrayDim *tempad2;

  pt_start:
  if (!tempc1 || !(tempc1->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
    LexExcept(cc,"Invalid class at ");

  ptr_stars_cnt=0;
  while (cc->token=='*') {
    if (mode.u8[1]) {
      LexPopNoRestore(cc);
      LexPush(cc);
    }
    Lex(cc);
    tempc1++;
    if (++ptr_stars_cnt>MAX_PTR_STARS)
      LexExcept(cc,"Too many *'s at ");
  }

  k=PrsKeyWord(cc);
  if (k==KW_UNION || k==KW_CLASS) {
    Lex(cc);
    tempc2=PrsClass(cc,k,fsp_flags,mode&255==PRS0_EXTERN);
    tempc2->fwd_class=tempc1;
    tempc1=tempc2;
    if (_tempc1) *_tempc1=tempc1;
    mode=PRS0_NULL|PRS1_NULL;
    goto pt_start;
  }

  if (cc->token=='(') {
    if (Lex(cc)!='*')
      LexExcept(cc,"Expecting '*' at ");
    ptr_stars_cnt=1; //fun_ptr
    while (Lex(cc)=='*')
      ptr_stars_cnt++; //fun_ptr
    if (ptr_stars_cnt>MAX_PTR_STARS)
      LexExcept(cc,"Too many *'s at ");
  } else
    ptr_stars_cnt=-1; //fun_ptr

  if (_ident) {
    if (cc->token==TK_IDENT) {
      tempex=cc->hash_entry;
      *_ident=cc->cur_str;
      cc->cur_str=NULL;
      Lex(cc);
    } else {
      if (!mode.u8[1])
	*_ident=NULL;
      else if (cc->token==',' || cc->token==';' || cc->token==')') {
	tempex=NULL;
	*_ident=StrNew("_anon_");
	tempm->flags|=MLF_NO_UNUSED_WARN;
      } else
	LexExcept(cc,"Expecting identifier at ");
    }
  }

  if (ptr_stars_cnt>=0) { //fun_ptr
    if (cc->token!=')')
      LexExcept(cc,"Missing ')' at ");
    if (Lex(cc)!='(')
      LexExcept(cc,"Expecting '(' at ");
    fun_ptr=PrsFunJoin(cc,tempc1,NULL,fsp_flags)+ptr_stars_cnt;
    tempc1=cmp.internal_types[RT_PTR]+ptr_stars_cnt;
  }
  if (!PrsArrayDims(cc,mode,tempad) && cc->token!='=') {
    tempc1++;
    tempad2=tempad->next->next;
    Free(tempad->next);
    tempad->next=tempad2;
  }

  tempc2=OptClassFwd(tempc1);
  if (tempc2->ptr_stars_cnt) {
    tempc2-=tempc2->ptr_stars_cnt;
    if (tempc2->type&HTT_INTERNAL_TYPE && !tempc2->size)
      LexWarn(cc,"use \"U8 *\" instead of \"U0 *\" at ");
  }

  if (_mode)	*_mode=mode;
  if (_fun_ptr)	*_fun_ptr=fun_ptr;
  if (_tempex)	*_tempex=tempex;
  return tempc1;
}

U0 PrsDotDotDot(CCmpCtrl *cc,CHashFun *tempf,I64 _reg)
{
  CMemberLst *tempm;
  CArrayDim *tempad;

  Bts(&tempf->flags,Ff_DOT_DOT_DOT);

  Lex(cc);
  tempm=MemberLstNew(_reg);
  tempm->flags=MLF_DOT_DOT_DOT;
  tempm->member_class=cmp.internal_types[RT_I64];
  tempm->str=StrNew("argc");
  tempm->offset=tempf->size;
  tempm->size=8;
  tempf->size+=8;
  MemberAdd(cc,tempm,tempf,PRS1B_FUN_ARG);

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
  MemberAdd(cc,tempm,tempf,PRS1B_FUN_ARG);

  if (cc->token==')')
    Lex(cc);
}

U0 PrsVarLst(CCmpCtrl *cc,CHashClass *tempc,I64 mode,I64 union_base=0)
{
  I64 i,k,old_flags=cc->flags,old_flags2,type,_reg;
  CHashClass *tempc1,*tempc2;
  CHash *temph;
  CMemberLst *tempm;
  CMemberLstMeta *temp_meta;
  U8 *machine_code;
  Bool undef_array_size,first;
  cc->flags|=CCF_DONT_MAKE_RES;
  if (mode.u8[1]==PRS1B_CLASS)
    cc->flags|=CCF_CLASS_IP;
  if ((mode.u8[1]!=PRS1B_LOCAL_VAR && mode.u8[1]!=PRS1B_STATIC_LOCAL_VAR ||
	mode&PRSF_UNION) && (cc->token=='(' || cc->token=='{'))
    Lex(cc);
  while (TRUE) {
    while (cc->token==';')
      Lex(cc);
    if (mode&PRSF_UNION)
      cc->class_ip=union_base;
    else
      cc->class_ip=tempc->size;
    while (cc->token=='$$') {
      if (Lex(cc)!='=') //skip $$
	LexExcept(cc,"Expecting '=' at ");
      Lex(cc); //skip =
      cc->class_ip=LexExpression(cc);
      if (-cc->class_ip>tempc->neg_offset)
	tempc->neg_offset=-cc->class_ip;
      if (mode&PRSF_UNION)
	union_base=cc->class_ip;
      else
	tempc->size=cc->class_ip;
      if (cc->token!=';')
	LexExcept(cc,"Missing ';' at");
      Lex(cc); //skip ;
    }
    if (cc->token==')' || cc->token=='}') {
      Lex(cc);
      goto pvl_done;
    }
    _reg=REG_UNDEF;
pvl_restart1:
    switch (PrsKeyWord(cc)) {
      case KW_REG:
	_reg=REG_ALLOC;
	if (Lex(cc)==TK_IDENT) {
	  k=DefineMatch(cc->cur_str,"ST_U64_REGS");
	  if (k>=0) {
	    _reg=k;
	    Lex(cc);
	  }
	}
	goto pvl_restart1;
      case KW_NOREG:
	_reg=REG_NONE;
	Lex(cc);
	goto pvl_restart1;
    }

    if (cc->token==TK_DOT_DOT_DOT && mode.u8[1]==PRS1B_FUN_ARG) {
      PrsDotDotDot(cc,tempc,_reg);
      goto pvl_done;
    }
    if (cc->token==TK_IDENT)
      temph=cc->hash_entry;
    else
      temph=NULL;
    if (!temph)
      LexExcept(cc,"Expecting type at ");
    k=PrsKeyWord(cc);
    if (k==KW_UNION) {
      Lex(cc);
      PrsVarLst(cc,tempc,mode|PRSF_UNION,tempc->size);
    } else {
      if (!(temph->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
	LexExcept(cc,"Expecting type at ");
      first=TRUE;
pvl_restart2:
      tempc1=temph;
      LexPush(cc);
      Lex(cc); //skip type or ','
      tempm=MemberLstNew(_reg);
      _reg=REG_UNDEF;
      if (mode.u8[1]==PRS1B_STATIC_LOCAL_VAR) {
	tempm->flags|=MLF_STATIC;
	tempm->reg=REG_NONE;
      }
      if (mode.u8[1]==PRS1B_FUN_ARG || mode.u8[1]==PRS1B_LOCAL_VAR) {
pvl_restart3:
	switch (PrsKeyWord(cc)) {
	  case KW_REG:
	    tempm->reg=REG_ALLOC;
	    LexPopNoRestore(cc);
	    LexPush(cc);
	    if (Lex(cc)==TK_IDENT) {
	      k=DefineMatch(cc->cur_str,"ST_U64_REGS");
	      if (k>=0) {
		tempm->reg=k;
		LexPopNoRestore(cc);
		LexPush(cc);
		Lex(cc);
	      }
	    }
	    goto pvl_restart3;
	  case KW_NOREG:
	    tempm->reg=REG_NONE;
	    LexPopNoRestore(cc);
	    LexPush(cc);
	    Lex(cc);
	    goto pvl_restart3;
	}
      }
      tempm->member_class=PrsType(cc,&tempc1,&mode,tempm,&tempm->str,
	    &tempm->fun_ptr,NULL,&tempm->dim,0);
      if (tempm->fun_ptr)
	tempm->flags|=MLF_FUN;
      if (first)
	MemberAdd(cc,tempm,tempc,mode.u8[1]);
      else
	MemberAdd(cc,tempm,tempc,PRS1B_NULL);
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
	    LexExcept(cc,"Static unions are not implemented ");
	  k=(i+7)&~7;
	  if (cc->flags&CCF_AOT_COMPILE)
	    tempm->static_data=MAlloc(k);
	  else
	    tempm->static_data=MAlloc(k,Fs->code_heap);
	  if (cc->flags&CCF_AOT_COMPILE)	{
	    tempm->static_data_ip=cc->aotc->ip;
	    k>>=3;
	    if (sys_var_init_flag) {
	      k<<=3;
	      while (k--)
		AOTStoreCodeU8(cc,sys_var_init_val);
	    } else
	      while (k--)
		AOTStoreCodeU64(cc,0);
	  } else
	    if (sys_var_init_flag)
	      MemSet(tempm->static_data,sys_var_init_val,k);
	  LexPopNoRestore(cc);
	  if (cc->token=='=') {
	    cc->flags=cc->flags&
		  ~CCF_DONT_MAKE_RES|old_flags&CCF_DONT_MAKE_RES;
	    if (undef_array_size) {
	      LexPush(cc);
	      LexPush(cc);
	      Lex(cc); //skip =
	      PrsStaticInit(cc,tempm,1);
	      LexPopNoRestore(cc);
	      i=tempc2->size*tempm->dim.total_cnt;
	      k=(i+7)&~7;
	      if (cc->flags&CCF_AOT_COMPILE)	{
		k>>=3;
		if (sys_var_init_flag) {
		  k<<=3;
		  while (k--)
		    AOTStoreCodeU8(cc,sys_var_init_val);
		} else
		  while (k--)
		    AOTStoreCodeU64(cc,0);
	      } else
		if (sys_var_init_flag)
		  MemSet(tempm->static_data,sys_var_init_val,k);
	      LexPopRestore(cc);
	    }
	    LexPush(cc);
	    Lex(cc); //skip =
	    PrsStaticInit(cc,tempm,2);
	    LexPopNoRestore(cc);
	    if (cc->flags&CCF_AOT_COMPILE)
	      for (k=0;k<i;k++)
		AOTStoreCodeU8At(cc,tempm->static_data_ip+k,
		      tempm->static_data[k]);
	    tempm->use_cnt=0;
	    cc->flags|=CCF_DONT_MAKE_RES;
	  }
	  if (cc->flags&CCF_AOT_COMPILE)
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
	  if (cc->token=='=') {
	    cc->flags=cc->flags&~CCF_DONT_MAKE_RES|
		  old_flags&CCF_DONT_MAKE_RES;
	    LexPopRestore(cc);
	    Lex(cc);
	    if (!PrsExpression(cc,NULL,TRUE))
	      throw('Compiler');
	    tempm->use_cnt=0;
	    cc->flags|=CCF_DONT_MAKE_RES;
	  } else
	    LexPopNoRestore(cc);
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
	  if (cc->token=='=') {
	    Lex(cc);
	    if (PrsKeyWord(cc)==KW_LASTCLASS) {
	      tempm->flags|=MLF_LASTCLASS;
	      Lex(cc);
	    } else {
	      old_flags2=cc->flags;
	      cc->flags&=~CCF_HAS_MISC_DATA;
	      machine_code=LexExpression2Bin(cc,&type);
	      if (!machine_code)
		throw('Compiler');
	      tempm->dft_val=Call(machine_code);
	      tempc2=OptClassFwd(tempc2);
	      if (tempc2->raw_type==RT_F64) {
		if (type!=RT_F64)
		  tempm->dft_val(F64)=tempm->dft_val;
	      } else {
		if (type==RT_F64)
		  tempm->dft_val=tempm->dft_val(F64);
	      }
	      if (cc->flags & CCF_HAS_MISC_DATA) {
		tempm->dft_val=StrNew(tempm->dft_val);
		tempm->flags|=MLF_STR_DFT_AVAILABLE;
	      }
	      Free(machine_code);
	      cc->flags|=old_flags2&CCF_HAS_MISC_DATA;
	    }
	    tempm->flags|=MLF_DFT_AVAILABLE;
	  }
	  LexPopNoRestore(cc);
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
	  while (cc->token==TK_IDENT) {
	    temp_meta=MAlloc(sizeof(CMemberLstMeta));
	    temp_meta->next=tempm->meta;
	    tempm->meta=temp_meta;
	    temp_meta->str=cc->cur_str;
	    temp_meta->flags=0;
	    cc->cur_str=NULL;
	    if (Lex(cc)==TK_STR) {
	      temp_meta->user_data=LexExtStr(cc);
	      temp_meta->flags|=MLMF_IS_STR;
	    } else
	      temp_meta->user_data=LexExpression(cc);
	  }
	  LexPopNoRestore(cc);
	  break;
      }
      switch (cc->token) {
	case ',':
	  if (mode.u8[1]==PRS1B_FUN_ARG && !(mode&PRSF_UNION))
	    Lex(cc);
	  else {
	    first=FALSE;
	    goto pvl_restart2;
	  }
	  break;
	case ')':
	case '}':
	  Lex(cc);
	  goto pvl_done;
	case ';':
	  cc->flags=cc->flags&~CCF_DONT_MAKE_RES|
		old_flags&CCF_DONT_MAKE_RES;
	  Lex(cc);
	  cc->flags|=CCF_DONT_MAKE_RES;
	  if ((mode.u8[1]==PRS1B_LOCAL_VAR||mode.u8[1]==
		PRS1B_STATIC_LOCAL_VAR) && !(mode&PRSF_UNION))
	    goto pvl_done;
	  break;
	default:
	  LexExcept(cc,"Missing ';' at");
      }
    }
  }
pvl_done:
  cc->flags=cc->flags&~(CCF_CLASS_IP|CCF_DONT_MAKE_RES)|
	old_flags& (CCF_CLASS_IP|CCF_DONT_MAKE_RES);
}
