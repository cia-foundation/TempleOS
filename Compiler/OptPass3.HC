U0 OptPass3(CCmpCtrl *cc,COptReg *reg_offsets)
{
  CHashClass *tmpc,*tmpc1,*tmpc2;
  CCodeMisc *lb;
  CIntermediateCode *tmpi,*tmpi1,*tmpi2,*tmpi_next,
	*tmpil1,*tmpil2;
  I64 code,i,j,l,member_cnt,used_reg_mask=0;
  CMemberLst *tmpm;
  COptMemberVar *mv=NULL;
  CAOT *tmpaot;
  CAOTImportExport *tmpie;
  CPrsStk *ps=cc->ps;
  ps->ptr=0;
  ps->ptr2=0;

  for (i=0;i<REG_REGS_NUM;i++) {
    reg_offsets[i].offset=I64_MAX;
    reg_offsets[i].m=NULL;
  }
  if (cc->htc.fun) {
    member_cnt=cc->htc.fun->member_cnt;
    if (Bt(&cc->htc.fun->flags,Ff_DOT_DOT_DOT))
      member_cnt+=2;
    mv=CAlloc(member_cnt*sizeof(COptMemberVar));
    member_cnt=0;
    tmpm=cc->htc.fun->member_lst_and_root;
    while (tmpm) {
      tmpc=OptClassFwd(tmpm->member_class);
      if (0<=tmpm->reg<REG_REGS_NUM) {
	if (Bts(&used_reg_mask,tmpm->reg))
	  PrintWarn("Reg in use\n $$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
		cc->lex_include_stk->full_name,
		cc->lex_include_stk->line_num,tmpm->str,cc->htc.fun->str);
	reg_offsets[tmpm->reg].offset=tmpm->offset;
	reg_offsets[tmpm->reg].m=tmpm;
      } else if (tmpc->raw_type!=RT_F64 && tmpm->reg!=REG_NONE ||
	    tmpm->reg==REG_ALLOC) {
	if (tmpm->reg==REG_ALLOC)
	  mv[member_cnt].score=I64_MAX/2; //big but not too big
	mv[member_cnt].offset_start=tmpm->offset;
	mv[member_cnt].offset_end=tmpm->offset+MaxI64(1,tmpm->size);
	mv[member_cnt++].m=tmpm;
      } else if (tmpm->reg==REG_ALLOC)
	PrintWarn("Can't reg var\n $$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
	      cc->lex_include_stk->full_name,
	      cc->lex_include_stk->line_num,tmpm->str,cc->htc.fun->str);
      tmpm=tmpm->next;
    }
  } else
    member_cnt=0;

  tmpi=cc->coc.coc_head.next;
  while (code=tmpi->ic_code) {
    tmpi_next=tmpi->next;
    if (code==IC_NOP1)
      OptFree(tmpi);
    else {
      if (tmpil1=OptLag(tmpi)) {
	if (!(tmpil2=OptLag(tmpil1)))
	  tmpil2=&cmp.ic_nop;
      } else
	tmpil1=tmpil2=&cmp.ic_nop;
      tmpc=tmpi->ic_class;
      switch [intermediate_code_table[code].arg_cnt] {
	case IS_V_ARG:
	  ps->ptr-=tmpi->ic_data>>3;
	  break;
	case IS_2_ARG:
	  tmpi2=PrsPop(ps);
	  if (tmpi2->ic_flags&ICF_RES_TO_F64)
	    tmpc2=cmp.internal_types[RT_F64];
	  else if (tmpi2->ic_flags & ICF_RES_TO_INT)
	    tmpc2=cmp.internal_types[RT_I64];
	  else
	    tmpc2=tmpi->t.arg2_class;
	  tmpi->arg2.type=MDF_STK+CmpRawType(tmpc2);
	  tmpi->ic_flags|=ICF_ARG2_WAS_STK;
	case IS_1_ARG:
	  tmpi1=PrsPop(ps);
	  if (tmpi1->ic_flags&ICF_RES_TO_F64)
	    tmpc1=cmp.internal_types[RT_F64];
	  else if (tmpi1->ic_flags & ICF_RES_TO_INT)
	    tmpc1=cmp.internal_types[RT_I64];
	  else
	    tmpc1=tmpi->t.arg1_class;
	  tmpi->arg1.type=MDF_STK+CmpRawType(tmpc1);
	  tmpi->arg1_type_pointed_to=CmpRawTypePointed(tmpc1);
	  tmpi->ic_flags|=ICF_ARG1_WAS_STK;
	  break;
	case IS_0_ARG: //nobound switch
	  break;
      }
      if (intermediate_code_table[code].res_cnt) {
	tmpi->res.type=MDF_STK+CmpRawType(tmpc);
	tmpi->ic_flags|=ICF_RES_WAS_STK;
	PrsPush(ps,tmpi);
      }
      switch [code] {
	case IC_IMM_F64:
	  tmpi->ic_flags&=~ICF_RES_TO_F64;
	  if (tmpi->ic_flags&ICF_RES_TO_INT) {
	    tmpi->ic_data=ToI64(tmpi->ic_data(F64));
	    tmpi->ic_flags&=~ICF_RES_TO_INT;
	    tmpi->ic_code=IC_IMM_I64;
	  }
	  break;
	case IC_IMM_I64:
	  tmpi->ic_flags&=~ICF_RES_TO_INT;
	  if (tmpi->ic_flags&ICF_RES_TO_F64) {
	    tmpi->ic_data(F64)=ToF64(tmpi->ic_data);
	    tmpi->ic_flags&=~ICF_RES_TO_F64;
	    tmpi->ic_code=IC_IMM_F64;
	  }
	  break;
	case IC_RBP:
	  tmpi->ic_code=IC_REG;
	  tmpi->arg1.reg=REG_RBP;
	  break;
	case IC_DEREF:
	  tmpi->arg1_type_pointed_to=tmpi->res.type.raw_type;
	  if (tmpi1->ic_code==IC_LEA) {
	    tmpi->ic_flags|=tmpi1->ic_flags;
	    tmpi->arg1.reg=tmpi1->arg1.reg;
	    tmpi->arg1.disp=tmpi1->arg1.disp;
	    tmpi->arg1.type=MDF_DISP+tmpi->res.type.raw_type;
	    tmpi->arg1_type_pointed_to=CmpRawTypePointed(tmpc);
	    tmpi->ic_code=IC_MOV;
	    OptFree(tmpi1);
	    if (tmpi->arg1.reg==REG_RBP)
	      for (i=0;i<member_cnt;i++)
		if (mv[i].offset_start==tmpi->arg1.disp) {
		  mv[i].lea_balance--;
		  mv[i].score++;
		  break;
		}
	  } else if (tmpil1->ic_code==IC_ADD_CONST) {
	    if (tmpil2->ic_code==IC_REG) {
	      tmpi->ic_flags|=tmpil2->ic_flags|tmpil1->ic_flags;
	      tmpi->arg1.reg=tmpil2->arg1.reg;
	      tmpi->arg1.disp=tmpi->ic_data;
	      tmpi->arg1.type=MDF_DISP+tmpi->res.type.raw_type;
	      tmpi->arg1_type_pointed_to=CmpRawTypePointed(tmpc);
	      tmpi->ic_code=IC_MOV;
	      OptFree(tmpil2);
	      OptFree(tmpil1);
	    }
	  }
	  break;
	case IC__PP:
	case IC__MM:
	case IC_PP_:
	case IC_MM_:
	  CmpMinTypePointed(tmpi,CmpRawTypePointed(tmpi->t.arg1_class));
	case IC_DEREF_PP:
	case IC_DEREF_MM:
	  if (tmpi1->ic_code==IC_LEA && tmpi1->arg1.type&MDF_DISP &&
		tmpi1->arg1.reg==REG_RBP)
	    for (i=0;i<member_cnt;i++)
	      if (mv[i].offset_start==tmpi1->arg1.disp) {
		mv[i].lea_balance--;
		mv[i].score++;
		break;
	      }
	  tmpi->arg1_type_pointed_to=tmpi->res.type.raw_type;
	  break;
	case IC_MUL:
	case IC_DIV:
	  if (tmpc->raw_type==RT_F64) {
	    CmpF2PushPop(tmpi,tmpi1,tmpi2);
	    break;
	  }
	  break;
	case IC_ADD:
	  if (tmpc->raw_type==RT_F64) {
	    CmpF2PushPop(tmpi,tmpi1,tmpi2);
	    break;
	  }
	  if (OptFixupBinaryOp2(&tmpi1,&tmpi2)) {
	    tmpi->ic_flags|=tmpi1->ic_flags;
	    if (tmpi->t.arg1_tree!=tmpi2)
	      tmpi->t.arg1_class=tmpi->t.arg2_class;
	    tmpi->ic_data=tmpi1->ic_data;
	    tmpi->ic_code=IC_ADD_CONST;
	    tmpi->arg1_type_pointed_to=tmpi->res.type.raw_type;
	    tmpi->arg2.type=MDF_NULL;
	    OptFree(tmpi1);
	    if (tmpil2->ic_code==IC_REG && tmpil2->arg1.reg==REG_RBP) {
	      tmpi->ic_flags|=tmpil2->ic_flags;
	      tmpi->ic_code=IC_LEA;
	      tmpi->arg1.reg=REG_RBP;
	      tmpi->arg1.type=MDF_DISP+tmpi->arg1.type.raw_type;
	      tmpi->arg1.disp=tmpi->ic_data;
	      for (i=0;i<member_cnt;i++)
		if (mv[i].offset_start<=tmpi->ic_data<mv[i].offset_end) {
		  mv[i].lea_balance++;
		  mv[i].score++;
		  break;
		}
	      OptFree(tmpil2);
	    }
	  }
	  break;
	case IC_SUB:
	  if (tmpc->raw_type==RT_F64) {
	    CmpF2PushPop(tmpi,tmpi1,tmpi2);
	    break;
	  }
	  if (tmpi2->ic_code==IC_IMM_I64) {
	    tmpi->ic_flags|=tmpi2->ic_flags;
	    tmpi->ic_data=tmpi2->ic_data;
	    tmpi->ic_code=IC_SUB_CONST;
	    tmpi->arg2.type=MDF_NULL;
	    OptFree(tmpi2);
	  }
	  break;
	case IC_LESS:
	case IC_GREATER_EQU:
	case IC_GREATER:
	case IC_LESS_EQU:
	  if (tmpi->ic_flags&ICF_USE_F64)
	    CmpF2PushPop(tmpi,tmpi1,tmpi2);
	  break;
	case IC_MUL_EQU:
	case IC_DIV_EQU:
	case IC_ADD_EQU:
	case IC_SUB_EQU:
	  if (tmpc->raw_type==RT_F64)
	    CmpF1PushPop(tmpi,tmpi2);
	case IC_ASSIGN_PP:
	case IC_ASSIGN_MM:
	case IC_ASSIGN:
	case IC_SHL_EQU:
	case IC_SHR_EQU:
	case IC_MOD_EQU:
	case IC_AND_EQU:
	case IC_OR_EQU:
	case IC_XOR_EQU:
	  if (tmpi1->ic_code==IC_LEA && tmpi1->arg1.type&MDF_DISP &&
		tmpi1->arg1.reg==REG_RBP)
	    for (i=0;i<member_cnt;i++)
	      if (mv[i].offset_start==tmpi1->arg1.disp) {
		mv[i].lea_balance--;
		mv[i].score++;
		break;
	      }
	  tmpi->arg1_type_pointed_to=tmpi->res.type.raw_type;
	  CmpMinTypePointed(tmpi,CmpRawTypePointed(tmpi->t.arg1_class));
	  break;
	case IC_RETURN_VAL:
	case IC_RETURN_VAL2:
	  if (tmpc) {
	    if (tmpc->raw_type==RT_F64 && tmpil1->ic_class->raw_type!=RT_F64)
	      tmpil1->ic_flags|=ICF_RES_TO_F64;
	    else if (tmpc->raw_type!=RT_F64 &&
		  tmpil1->ic_class->raw_type==RT_F64)
	      tmpil1->ic_flags|=ICF_RES_TO_INT;
	  }
	  break;
	case IC_SQR:
	case IC_ABS:
	case IC_SQRT:
	case IC_SIN:
	case IC_COS:
	case IC_TAN:
	case IC_ATAN:
	  if (tmpc->raw_type==RT_F64)
	    CmpF1PushPop(tmpi,tmpi1);
	  break;
	case IC_NOBOUND_SWITCH:
	case IC_SWITCH:
	  lb=OptLabelFwd(tmpi->ic_data(CCodeMisc *)->dft);
	  lb->use_cnt++;
	  break;
	case IC_ASM:
	  tmpaot=tmpi->ic_data;
	  tmpie=tmpaot->next_ie;
	  while (tmpie!=&tmpaot->next_ie) {
	    if (IET_REL_I0<=tmpie->type<=IET_IMM_I64 &&
		  tmpie->str && tmpie->flags&IEF_GOTO_LABEL &&
		  (lb=COCGoToLabelFind(cc,tmpie->str)))
	      lb->use_cnt++; //Prevent deadcode elimination.
	    tmpie=tmpie->next;
	  }
	  break;
	case IC_BR_NOT_EQU:
	case IC_BR_EQU_EQU:
	  if ((tmpi1->ic_code==IC_IMM_I64 ||
		tmpi1->ic_code==IC_IMM_F64) && !tmpi1->ic_data) {
	    OptFree(tmpi1);
	    MemCpy(&tmpi->arg1,&tmpi->arg2,sizeof(CICArg));
	    tmpi->arg2.type=MDF_NULL;
	    if (code==IC_BR_EQU_EQU)
	      code=tmpi->ic_code=IC_BR_ZERO;
	    else
	      code=tmpi->ic_code=IC_BR_NOT_ZERO;
	    tmpi1=tmpi2;
	    tmpc1=tmpc2;
	  } else if ((tmpi2->ic_code==IC_IMM_I64 ||
		tmpi2->ic_code==IC_IMM_F64) && !tmpi2->ic_data) {
	    OptFree(tmpi2);
	    tmpi->arg2.type=MDF_NULL;
	    if (code==IC_BR_EQU_EQU)
	      code=tmpi->ic_code=IC_BR_ZERO;
	    else
	      code=tmpi->ic_code=IC_BR_NOT_ZERO;
	  } else
	    goto here1;
	case IC_BR_ZERO:
	case IC_BR_NOT_ZERO:
	  if (tmpi1->ic_code==IC_IMM_I64 || tmpi1->ic_code==IC_IMM_F64) {
	    if (code==IC_BR_ZERO ^^ tmpi1->ic_data) {
	      OptFree(tmpi1);
	      tmpi->arg1.type=MDF_NULL;
	      tmpi->ic_code=IC_JMP;
	    } else {
	      OptFree(tmpi1);
	      tmpi=OptFree(tmpi);
	      break;
	    }
	  }
	  goto here1;
	case IC_BR_AND_ZERO:
	  if (tmpi1->ic_code==IC_IMM_I64) {
	    i=Bsr(tmpi1->ic_data);
	    if (0<=i==Bsf(tmpi1->ic_data)) {
	      tmpi1->ic_data=i;
	      tmpi->ic_flags|=ICF_BY_VAL;
	      tmpi->ic_code=IC_BR_NOT_BT;
	      goto here1;
	    }
	  }
	  if (tmpi2->ic_code==IC_IMM_I64) {
	    i=Bsr(tmpi2->ic_data);
	    if (0<=i==Bsf(tmpi2->ic_data)) {
	      tmpi2->ic_data=i;
	      tmpi->ic_flags|=ICF_SWAP|ICF_BY_VAL;
	      tmpi->ic_code=IC_BR_NOT_BT;
	    }
	  }
	  goto here1;
	case IC_BR_AND_NOT_ZERO:
	  if (tmpi1->ic_code==IC_IMM_I64) {
	    i=Bsr(tmpi1->ic_data);
	    if (0<=i==Bsf(tmpi1->ic_data)) {
	      tmpi1->ic_data=i;
	      tmpi->ic_flags|=ICF_BY_VAL;
	      tmpi->ic_code=IC_BR_BT;
	      goto here1;
	    }
	  }
	  if (tmpi2->ic_code==IC_IMM_I64) {
	    i=Bsr(tmpi2->ic_data);
	    if (0<=i==Bsf(tmpi2->ic_data)) {
	      tmpi2->ic_data=i;
	      tmpi->ic_flags|=ICF_SWAP|ICF_BY_VAL;
	      tmpi->ic_code=IC_BR_BT;
	    }
	  }
	  goto here1;
	case IC_BR_MM_ZERO:
	case IC_BR_MM_NOT_ZERO:
	  if (tmpi1->ic_code==IC_LEA && tmpi1->arg1.type&MDF_DISP &&
		tmpi1->arg1.reg==REG_RBP)
	    for (i=0;i<member_cnt;i++)
	      if (mv[i].offset_start==tmpi1->arg1.disp) {
		mv[i].lea_balance--;
		mv[i].score++;
		break;
	      }
	  tmpi->arg1_type_pointed_to=CmpRawType(tmpc);
	  goto here1;
	case IC_BR_LESS:
	case IC_BR_GREATER_EQU:
	case IC_BR_GREATER:
	case IC_BR_LESS_EQU:
	  if (tmpi->ic_flags&ICF_USE_F64)
	    CmpF2PushPop(tmpi,tmpi1,tmpi2);
	case IC_BR_EQU_EQU2...IC_BR_LESS_EQU2:
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
	  if (tmpi->ic_flags&ICF_PUSH_CMP)
	    lb=tmpi->ic_data;
	  else
	    lb=OptLabelFwd(tmpi->ic_data);
	  lb->use_cnt++;
	  break;
	case IC_NOP1:
	  tmpi=OptFree(tmpi);
	  break;
	case IC_NOP2:
	  ps->ptr+=tmpi->ic_data;
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
	case IC_EQU_EQU:
	case IC_NOT_EQU:
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
	  "Pass:%d Missing IC hndlr\n",cc->pass;
	  ICPut(cc,tmpi);
	  LexExcept(cc,"Compiler Optimization Error at ");
      }
    }
    tmpi=tmpi_next;
  }
/* REGISTER VARIABLE ASSIGNMENT

We just scored num occurrences of each [RBP] offset in the code to help decide
which variables should be assigned to register variables.

We counted the times each offset was added to RBP as a plus LEA and we subtract
the times the offset is dereferenced.  If the address was calculated more times
than the offset was dereferenced, the variable's address was passed or assigned
and we cannot use a register because you can't take address of a reg var.

RAX,RBX,RCX,RDX, R8 are free to be clobbered by each intermediate code.
RAX and R8 links intermediate codes together.  R9 is used for stack machine
temporaries.  RBP is used as stack frame.

RSI,RDI,R10,R11,R12,R13,R14,R15 are used for reg vars.	R12 and R13, however,
have a unusual ModR addressing mode in the x86_64 architecture, so I only use
R12 and R13 as non-pointer register variables, such as index variables i,j,k.

*/
  if (cc->htc.fun) {
    cc->htc.fun->used_reg_mask=cc->htc.fun->used_reg_mask&
	  ~(REGG_LOCAL_VARS|REGG_LOCAL_NON_PTR_VARS)|used_reg_mask;
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
	if (Bt(&cc->flags,CCf_PASS_TRACE_PRESENT))
	  "Fun:%s\n",cc->htc.fun->str;
	if (j>cmp.num_reg_vars) {
	  l=0;
	  for (i=0;i<j && l<cmp.num_non_ptr_vars;i++) {
	    tmpm=mv[i].m;
	    tmpc=OptClassFwd(tmpm->member_class);
	    if (!tmpc->ptr_stars_cnt && !tmpm->dim.next) {
	      while (l<cmp.num_non_ptr_vars &&
		    Bts(&cc->htc.fun->used_reg_mask,cmp.non_ptr_vars_map[l]))
		l++;
	      if (l<cmp.num_non_ptr_vars) {
		tmpm->reg=cmp.non_ptr_vars_map[l++];
		reg_offsets[tmpm->reg].offset=mv[i].offset_start;
		reg_offsets[tmpm->reg].m=tmpm;
		if (Bt(&cc->flags,CCf_PASS_TRACE_PRESENT))
		  "Reg %Z Var \"%-15ts\" %016X[RBP]\n",tmpm->reg,"ST_U64_REGS",
			tmpm->str,reg_offsets[tmpm->reg].offset;
		mv[i].offset_start=0; //flag as reg var
		if (tmpm->size<8 &&
		      !StrIMatch("Bool",tmpm->member_class->str) &&
		      tmpm->member_class->type&HTT_INTERNAL_TYPE)
		  PrintWarn("Using 64-bit reg var.\n "
			"$$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
			cc->lex_include_stk->full_name,
			cc->lex_include_stk->line_num,
			tmpm->str,cc->htc.fun->str);
	      }
	    }
	  }
	}
	l=0;
	for (i=0;i<j && l<cmp.num_reg_vars;i++) {
	  tmpm=mv[i].m;
//if not just flagged as reg var
	  if (mv[i].offset_start && (!mv[i].m->dim.next||
		tmpm->offset>0 && StrCmp(tmpm->str,"argv"))) {
	    while (l<cmp.num_reg_vars &&
		  Bts(&cc->htc.fun->used_reg_mask,cmp.to_reg_vars_map[l]))
	      l++;
	    if (l<cmp.num_reg_vars) {
	      tmpm->reg=cmp.to_reg_vars_map[l++];
	      reg_offsets[tmpm->reg].offset=mv[i].offset_start;
	      reg_offsets[tmpm->reg].m=tmpm;
	      if (Bt(&cc->flags,CCf_PASS_TRACE_PRESENT))
		"Reg %Z Var \"%-15ts\" %016X[RBP]\n",tmpm->reg,"ST_U64_REGS",
		      tmpm->str,reg_offsets[tmpm->reg].offset;
	      if (tmpm->size<8 &&
		    !StrIMatch("Bool",tmpm->member_class->str) &&
		    tmpm->member_class->type&HTT_INTERNAL_TYPE)
		PrintWarn("Using 64-bit reg var.\n "
		      "$$LK,\"FL:%s,%d\"$$ '%s' in '%s'\n",
		      cc->lex_include_stk->full_name,
		      cc->lex_include_stk->line_num,
		      tmpm->str,cc->htc.fun->str);
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

