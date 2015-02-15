U0 AsmUnresolvedAdd(CLex *lx,U8 *ex,I64 type,I64 ip,I64 rel_ip,
		CLexUndef *undef_local,CLexUndef *undef_glbl,I64 line_num,
		Bool U8_avail)
{
  CAsmUnresolvedRef *tempu=MAlloc(sizeof(CAsmUnresolvedRef));
  tempu->ex=ex;
  tempu->type=type;
  tempu->ip=ip;
  tempu->rel_ip=rel_ip;
  tempu->exe=lx->exe;
  tempu->imm_flag=FALSE;
  tempu->line_num=line_num;
  tempu->U8_avail=U8_avail;
  tempu->str=NULL;
  if (undef_local) {
    tempu->undef_hash=undef_local;
    tempu->next=lx->a->local_unresolved;
    lx->a->local_unresolved=tempu;
  } else {
    tempu->undef_hash=undef_glbl;
    tempu->next=lx->a->glbl_unresolved;
    lx->a->glbl_unresolved=tempu;
    if (undef_glbl->hash->type & HTF_IMPORT) {
      tempu->str=StrNew(undef_glbl->hash->str);
      if (undef_glbl->hash->type & HTF_IMM)
	tempu->imm_flag=TRUE;
    }
  }
}

U0 AsmLineLst(CLex *lx)
{
  CAOTCtrl *a=lx->a;
  CLexFile *tempf=lx->cur_lfn;
  CDocEntry *doc_e=tempf->line_start,*doc_e2;
  if (a->lst && a->last_lfn==tempf && doc_e) {
    if (doc_e2=a->last_line_lsted) {
      if (tempf->flags& LFSF_DOC) {
	while (doc_e2->y<doc_e->y) {
	  "%h*c%08X ",24-a->display_col,CH_SPACE,a->last_ip;
	  a->display_col=0;
	  doc_e2=LexPutLine(lx,doc_e2);
	}
      } else {
	while (*doc_e2(U8 *) && doc_e2!=doc_e) {
	  "%h*c%08X ",24-a->display_col,CH_SPACE,a->last_ip;
	  a->display_col=0;
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
  CAOTCtrl *a=lx->a;
  I64 i=a->ip>>ASM_CODE_BLK_BITS;
  CAsmCodeBlk *tempc=a->cb;
  while (i--)
    if (!(tempc=tempc->next)) {
      i=a->ip>>ASM_CODE_BLK_BITS+1;
      tempc=&a->cb;
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
      '\n';
      a->display_col=0;
    }
    "%02X ",b;
    a->display_col+=3;
  }
  if (a->ip>a->num_ex_U8s)
    a->num_ex_U8s=a->ip;
}

U0 AsmStoreCodeU32(CLex *lx,U32 d)
{
  CAOTCtrl *a=lx->a;
  I64	i=a->ip>>ASM_CODE_BLK_BITS,
	j=(a->ip+sizeof(U32))>>ASM_CODE_BLK_BITS;
  U32 *_d;
  CAsmCodeBlk *tempc;
  if (i!=j)
    for (i=0;i<sizeof(U32);i++)
      AsmStoreCodeU8(lx,d.u8[i]);
  else {
    tempc=a->cb;
    while (i--)
      if (!(tempc=tempc->next)) {
	i=a->ip>>ASM_CODE_BLK_BITS+1;
	tempc=&a->cb;
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
	'\n';
	a->display_col=0;
      }
      "%08X ",d;
      a->display_col+=9;
    }
    if (a->ip>a->num_ex_U8s)
      a->num_ex_U8s=a->ip;
  }
}

U0 AsmStoreCodeU64(CLex *lx,U64 q)
{
  CAOTCtrl *a=lx->a;
  I64	i=a->ip>>ASM_CODE_BLK_BITS,
	j=(a->ip+sizeof(I64))>>ASM_CODE_BLK_BITS,
	*_q;
  CAsmCodeBlk *tempc;
  if (i!=j)
    for (i=0;i<sizeof(I64);i++)
      AsmStoreCodeU8(lx,q.u8[i]);
  else {
    tempc=a->cb;
    while (i--)
      if (!(tempc=tempc->next)) {
	i=a->ip>>ASM_CODE_BLK_BITS+1;
	tempc=&a->cb;
	while (i--) {
	  if (!tempc->next)
	    tempc->next=CAlloc(sizeof(CAsmCodeBlk));
	  tempc=tempc->next;
	}
	break;
      }
    _q=tempc->body+ a->ip&(ASM_CODE_BLK_SIZE-1);
    *_q=q;
    a->ip+=sizeof(I64);
    if (a->lst) {
      if (a->display_col>=24) {
	'\n';
	a->display_col=0;
      }
      "%016X ",q;
      a->display_col+=17;
    }
    if (a->ip>a->num_ex_U8s)
      a->num_ex_U8s=a->ip;
  }
}

U0 AsmStoreCodeU8At(CLex *lx,I64 ip,...)
{
  CAOTCtrl *a=lx->a;
  CAsmCodeBlk *tempc;
  I64 i,j;
  for (j=0;j<argc;j++) {
    i=ip>>ASM_CODE_BLK_BITS+1;
    tempc=&a->cb;
    while (i--) {
      if (!tempc->next) {
	tempc->next=CAlloc(sizeof(CAsmCodeBlk));
	tempc->next->next=NULL;
      }
      tempc=tempc->next;
    }
    tempc->body[ip++ & (ASM_CODE_BLK_SIZE-1)]=argv[j];
  }
  if (ip>a->num_ex_U8s)
    a->num_ex_U8s=ip;
}

I64 AsmLexExpression(CLex *lx)
{
  U8 *ex;
  I64 result=0;
  lx->undef_hash_entry=NULL;
  lx->flags&=~(LF_UNRESOLVED+LF_LOCAL);
  lx->abs_cnts=0;
  if (!IsLexExpression2Bin(lx,&ex))
    LexSkipEol(lx);
  else {
    if (lx->abs_cnts.externs)
      LexExcept(lx,"Extern Not Allowed at ");
    if (lx->flags & LF_UNRESOLVED)
      LexExcept(lx,"undefined sym at ");
    result=Call(ex);
    Free(ex);
  }
  return result;
}

U0 AsmResolve(CLex *lx,CAsmUnresolvedRef *tempu,U8 *label,Bool undefined)
{
  CExeImportExport *tempe;
  CLexUndef *templu;
  I64 r=Call(tempu->ex);
  if (undefined) {
    templu=tempu->undef_hash;
    while (templu) {
      if (templu->hash->type & HTF_UNRESOLVED) {
	tempe=CAlloc(sizeof(CExeImportExport));
	tempe->next=templu->hash->ie_lst;
	templu->hash->ie_lst=tempe;
	tempe->ip=tempu->ip;
	tempe->exe=lx->exe;
	tempe->type=tempu->type;
      }
      templu=templu->next; //Technically, more than one won't work.
    }
  } else if (!(tempu->type&1))	{//Rel?
    r-=tempu->rel_ip;
    if (tempu->type==EIT_REL_I8 && !(MIN_I8<=r<=MAX_I8) ||
	  tempu->type==EIT_REL_I16 && !(MIN_I16<=r<=MAX_I16)) {
      PrintErr("branch out of range at line:%04d %s\n",
	tempu->line_num,label);
      LexExcept(lx);
    }
    if (tempu->U8_avail && tempu->type>EIT_IMM_U8 && -124<=r<=123)
      PrintWarn("could use I8 displacement at line:%04d %s %s\n",
	tempu->line_num,lx->a->last_label,label);
  }
  AsmStoreCodeU8At(lx,tempu->ip,r.u8[0]);
  if (tempu->type>=EIT_REL_I16) {
    AsmStoreCodeU8At(lx,tempu->ip+1,r.u8[1]);
    if (tempu->type>=EIT_REL_I32) {
      AsmStoreCodeU8At(lx,tempu->ip+2,r.u8[2],r.u8[3]);
      if (tempu->type>=EIT_REL_I64)
	AsmStoreCodeU8At(lx,tempu->ip+4,r.u8[4],r.u8[5],r.u8[6],r.u8[7]);
    }
  }
}

U0 AsmLocalsResolve(CLex *lx)
{
  CAOTCtrl *a=lx->a;
  CAsmUnresolvedRef *tempu=a->local_unresolved,*tempu1;
  CLexUndef *templu;
  Bool undefined;
  U8 *label=NULL;

  while (tempu) {
    undefined=FALSE;
    tempu1=tempu->next;
    templu=tempu->undef_hash;
    while (templu) {
      if (templu->hash->type & HTF_UNRESOLVED) {
	PrintErr("undefined sym at line:%04d %s %s\n",
	  tempu->line_num,a->last_label,templu->hash->str);
	LexExcept(lx);
      }
      label=templu->hash->str;
      templu=templu->next;
    }
    if (!undefined)
      AsmResolve(lx,tempu,label,FALSE);
    Free(tempu->ex);
    LinkedLstDel(tempu->undef_hash);
    Free(tempu);
    tempu=tempu1;
  }
  HashTableDel(lx->htc.local_hash_table);
  lx->htc.hash_table_lst=lx->htc.local_hash_table=HashTableNew(16);
  lx->htc.local_hash_table->next=lx->htc.glbl_hash_table;
  a->local_unresolved=NULL;
}

U0 AsmGlblsResolve(CLex *lx,CExe *tempx)
{
  CAOTCtrl *a=lx->a;
  CHashFun *tempf;
  CAsmUnresolvedRef *tempu=a->glbl_unresolved,*tempu1;
  I64 i,j;
  CExeImportExport *tempe,*tempe1;
  CLexUndef *templu;
  CHashExport *tempex;
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
	tempex=templu->hash;
	if (tempex->type & HTT_EXPORT_SYS_SYM &&
	      tempex->type & HTF_UNRESOLVED &&
	      !(tempex->type & HTF_IMPORT) &&
	      (tempf=HashFind(tempex->str,lx->htc.hash_table_lst,HTT_FUN)) &&
	      !Bt(&tempf->flags,Cf_EXTERN)) {
	  tempex->val=tempf->executable_addr;
	  tempex->type&=~HTF_UNRESOLVED;
	  label=templu->hash->str;
	} else {
	  if (!(tempex->type & HTF_IMPORT)) {
	    if (lx->htc.local_var_lst) {
	      tempex->type|=HTF_GOTO_LABEL;
	      tempex->use_cnt++;
	    } else {
	      PrintErr("undefined sym at line:%04d %s\n",
		    tempu->line_num,tempex->str);
	      LexExcept(lx);
	    }
	  } else if (undefined) {
	    PrintErr("Two imports in same expression not allowed at line:%04d %s\n",
		  tempu->line_num,tempex->str);
	    LexExcept(lx);
	  }
	  undefined=TRUE;
	}
      } else
	label=templu->hash->str;
      templu=templu->next;
    }
    AsmResolve(lx,tempu,label,undefined);
    Free(tempu->ex);
    LinkedLstDel(tempu->undef_hash);
    Free(tempu);
    tempu=tempu1;
  }

  for (i=0;i<=lx->htc.glbl_hash_table->mask;i++) {
    tempex=lx->htc.glbl_hash_table->body[i];
    while (tempex) {
      if (tempex->type & (HTF_IMPORT|HTF_GOTO_LABEL)) {
	if (tempex->use_cnt && (tempe=tempex->ie_lst)) {
	  if (tempex->type&HTF_GOTO_LABEL)
	    tempe->flags|=EIF_GOTO_LABEL;
	  if (tempex->import_name)
	    tempe->str=StrNew(tempex->import_name);
	  else
	    tempe->str=StrNew(tempex->str);
	  do {
	    tempe1=tempe->next;
	    QueIns(tempe,tempx->last_ie);
	  } while (tempe=tempe1);
	  tempex->ie_lst=NULL;
	}
      } else if (tempex->type & (HTF_EXPORT|HTF_RESOLVE)) {
	if (tempex->type & HTF_UNRESOLVED) {
	  PrintErr("undefined sym at %s\n",tempex->str);
	  LexExcept(lx);
	}
	if (tempex->type & HTF_RESOLVE) {
	  tempf=tempex;
	  tempeu=tempf->ext_lst;
	  while (tempeu) {
	    tempeu8=tempeu->next;
	    j=tempf->executable_addr-(tempeu->ip+4);
	    AsmStoreCodeU8At(lx,tempeu->ip,j.u8[0],j.u8[1],j.u8[2],j.u8[3]);
	    Free(tempeu);
	    tempeu=tempeu8;
	  }
	}
	if (tempex->type & HTF_EXPORT) {
	  tempe=CAlloc(sizeof(CExeImportExport));
	  tempe->type=EIT_REL32_EXPORT;
	  if (tempex->type & HTT_FUN)
	    tempe->ip=tempf->executable_addr;
	  else if (tempex->type & HTT_GLBL_VAR)
	    tempe->ip=tempex(CHashGlblVar *)->data_addr_ip;
	  else
	    tempe->ip=tempex->val;
	  tempe->exe=lx->exe;
	  if (tempex->type & HTF_IMM)
	    tempe->type++;
	  tempe->str=StrNew(tempex->str);
	  tempe->src_link=StrNew(tempex->src_link);
	  QueIns(tempe,tempx->last_ie);
	}
      }
      tempex=tempex->next;
    }
  }
}

Bool PrsAsmImm(CLex *lx,CAsmArg *arg)
{
  if (arg->imm_or_off_present)
    LexExcept(lx,"already one immediate at ");
  arg->imm_or_off_present=TRUE;
  arg->num.undef_local=NULL;
  arg->num.undef_glbl=NULL;
  lx->undef_hash_entry=NULL;
  lx->abs_cnts=0;
  lx->flags&=~(LF_UNRESOLVED+LF_LOCAL);
  if (!IsLexExpression2Bin(lx,&arg->num.ex))
    LexSkipEol(lx);
  else {
    if (lx->abs_cnts.externs)
      LexExcept(lx,"Extern Not Allowed at ");
    if (lx->flags & LF_UNRESOLVED) {
      if (lx->flags & LF_LOCAL) {
	arg->num.undef_local=lx->undef_hash_entry;
	lx->undef_hash_entry=NULL;
      } else {
	arg->num.undef_glbl=lx->undef_hash_entry;
	lx->undef_hash_entry=NULL;
      }
    } else {
      arg->num.i=Call(arg->num.ex);
      arg->num.undef_glbl=lx->undef_hash_entry;
      lx->undef_hash_entry=NULL;
      Free(arg->num.ex);
      arg->num.ex=NULL;
    }
  }
  return TRUE;
}

U0 PrsAsmArg(CLex *lx,CAsmArg *arg,Bool rel)
{
  CHashGeneric *temph,*temph1;
  CHashReg *tempr;
  MemSet(arg,0,sizeof(CAsmArg));
  arg->seg=REG_NONE;
  arg->reg1=REG_NONE;
  arg->reg2=REG_NONE;
  arg->fstk=REG_NONE;
  arg->scale=1;
  while (TRUE) {
    if (lx->token==TK_IDENT) {
      if (temph=lx->hash_entry) {
	if (temph->type&HTG_TYPE_MASK==HTT_REG) {
	  tempr=temph;
	  switch (tempr->reg_type) {
	    sub_switch_start:
	      case REGT_R8:
		arg->size=1;
		break;
	      case REGT_R16:
		arg->size=2;
		break;
	      case REGT_R32:
		arg->size=4;
		break;
	      case REGT_R64:
		arg->size=8;
		break;
	    sub_switch_end:
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
	} else {
	  if ((temph->type&HTG_TYPE_MASK==HTT_CLASS||
	       temph->type&HTG_TYPE_MASK==HTT_INTERNAL_TYPE) &&
	      (temph1=HashFind(lx->cur_str,cmp.asm_hash,HTT_ASM_KEYWORD)))
	    temph=temph1;
	  if (temph->type&HTG_TYPE_MASK==HTT_ASM_KEYWORD) {
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
		LexExcept(lx,"syntax error at ");
	    }
	    Lex(lx); //skip keyword
	  } else
	    goto pa_asm_direct_imm;
	}
      } else {
pa_asm_direct_imm:
	PrsAsmImm(lx,arg);
	arg->num.abs_cnts=lx->abs_cnts;
	if (arg->size<=1 && !rel && arg->num.abs_cnts&1) {
	  if (lx->a->seg_size==16)
	    arg->size=2;
	  else
	    arg->size=4;
	}
	if (lx->token!='[')
	  return;
      }
    } else if (lx->token=='[') {
      arg->indirect=TRUE;
      Lex(lx); // skip [
      while (lx->token && lx->token!=']') {
	if (lx->token==TK_IDENT) {
	  if (temph=lx->hash_entry) {
	    if (temph->type&HTG_TYPE_MASK==HTT_REG &&
		  REGT_R16<=temph(CHashReg *)->reg_type<=REGT_R64) {
	      tempr=temph;
	      if (arg->reg1==REG_NONE) {
		if (tempr->reg_num&7==REG_RSP) {
		  arg->reg1=4;
		  arg->reg2=tempr->reg_num;
		} else
		  arg->reg1=tempr->reg_num;
	      } else
		arg->reg2=tempr->reg_num;
	      Lex(lx);
	    } else
	      goto pa_asm_indirect_imm;
	  } else
	    goto pa_asm_indirect_imm;
	} else if (lx->token=='*') {
	  Lex(lx);
	  if (lx->token!=TK_INTEGER)
	    LexExcept(lx,"expecting scale factor at ");
	  arg->scale=lx->cur_i64;
	  Lex(lx); //skip scale
	  if (arg->reg2!=REG_NONE)
	    SwapI64(&arg->reg1,&arg->reg2);
	} else if (lx->token=='+') {
	  Lex(lx); //skip '+'
	} else {
pa_asm_indirect_imm:
	  PrsAsmImm(lx,arg);
	  arg->num.abs_cnts=lx->abs_cnts;
	}
      }
      if (lx->token!=']')
	LexExcept(lx,"missing ']' at ");
      Lex(lx); //skip ]
      return;
    } else
      goto pa_asm_direct_imm;
  }
}

U0 AsmMakeArgMask(CLex *lx,CAsmArg *arg,I64 *_mask)
{
  CAOTCtrl *a=lx->a;
  if (arg->just_seg) {
    switch (arg->seg) {
      case 0: *_mask=1<<ARGT_ES|1<<ARGT_SREG; break;
      case 1: *_mask=1<<ARGT_CS|1<<ARGT_SREG; break;
      case 2: *_mask=1<<ARGT_SS|1<<ARGT_SREG; break;
      case 3: *_mask=1<<ARGT_DS|1<<ARGT_SREG; break;
      case 4: *_mask=1<<ARGT_FS|1<<ARGT_SREG; break;
      case 5: *_mask=1<<ARGT_GS|1<<ARGT_SREG; break;
    }
    return;
  }
  if (arg->fstk!=REG_NONE) {
    if (arg->fstk)
      *_mask=1<<ARGT_STI;
    else
      *_mask=1<<ARGT_ST0|1<<ARGT_STI;
    return;
  }
  switch (arg->size) {
    case 0: *_mask=0x3FF0FFFFFE; break;
    case 1: *_mask=0x1110111112; break;
    case 2: *_mask=0x2220222224; break;
    case 4: *_mask=0x0440444448; break;
    case 8: *_mask=0x0880888880; break;
  }
  if (a->seg_size==64)
    *_mask&=0xFF0FFFFFFF;

  if (arg->reg1!=REG_NONE && arg->imm_or_off_present && !arg->num.i &&
     !arg->num.undef_glbl && !arg->num.undef_local)
      arg->imm_or_off_present=FALSE;  //Zero displacement

  if (arg->reg2!=REG_NONE || arg->scale!=1) {
    *_mask&=0x0000FF0000;
    return;
  }

  if (arg->indirect) {
    if (arg->imm_or_off_present)
      *_mask&=0x00FFFF0000;
    else
      *_mask&=0x000FFF0000;
  } else {
    if (arg->imm_or_off_present)
      *_mask&=0x000F000FFE;
    else
      *_mask&=0x3F0FFFF000;
  }
  if (arg->seg!=REG_NONE)
    *_mask&=0x00FFFF0000;
  if (arg->reg1==REG_NONE) {
    if (arg->indirect)
      *_mask&=0x00FFFF0000;
    else if (arg->num.i<0) {
      if (arg->num.i>=MIN_I8)
	*_mask&=0x8FE;
      else if (arg->num.i>=MIN_I16)
	*_mask&=0x8EE;
      else if (arg->num.i>=MIN_I32)
	*_mask&=0x8CE;
      else
	*_mask&=0x88E;
    } else {
      if (arg->num.i<=MAX_I8)
	*_mask&=0xFFE;
      else if (arg->num.i<=MAX_U8)
	*_mask&=0xFEE;
      else if (arg->num.i<=MAX_I16)
	*_mask&=0xEEE;
      else if (arg->num.i<=MAX_U16)
	*_mask&=0xECE;
      else if (arg->num.i<=MAX_I32)
	*_mask&=0xCCE;
      else if (arg->num.i<=MAX_U32)
	*_mask&=0xC8E;
      else
	*_mask&=0x88E;
    }
  } else {
    *_mask&=  0x3F00FFF000;
    if (!arg->indirect) //M8-M64
      *_mask&=0xFFFF0FFFFF;
  }
  switch (arg->reg1) {
    case REG_RAX: *_mask&=~0x3000000000; break;
    case REG_RCX: *_mask&=~0x2F00000000; break;
    case REG_RDX: *_mask&=~0x1F00000000; break;
    default:	  *_mask&=~0x3F00000000;
  }
}

Bool AsmStoreNum(CLex *lx,CAsmNum2 *num2,I64 cnt,Bool U8_avail)
{
  CAOTCtrl *a=lx->a;
  I64 i;
  CExeAbsAddr *tempa;

  if (!num2->imm_flag)
    num2->num.i-=num2->rel;
  for (i=0;i<cnt;i++) {
    if (num2->U8_cnt==1) {
      if (num2->num.undef_local||num2->num.undef_glbl)
	AsmUnresolvedAdd(lx,num2->num.ex,EIT_REL_I8+num2->imm_flag,a->ip,num2->rel,
	      num2->num.undef_local,num2->num.undef_glbl,lx->cur_lfn->line_num,U8_avail);
      else if (!num2->imm_flag && !(MIN_I8<=num2->num.i<=MAX_I8))
	LexExcept(lx,"branch out of range at ");
      if (num2->imm_flag) {
	if (num2->num.abs_cnts.abs_addres&1) {
	  tempa=CAlloc(sizeof(CExeAbsAddr));
	  tempa->next=a->abss;
	  a->abss=tempa;
	  tempa->ip=a->ip;
	  tempa->type=EAT_ADD_U8;
	}
      } else {
	if (num2->num.abs_cnts.c_addres&1) {
	  tempa=CAlloc(sizeof(CExeAbsAddr));
	  tempa->next=a->abss;
	  a->abss=tempa;
	  tempa->ip=a->ip;
	  tempa->type=EAT_SUB_U8;
	}
      }
      AsmStoreCodeU8(lx,num2->num.i);
    } else {
      if (num2->U8_cnt==2) {
	if (num2->num.undef_local||num2->num.undef_glbl)
	  AsmUnresolvedAdd(lx,num2->num.ex,EIT_REL_I16+num2->imm_flag,a->ip,num2->rel,
		num2->num.undef_local,num2->num.undef_glbl,lx->cur_lfn->line_num,U8_avail);
	else if (!num2->imm_flag && !(MIN_I16<=num2->num.i<=MAX_I16))
	  LexExcept(lx,"branch out of range at ");
	if (num2->imm_flag) {
	  if (num2->num.abs_cnts.abs_addres&1) {
	    tempa=CAlloc(sizeof(CExeAbsAddr));
	    tempa->next=a->abss;
	    a->abss=tempa;
	    tempa->ip=a->ip;
	    tempa->type=EAT_ADD_U16;
	  }
	} else {
	  if (num2->num.abs_cnts.c_addres&1) {
	    tempa=CAlloc(sizeof(CExeAbsAddr));
	    tempa->next=a->abss;
	    a->abss=tempa;
	    tempa->ip=a->ip;
	    tempa->type=EAT_SUB_U16;
	  }
	}
	AsmStoreCodeU8(lx,num2->num.i.u8[0]);
	AsmStoreCodeU8(lx,num2->num.i.u8[1]);
      } else if (num2->U8_cnt==4) {
	if (num2->num.undef_local||num2->num.undef_glbl)
	  AsmUnresolvedAdd(lx,num2->num.ex,EIT_REL_I32+num2->imm_flag,a->ip,num2->rel,
		num2->num.undef_local,num2->num.undef_glbl,lx->cur_lfn->line_num,U8_avail);
	else if (!num2->imm_flag && !(MIN_I32<=num2->num.i<=MAX_I32))
	  LexExcept(lx,"branch out of range at ");
	if (num2->imm_flag) {
	  if (num2->num.abs_cnts.abs_addres&1) {
	    tempa=CAlloc(sizeof(CExeAbsAddr));
	    tempa->next=a->abss;
	    a->abss=tempa;
	    tempa->ip=a->ip;
	    tempa->type=EAT_ADD_U32;
	  }
	} else {
	  if (num2->num.abs_cnts.c_addres&1) {
	    tempa=CAlloc(sizeof(CExeAbsAddr));
	    tempa->next=a->abss;
	    a->abss=tempa;
	    tempa->ip=a->ip;
	    tempa->type=EAT_SUB_U32;
	  }
	}
	AsmStoreCodeU32(lx,num2->num.i);
      } else if (num2->U8_cnt==8) {
	if (num2->num.undef_local||num2->num.undef_glbl)
	  AsmUnresolvedAdd(lx,num2->num.ex,EIT_REL_I64+num2->imm_flag,a->ip,num2->rel,
		num2->num.undef_local,num2->num.undef_glbl,lx->cur_lfn->line_num,U8_avail);
	if (num2->imm_flag) {
	  if (num2->num.abs_cnts.abs_addres&1) {
	    tempa=CAlloc(sizeof(CExeAbsAddr));
	    tempa->next=a->abss;
	    a->abss=tempa;
	    tempa->ip=a->ip;
	    tempa->type=EAT_ADD_U64;
	  }
	} else {
	  if (num2->num.abs_cnts.c_addres&1) {
	    tempa=CAlloc(sizeof(CExeAbsAddr));
	    tempa->next=a->abss;
	    a->abss=tempa;
	    tempa->ip=a->ip;
	    tempa->type=EAT_SUB_U64;
	  }
	}
	AsmStoreCodeU64(lx,num2->num.i);
      }
      if (U8_avail && !num2->num.undef_local && !num2->num.undef_glbl &&
	    !num2->imm_flag && -124<=num2->num.i<=123) {
	LexWarn(lx,"could use I8 displacement at ");
	return FALSE;
      }
    }
  }
  return TRUE;
}

U8 asm_seg_prefixes[6]={0x26,0x2E,0x36,0x3E,0x64,0x65};

Bool PrsAsmInstruction(CLex *lx,CHashOpcode *tempo,I64 argcnt)
{
  CAOTCtrl *a=lx->a;
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
      cur.disp.imm_flag=TRUE;
      cur.imm.imm_flag=TRUE;
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
	cur.imm.imm_flag=FALSE;
      } else if (om==OM_CW) {
	cur.imm.U8_cnt=2;
	cur.imm.imm_flag=FALSE;
      } else if (om==OM_CD) {
	cur.imm.U8_cnt=4;
	cur.imm.imm_flag=FALSE;
      }

      if (argcnt==1) {
	if (best.U8_cnt!=255 && !found_second_possible && !best.is_dft) {
	  found_second_possible=TRUE;
	  if (!a->arg1.size)
	    PrintWarn("no size specified at %s,%04d\n",
		lx->cur_lfn->full_name,lx->cur_lfn->line_num-1);
	}
	if (tempi->flags & IEF_PLUS_OPCODE) {
	  if (tempi->slash_val==SV_R_REG) {
	    cur.last_opcode_U8|=tempa1->reg1&7;
	    if (tempa1->reg1&15>7)
	      cur.REX|=1;
	    if (tempa1->reg1>=20) //RBPu8,RSPu8,RSIu8,RDIu8?
	      cur.has_REX=TRUE;
	  } else {//SV_I_REG
	    if (tempa1->fstk!=REG_NONE)
	      cur.last_opcode_U8+=tempa1->fstk;
	  }
	}
	if (a1==ARGT_R64 || a1==ARGT_RM64 || a1==ARGT_M64)
	  cur.REX|=8;
	if (ARGT_RM8<=a1<=ARGT_RM64 || ARGT_M8<=a1<=ARGT_M64) {
	  if (a->seg_size==16)
	    cur.has_addr_prefix=TRUE;

	  cur.has_ModrM=TRUE;
	  if (tempa1->imm_or_off_present && tempa1->indirect && tempa1->reg1==REG_NONE) {
	    cur.ModrM=cur.ModrM+5;
	    MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
	    cur.disp.U8_cnt=4;
	    if (a->seg_size==64)
	      cur.disp.imm_flag=FALSE;
	  } else {
	    if (tempa1->reg2==REG_NONE && tempa1->scale==1) {
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
	      if (tempa1->reg2==REG_NONE) {
		ModrM_complete=TRUE;
		cur.SIB|=(tempa1->reg1&7)<<3+REG_RBP;
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
		if (tempa1->reg2&7==REG_RBP && !tempa1->imm_or_off_present && tempa1->indirect) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		  ModrM_complete=TRUE;
		}
	      }
	    }
	    if (!ModrM_complete) {
	      if (tempa1->imm_or_off_present) {
		MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
		if (!cur.disp.num.ex && MIN_I8<=cur.disp.num.i<=MAX_I8) {
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
		if (tempa1->reg1&7==REG_RBP) {
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
	    PrintWarn("no size specified at %s,%04d\n",
		lx->cur_lfn->full_name,lx->cur_lfn->line_num-1);
	}
	if (tempi->flags & IEF_PLUS_OPCODE) {
	  if (tempi->slash_val==SV_R_REG) {
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
	  } else {//SV_I_REG
	    if (tempa1->fstk!=REG_NONE)
	      cur.last_opcode_U8|=tempa1->fstk;
	    if (tempa2->fstk!=REG_NONE)
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
	  if (tempi->slash_val==SV_R_REG) {
	    if (tempa2->just_seg)
	      cur.ModrM|=tempa2->seg<<3;
	    else {
	      if (tempa2->reg1==REG_NONE) {
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
	  if (tempa1->reg2==REG_NONE && tempa1->scale==1) {
	    if (tempa1->reg1!=REG_NONE) {
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
	      if (tempa1->reg2==REG_NONE) {
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
		if (tempa1->reg2&7==REG_RBP && !tempa1->imm_or_off_present && tempa1->indirect) {
		  cur.ModrM|=0x40;
		  cur.disp.U8_cnt=1;
		  ModrM_complete=TRUE;
		}
	      }
	  }
	  if (!ModrM_complete) {
	    if (tempa1->imm_or_off_present && tempa1->indirect && tempa1->reg1==REG_NONE) {
	      cur.ModrM=cur.ModrM&0xF8+5;
	      MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
	      cur.disp.U8_cnt=4;
	      if (a->seg_size==64)
		cur.disp.imm_flag=FALSE;
	    } else {
	      if (tempa1->imm_or_off_present) {
		MemCpy(&cur.disp.num,&tempa1->num,sizeof(CAsmNum));
		if (!cur.disp.num.ex && MIN_I8<=cur.disp.num.i<=MAX_I8) {
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
		if (tempa1->reg1&7==REG_RBP) {
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
	  else if (a2==ARGT_IMM32 || a2==ARGT_UIMM32) {
	    cur.imm.U8_cnt=4;
	    if (tempi->flags&IEF_REX_ONLY_R8_R15 && a2==ARGT_UIMM32)
	      cur.REX&=~8;
	  } else {
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
    seg=REG_NONE;
    if (argcnt>1 && a->arg2.seg!=REG_NONE && !a->arg2.just_seg)
      seg=a->arg2.seg;
    else if (argcnt>0 && a->arg1.seg!=REG_NONE && !a->arg1.just_seg)
      seg=a->arg1.seg;
    if (seg!=REG_NONE)
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
  LexExcept(lx,"invalid instruction at ");
}

U0 PrsAsmDefine(CLex *lx,I64 U8_cnt)
{
  Bool is_dup;
  I64 i,dup_val;
  U8 *ptr;
  CAsmNum2 num2;
  num2.U8_cnt=U8_cnt;

  while (lx->token && lx->token!=';') {
    num2.num.undef_local=NULL;
    num2.num.undef_glbl=NULL;
    if (lx->token==TK_STR) {
      ptr=lx->cur_str;
      i=lx->cur_str_len-1;
      while (i--)
	AsmStoreCodeU8(lx,*ptr++);
      Lex(lx);	//Skip Str
    } else {
      is_dup=FALSE;
      lx->abs_cnts=0;
      lx->undef_hash_entry=NULL;
      lx->flags&=~(LF_UNRESOLVED+LF_LOCAL);
      if (!IsLexExpression2Bin(lx,&num2.num.ex))
	LexSkipEol(lx);
      else {
	if (lx->abs_cnts.externs)
	  LexExcept(lx,"Extern Not Allowed at ");
	if (lx->flags & LF_UNRESOLVED) {
	  if (lx->flags & LF_LOCAL) {
	    num2.num.undef_local=lx->undef_hash_entry;
	    lx->undef_hash_entry=NULL;
	  } else {
	    num2.num.undef_glbl=lx->undef_hash_entry;
	    lx->undef_hash_entry=NULL;
	  }
	} else {
	  i=Call(num2.num.ex);
	  Free(num2.num.ex);
	}
      }
      if (lx->token==TK_IDENT && lx->hash_entry) {
	if (lx->hash_entry->type & HTT_ASM_KEYWORD && lx->hash_entry->user_data0==AKW_DUP) {
	  is_dup=TRUE;
	  if (Lex(lx)!='(')
	    LexExcept(lx,"expecting '(' at ");
	  Lex(lx); //skip (
	  dup_val=AsmLexExpression(lx);
	  if (lx->token!=')')
	    LexExcept(lx,"expecting ')' at ");
	  Lex(lx); //SKIP )
	}
      }
      num2.rel=0;
      num2.imm_flag=TRUE;
      num2.num.abs_cnts=lx->abs_cnts;
      if (is_dup) {
	if (num2.num.undef_local || num2.num.undef_glbl)
	  LexExcept(lx,"undefined DUP cnt at ");
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
    LexExcept(lx,"missing ';' at");
  Lex(lx);
}

U0 PrsBinFile(CLex *lx)
{
  I64 i,size;
  U8 *buf,*st;
  if (lx->token!=TK_STR)
    LexExcept(lx,"expecting string at ");
  st=DftExt(lx->cur_str,"BIN");
  buf=FileRead(st,&size);
  Free(st);
  for (i=0;i<size;i++)
    AsmStoreCodeU8(lx,buf[i]);
  if (Lex(lx)!=';')
    LexExcept(lx,"missing ';' at");
  Lex(lx);
}

U0 PrsAsmBlk(CLex *lx,I64 cmp_flags)
{
  CAOTCtrl *a=lx->a;
  I64 i,j,k,argcnt,
	old_flags=lx->flags & LF_ASM_EXPRESSIONS;
  CHashOpcode *tempo;
  CHashExport *tempex;
  U8 *next_last_label;
  CLexCodeMisc *g_lb;

  a->seg_size=64;
  lx->flags|=LF_ASM_EXPRESSIONS;
  if (!(cmp_flags&CMPF_ONE_ASM_INS)) {
    if (lx->token!='{')
      LexExcept(lx,"expecting '{' at ");
    Lex(lx);
  }
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
		LexExcept(lx,"expecting identifier at ");
	      else {
		tempex=NULL;
		tempex=CAlloc(sizeof(CHashExport));
		tempex->str=lx->cur_str;
		lx->cur_str=0;
		tempex->type=HTT_EXPORT_SYS_SYM|HTF_UNRESOLVED;
		HashAdd(tempex,lx->htc.glbl_hash_table);
		tempex->type|=HTF_IMPORT;
		if (Lex(lx)==',')
		  Lex(lx); //skip ','
	      }
	    }
	    if (lx->token!=';')
	      LexExcept(lx,"missing ';' at");
	    Lex(lx); //skip ';';
	    break;
	  case AKW_MODULE_ORG:
	    if (lx->htc.local_var_lst)
	      LexExcept(lx,"ORG not allowed in fun asm blk ");
	    if (a->module_org!=INVALID_PTR)
	      LexExcept(lx,"Just one org allowed ");
	    if (a->ip)
	      LexExcept(lx,"ORG must be at beginning ");
	    a->module_org=AsmLexExpression(lx);
	    break;
	  case AKW_ALIGN:
	    if (lx->htc.local_var_lst)
	      LexExcept(lx,"ALIGN not allowed in fun asm blk ");
	    i=AsmLexExpression(lx);
	    j=Bsf(i);
	    if (!i || j!=Bsr(i))
	      LexExcept(lx,"ALIGN must be power of two at ");
	    if (!(lx->flags&LF_AOT_COMPILE) && i>8)
	      LexExcept(lx,"In JIT mode, max ALIGN is 8 ");
	    if (j>a->max_align_bits) a->max_align_bits=j;
	    i=CeilU64(a->ip,i);
	    if (lx->token!=',')
	      LexExcept(lx,"expecting ',' at ");
	    Lex(lx);
	    k=AsmLexExpression(lx);
	    for (j=a->ip;j<i;j++)
	      AsmStoreCodeU8(lx,k);
	    break;
	  case AKW_DU8:
	    PrsAsmDefine(lx,1);
	    break;
	  case AKW_DU16:
	    PrsAsmDefine(lx,2);
	    break;
	  case AKW_DU32:
	    PrsAsmDefine(lx,4);
	    break;
	  case AKW_DU64:
	    PrsAsmDefine(lx,8);
	    break;
	  case AKW_BINFILE:
	    PrsBinFile(lx);
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
	    LexExcept(lx,"syntax error at ");
	}
      } else if (lx->hash_entry->type & HTT_OPCODE) {
	tempo=lx->hash_entry;
	Lex(lx); //skip opcode
	argcnt=0;
	if (tempo->ins[0].arg1) {
	  argcnt++;
	  if (ARGT_REL8<=tempo->ins[0].arg1<=ARGT_REL32)
	    PrsAsmArg(lx,&a->arg1,TRUE);
	  else
	    PrsAsmArg(lx,&a->arg1,FALSE);
	  if (tempo->ins[0].arg2) {
	    argcnt++;
	    if (lx->token!=',')
	      LexExcept(lx,"expecting ',' at ");
	    else {
	      Lex(lx); //skip ','
	      if (ARGT_REL8<=tempo->ins[0].arg2<=ARGT_REL32)
		PrsAsmArg(lx,&a->arg2,TRUE);
	      else
		PrsAsmArg(lx,&a->arg2,FALSE);
	    }
	  }
	}
	PrsAsmInstruction(lx,tempo,argcnt);
      } else if (lx->hash_entry->type & HTT_EXPORT_SYS_SYM) {
	if (Btr(&lx->hash_entry->type,HTf_UNRESOLVED)) {
	  if (lx->hash_entry->type & HTF_LOCAL) {
	    lx->hash_entry(CHashExport *)->val=a->ip;
	    if (Lex(lx)!=':')
	      LexExcept(lx,"expecting ':' at ");
	    Lex(lx);
	  } else {
	    if (lx->hash_entry->type & HTF_IMPORT)
	      LexExcept(lx,"attempt to define import at ");
	    lx->hash_entry(CHashExport *)->val=a->ip;
	    next_last_label=lx->hash_entry->str;
	    Lex(lx); //Skip cur_str
	    if (lx->token!=':' && lx->token!=TK_DBL_COLON)
	      LexExcept(lx,"expecting ':' at ");
	    if (lx->token==TK_DBL_COLON) {
	      lx->hash_entry->type|=HTF_EXPORT;
	      HashSrcFileSet(lx,lx->hash_entry);

	      AsmLocalsResolve(lx);
	      a->last_label=next_last_label;
	    }
	    Lex(lx);
	  }
	} else if (lx->hash_entry(CHashExport *)->val==a->ip) {
	  Lex(lx); //Skip cur_str
	  if (lx->token!=':' && lx->token!=TK_DBL_COLON)
	    LexExcept(lx,"expecting ':' at ");
	  Lex(lx);
	} else
	  LexExcept(lx,"redefinition at ");
      } else
	LexExcept(lx,"syntax error at ");
    } else if (lx->token==TK_IDENT) {
      tempex=CAlloc(sizeof(CHashExport));
      tempex->str=lx->cur_str;
      lx->cur_str=0;
      tempex->type=HTT_EXPORT_SYS_SYM;
      tempex->val=a->ip;
      Lex(lx);	//Skip cur_str
      if (lx->token!=':' && lx->token!=TK_DBL_COLON)
	LexExcept(lx,"expecting ':' at ");
      else {
	if (*tempex->str=='@' && tempex->str[1]=='@') {
	  if (lx->token==TK_DBL_COLON)
	    LexExcept(lx,"no local glbl exports at ");
	  HashAdd(tempex,lx->htc.local_hash_table);
	} else
	  HashAdd(tempex,lx->htc.glbl_hash_table);
	if (lx->htc.local_var_lst) {//AsmBlk in fun? Also add goto-like label.
	  if (!(g_lb=LCCGoToLabelFind(lx,tempex->str))) {
	    g_lb=LCCMiscNew(lx,CMT_ASM_LABEL);
	    g_lb->str=StrNew(tempex->str);
	  } else if (g_lb->flags&CMF_DEFINED)
	    LexExcept(lx,"Duplicate goto label at ");
	  g_lb->type=CMT_ASM_LABEL;
	  g_lb->flags|=CMF_DEFINED;
	  g_lb->ip=a->ip;
	  g_lb->use_cnt++; //Disable warning on unused labels.
	  LCCAdd(lx,IC_LABEL,g_lb,0);
	}
	if (lx->token==TK_DBL_COLON) {
	  tempex->type|=HTF_EXPORT;
	  HashSrcFileSet(lx,tempex);

	  AsmLocalsResolve(lx);
	  a->last_label=tempex->str;
	}
	Lex(lx);
      }
    } else if (lx->token==';')
      Lex(lx);
    else
      LexExcept(lx,"syntax error at ");
    if (cmp_flags&CMPF_ONE_ASM_INS && (lx->token!=TK_IDENT ||
	!(tempo=lx->hash_entry) || !(tempo->type&HTT_OPCODE)))
      break;
  }
  AsmLocalsResolve(lx);
  a->lst=FALSE;
  lx->flags&=lx->flags&~LF_ASM_EXPRESSIONS|old_flags;
}

