Bool OptIC4(CIntermediateCode *tempi)
{
  I64 i,ty,pt,ty1,pt1;
  CIntermediateCode *tempil1,*tempil2;

  if (tempi->ic_opcode<IC_IMM) return FALSE;

  tempil1=tempi;
  if (!(tempil2=OptLag1(tempil1)))
    return FALSE;

  if (tempil2->r.type&TY_MASK==TY_STK &&
      !(tempil2->ic_flags&ICF_PUSH_RESULT)) {
    if (tempil1->ic_opcode==IC_ADD_CONST &&
	tempil1->p1.type&TY_MASK==TY_STK) {
      if ((tempil2->ic_opcode==IC_REG ||
	  tempil2->ic_opcode==IC_MOV) &&
	  tempil2->p1.type&TY_MASK==TY_REG) {
	i=tempil1->ic_data;
	    if (MIN_I32<=i<=MAX_I32 &&
	    !Bt(&non_ptr_vars_mask,tempil2->p1.reg)) {
	  tempil1->ic_flags|=tempil2->ic_flags&ICF_NO_CVT_MASK;
	  tempil2->ic_opcode=IC_NOP;
	  tempil2->p1.type=TY_NULL+tempil2->p1.type&IT_MASK;
	  tempil2->r.type=TY_NULL+tempil2->r.type&IT_MASK;
	  tempil1->ic_opcode=IC_LEA;
	  tempil1->p1.type=TY_DISP+tempil1->p1.type&IT_MASK;
	  tempil1->p1.reg=tempil2->p1.reg;
	  tempil1->p1.disp=i;
	  return TRUE;
	}
      } else if (tempil2->ic_opcode==IC_SHL_CONST &&
	  tempil2->p1.type&TY_MASK==TY_REG) {
	i=tempil1->ic_data;
	    if (MIN_I32<=i<=MAX_I32 &&
	    tempil2->p1.reg!=CREG_RSP &&
	    1<=tempil2->ic_data<=3) {
	  tempil1->ic_flags|=tempil2->ic_flags&ICF_NO_CVT_MASK;
	  tempil2->ic_opcode=IC_NOP;
	  tempil2->p1.type=TY_NULL+tempil2->p1.type&IT_MASK;
	  tempil2->r.type=TY_NULL+tempil2->r.type&IT_MASK;
	  tempil1->ic_opcode=IC_LEA;
	  tempil1->p1.type=TY_SIB+tempil1->p1.type&IT_MASK;
	  tempil1->p1.reg=tempil2->p1.reg<<8+CREG_NONE;
	  if (tempil2->ic_data==1)
	    tempil1->p1.reg|=0x4000;
	  else if (tempil2->ic_data==2)
	    tempil1->p1.reg|=0x8000;
	  else
	    tempil1->p1.reg|=0xC000;
	  tempil1->p1.disp=i;
	  return TRUE;
	}
      }
    }
    if (tempil2->ic_opcode==IC_MOV ||
	tempil2->ic_opcode==IC_REG) {
      if (tempil1->p3.type&TY_MASK==TY_STK) {
	if (tempil2->ic_flags & ICF_R_TO_INT) {
	  if (tempil2->p1.type&TY_MASK==TY_IMM)
	    tempil2->p1.disp=tempil2->p1.disp(F64);
	  else
	    tempil1->ic_flags|=ICF_P3_TO_INT;
	} else if (tempil2->ic_flags&ICF_R_TO_F64) {
	  if (tempil2->p1.type&TY_MASK==TY_IMM)
	    tempil2->p1.disp(F64)=tempil2->p1.disp;
	  else
	    tempil1->ic_flags|=ICF_P3_TO_F64;
	}

	ty=tempil1->p3.type&IT_MASK;
	ty1=tempil2->p1.type&IT_MASK;
	if (ty1 && ty1<ty)
	  ty=ty1;
	ty1=tempil2->r.type&IT_MASK;
	if (ty1 && ty1<ty)
	  ty=ty1;
	tempil1->p3.type=tempil2->p1.type&TY_MASK+ty;

	pt=tempil1->p3.ptype;
	pt1=tempil2->p1.ptype;
	if (pt1 && pt1<pt && pt!=IT_F64)
	  pt=pt1;
	pt1=tempil2->r.ptype;
	if (pt1 && pt1<pt && pt!=IT_F64)
	  pt=pt1;
	tempil1->p3.ptype=pt;

	tempil1->p3.reg=tempil2->p1.reg;
	tempil1->p3.disp=tempil2->p1.disp;
	tempil1->ic_flags|=tempil2->ic_flags&ICF_NO_CVT_MASK;
	tempil2->ic_opcode=IC_NOP1;
	tempil2->ic_data=1;
	tempil2->p1.type=TY_NULL+tempil2->p1.type&IT_MASK;
	tempil2->r.type=TY_NULL+tempil2->r.type&IT_MASK;
	return TRUE;
      }
      if (tempil1->p2.type&TY_MASK==TY_STK) {
	if (tempil2->ic_flags & ICF_R_TO_INT) {
	  if (tempil2->p1.type&TY_MASK==TY_IMM)
	    tempil2->p1.disp=tempil2->p1.disp(F64);
	  else
	    tempil1->ic_flags|=ICF_P2_TO_INT;
	} else if (tempil2->ic_flags&ICF_R_TO_F64) {
	  if (tempil2->p1.type&TY_MASK==TY_IMM)
	    tempil2->p1.disp(F64)=tempil2->p1.disp;
	  else
	    tempil1->ic_flags|=ICF_P2_TO_F64;
	}

	ty=tempil1->p2.type&IT_MASK;
	ty1=tempil2->p1.type&IT_MASK;
	if (ty1 && ty1<ty)
	  ty=ty1;
	ty1=tempil2->r.type&IT_MASK;
	if (ty1 && ty1<ty)
	  ty=ty1;
	tempil1->p2.type=tempil2->p1.type&TY_MASK+ty;

	pt=tempil1->p2.ptype;
	pt1=tempil2->p1.ptype;
	if (pt1 && pt1<pt && pt!=IT_F64)
	  pt=pt1;
	pt1=tempil2->r.ptype;
	if (pt1 && pt1<pt && pt!=IT_F64)
	  pt=pt1;
	tempil1->p2.ptype=pt;

	tempil1->p2.reg=tempil2->p1.reg;
	tempil1->p2.disp=tempil2->p1.disp;
	tempil1->ic_flags|=tempil2->ic_flags&ICF_NO_CVT_MASK;
	tempil2->ic_opcode=IC_NOP1;
	tempil2->ic_data=1;
	tempil2->p1.type=TY_NULL+tempil2->p1.type&IT_MASK;
	tempil2->r.type=TY_NULL+tempil2->r.type&IT_MASK;
	return TRUE;
      }
      if (tempil1->p1.type&TY_MASK==TY_STK) {
	if (tempil2->ic_flags & ICF_R_TO_INT) {
	  if (tempil2->p1.type&TY_MASK==TY_IMM)
	    tempil2->p1.disp=tempil2->p1.disp(F64);
	  else
	    tempil1->ic_flags|=ICF_P1_TO_INT;
	} else if (tempil2->ic_flags&ICF_R_TO_F64) {
	  if (tempil2->p1.type&TY_MASK==TY_IMM) {
	    if (tempil2->p1.type&1) //signed/unsigned
	      tempil2->p1.disp(F64)=tempil2->p1.disp(U64);
	    else
	      tempil2->p1.disp(F64)=tempil2->p1.disp(I64);
	  } else
	    tempil1->ic_flags|=ICF_P1_TO_F64;
	}

	ty=tempil1->p1.type&IT_MASK;
	ty1=tempil2->p1.type&IT_MASK;
	if (ty1 && ty1<ty)
	  ty=ty1;
	ty1=tempil2->r.type&IT_MASK;
	if (ty1 && ty1<ty)
	  ty=ty1;
	tempil1->p1.type=tempil2->p1.type&TY_MASK+ty;

	pt=tempil1->p1.ptype;
	pt1=tempil2->p1.ptype;
	if (pt1 && pt1<pt && pt!=IT_F64)
	  pt=pt1;
	pt1=tempil2->r.ptype;
	if (pt1 && pt1<pt && pt!=IT_F64)
	  pt=pt1;
	tempil1->p1.ptype=pt;

	tempil1->p1.reg=tempil2->p1.reg;
	tempil1->p1.disp=tempil2->p1.disp;
	tempil1->ic_flags|=tempil2->ic_flags&ICF_NO_CVT_MASK;
	tempil2->ic_opcode=IC_NOP1;
	tempil2->ic_data=1;
	tempil2->p1.type=TY_NULL+tempil2->p1.type&IT_MASK;
	tempil2->r.type=TY_NULL+tempil2->r.type&IT_MASK;
	return TRUE;
      }
    }
    if (tempil1->ic_opcode==IC_DEREF) {
      if (tempil2->ic_opcode==IC_ADD_CONST &&
	  tempil2->p1.type&TY_MASK==TY_REG &&
	  tempil1->p1.type&TY_MASK==TY_STK) {
	i=tempil2->ic_data;
	    if (MIN_I32<=i<=MAX_I32 &&
	    !Bt(&non_ptr_vars_mask,tempil2->p1.reg)) {
	  tempil1->ic_flags|=tempil2->ic_flags;
	  tempil2->ic_opcode=IC_NOP1;
	  tempil2->ic_data=-1;
	  tempil2->p1.type=TY_NULL+tempil2->p1.type&IT_MASK;
	  tempil2->r.type=TY_NULL+tempil2->r.type&IT_MASK;
	  tempil1->ic_opcode=IC_MOV;
	  tempil1->p1.type=TY_DISP+tempil1->p1.ptype;
	  tempil1->p1.reg=tempil2->p1.reg;
	  tempil1->p1.disp=i;
	  return TRUE;
	}
      }
      if (tempil2->ic_opcode==IC_LEA &&
	  tempil1->p1.type&TY_MASK==TY_STK) {
	tempil1->ic_flags|=tempil2->ic_flags;
	tempil2->ic_opcode=IC_NOP;
	tempil1->ic_opcode=IC_MOV;
	tempil1->p1.type=tempil2->p1.type&TY_MASK+tempil1->p1.ptype;
	tempil1->p1.reg=tempil2->p1.reg;
	tempil1->p1.disp=tempil2->p1.disp;
	return TRUE;
      }
    }
  }
  if (tempil1->ic_opcode==IC_DEREF) {
    if (tempil1->p1.type&TY_MASK==TY_REG) {
      tempil1->p1.type=TY_DISP+tempil1->p1.ptype;
      tempil1->p1.disp=0;
      tempil1->ic_opcode=IC_MOV;
      return TRUE;
    }
  }
  return FALSE;
}

U0 OptPass4(CLex *lx,COptReg *register_offsets,I64 *_type)
{
  CHashClass *tempc,*tempc1,*tempc2,*tempc3;
  CIntermediateCode *tempi,*tempi1,*tempi2,*tempi3,*tempil1,*tempil2,*tempil3,
  *temp_lag[4];
  I64 code,i;
  CPrsStk *ps=OptStart(lx,temp_lag);
  Bool dead_code=FALSE;
  CCodeBlkMisc *lb;

  if (_type)
    *_type=IT_I64;

  tempi=lx->cbh.cb_in_ptr;
  if (tempi->ic_opcode==IC_START)
    tempi++;
  while (code=tempi->ic_opcode) {
    OptNext2(tempi,temp_lag);
    if (dead_code&&code!=IC_LABEL)
      tempi->ic_opcode=IC_NOP;
    else {
      tempc=tempi->ic_class;

      tempi3=&opt_nop;
      tempi2=&opt_nop;
      tempi1=&opt_nop;

      tempil1=temp_lag[1];
      tempil2=temp_lag[2];
      tempil3=temp_lag[3];
      nobound_switch (cmp_arg_cnt[code]) {
	case 9:
	  ps->ptr-=tempi->ic_data>>3;
	  break;
	case 3:
	  tempi3=PrsPop(ps);
	  tempc3=tempi3->ic_class;
	case 2:
	  tempi2=PrsPop(ps);
	  tempc2=tempi2->ic_class;
	case 1:
	  tempi1=PrsPop(ps);
	  tempc1=tempi1->ic_class;
	  break;
	case 0: //nobound_switch
	  break;
      }
      nobound_switch (code) {
	case IC_IMM:
	case IC_TYPE:
	  tempi->p1.type=TY_IMM+IT_I64;
	  tempi->p1.disp=tempi->ic_data;
	  tempi->ic_opcode=IC_MOV;
	  break;
	case IC_F64:
	  tempi->p1.type=TY_IMM+IT_I64;
	  tempi->p1.disp=tempi->ic_data;
	  tempi->p1.ptype=IT_F64;
	  tempi->ic_opcode=IC_MOV;
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
	case IC_NATIVE_TYPECAST:
	case IC_NOT:
	case IC_UNARY_MINUS:
	case IC_PUSH_CMP:
	  break;
	case IC_MOV:
	  if (tempi->p1.type&TY_MASK==TY_DISP &&
	      tempi->p1.reg==CREG_RBP) {
	    i=CmpOffset2Reg(tempi->p1.disp,register_offsets);
	    if (i>=0) {
	      tempi->p1.type=TY_REG+tempi->p1.type&IT_MASK;
	      tempi->p1.reg=i;
	      tempi->p1.disp=0;
	    }
	  }
	  break;
	case IC_DEREF:
	  if (tempi1->ic_opcode==IC_LEA) {
	    if (tempi1->p1.type&TY_MASK==TY_DISP &&
		tempi1->p1.reg==CREG_RBP) {
	      i=CmpOffset2Reg(tempi1->p1.disp,register_offsets);
	      if (i>=0) {
		tempi->ic_flags|=tempi1->ic_flags;
		tempi1->ic_opcode=IC_NOP;
		tempi->ic_opcode=IC_REG;
		tempi->p1.type=TY_REG+tempi->p1.type&IT_MASK;
		tempi->p1.reg=i;
		tempi->p1.disp=0;
	      }
	    }
	  } else if ((tempi1->ic_opcode==IC_ABS_ADDR ||
	      tempi1->ic_opcode==IC_MOV &&
	      tempi1->p1.type==TY_IMM+IT_I64 &&
	      0<=tempi1->p1.disp<=MAX_I32) &&
	      !(tempi1->ic_flags&ICF_NO_RIP)) {
	    if (tempi1->ic_opcode==IC_ABS_ADDR)
	      tempi->p1.disp=tempi1->ic_data;
	    else
	      tempi->p1.disp=tempi1->p1.disp;
	    tempi->ic_flags|=tempi1->ic_flags;
	    tempi1->ic_opcode=IC_NOP;
	    tempi1->p1.type=TY_NULL+tempi1->p1.type&IT_MASK;
	    tempi1->r.type=TY_NULL+tempi1->r.type&IT_MASK;
	    tempi->ic_opcode=IC_MOV;
	    tempi->p1.type=TY_RIP_DISP32+tempi->p1.ptype;
	    tempi->p1.reg=CREG_RIP;
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
	  if (tempi1->ic_opcode==IC_LEA) {
	    if (tempi1->p1.type&TY_MASK==TY_DISP &&
		tempi1->p1.reg==CREG_RBP) {
	      i=CmpOffset2Reg(tempi1->p1.disp,register_offsets);
	      if (i>=0) {
		tempi->ic_flags|=tempi1->ic_flags;
		tempi1->ic_opcode=IC_NOP1;
		tempi1->ic_data=1;
		tempi1->r.type=TY_NULL;
		tempi->p1.type=TY_REG+tempi->p1.type&IT_MASK;
		tempi->p1.reg=i;
		tempi->p1.disp=0;
		tempi->ic_flags|=ICF_NOT_ADDR;
	      } else
		goto lea_gone;
	    } else {
	      lea_gone:
	      tempi->ic_flags|=tempi1->ic_flags;
	      tempi1->ic_opcode=IC_NOP1;
	      tempi1->ic_data=1;
	      tempi1->r.type=TY_NULL;
	      tempi->p1.type=tempi1->p1.type;
	      tempi->p1.reg=tempi1->p1.reg;
	      tempi->p1.disp=tempi1->p1.disp;
	      tempi->ic_flags|=ICF_NOT_ADDR;
	    }
	  } else if ((tempi1->ic_opcode==IC_ABS_ADDR ||
	      tempi1->ic_opcode==IC_MOV &&
	      tempi1->p1.type==TY_IMM+IT_I64 &&
	      0<=tempi1->p1.disp<=MAX_I32) &&
	      !(tempi1->ic_flags&ICF_NO_RIP)) {
	    tempi->ic_flags|=tempi1->ic_flags;
	    if (tempi1->ic_opcode==IC_ABS_ADDR)
	      tempi->p1.disp=tempi1->ic_data;
	    else
	      tempi->p1.disp=tempi1->p1.disp;
	    tempi1->ic_opcode=IC_NOP;
	    tempi1->p1.type=TY_NULL+tempi1->p1.type&IT_MASK;
	    tempi1->r.type=TY_NULL+tempi1->r.type&IT_MASK;
	    tempi->p1.type=TY_RIP_DISP32+tempi->p1.ptype;
	    tempi->p1.reg=CREG_RIP;
	    tempi->ic_flags|=ICF_NOT_ADDR;
	  }
	  break;
	case IC_ADD:
	  if (tempi1->ic_opcode==IC_MOV && tempi1->p1.type==TY_REG+IT_I64 ||
	      tempi1->ic_opcode==IC_REG) {
	    if (tempi2->ic_opcode==IC_MOV && tempi2->p1.type==TY_REG+IT_I64 ||
		tempi2->ic_opcode==IC_REG) {
	      if (tempi2->p1.reg!=CREG_RSP) {
		tempi->p1.disp=0;
		tempi->p1.reg=tempi1->p1.reg+tempi2->p1.reg<<8;
		goto sib;
	      } else if (tempi1->p1.reg!=CREG_RSP) {
		tempi->p1.disp=0;
		tempi->p1.reg=tempi2->p1.reg+tempi1->p1.reg<<8;
sib:
		tempi->ic_flags|=tempi1->ic_flags&ICF_NO_CVT_MASK;
		tempi1->ic_opcode=IC_NOP1;
		tempi1->ic_data=1;
		tempi1->p1.type=TY_NULL+tempi1->p1.type&IT_MASK;
		tempi1->r.type=TY_NULL+tempi1->r.type&IT_MASK;

		tempi->ic_flags|=tempi2->ic_flags&ICF_NO_CVT_MASK;
		tempi2->ic_opcode=IC_NOP;

		tempi->ic_opcode=IC_LEA;
		tempi->p1.type=TY_SIB+IT_I64;
		tempi->p1.ptype=IT_I64;

		tempi->p2.type=TY_NULL+tempi->p2.type&IT_MASK;
	      }
	    } else if (tempi2->ic_opcode==IC_SHL_CONST && tempi2->p1.type==TY_REG+IT_I64 &&
		tempi2->ic_data<=3) {
	      if (tempi2->p1.reg!=CREG_RSP) {
		tempi->p1.disp=0;
		tempi->p1.reg=tempi1->p1.reg+tempi2->p1.reg<<8;
		if (tempi2->ic_data==1)
		  tempi->p1.reg|=0x4000;
		else if (tempi2->ic_data==2)
		  tempi->p1.reg|=0x8000;
		else
		  tempi->p1.reg|=0xC000;
		goto sib;
	      }
	    }
	  } else if (tempi1->ic_opcode==IC_LEA && tempi1->p1.type&TY_MASK==TY_DISP) {
	    if (tempi1->p1.reg==CREG_RBP && CmpOffset2Reg(tempi1->p1.disp,register_offsets)>=0)
	      break;
	    if (tempi2->ic_opcode==IC_MOV && tempi2->p1.type==TY_REG+IT_I64 ||
		tempi2->ic_opcode==IC_REG) {
	      if (tempi2->p1.reg!=CREG_RSP) {
		tempi->p1.disp=tempi1->p1.disp;
		tempi->p1.reg=tempi1->p1.reg+tempi2->p1.reg<<8;
		goto sib;
	      } else if (tempi1->p1.reg!=CREG_RSP) {
		tempi->p1.disp=tempi1->p1.disp;
		tempi->p1.reg=tempi2->p1.reg+tempi1->p1.reg<<8;
		goto sib;
	      }
	    } else if (tempi2->ic_opcode==IC_SHL_CONST && tempi2->p1.type==TY_REG+IT_I64 &&
		tempi2->ic_data<=3) {
	      if (tempi2->p1.reg!=CREG_RSP) {
		tempi->p1.disp=tempi1->p1.disp;
		tempi->p1.reg=tempi1->p1.reg+tempi2->p1.reg<<8;
		if (tempi2->ic_data==1)
		  tempi->p1.reg|=0x4000;
		else if (tempi2->ic_data==2)
		  tempi->p1.reg|=0x8000;
		else
		  tempi->p1.reg|=0xC000;
		goto sib;
	      }
	    }
	  }
	  break;
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
	case IC_EQUAL_EQUAL:
	case IC_NOT_EQUAL:
	case IC_LESS:
	case IC_GREATER:
	case IC_LESS_EQUAL:
	case IC_GREATER_EQUAL:
	case IC_AND_AND:
	case IC_OR_OR:
	case IC_XOR_XOR:
	case IC_COLON:
	  break;
	case IC_ASSIGN_PP:
	case IC_ASSIGN_MM:
	//this val was stashed during pass012 for pointer arithmetic
	  tempi->ic_class2=tempi->t.p3c;
	case IC_ASSIGN:
	case IC_SHL_EQUAL:
	case IC_SHR_EQUAL:
	case IC_MUL_EQUAL:
	case IC_DIV_EQUAL:
	case IC_MOD_EQUAL:
	case IC_AND_EQUAL:
	case IC_OR_EQUAL:
	case IC_XOR_EQUAL:
	case IC_ADD_EQUAL:
	case IC_SUB_EQUAL:
	  if (tempi1->ic_opcode==IC_LEA) {
	    if (TY_DISP<=tempi1->p1.type&TY_MASK<=TY_SIB) {
	      if (tempi1->p1.type&TY_MASK==TY_DISP && tempi1->p1.reg==CREG_RBP) {
		i=CmpOffset2Reg(tempi1->p1.disp,register_offsets);
		if (i>=0) {
		  tempi->ic_flags|=tempi1->ic_flags;
		  tempi1->ic_opcode=IC_NOP1;
		  tempi1->ic_data=1;
		  tempi1->p1.type=TY_NULL;
		  tempi1->r.type=TY_NULL;
		  tempi->p1.type=TY_REG+tempi->p1.type&IT_MASK;
		  tempi->p1.reg=i;
		  tempi->p1.disp=0;
		} else {
		  tempi->ic_flags|=tempi1->ic_flags;
		  tempi1->ic_opcode=IC_NOP1;
		  tempi1->ic_data=1;
		  tempi1->r.type=TY_NULL;
		  tempi1->p1.type=TY_NULL;
		  tempi->p1.type=TY_DISP+tempi->p1.type&IT_MASK;
		  tempi->p1.reg=CREG_RBP;
		  tempi->p1.disp=tempi1->p1.disp;
		}
	      } else {
		tempi->p1.type=tempi1->p1.type&TY_MASK+tempi->p1.type&IT_MASK;
		tempi->ic_flags|=tempi1->ic_flags;
		tempi1->ic_opcode=IC_NOP1;
		tempi1->ic_data=1;
		tempi1->r.type=TY_NULL;
		tempi1->p1.type=TY_NULL;
		tempi->p1.reg=tempi1->p1.reg;
		tempi->p1.disp=tempi1->p1.disp;
	      }
	      tempi->ic_flags|=ICF_NOT_ADDR;
	    }
	  } else if ((tempi1->ic_opcode==IC_ABS_ADDR ||
	      tempi1->ic_opcode==IC_MOV &&
	      tempi1->p1.type==TY_IMM+IT_I64 &&
	      0<=tempi1->p1.disp<=MAX_I32) &&
	      !(tempi1->ic_flags&ICF_NO_RIP)) {
	    tempi->ic_flags|=tempi1->ic_flags;
	    if (tempi1->ic_opcode==IC_ABS_ADDR)
	      tempi->p1.disp=tempi1->ic_data;
	    else
	      tempi->p1.disp=tempi1->p1.disp;
	    tempi1->ic_opcode=IC_NOP1;
	    tempi1->ic_data=1;
	    tempi1->p1.type=TY_NULL+tempi1->p1.type&IT_MASK;
	    tempi1->r.type=TY_NULL+tempi1->r.type&IT_MASK;
	    tempi1->r.type=TY_NULL;
	    tempi->p1.type=TY_RIP_DISP32+tempi->p1.type&IT_MASK;
	    tempi->p1.reg=CREG_RIP;
	    tempi->ic_flags|=ICF_NOT_ADDR;
	  }
	  break;
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
	case IC_CLI:
	case IC_STI:
	case IC_HLT:
	case IC_PUSHFD:
	case IC_POPFD:
	case IC_PUSH:
	case IC_POP:
	case IC_CLD:
	case IC_STD:
	case IC_INVLPG:
	case IC_CLFLUSH:
	case IC_PAUSE:
	case IC_GET_FLAGS:
	case IC_RDTSC:
	case IC_HPET:
	case IC_SET_FLAGS:
	case IC_GET_BASE_PTR:
	case IC_SET_BASE_PTR:
	case IC_GET_STK_PTR:
	case IC_GET_RAX:
	case IC_SET_STK_PTR:
	case IC_SET_RAX:
	  break;
	case IC_RETURN_VAL:
	case IC_RETURN_VAL2:
	  if (!tempi->ic_class) {
	    if (_type) {
	      tempil1=tempi;
	      while (tempil1=OptLag1(tempil1))
		if (tempil1->ic_class) {
		  if (tempil1->ic_flags & ICF_R_TO_F64)
		    *_type=IT_F64;
		  else if (tempil1->ic_flags & ICF_R_TO_INT)
		    *_type=IT_I64;
		  else
		    *_type=tempil1->ic_class->sub_type;
		  break;
		}
	    }
	    tempi->ic_class=sys_internal_types[IT_I64];
	  } else if (_type)
	    *_type=tempi->ic_class->sub_type;
	  break;
	case IC_GET_LABEL:
	case IC_ABS_ADDR:
	case IC_HEAP_GLBL:
	case IC_ADDR_IMPORT:
	case IC_BSF:
	case IC_BSR:
	case IC_PREEMPT:
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
	case IC_LIMIT_I64:
	case IC_LIMIT_U64:
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
	case IC_SWAP_U64:
	case IC_QUE_INIT:
	case IC_QUE_INS:
	case IC_QUE_INS_REV:
	case IC_QUE_REM:
	case IC_MEMCPY:
	case IC_MEMSET:
	case IC_MEMSET_U16:
	case IC_MEMSET_U32:
	case IC_MEMSET_U64:
	case IC_INU32:
	case IC_INU16:
	case IC_INU8:
	case IC_STRLEN:
	case IC_OUTU32:
	case IC_OUTU16:
	case IC_OUTU8:
	case IC_NOBOUND_SWITCH:
	case IC_SWITCH:
	case IC_RET:
	case IC_END:
	case IC_ADDR:
	case IC_QUESTION:
	case IC_IRQ:
	case IC_NOP:
	case IC_CALL_START:
	case IC_LEAVE:
	case IC_PUSH_REGS:
	case IC_POP_REGS:
	case IC_ASM:
	case IC_BR_AND_NOT_ZERO:
	case IC_BR_AND_ZERO:
	  break;
	case IC_BR_BT:
	case IC_BR_BTS:
	case IC_BR_BTR:
	case IC_BR_BTC:
	case IC_BR_NOT_BT:
	case IC_BR_NOT_BTS:
	case IC_BR_NOT_BTR:
	case IC_BR_NOT_BTC:
	case IC_BR_LBTS:
	case IC_BR_LBTR:
	case IC_BR_LBTC:
	case IC_BR_NOT_LBTS:
	case IC_BR_NOT_LBTR:
	case IC_BR_NOT_LBTC:
	case IC_BT:
	case IC_BTS:
	case IC_BTR:
	case IC_BTC:
	case IC_LBTS:
	case IC_LBTR:
	case IC_LBTC:
	  if (tempi2->ic_opcode==IC_ADDR &&
	      !(tempi->ic_flags&ICF_NOT_ADDR)) {
	    if (tempi2->p1.type&TY_MASK==TY_STK &&
		tempi2->r.type&TY_MASK==TY_STK) {
	      if (!(tempil2=OptLag1(tempi2)) ||
		   tempil2->ic_opcode!=IC_ABS_ADDR &&
		   !(tempil2->ic_opcode==IC_MOV &&
		     tempil2->p1.type==TY_IMM+IT_I64 &&
		     0<=tempil2->p1.disp<=MAX_I32) ||
		     tempil2->ic_flags&ICF_NO_RIP)
		tempil2=NULL;
	      else {
		if (tempil2->ic_opcode==IC_ABS_ADDR)
		  tempi->p2.disp=tempil2->ic_data;
		else
		  tempi->p2.disp=tempil2->p1.disp;
	      }
	    } else {
	      if (tempi2->p1.type==TY_IMM+IT_I64 &&
		  0<=tempi2->p1.disp<=MAX_I32 &&
		  !(tempi2->ic_flags&ICF_NO_RIP)) {
		tempil2=tempi2;
		tempi2=NULL;
		tempi->p2.disp=tempil2->p1.disp;
	      } else
		tempil2=NULL;
	    }
	    if (tempil2) {
	      if (tempi2) {
		tempi->ic_flags|=tempi2->ic_flags;
		tempi2->ic_opcode=IC_NOP;
		tempi2->ic_flags=0;
		tempi2 ->p1.type=TY_NULL+tempi2 ->p1.type&IT_MASK;
		tempi2 ->r .type=TY_NULL+tempi2 ->r .type&IT_MASK;
	      }
	      tempi->ic_flags|=tempil2->ic_flags|ICF_NOT_ADDR;
	      tempil2->ic_opcode=IC_NOP;
	      tempil2->ic_flags=0;
	      tempil2->p1.type=TY_NULL+tempil2->p1.type&IT_MASK;
	      tempil2->r .type=TY_NULL+tempil2->r .type&IT_MASK;
	      tempi->p2.type=TY_RIP_DISP32+tempi->p2.type&IT_MASK;
	      tempi->p2.reg=CREG_RIP;
	    }
	  }
	  break;
	case IC_BEQU:
	case IC_LBEQU:
	  if (tempi3->ic_opcode==IC_ADDR &&
	      !(tempi->ic_flags&ICF_NOT_ADDR)) {
	    if (tempi3->p1.type&TY_MASK==TY_STK &&
		tempi3->r.type&TY_MASK==TY_STK) {
	      if (!(tempil3=OptLag1(tempi3)) ||
		   tempil3->ic_opcode!=IC_ABS_ADDR &&
		   !(tempil3->ic_opcode==IC_MOV &&
		     tempil3->p1.type==TY_IMM+IT_I64 &&
		     0<=tempil3->p1.disp<=MAX_I32) ||
		     tempil3->ic_flags&ICF_NO_RIP)
		tempil3=NULL;
	      else {
		if (tempil3->ic_opcode==IC_ABS_ADDR)
		  tempi->p3.disp=tempil3->ic_data;
		else
		  tempi->p3.disp=tempil3->p1.disp;
	      }
	    } else {
	      if (tempi3->p1.type==TY_IMM+IT_I64 &&
		  0<=tempi3->p1.disp<=MAX_I32 &&
		  !(tempi3->ic_flags&ICF_NO_RIP)) {
		tempil3=tempi3;
		tempi3=NULL;
		tempi->p3.disp=tempil3->p1.disp;
	      } else
		tempil3=NULL;
	    }
	    if (tempil3) {
	      if (tempi3) {
		tempi->ic_flags|=tempi3->ic_flags;
		tempi3->ic_opcode=IC_NOP;
		tempi3->ic_flags=0;
		tempi3 ->p1.type=TY_NULL+tempi3 ->p1.type&IT_MASK;
		tempi3 ->r .type=TY_NULL+tempi3 ->r .type&IT_MASK;
	      }
	      tempi->ic_flags|=tempil3->ic_flags|ICF_NOT_ADDR;
	      tempil3->ic_opcode=IC_NOP;
	      tempil3->ic_flags=0;
	      tempil3->p1.type=TY_NULL+tempil3->p1.type&IT_MASK;
	      tempil3->r .type=TY_NULL+tempil3->r .type&IT_MASK;
	      tempi->p3.type=TY_RIP_DISP32+tempi->p3.type&IT_MASK;
	      tempi->p3.reg=CREG_RIP;
	    }
	  }
	  break;
	case IC_BR_NOT_EQUAL2:
	case IC_BR_EQUAL_EQUAL2:
	case IC_BR_GREATER_EQUAL2:
	case IC_BR_LESS_EQUAL2:
	case IC_BR_GREATER2:
	case IC_BR_LESS2:

	case IC_BR_ZERO:
	case IC_BR_NOT_ZERO:
	case IC_BR_NOT_EQUAL:
	case IC_BR_EQUAL_EQUAL:
	case IC_BR_GREATER_EQUAL:
	case IC_BR_LESS_EQUAL:
	case IC_BR_GREATER:
	case IC_BR_LESS:
	  lb=tempi->ic_data;
	  if (tempi->ic_flags&ICF_PUSH_CMP) {
	    lb->flags|=CBMF_POP_CMP;
	    lb->fwd=NULL;
	  }
	  break;
	case IC_LABEL:
	  lb=tempi->ic_data;
	  if (lb->flags&CBMF_USED)
	    dead_code=FALSE;
	  break;
	case IC_JMP:
	  dead_code=TRUE;
	  break;
	case IC_NOP1:
	  ps->ptr+=tempi->ic_data;
	  break;
	case IC_CALL_END2:
	  break;
	case IC_CALL_END:
	case IC_END_EXP:
	  if (tempi->p1.type&TY_MASK==TY_STK) {
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
	  break;
	default:
	  OptMissing(lx,tempi);
      }

      while (OptIC4(tempi));
      code=tempi->ic_opcode;
      if (cmp_result_cnt[code])
	PrsPush(ps,tempi);
    }
    tempi++;
  }
  OptEnd(lx,ps);
}
