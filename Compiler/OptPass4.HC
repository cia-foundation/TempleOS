Bool OptIC4(CIntermediateCode *tmpi)
{
  I64 i;
  CIntermediateCode *tmpil1,*tmpil2;

  if (tmpi->ic_code<IC_IMM_I64) return FALSE;

  tmpil1=tmpi;
  if (!(tmpil2=OptLag1(tmpil1)))
    return FALSE;

  if (tmpil2->res.type&MDF_STK && !(tmpil2->ic_flags&ICF_PUSH_RES)) {
    if (tmpil1->ic_code==IC_ADD_CONST && tmpil1->arg1.type&MDF_STK) {
      if ((tmpil2->ic_code==IC_REG || tmpil2->ic_code==IC_MOV) &&
	    tmpil2->arg1.type&MDF_REG) {
	i=tmpil1->ic_data;
	if (I32_MIN<=i<=I32_MAX &&
	      !Bt(&cmp.non_ptr_vars_mask,tmpil2->arg1.reg)) {
	  tmpil1->ic_flags|=tmpil2->ic_flags&ICG_NO_CVT_MASK;
	  tmpil1->ic_code=IC_LEA;
	  tmpil1->arg1.type=MDF_DISP+tmpil1->arg1.type.raw_type;
	  tmpil1->arg1.reg=tmpil2->arg1.reg;
	  tmpil1->arg1.disp=i;
	  OptFree(tmpil2);
	  return TRUE;
	}
      } else if (tmpil2->ic_code==IC_SHL_CONST && tmpil2->arg1.type&MDF_REG) {
	i=tmpil1->ic_data;
	if (I32_MIN<=i<=I32_MAX && tmpil2->arg1.reg!=REG_RSP &&
	      1<=tmpil2->ic_data<=3) {
	  tmpil1->ic_flags|=tmpil2->ic_flags&ICG_NO_CVT_MASK;
	  tmpil1->ic_code=IC_LEA;
	  tmpil1->arg1.type=MDF_SIB+tmpil1->arg1.type.raw_type;
	  tmpil1->arg1.reg=tmpil2->arg1.reg<<8+REG_NONE;
	  if (tmpil2->ic_data==1)
	    tmpil1->arg1.reg|=0x4000;
	  else if (tmpil2->ic_data==2)
	    tmpil1->arg1.reg|=0x8000;
	  else
	    tmpil1->arg1.reg|=0xC000;
	  tmpil1->arg1.disp=i;
	  OptFree(tmpil2);
	  return TRUE;
	}
      }
    }
    if (tmpil2->ic_code==IC_MOV || tmpil2->ic_code==IC_REG) {
      if (tmpil1->arg2.type&MDF_STK) {
	if (tmpil2->ic_flags & ICF_RES_TO_INT) {
	  if (tmpil2->arg1.type&MDF_IMM)
	    tmpil2->arg1.disp=tmpil2->arg1.disp(F64);
	  else
	    tmpil1->ic_flags|=ICF_ARG2_TO_INT;
	} else if (tmpil2->ic_flags&ICF_RES_TO_F64) {
	  if (tmpil2->arg1.type&MDF_IMM)
	    tmpil2->arg1.disp(F64)=tmpil2->arg1.disp;
	  else
	    tmpil1->ic_flags|=ICF_ARG2_TO_F64;
	}
	tmpil1->arg2.type=tmpil2->arg1.type&MDG_MASK+
	      MinI64(tmpil1->arg2.type.raw_type,
	      MinI64(tmpil2->res.type.raw_type,tmpil2->arg1.type.raw_type));
	tmpil1->arg2.reg=tmpil2->arg1.reg;
	tmpil1->arg2.disp=tmpil2->arg1.disp;
	tmpil1->ic_flags|=tmpil2->ic_flags&ICG_NO_CVT_MASK;
	OptSetNOP2(tmpil2);
	return TRUE;
      }
      if (tmpil1->arg1.type&MDF_STK) {
	if (tmpil2->ic_flags & ICF_RES_TO_INT) {
	  if (tmpil2->arg1.type&MDF_IMM)
	    tmpil2->arg1.disp=tmpil2->arg1.disp(F64);
	  else
	    tmpil1->ic_flags|=ICF_ARG1_TO_INT;
	} else if (tmpil2->ic_flags&ICF_RES_TO_F64) {
	  if (tmpil2->arg1.type&MDF_IMM) {
	    if (tmpil2->arg1.type&RTF_UNSIGNED)
	      tmpil2->arg1.disp(F64)=tmpil2->arg1.disp(U64);
	    else
	      tmpil2->arg1.disp(F64)=tmpil2->arg1.disp(I64);
	  } else
	    tmpil1->ic_flags|=ICF_ARG1_TO_F64;
	}
	tmpil1->arg1.type=tmpil2->arg1.type&MDG_MASK+
	      MinI64(tmpil1->arg1.type.raw_type,
	      MinI64(tmpil2->res.type.raw_type,tmpil2->arg1.type.raw_type));
	CmpMinTypePointed(tmpil1,tmpil2->arg1_type_pointed_to);
	tmpil1->arg1.reg=tmpil2->arg1.reg;
	tmpil1->arg1.disp=tmpil2->arg1.disp;
	tmpil1->ic_flags|=tmpil2->ic_flags&ICG_NO_CVT_MASK;
	OptSetNOP2(tmpil2);
	return TRUE;
      }
    }
    if (tmpil1->ic_code==IC_DEREF) {
      if (tmpil2->ic_code==IC_ADD_CONST && tmpil2->arg1.type&MDF_REG &&
	    tmpil1->arg1.type&MDF_STK) {
	i=tmpil2->ic_data;
	if (I32_MIN<=i<=I32_MAX &&
	      !Bt(&cmp.non_ptr_vars_mask,tmpil2->arg1.reg)) {
	  tmpil1->ic_flags|=tmpil2->ic_flags;
	  tmpil1->ic_code=IC_MOV;
	  tmpil1->arg1.type=MDF_DISP+tmpil1->arg1_type_pointed_to;
	  tmpil1->arg1.reg=tmpil2->arg1.reg;
	  tmpil1->arg1.disp=i;
	  OptSetNOP2(tmpil2,-1);
	  return TRUE;
	}
      }
      if (tmpil2->ic_code==IC_LEA && tmpil1->arg1.type&MDF_STK) {
	tmpil1->ic_flags|=tmpil2->ic_flags;
	tmpil1->ic_code=IC_MOV;
	tmpil1->arg1.type=tmpil2->arg1.type&MDG_MASK+
	      tmpil1->arg1_type_pointed_to;
	tmpil1->arg1.reg=tmpil2->arg1.reg;
	tmpil1->arg1.disp=tmpil2->arg1.disp;
	OptFree(tmpil2);
	return TRUE;
      }
    }
  }
  if (tmpil1->ic_code==IC_DEREF) {
    if (tmpil1->arg1.type&MDF_REG) {
      tmpil1->arg1.type=MDF_DISP+tmpil1->arg1_type_pointed_to;
      tmpil1->arg1.disp=0;
      tmpil1->ic_code=IC_MOV;
      return TRUE;
    }
  }
  return FALSE;
}

U0 OptPass4(CCmpCtrl *cc,COptReg *reg_offsets,I64 *_type)
{
  CHashClass *tmpc,*tmpc1,*tmpc2;
  CIntermediateCode *tmpi,*tmpi1,*tmpi2,*tmpil1,*tmpil2,*tmpil3,
	*tmpi_next;
  I64 code,i;
  Bool dead_code=FALSE;
  CCodeMisc *lb;
  CPrsStk *ps=cc->ps;
  ps->ptr=0;
  ps->ptr2=0;

  if (_type)
    *_type=RT_I64;

  tmpi=cc->coc.coc_head.next;
  while (code=tmpi->ic_code) {
    tmpi_next=tmpi->next;
    if (dead_code&&code!=IC_LABEL) {
      if (code==IC_JMP||code==IC_SUB_CALL) {
	lb=OptLabelFwd(tmpi->ic_data);
	if (lb->use_cnt>0)
	  lb->use_cnt--;
      }
      tmpi=OptFree(tmpi);
    } else {
      tmpc=tmpi->ic_class;
      tmpi1=tmpi2=&cmp.ic_nop;
      if (tmpil1=OptLag2(tmpi)) {
	if (tmpil2=OptLag2(tmpil1)) {
	  if (!(tmpil3=OptLag2(tmpil2)))
	    tmpil3=&cmp.ic_nop;
	} else
	  tmpil2=tmpil3=&cmp.ic_nop;
      } else
	tmpil1=tmpil2=tmpil3=&cmp.ic_nop;
      switch [intermediate_code_table[code].arg_cnt] {
	case IS_V_ARG:
	  ps->ptr-=tmpi->ic_data>>3;
	  break;
	case IS_2_ARG:
	  tmpi2=PrsPop(ps);
	  tmpc2=tmpi2->ic_class;
	case IS_1_ARG:
	  tmpi1=PrsPop(ps);
	  tmpc1=tmpi1->ic_class;
	  break;
	case IS_0_ARG: //nobound switch
	  break;
      }
      switch [code] {
	case IC_IMM_I64:
	case IC_TYPE:
	  tmpi->arg1.type=MDF_IMM+RT_I64;
	  tmpi->arg1.disp=tmpi->ic_data;
	  tmpi->ic_code=IC_MOV;
	  break;
	case IC_IMM_F64:
	  tmpi->arg1.type=MDF_IMM+RT_I64;
	  tmpi->arg1.disp=tmpi->ic_data;
	  tmpi->arg1_type_pointed_to=RT_F64;
	  tmpi->ic_code=IC_MOV;
	  break;
	case IC_MOV:
	  if (tmpi->arg1.type&MDF_DISP && tmpi->arg1.reg==REG_RBP) {
	    i=CmpOffset2Reg(tmpi->arg1.disp,reg_offsets);
	    if (i>=0) {
	      tmpi->arg1.type=MDF_REG+tmpi->arg1.type.raw_type;
	      tmpi->arg1.reg=i;
	      tmpi->arg1.disp=0;
	    }
	  }
	  break;
	case IC_DEREF:
	  if (tmpi1->ic_code==IC_LEA) {
	    if (tmpi1->arg1.type&MDF_DISP && tmpi1->arg1.reg==REG_RBP) {
	      i=CmpOffset2Reg(tmpi1->arg1.disp,reg_offsets);
	      if (i>=0) {
		tmpi->ic_flags|=tmpi1->ic_flags;
		tmpi->ic_code=IC_REG;
		tmpi->arg1.type=MDF_REG+tmpi->arg1.type.raw_type;
		tmpi->arg1.reg=i;
		tmpi->arg1.disp=0;
		OptFree(tmpi1);
	      }
	    }
	  } else if ((tmpi1->ic_code==IC_ABS_ADDR ||
		tmpi1->ic_code==IC_MOV &&
		tmpi1->arg1.type==MDF_IMM+RT_I64 &&
		0<=tmpi1->arg1.disp<=I32_MAX)&& !(tmpi1->ic_flags&ICF_NO_RIP)) {
	    if (tmpi1->ic_code==IC_ABS_ADDR)
	      tmpi->arg1.disp=tmpi1->ic_data;
	    else
	      tmpi->arg1.disp=tmpi1->arg1.disp;
	    tmpi->ic_flags|=tmpi1->ic_flags;
	    tmpi->ic_code=IC_MOV;
	    tmpi->arg1.type=MDF_RIP_DISP32+tmpi->arg1_type_pointed_to;
	    tmpi->arg1.reg=REG_RIP;
	    OptFree(tmpi1);
	  }
	  break;
	case IC_BR_MM_ZERO:
	case IC_BR_MM_NOT_ZERO:
//(branch ++ to zero is unlikely)
	case IC_DEREF_PP:
	case IC_DEREF_MM:
	case IC__PP:
	case IC__MM:
	case IC_PP_:
	case IC_MM_:
	  if (tmpi1->ic_code==IC_LEA) {
	    if (tmpi1->arg1.type&MDF_DISP && tmpi1->arg1.reg==REG_RBP) {
	      i=CmpOffset2Reg(tmpi1->arg1.disp,reg_offsets);
	      if (i>=0) {
		tmpi->ic_flags|=tmpi1->ic_flags;
		tmpi->arg1.type=MDF_REG+tmpi->arg1.type.raw_type;
		tmpi->arg1.reg=i;
		tmpi->arg1.disp=0;
		tmpi->ic_flags|=ICF_BY_VAL;
		OptSetNOP2(tmpi1);
	      } else
		goto p4_lea_gone;
	    } else {
p4_lea_gone:
	      tmpi->ic_flags|=tmpi1->ic_flags;
	      tmpi->arg1.type=tmpi1->arg1.type;
	      tmpi->arg1.reg=tmpi1->arg1.reg;
	      tmpi->arg1.disp=tmpi1->arg1.disp;
	      tmpi->ic_flags|=ICF_BY_VAL;
	      OptSetNOP2(tmpi1);
	    }
	  } else if ((tmpi1->ic_code==IC_ABS_ADDR || tmpi1->ic_code==IC_MOV &&
		tmpi1->arg1.type==MDF_IMM+RT_I64 &&
		0<=tmpi1->arg1.disp<=I32_MAX)&& !(tmpi1->ic_flags&ICF_NO_RIP)) {
	    tmpi->ic_flags|=tmpi1->ic_flags;
	    if (tmpi1->ic_code==IC_ABS_ADDR)
	      tmpi->arg1.disp=tmpi1->ic_data;
	    else
	      tmpi->arg1.disp=tmpi1->arg1.disp;
	    tmpi->arg1.type=MDF_RIP_DISP32+tmpi->arg1_type_pointed_to;
	    tmpi->arg1.reg=REG_RIP;
	    tmpi->ic_flags|=ICF_BY_VAL;
	    OptFree(tmpi1);
	  }
	  break;
	case IC_ADD:
	  if (tmpi1->ic_code==IC_MOV && tmpi1->arg1.type==MDF_REG+RT_I64 ||
		tmpi1->ic_code==IC_REG) {
	    if (tmpi2->ic_code==IC_MOV && tmpi2->arg1.type==MDF_REG+RT_I64 ||
		  tmpi2->ic_code==IC_REG) {
	      if (tmpi2->arg1.reg!=REG_RSP) {
		tmpi->arg1.disp=0;
		tmpi->arg1.reg=tmpi1->arg1.reg+tmpi2->arg1.reg<<8;
		goto p4_sib;
	      } else if (tmpi1->arg1.reg!=REG_RSP) {
		tmpi->arg1.disp=0;
		tmpi->arg1.reg=tmpi2->arg1.reg+tmpi1->arg1.reg<<8;
p4_sib:
		tmpi->ic_flags|=(tmpi1->ic_flags|tmpi2->ic_flags)
		&ICG_NO_CVT_MASK;
		OptSetNOP2(tmpi1);
		OptFree(tmpi2);

		tmpi->ic_code=IC_LEA;
		tmpi->arg1.type=MDF_SIB+RT_I64;
		tmpi->arg1_type_pointed_to=RT_I64;

		tmpi->arg2.type=MDF_NULL+tmpi->arg2.type.raw_type;
	      }
	    } else if (tmpi2->ic_code==IC_SHL_CONST &&
		  tmpi2->arg1.type==MDF_REG+RT_I64 && tmpi2->ic_data<=3) {
	      if (tmpi2->arg1.reg!=REG_RSP) {
		tmpi->arg1.disp=0;
		tmpi->arg1.reg=tmpi1->arg1.reg+tmpi2->arg1.reg<<8;
		if (tmpi2->ic_data==1)
		  tmpi->arg1.reg|=0x4000;
		else if (tmpi2->ic_data==2)
		  tmpi->arg1.reg|=0x8000;
		else
		  tmpi->arg1.reg|=0xC000;
		goto p4_sib;
	      }
	    }
	  } else if (tmpi1->ic_code==IC_LEA &&
		tmpi1->arg1.type&MDF_DISP) {
	    if (tmpi1->arg1.reg==REG_RBP &&
		  CmpOffset2Reg(tmpi1->arg1.disp,reg_offsets)>=0)
	      break;
	    if (tmpi2->ic_code==IC_MOV && tmpi2->arg1.type==MDF_REG+RT_I64 ||
		  tmpi2->ic_code==IC_REG) {
	      if (tmpi2->arg1.reg!=REG_RSP) {
		tmpi->arg1.disp=tmpi1->arg1.disp;
		tmpi->arg1.reg=tmpi1->arg1.reg+tmpi2->arg1.reg<<8;
		goto p4_sib;
	      } else if (tmpi1->arg1.reg!=REG_RSP) {
		tmpi->arg1.disp=tmpi1->arg1.disp;
		tmpi->arg1.reg=tmpi2->arg1.reg+tmpi1->arg1.reg<<8;
		goto p4_sib;
	      }
	    } else if (tmpi2->ic_code==IC_SHL_CONST &&
		  tmpi2->arg1.type==MDF_REG+RT_I64 && tmpi2->ic_data<=3) {
	      if (tmpi2->arg1.reg!=REG_RSP) {
		tmpi->arg1.disp=tmpi1->arg1.disp;
		tmpi->arg1.reg=tmpi1->arg1.reg+tmpi2->arg1.reg<<8;
		if (tmpi2->ic_data==1)
		  tmpi->arg1.reg|=0x4000;
		else if (tmpi2->ic_data==2)
		  tmpi->arg1.reg|=0x8000;
		else
		  tmpi->arg1.reg|=0xC000;
		goto p4_sib;
	      }
	    }
	  }
	  break;
	case IC_ASSIGN_PP:
	case IC_ASSIGN_MM:
//this val was stashed during pass012 for pointer arithmetic
	  tmpi->ic_class2=tmpi->t.class2; //See $LK,"ic_class2",A="FF:::/Compiler/BackB.HC,ic_class2"$
	case IC_ASSIGN:
	case IC_SHL_EQU:
	case IC_SHR_EQU:
	case IC_MUL_EQU:
	case IC_DIV_EQU:
	case IC_MOD_EQU:
	case IC_AND_EQU:
	case IC_OR_EQU:
	case IC_XOR_EQU:
	case IC_ADD_EQU:
	case IC_SUB_EQU:
	  if (tmpi1->ic_code==IC_LEA) {
	    if (tmpi1->arg1.type&(MDF_DISP|MDF_SIB)) {
	      tmpi2=tmpi->next;
	      if (tmpi1->arg1.type&MDF_DISP && tmpi1->arg1.reg==REG_RBP) {
		i=CmpOffset2Reg(tmpi1->arg1.disp,reg_offsets);
		if (i>=0) {
		  tmpi->ic_flags|=tmpi1->ic_flags;
		  tmpi->arg1.type=MDF_REG+tmpi->arg1.type.raw_type;
		  tmpi->arg1.reg=i;
		  tmpi->arg1.disp=0;
		  OptSetNOP2(tmpi1);
		} else {
		  tmpi->ic_flags|=tmpi1->ic_flags;
		  tmpi->arg1.type=MDF_DISP+tmpi->arg1.type.raw_type;
		  tmpi->arg1.reg=REG_RBP;
		  tmpi->arg1.disp=tmpi1->arg1.disp;
		  OptSetNOP2(tmpi1);
		}
	      } else {
		tmpi->ic_flags|=tmpi1->ic_flags;
		tmpi->arg1.type=tmpi1->arg1.type&MDG_MASK+
		      tmpi->arg1.type.raw_type;
		tmpi->arg1.reg=tmpi1->arg1.reg;
		tmpi->arg1.disp=tmpi1->arg1.disp;
		OptSetNOP2(tmpi1);
	      }
	      if (tmpi->res.type&MDF_STK && tmpi2->arg2.type&MDF_STK &&
		    code!=IC_ASSIGN_PP && code!=IC_ASSIGN_MM) {
		tmpi->res.type=tmpi->arg1.type;
		tmpi->res.reg=tmpi->arg1.reg;
		tmpi->res.disp=tmpi->arg1.disp;
		tmpi2->arg2.type=tmpi->arg1.type;
		tmpi2->arg2.reg=tmpi->arg1.reg;
		tmpi2->arg2.disp=tmpi->arg1.disp;
		CmpMinTypePointed(tmpi2,tmpi->arg1_type_pointed_to);
	      }
	      tmpi->ic_flags|=ICF_BY_VAL;
	    }
	  } else if ((tmpi1->ic_code==IC_ABS_ADDR ||
		tmpi1->ic_code==IC_MOV && tmpi1->arg1.type==MDF_IMM+RT_I64 &&
		0<=tmpi1->arg1.disp<=I32_MAX)&& !(tmpi1->ic_flags&ICF_NO_RIP)) {
	    tmpi->ic_flags|=tmpi1->ic_flags;
	    if (tmpi1->ic_code==IC_ABS_ADDR)
	      tmpi->arg1.disp=tmpi1->ic_data;
	    else
	      tmpi->arg1.disp=tmpi1->arg1.disp;
	    tmpi->arg1.type=MDF_RIP_DISP32+tmpi->arg1.type.raw_type;
	    tmpi->arg1.reg=REG_RIP;
	    tmpi->ic_flags|=ICF_BY_VAL;
	    OptSetNOP2(tmpi1);
	  }
	  break;
	case IC_RETURN_VAL:
	case IC_RETURN_VAL2:
	  if (!tmpi->ic_class) {
	    if (_type) {
	      tmpil1=tmpi;
	      while (tmpil1=OptLag1(tmpil1))
		if (tmpil1->ic_class) {
		  if (tmpil1->ic_flags & ICF_RES_TO_F64)
		    *_type=RT_F64;
		  else if (tmpil1->ic_flags & ICF_RES_TO_INT)
		    *_type=RT_I64;
		  else
		    *_type=tmpil1->ic_class->raw_type;
		  break;
		}
	    }
	    tmpi->ic_class=cmp.internal_types[RT_I64];
	  } else if (_type)
	    *_type=tmpi->ic_class->raw_type;
	  break;
	case IC_NOP1:
	  tmpi=OptFree(tmpi);
	  break;
	case IC_BR_BT:
	case IC_BR_BTS:
	case IC_BR_BTR:
	case IC_BR_BTC:
	case IC_BR_NOT_BT:
	case IC_BR_NOT_BTS:
	case IC_BR_NOT_BTR:
	case IC_BR_NOT_BTC:
	case IC_BT:
	case IC_BTS:
	case IC_BTR:
	case IC_BTC:
	case IC_LBTS:
	case IC_LBTR:
	case IC_LBTC:
	  if (!(tmpi->ic_flags&ICF_BY_VAL)) {
	    if (tmpi2->ic_code==IC_ADDR) {
	      if (tmpi2->arg1.type&MDF_STK &&
		    tmpi2->res.type&MDF_STK) {
		if (tmpil2=OptLag1(tmpi2)) {
		  if (tmpil2->ic_code==IC_LEA) {
		    if (tmpil2->arg1.type&(MDF_IMM|MDG_REG_DISP_SIB_RIP)) {
		      if (tmpi2) {
			tmpi->ic_flags|=tmpi2->ic_flags;
			OptFree(tmpi2);
		      }
		      tmpi->ic_flags|=tmpil2->ic_flags|ICF_BY_VAL;
		      tmpi->arg2.type=tmpil2->arg1.type;
		      tmpi->arg2.reg =tmpil2->arg1.reg;
		      tmpi->arg2.disp=tmpil2->arg1.disp;
		      OptFree(tmpil2);
		    }
		    break;
		  } else if (tmpil2->ic_code!=IC_ABS_ADDR &&
			!(tmpil2->ic_code==IC_MOV &&
			tmpil2->arg1.type==MDF_IMM+RT_I64 &&
			0<=tmpil2->arg1.disp<=I32_MAX) ||
			tmpil2->ic_flags&ICF_NO_RIP)
		    tmpil2=NULL;
		  else {
		    if (tmpil2->ic_code==IC_ABS_ADDR)
		      tmpi->arg2.disp=tmpil2->ic_data;
		    else
		      tmpi->arg2.disp=tmpil2->arg1.disp;
		  }
		}
	      } else {
		if (tmpi2->arg1.type==MDF_IMM+RT_I64 &&
		      0<=tmpi2->arg1.disp<=I32_MAX &&
		      !(tmpi2->ic_flags&ICF_NO_RIP)) {
		  tmpil2=tmpi2;
		  tmpi2=NULL;
		  tmpi->arg2.disp=tmpil2->arg1.disp;
		} else
		  tmpil2=NULL;
	      }
	      if (tmpil2) {
		if (tmpi2) {
		  tmpi->ic_flags|=tmpi2->ic_flags;
		  OptFree(tmpi2);
		}
		tmpi->ic_flags|=tmpil2->ic_flags|ICF_BY_VAL;
		tmpi->arg2.type=MDF_RIP_DISP32+tmpi->arg2.type.raw_type;
		tmpi->arg2.reg=REG_RIP;
		OptFree(tmpil2);
	      }
	    } else if (tmpi2->ic_code==IC_MOV && tmpi2->res.type&MDF_STK &&
		  tmpi2->arg1.type==MDF_IMM+RT_I64 &&
		  0<=tmpi2->arg1.disp<=I32_MAX &&
		  !(tmpi2->ic_flags&ICF_NO_RIP)) {
	      tmpi->arg2.disp=tmpi2->arg1.disp;
	      tmpi->ic_flags|=tmpi2->ic_flags|ICF_BY_VAL;
	      tmpi->arg2.type=MDF_RIP_DISP32+tmpi->arg2.type.raw_type;
	      tmpi->arg2.reg=REG_RIP;
	      OptFree(tmpi2);
	    }
	  }
	  break;
	case IC_BR_EQU_EQU ...IC_BR_LESS_EQU:
	case IC_BR_EQU_EQU2...IC_BR_LESS_EQU2:
	case IC_BR_CARRY:
	case IC_BR_NOT_CARRY:
	case IC_BR_ZERO:
	case IC_BR_NOT_ZERO:
	  lb=tmpi->ic_data;
	  if (tmpi->ic_flags&ICF_PUSH_CMP) {
	    lb->flags|=CMF_POP_CMP;
	    lb->fwd=NULL;
	  }
	  break;
	case IC_LABEL:
	  lb=tmpi->ic_data;
	  if (lb->use_cnt)
	    dead_code=FALSE;
	  break;
	case IC_JMP:
	case IC_RET:
	  dead_code=TRUE;
	  break;
	case IC_NOP2:
	  ps->ptr+=tmpi->ic_data;
	  break;
	case IC_CALL_END:
	case IC_END_EXP:
	  if (!(tmpil1->ic_flags&ICF_PUSH_RES)) {
	    if (tmpi->ic_flags&ICF_RES_NOT_USED) {
	      tmpil1->ic_flags|=ICF_RES_NOT_USED;
	      tmpil1->res.type=MDF_NULL+tmpil1->res.type.raw_type;
	    } else if (tmpi->arg1.type&MDF_STK &&
		  tmpil1->res.type&MDF_STK) {
	      tmpi->arg1.type=MDF_REG+tmpi->arg1.type.raw_type;
	      tmpi->arg1.disp=0;
	      tmpil1->res.type=MDF_REG+tmpil1->res.type.raw_type;
	      tmpil1->res.disp=0;
	      if (intermediate_code_table[tmpi->ic_code].arg_cnt==IS_2_ARG) {
		tmpi->arg1.reg=REG_R8;
		tmpil1->res.reg=REG_R8;
	      } else {
		tmpi->arg1.reg=REG_RAX;
		tmpil1->res.reg=REG_RAX;
	      }
	    }
	  }
	  break;
	case IC_STR_CONST:
	case IC_FS:
	case IC_GS:
	case IC_MOV_FS:
	case IC_MOV_GS:
	case IC_RIP:
	case IC_RBP:
	case IC_REG:
	case IC_COM:
	case IC_HOLYC_TYPECAST:
	case IC_NOT:
	case IC_UNARY_MINUS:
	case IC_PUSH_CMP:
	case IC_ADD_CONST:
	case IC_SUB_CONST:
	case IC_ENTER:
	case IC_ADD_RSP:
	case IC_ADD_RSP1:
	case IC_CALL:
	case IC_CALL_INDIRECT:
	case IC_CALL_INDIRECT2:
	case IC_CALL_EXTERN:
	case IC_CALL_IMPORT:
	case IC_PUSH:
	case IC_POP:
	case IC_INVLPG:
	case IC_CLFLUSH:
	case IC_GET_RFLAGS:
	case IC_CARRY:
	case IC_RDTSC:
	case IC_SET_RFLAGS:
	case IC_GET_RBP:
	case IC_SET_RBP:
	case IC_GET_RSP:
	case IC_GET_RAX:
	case IC_SET_RSP:
	case IC_SET_RAX:
	case IC_SHL_CONST:
	case IC_LEA:
	case IC_SHR_CONST:
	case IC_POWER:
	case IC_SHL:
	case IC_SHR:
	case IC_MUL:
	case IC_DIV:
	case IC_MOD:
	case IC_AND:
	case IC_OR:
	case IC_XOR:
	case IC_SUB:
	case IC_EQU_EQU...IC_LESS_EQU:
	case IC_AND_AND:
	case IC_OR_OR:
	case IC_XOR_XOR:
	case IC_GET_LABEL:
	case IC_ABS_ADDR:
	case IC_HEAP_GLBL:
	case IC_ADDR_IMPORT:
	case IC_BSF:
	case IC_BSR:
	case IC_SIGN_I64:
	case IC_TOUPPER:
	case IC_TO_I64:
	case IC_TO_F64:
	case IC_TO_BOOL:
	case IC_SQR:
	case IC_ABS:
	case IC_SQRT:
	case IC_SIN:
	case IC_COS:
	case IC_TAN:
	case IC_ATAN:
	case IC_ABS_I64:
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
	case IC_SWAP_I64:
	case IC_QUE_INIT:
	case IC_QUE_INS:
	case IC_QUE_INS_REV:
	case IC_QUE_REM:
	case IC_IN_U32:
	case IC_IN_U16:
	case IC_IN_U8:
	case IC_STRLEN:
	case IC_OUT_U32:
	case IC_OUT_U16:
	case IC_OUT_U8:
	case IC_NOBOUND_SWITCH:
	case IC_SWITCH:
	case IC_END:
	case IC_ADDR:
	case IC_CALL_START:
	case IC_LEAVE:
	case IC_PUSH_REGS:
	case IC_POP_REGS:
	case IC_ASM:
	case IC_BR_AND_NOT_ZERO:
	case IC_BR_AND_ZERO:
	case IC_SUB_CALL:
	case IC_CALL_END2:
	  break;
	default:
	  "Pass:%d Missing IC hndlr\n",cc->pass;
	  ICPut(cc,tmpi);
	  LexExcept(cc,"Compiler Optimization Error at ");
      }
      if (tmpi) {
	while (OptIC4(tmpi));
	code=tmpi->ic_code;
	if (intermediate_code_table[code].res_cnt)
	  PrsPush(ps,tmpi);
      }
    }
    tmpi=tmpi_next;
  }
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",cc->pass,ps->ptr;
    LexExcept(cc,"Compiler Optimization Error at ");
  }
}
