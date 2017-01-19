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
  CHashClass *tmpc=PrsPop(ps);
  if (i.u16[0]!=IC_DEREF) {
    PrsPush(ps,tmpc);
    PrsPush(ps,i);
  }
}

I64 PrsKeyWord(CCmpCtrl *cc)
{//Cvt cur token to $LK,"KEYWORD",A="FF:::/Compiler/OpCodes.DD,KEYWORD"$ or -1.
  CHashGeneric *tmph;
  if (cc->token==TK_IDENT &&(tmph=cc->hash_entry) && tmph->type&HTT_KEYWORD)
    return tmph->user_data0;
  else
    return -1;
}

CHashClass *PrsClassNew()
{/*Ptrs to classes are handled by
allocating 5 structures for each
new class and representing a pointer
to a class by advancing 1 struct fwd
for one * and two fwd for two **.
*/
  I64 i;
  CHashClass *res=CAlloc(sizeof(CHashClass)*(PTR_STARS_NUM+1),Fs->code_heap),
	*tmpc=res;
  for (i=0;i<=PTR_STARS_NUM;i++) {
    tmpc->type=HTT_CLASS;
    tmpc->raw_type=RT_PTR;
    tmpc->size=sizeof(U8 *);
    tmpc->ptr_stars_cnt=i;
    tmpc++;
  }
  res->last_in_member_lst=&res->member_lst_and_root;
  res->size=0;
  return res;
}

CHashFun *PrsFunNew()
{
  I64 i;
  CHashFun *res=CAlloc(sizeof(CHashFun)*(PTR_STARS_NUM+1),Fs->code_heap),
	*tmpf=res;
  for (i=0;i<=PTR_STARS_NUM;i++) {
    tmpf->type=HTT_FUN;
    tmpf->raw_type=RT_PTR;
    tmpf->size=sizeof(U8 *);
    tmpf->ptr_stars_cnt=i;
    tmpf++;
  }
  res->last_in_member_lst=&res->member_lst_and_root;
  res->size=0;
  return res;
}

CIntermediateCode *ICAdd(CCmpCtrl *cc,
   I64 opcode_and_precedence,I64 arg, CHashClass *c,I64 flags=0)
{
  CIntermediateCode *tmpi=MAlloc(sizeof(CIntermediateCode));
  tmpi->ic_code=opcode_and_precedence.u16[0];
  tmpi->ic_precedence=opcode_and_precedence.u16[1];
  tmpi->ic_data=arg;
  tmpi->ic_class=c;
  if (cc->pass_trace) {
    Bts(&cc->flags,CCf_PASS_TRACE_PRESENT);
    flags|=ICF_PASS_TRACE;
  }
  if (cc->lock_cnt)
    flags|=ICF_LOCK;
  tmpi->ic_flags=flags;
  tmpi->ic_line=cc->last_line_num;
  QueIns(tmpi,cc->coc.coc_head.last);
  return tmpi;
}

U0 COCInit(CCmpCtrl *cc)
{
  CCodeCtrl *tmpcbh=&cc->coc;
  QueInit(&tmpcbh->coc_head.next);
  QueInit(&tmpcbh->coc_next_misc);
  tmpcbh->coc_head.ic_code=IC_END;
}

U0 COCPush(CCmpCtrl *cc)
{
  CCodeCtrl *tmpcbh=MAlloc(sizeof(CCodeCtrl));
  MemCpy(tmpcbh,&cc->coc,sizeof(CCodeCtrl));
  cc->coc.coc_next=tmpcbh;
}

CCmpCtrl *COCPopNoFree(CCmpCtrl *cc)
{
  CCodeCtrl *tmpcbh=cc->coc.coc_next;
  MemCpy(&cc->coc,tmpcbh,sizeof(CCodeCtrl));
  return tmpcbh;
}

U0 COCPop(CCmpCtrl *cc)
{
  Free(COCPopNoFree(cc));
}

U0 COCAppend(CCmpCtrl *cc, CCodeCtrl *tmpcbh)
{
  if (tmpcbh->coc_head.next!=&cc->coc.coc_head.next) {
    cc->coc.coc_head.last->next=tmpcbh->coc_head.next;
    tmpcbh->coc_head.next->last=cc->coc.coc_head.last;
    cc->coc.coc_head.last=tmpcbh->coc_head.last;
    tmpcbh->coc_head.last->next=&cc->coc.coc_head.next;
  }
  if (tmpcbh->coc_next_misc!=&cc->coc.coc_next_misc) {
    cc->coc.coc_last_misc->next=tmpcbh->coc_next_misc;
    tmpcbh->coc_next_misc->last=cc->coc.coc_last_misc;
    cc->coc.coc_last_misc=tmpcbh->coc_last_misc;
    tmpcbh->coc_last_misc->next=&cc->coc.coc_next_misc;
  }
  Free(tmpcbh);
}

CCodeMisc *COCMiscNew(CCmpCtrl *cc,I64 ty)
{
  CCodeMisc *res=CAlloc(sizeof(CCodeMisc));
  res->addr=INVALID_PTR;
  res->type=ty;
  QueIns(res,cc->coc.coc_last_misc);
  return res;
}

CCodeMisc *COCGoToLabelFind(CCmpCtrl *cc,U8 *name)
{
  CCodeMisc *cm=cc->coc.coc_next_misc;
  while (cm!=&cc->coc.coc_next_misc) {
    if ((cm->type==CMT_GOTO_LABEL||cm->type==CMT_ASM_LABEL) &&
	  !StrCmp(cm->str,name))
      return cm;
    cm=cm->next;
  }
  return NULL;
}

I64 COCFloatConstFind(CCmpCtrl *cc,F64 d)
{
  I64 i;
  CCodeMisc *cm=cc->coc.coc_next_misc;
  while (cm!=&cc->coc.coc_next_misc) {
    if (cm->type==CMT_FLOAT_CONSTS) {
      for (i=0;i<cm->num_consts;i++)
	if (cm->float_consts[i]==d)
	  return cm->addr+i*sizeof(F64);
      if (cm->num_consts<CM_CONSTS_NUM)  {
	cm->float_consts[cm->num_consts++]=d;
	return cm->addr+i*sizeof(F64);
      }
    }
    cm=cm->next;
  }
  cm=COCMiscNew(cc,CMT_FLOAT_CONSTS);
  cm->float_consts=MAlloc(CM_CONSTS_NUM*sizeof(F64));
  cm->float_consts[cm->num_consts++]=d;
  return cm->addr;
}

U0 COCDel(CCmpCtrl *cc,CCodeCtrl *coc)
{
  CCodeMisc *cm,*cm1;
  U8 *undef=NULL;
  QueDel(&coc->coc_head.next);
  cm=coc->coc_next_misc;
  while (cm!=&coc->coc_next_misc) {
    cm1=cm->next;
    switch (cm->type) {
      case CMT_GOTO_LABEL:
      case CMT_ASM_LABEL:
	if (!(cm->flags&CMF_DEFINED)) {
	  undef=cm->str;
	  cm->str=NULL;
	} else if (!cm->use_cnt) {
	  PrintWarn("Unused label %s\n",cm->str);
	  LexWarn(cc,"Unused label at ");
	}
	break;
      case CMT_JMP_TABLE:
	Free(cm->jmp_table);
	break;
      case CMT_FLOAT_CONSTS:
	Free(cm->float_consts);
	break;
      case CMT_ARRAY_DIM:
	LinkedLstDel(cm->dim);
	break;
      case CMT_HASH_ENTRY:
	HashDel(cm->h);
	break;
    }
    Free(cm->str);
    Free(cm);
    cm=cm1;
  }
  if (undef) {
    PrintErr("Undefined goto label %s\n",undef);
    Free(undef);
    LexExcept(cc,"Undefined goto label at ");
  }
}

U0 COCHeaderPut(CCmpCtrl *cc,I64 pass,Bool put)
{
  CIntermediateCode *tmpi;
  if (Bt(&cc->flags,CCf_PASS_TRACE_PRESENT)) {
    if (put) {
      if (Bt(&cc->saved_pass_trace,pass-1)) {
	"$$IV,1$$Pass %d:$$IV,0$$\n",pass-1;
	tmpi=cc->coc.coc_head.next;
	while (tmpi->ic_code) {
	  if (tmpi->ic_flags&ICF_PASS_TRACE)
	    ICPut(cc,tmpi);
	  tmpi=tmpi->next;
	}
      }
    } else if (Bt(&cc->saved_pass_trace,pass))
      "$$IV,1$$Pass %d:$$IV,0$$\n",pass;
  }
  cc->pass=pass;
}

U8 *COCCompile(CCmpCtrl *cc,I64 *_code_size,CDbgInfo **_dbg,I64 *_type)
{
  U8 *res;
  CCodeMisc *lb;
  I64 i,code_size,last_code_size;

  COptReg reg_offsets[REG_REGS_NUM];
  if (_dbg) *_dbg=NULL;
  cc->pass=0;
  COCHeaderPut(cc,1,TRUE);
  OptPass012(cc);
  COCHeaderPut(cc,2,TRUE);
  OptPass012(cc);
  COCHeaderPut(cc,3,TRUE);
  OptPass3(cc,reg_offsets);
  COCHeaderPut(cc,4,TRUE);
  OptPass4(cc,reg_offsets,_type);
  COCHeaderPut(cc,5,TRUE);
  OptPass5(cc);
  COCHeaderPut(cc,6,TRUE);
  OptPass6(cc);
  COCHeaderPut(cc,7,TRUE);

  lb=cc->coc.coc_next_misc;
  while (lb!=&cc->coc.coc_next_misc) {
    if (lb->type==CMT_JMP_TABLE) {
      for (i=0;i<lb->range;i++)
	lb->jmp_table[i]=OptLabelFwd(lb->jmp_table[i]);
      lb->dft=OptLabelFwd(lb->dft);
    }
    lb=lb->next;
  }

  COCHeaderPut(cc,7,FALSE);
  OptPass789A(cc,reg_offsets,NULL,NULL);
  COCHeaderPut(cc,8,FALSE);
  OptPass789A(cc,reg_offsets,NULL,NULL);
  COCHeaderPut(cc,9,FALSE);
  code_size=OptPass789A(cc,reg_offsets,NULL,NULL);
  do {
    last_code_size=code_size;
    COCHeaderPut(cc,9,FALSE);
    code_size=OptPass789A(cc,reg_offsets,NULL,NULL);
    if (code_size>last_code_size) {
      "Pass:9 Code Size\n";
      LexExcept(cc,"Compiler Optimization Error at ");
    }
  } while (code_size<last_code_size);

  if (cc->flags&CCF_AOT_COMPILE)
    res=MAlloc(code_size);
  else {
    res=MAlloc(code_size,Fs->code_heap);
    if (cc->htc.fun)
      Fs->last_fun=cc->htc.fun;
  }
  COCHeaderPut(cc,10,FALSE);
  code_size=OptPass789A(cc,reg_offsets,res,_dbg);

  COCDel(cc,&cc->coc);
  if (Bt(&cc->opts,OPTf_TRACE)) {
    if (cc->flags&CCF_AOT_COMPILE) {
      if (cc->aotc->seg_size==16)
	Un(res,code_size,16);
      else if (cc->aotc->seg_size==64)
	Un(res,code_size,64);
      else
	Un(res,code_size,32);
    } else
      Un(res,code_size,64);
  }
  if (_code_size) *_code_size=code_size;
  cc->saved_pass_trace=cc->pass_trace;
  return res;
}
