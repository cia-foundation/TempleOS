U0 AsmPrsInsFlags(CCmpCtrl *cc,CInst *tmpins)
{
  I64 i;
  while (TRUE) {
    switch (cc->token) {
      case TK_IDENT:
	if ((i=LstMatch(cc->cur_str,"NO\0CB\0CW\0CD\0CP\0IB\0IW\0ID\0"))>=0) {
	  tmpins->opcode_modifier=i;
	  break;
	} else
	  return;
      case TK_I64:
	if (cc->cur_i64==16)
	  tmpins->flags|=IEF_OP_SIZE16;
	else if (cc->cur_i64==32)
	  tmpins->flags|=IEF_OP_SIZE32;
	else
	  return;
	break;
      case '+':
	tmpins->flags|=IEF_PLUS_OPCODE;
      case '/':
	if (Lex(cc)==TK_I64 && cc->cur_i64<8)
	  tmpins->slash_val=cc->cur_i64;
	else if (cc->token==TK_IDENT) {
	  if (!StrCmp(cc->cur_str,"R"))
	    tmpins->slash_val=SV_R_REG;
	  else if (!StrCmp(cc->cur_str,"I"))
	    tmpins->slash_val=SV_I_REG;
	  else
	    return;
	} else
	  return;
	break;
      case '!': tmpins->flags|=IEF_DONT_SWITCH_MODES;	break;
      case '&': tmpins->flags|=IEF_DFT;			break;
      case '%': tmpins->flags|=IEF_NOT_IN_64_BIT;	break;
      case '=': tmpins->flags|=IEF_48_REX;		break;
      case '`': tmpins->flags|=IEF_REX_ONLY_R8_R15;	break;
      case '^': tmpins->flags|=IEF_REX_XOR_LIKE;		break;
      case '*': tmpins->flags|=IEF_STI_LIKE;		break;
      case '$$': tmpins->flags|=IEF_ENDING_ZERO;		break;
      default:
	return;
    }
    Lex(cc);
  }
}

U0 AsmHashLoad()
{//See $LK,"::/Compiler/OpCodes.DD"$.
  I64 i,j,size,size_max;
  CInternalType *tmpit;
  CCmpCtrl *cc;
  CHashGeneric *tmph;
  CHashReg *tmpr;
  CHashOpcode *tmpo,*tmpo2,*tmpo_max;
  CInst *tmpins;
  CHashClass *tmpc;

  cmp.size_arg_mask[0]=0x3FF0FFFFFE;
  cmp.size_arg_mask[1]=0x1110111112;
  cmp.size_arg_mask[2]=0x2220222224;
  cmp.size_arg_mask[4]=0x0440444448;
  cmp.size_arg_mask[8]=0x0880888880;

  cmp.asm_hash=HashTableNew(1024);
  size_max=offset(CHashOpcode.ins)+sizeof(CInst)<<5;
  tmpo_max=MAlloc(size_max);

  cc=CmpCtrlNew(FileRead("OpCodes.DD"),,"OpCodes.DD.Z");
  cc->htc.hash_table_lst=NULL;
  Lex(cc);
  while (cc->token) {
    if (cc->token!=TK_IDENT)
      LexExcept(cc,"Expecting identifier at ");
    i=LstMatch(cc->cur_str,"NONE\0R8\0R16\0R32\0R64\0SEG\0FSTK\0"
	  "MM\0XMM\0OPCODE\0KEYWORD\0ASM_KEYWORD\0");
    if (i<=0)
      LexExcept(cc,"Unknown Stmt");
    Lex(cc); //skip keyword
    if (cc->token!=TK_IDENT)
      LexExcept(cc,"Expecting identifier at ");
    switch (i) {
      case REGT_R8...REGT_XMM:
	tmpr=CAlloc(sizeof(CHashReg));
	tmpr->str=cc->cur_str;
	cc->cur_str=NULL;
	Lex(cc); //skip keyword name
	if (cc->token!=TK_I64)
	  LexExcept(cc,"Expecting int at ");
	tmpr->type=HTT_REG;
	tmpr->reg_type=i;
	tmpr->reg_num=cc->cur_i64;
	HashAdd(tmpr,cmp.asm_hash);
	Lex(cc); //Skip INT
	break;
      case: //OPCODE
	if (cc->token!=TK_IDENT)
	  LexExcept(cc,"Expecting opcode at ");
	MemSet(tmpo_max,0,size_max);
	tmpo_max->type=HTT_OPCODE;
	tmpo_max->inst_entry_cnt=0;
	tmpo_max->str=cc->cur_str;
	cc->cur_str=0;
	Lex(cc);	//Skip OPCODE
	while (cc->token && cc->token!=';' && cc->token!=':') {
	  tmpins=&tmpo_max->ins[tmpo_max->inst_entry_cnt];
	  tmpins->ins_entry_num=tmpo_max->inst_entry_cnt++;
	  tmpins->slash_val=SV_NONE; //Not zero!!
	  while (cc->token==TK_I64) {
	    tmpins->opcode[tmpins->opcode_cnt++]=cc->cur_i64;
	    Lex(cc);
	  }
	  if (cc->token==',')
	    Lex(cc);
	  else if (cc->token!=';')
	    LexExcept(cc,"Expecting ',' at ");

	  AsmPrsInsFlags(cc,tmpins);

	  tmpins->uasm_slash_val=tmpins->slash_val;
	  if (tmpins->flags&IEF_STI_LIKE && tmpins->slash_val!=SV_I_REG)
	    tmpins->uasm_slash_val=SV_STI_LIKE;

	  tmpins->arg1=tmpins->arg2=tmpins->size1=tmpins->size2=0;
	  if (cc->token==TK_IDENT) {
	    j=DefineMatch(cc->cur_str,"ST_ARG_TYPES");
	    tmpins->arg1=j;
	    if (Bt(&cmp.size_arg_mask[1],j))
	      tmpins->size1=8;
	    else if (Bt(&cmp.size_arg_mask[2],j))
	      tmpins->size1=16;
	    else if (Bt(&cmp.size_arg_mask[4],j))
	      tmpins->size1=32;
	    else if (Bt(&cmp.size_arg_mask[8],j))
	      tmpins->size1=64;

	    if (Lex(cc)==TK_IDENT) {
	      j=DefineMatch(cc->cur_str,"ST_ARG_TYPES");
	      Lex(cc);
	      tmpins->arg2=j;
	      if (Bt(&cmp.size_arg_mask[1],j))
		tmpins->size2=8;
	      else if (Bt(&cmp.size_arg_mask[2],j))
		tmpins->size2=16;
	      else if (Bt(&cmp.size_arg_mask[4],j))
		tmpins->size2=32;
	      else if (Bt(&cmp.size_arg_mask[8],j))
		tmpins->size2=64;
	    }
	  }
	}
	size=offset(CHashOpcode.ins)+sizeof(CInst)*tmpo_max->inst_entry_cnt;
	tmpo=MAlloc(size);
	MemCpy(tmpo,tmpo_max,size);
	tmpo->use_cnt=0;
	if (HashFind(tmpo->str,cmp.asm_hash,HTT_OPCODE))
	  LexExcept(cc,"Duplicate OPCODE entry ");
	HashAdd(tmpo,cmp.asm_hash);
	//Parse aliases.
	if (cc->token==':') {
	  while (Lex(cc)==TK_IDENT) {
	    tmpo2=MAllocIdent(tmpo);
	    tmpo2->str=cc->cur_str;
	    cc->cur_str=0;
	    tmpo2->oc_flags|=OCF_ALIAS;
	    if (HashFind(tmpo2->str,cmp.asm_hash,HTT_OPCODE))
	      LexExcept(cc,"Duplicate OPCODE ALIAS entry ");
	    HashAdd(tmpo2,cmp.asm_hash);
	  }
	}
	break;
      case: //KEYWORD
      case: //ASM_KEYWORD
	tmph=CAlloc(sizeof(CHashGeneric));
	tmph->str=cc->cur_str;
	cc->cur_str=NULL;
	Lex(cc); //skip keyword name
	if (cc->token!=TK_I64)
	  LexExcept(cc,"Expecting int at ");
	tmph->user_data0=cc->cur_i64;
	if (i==10)
	  tmph->type=HTT_KEYWORD;
	else
	  tmph->type=HTT_ASM_KEYWORD;
	HashAdd(tmph,cmp.asm_hash);
	Lex(cc); //Skip INT
	break;
    }
    if (cc->token!=';')
      LexExcept(cc,"Missing ';' at");
    Lex(cc); //Skip ';'
  }
  Free(tmpo_max);
  CmpCtrlDel(cc);
  for (i=0;i<INTERNAL_TYPES_NUM;i++) {
    tmpit=&internal_types_table[i];
    tmpc=PrsClassNew;
    tmpc->type=HTT_INTERNAL_TYPE;
    tmpc->raw_type=tmpit->type;
    Bts(&tmpc->flags,Cf_INTERNAL_TYPE);
    tmpc->size=tmpit->size;
    tmpc->str=AStrNew(tmpit->name);
    HashAdd(tmpc,cmp.asm_hash);
    cmp.internal_types[tmpc->raw_type]=tmpc;
  }
  adam_task->hash_table->next=cmp.asm_hash;
}
