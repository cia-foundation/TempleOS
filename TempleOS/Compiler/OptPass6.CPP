Bool OptIC6(CIntermediateCode *tempi)
{
  CIntermediateCode *tempil1,*tempil2;
  if (tempi->ic_opcode<IC_IMM_U64) return FALSE;
  tempil1=tempi;
  if (!(tempil2=OptLag1(tempil1)))
    return FALSE;
  if (tempil2->ic_opcode==IC_ADD_CONST &&
      tempil1->ic_opcode==IC_DEREF &&
      tempil1->ic_flags&ICF_P1_WAS_STK &&
      tempil1->p1.type&TY_MASK==TY_REG &&
      tempil2->r.type&TY_MASK==TY_REG &&
      MIN_I32<=tempil2->ic_data<=MAX_I32 &&
      !Bt(&non_ptr_vars_mask,tempil2->p1.reg))
    if (tempil2->p1.type&TY_MASK==TY_REG) {
      tempil1->ic_flags= tempil1->ic_flags & ~ICF_P1_WAS_STK |
			 tempil2->ic_flags;
      tempil1->ic_opcode=IC_MOV;
      tempil1->p1.type=TY_DISP+tempil1->p1.ptype;
      tempil1->p1.reg=tempil2->p1.reg;
      tempil1->p1.disp=tempil2->ic_data;
      OptSetNOP2(tempil2,-1);
      return TRUE;
    } else {
      tempil2->ic_opcode=IC_MOV;
      tempil1->ic_opcode=IC_MOV;
      tempil1->p1.type=TY_DISP+tempil1->p1.ptype;
      tempil1->p1.disp=tempil2->ic_data;
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
  code=tempi->ic_opcode;
  if (code<=IC_NOP2)
    return;
  else {
    if (tempi->ic_flags&ICF_TRACE) {
      if (Bt(&lx->pass_trace_mask,6)) {
	"%2d:",i;
	CBPutIC(tempi);
      }
    }
    if (code==IC_CALL_START) {
      if (i<num_stk)
	k=i;
      else
	k=num_stk;
      j=0;
      if (fun_num&1)
	for (l=0;l<k;l++)
	  Bts(&j,to_stk_temps_map_odd[l]);
	else
	for (l=0;l<k;l++)
	  Bts(&j,to_stk_temps_map_even[l]);
	if (tempf=tempi->ic_data) {
	if (Bt(tempf->flags,Ff_INTERNAL))
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
    }
    // else if (code==IC_LEAVE)
    //	i=0;
    else if (code==IC_CALL_END2) {
      ps->ptr--;
      i=PrsPop(ps);
    }
    if (cmp_arg_cnt[code]==9)
      i-=tempi->ic_data>>3;
    if (tempi->p2.type&TY_MASK==TY_STK) {
      i--;
      if (i<num_stk) {
	tempi->p2.type=TY_REG+tempi->p2.type&IT_MASK;
	if (fun_num&1)
	  tempi->p2.reg=to_stk_temps_map_odd[i];
	else
	  tempi->p2.reg=to_stk_temps_map_even[i];
	tempi->p2.disp=0;
      }
    }
    if (tempi->p1.type&TY_MASK==TY_STK) {
      i--;
      if (i<num_stk) {
	tempi->p1.type=TY_REG+tempi->p1.type&IT_MASK;
	if (fun_num&1)
	  tempi->p1.reg=to_stk_temps_map_odd[i];
	else
	  tempi->p1.reg=to_stk_temps_map_even[i];
	tempi->p1.disp=0;
      }
    }
    if (tempi->r.type&TY_MASK==TY_STK &&
	!(tempi->ic_flags & ICF_PUSH_RESULT)) {
      i++;
      if (i<=num_stk) {
	tempi->r.type=TY_REG+tempi->r.type&IT_MASK;
	if (fun_num&1) {
	  tempi->r.reg=to_stk_temps_map_odd[i-1];
	  Bts(&clobbered_reg_mask,to_stk_temps_map_odd[i-1]);
	} else {
	  tempi->r.reg=to_stk_temps_map_even[i-1];
	  Bts(&clobbered_reg_mask,to_stk_temps_map_even[i-1]);
	}
	tempi->r.disp=0;
      }
    }
    while (OptIC6(tempi));
    if (tempi->r.type&IT_MASK!=IT_F64 &&
	!(tempi->ic_flags&ICF_USE_F64))
      tempi->ic_flags|=ICF_USE_INT;
  }
  *i_=i;
  *_clobbered_reg_mask=clobbered_reg_mask;
}

U0 OptPass6(CLex *lx,CHashFun *fun_class)
{
  CIntermediateCode *tempi,*tempil1,*tempif1,*tempif2,
	*temp_lag[4],*old_tempil1=NULL;
  CPrsStk *ps=OptStart(lx,temp_lag);
  I64 i=0,code,num_stk,clobbered_reg_mask=CALWAYS_CLOBBERED_MASK,
	fun_num=0;

  if (fun_class)
    fun_num=compiled_funs++;

  if (Bt(&lx->opts,OPTf_NO_REG_VAR) ||
	lx->flags&LF_NO_REG_OPT)
    num_stk=0;
  else
    num_stk=num_stk_temps;

  tempi=lx->cbh.cb_in_ptr;
  if (tempi->ic_opcode==IC_START)
    tempi++;
  while (code=tempi->ic_opcode) {
    OptNext3(tempi,temp_lag);
    tempil1=temp_lag[1];
    if (tempil1->ic_opcode==IC_END_EXP) {
      if (code!=IC_RETURN_VAL2 && code!=IC_NOP1) {
	if (!(temp_lag[2]->ic_flags&
	      (ICF_R_TO_F64|ICF_R_TO_INT|ICF_PUSH_RESULT|ICF_DEPEND_RESULT))) {
	  temp_lag[2]->ic_flags|=ICF_NO_DEPEND_RESULT;
	  temp_lag[2]->r.type=TY_NULL+temp_lag[2]->r.type&IT_MASK;
	  tempil1->ic_flags|=ICF_NO_DEPEND_RESULT;
	  tempil1->r.type=TY_NULL+tempil1->r.type&IT_MASK;
	}
      }
    }
    if (code<=IC_NOP2) {
    } else {
      if (tempi->p2.type&TY_MASK==TY_STK) {
	if (tempil1->r.type&TY_MASK==TY_STK &&
	      !(tempil1->ic_flags&ICF_PUSH_RESULT)) {
	  tempif2=NULL;
	  if (tempi->ic_opcode==IC_ASSIGN &&
		tempi->ic_flags&ICF_NOT_ADDR &&
		tempil1->ic_opcode!=IC_CALL_END  &&
		tempil1->ic_opcode!=IC_CALL_END2 &&
		tempil1->ic_opcode!=IC_SET_RAX	 &&
		!(tempi->ic_flags&
		(ICF_P2_TO_F64|ICF_P2_TO_INT)) &&
		!(tempil1->ic_flags&
		(ICF_R_TO_F64|ICF_R_TO_INT))) {
	    if (tempif1=OptLead1(lx,tempi)) //predict NO_DEPEND
	      if (tempif2=OptLead1(lx,tempif1))
		if (tempif1->ic_opcode==IC_END_EXP &&
		      tempif2->ic_opcode!=IC_RETURN_VAL2) {
		  tempil1->r.type =tempi->p1.type&TY_MASK+tempi->p1.ptype;
		  tempil1->r.ptype=tempi->p1.ptype;
		  tempil1->r.reg  =tempi->p1.reg;
		  tempil1->r.disp =tempi->p1.disp;
		  tempil1->ic_flags=tempil1->ic_flags&~(ICF_NO_DEPEND_RESULT|ICF_R_WAS_STK)|
			tempi->ic_flags&~(ICF_NOT_ADDR|ICF_P1_WAS_STK|ICF_P2_WAS_STK)|ICF_DEPEND_RESULT;
		  old_tempil1=NULL;
		  OptSetNOP1(tempi);
		} else
		  tempif2=NULL;
	  }
	  if (!tempif2) {
	    tempi->p2.type=TY_REG+tempi->p2.type&IT_MASK;
	    tempi->p2.reg=CREG_RAX;
	    tempi->p2.disp=0;
	    tempil1->r.type=TY_REG+tempil1->r.type&IT_MASK;
	    tempil1->r.reg=CREG_RAX;
	    tempil1->r.disp=0;
	  }
	}
      } else if (tempi->p1.type&TY_MASK==TY_STK) {
	if (tempil1->r.type&TY_MASK==TY_STK &&
	      !(tempil1->ic_flags&ICF_PUSH_RESULT)) {
	  tempi->p1.type=TY_REG+tempi->p1.type&IT_MASK;
	  tempi->p1.reg=CREG_RAX;
	  tempi->p1.disp=0;
	  tempil1->r.type=TY_REG+tempil1->r.type&IT_MASK;
	  tempil1->r.reg=CREG_RAX;
	  tempil1->r.disp=0;
	}
      }
      if (tempi->ic_flags & ICF_PUSH_RESULT)
	tempi->r.type==TY_STK+tempi->r.type&IT_MASK;

    }
    if (old_tempil1!=tempil1) {
      OptPass6Lag(lx,ps,tempil1,&i,num_stk,&clobbered_reg_mask,fun_num);
      old_tempil1=tempil1;
    }
    tempi++;
  }
  OptEnd(lx,ps);

  if (fun_class) {
    fun_class->used_reg_mask&=~CSTK_TEMPS_MASK;
    fun_class->used_reg_mask|=clobbered_reg_mask;
    fun_class->clobbered_reg_mask=clobbered_reg_mask;
    if (Bt(&lx->flags,Lf_OPT_TRACE_PRESENT) &&
	  Bt(&lx->pass_trace_mask,6)) {
      "Used      Reg Mask:%04X\n",fun_class->used_reg_mask;
      "Clobbered Reg Mask:%04X\n",clobbered_reg_mask;
    }
  }
}
