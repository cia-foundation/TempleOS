Bool OptIC6(CIntermediateCode *tmpi)
{
  CIntermediateCode *tmpil1;
  if (tmpi->ic_code<IC_IMM_I64 || !(tmpil1=OptLag1(tmpi)))
    return FALSE;
  if (tmpil1->ic_code==IC_ADD_CONST && tmpi->ic_code==IC_DEREF &&
	tmpi->ic_flags&ICF_ARG1_WAS_STK	&&  tmpi->arg1.type&MDF_REG &&
	tmpil1->res.type&MDF_REG && I32_MIN<=tmpil1->ic_data<=I32_MAX &&
	!Bt(&cmp.non_ptr_vars_mask,tmpil1->arg1.reg)) {
    if (tmpil1->arg1.type&MDF_REG) {
      tmpi->ic_flags=tmpi->ic_flags&~ICF_ARG1_WAS_STK | tmpil1->ic_flags;
      tmpi->ic_code=IC_MOV;
      tmpi->arg1.type=MDF_DISP+tmpi->arg1_type_pointed_to;
      tmpi->arg1.reg=tmpil1->arg1.reg;
      tmpi->arg1.disp=tmpil1->ic_data;
      OptSetNOP2(tmpil1,-1);
    } else {
      tmpil1->ic_code=IC_MOV;
      tmpi->ic_code=IC_MOV;
      tmpi->arg1.type=MDF_DISP+tmpi->arg1_type_pointed_to;
      tmpi->arg1.disp=tmpil1->ic_data;
    }
    return TRUE;
  }
  return FALSE;
}

U0 OptPass6Lag(CCmpCtrl *cc,CPrsStk *ps,CIntermediateCode *tmpi,
	I64 *_stk_ptr,I64 reg_stk_size,I64 *_clobbered_reg_mask)
{
  I64 stk_ptr=*_stk_ptr,code,
	clobbered_stk_tmp_mask,clobbered_reg_mask=*_clobbered_reg_mask;
  CHashFun *tmpf;
  code=tmpi->ic_code;
  if (tmpi->ic_flags&ICF_PASS_TRACE && Bt(&cc->saved_pass_trace,6)) {
    "%2d:",stk_ptr;
    ICPut(cc,tmpi);
  }
  if (code==IC_CALL_START) {
    if (reg_stk_size==1 && stk_ptr>0)
      clobbered_stk_tmp_mask=REGG_STK_TMP;
    else
      clobbered_stk_tmp_mask=0;
    if (tmpf=tmpi->ic_data) {
      if (Bt(&tmpf->flags,Ff_INTERNAL))
	clobbered_stk_tmp_mask=0;
      else {
	clobbered_stk_tmp_mask&=tmpf->clobbered_reg_mask;
	clobbered_reg_mask|=tmpf->clobbered_reg_mask;
      }
    }
    tmpi->ic_data=clobbered_stk_tmp_mask;
    PrsPush(ps,stk_ptr);
    PrsPush(ps,clobbered_stk_tmp_mask);
  } else if (code==IC_CALL_END) {
    tmpi->ic_data=PrsPop(ps);
    stk_ptr=PrsPop(ps);
  } else if (code==IC_CALL_END2) {
    ps->ptr--;
    stk_ptr=PrsPop(ps);
  }
  if (intermediate_code_table[code].arg_cnt==IS_V_ARG)
    stk_ptr-=tmpi->ic_data>>3;
  if (tmpi->arg2.type&MDF_STK) {
    stk_ptr--;
    if (stk_ptr<reg_stk_size) {
      tmpi->arg2.type=MDF_REG+tmpi->arg2.type.raw_type;
      tmpi->arg2.reg=Bsf(REGG_STK_TMP);
      tmpi->arg2.disp=0;
    }
  }
  if (tmpi->arg1.type&MDF_STK) {
    stk_ptr--;
    if (stk_ptr<reg_stk_size) {
      tmpi->arg1.type=MDF_REG+tmpi->arg1.type.raw_type;
      tmpi->arg1.reg=Bsf(REGG_STK_TMP);
      tmpi->arg1.disp=0;
    }
  }
  if (tmpi->res.type&MDF_STK && !(tmpi->ic_flags & ICF_PUSH_RES)) {
    stk_ptr++;
    if (stk_ptr<=reg_stk_size) {
      tmpi->res.type=MDF_REG+tmpi->res.type.raw_type;
      tmpi->res.reg=Bsf(REGG_STK_TMP);
      clobbered_reg_mask|=REGG_STK_TMP;
      tmpi->res.disp=0;
    }
  }
  while (OptIC6(tmpi));
  if (tmpi->res.type.raw_type!=RT_F64 && !(tmpi->ic_flags&ICF_USE_F64))
    tmpi->ic_flags|=ICF_USE_INT;
  *_stk_ptr=stk_ptr;
  *_clobbered_reg_mask=clobbered_reg_mask;
}

U0 OptPass6(CCmpCtrl *cc)
{
  CIntermediateCode *tmpi,*tmpi_next,*tmpil1,*tmpil2,*old_tmpil2;
  I64 stk_ptr=0,reg_stk_size,clobbered_reg_mask=REGG_CLOBBERED;
  CPrsStk *ps=cc->ps;
  ps->ptr=0;
  ps->ptr2=0;
  if (Bt(&cc->opts,OPTf_NO_REG_VAR) || cc->flags&CCF_NO_REG_OPT)
    reg_stk_size=0;
  else
    reg_stk_size=1;
#assert REGG_STK_TMP==1<<9
  tmpi=cc->coc.coc_head.next;
  old_tmpil2=NULL;
  tmpil1=tmpil2=&cmp.ic_nop;
  while (tmpi->ic_code) {
    if (tmpi->ic_code>IC_NOP2) {
      if (tmpil1->ic_code>IC_NOP2)
	tmpil2=tmpil1;
      tmpil1=tmpi;
      if (tmpi->arg2.type&MDF_STK) {
	if (tmpil2->res.type&MDF_STK &&
	      !(tmpil2->ic_flags&ICF_PUSH_RES)) {
	  if (tmpi->ic_code==IC_ASSIGN && tmpi->ic_flags&ICF_BY_VAL &&
		tmpi->ic_flags&ICF_RES_NOT_USED &&
		tmpil2->ic_code!=IC_CALL_END &&
		tmpil2->ic_code!=IC_CALL_END2 &&
		tmpil2->ic_code!=IC_SET_RAX &&
		!(tmpi->ic_flags&(ICF_ARG2_TO_F64|ICF_ARG2_TO_INT)) &&
		!(tmpil2->ic_flags&(ICF_RES_TO_F64|ICF_RES_TO_INT))) {
	    tmpil2->res.type =tmpi->arg1.type&MDG_MASK+
		  tmpi->arg1_type_pointed_to;
	    tmpil2->res.reg  =tmpi->arg1.reg;
	    tmpil2->res.disp =tmpi->arg1.disp;
	    tmpil2->ic_flags=tmpil2->ic_flags
		  &~(ICF_RES_NOT_USED|ICF_RES_WAS_STK)
	    |tmpi->ic_flags&~(ICF_BY_VAL|ICF_ARG1_WAS_STK|ICF_ARG2_WAS_STK);
	    old_tmpil2=NULL;
	    OptSetNOP1(tmpi);
	  } else {
	    tmpi->arg2.type=MDF_REG+tmpi->arg2.type.raw_type;
	    tmpi->arg2.reg=REG_RAX;
	    tmpi->arg2.disp=0;
	    tmpil2->res.type=MDF_REG+tmpil2->res.type.raw_type;
	    tmpil2->res.reg=REG_RAX;
	    tmpil2->res.disp=0;
	  }
	}
      } else if (tmpi->arg1.type&MDF_STK && tmpil2->res.type&MDF_STK &&
	    !(tmpil2->ic_flags&ICF_PUSH_RES)) {
	tmpi->arg1.type=MDF_REG+tmpi->arg1.type.raw_type;
	tmpi->arg1.disp=0;
	tmpil2->res.type=MDF_REG+tmpil2->res.type.raw_type;
	tmpil2->res.disp=0;
	if (intermediate_code_table[tmpi->ic_code].arg_cnt==IS_2_ARG) {
	  tmpi->arg1.reg=REG_R8;
	  tmpil2->res.reg=REG_R8;
	} else {
	  tmpi->arg1.reg=REG_RAX;
	  tmpil2->res.reg=REG_RAX;
	}
      }
      if (tmpi->ic_flags & ICF_PUSH_RES)
	tmpi->res.type==MDF_STK+tmpi->res.type.raw_type;
      if (old_tmpil2!=tmpil2) {
	if (tmpil2->ic_code>IC_NOP2)
	  OptPass6Lag(cc,ps,tmpil2,&stk_ptr,reg_stk_size,&clobbered_reg_mask);
	old_tmpil2=tmpil2;
      }
    }
    tmpi_next=tmpi->next;
    if (tmpi->ic_code<=IC_NOP2)
      OptFree(tmpi);
    tmpi=tmpi_next;
  }
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",cc->pass,ps->ptr;
    LexExcept(cc,"Compiler Optimization Error at ");
  }
  if (cc->htc.fun) {
    cc->htc.fun->used_reg_mask&=~REGG_STK_TMP;
    cc->htc.fun->used_reg_mask|=clobbered_reg_mask;
    cc->htc.fun->clobbered_reg_mask=clobbered_reg_mask;
    if (Bt(&cc->flags,CCf_PASS_TRACE_PRESENT) &&
	  Bt(&cc->saved_pass_trace,6)) {
      "Used      Reg Mask:%04X\n",cc->htc.fun->used_reg_mask;
      "Clobbered Reg Mask:%04X\n",clobbered_reg_mask;
    }
  }
}
