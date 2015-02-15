#define NUM_INTERNAL_TYPES	17

class CInternalType
{
  U8 type,size,name[8];
};

CInternalType internal_types_table[NUM_INTERNAL_TYPES]={
{RT_I0,0,"I0i"},{RT_I0,0,"I0"},{RT_U0,0,"U0i"},{RT_U0,0,"U0"},
{RT_I8,1,"I8i"},{RT_I8,1,"I8"},{RT_I8,1,"Bool"},
{RT_U8,1,"U8i"},{RT_U8,1,"U8"},
{RT_I16,2,"I16i"},{RT_U16,2,"U16i"},
{RT_I32,4,"I32i"},{RT_U32,4,"U32i"},
{RT_I64,8,"I64i"},{RT_U64,8,"U64i"},
{RT_F64,8,"F64i"},{RT_F64,8,"F64"},
};

U0 AsmPrsInsFlags(CLex *lx,CInstruction *tempi)
{
  I64 i;
  while (TRUE) {
    switch (lx->token) {
      case TK_IDENT:
	if ((i=DefineMatch(lx->cur_str,"ST_OPCODE_MODIFIERS"))>=0) {
	  tempi->opcode_modifier=i;
	  break;
	} else
	  return;
      case TK_INTEGER:
	if (lx->cur_i64==16)
	  tempi->flags|=IEF_OP_SIZE16;
	else if (lx->cur_i64==32)
	  tempi->flags|=IEF_OP_SIZE32;
	else
	  return;
	break;
      case '+':
	tempi->flags|=IEF_PLUS_OPCODE;
      case '/':
	if (Lex(lx)==TK_INTEGER && lx->cur_i64<8)
	  tempi->slash_val=lx->cur_i64;
	else if (lx->token==TK_IDENT) {
	  if (!StrCmp(lx->cur_str,"R"))
	    tempi->slash_val=SV_R_REG;
	  else if (!StrCmp(lx->cur_str,"I"))
	    tempi->slash_val=SV_I_REG;
	  else
	    return;
	} else
	  return;
	break;
      case '!': tempi->flags|=IEF_DONT_SWITCH_MODES; break;
      case '&': tempi->flags|=IEF_DFT;		break;
      case ':': tempi->flags|=IEF_STI;		break;
      case '-': tempi->flags|=IEF_NO_REX;	break;
      case '~': tempi->flags|=IEF_40_REX;	break;
      case '=': tempi->flags|=IEF_48_REX;	break;
      case '`': tempi->flags|=IEF_REX_ONLY_R8_R15; break;
      case '^': tempi->flags|=IEF_REX_XOR_LIKE;	break;
      case '$$': tempi->flags|=IEF_ENDING_ZERO;	break;
      default:
	return;
    }
    Lex(lx);
  }
}

U0 AsmHashLoad()
{
  I64 i,j,size,size1;
  CInternalType *tempit;
  CLex *lx;
  CHashGeneric *temph;
  CHashReg *tempr;
  CHashOpcode *tempo,*tempo1;
  CInstruction *tempi;
  CHashClass *tempc;

  cmp.asm_hash=HashTableNew(1024);
  size1=offset(CHashOpcode.ins)+sizeof(CInstruction)<<5;
  tempo=MAlloc(size1);

  lx=LexNew(FileRead("OpCodes.TXT"),,"OpCodes.TXT.Z");
  lx->htc.hash_table_lst=NULL;
  Lex(lx);
  while (lx->token) {
    if (lx->token!=TK_IDENT)
      LexExcept(lx,"expecting identifier at ");
    i=LstMatch(lx->cur_str,"R8\0R16\0R32\0R64\0SEG\0FSTK\0MM\0XMM\0OPCODE\0KEYWORD\0ASM_KEYWORD\0");
    if (i<0)
      LexExcept(lx,"Unknown Statement");
    Lex(lx); //skip keyword
    if (lx->token!=TK_IDENT)
      LexExcept(lx,"expecting identifier at ");
    switch (i) {
      case 8: //OPCODE
	MemSet(tempo,0,size1);
	tempo->type=HTT_OPCODE;
	tempo->instruction_entry_cnt=0;
	tempo->str=lx->cur_str;
	lx->cur_str=0;
	Lex(lx);	//Skip OPCODE
	while (lx->token && lx->token!=';') {
	  tempi=&tempo->ins[tempo->instruction_entry_cnt];
	  tempi->ins_entry_num=tempo->instruction_entry_cnt++;
	  tempi->slash_val=SV_NONE; //Not zero!!
	  while (lx->token==TK_INTEGER) {
	    tempi->opcode[tempi->opcode_cnt++]=lx->cur_i64;
	    Lex(lx);
	  }
	  if (lx->token==',')
	    Lex(lx);
	  else if (lx->token!=';')
	    LexExcept(lx,"expecting ',' at ");

	  AsmPrsInsFlags(lx,tempi);

	  tempi->arg1=0;
	  tempi->arg2=0;
	  if (lx->token==TK_IDENT) {
	    j=DefineMatch(lx->cur_str,"ST_ARG_TYPES");
	    tempi->arg1=j;
	    if (ARGT_REL8<=j<=ARGT_REL32 ||
		ARGT_MOFFS8<=j<=ARGT_MOFFS64)
	      tempi|=IEF_NO_REX;

	    if (Lex(lx)==TK_IDENT) {
	      j=DefineMatch(lx->cur_str,"ST_ARG_TYPES");
	      Lex(lx);
	      tempi->arg2=j;
	      if (ARGT_REL8<=j<=ARGT_REL32 ||
	      ARGT_MOFFS8<=j<=ARGT_MOFFS64)
		tempi|=IEF_NO_REX;
	    }
	  }
	}
	size=offset(CHashOpcode.ins)+
	  sizeof(CInstruction)*tempo->instruction_entry_cnt;

	tempo1=MAlloc(size);
	MemCpy(tempo1,tempo,size);
	tempo1->use_cnt=0;
	if (HashFind(tempo1->str,cmp.asm_hash,HTT_OPCODE))
	  LexExcept(lx,"Duplicate OPCODE entry ");
	HashAdd(tempo1,cmp.asm_hash);
	break;
      case 9:  //KEYWORD
      case 10: //ASM_KEYWORD
	temph=CAlloc(sizeof(CHashGeneric));
	temph->str=lx->cur_str;
	lx->cur_str=NULL;
	Lex(lx); //skip keyword name
	if (lx->token!=TK_INTEGER)
	  LexExcept(lx,"expecting integer at ");
	temph->user_data0=lx->cur_i64;
	if (i==9)
	  temph->type=HTT_KEYWORD;
	else
	  temph->type=HTT_ASM_KEYWORD;
	HashAdd(temph,cmp.asm_hash);
	Lex(lx); //Skip INT
	break;
      case REGT_R8:
      case REGT_R16:
      case REGT_R32:
      case REGT_R64:
      case REGT_SEG:
      case REGT_FSTK:
      case REGT_MM:
      case REGT_XMM:
	tempr=CAlloc(sizeof(CHashReg));
	tempr->str=lx->cur_str;
	lx->cur_str=NULL;
	Lex(lx); //skip keyword name
	if (lx->token!=TK_INTEGER)
	  LexExcept(lx,"expecting integer at ");
	tempr->type=HTT_REG;
	tempr->reg_type=i;
	tempr->reg_num=lx->cur_i64;
	HashAdd(tempr,cmp.asm_hash);
	Lex(lx); //Skip INT
	break;
    }
    if (lx->token!=';')
      LexExcept(lx,"missing ';' at");
    Lex(lx); //Skip ';'
  }
  Free(tempo);
  LexDel(lx);
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

U0 CmpLoadDefines()
{
  DefineLstLoad("ST_RAW_TYPES",
	" \0 \0I0\0U0\0I8\0U8\0I16\0U16\0I32\0U32\0I64\0U64\0"
	"F32\0UF32\0F64\0UF64\0");
  DefineLstLoad("ST_TY_TYPES","NULL\0IMM\0REG\0DISP\0SIB\0RIP\0STK\0");
  DefineLstLoad("ST_U8_REGS","AL\0CL\0DL\0BL\0AH\0CH\0DH\0BH\0"
			"R8u8\0R9u8\0R10u8\0R11u8\0R12u8\0R13u8\0R14u8\0R15u8\0");
  DefineLstLoad("ST_U8_REX_REGS","AL\0CL\0DL\0BL\0RSPu8\0RBPu8\0RSIu8\0RDIu8\0"
			"R8u8\0R9u8\0R10u8\0R11u8\0R12u8\0R13u8\0R14u8\0R15u8\0");
  DefineLstLoad("ST_U16_REGS","AX\0CX\0DX\0BX\0SP\0BP\0SI\0DI\0"
			"R8u16\0R9u16\0R10u16\0R11u16\0R12u16\0R13u16\0R14u16\0R15u16\0");
  DefineLstLoad("ST_U32_REGS","EAX\0ECX\0EDX\0EBX\0ESP\0EBP\0ESI\0EDI\0"
			"R8u32\0R9u32\0R10u32\0R11u32\0R12u32\0R13u32\0R14u32\0R15u32\0");
  DefineLstLoad("ST_U64_REGS","RAX\0RCX\0RDX\0RBX\0RSP\0RBP\0RSI\0RDI\0"
			"R8\0R9\0R10\0R11\0R12\0R13\0R14\0R15\0");
  DefineLstLoad("ST_MM_REGS","MM0\0MM1\0MM2\0MM3\0MM4\0MM5\0MM6\0MM7\0");
  DefineLstLoad("ST_XMM_REGS","XMM0\0XMM1\0XMM2\0XMM3\0XMM4\0XMM5\0XMM6\0XMM7\0");
  DefineLstLoad("ST_OPCODE_MODIFIERS","NO\0CB\0CW\0CD\0CP\0IB\0IW\0ID\0");
  DefineLstLoad("ST_ARG_TYPES","NO\0REL8\0REL16\0REL32\0"
		"IMM8\0IMM16\0IMM32\0IMM64\0UIMM8\0UIMM16\0UIMM32\0UIMM64\0"
		"R8\0R16\0R32\0R64\0RM8\0RM16\0RM32\0RM64\0"
		"M8\0M16\0M32\0M64\0M1632\0M16N32\0M16N16\0M32N32\0"
		"MOFFS8\0MOFFS16\0MOFFS32\0MOFFS64\0"
		"AL\0AX\0EAX\0RAX\0CL\0DX\0 \0SREG\0"
		"SS\0DS\0ES\0FS\0GS\0CS\0ST0\0STI\0"
		"MM\0MM32\0MM64\0XMM\0XMM32\0XMM64\0XMM128\0XMM0\0");
  DefineLstLoad("ST_SEG_REGS","ES\0CS\0SS\0DS\0FS\0GS\0");
  DefineLstLoad("ST_FSTK_REGS","ST0\0ST1\0ST2\0ST3\0ST4\0ST5\0ST6\0ST7\0");
}

U0 CmpFillTables()
{
  I64 i,k;
  U32 *d;

  cmp.to_reg_vars_map=CAlloc(NUM_REGS*sizeof(U8));
  cmp.num_reg_vars=0;
  cmp.reg_vars_mask=REG_VARS_MASK;
  for (i=0;i<NUM_REGS;i++)
    if (Bt(&cmp.reg_vars_mask,i))
      cmp.to_reg_vars_map[cmp.num_reg_vars++]=i;

  cmp.non_ptr_vars_map=CAlloc(NUM_REGS*sizeof(U8));
  cmp.num_non_ptr_vars=0;
  cmp.non_ptr_vars_mask=REG_NON_PTR_MASK;
  for (i=0;i<NUM_REGS;i++)
    if (Bt(&cmp.non_ptr_vars_mask,i))
      cmp.non_ptr_vars_map[cmp.num_non_ptr_vars++]=i;

/* STK TEMP ALLOCATION: See $LK,"::/Compiler/OptPass6.CPP"$.

Expressions are evaluated with a stack machine.

1+(2+3)*4


STK[2]: 1    1	 1   1	 1   1	 21
STK[1]:      2	 2   5	 5  20
STK[0]: 	 3	 4

The top of the stack is stored in RAX.

STK[2]: RAX  1	 1   1	 1   1	 RAX
STK[1]:     RAX  2  RAX  5  RAX
STK[0]: 	RAX	RAX

Two regs are alloced for stk temps.

STK[0]:
   R9 : RAX R9	R9  R9	R9  R9	 RAX
   R8 :     RAX R8  RAX R8  RAX
      : 	RAX	RAX

Every other funtion, the regs are reversed.  This is to reduce
pushes before function calls.  R8 is used in even numbered functions
while R9 is used in odd, so maybe we can avoid pushes if only one
stk temp reg is used.

STK[0]:
   R9 : RAX R8	R8  R8	R8  R8	 RAX
   R8 :     RAX R9  RAX R9  RAX
      : 	RAX	RAX
*/
  cmp.to_stk_temps_map_odd =CAlloc(NUM_REGS*sizeof(U8));
  cmp.to_stk_temps_map_even=CAlloc(NUM_REGS*sizeof(U8));
  cmp.num_stk_temps=0;
  cmp.stk_temps_mask=REG_STK_TEMPS_MASK;
  for (i=0;i<NUM_REGS;i++)
    if (Bt(&cmp.stk_temps_mask,i))
      cmp.to_stk_temps_map_even[cmp.num_stk_temps++]=i;
  k=cmp.num_stk_temps;
  for (i=0;i<NUM_REGS;i++)
    if (Bt(&cmp.stk_temps_mask,i))
      cmp.to_stk_temps_map_odd[--k]=i;

  cmp.dual_U16_tokens1=d=CAlloc(sizeof(U32)*TK_NUM_TK);
  d['!']=TK_NOT_EQUAL<<16+'=';
  d['&']=TK_AND_AND<<16+'&';
  d['*']=TK_MUL_EQUAL<<16+'=';
  d['+']=TK_PLUS_PLUS<<16+'+';
  d['-']=TK_DEREFERENCE<<16+'>';
  d['/']='*';
  d[':']=TK_DBL_COLON<<16+':';
  d['<']=TK_LESS_EQUAL<<16+'=';
  d['=']=TK_EQUAL_EQUAL<<16+'=';
  d['>']=TK_GREATER_EQUAL<<16+'=';
  d['^']=TK_XOR_EQUAL<<16+'=';
  d['|']=TK_OR_OR<<16+'|';
  d['%']=TK_MOD_EQUAL<<16+'=';

  cmp.dual_U16_tokens2=d=CAlloc(sizeof(U32)*TK_NUM_TK);
  d['&']=TK_AND_EQUAL<<16+'=';
  d['+']=TK_ADD_EQUAL<<16+'=';
  d['-']=TK_MINUS_MINUS<<16+'-';
  d['/']='/';
  d['<']=TK_SHL<<16+'<';
  d['>']=TK_SHR<<16+'>';
  d['^']=TK_XOR_XOR<<16+'^';
  d['|']=TK_OR_EQUAL<<16+'=';

  cmp.dual_U16_tokens3=d=CAlloc(sizeof(U32)*TK_NUM_TK);
  d['-']=TK_SUB_EQUAL<<16+'=';
  d['/']=TK_DIV_EQUAL<<16+'=';

  cmp.binary_ops=d=CAlloc(sizeof(U32)*TK_NUM_TK);
  d['`']   =(PREC_EXP+ASSOCF_RIGHT)<<16+IC_POWER;
  d[TK_SHL]=(PREC_EXP+ASSOCF_LEFT)<<16+IC_SHL;
  d[TK_SHR]=(PREC_EXP+ASSOCF_LEFT)<<16+IC_SHR;

  d['*']=PREC_MUL<<16+IC_MUL;
  d['/']=(PREC_MUL+ASSOCF_LEFT)<<16+IC_DIV;
  d['%']=(PREC_MUL+ASSOCF_LEFT)<<16+IC_MOD;

  d['&']=PREC_AND<<16+IC_AND;

  d['^']=PREC_XOR<<16+IC_XOR;

  d['|']=PREC_OR<<16+IC_OR;

  d['+']=PREC_ADD<<16+IC_ADD;
  d['-']=(PREC_ADD+ASSOCF_LEFT)<<16+IC_SUB;

  d['<']		=PREC_CMP<<16+IC_LESS;
  d['>']		=PREC_CMP<<16+IC_GREATER;
  d[TK_LESS_EQUAL]	=PREC_CMP<<16+IC_LESS_EQUAL;
  d[TK_GREATER_EQUAL]	=PREC_CMP<<16+IC_GREATER_EQUAL;

  d[TK_EQUAL_EQUAL]	=PREC_CMP2<<16+IC_EQUAL_EQUAL;
  d[TK_NOT_EQUAL]	=PREC_CMP2<<16+IC_NOT_EQUAL;

  d[TK_AND_AND]	=PREC_AND_AND<<16+IC_AND_AND;

  d[TK_XOR_XOR]	=PREC_XOR_XOR<<16+IC_XOR_XOR;

  d[TK_OR_OR]	=PREC_OR_OR<<16+IC_OR_OR;

  d['=']	 =(PREC_ASSIGN+ASSOCF_RIGHT)<<16+IC_ASSIGN;
  d[TK_SHL_EQUAL]=(PREC_ASSIGN+ASSOCF_RIGHT)<<16+IC_SHL_EQUAL;
  d[TK_SHR_EQUAL]=(PREC_ASSIGN+ASSOCF_RIGHT)<<16+IC_SHR_EQUAL;
  d[TK_MUL_EQUAL]=(PREC_ASSIGN+ASSOCF_RIGHT)<<16+IC_MUL_EQUAL;
  d[TK_DIV_EQUAL]=(PREC_ASSIGN+ASSOCF_RIGHT)<<16+IC_DIV_EQUAL;
  d[TK_MOD_EQUAL]=(PREC_ASSIGN+ASSOCF_RIGHT)<<16+IC_MOD_EQUAL;
  d[TK_AND_EQUAL]=(PREC_ASSIGN+ASSOCF_RIGHT)<<16+IC_AND_EQUAL;
  d[TK_OR_EQUAL] =(PREC_ASSIGN+ASSOCF_RIGHT)<<16+IC_OR_EQUAL;
  d[TK_XOR_EQUAL]=(PREC_ASSIGN+ASSOCF_RIGHT)<<16+IC_XOR_EQUAL;
  d[TK_ADD_EQUAL]=(PREC_ASSIGN+ASSOCF_RIGHT)<<16+IC_ADD_EQUAL;
  d[TK_SUB_EQUAL]=(PREC_ASSIGN+ASSOCF_RIGHT)<<16+IC_SUB_EQUAL;
}
