U0 OptPass3(CCmpCtrl *cc,COptReg *reg_offsets)
{
  CHashClass *tempc,*tempc1,*tempc2;
  CCodeMisc *lb;
  CIntermediateCode *tempi,*tempi1,*tempi2,*tempi_next,
	*tempil1,*tempil2;
  I64 code,i,j,l,member_cnt,used_reg_mask=0;
  CMemberLst *tempm;
  COptMemberVar *mv=NULL;
  CAOT *tempaot;
  CAOTImportExport *tempie;
  CPrsStk *ps=cc->ps;
  ps->ptr=0;
  ps->ptr2=0;

  for (i=0;i<NUM_REGS;i++) {
    reg_offsets[i].offset=MAX_I64;
    reg_offsets[i].m=NULL;
  }
  if (cc->htc.fun) {
    member_cnt=cc->htc.fun->member_cnt;
    if (Bt(&cc->htc.fun->flags,Ff_DOT_DOT_DOT))
      member_cnt+=2;
    mv=CAlloc(member_cnt*sizeof(COptMemberVar));
    member_cnt=0;
    tempm=cc->htc.fun->member_lst;
    while (tempm) {
      tempc=OptClassFwd(tempm->member_class);
      if (0<=tempm->reg<NUM_REGS) {
	if (Bts(&used_reg_mask,tempm->reg))
	  PrintWarn("Reg in use\n $$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
		cc->lex_include_stk->full_name,
		cc->lex_include_stk->line_num,tempm->str,cc->htc.fun->str);
	reg_offsets[tempm->reg].offset=tempm->offset;
	reg_offsets[tempm->reg].m=tempm;
      } else if (tempc->raw_type!=RT_F64 && tempm->reg!=REG_NONE ||
	    tempm->reg==REG_ALLOC) {
	if (tempm->reg==REG_ALLOC)
	  mv[member_cnt].score=MAX_I64/2; //big but not too big
	mv[member_cnt].offset_start=tempm->offset;
	mv[member_cnt].offset_end=tempm->offset+MaxI64(1,tempm->size);
	mv[member_cnt++].m=tempm;
      } else {
	if (tempm->reg==REG_ALLOC)
	  PrintWarn("Can't reg var\n $$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
		cc->lex_include_stk->full_name,
		cc->lex_include_stk->line_num,tempm->str,cc->htc.fun->str);
      }
      tempm=tempm->next;
    }
  } else
    member_cnt=0;

  tempi=cc->coc.coc_root.next;
  while (code=tempi->ic_code) {
    tempi_next=tempi->next;
    if (code==IC_NOP1)
      OptFree(tempi);
    else {
      if (tempil1=OptLag(tempi)) {
	if (!(tempil2=OptLag(tempil1)))
	  tempil2=&cmp.ic_nop;
      } else
	tempil1=tempil2=&cmp.ic_nop;
      tempc=tempi->ic_class;
      nobound_switch (intermediate_code_table[code].arg_cnt) {
	case IS_V_ARG:
	  ps->ptr-=tempi->ic_data>>3;
	  break;
	case IS_2_ARG:
	  tempi2=PrsPop(ps);
	  if (tempi2->ic_flags&ICF_R_TO_F64)
	    tempc2=cmp.internal_types[RT_F64];
	  else if (tempi2->ic_flags & ICF_R_TO_INT)
	    tempc2=cmp.internal_types[RT_I64];
	  else
	    tempc2=tempi->t.a2c;
	  tempi->a2.type=TY_STK+CmpRawType(tempc2);
	  tempi->a2.type_pointed=CmpRawTypePointed(tempc2);
	  tempi->ic_flags|=ICF_A2_WAS_STK;
	case IS_1_ARG:
	  tempi1=PrsPop(ps);
	  if (tempi1->ic_flags&ICF_R_TO_F64)
	    tempc1=cmp.internal_types[RT_F64];
	  else if (tempi1->ic_flags & ICF_R_TO_INT)
	    tempc1=cmp.internal_types[RT_I64];
	  else
	    tempc1=tempi->t.a1c;
	  tempi->a1.type=TY_STK+CmpRawType(tempc1);
	  tempi->a1.type_pointed=CmpRawTypePointed(tempc1);
	  tempi->ic_flags|=ICF_A1_WAS_STK;
	  break;
	case IS_0_ARG: //nobound_switch
	  break;
      }
      if (intermediate_code_table[code].result_cnt) {
	tempi->r.type=TY_STK+CmpRawType(tempc);
	tempi->r.type_pointed=CmpRawTypePointed(tempc);
	tempi->ic_flags|=ICF_R_WAS_STK;
	PrsPush(ps,tempi);
      }
      nobound_switch (code) {
	case IC_IMM_F64:
	  tempi->ic_flags&=~ICF_R_TO_F64;
	  if (tempi->ic_flags&ICF_R_TO_INT) {
	    tempi->ic_data=ToI64(tempi->ic_data(F64));
	    tempi->ic_flags&=~ICF_R_TO_INT;
	    tempi->ic_code=IC_IMM_I64;
	  }
	  break;
	case IC_IMM_I64:
	  tempi->ic_flags&=~ICF_R_TO_INT;
	  if (tempi->ic_flags&ICF_R_TO_F64) {
	    tempi->ic_data(F64)=ToF64(tempi->ic_data);
	    tempi->ic_flags&=~ICF_R_TO_F64;
	    tempi->ic_code=IC_IMM_F64;
	  }
	  break;
	case IC_RBP:
	  tempi->ic_code=IC_REG;
	  tempi->a1.reg=REG_RBP;
	  break;
	case IC_DEREF:
	  tempi->a1.type_pointed=tempi->r.type&RT_MASK;
	  if (tempi1->ic_code==IC_LEA) {
	    tempi->ic_flags|=tempi1->ic_flags;
	    tempi->a1.reg=tempi1->a1.reg;
	    tempi->a1.disp=tempi1->a1.disp;
	    tempi->a1.type=TY_DISP+tempi->r.type&RT_MASK;
	    tempi->a1.type_pointed=tempi->r.type_pointed;
	    tempi->ic_code=IC_MOV;
	    OptFree(tempi1);
	    if (tempi->a1.reg==REG_RBP)
	      for (i=0;i<member_cnt;i++)
		if (mv[i].offset_start==tempi->a1.disp) {
		  mv[i].lea_balance--;
		  mv[i].score++;
		  break;
		}
	  } else if (tempil1->ic_code==IC_ADD_CONST) {
	    if (tempil2->ic_code==IC_REG) {
	      tempi->ic_flags|=tempil2->ic_flags|tempil1->ic_flags;
	      tempi->a1.reg=tempil2->a1.reg;
	      tempi->a1.disp=tempi->ic_data;
	      tempi->a1.type=TY_DISP+tempi->r.type&RT_MASK;
	      tempi->a1.type_pointed=tempi->r.type_pointed;
	      tempi->ic_code=IC_MOV;
	      OptFree(tempil2);
	      OptFree(tempil1);
	    }
	  }
	  break;
	case IC__PP:
	case IC__MM:
	case IC_PP_:
	case IC_MM_:
	  tempi->r.type_pointed=CmpRawTypePointed(tempi->t.a1c);
	case IC_DEREF_PP:
	case IC_DEREF_MM:
	  if (tempi1->ic_code==IC_LEA &&
		tempi1->a1.type&TY_MASK==TY_DISP &&
		tempi1->a1.reg==REG_RBP)
	    for (i=0;i<member_cnt;i++)
	      if (mv[i].offset_start==tempi1->a1.disp) {
		mv[i].lea_balance--;
		mv[i].score++;
		break;
	      }
	  tempi->a1.type_pointed=tempi->r.type&RT_MASK;
	  break;
	case IC_MUL:
	case IC_DIV:
	  if (tempc->raw_type==RT_F64) {
	    CmpF2PushPop(tempi,tempi1,tempi2);
	    break;
	  }
	  break;
	case IC_ADD:
	  if (tempc->raw_type==RT_F64) {
	    CmpF2PushPop(tempi,tempi1,tempi2);
	    break;
	  }
	  if (OptFixupBinaryOp2(&tempi1,&tempi2)) {
	    tempi->ic_flags|=tempi1->ic_flags;
	    if (tempi->t.a1t!=tempi2)
	      tempi->t.a1c=tempi->t.a2c;
	    tempi->ic_data=tempi1->ic_data;
	    tempi->ic_code=IC_ADD_CONST;
	    tempi->a1.type_pointed=tempi->r.type&RT_MASK;
	    tempi->a2.type=TY_NULL;
	    OptFree(tempi1);
	    if (tempil2->ic_code==IC_REG &&
		  tempil2->a1.reg==REG_RBP) {
	      tempi->ic_flags|=tempil2->ic_flags;
	      tempi->ic_code=IC_LEA;
	      tempi->a1.reg=REG_RBP;
	      tempi->a1.type=TY_DISP+tempi->a1.type&RT_MASK;
	      tempi->a1.disp=tempi->ic_data;
	      for (i=0;i<member_cnt;i++)
		if (mv[i].offset_start<=tempi->ic_data<mv[i].offset_end) {
		  mv[i].lea_balance++;
		  mv[i].score++;
		  break;
		}
	      OptFree(tempil2);
	    }
	  }
	  break;
	case IC_SUB:
	  if (tempc->raw_type==RT_F64) {
	    CmpF2PushPop(tempi,tempi1,tempi2);
	    break;
	  }
	  if (tempi2->ic_code==IC_IMM_I64) {
	    tempi->ic_flags|=tempi2->ic_flags;
	    tempi->ic_data=tempi2->ic_data;
	    tempi->ic_code=IC_SUB_CONST;
	    tempi->a2.type=TY_NULL;
	    OptFree(tempi2);
	  }
	  break;
	case IC_LESS:
	case IC_GREATER_EQUAL:
	case IC_GREATER:
	case IC_LESS_EQUAL:
	  if (tempi->ic_flags&ICF_USE_F64)
	    CmpF2PushPop(tempi,tempi1,tempi2);
	  break;
	case IC_MUL_EQUAL:
	case IC_DIV_EQUAL:
	case IC_ADD_EQUAL:
	case IC_SUB_EQUAL:
	  if (tempc->raw_type==RT_F64)
	    CmpF1PushPop(tempi,tempi2);
	case IC_ASSIGN_PP:
	case IC_ASSIGN_MM:
	case IC_ASSIGN:
	case IC_SHL_EQUAL:
	case IC_SHR_EQUAL:
	case IC_MOD_EQUAL:
	case IC_AND_EQUAL:
	case IC_OR_EQUAL:
	case IC_XOR_EQUAL:
	  if (tempi1->ic_code==IC_LEA &&
		tempi1->a1.type&TY_MASK==TY_DISP &&
		tempi1->a1.reg==REG_RBP)
	    for (i=0;i<member_cnt;i++)
	      if (mv[i].offset_start==tempi1->a1.disp) {
		mv[i].lea_balance--;
		mv[i].score++;
		break;
	      }
	  tempi->a1.type_pointed=tempi->r.type&RT_MASK;
	  tempi->r.type_pointed=CmpRawTypePointed(tempi->t.a1c);
	  break;
	case IC_RETURN_VAL:
	case IC_RETURN_VAL2:
	  if (tempc) {
	    if (tempc->raw_type==RT_F64 && tempil1->ic_class->raw_type!=RT_F64)
	      tempil1->ic_flags|=ICF_R_TO_F64;
	    else if (tempc->raw_type!=RT_F64 &&
		  tempil1->ic_class->raw_type==RT_F64)
	      tempil1->ic_flags|=ICF_R_TO_INT;
	  }
	  break;
	case IC_SQR:
	case IC_ABS:
	case IC_SQRT:
	case IC_SIN:
	case IC_COS:
	case IC_TAN:
	case IC_ATAN:
	  if (tempc->raw_type==RT_F64)
	    CmpF1PushPop(tempi,tempi1);
	  break;
	case IC_NOBOUND_SWITCH:
	case IC_SWITCH:
	  lb=OptLabelFwd(tempi->ic_data(CCodeMisc *)->dft);
	  lb->use_cnt++;
	  break;
	case IC_ASM:
	  tempaot=tempi->ic_data;
	  tempie=tempaot->next_ie;
	  while (tempie!=&tempaot->next_ie) {
	    if (IET_REL_I0<=tempie->type<=IET_IMM_I64 &&
		  tempie->str && tempie->flags&IEF_GOTO_LABEL &&
		  (lb=COCGoToLabelFind(cc,tempie->str)))
	      lb->use_cnt++; //Prevent deadcode elimination.
	    tempie=tempie->next;
	  }
	  break;
	case IC_BR_NOT_EQUAL:
	case IC_BR_EQUAL_EQUAL:
	  if ((tempi1->ic_code==IC_IMM_I64 ||
		tempi1->ic_code==IC_IMM_F64) && !tempi1->ic_data) {
	    OptFree(tempi1);
	    MemCpy(&tempi->a1,&tempi->a2,sizeof(CICArg));
	    tempi->a2.type=TY_NULL;
	    if (code==IC_BR_EQUAL_EQUAL)
	      code=tempi->ic_code=IC_BR_ZERO;
	    else
	      code=tempi->ic_code=IC_BR_NOT_ZERO;
	    tempi1=tempi2;
	    tempc1=tempc2;
	  } else if ((tempi2->ic_code==IC_IMM_I64 ||
		tempi2->ic_code==IC_IMM_F64) && !tempi2->ic_data) {
	    OptFree(tempi2);
	    tempi->a2.type=TY_NULL;
	    if (code==IC_BR_EQUAL_EQUAL)
	      code=tempi->ic_code=IC_BR_ZERO;
	    else
	      code=tempi->ic_code=IC_BR_NOT_ZERO;
	  } else
	    goto here1;
	case IC_BR_ZERO:
	case IC_BR_NOT_ZERO:
	  if (tempi1->ic_code==IC_IMM_I64 || tempi1->ic_code==IC_IMM_F64) {
	    if (code==IC_BR_ZERO ^^ tempi1->ic_data) {
	      OptFree(tempi1);
	      tempi->a1.type=TY_NULL;
	      tempi->ic_code=IC_JMP;
	    } else {
	      OptFree(tempi1);
	      tempi=OptFree(tempi);
	      break;
	    }
	  }
	  goto here1;
	case IC_BR_AND_ZERO:
	  if (tempi1->ic_code==IC_IMM_I64) {
	    i=Bsr(tempi1->ic_data);
	    if (0<=i==Bsf(tempi1->ic_data)) {
	      tempi1->ic_data=i;
	      tempi->ic_flags|=ICF_NOT_ADDR;
	      tempi->ic_code=IC_BR_NOT_BT;
	      goto here1;
	    }
	  }
	  if (tempi2->ic_code==IC_IMM_I64) {
	    i=Bsr(tempi2->ic_data);
	    if (0<=i==Bsf(tempi2->ic_data)) {
	      tempi2->ic_data=i;
	      tempi->ic_flags|=ICF_SWAP|ICF_NOT_ADDR;
	      tempi->ic_code=IC_BR_NOT_BT;
	    }
	  }
	  goto here1;
	case IC_BR_AND_NOT_ZERO:
	  if (tempi1->ic_code==IC_IMM_I64) {
	    i=Bsr(tempi1->ic_data);
	    if (0<=i==Bsf(tempi1->ic_data)) {
	      tempi1->ic_data=i;
	      tempi->ic_flags|=ICF_NOT_ADDR;
	      tempi->ic_code=IC_BR_BT;
	      goto here1;
	    }
	  }
	  if (tempi2->ic_code==IC_IMM_I64) {
	    i=Bsr(tempi2->ic_data);
	    if (0<=i==Bsf(tempi2->ic_data)) {
	      tempi2->ic_data=i;
	      tempi->ic_flags|=ICF_SWAP|ICF_NOT_ADDR;
	      tempi->ic_code=IC_BR_BT;
	    }
	  }
	  goto here1;
	case IC_BR_MM_ZERO:
	case IC_BR_MM_NOT_ZERO:
	  if (tempi1->ic_code==IC_LEA &&
		tempi1->a1.type&TY_MASK==TY_DISP &&
		tempi1->a1.reg==REG_RBP)
	    for (i=0;i<member_cnt;i++)
	      if (mv[i].offset_start==tempi1->a1.disp) {
		mv[i].lea_balance--;
		mv[i].score++;
		break;
	      }
	  tempi->a1.type_pointed=CmpRawType(tempc);
	  goto here1;
	case IC_BR_LESS:
	case IC_BR_GREATER_EQUAL:
	case IC_BR_GREATER:
	case IC_BR_LESS_EQUAL:
	  if (tempi->ic_flags&ICF_USE_F64)
	    CmpF2PushPop(tempi,tempi1,tempi2);
	case IC_BR_EQUAL_EQUAL2...IC_BR_LESS_EQUAL2:
	case IC_BR_CARRY:
	case IC_BR_NOT_CARRY:
	case IC_GET_LABEL:
	case IC_BR_BT:
	case IC_BR_BTS:
	case IC_BR_BTR:
	case IC_BR_BTC:
	case IC_BR_NOT_BT:
	case IC_BR_NOT_BTS:
	case IC_BR_NOT_BTR:
	case IC_BR_NOT_BTC:
	case IC_JMP:
	case IC_SUB_CALL:
here1:
	  if (tempi->ic_flags&ICF_PUSH_CMP)
	    lb=tempi->ic_data;
	  else
	    lb=OptLabelFwd(tempi->ic_data);
	  lb->use_cnt++;
	  break;
	case IC_NOP1:
	  tempi=OptFree(tempi);
	  break;
	case IC_NOP2:
	  ps->ptr+=tempi->ic_data;
	  break;
	case IC_SHL_CONST:
	case IC_SHR_CONST:
	case IC_ENTER:
	case IC_ADD_RSP:
	case IC_ADD_RSP1:
	case IC_CALL:
	case IC_CALL_INDIRECT:
	case IC_CALL_INDIRECT2:
	case IC_CALL_EXTERN:
	case IC_CALL_IMPORT:
	case IC_PUSH:
	case IC_POP:
	case IC_INVLPG:
	case IC_CLFLUSH:
	case IC_GET_RFLAGS:
	case IC_CARRY:
	case IC_RDTSC:
	case IC_HPET:
	case IC_SET_RFLAGS:
	case IC_GET_RBP:
	case IC_SET_RBP:
	case IC_GET_RSP:
	case IC_GET_RAX:
	case IC_SET_RSP:
	case IC_SET_RAX:
	case IC_ABS_ADDR:
	case IC_HEAP_GLBL:
	case IC_ADDR_IMPORT:
	case IC_TYPE:
	case IC_BT:
	case IC_BTS:
	case IC_BTR:
	case IC_BTC:
	case IC_LBTS:
	case IC_LBTR:
	case IC_LBTC:
	case IC_BSF:
	case IC_BSR:
	case IC_PREEMPT:
	case IC_SIGN_I64:
	case IC_TOUPPER:
	case IC_TO_I64:
	case IC_TO_F64:
	case IC_TO_BOOL:
	case IC_ABS_I64:
	case IC_MIN_I64:
	case IC_MAX_I64:
	case IC_MIN_U64:
	case IC_MAX_U64:
	case IC_MOD_U64:
	case IC_SQR_I64:
	case IC_SQR_U64:
	case IC_SWAP_U8:
	case IC_SWAP_U16:
	case IC_SWAP_U32:
	case IC_SWAP_I64:
	case IC_QUE_INIT:
	case IC_QUE_INS:
	case IC_QUE_INS_REV:
	case IC_QUE_REM:
	case IC_IN_U32:
	case IC_IN_U16:
	case IC_IN_U8:
	case IC_STRLEN:
	case IC_OUT_U32:
	case IC_OUT_U16:
	case IC_OUT_U8:
	case IC_STR_CONST:
	case IC_FS:
	case IC_GS:
	case IC_MOV_FS:
	case IC_MOV_GS:
	case IC_RIP:
	case IC_PUSH_CMP:
	case IC_REG:
	case IC_COM:
	case IC_HOLYC_TYPECAST:
	case IC_NOT:
	case IC_UNARY_MINUS:
	case IC_POWER:
	case IC_SHL:
	case IC_SHR:
	case IC_MOD:
	case IC_AND:
	case IC_OR:
	case IC_XOR:
	case IC_EQUAL_EQUAL:
	case IC_NOT_EQUAL:
	case IC_AND_AND:
	case IC_OR_OR:
	case IC_XOR_XOR:
	case IC_LEAVE:
	case IC_RET:
	case IC_ADDR:
	case IC_END:
	case IC_END_EXP:
	case IC_CALL_END:
	case IC_CALL_END2:
	case IC_CALL_START:
	case IC_PUSH_REGS:
	case IC_POP_REGS:
	case IC_LABEL:
	  break;
	default:
	  "Pass:%d Missing IC handler\n",cc->pass;
	  ICPut(cc,tempi);
	  LexExcept(cc,"Compiler Optimization Error at ");
      }
    }
    tempi=tempi_next;
  }
/* REGISTER VARIABLE ASSIGNMENT

We just scored num occurrences of each [RBP] offset in the code to help decide
which variables should be assigned to register variables.

We counted the times each offset was added to RBP as a plus LEA and we subtract
the times the offset is dereferenced.  If the address was calculated more times
than the offset was dereferenced, the variable's address was passed or assigned
and we cannot use a register because you can't take address of a reg var.

RAX,RBX,RCX,RDX are free to be clobbered by each intermediate code.  RAX links
intermediate codes together.  R8 and R9 are used for stack machine temporaries.
RBP is used as stack frame.

RSI,RDI,R10,R11,R12,R13,R14,R15 are used for reg vars.	R12 and R13, however,
have a unusual ModR addressing mode in the x86_64 architecture, so I only use
R12 and R13 as non-pointer register variables, such as index variables i,j,k.

*/
  if (cc->htc.fun) {
    cc->htc.fun->used_reg_mask=cc->htc.fun->used_reg_mask&
	  ~(REGG_C_LOCAL_VARS|REGG_C_LOCAL_NON_PTR_VARS)|used_reg_mask;
    if (!Bt(&cc->opts,OPTf_NO_REG_VAR) &&
	  !(cc->flags & CCF_NO_REG_OPT)) {
      QSort(mv,member_cnt,sizeof(COptMemberVar),&OptMVCompare);
      while (member_cnt && !mv[member_cnt-1].score)
	member_cnt--;
      j=0;
      for (i=0;i<member_cnt;i++) {
	if (!mv[i].lea_balance && mv[i].offset_start) {//addr operator cancels
	  mv[j].m=mv[i].m;
	  mv[j].offset_start=mv[i].offset_start;
	  mv[j++].offset_end=mv[i].offset_end;
	} else {
	  if (mv[i].m->reg==REG_ALLOC)
	    PrintWarn("Can't reg var\n $$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
		  cc->lex_include_stk->full_name,
		  cc->lex_include_stk->line_num,mv[i].m->str,cc->htc.fun->str);
	}
      }
      if (j>0) {
	if (Bt(&cc->flags,CCf_OPT_TRACE_PRESENT))
	  "Fun:%s\n",cc->htc.fun->str;
	if (j>cmp.num_reg_vars) {
	  l=0;
	  for (i=0;i<j && l<cmp.num_non_ptr_vars;i++) {
	    tempm=mv[i].m;
	    tempc=OptClassFwd(tempm->member_class);
	    if (!tempc->ptr_stars_cnt && !tempm->dim.next) {
	      while (l<cmp.num_non_ptr_vars &&
		    Bts(&cc->htc.fun->used_reg_mask,cmp.non_ptr_vars_map[l]))
		l++;
	      if (l<cmp.num_non_ptr_vars) {
		tempm->reg=cmp.non_ptr_vars_map[l++];
		reg_offsets[tempm->reg].offset=mv[i].offset_start;
		reg_offsets[tempm->reg].m=tempm;
		if (Bt(&cc->flags,CCf_OPT_TRACE_PRESENT))
		  "Reg %Z Var \"%-15ts\" %016X[RBP]\n",tempm->reg,"ST_U64_REGS",
			tempm->str,reg_offsets[tempm->reg].offset;
		mv[i].offset_start=0; //flag as reg var
		if (tempm->size<8 &&
		      !StrIMatch("Bool",tempm->member_class->str) &&
		      tempm->member_class->type&HTT_INTERNAL_TYPE)
		  PrintWarn("Using 64-bit reg var.\n "
			"$$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
			cc->lex_include_stk->full_name,
			cc->lex_include_stk->line_num,
			tempm->str,cc->htc.fun->str);
	      }
	    }
	  }
	}
	l=0;
	for (i=0;i<j && l<cmp.num_reg_vars;i++) {
	  tempm=mv[i].m;
//if not just flagged as reg var
	  if (mv[i].offset_start && (!mv[i].m->dim.next||
		tempm->offset>0 && StrCmp(tempm->str,"argv"))) {
	    while (l<cmp.num_reg_vars &&
		  Bts(&cc->htc.fun->used_reg_mask,cmp.to_reg_vars_map[l]))
	      l++;
	    if (l<cmp.num_reg_vars) {
	      tempm->reg=cmp.to_reg_vars_map[l++];
	      reg_offsets[tempm->reg].offset=mv[i].offset_start;
	      reg_offsets[tempm->reg].m=tempm;
	      if (Bt(&cc->flags,CCf_OPT_TRACE_PRESENT))
		"Reg %Z Var \"%-15ts\" %016X[RBP]\n",tempm->reg,"ST_U64_REGS",
		      tempm->str,reg_offsets[tempm->reg].offset;
	      if (tempm->size<8 &&
		    !StrIMatch("Bool",tempm->member_class->str) &&
		    tempm->member_class->type&HTT_INTERNAL_TYPE)
		PrintWarn("Using 64-bit reg var.\n "
		      "$$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
		      cc->lex_include_stk->full_name,
		      cc->lex_include_stk->line_num,
		      tempm->str,cc->htc.fun->str);
	    }
	  }
	}
      }
    }
    Free(mv);
  }
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",cc->pass,ps->ptr;
    LexExcept(cc,"Compiler Optimization Error at ");
  }
}

