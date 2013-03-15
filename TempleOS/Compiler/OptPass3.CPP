I64 CmpType(CHashClass *tempc)
{
  if (tempc) {
    while (tempc->fwd_class)
      tempc=tempc->fwd_class;
    return tempc->sub_type;
  }
  return 0;
}

I64 CmpPType(CHashClass *tempc)
{
  if (tempc) {
    if (tempc->ptr_cnt)
      tempc--;
    while (tempc->fwd_class)
      tempc=tempc->fwd_class;
    return tempc->sub_type;
  }
  return 0;
}

U0 CmpF1PushPop(CIntermediateCode *tempi,CIntermediateCode *tempi2)
{
  if (Bt(fpop_bitmap,tempi2->ic_opcode)|| tempi2->ic_flags&ICF_R_TO_F64)
    Bts(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0);
}

U0 CmpF2PushPop(CIntermediateCode *tempi,CIntermediateCode *tempi1,CIntermediateCode *tempi2)
{
  if ((tempi2->ic_opcode==IC_MOV || tempi2->ic_opcode==IC_F64) &&
       !(tempi2->ic_flags&ICF_R_TO_F64) &&
      (Bt(fpop_bitmap,tempi1->ic_opcode) || tempi1->ic_flags&ICF_R_TO_F64))
    Bts(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0);
  else if ((Bt(fpop_bitmap,tempi2->ic_opcode) || tempi2->ic_flags&ICF_R_TO_F64)&&
       !(tempi1->ic_flags&ICF_R_TO_F64))
    Bts(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0);
}

class OptMemberVar
{
  I64 score,offset,lea_balance;
  CMemberLst *m;
};

I64 OptMVCompare(OptMemberVar *mv1,OptMemberVar *mv2)
{
  return mv2->score-mv1->score;
}

U0 OptPass3(CLex *lx,CHashFun *fun_class,COptReg *register_offsets)
{
  CHashClass *tempc,*tempc1,*tempc2,*tempc3;
  CCodeBlkMisc *lb;
  CIntermediateCode *tempi,*tempi1,*tempi2,*tempi3,
      *tempil1,*tempil2,
  *temp_lag[4];
  I64 code,i,j,l,member_cnt,used_reg_mask=0;
  CMemberLst *tempm;
  CPrsStk *ps=OptStart(lx,temp_lag);
  OptMemberVar *mv=NULL;
  for (i=0;i<NUM_REGS;i++) {
    register_offsets[i].offset=MAX_I64;
    register_offsets[i].m=NULL;
  }
  if (fun_class) {
    member_cnt=fun_class->member_cnt;
    if (Bt(&fun_class->flags,Ff_DOT_DOT_DOT))
      member_cnt+=2;
    mv=CAlloc(member_cnt*sizeof(OptMemberVar));
    member_cnt=0;
    tempm=fun_class->member_lst;
    while (tempm) {
      tempc=tempm->member_class;
      while (tempc->fwd_class)
	tempc=tempc->fwd_class;
      if (0<=tempm->register<NUM_REGS) {
	if (Bts(&used_reg_mask,tempm->register))
	  PrintFWarn("%S\n $$LK-A,\"FL:%s,%d\"$$ '%s' in '%s'\n","ST_REG_IN_USE",
	    lx->cur_lfn->name,lx->cur_lfn->line_num,tempm->str,fun_class->str);
	register_offsets[tempm->register].offset=tempm->offset;
	register_offsets[tempm->register].m=tempm;
      } else if (tempc->sub_type!=IT_F64 && tempm->register!=CREG_NONE ||
	       tempm->register==CREG_ALLOC) {
	if (tempm->register==CREG_ALLOC)
	  mv[member_cnt].score=MAX_I64/2; //big but not too big
	mv[member_cnt].offset=tempm->offset;
	mv[member_cnt++].m=tempm;
      } else {
	if (tempm->register==CREG_ALLOC)
	  PrintFWarn("%S\n $$LK-A,\"FL:%s,%d\"$$ '%s' in '%s'\n","ST_CANT_REG_VAR",
	    lx->cur_lfn->name,lx->cur_lfn->line_num,tempm->str,fun_class->str);
      }
      tempm=tempm->next;
    }
  } else
    member_cnt=0;

  tempi=lx->cbh.cb_in_ptr;
  if (tempi->ic_opcode==IC_START)
    tempi++;
  while (code=tempi->ic_opcode) {
    if (code!=IC_NOP) {
      OptNext(tempi,temp_lag);
      tempil1=temp_lag[1];
      tempil2=temp_lag[2];
      tempc=tempi->ic_class;
      nobound_switch (cmp_arg_cnt[code]) {
	case 9:
	  ps->ptr-=tempi->ic_data>>3;
	  break;
	case 3:
	  tempi3=PrsPop(ps);
	  if (tempi3->ic_flags&ICF_R_TO_F64)
	    tempc3=sys_internal_types[IT_F64];
	  else if (tempi3->ic_flags & ICF_R_TO_INT)
	    tempc3=sys_internal_types[IT_I64];
	  else
	    tempc3=tempi->t.p3c;
	  tempi->p3.type=TY_STK+CmpType(tempc3);
	  tempi->p3.ptype=CmpPType(tempc3);
	  tempi->ic_flags|=ICF_P3_WAS_STK;
	case 2:
	  tempi2=PrsPop(ps);
	  if (tempi2->ic_flags&ICF_R_TO_F64)
	    tempc2=sys_internal_types[IT_F64];
	  else if (tempi2->ic_flags & ICF_R_TO_INT)
	    tempc2=sys_internal_types[IT_I64];
	  else
	    tempc2=tempi->t.p2c;
	  tempi->p2.type=TY_STK+CmpType(tempc2);
	  tempi->p2.ptype=CmpPType(tempc2);
	  tempi->ic_flags|=ICF_P2_WAS_STK;
	case 1:
	  tempi1=PrsPop(ps);
	  if (tempi1->ic_flags&ICF_R_TO_F64)
	    tempc1=sys_internal_types[IT_F64];
	  else if (tempi1->ic_flags & ICF_R_TO_INT)
	    tempc1=sys_internal_types[IT_I64];
	  else
	    tempc1=tempi->t.p1c;
	  tempi->p1.type=TY_STK+CmpType(tempc1);
	  tempi->p1.ptype=CmpPType(tempc1);
	  tempi->ic_flags|=ICF_P1_WAS_STK;
	  break;
	case 0: //nobound_switch
	  break;
      }
      if (cmp_result_cnt[code]) {
	tempi->r.type=TY_STK+CmpType(tempc);
	tempi->r.ptype=CmpPType(tempc);
	tempi->ic_flags|=ICF_R_WAS_STK;
	PrsPush(ps,tempi);
      }
      nobound_switch (code) {
	case IC_F64:
	  tempi->ic_flags&=~ICF_R_TO_F64;
	  if (tempi->ic_flags&ICF_R_TO_INT) {
	    tempi->ic_data=ToI64(tempi->ic_data(F64));
	    tempi->ic_flags&=~ICF_R_TO_INT;
	    tempi->ic_opcode=IC_IMM;
	  }
	  break;
	case IC_IMM:
	  tempi->ic_flags&=~ICF_R_TO_INT;
	  if (tempi->ic_flags&ICF_R_TO_F64) {
	    tempi->ic_data(F64)=ToF64(tempi->ic_data);
	    tempi->ic_flags&=~ICF_R_TO_F64;
	    tempi->ic_opcode=IC_F64;
	  }
	  break;
	case IC_STR_CONST:
	case IC_FS:
	case IC_GS:
	case IC_MOV_FS:
	case IC_MOV_GS:
	case IC_RIP:
	case IC_PUSH_CMP:
	  break;
	case IC_RBP:
	  tempi->ic_opcode=IC_REG;
	  tempi->p1.reg=CREG_RBP;
	  break;
	case IC_REG:
	case IC_COM:
	case IC_NATIVE_TYPECAST:
	case IC_NOT:
	case IC_UNARY_MINUS:
	  break;
	case IC_DEREF:
	  tempi->p1.ptype=tempi->r.type&IT_MASK;
	  if (tempi1->ic_opcode==IC_LEA) {
	    tempi->ic_flags|=tempi1->ic_flags;
	    tempi1->ic_opcode=IC_NOP;
	    tempi->p1.reg=tempi1->p1.reg;
	    tempi->p1.disp=tempi1->p1.disp;
	    tempi->p1.type=TY_DISP+tempi->r.type&IT_MASK;
	    tempi->p1.ptype=tempi->r.ptype;
	    tempi->ic_opcode=IC_MOV;
	    if (tempi->p1.reg==CREG_RBP)
	      for (i=0;i<member_cnt;i++)
		if (mv[i].offset==tempi->p1.disp) {
		  mv[i].lea_balance--;
		  mv[i].score+=1+tempi->ic_indent_level;
		  break;
		}
	      } else if (tempil1->ic_opcode==IC_ADD_CONST) {
	    if (tempil2->ic_opcode==IC_REG) {
	      tempi->ic_flags|=tempil2->ic_flags|tempil1->ic_flags;
	      tempil2->ic_opcode=IC_NOP;
	      tempil1->ic_opcode=IC_NOP;
	      tempi->p1.reg=tempil2->p1.reg;
	      tempi->p1.disp=tempi->ic_data;
	      tempi->p1.type=TY_DISP+tempi->r.type&IT_MASK;
	      tempi->p1.ptype=tempi->r.ptype;
	      tempi->ic_opcode=IC_MOV;
	    }
	  }
	  break;
	case IC__PP:
	case IC__MM:
	case IC_PP_:
	case IC_MM_:
	  tempi->r.ptype=CmpPType(tempi->t.p1c);
	case IC_DEREF_PP:
	case IC_DEREF_MM:
	  if (tempi1->ic_opcode==IC_LEA &&
	      tempi1->p1.type&TY_MASK==TY_DISP &&
	      tempi1->p1.reg==CREG_RBP)
	    for (i=0;i<member_cnt;i++)
	      if (mv[i].offset==tempi1->p1.disp) {
		mv[i].lea_balance--;
		mv[i].score+=1+tempi->ic_indent_level;
		break;
	      }
	  tempi->p1.ptype=tempi->r.type&IT_MASK;
	  break;
	case IC_POWER:
	case IC_SHL:
	case IC_SHR:
	case IC_MOD:
	case IC_AND:
	case IC_OR:
	case IC_XOR:
	  break;

	case IC_MUL:
	case IC_DIV:
	  if (tempc->sub_type==IT_F64) {
	    CmpF2PushPop(tempi,tempi1,tempi2);
	    break;
	  }
	  break;
	case IC_ADD:
	  if (tempc->sub_type==IT_F64) {
	    CmpF2PushPop(tempi,tempi1,tempi2);
	    break;
	  }
	  if (OptFixupBinaryOp2(&tempi1,&tempi2)) {
	    tempi->ic_flags|=tempi1->ic_flags;
	    tempi1->ic_opcode=IC_NOP;
	    if (tempi->t.p1t!=tempi2)
	      tempi->t.p1c=tempi->t.p2c;
	    tempi->ic_data=tempi1->ic_data;
	    tempi->ic_opcode=IC_ADD_CONST;
	    tempi->p1.ptype=tempi->r.type&IT_MASK;
	    tempi->p2.type=TY_NULL;
	    if (tempil2->ic_opcode==IC_REG &&
		tempil2->p1.reg==CREG_RBP) {
	      tempi->ic_flags|=tempil2->ic_flags;
	      tempil2->ic_opcode=IC_NOP;
	      tempi->ic_opcode=IC_LEA;
	      tempi->p1.reg=CREG_RBP;
	      tempi->p1.type=TY_DISP+tempi->p1.type&IT_MASK;
	      tempi->p1.disp=tempi->ic_data;
	      for (i=0;i<member_cnt;i++)
		if (mv[i].offset==tempi->ic_data) {
		  mv[i].lea_balance++;
		  mv[i].score+=1+tempi->ic_indent_level;
		  break;
		}
	      }
	  }
	  break;
	case IC_SUB:
	  if (tempc->sub_type==IT_F64) {
	    CmpF2PushPop(tempi,tempi1,tempi2);
	    break;
	  }
	  if (tempi2->ic_opcode==IC_IMM) {
	    tempi->ic_flags|=tempi2->ic_flags;
	    tempi2->ic_opcode=IC_NOP;
	    tempi->ic_data=tempi2->ic_data;
	    tempi->ic_opcode=IC_SUB_CONST;
	    tempi->p2.type=TY_NULL;
	  }
	  break;
	case IC_LESS:
	case IC_GREATER:
	case IC_LESS_EQUAL:
	case IC_GREATER_EQUAL:
	  if (tempi->ic_flags&ICF_USE_F64)
	    CmpF2PushPop(tempi,tempi1,tempi2);
	  break;
	case IC_EQUAL_EQUAL:
	case IC_NOT_EQUAL:
	case IC_AND_AND:
	case IC_OR_OR:
	case IC_XOR_XOR:
	case IC_COLON:
	  break;

	case IC_MUL_EQUAL:
	case IC_DIV_EQUAL:
	case IC_ADD_EQUAL:
	case IC_SUB_EQUAL:
	  if (tempc->sub_type==IT_F64)
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
	  if (tempi1->ic_opcode==IC_LEA &&
	      tempi1->p1.type&TY_MASK==TY_DISP &&
	      tempi1->p1.reg==CREG_RBP)
	    for (i=0;i<member_cnt;i++)
	      if (mv[i].offset==tempi1->p1.disp) {
		mv[i].lea_balance--;
		mv[i].score+=1+tempi->ic_indent_level;
		break;
	      }
	    tempi->p1.ptype=tempi->r.type&IT_MASK;
	  tempi->r.ptype=CmpPType(tempi->t.p1c);
	  break;
	case IC_RETURN_VAL:
	case IC_RETURN_VAL2:
	  if (tempc) {
	    if (tempc->sub_type==IT_F64 && tempil1->ic_class->sub_type!=IT_F64)
	      tempil1->ic_flags|=ICF_R_TO_F64;
	    else if (tempc->sub_type!=IT_F64 && tempil1->ic_class->sub_type==IT_F64)
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
	  if (tempc->sub_type==IT_F64)
	    CmpF1PushPop(tempi,tempi1);
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
	case IC_CLI:
	case IC_STI:
	case IC_HLT:
	case IC_PUSHFD:
	case IC_POPFD:
	case IC_PUSH:
	case IC_POP:
	case IC_CLD:
	case IC_STD:
	case IC_INVLPG:
	case IC_CLFLUSH:
	case IC_PAUSE:
	case IC_GET_FLAGS:
	case IC_CARRY:
	case IC_RDTSC:
	case IC_HPET:
	case IC_SET_FLAGS:
	case IC_GET_BASE_PTR:
	case IC_SET_BASE_PTR:
	case IC_GET_STK_PTR:
	case IC_GET_RAX:
	case IC_SET_STK_PTR:
	case IC_SET_RAX:
	case IC_ABS_ADDR:
	case IC_HEAP_GLBL:
	case IC_ADDR_IMPORT:
	case IC_TYPE:
	case IC_BT:
	case IC_BTS:
	case IC_BTR:
	case IC_BTC:
	case IC_BEQU:
	case IC_LBTS:
	case IC_LBTR:
	case IC_LBTC:
	case IC_LBEQU:
	case IC_BSF:
	case IC_BSR:
	case IC_PREEMPT:
	case IC_SIGN_I64:
	case IC_TOUPPER:
	case IC_TO_I64:
	case IC_TO_F64:
	case IC_TO_BOOL:
	case IC_ABS_I64:
	case IC_LIMIT_I64:
	case IC_LIMIT_U64:
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
	case IC_SWAP_U64:
	case IC_QUE_INIT:
	case IC_QUE_INS:
	case IC_QUE_INS_REV:
	case IC_QUE_REM:
	case IC_MEMCPY:
	case IC_MEMSET:
	case IC_MEMSET_U16:
	case IC_MEMSET_U32:
	case IC_MEMSET_U64:
	case IC_INU32:
	case IC_INU16:
	case IC_INU8:
	case IC_STRLEN:
	case IC_OUTU32:
	case IC_OUTU16:
	case IC_OUTU8:
	case IC_ASM:
	  break;
	case IC_NOBOUND_SWITCH:
	case IC_SWITCH:
	  lb=tempi->ic_data;
	  lb=lb->dft;
	  while (lb->fwd)
	    lb=lb->fwd;
	  lb->flags|=CBMF_USED;
	  break;
	case IC_BR_ZERO:
	case IC_BR_NOT_ZERO:
	  if (tempi1->ic_opcode==IC_IMM ||
	      tempi1->ic_opcode==IC_F64) {
	    tempi1->ic_opcode=IC_NOP;
	    tempi->p1.type=TY_NULL;
	    tempi1->r.type=TY_NULL;
	    if (code==IC_BR_ZERO ^^ tempi1->ic_data)
	      tempi->ic_opcode=IC_JMP;
	    else {
	      tempi->ic_opcode=IC_NOP;
	      tempi->ic_class=NULL;
	      break;
	    }
	  }
	  goto here1;
	case IC_BR_AND_ZERO:
	  if (tempi1->ic_opcode==IC_IMM) {
	    i=Bsr(tempi1->ic_data);
	    if (0<=i==Bsf(tempi1->ic_data)) {
	      tempi1->ic_data=i;
	      tempi->ic_flags|=ICF_NOT_ADDR;
	      tempi->ic_opcode=IC_BR_NOT_BT;
	      goto here1;
	    }
	  }
	  if (tempi2->ic_opcode==IC_IMM) {
	    i=Bsr(tempi2->ic_data);
	    if (0<=i==Bsf(tempi2->ic_data)) {
	      tempi2->ic_data=i;
	      tempi->ic_flags|=ICF_SWAP|ICF_NOT_ADDR;
	      tempi->ic_opcode=IC_BR_NOT_BT;
	    }
	  }
	  goto here1;
	case IC_BR_AND_NOT_ZERO:
	  if (tempi1->ic_opcode==IC_IMM) {
	    i=Bsr(tempi1->ic_data);
	    if (0<=i==Bsf(tempi1->ic_data)) {
	      tempi1->ic_data=i;
	      tempi->ic_flags|=ICF_NOT_ADDR;
	      tempi->ic_opcode=IC_BR_BT;
	      goto here1;
	    }
	  }
	  if (tempi2->ic_opcode==IC_IMM) {
	    i=Bsr(tempi2->ic_data);
	    if (0<=i==Bsf(tempi2->ic_data)) {
	      tempi2->ic_data=i;
	      tempi->ic_flags|=ICF_SWAP|ICF_NOT_ADDR;
	      tempi->ic_opcode=IC_BR_BT;
	    }
	  }
	  goto here1;
	case IC_BR_MM_ZERO:
	case IC_BR_MM_NOT_ZERO:
	  if (tempi1->ic_opcode==IC_LEA &&
	      tempi1->p1.type&TY_MASK==TY_DISP &&
	      tempi1->p1.reg==CREG_RBP)
	    for (i=0;i<member_cnt;i++)
	      if (mv[i].offset==tempi1->p1.disp) {
		mv[i].lea_balance--;
		mv[i].score+=1+tempi->ic_indent_level;
		break;
	      }
	  tempi->p1.ptype=CmpType(tempc);
	  goto here1;
	case IC_BR_GREATER_EQUAL:
	case IC_BR_LESS_EQUAL:
	case IC_BR_GREATER:
	case IC_BR_LESS:
	  if (tempi->ic_flags&ICF_USE_F64)
	    CmpF2PushPop(tempi,tempi1,tempi2);
	case IC_BR_GREATER_EQUAL2:
	case IC_BR_LESS_EQUAL2:
	case IC_BR_GREATER2:
	case IC_BR_LESS2:
	case IC_BR_NOT_EQUAL2:
	case IC_BR_EQUAL_EQUAL2:
	case IC_BR_NOT_EQUAL:
	case IC_BR_EQUAL_EQUAL:
	case IC_GET_LABEL:
	case IC_BR_BT:
	case IC_BR_BTS:
	case IC_BR_BTR:
	case IC_BR_BTC:
	case IC_BR_NOT_BT:
	case IC_BR_NOT_BTS:
	case IC_BR_NOT_BTR:
	case IC_BR_NOT_BTC:
	case IC_BR_LBTS:
	case IC_BR_LBTR:
	case IC_BR_LBTC:
	case IC_BR_NOT_LBTS:
	case IC_BR_NOT_LBTR:
	case IC_BR_NOT_LBTC:
	case IC_JMP:
here1:
	  lb=tempi->ic_data;
	  if (!(tempi->ic_flags&ICF_PUSH_CMP))
	    while (lb->fwd)
	      lb=lb->fwd;
	  lb->flags|=CBMF_USED;
	  break;
	case IC_LEAVE:
	case IC_RET:
	  break;
	case IC_ADDR:
	case IC_NOP:
	case IC_END:
	case IC_QUESTION:
	case IC_IRQ:
	case IC_END_EXP:
	case IC_CALL_END:
	case IC_CALL_END2:
	case IC_CALL_START:
	case IC_PUSH_REGS:
	case IC_POP_REGS:
	case IC_LABEL:
	  break;
	case IC_NOP1:
	  ps->ptr+=tempi->ic_data;
	  break;

	default:
	  OptMissing(lx,tempi);
      }
    }
    tempi++;
  }
  if (fun_class) {
    fun_class->used_reg_mask=fun_class->used_reg_mask&~(CREG_VARS_MASK|CREG_NON_PTR_MASK)|used_reg_mask;
    if (!Bt(&lx->opts,OPTf_NO_REG_VAR) &&
      !(lx->flags & LF_NO_REG_OPT)) {
      QSort(mv,member_cnt,sizeof(OptMemberVar),&OptMVCompare);
      while (member_cnt && !mv[member_cnt-1].score)
	member_cnt--;
      j=0;
      for (i=0;i<member_cnt;i++) {
	if (!mv[i].lea_balance && mv[i].offset) {//addr operator cancels
	  mv[j].m=mv[i].m;
	  mv[j++].offset=mv[i].offset;
	} else {
	  if (mv[i].m->register==CREG_ALLOC)
	    PrintFWarn("%S\n $$LK-A,\"FL:%s,%d\"$$ '%s' in '%s'\n","ST_CANT_REG_VAR",
	      lx->cur_lfn->name,lx->cur_lfn->line_num,mv[i].m->str,fun_class->str);
	}
      }
      if (j>0) {
	if (j>num_reg_vars) {
	  l=0;
	  for (i=0;i<j && l<num_non_ptr_vars;i++) {
	    tempm=mv[i].m;
	    tempc=tempm->member_class;
	    while (tempc->fwd_class)
	      tempc=tempc->fwd_class;
	    if (!tempc->ptr_cnt && !tempm->dim.next) {
	      while (l<num_non_ptr_vars &&
		Bts(&fun_class->used_reg_mask,to_non_ptr_vars_map[l]))
		l++;
	      if (l<num_non_ptr_vars) {
		tempm->register=to_non_ptr_vars_map[l++];
		register_offsets[tempm->register].offset=mv[i].offset;
		register_offsets[tempm->register].m=tempm;
		if (Bt(&lx->flags,Lf_OPT_TRACE_PRESENT))
		  "Fun:%s\nReg %Z Var \"%s\" %08X[RBP]\n",
		    fun_class->str,tempm->register,"ST_U64_REGS",
		      tempm->str,register_offsets[tempm->register].offset;
		mv[i].offset=0; //flag as reg var
	      }
	    }
	  }
	}
	l=0;
	for (i=0;i<j && l<num_reg_vars;i++) {
	  tempm=mv[i].m;
	  //if not just flagged as reg var
	  if (mv[i].offset && (!mv[i].m->dim.next||
	      tempm->offset>0 && StrCmp(tempm->str,"argv"))) {
	    while (l<num_reg_vars &&
	      Bts(&fun_class->used_reg_mask,to_reg_vars_map[l]))
	      l++;
	    if (l<num_reg_vars) {
	      tempm->register=to_reg_vars_map[l++];
	      register_offsets[tempm->register].offset=mv[i].offset;
	      register_offsets[tempm->register].m=tempm;
	      if (Bt(&lx->flags,Lf_OPT_TRACE_PRESENT))
		"Reg %Z Var \"%s\" %08X[RBP]\n",tempm->register,"ST_U64_REGS",
		  tempm->str,register_offsets[tempm->register].offset;
	    }
	  }
	}
      }
    }
    Free(mv);
  }
  OptEnd(lx,ps);
}

