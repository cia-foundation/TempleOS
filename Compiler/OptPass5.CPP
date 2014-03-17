U0 OptPass5(CLex *lx)
{
  CIntermediateCode *tempi,*tempi1;
  I64 code,i,ty,pt,ty1,pt1;
  CPrsStk *ps=lx->ps;
  ps->ptr=0;
  ps->ptr2=0;

  tempi=lx->lcc.lcc_root.next;
  while (code=tempi->ic_code) {
    if (code>IC_NOP2) {
      if (tempi->ic_flags&ICF_TRACE) {
	if (Bt(&lx->pass_trace_mask,5)) {
	  "%2d:",ps->ptr;
	  ICPut(tempi);
	}
      }
      if (cmp_arg_cnt[code]==9)
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
      if (tempi->a2.type&TY_MASK==TY_STK) {
	tempi1=PrsPop(ps);
	if (tempi1->ic_code==IC_MOV ||
	    tempi1->ic_code==IC_REG) {
	  if (tempi1->ic_flags & ICF_R_TO_INT) {
	    if (tempi1->a1.type&TY_MASK==TY_IMM)
	      tempi1->a1.disp=tempi1->a1.disp(F64);
	    else
	      tempi->ic_flags|=ICF_A2_TO_INT;
	  } else if (tempi1->ic_flags&ICF_R_TO_F64) {
	    if (tempi1->a1.type&TY_MASK==TY_IMM)
	      tempi1->a1.disp(F64)=tempi1->a1.disp;
	    else
	      tempi->ic_flags|=ICF_A2_TO_F64;
	  }

	  ty=tempi->a2.type&RT_MASK;
	  ty1=tempi1->a1.type&RT_MASK;
	  if (ty1 && ty1<ty)
	    ty=ty1;
	  ty1=tempi1->r.type&RT_MASK;
	  if (ty1 && ty1<ty)
	    ty=ty1;
	  tempi->a2.type=tempi1->a1.type&TY_MASK+ty;

	  pt=tempi->a2.type_pointed;
	  pt1=tempi1->a1.type_pointed;
	  if (pt1 && pt1<pt && pt!=RT_F64)
	    pt=pt1;
	  pt1=tempi1->r.type_pointed;
	  if (pt1 && pt1<pt && pt!=RT_F64)
	    pt=pt1;
	  tempi->a2.type_pointed=pt;

	  tempi->a2.reg=tempi1->a1.reg;
	  tempi->a2.disp=tempi1->a1.disp;
	  tempi->ic_flags|=tempi1->ic_flags&ICG_NO_CVT_MASK;
	  OptSetNOP2(tempi1);
	}
      }

      if (tempi->a1.type&TY_MASK==TY_STK) {
	tempi1=PrsPop(ps);
	if (tempi1->ic_code==IC_MOV ||
	    tempi1->ic_code==IC_REG) {
	  if (tempi1->ic_flags & ICF_R_TO_INT) {
	    if (tempi1->a1.type&TY_MASK==TY_IMM)
	      tempi1->a1.disp=tempi1->a1.disp(F64);
	    else
	      tempi->ic_flags|=ICF_A1_TO_INT;
	  } else if (tempi1->ic_flags&ICF_R_TO_F64) {
	    if (tempi1->a1.type&TY_MASK==TY_IMM) {
	      if (tempi1->a1.type & 1) //signed/unsigned
		tempi1->a1.disp(F64)=tempi1->a1.disp(U64);
	      else
		tempi1->a1.disp(F64)=tempi1->a1.disp(I64);
	    } else
	      tempi->ic_flags|=ICF_A1_TO_F64;
	  }

	  ty=tempi->a1.type&RT_MASK;
	  ty1=tempi1->a1.type&RT_MASK;
	  if (ty1 && ty1<ty)
	    ty=ty1;
	  ty1=tempi1->r.type&RT_MASK;
	  if (ty1 && ty1<ty)
	    ty=ty1;
	  tempi->a1.type=tempi1->a1.type&TY_MASK+ty;

	  pt=tempi->a1.type_pointed;
	  pt1=tempi1->a1.type_pointed;
	  if (pt1 && pt1<pt && pt!=RT_F64)
	    pt=pt1;
	  pt1=tempi1->r.type_pointed;
	  if (pt1 && pt1<pt && pt!=RT_F64)
	    pt=pt1;
	  tempi->a1.type_pointed=pt;

	  tempi->a1.reg=tempi1->a1.reg;
	  tempi->a1.disp=tempi1->a1.disp;
	  tempi->ic_flags|=tempi1->ic_flags&ICG_NO_CVT_MASK;
	  OptSetNOP2(tempi1);
	}
      }
      if (tempi->r.type&TY_MASK==TY_STK)
	PrsPush(ps,tempi);
    }
    tempi=tempi->next;
  }
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",lx->pass,ps->ptr;
    LexExcept(lx,"Compiler Optimization Error at ");
  }
}
