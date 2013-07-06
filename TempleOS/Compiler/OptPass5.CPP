U0 OptPass5(CLex *lx)
{
  CIntermediateCode *tempi,*tempi1;
  I64 code,i,ty,pt,ty1,pt1;
  CPrsStk *ps=OptStart(lx);

  tempi=lx->cbh.cb_in_ptr;
  if (tempi->ic_opcode==IC_START)
    tempi++;
  while (code=tempi->ic_opcode) {
    if (code>IC_NOP2) {
      if (tempi->ic_flags&ICF_TRACE) {
	if (Bt(&lx->pass_trace_mask,5)) {
	  "%2d:",ps->ptr;
	  CBPutIC(tempi);
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

      if (tempi->p2.type&TY_MASK==TY_STK) {
	tempi1=PrsPop(ps);
	if (tempi1->ic_opcode==IC_MOV ||
	    tempi1->ic_opcode==IC_REG) {
	  if (tempi1->ic_flags & ICF_R_TO_INT) {
	    if (tempi1->p1.type&TY_MASK==TY_IMM)
	      tempi1->p1.disp=tempi1->p1.disp(F64);
	    else
	      tempi->ic_flags|=ICF_P2_TO_INT;
	  } else if (tempi1->ic_flags&ICF_R_TO_F64) {
	    if (tempi1->p1.type&TY_MASK==TY_IMM)
	      tempi1->p1.disp(F64)=tempi1->p1.disp;
	    else
	      tempi->ic_flags|=ICF_P2_TO_F64;
	  }

	  ty=tempi->p2.type&IT_MASK;
	  ty1=tempi1->p1.type&IT_MASK;
	  if (ty1 && ty1<ty)
	    ty=ty1;
	  ty1=tempi1->r.type&IT_MASK;
	  if (ty1 && ty1<ty)
	    ty=ty1;
	  tempi->p2.type=tempi1->p1.type&TY_MASK+ty;

	  pt=tempi->p2.ptype;
	  pt1=tempi1->p1.ptype;
	  if (pt1 && pt1<pt && pt!=IT_F64)
	    pt=pt1;
	  pt1=tempi1->r.ptype;
	  if (pt1 && pt1<pt && pt!=IT_F64)
	    pt=pt1;
	  tempi->p2.ptype=pt;

	  tempi->p2.reg=tempi1->p1.reg;
	  tempi->p2.disp=tempi1->p1.disp;
	  tempi->ic_flags|=tempi1->ic_flags&ICF_NO_CVT_MASK;
	  OptSetNOP2(tempi1);
	}
      }

      if (tempi->p1.type&TY_MASK==TY_STK) {
	tempi1=PrsPop(ps);
	if (tempi1->ic_opcode==IC_MOV ||
	    tempi1->ic_opcode==IC_REG) {
	  if (tempi1->ic_flags & ICF_R_TO_INT) {
	    if (tempi1->p1.type&TY_MASK==TY_IMM)
	      tempi1->p1.disp=tempi1->p1.disp(F64);
	    else
	      tempi->ic_flags|=ICF_P1_TO_INT;
	  } else if (tempi1->ic_flags&ICF_R_TO_F64) {
	    if (tempi1->p1.type&TY_MASK==TY_IMM) {
	      if (tempi1->p1.type & 1) //signed/unsigned
		tempi1->p1.disp(F64)=tempi1->p1.disp(U64);
	      else
		tempi1->p1.disp(F64)=tempi1->p1.disp(I64);
	    } else
	      tempi->ic_flags|=ICF_P1_TO_F64;
	  }

	  ty=tempi->p1.type&IT_MASK;
	  ty1=tempi1->p1.type&IT_MASK;
	  if (ty1 && ty1<ty)
	    ty=ty1;
	  ty1=tempi1->r.type&IT_MASK;
	  if (ty1 && ty1<ty)
	    ty=ty1;
	  tempi->p1.type=tempi1->p1.type&TY_MASK+ty;

	  pt=tempi->p1.ptype;
	  pt1=tempi1->p1.ptype;
	  if (pt1 && pt1<pt && pt!=IT_F64)
	    pt=pt1;
	  pt1=tempi1->r.ptype;
	  if (pt1 && pt1<pt && pt!=IT_F64)
	    pt=pt1;
	  tempi->p1.ptype=pt;

	  tempi->p1.reg=tempi1->p1.reg;
	  tempi->p1.disp=tempi1->p1.disp;
	  tempi->ic_flags|=tempi1->ic_flags&ICF_NO_CVT_MASK;
	  OptSetNOP2(tempi1);
	}
      }
      if (tempi->r.type&TY_MASK==TY_STK)
	PrsPush(ps,tempi);
    }
    tempi++;
  }
  OptEnd(lx,ps);
}
