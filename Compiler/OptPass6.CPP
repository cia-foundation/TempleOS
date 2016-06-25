Bool OptIC6(CIntermediateCode *tempi)
{
  CIntermediateCode *tempil1;
  if (tempi->ic_code<IC_IMM_I64 || !(tempil1=OptLag1(tempi)))
    return FALSE;
  if (tempil1->ic_code==IC_ADD_CONST && tempi->ic_code==IC_DEREF &&
	tempi->ic_flags&ICF_A1_WAS_STK	&&  tempi->a1.type&MDF_REG &&
	tempil1->r.type&MDF_REG && MIN_I32<=tempil1->ic_data<=MAX_I32 &&
	!Bt(&cmp.non_ptr_vars_mask,tempil1->a1.reg)) {
    if (tempil1->a1.type&MDF_REG) {
      tempi->ic_flags=tempi->ic_flags&~ICF_A1_WAS_STK | tempil1->ic_flags;
      tempi->ic_code=IC_MOV;
      tempi->a1.type=MDF_DISP+tempi->a1_type_pointed_to;
      tempi->a1.reg=tempil1->a1.reg;
      tempi->a1.disp=tempil1->ic_data;
      OptSetNOP2(tempil1,-1);
    } else {
      tempil1->ic_code=IC_MOV;
      tempi->ic_code=IC_MOV;
      tempi->a1.type=MDF_DISP+tempi->a1_type_pointed_to;
      tempi->a1.disp=tempil1->ic_data;
    }
    return TRUE;
  }
  return FALSE;
}

U0 OptPass6Lag(CCmpCtrl *cc,CPrsStk *ps,CIntermediateCode *tempi,
	I64 *_stk_ptr,I64 reg_stk_size,I64 *_clobbered_reg_mask)
{
  I64 stk_ptr=*_stk_ptr,code,
	clobbered_stk_temp_mask,clobbered_reg_mask=*_clobbered_reg_mask;
  CHashFun *tempf;
  code=tempi->ic_code;
  if (tempi->ic_flags&ICF_PASS_TRACE && Bt(&cc->saved_pass_trace,6)) {
    "%2d:",stk_ptr;
    ICPut(cc,tempi);
  }
  if (code==IC_CALL_START) {
    if (reg_stk_size==1 && stk_ptr>0)
      clobbered_stk_temp_mask=REGG_STK_TEMP;
    else
      clobbered_stk_temp_mask=0;
    if (tempf=tempi->ic_data) {
      if (Bt(&tempf->flags,Ff_INTERNAL))
	clobbered_stk_temp_mask=0;
      else {
	clobbered_stk_temp_mask&=tempf->clobbered_reg_mask;
	clobbered_reg_mask|=tempf->clobbered_reg_mask;
      }
    }
    tempi->ic_data=clobbered_stk_temp_mask;
    PrsPush(ps,stk_ptr);
    PrsPush(ps,clobbered_stk_temp_mask);
  } else if (code==IC_CALL_END) {
    tempi->ic_data=PrsPop(ps);
    stk_ptr=PrsPop(ps);
  } else if (code==IC_CALL_END2) {
    ps->ptr--;
    stk_ptr=PrsPop(ps);
  }
  if (intermediate_code_table[code].arg_cnt==IS_V_ARG)
    stk_ptr-=tempi->ic_data>>3;
  if (tempi->a2.type&MDF_STK) {
    stk_ptr--;
    if (stk_ptr<reg_stk_size) {
      tempi->a2.type=MDF_REG+tempi->a2.type.raw_type;
      tempi->a2.reg=Bsf(REGG_STK_TEMP);
      tempi->a2.disp=0;
    }
  }
  if (tempi->a1.type&MDF_STK) {
    stk_ptr--;
    if (stk_ptr<reg_stk_size) {
      tempi->a1.type=MDF_REG+tempi->a1.type.raw_type;
      tempi->a1.reg=Bsf(REGG_STK_TEMP);
      tempi->a1.disp=0;
    }
  }
  if (tempi->r.type&MDF_STK && !(tempi->ic_flags & ICF_PUSH_RES)) {
    stk_ptr++;
    if (stk_ptr<=reg_stk_size) {
      tempi->r.type=MDF_REG+tempi->r.type.raw_type;
      tempi->r.reg=Bsf(REGG_STK_TEMP);
      clobbered_reg_mask|=REGG_STK_TEMP;
      tempi->r.disp=0;
    }
  }
  while (OptIC6(tempi));
  if (tempi->r.type.raw_type!=RT_F64 && !(tempi->ic_flags&ICF_USE_F64))
    tempi->ic_flags|=ICF_USE_INT;
  *_stk_ptr=stk_ptr;
  *_clobbered_reg_mask=clobbered_reg_mask;
}

U0 OptPass6(CCmpCtrl *cc)
{
  CIntermediateCode *tempi,*tempi_next,*tempil1,*tempil2,*old_tempil2;
  I64 stk_ptr=0,reg_stk_size,clobbered_reg_mask=REGG_CLOBBERED;
  CPrsStk *ps=cc->ps;
  ps->ptr=0;
  ps->ptr2=0;
  if (Bt(&cc->opts,OPTf_NO_REG_VAR) || cc->flags&CCF_NO_REG_OPT)
    reg_stk_size=0;
  else
    reg_stk_size=1;
#assert REGG_STK_TEMP==1<<9
  tempi=cc->coc.coc_root.next;
  old_tempil2=NULL;
  tempil1=tempil2=&cmp.ic_nop;
  while (tempi->ic_code) {
    if (tempi->ic_code>IC_NOP2) {
      if (tempil1->ic_code>IC_NOP2)
	tempil2=tempil1;
      tempil1=tempi;
      if (tempi->a2.type&MDF_STK) {
	if (tempil2->r.type&MDF_STK &&
	      !(tempil2->ic_flags&ICF_PUSH_RES)) {
	  if (tempi->ic_code==IC_ASSIGN && tempi->ic_flags&ICF_BY_VAL &&
		tempi->ic_flags&ICF_RES_NOT_USED &&
		tempil2->ic_code!=IC_CALL_END &&
		tempil2->ic_code!=IC_CALL_END2 &&
		tempil2->ic_code!=IC_SET_RAX &&
		!(tempi->ic_flags&(ICF_A2_TO_F64|ICF_A2_TO_INT)) &&
		!(tempil2->ic_flags&(ICF_R_TO_F64|ICF_R_TO_INT))) {
	    tempil2->r.type =tempi->a1.type&MDG_MASK+tempi->a1_type_pointed_to;
	    tempil2->r.reg  =tempi->a1.reg;
	    tempil2->r.disp =tempi->a1.disp;
	    tempil2->ic_flags=tempil2->ic_flags
		  &~(ICF_RES_NOT_USED|ICF_R_WAS_STK)
	    |tempi->ic_flags&~(ICF_BY_VAL|ICF_A1_WAS_STK|ICF_A2_WAS_STK);
	    old_tempil2=NULL;
	    OptSetNOP1(tempi);
	  } else {
	    tempi->a2.type=MDF_REG+tempi->a2.type.raw_type;
	    tempi->a2.reg=REG_RAX;
	    tempi->a2.disp=0;
	    tempil2->r.type=MDF_REG+tempil2->r.type.raw_type;
	    tempil2->r.reg=REG_RAX;
	    tempil2->r.disp=0;
	  }
	}
      } else if (tempi->a1.type&MDF_STK && tempil2->r.type&MDF_STK &&
	    !(tempil2->ic_flags&ICF_PUSH_RES)) {
	tempi->a1.type=MDF_REG+tempi->a1.type.raw_type;
	tempi->a1.disp=0;
	tempil2->r.type=MDF_REG+tempil2->r.type.raw_type;
	tempil2->r.disp=0;
	if (intermediate_code_table[tempi->ic_code].arg_cnt==IS_2_ARG) {
	  tempi->a1.reg=REG_R8;
	  tempil2->r.reg=REG_R8;
	} else {
	  tempi->a1.reg=REG_RAX;
	  tempil2->r.reg=REG_RAX;
	}
      }
      if (tempi->ic_flags & ICF_PUSH_RES)
	tempi->r.type==MDF_STK+tempi->r.type.raw_type;
      if (old_tempil2!=tempil2) {
	if (tempil2->ic_code>IC_NOP2)
	  OptPass6Lag(cc,ps,tempil2,&stk_ptr,reg_stk_size,&clobbered_reg_mask);
	old_tempil2=tempil2;
      }
    }
    tempi_next=tempi->next;
    if (tempi->ic_code<=IC_NOP2)
      OptFree(tempi);
    tempi=tempi_next;
  }
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",cc->pass,ps->ptr;
    LexExcept(cc,"Compiler Optimization Error at ");
  }
  if (cc->htc.fun) {
    cc->htc.fun->used_reg_mask&=~REGG_STK_TEMP;
    cc->htc.fun->used_reg_mask|=clobbered_reg_mask;
    cc->htc.fun->clobbered_reg_mask=clobbered_reg_mask;
    if (Bt(&cc->flags,CCf_PASS_TRACE_PRESENT) &&
	  Bt(&cc->saved_pass_trace,6)) {
      "Used      Reg Mask:%04X\n",cc->htc.fun->used_reg_mask;
      "Clobbered Reg Mask:%04X\n",clobbered_reg_mask;
    }
  }
}
