U0 LexPutToken(CCmpCtrl *cc)
{//Print cur token to StdOut. (Crude)
  '"';
  if (cc->token==TK_IDENT || cc->token==TK_STR)
    "%s",cc->cur_str;
  else if (cc->token==TK_I64)
    "INT:%X",cc->cur_i64;
  else if (cc->token==TK_CHAR_CONST)
    "U8:%X",cc->cur_i64;
  else if (cc->token==TK_F64)
    "FLOAT:%e",cc->cur_f64;
  else if (cc->token=='\n')
    "<NEW_LINE>";
  else if (Bt(char_bmp_displayable,cc->token))
    '' cc->token;
  else {
    "T:%X",cc->token;
  }
  "\" ";
}

U8 *LexPutLine(CCmpCtrl *cc,U8 *start)
{//Print cur pos to end of line to StdOut.
  I64 ch;
  U8 *ptr;
  if (!start) return NULL;
  if (cc->lex_include_stk->flags&LFSF_DOC)
    return DocPutLine(cc->lex_include_stk->doc,start);
  else {
    ptr=start;
    while (ch=*ptr++) {
      if (ch=='\n') {
	if (*ptr=='\r')
	  ptr++;
	break;
      } else if (ch=='\r') {
	if (*ptr=='\n')
	  ptr++;
	break;
      }
      '' ch;
    }
    if (!ch) ptr--;
    '\n';
    return ptr;
  }
  return NULL;
}

U0 LexPutPos(CCmpCtrl *cc)
{//Print token, line link and, then, $LK,"LexPutLine",A="MN:LexPutLine"$().
  LexPutToken(cc);
  FixSet(cc->lex_include_stk->full_name,cc->lex_include_stk->line_num);
  if (IsRaw)
    "%s,%d ",cc->lex_include_stk->full_name,cc->lex_include_stk->line_num;
  else {
    PutFileLink(cc->lex_include_stk->full_name,,cc->lex_include_stk->line_num);
    AdamErr("%s,%d\n",
	  cc->lex_include_stk->full_name,cc->lex_include_stk->line_num);
    '' CH_SPACE;
  }
  LexPutLine(cc,cc->lex_include_stk->line_start);
}

U0 LexWarn(CCmpCtrl *cc,U8 *str=NULL)
{//Print warn msg, then, $LK,"LexPutPos",A="MN:LexPutPos"$().
  if (str) PrintWarn(str);
  if (cc->htc.fun) {
    "in fun '%s'.\n",cc->htc.fun->str;
    if (IsRaw)
      "%s\n",cc->htc.fun->src_link;
    else {
      "$$LK,\"%s\"$$\n",cc->htc.fun->src_link;
      AdamErr("%s\n",cc->htc.fun->src_link);
    }
  } else
    LexPutPos(cc);
  cc->warning_cnt++;
}

U0 LexExcept(CCmpCtrl *cc,U8 *str=NULL)
{//Print err msg, $LK,"LexPutPos",A="MN:LexPutPos"$() and throw exception.
  if (!Bt(&sys_run_level,RLf_ADAM_SERVER)) {
    Raw(ON);
    "Note: Still in boot phase.\n";
  }
  if (str) PrintErr(str);
  if (!IsRaw)
    AdamErr("Task:%08X %s\n",Fs,str);
  LexPutPos(cc);
  cc->error_cnt++;
  FlushMsgs;
  if (!Bt(&sys_run_level,RLf_ADAM_SERVER))
    Dbg("Type \"Fix;\"");
  throw('Compiler');
}

U0 UndefinedExtern()
{
  PrintErr("Undefined Extern\nat %P\n",Caller);
  throw('UndefExt');
}

U0 UnusedExternWarning(CCmpCtrl *cc,CHashClass *tmpc)
{
  PrintWarn("Unused extern '%s'\n",tmpc->str);
  cc->warning_cnt++;
}

U0 ParenWarning(CCmpCtrl *cc)
{
  if (Bt(&cc->opts,OPTf_WARN_PAREN) &&
	!(cc->lex_include_stk->flags&LFSF_DEFINE))
    LexWarn(cc,"Unnecessary parenthesis ");
}

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

U0 ICArgPut(CICArg *a,I64 type_pointed_to)
{
  if (type_pointed_to)
    "[%Z](%Z) %Z ",a->type.raw_type,"ST_RAW_TYPES",
	  type_pointed_to,"ST_RAW_TYPES",Bsr(a->type>>8)+1,"ST_TY_TYPES";
  else
    "%Z %Z ",a->type.raw_type,"ST_RAW_TYPES",
	  Bsr(a->type>>8)+1,"ST_TY_TYPES";
  switch (Bsr(a->type)) {
    case MDf_STK:
      "STK";
      break;
    case MDf_IMM:
      "#%X",a->disp;
      break;
    case MDf_REG:
      "%Z",a->reg,"ST_U64_REGS";
      break;
    case MDf_DISP:
      "%X[%Z]",a->disp,a->reg,"ST_U64_REGS";
      break;
    case MDf_RIP_DISP32:
      "[%X]",a->disp;
      break;
    case MDf_SIB:
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
  '' CH_SPACE;
}

U0 ICPut(CCmpCtrl *cc,CIntermediateCode *tmpi)
{
  I64 opcode=tmpi->ic_code,i;
  if (opcode>=IC_END_EXP && opcode!=IC_NOP2) {
    "%15ts %016X ",intermediate_code_table[opcode].name,tmpi->ic_data;
    if (cc->pass) {
      if (tmpi->res.type.mode) {
	"$$PURPLE$$RES:$$FG$$";
	ICArgPut(&tmpi->res,0);
      }
      if (tmpi->arg1.type.mode) {
	"$$PURPLE$$ARG1:$$FG$$";
	if (intermediate_code_table[tmpi->ic_code].type==IST_DEREF ||
	      intermediate_code_table[tmpi->ic_code].type==IST_ASSIGN)
	  ICArgPut(&tmpi->arg1,tmpi->arg1_type_pointed_to);
	else
	  ICArgPut(&tmpi->arg1,0);
      }
      if (tmpi->arg2.type.mode) {
	"$$PURPLE$$ARG2:$$FG$$";
	ICArgPut(&tmpi->arg2,0);
      }
      "$$PURPLE$$:$$FG$$";
    }
    ICClassPut(tmpi->ic_class);
    if (tmpi->ic_flags & ICF_LOCK)
      "$$BROWN$$lock$$FG$$ ";
    if (tmpi->ic_flags & ICF_ARG2_TO_F64)
      "$$LTBLUE$$a2d$$FG$$ ";
    if (tmpi->ic_flags & ICF_ARG2_TO_INT)
      "$$GREEN$$a2i$$FG$$ ";
    if (tmpi->ic_flags & ICF_ARG1_TO_F64)
      "$$LTBLUE$$a1d$$FG$$ ";
    if (tmpi->ic_flags & ICF_ARG1_TO_INT)
      "$$GREEN$$a1i$$FG$$ ";
    if (tmpi->ic_flags & ICF_RES_TO_F64)
      "$$LTBLUE$$rd$$FG$$ ";
    if (tmpi->ic_flags & ICF_RES_TO_INT)
      "$$GREEN$$ri$$FG$$ ";
    if (tmpi->ic_flags & ICF_USE_F64)
      "[F64] ";
    if (tmpi->ic_flags & ICF_USE_UNSIGNED)
      "[unsigned] ";
    if (tmpi->ic_flags & ICF_USE_INT)
      "[int] ";
    if (tmpi->ic_flags & ICF_RES_NOT_USED)
      "NO_RES ";
    if (tmpi->ic_flags & ICF_BY_VAL)
      "BY_VAL ";
    if (tmpi->ic_flags & ICF_PUSH_RES)
      "PUSH ";
    if (tmpi->ic_flags & ICF_PUSH_CMP)
      "PUSH_CMP ";
    if (tmpi->ic_flags & ICF_POP_CMP)
      "POP_CMP ";
    if (tmpi->ic_flags & ICF_DEL_PREV_INS)
      "DEL_PREV ";
    if (tmpi->ic_flags & ICF_PREV_DELETED)
      "PREV_DEL ";
    for (i=0;i<3;i++) {
      if (Bt(&tmpi->ic_flags,ICf_DONT_PUSH_FLOAT0+i))
	"DONT_PUSH#%d ",i;
      if (Bt(&tmpi->ic_flags,ICf_DONT_POP_FLOAT0+i))
	"DONT_POP#%d ",i;
    }
    if (tmpi->ic_flags & ICF_ALT_TEMPLATE)
      "ALT_TMP ";
    '\n';
  }
}
