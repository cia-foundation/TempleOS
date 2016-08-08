Bool OptIC4(CIntermediateCode *tempi)
{
  I64 i;
  CIntermediateCode *tempil1,*tempil2;

  if (tempi->ic_code<IC_IMM_I64) return FALSE;

  tempil1=tempi;
  if (!(tempil2=OptLag1(tempil1)))
    return FALSE;

  if (tempil2->r.type&MDF_STK && !(tempil2->ic_flags&ICF_PUSH_RES)) {
    if (tempil1->ic_code==IC_ADD_CONST && tempil1->a1.type&MDF_STK) {
      if ((tempil2->ic_code==IC_REG || tempil2->ic_code==IC_MOV) &&
	    tempil2->a1.type&MDF_REG) {
	i=tempil1->ic_data;
	if (MIN_I32<=i<=MAX_I32 &&
	      !Bt(&cmp.non_ptr_vars_mask,tempil2->a1.reg)) {
	  tempil1->ic_flags|=tempil2->ic_flags&ICG_NO_CVT_MASK;
	  tempil1->ic_code=IC_LEA;
	  tempil1->a1.type=MDF_DISP+tempil1->a1.type.raw_type;
	  tempil1->a1.reg=tempil2->a1.reg;
	  tempil1->a1.disp=i;
	  OptFree(tempil2);
	  return TRUE;
	}
      } else if (tempil2->ic_code==IC_SHL_CONST && tempil2->a1.type&MDF_REG) {
	i=tempil1->ic_data;
	if (MIN_I32<=i<=MAX_I32 && tempil2->a1.reg!=REG_RSP &&
	      1<=tempil2->ic_data<=3) {
	  tempil1->ic_flags|=tempil2->ic_flags&ICG_NO_CVT_MASK;
	  tempil1->ic_code=IC_LEA;
	  tempil1->a1.type=MDF_SIB+tempil1->a1.type.raw_type;
	  tempil1->a1.reg=tempil2->a1.reg<<8+REG_NONE;
	  if (tempil2->ic_data==1)
	    tempil1->a1.reg|=0x4000;
	  else if (tempil2->ic_data==2)
	    tempil1->a1.reg|=0x8000;
	  else
	    tempil1->a1.reg|=0xC000;
	  tempil1->a1.disp=i;
	  OptFree(tempil2);
	  return TRUE;
	}
      }
    }
    if (tempil2->ic_code==IC_MOV || tempil2->ic_code==IC_REG) {
      if (tempil1->a2.type&MDF_STK) {
	if (tempil2->ic_flags & ICF_R_TO_INT) {
	  if (tempil2->a1.type&MDF_IMM)
	    tempil2->a1.disp=tempil2->a1.disp(F64);
	  else
	    tempil1->ic_flags|=ICF_A2_TO_INT;
	} else if (tempil2->ic_flags&ICF_R_TO_F64) {
	  if (tempil2->a1.type&MDF_IMM)
	    tempil2->a1.disp(F64)=tempil2->a1.disp;
	  else
	    tempil1->ic_flags|=ICF_A2_TO_F64;
	}
	tempil1->a2.type=tempil2->a1.type&MDG_MASK+
	      MinI64(tempil1->a2.type.raw_type,
	      MinI64(tempil2->r.type.raw_type,tempil2->a1.type.raw_type));
	tempil1->a2.reg=tempil2->a1.reg;
	tempil1->a2.disp=tempil2->a1.disp;
	tempil1->ic_flags|=tempil2->ic_flags&ICG_NO_CVT_MASK;
	OptSetNOP2(tempil2);
	return TRUE;
      }
      if (tempil1->a1.type&MDF_STK) {
	if (tempil2->ic_flags & ICF_R_TO_INT) {
	  if (tempil2->a1.type&MDF_IMM)
	    tempil2->a1.disp=tempil2->a1.disp(F64);
	  else
	    tempil1->ic_flags|=ICF_A1_TO_INT;
	} else if (tempil2->ic_flags&ICF_R_TO_F64) {
	  if (tempil2->a1.type&MDF_IMM) {
	    if (tempil2->a1.type&RTF_UNSIGNED)
	      tempil2->a1.disp(F64)=tempil2->a1.disp(U64);
	    else
	      tempil2->a1.disp(F64)=tempil2->a1.disp(I64);
	  } else
	    tempil1->ic_flags|=ICF_A1_TO_F64;
	}
	tempil1->a1.type=tempil2->a1.type&MDG_MASK+
	      MinI64(tempil1->a1.type.raw_type,
	      MinI64(tempil2->r.type.raw_type,tempil2->a1.type.raw_type));
	CmpMinTypePointed(tempil1,tempil2->a1_type_pointed_to);
	tempil1->a1.reg=tempil2->a1.reg;
	tempil1->a1.disp=tempil2->a1.disp;
	tempil1->ic_flags|=tempil2->ic_flags&ICG_NO_CVT_MASK;
	OptSetNOP2(tempil2);
	return TRUE;
      }
    }
    if (tempil1->ic_code==IC_DEREF) {
      if (tempil2->ic_code==IC_ADD_CONST && tempil2->a1.type&MDF_REG &&
	    tempil1->a1.type&MDF_STK) {
	i=tempil2->ic_data;
	if (MIN_I32<=i<=MAX_I32 &&
	      !Bt(&cmp.non_ptr_vars_mask,tempil2->a1.reg)) {
	  tempil1->ic_flags|=tempil2->ic_flags;
	  tempil1->ic_code=IC_MOV;
	  tempil1->a1.type=MDF_DISP+tempil1->a1_type_pointed_to;
	  tempil1->a1.reg=tempil2->a1.reg;
	  tempil1->a1.disp=i;
	  OptSetNOP2(tempil2,-1);
	  return TRUE;
	}
      }
      if (tempil2->ic_code==IC_LEA && tempil1->a1.type&MDF_STK) {
	tempil1->ic_flags|=tempil2->ic_flags;
	tempil1->ic_code=IC_MOV;
	tempil1->a1.type=tempil2->a1.type&MDG_MASK+tempil1->a1_type_pointed_to;
	tempil1->a1.reg=tempil2->a1.reg;
	tempil1->a1.disp=tempil2->a1.disp;
	OptFree(tempil2);
	return TRUE;
      }
    }
  }
  if (tempil1->ic_code==IC_DEREF) {
    if (tempil1->a1.type&MDF_REG) {
      tempil1->a1.type=MDF_DISP+tempil1->a1_type_pointed_to;
      tempil1->a1.disp=0;
      tempil1->ic_code=IC_MOV;
      return TRUE;
    }
  }
  return FALSE;
}

U0 OptPass4(CCmpCtrl *cc,COptReg *reg_offsets,I64 *_type)
{
  CHashClass *tempc,*tempc1,*tempc2;
  CIntermediateCode *tempi,*tempi1,*tempi2,*tempil1,*tempil2,*tempil3,
	*tempi_next;
  I64 code,i;
  Bool dead_code=FALSE;
  CCodeMisc *lb;
  CPrsStk *ps=cc->ps;
  ps->ptr=0;
  ps->ptr2=0;

  if (_type)
    *_type=RT_I64;

  tempi=cc->coc.coc_head.next;
  while (code=tempi->ic_code) {
    tempi_next=tempi->next;
    if (dead_code&&code!=IC_LABEL) {
      if (code==IC_JMP||code==IC_SUB_CALL) {
	lb=OptLabelFwd(tempi->ic_data);
	if (lb->use_cnt>0)
	  lb->use_cnt--;
      }
      tempi=OptFree(tempi);
    } else {
      tempc=tempi->ic_class;
      tempi1=tempi2=&cmp.ic_nop;
      if (tempil1=OptLag2(tempi)) {
	if (tempil2=OptLag2(tempil1)) {
	  if (!(tempil3=OptLag2(tempil2)))
	    tempil3=&cmp.ic_nop;
	} else
	  tempil2=tempil3=&cmp.ic_nop;
      } else
	tempil1=tempil2=tempil3=&cmp.ic_nop;
      switch [intermediate_code_table[code].arg_cnt] {
	case IS_V_ARG:
	  ps->ptr-=tempi->ic_data>>3;
	  break;
	case IS_2_ARG:
	  tempi2=PrsPop(ps);
	  tempc2=tempi2->ic_class;
	case IS_1_ARG:
	  tempi1=PrsPop(ps);
	  tempc1=tempi1->ic_class;
	  break;
	case IS_0_ARG: //nobound switch
	  break;
      }
      switch [code] {
	case IC_IMM_I64:
	case IC_TYPE:
	  tempi->a1.type=MDF_IMM+RT_I64;
	  tempi->a1.disp=tempi->ic_data;
	  tempi->ic_code=IC_MOV;
	  break;
	case IC_IMM_F64:
	  tempi->a1.type=MDF_IMM+RT_I64;
	  tempi->a1.disp=tempi->ic_data;
	  tempi->a1_type_pointed_to=RT_F64;
	  tempi->ic_code=IC_MOV;
	  break;
	case IC_MOV:
	  if (tempi->a1.type&MDF_DISP && tempi->a1.reg==REG_RBP) {
	    i=CmpOffset2Reg(tempi->a1.disp,reg_offsets);
	    if (i>=0) {
	      tempi->a1.type=MDF_REG+tempi->a1.type.raw_type;
	      tempi->a1.reg=i;
	      tempi->a1.disp=0;
	    }
	  }
	  break;
	case IC_DEREF:
	  if (tempi1->ic_code==IC_LEA) {
	    if (tempi1->a1.type&MDF_DISP && tempi1->a1.reg==REG_RBP) {
	      i=CmpOffset2Reg(tempi1->a1.disp,reg_offsets);
	      if (i>=0) {
		tempi->ic_flags|=tempi1->ic_flags;
		tempi->ic_code=IC_REG;
		tempi->a1.type=MDF_REG+tempi->a1.type.raw_type;
		tempi->a1.reg=i;
		tempi->a1.disp=0;
		OptFree(tempi1);
	      }
	    }
	  } else if ((tempi1->ic_code==IC_ABS_ADDR ||
		tempi1->ic_code==IC_MOV &&
		tempi1->a1.type==MDF_IMM+RT_I64 &&
		0<=tempi1->a1.disp<=MAX_I32)&& !(tempi1->ic_flags&ICF_NO_RIP)) {
	    if (tempi1->ic_code==IC_ABS_ADDR)
	      tempi->a1.disp=tempi1->ic_data;
	    else
	      tempi->a1.disp=tempi1->a1.disp;
	    tempi->ic_flags|=tempi1->ic_flags;
	    tempi->ic_code=IC_MOV;
	    tempi->a1.type=MDF_RIP_DISP32+tempi->a1_type_pointed_to;
	    tempi->a1.reg=REG_RIP;
	    OptFree(tempi1);
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
	  if (tempi1->ic_code==IC_LEA) {
	    if (tempi1->a1.type&MDF_DISP && tempi1->a1.reg==REG_RBP) {
	      i=CmpOffset2Reg(tempi1->a1.disp,reg_offsets);
	      if (i>=0) {
		tempi->ic_flags|=tempi1->ic_flags;
		tempi->a1.type=MDF_REG+tempi->a1.type.raw_type;
		tempi->a1.reg=i;
		tempi->a1.disp=0;
		tempi->ic_flags|=ICF_BY_VAL;
		OptSetNOP2(tempi1);
	      } else
		goto p4_lea_gone;
	    } else {
p4_lea_gone:
	      tempi->ic_flags|=tempi1->ic_flags;
	      tempi->a1.type=tempi1->a1.type;
	      tempi->a1.reg=tempi1->a1.reg;
	      tempi->a1.disp=tempi1->a1.disp;
	      tempi->ic_flags|=ICF_BY_VAL;
	      OptSetNOP2(tempi1);
	    }
	  } else if ((tempi1->ic_code==IC_ABS_ADDR || tempi1->ic_code==IC_MOV &&
		tempi1->a1.type==MDF_IMM+RT_I64 &&
		0<=tempi1->a1.disp<=MAX_I32)&& !(tempi1->ic_flags&ICF_NO_RIP)) {
	    tempi->ic_flags|=tempi1->ic_flags;
	    if (tempi1->ic_code==IC_ABS_ADDR)
	      tempi->a1.disp=tempi1->ic_data;
	    else
	      tempi->a1.disp=tempi1->a1.disp;
	    tempi->a1.type=MDF_RIP_DISP32+tempi->a1_type_pointed_to;
	    tempi->a1.reg=REG_RIP;
	    tempi->ic_flags|=ICF_BY_VAL;
	    OptFree(tempi1);
	  }
	  break;
	case IC_ADD:
	  if (tempi1->ic_code==IC_MOV && tempi1->a1.type==MDF_REG+RT_I64 ||
		tempi1->ic_code==IC_REG) {
	    if (tempi2->ic_code==IC_MOV && tempi2->a1.type==MDF_REG+RT_I64 ||
		  tempi2->ic_code==IC_REG) {
	      if (tempi2->a1.reg!=REG_RSP) {
		tempi->a1.disp=0;
		tempi->a1.reg=tempi1->a1.reg+tempi2->a1.reg<<8;
		goto p4_sib;
	      } else if (tempi1->a1.reg!=REG_RSP) {
		tempi->a1.disp=0;
		tempi->a1.reg=tempi2->a1.reg+tempi1->a1.reg<<8;
p4_sib:
		tempi->ic_flags|=(tempi1->ic_flags|tempi2->ic_flags)
		&ICG_NO_CVT_MASK;
		OptSetNOP2(tempi1);
		OptFree(tempi2);

		tempi->ic_code=IC_LEA;
		tempi->a1.type=MDF_SIB+RT_I64;
		tempi->a1_type_pointed_to=RT_I64;

		tempi->a2.type=MDF_NULL+tempi->a2.type.raw_type;
	      }
	    } else if (tempi2->ic_code==IC_SHL_CONST &&
		  tempi2->a1.type==MDF_REG+RT_I64 && tempi2->ic_data<=3) {
	      if (tempi2->a1.reg!=REG_RSP) {
		tempi->a1.disp=0;
		tempi->a1.reg=tempi1->a1.reg+tempi2->a1.reg<<8;
		if (tempi2->ic_data==1)
		  tempi->a1.reg|=0x4000;
		else if (tempi2->ic_data==2)
		  tempi->a1.reg|=0x8000;
		else
		  tempi->a1.reg|=0xC000;
		goto p4_sib;
	      }
	    }
	  } else if (tempi1->ic_code==IC_LEA &&
		tempi1->a1.type&MDF_DISP) {
	    if (tempi1->a1.reg==REG_RBP &&
		  CmpOffset2Reg(tempi1->a1.disp,reg_offsets)>=0)
	      break;
	    if (tempi2->ic_code==IC_MOV && tempi2->a1.type==MDF_REG+RT_I64 ||
		  tempi2->ic_code==IC_REG) {
	      if (tempi2->a1.reg!=REG_RSP) {
		tempi->a1.disp=tempi1->a1.disp;
		tempi->a1.reg=tempi1->a1.reg+tempi2->a1.reg<<8;
		goto p4_sib;
	      } else if (tempi1->a1.reg!=REG_RSP) {
		tempi->a1.disp=tempi1->a1.disp;
		tempi->a1.reg=tempi2->a1.reg+tempi1->a1.reg<<8;
		goto p4_sib;
	      }
	    } else if (tempi2->ic_code==IC_SHL_CONST &&
		  tempi2->a1.type==MDF_REG+RT_I64 && tempi2->ic_data<=3) {
	      if (tempi2->a1.reg!=REG_RSP) {
		tempi->a1.disp=tempi1->a1.disp;
		tempi->a1.reg=tempi1->a1.reg+tempi2->a1.reg<<8;
		if (tempi2->ic_data==1)
		  tempi->a1.reg|=0x4000;
		else if (tempi2->ic_data==2)
		  tempi->a1.reg|=0x8000;
		else
		  tempi->a1.reg|=0xC000;
		goto p4_sib;
	      }
	    }
	  }
	  break;
	case IC_ASSIGN_PP:
	case IC_ASSIGN_MM:
//this val was stashed during pass012 for pointer arithmetic
	  tempi->ic_class2=tempi->t.class2; //See $LK,"ic_class2",A="FF:::/Compiler/BackB.CPP,ic_class2"$
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
	  if (tempi1->ic_code==IC_LEA) {
	    if (tempi1->a1.type&(MDF_DISP|MDF_SIB)) {
	      tempi2=tempi->next;
	      if (tempi1->a1.type&MDF_DISP && tempi1->a1.reg==REG_RBP) {
		i=CmpOffset2Reg(tempi1->a1.disp,reg_offsets);
		if (i>=0) {
		  tempi->ic_flags|=tempi1->ic_flags;
		  tempi->a1.type=MDF_REG+tempi->a1.type.raw_type;
		  tempi->a1.reg=i;
		  tempi->a1.disp=0;
		  OptSetNOP2(tempi1);
		} else {
		  tempi->ic_flags|=tempi1->ic_flags;
		  tempi->a1.type=MDF_DISP+tempi->a1.type.raw_type;
		  tempi->a1.reg=REG_RBP;
		  tempi->a1.disp=tempi1->a1.disp;
		  OptSetNOP2(tempi1);
		}
	      } else {
		tempi->ic_flags|=tempi1->ic_flags;
		tempi->a1.type=tempi1->a1.type&MDG_MASK+tempi->a1.type.raw_type;
		tempi->a1.reg=tempi1->a1.reg;
		tempi->a1.disp=tempi1->a1.disp;
		OptSetNOP2(tempi1);
	      }
	      if (tempi->r.type&MDF_STK && tempi2->a2.type&MDF_STK &&
		    code!=IC_ASSIGN_PP && code!=IC_ASSIGN_MM) {
		tempi->r.type=tempi->a1.type;
		tempi->r.reg=tempi->a1.reg;
		tempi->r.disp=tempi->a1.disp;
		tempi2->a2.type=tempi->a1.type;
		tempi2->a2.reg=tempi->a1.reg;
		tempi2->a2.disp=tempi->a1.disp;
		CmpMinTypePointed(tempi2,tempi->a1_type_pointed_to);
	      }
	      tempi->ic_flags|=ICF_BY_VAL;
	    }
	  } else if ((tempi1->ic_code==IC_ABS_ADDR ||
		tempi1->ic_code==IC_MOV && tempi1->a1.type==MDF_IMM+RT_I64 &&
		0<=tempi1->a1.disp<=MAX_I32)&& !(tempi1->ic_flags&ICF_NO_RIP)) {
	    tempi->ic_flags|=tempi1->ic_flags;
	    if (tempi1->ic_code==IC_ABS_ADDR)
	      tempi->a1.disp=tempi1->ic_data;
	    else
	      tempi->a1.disp=tempi1->a1.disp;
	    tempi->a1.type=MDF_RIP_DISP32+tempi->a1.type.raw_type;
	    tempi->a1.reg=REG_RIP;
	    tempi->ic_flags|=ICF_BY_VAL;
	    OptSetNOP2(tempi1);
	  }
	  break;
	case IC_RETURN_VAL:
	case IC_RETURN_VAL2:
	  if (!tempi->ic_class) {
	    if (_type) {
	      tempil1=tempi;
	      while (tempil1=OptLag1(tempil1))
		if (tempil1->ic_class) {
		  if (tempil1->ic_flags & ICF_R_TO_F64)
		    *_type=RT_F64;
		  else if (tempil1->ic_flags & ICF_R_TO_INT)
		    *_type=RT_I64;
		  else
		    *_type=tempil1->ic_class->raw_type;
		  break;
		}
	    }
	    tempi->ic_class=cmp.internal_types[RT_I64];
	  } else if (_type)
	    *_type=tempi->ic_class->raw_type;
	  break;
	case IC_NOP1:
	  tempi=OptFree(tempi);
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
	  if (!(tempi->ic_flags&ICF_BY_VAL)) {
	    if (tempi2->ic_code==IC_ADDR) {
	      if (tempi2->a1.type&MDF_STK &&
		    tempi2->r.type&MDF_STK) {
		if (tempil2=OptLag1(tempi2)) {
		  if (tempil2->ic_code==IC_LEA) {
		    if (tempil2->a1.type&(MDF_IMM|MDG_REG_DISP_SIB_RIP)) {
		      if (tempi2) {
			tempi->ic_flags|=tempi2->ic_flags;
			OptFree(tempi2);
		      }
		      tempi->ic_flags|=tempil2->ic_flags|ICF_BY_VAL;
		      tempi->a2.type=tempil2->a1.type;
		      tempi->a2.reg =tempil2->a1.reg;
		      tempi->a2.disp=tempil2->a1.disp;
		      OptFree(tempil2);
		    }
		    break;
		  } else if (tempil2->ic_code!=IC_ABS_ADDR &&
			!(tempil2->ic_code==IC_MOV &&
			tempil2->a1.type==MDF_IMM+RT_I64 &&
			0<=tempil2->a1.disp<=MAX_I32) ||
			tempil2->ic_flags&ICF_NO_RIP)
		    tempil2=NULL;
		  else {
		    if (tempil2->ic_code==IC_ABS_ADDR)
		      tempi->a2.disp=tempil2->ic_data;
		    else
		      tempi->a2.disp=tempil2->a1.disp;
		  }
		}
	      } else {
		if (tempi2->a1.type==MDF_IMM+RT_I64 &&
		      0<=tempi2->a1.disp<=MAX_I32 &&
		      !(tempi2->ic_flags&ICF_NO_RIP)) {
		  tempil2=tempi2;
		  tempi2=NULL;
		  tempi->a2.disp=tempil2->a1.disp;
		} else
		  tempil2=NULL;
	      }
	      if (tempil2) {
		if (tempi2) {
		  tempi->ic_flags|=tempi2->ic_flags;
		  OptFree(tempi2);
		}
		tempi->ic_flags|=tempil2->ic_flags|ICF_BY_VAL;
		tempi->a2.type=MDF_RIP_DISP32+tempi->a2.type.raw_type;
		tempi->a2.reg=REG_RIP;
		OptFree(tempil2);
	      }
	    } else if (tempi2->ic_code==IC_MOV && tempi2->r.type&MDF_STK &&
		  tempi2->a1.type==MDF_IMM+RT_I64 &&
		  0<=tempi2->a1.disp<=MAX_I32 &&
		  !(tempi2->ic_flags&ICF_NO_RIP)) {
	      tempi->a2.disp=tempi2->a1.disp;
	      tempi->ic_flags|=tempi2->ic_flags|ICF_BY_VAL;
	      tempi->a2.type=MDF_RIP_DISP32+tempi->a2.type.raw_type;
	      tempi->a2.reg=REG_RIP;
	      OptFree(tempi2);
	    }
	  }
	  break;
	case IC_BR_EQU_EQU ...IC_BR_LESS_EQU:
	case IC_BR_EQU_EQU2...IC_BR_LESS_EQU2:
	case IC_BR_CARRY:
	case IC_BR_NOT_CARRY:
	case IC_BR_ZERO:
	case IC_BR_NOT_ZERO:
	  lb=tempi->ic_data;
	  if (tempi->ic_flags&ICF_PUSH_CMP) {
	    lb->flags|=CMF_POP_CMP;
	    lb->fwd=NULL;
	  }
	  break;
	case IC_LABEL:
	  lb=tempi->ic_data;
	  if (lb->use_cnt)
	    dead_code=FALSE;
	  break;
	case IC_JMP:
	case IC_RET:
	  dead_code=TRUE;
	  break;
	case IC_NOP2:
	  ps->ptr+=tempi->ic_data;
	  break;
	case IC_CALL_END:
	case IC_END_EXP:
	  if (!(tempil1->ic_flags&ICF_PUSH_RES)) {
	    if (tempi->ic_flags&ICF_RES_NOT_USED) {
	      tempil1->ic_flags|=ICF_RES_NOT_USED;
	      tempil1->r.type=MDF_NULL+tempil1->r.type.raw_type;
	    } else if (tempi->a1.type&MDF_STK &&
		  tempil1->r.type&MDF_STK) {
	      tempi->a1.type=MDF_REG+tempi->a1.type.raw_type;
	      tempi->a1.disp=0;
	      tempil1->r.type=MDF_REG+tempil1->r.type.raw_type;
	      tempil1->r.disp=0;
	      if (intermediate_code_table[tempi->ic_code].arg_cnt==IS_2_ARG) {
		tempi->a1.reg=REG_R8;
		tempil1->r.reg=REG_R8;
	      } else {
		tempi->a1.reg=REG_RAX;
		tempil1->r.reg=REG_RAX;
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
	  "Pass:%d Missing IC handler\n",cc->pass;
	  ICPut(cc,tempi);
	  LexExcept(cc,"Compiler Optimization Error at ");
      }
      if (tempi) {
	while (OptIC4(tempi));
	code=tempi->ic_code;
	if (intermediate_code_table[code].res_cnt)
	  PrsPush(ps,tempi);
      }
    }
    tempi=tempi_next;
  }
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",cc->pass,ps->ptr;
    LexExcept(cc,"Compiler Optimization Error at ");
  }
}
