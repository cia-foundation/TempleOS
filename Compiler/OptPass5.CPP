U0 OptPass5(CCmpCtrl *cc)
{
  CIntermediateCode *tempi,*tempi1;
  I64 code,i;
  CPrsStk *ps=cc->ps;
  ps->ptr=0;
  ps->ptr2=0;

  tempi=cc->coc.coc_root.next;
  while (code=tempi->ic_code) {
    if (code>IC_NOP2) {
      if (tempi->ic_flags&ICF_PASS_TRACE) {
	if (Bt(&cc->saved_pass_trace,5)) {
	  "%2d:",ps->ptr;
	  ICPut(cc,tempi);
	}
      }
      if (intermediate_code_table[code].arg_cnt==IS_V_ARG)
	ps->ptr-=tempi->ic_data>>3;
      if (code==IC_PUSH_REGS) {
	for (i=0;i<NUM_REGS;i++)
	  if (Bt(&tempi->ic_data,i))
	    ps->ptr++;
      } else if (code==IC_POP_REGS) {
	for (i=0;i<NUM_REGS;i++)
	  if (Bt(&tempi->ic_data,i))
	    ps->ptr--;
      }
      if (tempi->a2.type&MDF_STK) {
	tempi1=PrsPop(ps);
	if (tempi1->ic_code==IC_MOV || tempi1->ic_code==IC_REG) {
	  if (tempi1->ic_flags & ICF_R_TO_INT) {
	    if (tempi1->a1.type&MDF_IMM)
	      tempi1->a1.disp=tempi1->a1.disp(F64);
	    else
	      tempi->ic_flags|=ICF_A2_TO_INT;
	  } else if (tempi1->ic_flags&ICF_R_TO_F64) {
	    if (tempi1->a1.type&MDF_IMM)
	      tempi1->a1.disp(F64)=tempi1->a1.disp;
	    else
	      tempi->ic_flags|=ICF_A2_TO_F64;
	  }
	  tempi->a2.type=tempi1->a1.type&MDG_MASK+
		MinI64(tempi->a2.type.raw_type,
		MinI64(tempi1->r.type.raw_type,tempi1->a1.type.raw_type));
	  tempi->a2.reg=tempi1->a1.reg;
	  tempi->a2.disp=tempi1->a1.disp;
	  tempi->ic_flags|=tempi1->ic_flags&ICG_NO_CVT_MASK;
	  OptSetNOP2(tempi1);
	}
      }

      if (tempi->a1.type&MDF_STK) {
	tempi1=PrsPop(ps);
	if (tempi1->ic_code==IC_MOV || tempi1->ic_code==IC_REG) {
	  if (tempi1->ic_flags & ICF_R_TO_INT) {
	    if (tempi1->a1.type&MDF_IMM)
	      tempi1->a1.disp=tempi1->a1.disp(F64);
	    else
	      tempi->ic_flags|=ICF_A1_TO_INT;
	  } else if (tempi1->ic_flags&ICF_R_TO_F64) {
	    if (tempi1->a1.type&MDF_IMM) {
	      if (tempi1->a1.type&RTF_UNSIGNED)
		tempi1->a1.disp(F64)=tempi1->a1.disp(U64);
	      else
		tempi1->a1.disp(F64)=tempi1->a1.disp(I64);
	    } else
	      tempi->ic_flags|=ICF_A1_TO_F64;
	  }
	  tempi->a1.type=tempi1->a1.type&MDG_MASK+
		MinI64(tempi->a1.type.raw_type,
		MinI64(tempi1->r.type.raw_type,tempi1->a1.type.raw_type));
	  CmpMinTypePointed(tempi,tempi1->a1_type_pointed_to);
	  tempi->a1.reg=tempi1->a1.reg;
	  tempi->a1.disp=tempi1->a1.disp;
	  tempi->ic_flags|=tempi1->ic_flags&ICG_NO_CVT_MASK;
	  OptSetNOP2(tempi1);
	}
      }
      if (tempi->r.type&MDF_STK &&
	    !(tempi->ic_flags&ICF_RES_NOT_USED))
	PrsPush(ps,tempi);
    }
    tempi=tempi->next;
  }
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",cc->pass,ps->ptr;
    LexExcept(cc,"Compiler Optimization Error at ");
  }
}
