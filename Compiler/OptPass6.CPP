Bool OptIC6(CIntermediateCode *tempi)
{
  CIntermediateCode *tempil1;
  if (tempi->ic_code<IC_IMM_U64 || !(tempil1=OptLag1(tempi)))
    return FALSE;
  if (tempil1->ic_code==IC_ADD_CONST && tempi->ic_code==IC_DEREF &&
      tempi->ic_flags&ICF_A1_WAS_STK  &&  tempi->a1.type&TY_MASK==TY_REG &&
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

U0 OptPass6Lag(CLex *lx,
  CPrsStk *ps,CIntermediateCode *tempi,I64 *i_,I64 num_stk,
  I64 *_clobbered_reg_mask,I64 fun_num)
{
  I64 i=*i_,j,k,l,code,clobbered_reg_mask=*_clobbered_reg_mask;
  CHashFun *tempf;
  code=tempi->ic_code;
  if (tempi->ic_flags&ICF_TRACE && Bt(&lx->pass_trace_mask,6)) {
    "%2d:",i;
    ICPut(tempi);
  }
  if (code==IC_CALL_START) {
    k=MinI64(i,num_stk);
    j=0;
    //See $LK,"STK TEMP ALLOCATION",A="FF:::/Compiler/CmpInit.CPP.Z,STK TEMP ALLOCATION"$
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
  if (cmp.arg_cnt[code]==9)
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

U0 OptPass6(CLex *lx)
{
  CIntermediateCode *tempi,*tempil1,*tempil2,*tempil3,*tempif1,*tempif2,*old_tempil2;
  I64 i=0,code,num_stk,clobbered_reg_mask=REG_ALWAYS_CLOBBERED_MASK,fun_num=0;
  CPrsStk *ps=lx->ps;
  ps->ptr=0;
  ps->ptr2=0;
  if (lx->htc.fun)
    fun_num=cmp.compiled_funs++;
  if (Bt(&lx->opts,OPTf_NO_REG_VAR) || lx->flags&LF_NO_REG_OPT)
    num_stk=0;
  else
    num_stk=cmp.num_stk_temps;
  tempi=lx->lcc.lcc_root.next;
  old_tempil2=NULL;
  tempil1=tempil2=tempil3=&cmp.ic_nop;
  while (code=tempi->ic_code) {
    if (		  code>=IC_END_EXP &&		    code!=IC_NOP2) {
      if (  tempil1->ic_code>=IC_END_EXP && tempil1->ic_code!=IC_NOP2) {
	if (tempil2->ic_code>=IC_END_EXP && tempil2->ic_code!=IC_NOP2)
	  tempil3=tempil2;
	tempil2=tempil1;
      }
      tempil1=tempi;
    }
    if (tempil2->ic_code==IC_END_EXP && code!=IC_RETURN_VAL2 && code!=IC_NOP1 &&
       !(tempil3->ic_flags&(ICF_R_TO_F64|ICF_R_TO_INT|ICF_PUSH_RESULT|ICF_DEPEND_RESULT))) {
      tempil3->ic_flags|=ICF_NO_DEPEND_RESULT;
      tempil3->r.type=TY_NULL+tempil3->r.type&RT_MASK;
      tempil2->ic_flags|=ICF_NO_DEPEND_RESULT;
      tempil2->r.type=TY_NULL+tempil2->r.type&RT_MASK;
    }
    if (code>IC_NOP2) {
      if (tempi->a2.type&TY_MASK==TY_STK) {
	if (tempil2->r.type&TY_MASK==TY_STK &&
	      !(tempil2->ic_flags&ICF_PUSH_RESULT)) {
	  tempif2=NULL;
	  if (tempi->ic_code==IC_ASSIGN &&
		tempi->ic_flags&ICF_NOT_ADDR &&
		tempil2->ic_code!=IC_CALL_END  &&
		tempil2->ic_code!=IC_CALL_END2 &&
		tempil2->ic_code!=IC_SET_RAX	 &&
		!(tempi->ic_flags&
		(ICF_A2_TO_F64|ICF_A2_TO_INT)) &&
		!(tempil2->ic_flags&
		(ICF_R_TO_F64|ICF_R_TO_INT))) {
	    if (tempif1=OptLead1(tempi)) //predict NO_DEPEND
	      if (tempif2=OptLead1(tempif1))
		if (tempif1->ic_code==IC_END_EXP &&
		      tempif2->ic_code!=IC_RETURN_VAL2) {
		  tempil2->r.type =tempi->a1.type&TY_MASK+tempi->a1.type_pointed;
		  tempil2->r.type_pointed=tempi->a1.type_pointed;
		  tempil2->r.reg  =tempi->a1.reg;
		  tempil2->r.disp =tempi->a1.disp;
		  tempil2->ic_flags=tempil2->ic_flags&~(ICF_NO_DEPEND_RESULT|ICF_R_WAS_STK)|
			tempi->ic_flags&~(ICF_NOT_ADDR|ICF_A1_WAS_STK|ICF_A2_WAS_STK)|ICF_DEPEND_RESULT;
		  old_tempil2=NULL;
		  OptSetNOP1(tempi);
		} else
		  tempif2=NULL;
	  }
	  if (!tempif2) {
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
    }
    if (old_tempil2!=tempil2) {
      if (tempil2->ic_code>IC_NOP2)
	OptPass6Lag(lx,ps,tempil2,&i,num_stk,&clobbered_reg_mask,fun_num);
      old_tempil2=tempil2;
    }
    tempi=tempi->next;
  }
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",lx->pass,ps->ptr;
    LexExcept(lx,"Compiler Optimization Error at ");
  }
  if (lx->htc.fun) {
    lx->htc.fun->used_reg_mask&=~REG_STK_TEMPS_MASK;
    lx->htc.fun->used_reg_mask|=clobbered_reg_mask;
    lx->htc.fun->clobbered_reg_mask=clobbered_reg_mask;
    if (Bt(&lx->flags,Lf_OPT_TRACE_PRESENT) &&
	  Bt(&lx->pass_trace_mask,6)) {
      "Used      Reg Mask:%04X\n",lx->htc.fun->used_reg_mask;
      "Clobbered Reg Mask:%04X\n",clobbered_reg_mask;
    }
  }
}
