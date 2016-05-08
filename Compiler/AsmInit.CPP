U0 AsmPrsInsFlags(CCmpCtrl *cc,CInst *tempi)
{
  I64 i;
  while (TRUE) {
    switch (cc->token) {
      case TK_IDENT:
	if ((i=LstMatch(cc->cur_str,"NO\0CB\0CW\0CD\0CP\0IB\0IW\0ID\0"))>=0) {
	  tempi->opcode_modifier=i;
	  break;
	} else
	  return;
      case TK_I64:
	if (cc->cur_i64==16)
	  tempi->flags|=IEF_OP_SIZE16;
	else if (cc->cur_i64==32)
	  tempi->flags|=IEF_OP_SIZE32;
	else
	  return;
	break;
      case '+':
	tempi->flags|=IEF_PLUS_OPCODE;
      case '/':
	if (Lex(cc)==TK_I64 && cc->cur_i64<8)
	  tempi->slash_val=cc->cur_i64;
	else if (cc->token==TK_IDENT) {
	  if (!StrCmp(cc->cur_str,"R"))
	    tempi->slash_val=SV_R_REG;
	  else if (!StrCmp(cc->cur_str,"I"))
	    tempi->slash_val=SV_I_REG;
	  else
	    return;
	} else
	  return;
	break;
      case '!': tempi->flags|=IEF_DONT_SWITCH_MODES;	break;
      case '&': tempi->flags|=IEF_DFT;			break;
      case '%': tempi->flags|=IEF_NOT_IN_64_BIT;	break;
      case '=': tempi->flags|=IEF_48_REX;		break;
      case '`': tempi->flags|=IEF_REX_ONLY_R8_R15;	break;
      case '^': tempi->flags|=IEF_REX_XOR_LIKE;		break;
      case '*': tempi->flags|=IEF_STI_LIKE;		break;
      case '$$': tempi->flags|=IEF_ENDING_ZERO;		break;
      default:
	return;
    }
    Lex(cc);
  }
}

U0 AsmHashLoad()
{//See $LK,"::/Compiler/OpCodes.TXT"$.
  I64 i,j,size,size_max;
  CInternalType *tempit;
  CCmpCtrl *cc;
  CHashGeneric *temph;
  CHashReg *tempr;
  CHashOpcode *tempo,*tempo2,*tempo_max;
  CInst *tempi;
  CHashClass *tempc;

  cmp.size_arg_mask[0]=0x3FF0FFFFFE;
  cmp.size_arg_mask[1]=0x1110111112;
  cmp.size_arg_mask[2]=0x2220222224;
  cmp.size_arg_mask[4]=0x0440444448;
  cmp.size_arg_mask[8]=0x0880888880;

  cmp.asm_hash=HashTableNew(1024);
  size_max=offset(CHashOpcode.ins)+sizeof(CInst)<<5;
  tempo_max=MAlloc(size_max);

  cc=CmpCtrlNew(FileRead("OpCodes.TXT"),,"OpCodes.TXT.Z");
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
	tempr=CAlloc(sizeof(CHashReg));
	tempr->str=cc->cur_str;
	cc->cur_str=NULL;
	Lex(cc); //skip keyword name
	if (cc->token!=TK_I64)
	  LexExcept(cc,"Expecting int at ");
	tempr->type=HTT_REG;
	tempr->reg_type=i;
	tempr->reg_num=cc->cur_i64;
	HashAdd(tempr,cmp.asm_hash);
	Lex(cc); //Skip INT
	break;
      case: //OPCODE
	if (cc->token!=TK_IDENT)
	  LexExcept(cc,"Expecting opcode at ");
	MemSet(tempo_max,0,size_max);
	tempo_max->type=HTT_OPCODE;
	tempo_max->inst_entry_cnt=0;
	tempo_max->str=cc->cur_str;
	cc->cur_str=0;
	Lex(cc);	//Skip OPCODE
	while (cc->token && cc->token!=';' && cc->token!=':') {
	  tempi=&tempo_max->ins[tempo_max->inst_entry_cnt];
	  tempi->ins_entry_num=tempo_max->inst_entry_cnt++;
	  tempi->slash_val=SV_NONE; //Not zero!!
	  while (cc->token==TK_I64) {
	    tempi->opcode[tempi->opcode_cnt++]=cc->cur_i64;
	    Lex(cc);
	  }
	  if (cc->token==',')
	    Lex(cc);
	  else if (cc->token!=';')
	    LexExcept(cc,"Expecting ',' at ");

	  AsmPrsInsFlags(cc,tempi);

	  tempi->uasm_slash_val=tempi->slash_val;
	  if (tempi->flags&IEF_STI_LIKE && tempi->slash_val!=SV_I_REG)
	    tempi->uasm_slash_val=SV_STI_LIKE;

	  tempi->arg1=tempi->arg2=tempi->size1=tempi->size2=0;
	  if (cc->token==TK_IDENT) {
	    j=DefineMatch(cc->cur_str,"ST_ARG_TYPES");
	    tempi->arg1=j;
	    if (Bt(&cmp.size_arg_mask[1],j))
	      tempi->size1=8;
	    else if (Bt(&cmp.size_arg_mask[2],j))
	      tempi->size1=16;
	    else if (Bt(&cmp.size_arg_mask[4],j))
	      tempi->size1=32;
	    else if (Bt(&cmp.size_arg_mask[8],j))
	      tempi->size1=64;

	    if (Lex(cc)==TK_IDENT) {
	      j=DefineMatch(cc->cur_str,"ST_ARG_TYPES");
	      Lex(cc);
	      tempi->arg2=j;
	      if (Bt(&cmp.size_arg_mask[1],j))
		tempi->size2=8;
	      else if (Bt(&cmp.size_arg_mask[2],j))
		tempi->size2=16;
	      else if (Bt(&cmp.size_arg_mask[4],j))
		tempi->size2=32;
	      else if (Bt(&cmp.size_arg_mask[8],j))
		tempi->size2=64;
	    }
	  }
	}
	size=offset(CHashOpcode.ins)+sizeof(CInst)*tempo_max->inst_entry_cnt;
	tempo=MAlloc(size);
	MemCpy(tempo,tempo_max,size);
	tempo->use_cnt=0;
	if (HashFind(tempo->str,cmp.asm_hash,HTT_OPCODE))
	  LexExcept(cc,"Duplicate OPCODE entry ");
	HashAdd(tempo,cmp.asm_hash);
	//Parse aliases.
	if (cc->token==':') {
	  while (Lex(cc)==TK_IDENT) {
	    tempo2=MAllocIdent(tempo);
	    tempo2->str=cc->cur_str;
	    cc->cur_str=0;
	    tempo2->oc_flags|=OCF_ALIAS;
	    if (HashFind(tempo2->str,cmp.asm_hash,HTT_OPCODE))
	      LexExcept(cc,"Duplicate OPCODE ALIAS entry ");
	    HashAdd(tempo2,cmp.asm_hash);
	  }
	}
	break;
      case: //KEYWORD
      case: //ASM_KEYWORD
	temph=CAlloc(sizeof(CHashGeneric));
	temph->str=cc->cur_str;
	cc->cur_str=NULL;
	Lex(cc); //skip keyword name
	if (cc->token!=TK_I64)
	  LexExcept(cc,"Expecting int at ");
	temph->user_data0=cc->cur_i64;
	if (i==10)
	  temph->type=HTT_KEYWORD;
	else
	  temph->type=HTT_ASM_KEYWORD;
	HashAdd(temph,cmp.asm_hash);
	Lex(cc); //Skip INT
	break;
    }
    if (cc->token!=';')
      LexExcept(cc,"Missing ';' at");
    Lex(cc); //Skip ';'
  }
  Free(tempo_max);
  CmpCtrlDel(cc);
  for (i=0;i<NUM_INTERNAL_TYPES;i++) {
    tempit=&internal_types_table[i];
    tempc=PrsClassNew;
    tempc->type=HTT_INTERNAL_TYPE;
    tempc->raw_type=tempit->type;
    Bts(&tempc->flags,Cf_INTERNAL_TYPE);
    tempc->size=tempit->size;
    tempc->str=AStrNew(tempit->name);
    HashAdd(tempc,cmp.asm_hash);
    cmp.internal_types[tempc->raw_type]=tempc;
  }
  adam_task->hash_table->next=cmp.asm_hash;
}
