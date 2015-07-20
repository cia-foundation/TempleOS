Bool OptIC6(CIntermediateCode *tempi)
{
  CIntermediateCode *tempil1;
  if (tempi->ic_code<IC_IMM_I64 || !(tempil1=OptLag1(tempi)))
    return FALSE;
  if (tempil1->ic_code==IC_ADD_CONST && tempi->ic_code==IC_DEREF &&
	tempi->ic_flags&ICF_A1_WAS_STK	&&  tempi->a1.type&TY_MASK==TY_REG &&
	tempil1->r.type&TY_MASK==TY_REG && MIN_I32<=tempil1->ic_data<=MAX_I32 &&
	!Bt(&cmp.non_ptr_vars_mask,tempil1->a1.reg)) {
    if (tempil1->a1.type&TY_MASK==TY_REG) {
      tempi->ic_flags=tempi->ic_flags&~ICF_A1_WAS_STK | tempil1->ic_flags;
      tempi->ic_code=IC_MOV;
      tempi->a1.type=TY_DISP+tempi->a1.type_pointed;
      tempi->a1.reg=tempil1->a1.reg;
      tempi->a1.disp=tempil1->ic_data;
      OptSetNOP2(tempil1,-1);
    } else {
      tempil1->ic_code=IC_MOV;
      tempi->ic_code=IC_MOV;
      tempi->a1.type=TY_DISP+tempi->a1.type_pointed;
      tempi->a1.disp=tempil1->ic_data;
    }
    return TRUE;
  }
  return FALSE;
}

U0 OptPass6Lag(CCmpCtrl *cc,CPrsStk *ps,CIntermediateCode *tempi,
  I64 *i_,I64 num_stk,I64 *_clobbered_reg_mask,I64 fun_num)
{
  I64 i=*i_,j,k,l,code,clobbered_reg_mask=*_clobbered_reg_mask;
  CHashFun *tempf;
  code=tempi->ic_code;
  if (tempi->ic_flags&ICF_TRACE && Bt(&cc->otrace_mask,6)) {
    "%2d:",i;
    ICPut(cc,tempi);
  }
  if (code==IC_CALL_START) {
    k=MinI64(i,num_stk);
    j=0;
//See $LK,"STK TEMP ALLOCATION",A="FF:::/Compiler/CInit.CPP,STK TEMP ALLOCATION"$
    if (fun_num&1)
      for (l=0;l<k;l++)
	Bts(&j,cmp.to_stk_temps_map_odd[l]);
    else
      for (l=0;l<k;l++)
	Bts(&j,cmp.to_stk_temps_map_even[l]);
    if (tempf=tempi->ic_data) {
      if (Bt(&tempf->flags,Ff_INTERNAL))
	j=0;
      else {
	j&=tempf->clobbered_reg_mask;
	clobbered_reg_mask|=tempf->clobbered_reg_mask;
      }
    }
    tempi->ic_data=j;
    PrsPush(ps,i);
    PrsPush(ps,j);
  } else if (code==IC_CALL_END) {
    tempi->ic_data=PrsPop(ps);
    i=PrsPop(ps);
  } else if (code==IC_CALL_END2) {
    ps->ptr--;
    i=PrsPop(ps);
  }
  if (intermediate_code_table[code].arg_cnt==IS_V_ARG)
    i-=tempi->ic_data>>3;
  if (tempi->a2.type&TY_MASK==TY_STK) {
    i--;
    if (i<num_stk) {
      tempi->a2.type=TY_REG+tempi->a2.type&RT_MASK;
      if (fun_num&1)
	tempi->a2.reg=cmp.to_stk_temps_map_odd[i];
      else
	tempi->a2.reg=cmp.to_stk_temps_map_even[i];
      tempi->a2.disp=0;
    }
  }
  if (tempi->a1.type&TY_MASK==TY_STK) {
    i--;
    if (i<num_stk) {
      tempi->a1.type=TY_REG+tempi->a1.type&RT_MASK;
      if (fun_num&1)
	tempi->a1.reg=cmp.to_stk_temps_map_odd[i];
      else
	tempi->a1.reg=cmp.to_stk_temps_map_even[i];
      tempi->a1.disp=0;
    }
  }
  if (tempi->r.type&TY_MASK==TY_STK && !(tempi->ic_flags & ICF_PUSH_RESULT)) {
    i++;
    if (i<=num_stk) {
      tempi->r.type=TY_REG+tempi->r.type&RT_MASK;
      if (fun_num&1) {
	tempi->r.reg=cmp.to_stk_temps_map_odd[i-1];
	Bts(&clobbered_reg_mask,cmp.to_stk_temps_map_odd[i-1]);
      } else {
	tempi->r.reg=cmp.to_stk_temps_map_even[i-1];
	Bts(&clobbered_reg_mask,cmp.to_stk_temps_map_even[i-1]);
      }
      tempi->r.disp=0;
    }
  }
  while (OptIC6(tempi));
  if (tempi->r.type&RT_MASK!=RT_F64 && !(tempi->ic_flags&ICF_USE_F64))
    tempi->ic_flags|=ICF_USE_INT;
  *i_=i;
  *_clobbered_reg_mask=clobbered_reg_mask;
}

U0 OptPass6(CCmpCtrl *cc)
{
  CIntermediateCode *tempi,*tempi_next,*tempil1,*tempil2,*old_tempil2;
  I64 i=0,num_stk,clobbered_reg_mask=REGG_C_CLOBBERED,fun_num=0;
  CPrsStk *ps=cc->ps;
  ps->ptr=0;
  ps->ptr2=0;
  if (Bt(&cc->opts,OPTf_NO_REG_VAR) || cc->flags&CCF_NO_REG_OPT)
    num_stk=0;
  else
    num_stk=cmp.num_stk_temps;
  tempi=cc->coc.coc_root.next;
  old_tempil2=NULL;
  tempil1=tempil2=&cmp.ic_nop;
  while (tempi->ic_code) {
    if (tempi->ic_code>IC_NOP2) {
      if (tempil1->ic_code>IC_NOP2)
	tempil2=tempil1;
      tempil1=tempi;
      if (tempi->a2.type&TY_MASK==TY_STK) {
	if (tempil2->r.type&TY_MASK==TY_STK &&
	      !(tempil2->ic_flags&ICF_PUSH_RESULT)) {
	  if (tempi->ic_code==IC_ASSIGN && tempi->ic_flags&ICF_NOT_ADDR &&
		tempi->ic_flags&ICF_NO_DEPEND_RESULT &&
		tempil2->ic_code!=IC_CALL_END &&
		tempil2->ic_code!=IC_CALL_END2 &&
		tempil2->ic_code!=IC_SET_RAX	 &&
		!(tempi->ic_flags&(ICF_A2_TO_F64|ICF_A2_TO_INT)) &&
		!(tempil2->ic_flags&(ICF_R_TO_F64|ICF_R_TO_INT))) {
	    tempil2->r.type =tempi->a1.type&TY_MASK+tempi->a1.type_pointed;
	    tempil2->r.type_pointed=tempi->a1.type_pointed;
	    tempil2->r.reg  =tempi->a1.reg;
	    tempil2->r.disp =tempi->a1.disp;
	    tempil2->ic_flags=tempil2->ic_flags
		  &~(ICF_NO_DEPEND_RESULT|ICF_R_WAS_STK)
	    |tempi->ic_flags&~(ICF_NOT_ADDR|ICF_A1_WAS_STK|ICF_A2_WAS_STK)
	    |ICF_DEPEND_RESULT;
	    old_tempil2=NULL;
	    OptSetNOP1(tempi);
	  } else {
	    tempi->a2.type=TY_REG+tempi->a2.type&RT_MASK;
	    tempi->a2.reg=REG_RAX;
	    tempi->a2.disp=0;
	    tempil2->r.type=TY_REG+tempil2->r.type&RT_MASK;
	    tempil2->r.reg=REG_RAX;
	    tempil2->r.disp=0;
	  }
	}
      } else if (tempi->a1.type&TY_MASK==TY_STK &&
	    tempil2->r.type&TY_MASK==TY_STK &&
	    !(tempil2->ic_flags&ICF_PUSH_RESULT)) {
	tempi->a1.type=TY_REG+tempi->a1.type&RT_MASK;
	tempi->a1.reg=REG_RAX;
	tempi->a1.disp=0;
	tempil2->r.type=TY_REG+tempil2->r.type&RT_MASK;
	tempil2->r.reg=REG_RAX;
	tempil2->r.disp=0;
      }
      if (tempi->ic_flags & ICF_PUSH_RESULT)
	tempi->r.type==TY_STK+tempi->r.type&RT_MASK;
      if (old_tempil2!=tempil2) {
	if (tempil2->ic_code>IC_NOP2)
	  OptPass6Lag(cc,ps,tempil2,&i,num_stk,&clobbered_reg_mask,fun_num);
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
    cc->htc.fun->used_reg_mask&=~REGG_C_TEMP_CLOBBERED;
    cc->htc.fun->used_reg_mask|=clobbered_reg_mask;
    cc->htc.fun->clobbered_reg_mask=clobbered_reg_mask;
    if (Bt(&cc->flags,CCf_OPT_TRACE_PRESENT) &&
	  Bt(&cc->otrace_mask,6)) {
      "Used      Reg Mask:%04X\n",cc->htc.fun->used_reg_mask;
      "Clobbered Reg Mask:%04X\n",clobbered_reg_mask;
    }
  }
}
