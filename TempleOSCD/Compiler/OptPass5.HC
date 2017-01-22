U0 OptPass5(CCmpCtrl *cc)
{
  CIntermediateCode *tmpi,*tmpi1;
  I64 code,i;
  CPrsStk *ps=cc->ps;
  ps->ptr=0;
  ps->ptr2=0;

  tmpi=cc->coc.coc_head.next;
  while (code=tmpi->ic_code) {
    if (code>IC_NOP2) {
      if (tmpi->ic_flags&ICF_PASS_TRACE) {
	if (Bt(&cc->saved_pass_trace,5)) {
	  "%2d:",ps->ptr;
	  ICPut(cc,tmpi);
	}
      }
      if (intermediate_code_table[code].arg_cnt==IS_V_ARG)
	ps->ptr-=tmpi->ic_data>>3;
      if (code==IC_PUSH_REGS) {
	for (i=0;i<REG_REGS_NUM;i++)
	  if (Bt(&tmpi->ic_data,i))
	    ps->ptr++;
      } else if (code==IC_POP_REGS) {
	for (i=0;i<REG_REGS_NUM;i++)
	  if (Bt(&tmpi->ic_data,i))
	    ps->ptr--;
      }
      if (tmpi->arg2.type&MDF_STK) {
	tmpi1=PrsPop(ps);
	if (tmpi1->ic_code==IC_MOV || tmpi1->ic_code==IC_REG) {
	  if (tmpi1->ic_flags & ICF_RES_TO_INT) {
	    if (tmpi1->arg1.type&MDF_IMM)
	      tmpi1->arg1.disp=tmpi1->arg1.disp(F64);
	    else
	      tmpi->ic_flags|=ICF_ARG2_TO_INT;
	  } else if (tmpi1->ic_flags&ICF_RES_TO_F64) {
	    if (tmpi1->arg1.type&MDF_IMM)
	      tmpi1->arg1.disp(F64)=tmpi1->arg1.disp;
	    else
	      tmpi->ic_flags|=ICF_ARG2_TO_F64;
	  }
	  tmpi->arg2.type=tmpi1->arg1.type&MDG_MASK+
		MinI64(tmpi->arg2.type.raw_type,
		MinI64(tmpi1->res.type.raw_type,tmpi1->arg1.type.raw_type));
	  tmpi->arg2.reg=tmpi1->arg1.reg;
	  tmpi->arg2.disp=tmpi1->arg1.disp;
	  tmpi->ic_flags|=tmpi1->ic_flags&ICG_NO_CVT_MASK;
	  OptSetNOP2(tmpi1);
	}
      }

      if (tmpi->arg1.type&MDF_STK) {
	tmpi1=PrsPop(ps);
	if (tmpi1->ic_code==IC_MOV || tmpi1->ic_code==IC_REG) {
	  if (tmpi1->ic_flags & ICF_RES_TO_INT) {
	    if (tmpi1->arg1.type&MDF_IMM)
	      tmpi1->arg1.disp=tmpi1->arg1.disp(F64);
	    else
	      tmpi->ic_flags|=ICF_ARG1_TO_INT;
	  } else if (tmpi1->ic_flags&ICF_RES_TO_F64) {
	    if (tmpi1->arg1.type&MDF_IMM) {
	      if (tmpi1->arg1.type&RTF_UNSIGNED)
		tmpi1->arg1.disp(F64)=tmpi1->arg1.disp(U64);
	      else
		tmpi1->arg1.disp(F64)=tmpi1->arg1.disp(I64);
	    } else
	      tmpi->ic_flags|=ICF_ARG1_TO_F64;
	  }
	  tmpi->arg1.type=tmpi1->arg1.type&MDG_MASK+
		MinI64(tmpi->arg1.type.raw_type,
		MinI64(tmpi1->res.type.raw_type,tmpi1->arg1.type.raw_type));
	  CmpMinTypePointed(tmpi,tmpi1->arg1_type_pointed_to);
	  tmpi->arg1.reg=tmpi1->arg1.reg;
	  tmpi->arg1.disp=tmpi1->arg1.disp;
	  tmpi->ic_flags|=tmpi1->ic_flags&ICG_NO_CVT_MASK;
	  OptSetNOP2(tmpi1);
	}
      }
      if (tmpi->res.type&MDF_STK &&
	    !(tmpi->ic_flags&ICF_RES_NOT_USED))
	PrsPush(ps,tmpi);
    }
    tmpi=tmpi->next;
  }
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",cc->pass,ps->ptr;
    LexExcept(cc,"Compiler Optimization Error at ");
  }
}
