Bool OptEqu(I64 num,Bool val)
{//Set compiler option to value.
  return BEqu(&Fs->last_lex->opts,num,val);
}

Bool OptOn(I64 num)
{//Turn on compiler option.
  return OptEqu(num,ON);
}

Bool OptOff(I64 num)
{//Turn off compiler option.
  return OptEqu(num,OFF);
}

Bool GetOpt(I64 num)
{//Get state of compiler option.
  return Bt(&Fs->last_lex->opts,num);
}

Bool OTrace(Bool val)
{//Displays intermediate code after passes through optimization.
  return OptEqu(OPTf_OPT_TRACE,val);
}

I64 OTraceMask(I64 i=0b1001111101)
{//Ctrls which optimizer passes are displayed.
  I64 old=Fs->last_lex->pass_trace_mask;
  Fs->last_lex->pass_trace_mask=i;
  return old;
}

Bool CTrace(Bool val)
{//Displays assembly code output from compiler.
  return OptEqu(OPTf_COMPILE_TRACE,val);
}

Bool Echo(Bool val)
{//Displays text as it is being compiled.
  return OptEqu(OPTf_ECHO,val);
}

I64 IncludeDepth()
{//Returns how many #includes we are in. See $LK,"IncludeDepth",A="FF:::/Demo/Graphics/Camp.CPP,IncludeDepth"$.
  CLex *lx=Fs->last_lex;
  return lx->cur_lfn-&lx->lfns;
}

U0 ICClassPut(CHashClass *c)
{
  I64 i;
  if (!c) return;
  if (c->ptr_cnt>4) {
    PrintErr("put_class ptrcnt=%d\n",c->ptr_cnt);
    while (TRUE)
//TODO:
      Yield;
  }
  for (i=0;i<c->ptr_cnt;i++)
    '*';
  c-=c->ptr_cnt;
  if (c->str)
    "%s",c->str;
  else {
    PrintErr("put_class str=NULL\n");
    while (TRUE)
//TODO:
      Yield;
  }
  '' CH_SPACE;
}

U0 ICArgPut(CICArg *a)
{
  if (a->type_pointed)
    "%Z %Z %Z ",
      a->type&RT_MASK,"ST_RAW_TYPES",
      a->type_pointed&RT_MASK,"ST_RAW_TYPES",
      a->type>>4,"ST_TY_TYPES";
  else
    "%Z %Z ",
      a->type&RT_MASK,"ST_RAW_TYPES",
      a->type>>4,"ST_TY_TYPES";
  switch (a->type>>4) {
    case TY_STK>>4:
      "STK";
      break;
    case TY_IMM>>4:
      "#%X",a->disp;
      break;
    case TY_REG>>4:
      "%Z",a->reg,"ST_U64_REGS";
      break;
    case TY_DISP>>4:
      "%X[%Z]",a->disp,a->reg,"ST_U64_REGS";
      break;
    case TY_RIP_DISP32>>4:
      "[%X]",a->disp;
      break;
    case TY_SIB>>4:
      if (a->disp)
	"%X",a->disp;
      if (a->reg==REG_RIP)
	'[';
      else
	"[%Z+",a->reg&15,"ST_U64_REGS";
      "%Z",a->reg>>8&15,"ST_U64_REGS";
      switch (a->reg>>14) {
	case 0: "]";   break;
	case 1: "*2]"; break;
	case 2: "*4]"; break;
	case 3: "*8]"; break;
      }
      break;
  }
}

U0 ICPut(CIntermediateCode *tempi)
{
  I64 opcode=tempi->ic_code,i;
  if (opcode>=IC_END_EXP && opcode!=IC_NOP2) {
    "%15ts %016X ",
      DefineSub(opcode,"ST_INTERMEDIATE_CODE")+5,
      tempi->ic_data;
    if (tempi->r.type&TY_MASK) {
      ICArgPut(&tempi->r);
      "<-";
    }
    if (tempi->a1.type&TY_MASK)
      ICArgPut(&tempi->a1);
    ',';
    if (tempi->a2.type&TY_MASK)
      ICArgPut(&tempi->a2);
    '' CH_SPACE;
    ICClassPut(tempi->ic_class);
    if (tempi->ic_flags & ICF_LOCK)
      "$$BROWN$$lock$$FG$$ ";
    if (tempi->ic_flags & ICF_A2_TO_F64)
      "$$LTBLUE$$a2d$$FG$$ ";
    if (tempi->ic_flags & ICF_A2_TO_INT)
      "$$GREEN$$a2i$$FG$$ ";
    if (tempi->ic_flags & ICF_A1_TO_F64)
      "$$LTBLUE$$a1d$$FG$$ ";
    if (tempi->ic_flags & ICF_A1_TO_INT)
      "$$GREEN$$a1i$$FG$$ ";
    if (tempi->ic_flags & ICF_R_TO_F64)
      "$$LTBLUE$$rd$$FG$$ ";
    if (tempi->ic_flags & ICF_R_TO_INT)
      "$$GREEN$$ri$$FG$$ ";
    if (tempi->ic_flags & ICF_USE_F64)
      "[F64] ";
    if (tempi->ic_flags & ICF_USE_UNSIGNED)
      "[unsigned] ";
    if (tempi->ic_flags & ICF_USE_INT)
      "[int] ";
    if (tempi->ic_flags & ICF_NO_DEPEND_RESULT)
      "-DEP ";
    if (tempi->ic_flags & ICF_DEPEND_RESULT)
      "+DEP ";
    if (tempi->ic_flags & ICF_NOT_ADDR)
      "NOT_ADDR ";
    if (tempi->ic_flags & ICF_PUSH_RESULT)
      "PUSH ";
    if (tempi->ic_flags & ICF_PUSH_CMP)
      "PUSH_CMP ";
    if (tempi->ic_flags & ICF_POP_CMP)
      "POP_CMP ";
    if (tempi->ic_flags & ICF_DEL_PREVIOUS_INS)
      "DELINS ";
    for (i=0;i<3;i++) {
      if (Bt(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0+i))
	"DONT_PUSH#%d ",i;
      if (Bt(&tempi->ic_flags,ICf_DONT_POP_FLOAT0+i))
	"DONT_POP#%d ",i;
    }
    if (tempi->ic_flags & ICF_ALTERNATE_TEMPLATE)
      "ALT_TEMP ";
    '\n';
  }
}

U0 LCCQuePut(CLex *lx)
{
  CIntermediateCode *tempi=lx->lcc.lcc_root.next;
  while (tempi->ic_code) {
    if (tempi->ic_flags&ICF_TRACE)
      ICPut(tempi);
    tempi=tempi->next;
  }
}

U0 LCCInit(CLex *lx)
{
  CLexCodeCtrl *tempcbh=&lx->lcc;
  QueInit(&tempcbh->lcc_root.next);
  QueInit(&tempcbh->lcc_next_misc);
  tempcbh->lcc_root.ic_code=IC_END;
}

CIntermediateCode *LCCAdd(CLex *lx,
   I64 opcode_and_precedence,I64 arg, CHashClass *c,I64 flags=0)
{
  CIntermediateCode *tempi=MAlloc(sizeof(CIntermediateCode));
  tempi->ic_code=opcode_and_precedence.u16[0];
  tempi->ic_precedence=opcode_and_precedence.u16[1];
  tempi->ic_data=arg;
  tempi->ic_class=c;
  if (Bt(&lx->opts,OPTf_OPT_TRACE)) {
    Bts(&lx->flags,Lf_OPT_TRACE_PRESENT);
    flags|=ICF_TRACE;
  }
  if (lx->lock_cnt)
    flags|=ICF_LOCK;
  tempi->ic_flags=flags;
  tempi->ic_line=lx->last_line_num;
  QueIns(tempi,lx->lcc.lcc_root.last);
  return tempi;
}

U0 LCCPush(CLex *lx)
{
  CLexCodeCtrl *tempcbh=MAlloc(sizeof(CLexCodeCtrl));
  MemCpy(tempcbh,&lx->lcc,sizeof(CLexCodeCtrl));
  lx->lcc.lcc_next=tempcbh;
}

CLex *LCCPopNoFree(CLex *lx)
{
  CLexCodeCtrl *tempcbh=lx->lcc.lcc_next;
  MemCpy(&lx->lcc,tempcbh,sizeof(CLexCodeCtrl));
  return tempcbh;
}

U0 LCCPop(CLex *lx)
{
  Free(LCCPopNoFree(lx));
}

U0 LCCAppend(CLex *lx, CLexCodeCtrl *tempcbh)
{
  if (tempcbh->lcc_root.next!=&lx->lcc.lcc_root.next) {
    lx->lcc.lcc_root.last->next=tempcbh->lcc_root.next;
    tempcbh->lcc_root.next->last=lx->lcc.lcc_root.last;
    lx->lcc.lcc_root.last=tempcbh->lcc_root.last;
    tempcbh->lcc_root.last->next=&lx->lcc.lcc_root.next;
  }
  if (tempcbh->lcc_next_misc!=&lx->lcc.lcc_next_misc) {
    lx->lcc.lcc_last_misc->next=tempcbh->lcc_next_misc;
    tempcbh->lcc_next_misc->last=lx->lcc.lcc_last_misc;
    lx->lcc.lcc_last_misc=tempcbh->lcc_last_misc;
    tempcbh->lcc_last_misc->next=&lx->lcc.lcc_next_misc;
  }
  Free(tempcbh);
}

CLexCodeMisc *LCCMiscNew(CLex *lx,I64 ty)
{
  CLexCodeMisc *result=CAlloc(sizeof(CLexCodeMisc));
  result->addr=INVALID_PTR;
  result->type=ty;
  QueIns(result,lx->lcc.lcc_last_misc);
  return result;
}

CLexCodeMisc *LCCGoToLabelFind(CLex *lx,U8 *name)
{
  CLexCodeMisc *mc=lx->lcc.lcc_next_misc;
  while (mc!=&lx->lcc.lcc_next_misc) {
    if ((mc->type==CMT_GOTO_LABEL||mc->type==CMT_ASM_LABEL) &&
	!StrCmp(mc->str,name))
      return mc;
    mc=mc->next;
  }
  return NULL;
}

I64 LCCFloatConstFind(CLex *lx,F64 d)
{
  I64 i;
  CLexCodeMisc *mc=lx->lcc.lcc_next_misc;
  while (mc!=&lx->lcc.lcc_next_misc) {
    if (mc->type==CMT_FLOAT_CONSTS) {
      for (i=0;i<mc->num_consts;i++)
	if (mc->float_consts[i]==d)
	  return mc->addr+i*sizeof(F64);
      if (mc->num_consts<CM_MAX_CONSTS)  {
	mc->float_consts[mc->num_consts++]=d;
	return mc->addr+i*sizeof(F64);
      }
    }
    mc=mc->next;
  }
  mc=LCCMiscNew(lx,CMT_FLOAT_CONSTS);
  mc->float_consts=MAlloc(CM_MAX_CONSTS*sizeof(F64));
  mc->float_consts[mc->num_consts++]=d;
  return mc->addr;
}

U0 LCCDel(CLex *lx,CLexCodeCtrl *lcc)
{
  CLexCodeMisc *mc,*mc1;
  U8 *undef=NULL;
  QueDel(&lcc->lcc_root.next);
  mc=lcc->lcc_next_misc;
  while (mc!=&lcc->lcc_next_misc) {
    mc1=mc->next;
    switch (mc->type) {
      case CMT_GOTO_LABEL:
      case CMT_ASM_LABEL:
	if (!(mc->flags&CMF_DEFINED)) {
	  undef=mc->str;
	  mc->str=NULL;
	} else if (!mc->use_cnt) {
	  PrintWarn("Unused label %s\n",mc->str);
	  LexWarn(lx,"Unused label at ");
	}
	break;
      case CMT_JMP_TABLE:
	Free(mc->jmp_table);
	break;
      case CMT_FLOAT_CONSTS:
	Free(mc->float_consts);
	break;
      case CMT_ARRAY_DIM:
	LinkedLstDel(mc->dim);
	break;
      case CMT_HASH_ENTRY:
	HashDel(mc->h);
	break;
    }
    Free(mc->str);
    Free(mc);
    mc=mc1;
  }
  if (undef) {
    PrintErr("Undefined goto label %s\n",undef);
    Free(undef);
    LexExcept(lx,"Undefined goto label at ");
  }
}

U0 LCCHeaderPut(CLex *lx,I64 pass,Bool put)
{
  if (Bt(&lx->flags,Lf_OPT_TRACE_PRESENT)) {
    if (put) {
      if (Bt(&lx->pass_trace_mask,pass-1)) {
	"$$IV,1$$Pass %d:$$IV,0$$\n",pass-1;
	LCCQuePut(lx);
      }
    } else if (Bt(&lx->pass_trace_mask,pass))
      "$$IV,1$$Pass %d:$$IV,0$$\n",pass;
  }
  lx->pass=pass;
}

U8 *LCCCompile(CLex *lx,I64 *_code_size,CDbgInfo **_dbg,I64 *_type)
{
  U8 *result;
  CLexCodeMisc *lb;
  I64 i,code_size,last_code_size;

  COptReg reg_offsets[NUM_REGS];
  if (_dbg) *_dbg=NULL;
  LCCHeaderPut(lx,1,TRUE);
  OptPass012(lx);
  LCCHeaderPut(lx,2,TRUE);
  OptPass012(lx);
  LCCHeaderPut(lx,3,TRUE);
  OptPass3(lx,reg_offsets);
  LCCHeaderPut(lx,4,TRUE);
  OptPass4(lx,reg_offsets,_type);
  LCCHeaderPut(lx,5,TRUE);
  OptPass5(lx);
  LCCHeaderPut(lx,6,TRUE);
  OptPass6(lx);
  LCCHeaderPut(lx,7,TRUE);

  lb=lx->lcc.lcc_next_misc;
  while (lb!=&lx->lcc.lcc_next_misc) {
    if (lb->type==CMT_JMP_TABLE) {
      for (i=0;i<lb->range;i++)
	lb->jmp_table[i]=OptLabelFwd(lb->jmp_table[i]);
      lb->dft=OptLabelFwd(lb->dft);
    }
    lb=lb->next;
  }

  LCCHeaderPut(lx,7,FALSE);
  OptPass789(lx,reg_offsets,NULL,NULL);
  LCCHeaderPut(lx,8,FALSE);
  code_size=OptPass789(lx,reg_offsets,NULL,NULL);
  do {
    last_code_size=code_size;
    LCCHeaderPut(lx,8,FALSE);
    code_size=OptPass789(lx,reg_offsets,NULL,NULL);
    if (code_size>last_code_size) {
      "Pass:8 Code Size\n";
      LexExcept(lx,"Compiler Optimization Error at ");
    }
  } while (code_size<last_code_size);

  if (lx->flags&LF_AOT_COMPILE)
    result=MAlloc(code_size);
  else
    result=MAlloc(code_size,Fs->code_heap);
  LCCHeaderPut(lx,9,FALSE);
  code_size=OptPass789(lx,reg_offsets,result,_dbg);

  cmp.compiled_bytes+=code_size;
  LCCDel(lx,&lx->lcc);
  if (Bt(&lx->opts,OPTf_COMPILE_TRACE)) {
    if (lx->flags&LF_AOT_COMPILE) {
      if (lx->a->seg_size==16)
	Un(result,code_size,16);
      else if (lx->a->seg_size==64)
	Un(result,code_size,64);
      else
	Un(result,code_size,32);
    } else
      Un(result,code_size,64);
  }
  if (_code_size) *_code_size=code_size;
  return result;
}
