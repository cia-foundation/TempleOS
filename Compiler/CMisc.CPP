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

I64 OTraceMask(I64 i=0b10001111101)
{//Ctrls which optimizer passes are displayed.
  I64 old=Fs->last_lex->otrace_mask;
  Fs->last_lex->otrace_mask=i;
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

U0 StreamDir()
{
  U8 *dirname;
  if (dirname=DirFile(Fs->last_lex->cur_lfn->full_name)) {
    StreamPrint("\"%s\"",dirname);
    Free(dirname);
  }
}

CIntermediateStruct intermediate_code_table[IC_NUM_ENTRIES]=
{
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"END"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"NOP1"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"END_EXP"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"NOP2"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"LABEL"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"CALL_START"},
{IS_0_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"CALL_END"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"CALL_END2"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"RETURN_VAL"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"RETURN_VAL2"},
{IS_0_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"IMM_I64"},
{IS_0_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"IMM_F64"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"STR_CONST"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"ABS_ADDR"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"ADDR_IMPORT"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"HEAP_GLBL"},
{IS_0_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"SIZEOF"},
{IS_0_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"TYPE"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"GET_LABEL"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"RBP"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"REG"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"FS"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"MOV_FS"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"GS"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"MOV_GS"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"LEA"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"MOV"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"TO_I64"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"TO_F64"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"TO_BOOL"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"TOUPPER"},
{IS_1_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"HOLYC_TYPECAST"},
{IS_1_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"ADDR"},
{IS_1_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"COM"},
{IS_1_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"NOT"},
{IS_1_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"UNARY_MINUS"},
{IS_1_ARG,1,IST_DEREF,FALSE,TRUE,0,0,0,"DEREF"},
{IS_1_ARG,1,IST_DEREF,FALSE,TRUE,0,0,0,"DEREF_PP"},
{IS_1_ARG,1,IST_DEREF,FALSE,TRUE,0,0,0,"DEREF_MM"},
{IS_1_ARG,1,IST_DEREF,TRUE,TRUE,0,0,0,"_PP"},
{IS_1_ARG,1,IST_DEREF,TRUE,TRUE,0,0,0,"_MM"},
{IS_1_ARG,1,IST_DEREF,TRUE,TRUE,0,0,0,"PP_"},
{IS_1_ARG,1,IST_DEREF,TRUE,TRUE,0,0,0,"MM_"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"SHL"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"SHR"},
{IS_1_ARG,1,IST_ASSIGN,FALSE,FALSE,0,0,0,"SHL_CONST"},
{IS_1_ARG,1,IST_ASSIGN,FALSE,FALSE,0,0,0,"SHR_CONST"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"POWER"},
{IS_2_ARG,1,IST_NULL,TRUE,FALSE,0,0,0,"MUL"},
{IS_2_ARG,1,IST_NULL,TRUE,FALSE,0,0,0,"DIV"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"MOD"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"AND"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"OR"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"XOR"},
{IS_2_ARG,1,IST_NULL,TRUE,FALSE,0,0,0,"ADD"},
{IS_2_ARG,1,IST_NULL,TRUE,FALSE,0,0,0,"SUB"},
{IS_1_ARG,1,IST_ASSIGN,FALSE,FALSE,0,0,0,"ADD_CONST"},
{IS_1_ARG,1,IST_ASSIGN,FALSE,FALSE,0,0,0,"SUB_CONST"},
{IS_2_ARG,1,IST_CMP,FALSE,FALSE,0,0,0,"EQUAL_EQUAL"},
{IS_2_ARG,1,IST_CMP,FALSE,FALSE,0,0,0,"NOT_EQUAL"},
{IS_2_ARG,1,IST_CMP,FALSE,FALSE,0,0,0,"LESS"},
{IS_2_ARG,1,IST_CMP,FALSE,FALSE,0,0,0,"GREATER_EQUAL"},
{IS_2_ARG,1,IST_CMP,FALSE,FALSE,0,0,0,"GREATER"},
{IS_2_ARG,1,IST_CMP,FALSE,FALSE,0,0,0,"LESS_EQUAL"},
{IS_0_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"PUSH_CMP"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"AND_AND"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"OR_OR"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"XOR_XOR"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"ASSIGN"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"ASSIGN_PP"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"ASSIGN_MM"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"SHL_EQUAL"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"SHR_EQUAL"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"MUL_EQUAL"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"DIV_EQUAL"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"MOD_EQUAL"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"AND_EQUAL"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"OR_EQUAL"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"XOR_EQUAL"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"ADD_EQUAL"},
{IS_2_ARG,1,IST_ASSIGN,FALSE,TRUE,0,0,0,"SUB_EQUAL"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"JMP"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"SUB_CALL"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"SWITCH"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"NOBOUND_SWITCH"},
{IS_V_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"ADD_RSP"},
{IS_V_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"ADD_RSP1"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"ENTER"},
{IS_0_ARG,0,IST_ASSIGN,FALSE,FALSE,0,0,0,"PUSH_REGS"},
{IS_0_ARG,0,IST_ASSIGN,FALSE,FALSE,0,0,0,"POP_REGS"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"LEAVE"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"RET"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"CALL"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"CALL_INDIRECT"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"CALL_INDIRECT2"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"CALL_IMPORT"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"CALL_EXTERN"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"ASM"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"PREEMPT"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"PUSH"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"POP"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"CLFLUSH"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"INVLPG"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"IN_U8"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"IN_U16"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"IN_U32"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"OUT_U8"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"OUT_U16"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"OUT_U32"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"GET_RFLAGS"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"CARRY"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"SET_RFLAGS"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"GET_RAX"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"SET_RAX"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"GET_RBP"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"SET_RBP"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"GET_RSP"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"SET_RSP"},
{IS_0_ARG,1,IST_NULL,FALSE,TRUE,0,0,0,"RIP"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"RDTSC"},
{IS_0_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"HPET"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BT"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BTS"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BTR"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BTC"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"LBTS"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"LBTR"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"LBTC"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BSF"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BSR"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"QUE_INIT"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"QUE_INS"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"QUE_INS_REV"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"QUE_REM"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"STRLEN"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_ZERO"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_NOT_ZERO"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_CARRY"},
{IS_0_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_NOT_CARRY"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_EQUAL_EQUAL"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_NOT_EQUAL"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_LESS"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_GREATER_EQUAL"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_GREATER"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_LESS_EQUAL"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"BR_2EQUAL_EQUAL"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"BR_2NOT_EQUAL"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"BR_2LESS"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"BR_2GREATER_EQUAL"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"BR_2GREATER"},
{IS_2_ARG,1,IST_NULL,FALSE,FALSE,0,0,0,"BR_2LESS_EQUAL"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_AND_ZERO"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_AND_NOT_ZERO"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BR_MM_ZERO"},
{IS_1_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BR_MM_NOT_ZERO"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_AND_AND_ZERO"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_AND_AND_NOT_ZERO"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_OR_OR_ZERO"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"BR_OR_OR_NOT_ZERO"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BR_BT"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BR_BTS"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BR_BTR"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BR_BTC"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BR_NOT_BT"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BR_NOT_BTS"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BR_NOT_BTR"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"BR_NOT_BTC"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"SWAP_U8"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"SWAP_U16"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"SWAP_U32"},
{IS_2_ARG,0,IST_NULL,FALSE,TRUE,0,0,0,"SWAP_U64"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"ABS_I64"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"SIGN_I64"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"MIN_I64"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"MIN_U64"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"MAX_I64"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"MAX_U64"},
{IS_2_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"MOD_U64"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"SQRI64"},
{IS_1_ARG,0,IST_NULL,FALSE,FALSE,0,0,0,"SQRU64"},
{IS_1_ARG,1,IST_NULL,TRUE,FALSE,0,0,0,"SQR"},
{IS_1_ARG,1,IST_NULL,TRUE,FALSE,0,0,0,"ABS"},
{IS_1_ARG,1,IST_NULL,TRUE,FALSE,0,0,0,"SQRT"},
{IS_1_ARG,1,IST_NULL,TRUE,FALSE,0,0,0,"SIN"},
{IS_1_ARG,1,IST_NULL,TRUE,FALSE,0,0,0,"COS"},
{IS_1_ARG,1,IST_NULL,TRUE,FALSE,0,0,0,"TAN"},
{IS_1_ARG,1,IST_NULL,TRUE,FALSE,0,0,0,"ATAN"},
};

U0 ICClassPut(CHashClass *c)
{
  I64 i;
  if (!c) return;
  if (c->ptr_stars_cnt>4) {
    PrintErr("put_class ptrcnt=%d\n",c->ptr_stars_cnt);
    while (TRUE)
//TODO:
      Yield;
  }
  for (i=0;i<c->ptr_stars_cnt;i++)
    '*';
  c-=c->ptr_stars_cnt;
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

U0 ICPut(CLex *lx,CIntermediateCode *tempi)
{
  I64 opcode=tempi->ic_code,i;
  if (opcode>=IC_END_EXP && opcode!=IC_NOP2) {
    "%15ts %016X ",intermediate_code_table[opcode].name,tempi->ic_data;
    if (lx->pass) {
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
    }
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
      "DELPREV ";
    if (tempi->ic_flags & ICF_PREVIOUS_DELETED)
      "PREVDEL ";
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
      ICPut(lx,tempi);
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
      if (Bt(&lx->otrace_mask,pass-1)) {
	"$$IV,1$$Pass %d:$$IV,0$$\n",pass-1;
	LCCQuePut(lx);
      }
    } else if (Bt(&lx->otrace_mask,pass))
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
  lx->pass=0;
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
  OptPass789A(lx,reg_offsets,NULL,NULL);
  LCCHeaderPut(lx,8,FALSE);
  OptPass789A(lx,reg_offsets,NULL,NULL);
  LCCHeaderPut(lx,9,FALSE);
  code_size=OptPass789A(lx,reg_offsets,NULL,NULL);
  do {
    last_code_size=code_size;
    LCCHeaderPut(lx,9,FALSE);
    code_size=OptPass789A(lx,reg_offsets,NULL,NULL);
    if (code_size>last_code_size) {
      "Pass:9 Code Size\n";
      LexExcept(lx,"Compiler Optimization Error at ");
    }
  } while (code_size<last_code_size);

  if (lx->flags&LF_AOT_COMPILE)
    result=MAlloc(code_size);
  else
    result=MAlloc(code_size,Fs->code_heap);
  LCCHeaderPut(lx,10,FALSE);
  code_size=OptPass789A(lx,reg_offsets,result,_dbg);

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
