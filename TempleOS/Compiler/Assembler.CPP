U0 AsmUnresolvedAdd(CAsmCtrl *a,U8 *exp,I64 type,I64 ip,I64 rel_ip,
		    CLexUndef *undef_hash,I64 line_num,Bool U8_avail,Bool glbl)
{
  CAsmUnresolvedRef *tempu=MAlloc(sizeof(CAsmUnresolvedRef));
  tempu->exp=exp;
  tempu->type=type;
  tempu->ip=ip;
  tempu->rel_ip=rel_ip;
  tempu->undef_hash=undef_hash;
  tempu->lit=FALSE;
  tempu->line_num=line_num;
  tempu->U8_avail=U8_avail;
  tempu->str=NULL;
  if (glbl) {
    tempu->next=a->glbl_unresolved;
    a->glbl_unresolved=tempu;
    if (undef_hash->hash->type & HTF_IMPORT) {
      tempu->str=StrNew(undef_hash->hash->str);
      if (undef_hash->hash->type & HTF_LIT)
	tempu->lit=TRUE;
    }
  } else {
    tempu->next=a->local_unresolved;
    a->local_unresolved=tempu;
    a->has_locals=TRUE;
  }
}

U0 AsmLineLst(CLex *lx)
{
  CAsmCtrl *a=lx->a;
  CLexFile *tempf=lx->cur_lfn;
  CDocEntry *doc_e=tempf->line_start,*doc_e2;
  if (a->lst && a->last_lfn==tempf && doc_e) {
    if (doc_e2=a->last_line_lsted) {
      if (tempf->flags& LFSF_IS_DOC) {
	while (doc_e2->y<doc_e->y) {
	  Spaces(24-a->display_col);
	  a->display_col=0;
	  "%08X ",a->last_ip;
	  doc_e2=LexPutLine(lx,doc_e2);
	}
      } else {
	while (*doc_e2(U8 *) && doc_e2!=doc_e) {
	  Spaces(24-a->display_col);
	  a->display_col=0;
	  "%08X ",a->last_ip;
	  doc_e2=LexPutLine(lx,doc_e2);
	}
      }
      a->last_line_lsted=doc_e2;
    } else
      a->last_line_lsted=doc_e;
  } else {
    a->last_line_lsted=NULL;
    a->last_lfn=tempf;
  }
  a->last_ip=a->ip;
}

U0 AsmStoreCodeU8(CLex *lx,U8 b)
{
  CAsmCtrl *a=lx->a;
  I64 i=a->ip>>ASM_CODE_BLK_BITS;
  CAsmCodeBlk *tempc=a->code;
  while (i--)
    if (!(tempc=tempc->next)) {
      i=a->ip>>ASM_CODE_BLK_BITS+1;
      tempc=&a->code;
      while (i--) {
	if (!tempc->next)
	  tempc->next=CAlloc(sizeof(CAsmCodeBlk));
	tempc=tempc->next;
      }
      break;
    }
  tempc->body[a->ip++ & (ASM_CODE_BLK_SIZE-1)]=b;
  if (a->lst) {
    if (a->display_col>=24) {
      NewLine;
      a->display_col=0;
    }
    "%02X ",b;
    a->display_col+=3;
  }
  if (a->ip>a->num_code_U8s)
    a->num_code_U8s=a->ip;
}

U0 AsmStoreCodeU32(CLex *lx,U32 d)
{
  CAsmCtrl *a=lx->a;
  I64 i=a->ip>>ASM_CODE_BLK_BITS,
     j=(a->ip+sizeof(U32))>>ASM_CODE_BLK_BITS;
  U32 *_d;
  CAsmCodeBlk *tempc=a->code;
  if (i!=j) {
    for (i=0;i<sizeof(U32);i++)
      AsmStoreCodeU8(lx,d.u8[i]);
    return;
  }
  tempc=a->code;
  while (i--)
    if (!(tempc=tempc->next)) {
      i=a->ip>>ASM_CODE_BLK_BITS+1;
      tempc=&a->code;
      while (i--) {
	if (!tempc->next)
	  tempc->next=CAlloc(sizeof(CAsmCodeBlk));
	tempc=tempc->next;
      }
      break;
    }
  _d=tempc->body+ a->ip&(ASM_CODE_BLK_SIZE-1);
  *_d=d;
  a->ip+=sizeof(U32);
  if (a->lst) {
    if (a->display_col>=24) {
      NewLine;
      a->display_col=0;
    }
    "%08X ",d;
    a->display_col+=9;
  }
  if (a->ip>a->num_code_U8s)
    a->num_code_U8s=a->ip;
}

U0 AsmStoreCodeU64(CLex *lx,U64 d)
{
  CAsmCtrl *a=lx->a;
  I64 i=a->ip>>ASM_CODE_BLK_BITS,
      j=(a->ip+sizeof(I64))>>ASM_CODE_BLK_BITS,
      *qq;
  CAsmCodeBlk *tempc=a->code;
  if (i!=j) {
    for (i=0;i<sizeof(I64);i++)
      AsmStoreCodeU8(lx,d.u8[i]);
    return;
  }
  tempc=a->code;
  while (i--)
    if (!(tempc=tempc->next)) {
      i=a->ip>>ASM_CODE_BLK_BITS+1;
      tempc=&a->code;
      while (i--) {
	if (!tempc->next)
	  tempc->next=CAlloc(sizeof(CAsmCodeBlk));
	tempc=tempc->next;
      }
      break;
    }
  qq=tempc->body+ a->ip&(ASM_CODE_BLK_SIZE-1);
  *qq=d;
  a->ip+=sizeof(I64);
  if (a->lst) {
    if (a->display_col>=24) {
      NewLine;
      a->display_col=0;
    }
    "%016X ",d;
    a->display_col+=17;
  }
  if (a->ip>a->num_code_U8s)
    a->num_code_U8s=a->ip;
}

U0 AsmStoreCodeU8At(CLex *lx,U8 b,I64 ip)
{
  CAsmCtrl *a=lx->a;
  I64 i=ip>>ASM_CODE_BLK_BITS+1;
  CAsmCodeBlk *tempc=&a->code;
  while (i--) {
    if (!tempc->next) {
      tempc->next=CAlloc(sizeof(CAsmCodeBlk));
      tempc->next->next=NULL;
    }
    tempc=tempc->next;
  }
  tempc->body[ip & (ASM_CODE_BLK_SIZE-1)]=b;
  if (ip+1>a->num_code_U8s)
    a->num_code_U8s=ip+1;
}

U0 AsmReserveCodeTo(CLex *lx,I64 ip)
{
  CAsmCtrl *a=lx->a;
  I64 i=ip>>ASM_CODE_BLK_BITS+1;
  CAsmCodeBlk *tempc=&a->code;
  while (i--) {
    if (!tempc->next) {
      tempc->next=CAlloc(sizeof(CAsmCodeBlk));
      tempc->next->next=NULL;
    }
    tempc=tempc->next;
  }
  if (ip>a->num_code_U8s)
    a->num_code_U8s=ip;
}


I64 AsmExpressionEval(CLex *lx)
{
  U8 *exp;
  I64 result=0;
  lx->undef_hash_entry=NULL;
  lx->flags&=~(LF_UNRESOLVED+LF_LOCAL);
  lx->abs_cnts=0;
  if (!GetExpression(lx,&exp))
    LexSkipEol(lx);
  else {
    if (lx->abs_cnts.externs)
      LexExcept(lx,"ST_EXTERNS_NOT_ALLOWED");
    if (lx->flags & LF_UNRESOLVED)
      LexExcept(lx,"ST_UNDEFINED_SYM");
    else {
      result=Call(exp);
      ExpressionDel(exp);
    }
  }
  return result;
}

U0 AsmLocalsResolve(CLex *lx)
{
  CAsmCtrl *a=lx->a;
  CAsmUnresolvedRef *tempu=a->local_unresolved,*tempu1;
  I64 r=0;
  CLexUndef *templu;
  Bool undefined;
  U8 *label=NULL;

  if (!a->has_locals)
    return;
  while (tempu) {
    undefined=FALSE;
    tempu1=tempu->next;
    templu=tempu->undef_hash;
    while (templu) {
      if (templu->hash->type & HTF_UNRESOLVED) {
	PrintFErr("%S%04d %s %s\n","ST_UNDEFINED_SYM_LINE",
	  tempu->line_num,a->last_label,templu->hash->str);
	LexExcept(lx);
      }
      label=templu->hash->str;
      templu=templu->next;
    }
    if (!undefined) {
      r=Call(tempu->exp);
      if (!(tempu->type & 1)) {
	r-=tempu->rel_ip;
	if (tempu->type==EIE_REL_U8 && !(MIN_I8<=r<=MAX_I8) ||
	    tempu->type==EIE_REL_U16 && !(MIN_I16<=r<=MAX_I16) ) {
	  PrintFErr("%S%04d %s %s\n","ST_BRANCH_OUT_OF_RANGE_LINE",
	    tempu->line_num,a->last_label,label);
	  LexExcept(lx);
	}
	if (tempu->U8_avail && tempu->type>EIE_LIT_U8 && -124<=r<=123)
	  PrintFWarn("%S%04d %s %s\n","ST_COULD_USE_I8_DISP_LINE",
	    tempu->line_num,a->last_label,label);
      }
      AsmStoreCodeU8At(lx,r.u8[0],tempu->ip);
      if (tempu->type>=EIE_REL_U16) {
	AsmStoreCodeU8At(lx,r.u8[1],tempu->ip+1);
	if (tempu->type>=EIE_REL_U32) {
	  AsmStoreCodeU8At(lx,r.u8[2],tempu->ip+2);
	  AsmStoreCodeU8At(lx,r.u8[3],tempu->ip+3);
	  if (tempu->type>=EIE_REL_U64) {
	    AsmStoreCodeU8At(lx,r.u8[4],tempu->ip+4);
	    AsmStoreCodeU8At(lx,r.u8[5],tempu->ip+5);
	    AsmStoreCodeU8At(lx,r.u8[6],tempu->ip+6);
	    AsmStoreCodeU8At(lx,r.u8[7],tempu->ip+7);
	  }
	}
      }
    }
    ExpressionDel(tempu->exp);
    LinkedLstDel(tempu->undef_hash);
    Free(tempu);
    tempu=tempu1;
  }
  HashTableDel(lx->local_hash_table);
  lx->local_hash_table=lx->hash_table_lst->next=
    HashTableNew(16);
  lx->local_hash_table->next=lx->glbl_hash_table;
  a->local_unresolved=NULL;
  a->has_locals=FALSE;
}

U0 AsmGlblsResolve(CLex *lx,CExe *ex)
{
  CAsmCtrl *a=lx->a;
  CHashFun *tempf;
  CAsmUnresolvedRef *tempu=a->glbl_unresolved,*tempu1;
  I64 j,r=0;
  CExeImportExport *tempe,*tempe1;
  CLexUndef *templu;
  CHashExport *tempess;
  U8 *label;
  Bool undefined;
  CExternUsage *tempeu,*tempeu8;

  while (tempu) {
    label=NULL;
    undefined=FALSE;
    tempu1=tempu->next;
    templu=tempu->undef_hash;
    while (templu) {
      if (templu->hash->type & HTF_UNRESOLVED) {
	tempess=templu->hash;
	if (tempess->type & HTT_EXPORT_SYS_SYM &&
	    tempess->type & HTF_UNRESOLVED &&
	    !(tempess->type & HTF_IMPORT) &&
	    (tempf=HashFind(tempess->str,lx->hash_table_lst,HTT_FUN)) &&
	    !Bt(tempf->flags,Cf_EXTERN)) {
	  tempess->val=tempf->executable_addr;
	  tempess->type&=~HTF_UNRESOLVED;
	  label=templu->hash->str;
	} else {
	  if (!(tempess->type & HTF_IMPORT)) {
	    PrintFErr("%S%04d %s\n",
	      "ST_UNDEFINED_SYM_LINE",tempu->line_num,tempess->str);
	    LexExcept(lx);
	  }
	  undefined=TRUE;
	}
      } else
	label=templu->hash->str;
      templu=templu->next;
    }
    r=Call(tempu->exp);
    if (!(tempu->type & 1))
      r-=tempu->rel_ip;
    if (undefined) {
      templu=tempu->undef_hash;
      if (templu->hash->type & HTF_IMPORT) {
	tempe=CAlloc(sizeof(CExeImportExport));
	tempe->next=templu->hash->ie_lst;
	templu->hash->ie_lst=tempe;
	tempe->ip=tempu->ip;
	tempe->type=tempu->type;
      }
    } else {
      if (!(tempu->type & 1))  { //Rel?
	if (tempu->type==EIE_REL_U8 && !(MIN_I8<=r<=MAX_I8) ||
	    tempu->type==EIE_REL_U16 && !(MIN_I16<=r<=MAX_I16)) {
	  PrintFErr("%S%04d %s\n","ST_BRANCH_OUT_OF_RANGE_LINE",
	    tempu->line_num,label);
	  LexExcept(lx);
	}
	if (tempu->U8_avail && tempu->type>EIE_LIT_U8 && -124<=r<=123)
	  PrintFWarn("%S%04d %s %s\n","ST_COULD_USE_I8_DISP_LINE",
	    tempu->line_num,a->last_label,label);
      }
    }
    AsmStoreCodeU8At(lx,r.u8[0],tempu->ip);
    if (tempu->type>=EIE_REL_U16) {
      AsmStoreCodeU8At(lx,r.u8[1],tempu->ip+1);
      if (tempu->type>=EIE_REL_U32) {
	AsmStoreCodeU8At(lx,r.u8[2],tempu->ip+2);
	AsmStoreCodeU8At(lx,r.u8[3],tempu->ip+3);
	if (tempu->type>=EIE_REL_U64) {
	  AsmStoreCodeU8At(lx,r.u8[4],tempu->ip+4);
	  AsmStoreCodeU8At(lx,r.u8[5],tempu->ip+5);
	  AsmStoreCodeU8At(lx,r.u8[6],tempu->ip+6);
	  AsmStoreCodeU8At(lx,r.u8[7],tempu->ip+7);
	}
      }
    }
    ExpressionDel(tempu->exp);
    LinkedLstDel(tempu->undef_hash);
    Free(tempu);
    tempu=tempu1;
  }

  tempess=lx->glbl_hash_table->next_added;
  while (tempess!=lx->glbl_hash_table) {
    if (tempess->type & HTF_IMPORT && tempess->use_cnt) {
      if (tempe=tempess->ie_lst) {
	if (tempess->import_name)
	  tempe->str=StrNew(tempess->import_name);
	else
	  tempe->str=StrNew(tempess->str);
	do {
	  tempe1=tempe->next;
	  QueIns(tempe,ex->last_ie);
	} while (tempe=tempe1);
	tempess->ie_lst=NULL;
      }
    } else if (tempess->type & HTF_EXPORT) {
      if (tempess->type & HTF_UNRESOLVED) {
	PrintFErr("%S%s\n","ST_UNDEFINED_SYM",tempess->str);
	LexExcept(lx);
      }
      tempe=CAlloc(sizeof(CExeImportExport));
      tempe->type=EIE_REL_EXPORT;
      if (tempess->type & HTT_FUN) {
	tempf=tempess;
	tempe->ip=tempf->executable_addr;
	tempeu=tempf->ext_lst;
	while (tempeu) {
	  tempeu8=tempeu->next;
	  j=tempf->executable_addr-(tempeu->ip+4);
	  AsmStoreCodeU8At(lx,j.u8[0],tempeu->ip);
	  AsmStoreCodeU8At(lx,j.u8[1],tempeu->ip+1);
	  AsmStoreCodeU8At(lx,j.u8[2],tempeu->ip+2);
	  AsmStoreCodeU8At(lx,j.u8[3],tempeu->ip+3);
	  Free(tempeu);
	  tempeu=tempeu8;
	}
      } else if (tempess->type & HTT_GLBL_VAR)
	tempe->ip=tempess(CHashGlblVar *)->data_addr_ip;
      else
	tempe->ip=tempess->val;
      if (tempess->type & HTF_LIT)
	tempe->type++;
      tempe->str=StrNew(tempess->str);
      tempe->src_link=StrNew(tempess->src_link);
      QueIns(tempe,ex->last_ie);
    }
    tempess=tempess->next_added;
  }
}

Bool PrsAsmImm(CLex *lx,CAsmArg *arg)
{
  if (arg->imm_or_off_present)
    LexExcept(lx,"ST_ALREADY_ONE_IMMEDIATE");
  arg->imm_or_off_present=TRUE;
  arg->num.undef_local=NULL;
  arg->num.undef_glbl=NULL;
  lx->undef_hash_entry=NULL;
  lx->abs_cnts=0;
  lx->flags&=~(LF_UNRESOLVED+LF_LOCAL);
  if (!GetExpression(lx,&arg->num.exp))
    LexSkipEol(lx);
  else {
    if (lx->abs_cnts.externs)
      LexExcept(lx,"ST_EXTERNS_NOT_ALLOWED");
    if (lx->flags & LF_UNRESOLVED) {
      if (lx->flags & LF_LOCAL) {
	arg->num.undef_local=lx->undef_hash_entry;
	lx->undef_hash_entry=NULL;
      } else {
	arg->num.undef_glbl=lx->undef_hash_entry;
	lx->undef_hash_entry=NULL;
	lx->a->has_locals=TRUE;
      }
    } else {
      arg->num.i=Call(arg->num.exp);
      arg->num.undef_glbl=lx->undef_hash_entry;
      lx->undef_hash_entry=NULL;
      ExpressionDel(arg->num.exp);
      arg->num.exp=NULL;
    }
  }
  return TRUE;
}

U0 PrsAsmArg(CLex *lx,CAsmArg *arg)
{
  CHashGeneric *temph;
  CHashReg *tempr;
  MemSet(arg,0,sizeof(CAsmArg));
  arg->seg=CREG_NONE;
  arg->reg1=CREG_NONE;
  arg->reg2=CREG_NONE;
  arg->fstk=CREG_NONE;
  arg->scale=1;
  while (TRUE) {
    if (lx->token==TK_IDENT) {
      if (temph=lx->hash_entry) {
	if (temph->type&HTT_TYPE_MASK==HTT_REG) {
	  tempr=temph;
	  switch (tempr->reg_type) {
	    case REGT_R8:
	      arg->size=1;
	      arg->reg1=tempr->reg_num;
	      Lex(lx);
	      return;
	    case REGT_R16:
	      arg->size=2;
	      arg->reg1=tempr->reg_num;
	      Lex(lx);
	      return;
	    case REGT_R32:
	      arg->size=4;
	      arg->reg1=tempr->reg_num;
	      Lex(lx);
	      return;
	    case REGT_R64:
	      arg->size=8;
	      arg->reg1=tempr->reg_num;
	      Lex(lx);
	      return;
	    case REGT_SEG:
	      arg->seg=tempr->reg_num;
	      if (Lex(lx)!=':') {
		arg->just_seg=TRUE;
		return;
	      } else
	      Lex(lx); //skip ":"
	      break;
	    case REGT_FSTK:
	      arg->size=8;
	      arg->fstk=tempr->reg_num;
	      Lex(lx);
	      return;
	  }
	} else if (temph->type&HTT_TYPE_MASK==HTT_ASM_KEYWORD) {
	  switch (temph->user_data0) {
	    case AKW_I8:
	    case AKW_U8:
	      arg->size=1;
	      break;
	    case AKW_I16:
	    case AKW_U16:
	      arg->size=2;
	      break;
	    case AKW_I32:
	    case AKW_U32:
	      arg->size=4;
	      break;
	    case AKW_I64:
	    case AKW_U64:
	      arg->size=8;
	      break;
	    default:
	      LexExcept(lx,"ST_SYNTAX_ERROR");
	  }
	Lex(lx); //skip keyword
	} else {
	  PrsAsmImm(lx,arg);
	  arg->num.abs_cnts=lx->abs_cnts;
	  if (lx->token!='[')
	    return;
	}
      } else {
	PrsAsmImm(lx,arg);
	arg->num.abs_cnts=lx->abs_cnts;
	if (lx->token!='[')
	  return;
      }
    } else if (lx->token=='[') {
      arg->indirect=TRUE;
      Lex(lx); // skip [
      while (lx->token && lx->token!=']') {
	if (lx->token==TK_IDENT) {
	  if (temph=lx->hash_entry) {
	    if (temph->type&HTT_TYPE_MASK==HTT_REG &&
	      REGT_R16<=temph(CHashReg *)->reg_type<=REGT_R64) {
	      tempr=temph;
	      if (arg->reg1==CREG_NONE) {
		if (tempr->reg_num&7==CREG_RSP) {
		  arg->reg1=4;
		  arg->reg2=tempr->reg_num;
		} else
		  arg->reg1=tempr->reg_num;
	      } else
		arg->reg2=tempr->reg_num;
	      Lex(lx);
	    } else {
	      PrsAsmImm(lx,arg);
	      arg->num.abs_cnts=lx->abs_cnts;
	    }
	  } else {
	    PrsAsmImm(lx,arg);
	    arg->num.abs_cnts=lx->abs_cnts;
	  }
	} else if (lx->token=='*') {
	  Lex(lx);
	  if (lx->token!=TK_INTEGER)
	    LexExcept(lx,"ST_EXPECTING_SCALE_FACTOR");
	  arg->scale=lx->cur_i;
	Lex(lx); //skip scale
	  if (arg->reg2!=CREG_NONE)
	    SwapU64(&arg->reg1,&arg->reg2);
	} else if (lx->token=='+') {
	Lex(lx); //skip '+'
	} else {
	  PrsAsmImm(lx,arg);
	  arg->num.abs_cnts=lx->abs_cnts;
	}
      }
      if (lx->token!=']')
	LexExcept(lx,"ST_MISSING_RIGHT_BRACKET");
      else {
      Lex(lx); //skip ]
	return;
      }
    } else {
      PrsAsmImm(lx,arg);
      arg->num.abs_cnts=lx->abs_cnts;
      if (lx->token!='[')
	return;
    }
  }
}


U0 AsmMakeArgMask(CLex *lx,CAsmArg *arg,I64 *mask)
{
  CAsmCtrl *a=lx->a;
  if (arg->just_seg) {
    switch (arg->seg) {
      case 0: *mask=1<<ARGT_ES|1<<ARGT_SREG; break;
      case 1: *mask=1<<ARGT_CS|1<<ARGT_SREG; break;
      case 2: *mask=1<<ARGT_SS|1<<ARGT_SREG; break;
      case 3: *mask=1<<ARGT_DS|1<<ARGT_SREG; break;
      case 4: *mask=1<<ARGT_FS|1<<ARGT_SREG; break;
      case 5: *mask=1<<ARGT_GS|1<<ARGT_SREG; break;
    }
    return;
  }
  if (arg->fstk!=CREG_NONE) {
    if (arg->fstk)
      *mask=1<<ARGT_STI;
    else
      *mask=1<<ARGT_ST0|1<<ARGT_STI;
    return;
  }
  switch (arg->size) {
    case 0: *mask=0x3FF0FFFFFE; break;
    case 1: *mask=0x1110111112; break;
    case 2: *mask=0x2220222224; break;
    case 4: *mask=0x0440444448; break;
    case 8: *mask=0x0880888880; break;
  }
  if (a->seg_size==64)
    *mask&=  0xFF0FFFFFFF;

  if (arg->reg1!=CREG_NONE && arg->imm_or_off_present && !arg->num.i) {
    if (!arg->num.undef_glbl && !arg->num.undef_local)
      arg->imm_or_off_present=FALSE;  //Zero displacement
  }

  if (arg->reg2!=CREG_NONE || arg->scale!=1) {
    *mask&=  0x0000FF0000;
    return;
  }

  if (arg->indirect) {
    if (arg->imm_or_off_present)
      *mask&=0x00FFFF0000;
    else
      *mask&=0x000FFF0000;
  } else {
    if (arg->imm_or_off_present)
      *mask&=0x000F00F0FE;
    else
      *mask&=0x3F0FFF0F00;
  }
  if (arg->seg!=CREG_NONE)
    *mask&=  0x00FFFF0000;
  if (arg->reg1==CREG_NONE) {
    if (arg->indirect)
      *mask&=0x00FFFF0000;
    else if (MIN_I8<=arg->num.i<=MAX_I8)
      *mask&=0x000000F0FE;
    else if (MIN_I8<=arg->num.i<=MAX_U8)
      *mask&=0x000000E0FE;
    else if (MIN_I16<=arg->num.i<=MAX_I16)
      *mask&=0x000000E0EE;
    else if (MIN_I16<=arg->num.i<=MAX_U16)
      *mask&=0x000000C0EE;
    else if (MIN_I32<=arg->num.i<=MAX_I32)
      *mask&=0x000000C0CE;
    else if (MIN_I32<=arg->num.i<=MAX_U32)
      *mask&=0x00000080CE;
    else
      *mask&=0x000000808E;
  } else {
    *mask&=  0x3F00FF0F00;
    if (!arg->indirect) //M8-M64
      *mask&=  0xFFFF0FFFFF;
  }
  if (arg->reg1)
    *mask&=  0x30FFFFFFFF;
  if (arg->reg1!=1)
    *mask&=  0x2FFFFFFFFF;
  if (arg->reg1!=2)
    *mask&=  0x1FFFFFFFFF;
}

Bool AsmStoreNum(CLex *lx,CAsmNum2 *num2,I64 cnt,Bool U8_avail)
{
  CAsmCtrl *a=lx->a;
  I64 i;
  CExeAbsAddr *tempa;

  if (!num2->lit)
    num2->num.i-=num2->rel;
  for (i=0;i<cnt;i++) {
    if (num2->U8_cnt==1) {
      if (num2->num.undef_local)
	AsmUnresolvedAdd(a,num2->num.exp,EIE_REL_U8+num2->lit,a->ip,num2->rel,
	  num2->num.undef_local,lx->cur_lfn->line_num,U8_avail,FALSE);
      else if (num2->num.undef_glbl)
	AsmUnresolvedAdd(a,num2->num.exp,EIE_REL_U8+num2->lit,a->ip,num2->rel,
	  num2->num.undef_glbl,lx->cur_lfn->line_num,U8_avail,TRUE);
      else {
	if (!num2->lit && !(MIN_I8<=num2->num.i<=MAX_I8))
	  LexExcept(lx,"ST_BRANCH_OUT_OF_RANGE");
      }
      if (num2->lit) {
	if (num2->num.abs_cnts.abs_addres & 1) {
	  tempa=CAlloc(sizeof(CExeAbsAddr));
	  tempa->next=a->abss;
	  a->abss=tempa;
	  tempa->ip=a->ip;
	  tempa->type=EAA_ADD_U8;
	}
      } else {
	if (num2->num.abs_cnts.c_addres & 1) {
	  tempa=CAlloc(sizeof(CExeAbsAddr));
	  tempa->next=a->abss;
	  a->abss=tempa;
	  tempa->ip=a->ip;
	  tempa->type=EAA_SUB_U8;
	}
      }
      AsmStoreCodeU8(lx,num2->num.i.u8[0]);
    } else if (num2->U8_cnt==2) {
      if (num2->num.undef_local)
	AsmUnresolvedAdd(a,num2->num.exp,EIE_REL_U16+num2->lit,a->ip,num2->rel,
	  num2->num.undef_local,lx->cur_lfn->line_num,U8_avail,FALSE);
      else if (num2->num.undef_glbl)
	AsmUnresolvedAdd(a,num2->num.exp,EIE_REL_U16+num2->lit,a->ip,num2->rel,
	  num2->num.undef_glbl,lx->cur_lfn->line_num,U8_avail,TRUE);
      else if (!num2->lit && !(MIN_I16<=num2->num.i<=MAX_I16))
	LexExcept(lx,"ST_BRANCH_OUT_OF_RANGE");
      if (num2->lit) {
	if (num2->num.abs_cnts.abs_addres & 1) {
	  tempa=CAlloc(sizeof(CExeAbsAddr));
	  tempa->next=a->abss;
	  a->abss=tempa;
	  tempa->ip=a->ip;
	  tempa->type=EAA_ADD_U16;
	}
      } else {
	if (num2->num.abs_cnts.c_addres & 1) {
	  tempa=CAlloc(sizeof(CExeAbsAddr));
	  tempa->next=a->abss;
	  a->abss=tempa;
	  tempa->ip=a->ip;
	  tempa->type=EAA_SUB_U16;
	}
      }
      AsmStoreCodeU8(lx,num2->num.i.u8[0]);
      AsmStoreCodeU8(lx,num2->num.i.u8[1]);
      if (U8_avail && !num2->num.undef_local && !num2->num.undef_glbl &&
	  !num2->lit && -124<=num2->num.i<=123) {
	LexPutWarn(lx,"ST_COULD_USE_I8_DISP");
	return FALSE;
      }
    } else if (num2->U8_cnt==4) {
      if (num2->num.undef_local)
	AsmUnresolvedAdd(a,num2->num.exp,EIE_REL_U32+num2->lit,a->ip,num2->rel,
	  num2->num.undef_local,lx->cur_lfn->line_num,U8_avail,FALSE);
      else if (num2->num.undef_glbl)
	AsmUnresolvedAdd(a,num2->num.exp,EIE_REL_U32+num2->lit,a->ip,num2->rel,
	  num2->num.undef_glbl,lx->cur_lfn->line_num,U8_avail,TRUE);
      if (num2->lit) {
	if (num2->num.abs_cnts.abs_addres & 1) {
	  tempa=CAlloc(sizeof(CExeAbsAddr));
	  tempa->next=a->abss;
	  a->abss=tempa;
	  tempa->ip=a->ip;
	  tempa->type=EAA_ADD_U32;
	}
      } else {
	if (num2->num.abs_cnts.c_addres & 1) {
	  tempa=CAlloc(sizeof(CExeAbsAddr));
	  tempa->next=a->abss;
	  a->abss=tempa;
	  tempa->ip=a->ip;
	  tempa->type=EAA_SUB_U32;
	}
      }
      AsmStoreCodeU32(lx,num2->num.i.u32[0]);
      if (U8_avail && !num2->num.undef_local && !num2->num.undef_glbl &&
	  !num2->lit && -124<=num2->num.i<=123) {
	LexPutWarn(lx,"ST_COULD_USE_I8_DISP");
	return FALSE;
      }
    } else if (num2->U8_cnt==8) {
//TODO
      if (num2->num.undef_local)
	AsmUnresolvedAdd(a,num2->num.exp,EIE_REL_U32+num2->lit,a->ip,num2->rel,
	  num2->num.undef_local,lx->cur_lfn->line_num,U8_avail,FALSE);
      else if (num2->num.undef_glbl)
	AsmUnresolvedAdd(a,num2->num.exp,EIE_REL_U32+num2->lit,a->ip,num2->rel,
	  num2->num.undef_glbl,lx->cur_lfn->line_num,U8_avail,TRUE);
      if (num2->lit) {
	if (num2->num.abs_cnts.abs_addres & 1) {
	  tempa=CAlloc(sizeof(CExeAbsAddr));
	  tempa->next=a->abss;
	  a->abss=tempa;
	  tempa->ip=a->ip;
	  tempa->type=EAA_ADD_U64;
	}
      } else {
	if (num2->num.abs_cnts.c_addres & 1) {
	  tempa=CAlloc(sizeof(CExeAbsAddr));
	  tempa->next=a->abss;
	  a->abss=tempa;
	  tempa->ip=a->ip;
	  tempa->type=EAA_SUB_U64;
	}
      }
      AsmStoreCodeU64(lx,num2->num.i);
      if (U8_avail && !num2->num.undef_local && !num2->num.undef_glbl &&
	  !num2->lit && -124<=num2->num.i<=123) {
	LexPutWarn(lx,"ST_COULD_USE_I8_DISP");
	return FALSE;
      }
    }
  }
  return TRUE;
}

U8 asm_seg_prefixes[6]={0x26,0x2E,0x36,0x3E,0x64,0x65};

Bool AsmCodeInstruction(CLex *lx,
			  CHashOpcode *tempo,I64 argcnt)
{
  CAsmCtrl *a=lx->a;
  I64 i,j,a1,a2,om,seg,arg1mask,arg2mask;
  CAsmArg *tempa1,*tempa2;
  Bool ModrM_complete,U8_avail=FALSE,found_second_possible=FALSE;
  CInstruction *tempi;

  CAsmIns cur,best;
  best.U8_cnt=255;

  if (argcnt>0)
    AsmMakeArgMask(lx,&a->arg1,&arg1mask);
  else
    arg1mask=1;
  if (argcnt>1)
    AsmMakeArgMask(lx,&a->arg2,&arg2mask);
  else
    arg2mask=1;
  for (i=0;i<tempo->instruction_entry_cnt;i++) {
    tempi=&tempo->ins[i];
    if (tempi->arg1==ARGT_REL8 || tempi->arg2==ARGT_REL8)
      U8_avail=TRUE;
    if (Bt(&arg1mask,tempi->arg1) && Bt(&arg2mask,tempi->arg2)) {
      MemSet(&cur,0,sizeof(CAsmIns));
      cur.tempi=tempi;
      ModrM_complete=FALSE;
      cur.is_dft=ToBool(tempi->flags & IEF_DFT);
      if (a->seg_size==64) {
	if (tempi->flags & IEF_48_REX)
	  cur.REX=0x48;
	else if (!(tempi->flags & IEF_NO_REX))
	  cur.REX=0x40;
      }
      cur.disp.lit=1;
      cur.imm.lit=1;
      om=tempi->opcode_modifier;
      a1=tempi->arg1;
      a2=tempi->arg2;
      tempa1=&a->arg1;
      tempa2=&a->arg2;
      cur.last_opcode_U8=tempi->opcode[tempi->opcode_cnt-1];

      if (tempi->slash_val<8) {
	cur.ModrM|=tempi->slash_val<<3;
	cur.has_ModrM=TRUE;
      }

      if (a->seg_size==16 && tempi->flags & IEF_OP_SIZE32 ||
	  a->seg_size!=16 && tempi->flags & IEF_OP_SIZE16)
	cur.has_operand_prefix=TRUE;

      if (om==OM_IB)	  cur.imm.U8_cnt=1;
      else if (om==OM_IW) cur.imm.U8_cnt=2;
      else if (om==OM_ID) cur.imm.U8_cnt=4;

      if (om==OM_CB) {
	cur.imm.U8_cnt=1;
	cur.imm.lit=0;
      } else if (om==OM_CW) {
	cur.imm.U8_cnt=2;
	cur.imm.lit=0;
      } else if (om==OM_CD) {
	cur.imm.U8_cnt=4;
	cur.imm.lit=0;
      }

      if (argcnt==1) {
	if (best.U8_cnt!=255 && !found_second_possible && !best.is_dft) {
	  found_second_possible=TRUE;
	  if (!a->arg1.size)
	    PrintFWarn("%S%s,%04d\n","ST_NO_SIZE_SPECIFIED",
		   lx->cur_lfn->name,lx->cur_lfn->line_num-1);
	}
	if (tempi->flags & IEF_PLUS_OPCODE) {
	  if (tempi->slash_val==SV_R_REGISTER) {
	    cur.last_opcode_U8|=tempa1->reg1&7;
	    if (tempa1->reg1&15>7)
	      cur.REX|=1;
	    if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
	      cur.has_REX=TRUE;
	  } else { //SV_I_REGISTER
	    if (tempa1->fstk!=CREG_NONE)
	      cur.last_opcode_U8+=tempa1->fstk;
	  }
	}
	if (a1==ARGT_R64 || a1==ARGT_RM64 || a1==ARGT_M64)
	  cur.REX|=8;
	if (ARGT_RM8<=a1<=ARGT_RM64 || ARGT_M8<=a1<=ARGT_M64) {
	  if (a->seg_size==16)
	    cur.has_addr_prefix=TRUE;

	  cur.has_ModrM=TRUE;
	  if (tempa1->imm_or_off_present && tempa1->indirect && tempa1->reg1==CREG_NONE) {
	    cur.ModrM=cur.ModrM+5;
	    MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
	    cur.disp.U8_cnt=4;
	    if (a->seg_size==64)
	      cur.disp.lit=0;
	  } else {
	    if (tempa1->reg2==CREG_NONE && tempa1->scale==1) {
	      cur.ModrM|=tempa1->reg1&7;
	      if (tempa1->reg1&15>7)
		cur.REX|=1;
	      if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		cur.has_REX=TRUE;
	    } else {
	      cur.ModrM|=4;
	      cur.has_SIB=TRUE;
	      if (tempa1->scale==1)
		cur.SIB=0;
	      else if (tempa1->scale==2)
		cur.SIB=0x40;
	      else if (tempa1->scale==4)
		cur.SIB=0x80;
	      else if (tempa1->scale==8)
		cur.SIB=0xC0;
	      if (tempa1->reg2==CREG_NONE) {
		ModrM_complete=TRUE;
		cur.SIB|=(tempa1->reg1&7)<<3+CREG_RBP;
		if (tempa1->reg1&15>7)
		  cur.REX|=2;
		if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
		MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
		cur.disp.U8_cnt=4;
	      } else {
		cur.SIB|=(tempa1->reg1&7)<<3+tempa1->reg2&7;
		if (tempa1->reg1&15>7)
		  cur.REX|=2;
		if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
		if (tempa1->reg2&15>7)
		  cur.REX|=1;
		if (tempa1->reg2>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
		if (tempa1->reg2&7==CREG_RBP && !tempa1->imm_or_off_present && tempa1->indirect) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		  ModrM_complete=TRUE;
		}
	      }
	    }
	    if (!ModrM_complete) {
	      if (tempa1->imm_or_off_present) {
		MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
		if (!cur.disp.num.exp && MIN_I8<=cur.disp.num.i<=MAX_I8) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		} else if (a->seg_size==16) {
		  cur.ModrM|=0x80;
		  cur.disp.U8_cnt=2;
		} else {
		  cur.ModrM|=0x80;
		  cur.disp.U8_cnt=4;
		}
	      } else if (!tempa1->indirect) {
		cur.has_addr_prefix=FALSE;
		cur.ModrM|=0xC0;
	      } else {
		if (tempa1->reg1&7==CREG_RBP) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		}
	      }
	    }
	  }
	} else if (ARGT_REL8<=a1<=ARGT_REL32 ||
		   ARGT_IMM8<=a1<=ARGT_IMM64 ||
		   ARGT_UIMM8<=a1<=ARGT_UIMM64) {
	  if (a1==ARGT_IMM64 || a2==ARGT_UIMM64)
	    cur.REX|=8;
	  MemCpy(&cur.imm.num,&tempa1->num,sizeof(CAsmNum));
	}
      } else if (argcnt==2) {
	if (best.U8_cnt!=255 && !found_second_possible && !best.is_dft) {
	  found_second_possible=TRUE;
	  if (!a->arg1.size && !a->arg2.size)
	    PrintFWarn("%S%s,%04d\n","ST_NO_SIZE_SPECIFIED",
	      lx->cur_lfn->name,lx->cur_lfn->line_num-1);
	}
	if (tempi->flags & IEF_PLUS_OPCODE) {
	  if (tempi->slash_val==SV_R_REGISTER) {
	    if (ARGT_AL<=a1<=ARGT_RAX) {
	      cur.last_opcode_U8|=tempa2->reg1&7;
	      if (tempa2->reg1&15>7)
		cur.REX|=1;
	      if (tempa2->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		cur.has_REX=TRUE;
	    } else {
	      cur.last_opcode_U8|=tempa1->reg1&7;
	      if (tempa1->reg1&15>7)
		cur.REX|=1;
	      if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		cur.has_REX=TRUE;
	    }
	  } else { //SV_I_REGISTER
	    if (tempa1->fstk!=CREG_NONE)
	      cur.last_opcode_U8|=tempa1->fstk;
	    if (tempa2->fstk!=CREG_NONE)
	      cur.last_opcode_U8|=tempa2->fstk;
	  }
	}
	if (a1==ARGT_RM64 || a2==ARGT_RM64 ||
	    a1==ARGT_M64 || a2==ARGT_M64 ||
	    a1==ARGT_R64 || a2==ARGT_R64)
	  cur.REX|=8;
	if (ARGT_RM8<=a1<=ARGT_RM64 || ARGT_RM8<=a2<=ARGT_RM64 ||
	    ARGT_M8 <=a1<=ARGT_M64  || ARGT_M8 <=a2<=ARGT_M64) {
	  if (a->seg_size==16)
	    cur.has_addr_prefix=TRUE;
	  cur.has_ModrM=TRUE;
	  if (ARGT_RM8<=a2<=ARGT_RM64 || ARGT_M8<=a2<=ARGT_M64) {
	    tempa1=&a->arg2;
	    tempa2=&a->arg1;
	  }
	  if (tempi->slash_val==SV_R_REGISTER) {
	    if (tempa2->just_seg)
	      cur.ModrM|=tempa2->seg<<3;
	    else {
	      if (tempa2->reg1==CREG_NONE) {
		cur.ModrM|=(tempa1->reg1&7)<<3;
		if (tempa1->reg1&15>7)
		  cur.REX|=4;
		if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
	      } else {
		cur.ModrM|=(tempa2->reg1&7)<<3;
		if (tempa2->reg1&15>7)
		  cur.REX|=4;
		if (tempa2->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
	      }
	    }
	  }
	  if (tempa1->reg2==CREG_NONE && tempa1->scale==1) {
	    if (tempa1->reg1!=CREG_NONE) {
	      cur.ModrM|=tempa1->reg1&7;
	      if (tempa1->reg1&15>7)
		cur.REX|=1;
	      if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		cur.has_REX=TRUE;
	    }
	  } else {
	    cur.ModrM|=4;
	    cur.has_SIB=TRUE;
	    if (tempa1->scale==1)
	      cur.SIB=0;
	    else if (tempa1->scale==2)
	      cur.SIB=0x40;
	    else if (tempa1->scale==4)
	      cur.SIB=0x80;
	    else if (tempa1->scale==8)
	      cur.SIB=0xC0;
	      if (tempa1->reg2==CREG_NONE) {
		ModrM_complete=TRUE;
		cur.SIB|=(tempa1->reg1&7)<<3+5;
		if (tempa1->reg1&15>7)
		  cur.REX|=2;
		if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
		MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
		cur.disp.U8_cnt=4;
	      } else {
		cur.SIB|=(tempa1->reg1&7)<<3+tempa1->reg2&7;
		if (tempa1->reg1&15>7)
		  cur.REX|=2;
		if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
		if (tempa1->reg2&15>7)
		  cur.REX|=1;
		if (tempa1->reg2>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
		  cur.has_REX=TRUE;
		if (tempa1->reg2&7==CREG_RBP && !tempa1->imm_or_off_present && tempa1->indirect) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		  ModrM_complete=TRUE;
		}
	      }
	  }
	  if (!ModrM_complete) {
	    if (tempa1->imm_or_off_present && tempa1->indirect && tempa1->reg1==CREG_NONE) {
	      cur.ModrM=cur.ModrM&0xF8+5;
	      MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
	      cur.disp.U8_cnt=4;
	      if (a->seg_size==64)
		cur.disp.lit=0;
	    } else {
	      if (tempa1->imm_or_off_present) {
		MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
		if (!cur.disp.num.exp && MIN_I8<=cur.disp.num.i<=MAX_I8) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		} else if (a->seg_size==16) {
		  cur.ModrM|=0x80;
		  cur.disp.U8_cnt=2;
		} else {
		  cur.ModrM|=0x80;
		  cur.disp.U8_cnt=4;
		}
	      } else if (!tempa1->indirect) {
		cur.has_addr_prefix=FALSE;
		cur.ModrM|=0xC0;
	      } else {
		if (tempa1->reg1&7==CREG_RBP) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		}
	      }
	    }
	  }
	} else if (ARGT_MOFFS8<=a1<=ARGT_MOFFS64) {
	  MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
	  if (a->seg_size==16)
	    cur.disp.U8_cnt=2;
	  else
	    cur.disp.U8_cnt=4;
	  cur.has_addr_prefix=FALSE;
	} else if (ARGT_MOFFS8<=a2<=ARGT_MOFFS64) {
	  MemCpy(&cur.disp.num,&tempa2->num,sizeof(CAsmNum));
	  if (a->seg_size==16)
	    cur.disp.U8_cnt=2;
	  else
	    cur.disp.U8_cnt=4;
	  cur.has_addr_prefix=FALSE;
	} else if (ARGT_IMM8<=a1<=ARGT_IMM64 ||
		   ARGT_UIMM8<=a1<=ARGT_UIMM64) {
	  MemCpy(&cur.imm.num,&tempa1->num,sizeof(CAsmNum));
	  if (a1==ARGT_IMM8 || a1==ARGT_UIMM8) cur.imm.U8_cnt=1;
	  else if (a1==ARGT_IMM16 || a1==ARGT_UIMM16) cur.imm.U8_cnt=2;
	  else if (a1==ARGT_IMM32 || a1==ARGT_UIMM32) cur.imm.U8_cnt=4;
	  else {
	    cur.imm.U8_cnt=8;
	    cur.REX|=8;
	  }
	}
	if (ARGT_IMM8<=a2<=ARGT_IMM64 ||
	    ARGT_UIMM8<=a2<=ARGT_UIMM64) {
	  MemCpy(&cur.imm.num,&tempa2->num,sizeof(CAsmNum));
	  if (a2==ARGT_IMM8 || a2==ARGT_UIMM8) cur.imm.U8_cnt=1;
	  else if (a2==ARGT_IMM16 || a2==ARGT_UIMM16) cur.imm.U8_cnt=2;
	  else if (a2==ARGT_IMM32 || a2==ARGT_UIMM32) cur.imm.U8_cnt=4;
	  else {
	    cur.imm.U8_cnt=8;
	    cur.REX|=8;
	  }
	}
      }
      cur.U8_cnt=tempi->opcode_cnt+cur.disp.U8_cnt+cur.imm.U8_cnt;
      if (cur.has_ModrM) cur.U8_cnt++;
      if (cur.has_SIB) cur.U8_cnt++;
      if (a->seg_size==64 && cur.REX&0x40==0x40 &&
	  (cur.REX!=0x40 || tempi->flags&IEF_40_REX || cur.has_REX) &&
	  (cur.REX&7 || !(tempi->flags&IEF_REX_ONLY_R8_R15 ||
		      tempi->flags&IEF_REX_XOR_LIKE && tempa1->reg1==tempa2->reg1 && cur.ModrM&0xC0==0xC0)))
	  cur.U8_cnt++;
      if (cur.U8_cnt<best.U8_cnt &&
	 !(tempi->flags & IEF_DONT_SWITCH_MODES &&
	  (cur.has_addr_prefix || cur.has_operand_prefix)))
	MemCpy(&best,&cur,sizeof(CAsmIns));
    }
  }
  if (best.U8_cnt<255) {
    tempi=best.tempi;
    seg=CREG_NONE;
    if (argcnt>1 && a->arg2.seg!=CREG_NONE && !a->arg2.just_seg)
      seg=a->arg2.seg;
    else if (argcnt>0 && a->arg1.seg!=CREG_NONE && !a->arg1.just_seg)
      seg=a->arg1.seg;
    if (seg!=CREG_NONE)
      AsmStoreCodeU8(lx,asm_seg_prefixes[seg]);
    if (best.has_operand_prefix)
      AsmStoreCodeU8(lx,OC_OP_SIZE_PREFIX); //Operand size override
    if (best.has_addr_prefix  || a->seg_size==16 && cur.has_SIB)
      AsmStoreCodeU8(lx,OC_ADDR_SIZE_PREFIX); //Operand size override
    if (a->seg_size==64 && best.REX&0x40==0x40 &&
	(best.REX!=0x40 || tempi->flags&IEF_40_REX || best.has_REX) &&
	(best.REX&7||!(tempi->flags&IEF_REX_ONLY_R8_R15 ||
		    tempi->flags&IEF_REX_XOR_LIKE && tempa1->reg1==tempa2->reg1 && best.ModrM&0xC0==0xC0)))
	AsmStoreCodeU8(lx,best.REX);
    for (j=0;j<tempi->opcode_cnt-1;j++)
      AsmStoreCodeU8(lx,tempi->opcode[j]);
    AsmStoreCodeU8(lx,best.last_opcode_U8);

    if (best.has_ModrM)
      AsmStoreCodeU8(lx,best.ModrM);
    if (best.has_SIB)
      AsmStoreCodeU8(lx,best.SIB);

    if (best.disp.U8_cnt) {
      best.disp.rel=a->ip+best.disp.U8_cnt+best.imm.U8_cnt;
      if (!AsmStoreNum(lx,&best.disp,1,U8_avail))
	return FALSE;
    }

    if (best.imm.U8_cnt) {
      best.imm.rel=a->ip+best.imm.U8_cnt;
      if (!AsmStoreNum(lx,&best.imm,1,U8_avail))
	return FALSE;
    }
    if (tempi->flags&IEF_ENDING_ZERO) //ENTER instruction
      AsmStoreCodeU8(lx,0);
    return TRUE;
  }
  LexExcept(lx,"ST_INVALID_INSTRUCTION");
}


U0 DefineU8s(CLex *lx,I64 U8_cnt)
{
  CAsmCtrl *a=lx->a;
  Bool is_dup;
  I64 i,dup_val;
  U8 *ptr;
  CAsmNum2 num2;
  num2.U8_cnt=U8_cnt;

  while (lx->token && lx->token!=';') {
    num2.num.undef_local=NULL;
    num2.num.undef_glbl=NULL;
    if (lx->token==TK_STR) {
      ptr=lx->ident;
      i=lx->ident_len-1;
      while (i--)
	AsmStoreCodeU8(lx,*ptr++);
    Lex(lx);	//Skip Str
    } else {
      is_dup=FALSE;
      lx->abs_cnts=0;
      lx->undef_hash_entry=NULL;
      lx->flags&=~(LF_UNRESOLVED+LF_LOCAL);
      if (!GetExpression(lx,&num2.num.exp))
	LexSkipEol(lx);
      else {
	if (lx->abs_cnts.externs)
	  LexExcept(lx,"ST_EXTERNS_NOT_ALLOWED");
	if (lx->flags & LF_UNRESOLVED) {
	  if (lx->flags & LF_LOCAL) {
	    num2.num.undef_local=lx->undef_hash_entry;
	    lx->undef_hash_entry=NULL;
	    a->has_locals=TRUE;
	  } else {
	    num2.num.undef_glbl=lx->undef_hash_entry;
	    lx->undef_hash_entry=NULL;
	  }
	} else {
	  i=Call(num2.num.exp);
	  ExpressionDel(num2.num.exp);
	}
      }
      if (lx->token==TK_IDENT && lx->hash_entry) {
	if (lx->hash_entry->type & HTT_ASM_KEYWORD && lx->hash_entry->user_data0==AKW_DUP) {
	  is_dup=TRUE;
	  if (Lex(lx)!='(')
	    LexExcept(lx,"ST_EXPECTING_LEFT_PAREN");
	  Lex(lx); //skip (
	  dup_val=AsmExpressionEval(lx);
	  if (lx->token!=')')
	    LexExcept(lx,"ST_EXPECTING_RIGHT_PAREN");
	  Lex(lx); //SKIP )
	}
      }
      num2.rel=0;
      num2.lit=1;
      num2.num.abs_cnts=lx->abs_cnts;
      if (is_dup) {
	if (num2.num.undef_local || num2.num.undef_glbl)
	  LexExcept(lx,"ST_UNDEFINED_DUP_CNT");
	num2.num.i=dup_val;
	AsmStoreNum(lx,&num2,i,FALSE);
      } else {
	num2.num.i=i;
	AsmStoreNum(lx,&num2,1,FALSE);
      }
    }
    if (lx->token==',')
      Lex(lx);
  }
  if (lx->token!=';')
    LexExcept(lx,"ST_MISSING_SEMICOLON");
  else
    Lex(lx);
}

U0 PrsAsmBlk(CLex *lx)
{
  CAsmCtrl *a=lx->a;
  I64 i,j,k,argcnt,
      old_hash_mask=lx->hash_mask,
      old_flags=lx->flags & LF_ASM_EXPRESSIONS;
  CHashOpcode *tempo;
  CHashExport *tempess;
  U8 *next_last_label;

  a->seg_size=64;
  lx->flags|=LF_ASM_EXPRESSIONS;
  lx->hash_mask=lx->hash_mask&~(HTF_FLAGS_MASK|HTT_FILE|HTT_INTERNAL_TYPE|HTT_MODULE)|HTT_ASM_KEYWORD;
  if (lx->token!='{')
    LexExcept(lx,"ST_EXPECTING_LEFT_BRACE");
  else
    Lex(lx);
  while (lx->token && lx->token!='}') {
    AsmLineLst(lx);
    if (lx->token==TK_IDENT && lx->hash_entry) {
      if (lx->hash_entry->type&HTT_ASM_KEYWORD) {
	i=lx->hash_entry->user_data0;
	Lex(lx); //skip keyword
	switch (i) {
	  case AKW_IMPORT:
	    while (lx->token && lx->token!=';') {
	      if (lx->token!=TK_IDENT)
		LexExcept(lx,"ST_EXPECTING_IDENTIFIER");
	      else {
		tempess=NULL;
		tempess=CAlloc(sizeof(CHashExport));
		tempess->str=lx->ident;
		lx->ident=0;
		tempess->type=HTT_EXPORT_SYS_SYM|HTF_UNRESOLVED;
		HashAdd(tempess,lx->glbl_hash_table);
		tempess->type|=HTF_IMPORT;
		if (Lex(lx)==',')
		  Lex(lx); //skip ','
	      }
	    }
	    if (lx->token!=';')
	      LexExcept(lx,"ST_MISSING_SEMICOLON");
	    else
	      Lex(lx); //skip ';';
	    break;
	  case AKW_MODULE_ORG:
	    if (lx->local_var_lst)
	      LexExcept(lx,"ST_NO_FUN_ASM_ORG");
	    if (a->module_org!=INVALID_PTR)
	      LexExcept(lx,"ST_JUST_ONE_ORG_ALLOWED");
	    if (a->ip)
	      LexExcept(lx,"ST_ORG_MUST_BE_AT_BEGINNING");
	    a->module_org=AsmExpressionEval(lx);
	    break;
	  case AKW_ALIGN:
	    if (lx->local_var_lst)
	      LexExcept(lx,"ST_NO_FUN_ASM_ALIGN");
	    i=AsmExpressionEval(lx);
	    if (Bsf(i)!=Bsr(i))
	      LexExcept(lx,"ST_ALIGN_MUST_BE_POWER_OF_TWO");
	    if (!(lx->flags&LF_AOT_COMPILE) && i>8)
	      LexExcept(lx,"ST_ALIGN_TOO_LARGE");
	    if (i>a->max_align) a->max_align=i;
	    i=CeilU64(a->ip,i);
	    if (lx->token==',')
	      Lex(lx);
	    k=AsmExpressionEval(lx);
	    for (j=a->ip;j<i;j++)
	      AsmStoreCodeU8(lx,k);
	    break;
	  case AKW_DU8:
	    DefineU8s(lx,1);
	    break;
	  case AKW_DU16:
	    DefineU8s(lx,2);
	    break;
	  case AKW_DU32:
	    DefineU8s(lx,4);
	    break;
	  case AKW_DU64:
	    DefineU8s(lx,8);
	    break;
	  case AKW_LIST:
	    a->lst=TRUE;
	    break;
	  case AKW_NOLIST:
	    a->lst=FALSE;
	    break;
	  case AKW_USE16:
	    a->seg_size=16;
	    break;
	  case AKW_USE32:
	    a->seg_size=32;
	    break;
	  case AKW_USE64:
	    a->seg_size=64;
	    break;
	  default:
	    LexExcept(lx,"ST_SYNTAX_ERROR");
	}
      } else if (lx->hash_entry->type & HTT_OPCODE) {
	tempo=lx->hash_entry;
	Lex(lx); //skip opcode
	argcnt=0;
	if (tempo->ins[0].arg1) {
	  argcnt++;
	  PrsAsmArg(lx,&a->arg1);
	  if (tempo->ins[0].arg2) {
	    argcnt++;
	    if (lx->token!=',')
	      LexExcept(lx,"ST_EXPECTING_COMMA");
	    else {
	      Lex(lx); //skip ','
	      PrsAsmArg(lx,&a->arg2);
	    }
	  }
	}
	AsmCodeInstruction(lx,tempo,argcnt);
      } else if (lx->hash_entry->type & HTT_EXPORT_SYS_SYM) {
	if (Btr(&lx->hash_entry->type,HTf_UNRESOLVED)) {
	  if (lx->hash_entry->type & HTF_LOCAL) {
	    lx->hash_entry(CHashExport *)->val=a->ip;
	    if (Lex(lx)!=':')
	      LexExcept(lx,"ST_EXPECTING_COLON");
	    Lex(lx);
	  } else {
	    if (lx->hash_entry->type & HTF_IMPORT)
	      LexExcept(lx,"ST_ATTEMPT_DEFINE_IMPORT");
	    lx->hash_entry(CHashExport *)->val=a->ip;
	    next_last_label=lx->hash_entry->str;
	    Lex(lx); //Skip ident
	    if (lx->token!=':' && lx->token!=TK_DBL_COLON)
	      LexExcept(lx,"ST_EXPECTING_COLON");
	    if (lx->token==TK_DBL_COLON) {
	      lx->hash_entry->type|=HTF_EXPORT;
	      SetSrcFileInfo(lx,lx->hash_entry);
	      AsmLocalsResolve(lx);
	      a->last_label=next_last_label;
	    }
	    Lex(lx);
	  }
	} else if (lx->hash_entry(CHashExport *)->val==a->ip) {
	  Lex(lx); //Skip ident
	  if (lx->token!=':' && lx->token!=TK_DBL_COLON)
	    LexExcept(lx,"ST_EXPECTING_COLON");
	  Lex(lx);
	} else
	  LexExcept(lx,"ST_REDEFINITION");
      } else
	LexExcept(lx,"ST_SYNTAX_ERROR");
    } else {
      if (lx->token==TK_IDENT) {
	tempess=CAlloc(sizeof(CHashExport));
	tempess->str=lx->ident;
	lx->ident=0;
	tempess->type=HTT_EXPORT_SYS_SYM;
	tempess->val=a->ip;
	Lex(lx);  //Skip ident
	if (lx->token!=':' && lx->token!=TK_DBL_COLON)
	  LexExcept(lx,"ST_EXPECTING_COLON");
	else {
	  if (*tempess->str=='@' && tempess->str[1]=='@') {
	    if (lx->token==TK_DBL_COLON)
	      LexExcept(lx,"ST_NO_LOCAL_GLBL_EXPORT");
	    HashAdd(tempess,lx->local_hash_table);
	    a->has_locals=TRUE;
	  } else
	    HashAdd(tempess,lx->glbl_hash_table);
	  if (lx->token==TK_DBL_COLON) {
	    tempess->type|=HTF_EXPORT;
	    SetSrcFileInfo(lx,tempess);
	    AsmLocalsResolve(lx);
	    a->last_label=tempess->str;
	  }
	  Lex(lx);
	}
      } else if (lx->token==';')
	Lex(lx);
      else
	LexExcept(lx,"ST_SYNTAX_ERROR");
    }
  }
  AsmLocalsResolve(lx);
  AsmReserveCodeTo(lx,a->ip);
  lx->flags&=lx->flags&~LF_ASM_EXPRESSIONS|old_flags;
  lx->hash_mask=old_hash_mask;
}

