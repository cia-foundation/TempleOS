/*OptPass012

Pass#0
When parsing the arg expressions to a function
call, there is a call to OptPass012 to determine
the type of the expression. $LK,"OptPass012",A="FF:::/Compiler/PrsExp.CPP,OptPass012"$

Pass#1&2
Constant expressions are simplified.
Eliminated opcodes are set to NOP.
Types are determined by reconstructing an
expression tree for operators
$LK,"CIntermediateCode",A="MN:CIntermediateCode"$.$LK,"CICTreeLinks",A="MN:CICTreeLinks"$.

Pointer arithmetic size is set, once
the type is determined.

Branches are expressed with short-circuit
logic. 3-Arg comparisons are established.

*/

CIntermediateCode *OptPass012(CCmpCtrl *cc)
{/*Simplify CONST arithmetic.
Sets the class throughout Expression trees.
Returns the type of an Expression for use
in int<-->F64 conversions of fun
args.
*/
  I64 code,i;
  Bool is_unsigned;
  CHashClass *tempc,*tempc1,*tempc2;
  CIntermediateCode *tempi,*tempi1,*tempi2,*tempi3,*tempi_push,
	*last_with_class=NULL;
  CCodeMisc *lb,*lb1,*lb2;
  CPrsStk *ps;
  if (!(ps=cc->ps))
    ps=cc->ps=MAlloc(sizeof(CPrsStk));
  ps->ptr=0;
  ps->ptr2=0;
  tempi=cc->coc.coc_root.next;
  while (code=tempi->ic_code) {
    tempc=tempi->ic_class;
    tempi->ic_class2=tempc;
    tempi_push=tempi;
    MemSet(&tempi->a1,0,sizeof(CICArg)*4);
    nobound_switch (intermediate_code_table[code].arg_cnt) {
      case IS_V_ARG:
	ps->ptr-=tempi->ic_data>>2;
	break;
      case IS_2_ARG:
	tempi2=PrsPop(ps);
	tempc2=tempi2->ic_class;
	tempi->t.a2t=tempi2;
	tempi->t.a2c=PrsPop(ps);
      case IS_1_ARG:
	tempi1=PrsPop(ps);
	tempc1=tempi1->ic_class;
	tempi->t.a1t=tempi1;
	tempi->t.a1c=PrsPop(ps);
	break;
      case IS_0_ARG: //nobound_switch
	break;
    }
    if (intermediate_code_table[code].not_const)
      cc->flags|=CCF_NOT_CONST;
    nobound_switch (code) {
      case IC_IMM_F64:
	tempi->ic_flags&=~ICF_R_TO_F64;
	if (cc->pass==2 && tempi->ic_flags&ICF_R_TO_INT) {
	  tempi->ic_data=ToI64(tempi->ic_data(F64));
	  tempi->ic_flags&=~ICF_R_TO_INT;
	  tempi->ic_code=IC_IMM_I64;
	  tempi->ic_class=cmp.internal_types[RT_I64];
	}
	break;
      case IC_IMM_I64:
	tempi->ic_flags&=~ICF_R_TO_INT;
	if (cc->pass==2 && tempi->ic_flags&ICF_R_TO_F64) {
	  tempi->ic_data(F64)=ToF64(tempi->ic_data);
	  tempi->ic_flags&=~ICF_R_TO_F64;
	  tempi->ic_code=IC_IMM_F64;
	  tempi->ic_class=cmp.internal_types[RT_F64];
	}
	break;
      case IC_HOLYC_TYPECAST:
	if (tempi1->ic_code==IC_IMM_I64 || tempi1->ic_code==IC_IMM_F64) {
	  if (tempi->ic_class->raw_type==RT_F64)
	    tempi1->ic_code=IC_IMM_F64;
	  else
	    tempi1->ic_code=IC_IMM_I64;
	  tempi1->ic_class=tempi->ic_class;
	  tempi1->ic_flags|=tempi->ic_flags;
	  tempi_push=tempi1;
	  OptSetNOP1(tempi);
	} else {
	  if (tempi->ic_data) {//was paren
	    if (!tempi_push->ic_class->ptr_stars_cnt) {
	      if (tempi_push->ic_class->raw_type==RT_F64)
		tempi_push->ic_class2=cmp.internal_types[RT_F64];
	      else
		tempi_push->ic_class2=cmp.internal_types[RT_I64];
	    }
	  } else {
	    tempi1->ic_class=tempi->ic_class;
	    tempi1->ic_flags|=tempi->ic_flags;
	    tempi_push=tempi1;
	    OptSetNOP1(tempi);
	  }
	}
	break;
      case IC_FS:
      case IC_GS:
//CALL,FS/GS,CALL_END,IMM,ADD,DEREF-->MOV_FS/GS
	tempi1=tempi->next->next; //IMM
	tempi2=tempi1->next; //ADD
	tempi3=tempi2->next; //DEREF
	if (tempi1->ic_code==IC_IMM_I64 && tempi2->ic_code==IC_ADD &&
	      tempi3->ic_code==IC_DEREF &&
	      !(tempi3->ic_flags&~ICG_NO_CVT_MASK)) {
	  tempi->ic_flags|=tempi1->ic_flags|tempi2->ic_flags|tempi3->ic_flags;
	  if (tempi->ic_code==IC_FS)
	    tempi->ic_code=IC_MOV_FS;
	  else
	    tempi->ic_code=IC_MOV_GS;
	  tempi->ic_data=tempi1->ic_data;
	  tempi->ic_class =tempi3->ic_class;
	  tempi->ic_class2=tempi3->ic_class2;
	  OptSetNOP1(tempi1);
	  OptSetNOP1(tempi2);
	  OptSetNOP1(tempi3);

	  tempi1=tempi->last; //CALL
	  tempi2=tempi->next; //CALL_END
	  tempi->ic_flags|=tempi1->ic_flags|tempi2->ic_flags;
	  OptSetNOP1(tempi1);
	  OptSetNOP1(tempi2);
	}
	break;
      case IC_PUSH_CMP:
	if (tempi1=OptLag(tempi)) {
	  if (tempi1->ic_code==IC_AND_AND)
	    tempi1=OptLag(tempi1);
	  if (tempi1)
	    tempi->ic_class=tempi1->ic_class;
	}
	tempi->ic_class2=tempi->ic_class;
	tempi->ic_data=0;
	if (tempi->ic_class->raw_type==RT_F64)
	  tempi->ic_flags|=ICF_USE_F64;
	break;
      case IC_COM:
	if (tempi1->ic_code==IC_IMM_I64) {
	  tempi->ic_data=~tempi1->ic_data;
	  tempi->ic_code=IC_IMM_I64;
	  tempi->ic_flags|=tempi1->ic_flags;
	  OptSetNOP1(tempi1);
	}
	tempi_push->ic_class2=cmp.internal_types[RT_I64];
	break;
      sub_switch_start:
	case IC_NOT:
	  if (tempc->raw_type==RT_F64) {
	    if (tempi1->ic_code==IC_IMM_F64) {
	      tempi->ic_data(F64)=!tempi1->ic_data(F64);
	      tempi->ic_code=IC_IMM_F64;
	      tempi->ic_flags|=tempi1->ic_flags;
	      OptSetNOP1(tempi1);
	    }
	    break;
	  }
	  if (tempi1->ic_code==IC_IMM_I64) {
	    tempi->ic_data=!tempi1->ic_data;
	    tempi->ic_code=IC_IMM_I64;
	    tempi->ic_flags|=tempi1->ic_flags;
	    OptSetNOP1(tempi1);
	  }
	  break;
	case IC_UNARY_MINUS:
	  if (i=OptFixupUnaryOp(tempi,tempi1,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      tempi->ic_data=-tempi1->ic_data(I64);
	      tempi->ic_code=IC_IMM_I64;
	    } else {
	      tempi->ic_data(F64)=-tempi1->ic_data(F64);
	      tempi->ic_code=IC_IMM_F64;
	    }
	  }
	  if (tempc1->type&HTT_INTERNAL_TYPE && tempc1->raw_type& 1)
	    tempi->ic_class=cmp.internal_types[tempc1->raw_type-1];
	  break;
	case IC_SHL_CONST:
	  if (i=OptFixupUnaryOp(tempi,tempi1,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tempi->ic_data=tempi1->ic_data(U64)<<tempi->ic_data(U64);
	      else
		tempi->ic_data=tempi1->ic_data(I64)<<tempi->ic_data(I64);
	      tempi->ic_code=IC_IMM_I64;
	    } else {
	      tempi->ic_data(F64)=tempi1->ic_data(F64)<<tempi->ic_data;
	      tempi->ic_code=IC_IMM_F64;
	    }
	  } else if (tempi1->ic_code==IC_SHL_CONST) {
	    tempi->ic_flags|=tempi1->ic_flags;
	    tempi->ic_data+=tempi1->ic_data;
	    OptSetNOP1(tempi1);
	  }
	  break;
	case IC_SHR_CONST:
	  if (i=OptFixupUnaryOp(tempi,tempi1,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tempi->ic_data=tempi1->ic_data(U64)>>tempi->ic_data(U64);
	      else
		tempi->ic_data=tempi1->ic_data(I64)>>tempi->ic_data(I64);
	      tempi->ic_code=IC_IMM_I64;
	    } else {
	      tempi->ic_data(F64)=tempi1->ic_data(F64)>>tempi->ic_data;
	      tempi->ic_code=IC_IMM_F64;
	    }
	  } else if (tempi1->ic_code==IC_SHR_CONST) {
	    tempi->ic_flags|=tempi1->ic_flags;
	    tempi->ic_data+=tempi1->ic_data;
	    OptSetNOP1(tempi1);
	  }
	  break;
	case IC_SHL:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tempi->ic_data=tempi1->ic_data(U64)<<tempi2->ic_data(U64);
	      else
		tempi->ic_data=tempi1->ic_data(I64)<<tempi2->ic_data(I64);
	      tempi->ic_code=IC_IMM_I64;
	    } else {
	      tempi->ic_data(F64)=tempi1->ic_data(F64) << tempi2->ic_data(F64);
	      tempi->ic_code=IC_IMM_F64;
	    }
	  } else if (tempi2->ic_code==IC_IMM_I64) {
	    tempi->ic_flags|=tempi2->ic_flags;
	    tempi->ic_data=tempi2->ic_data;
	    tempi->ic_code=IC_SHL_CONST;
	    OptSetNOP1(tempi2);
	  }
	  break;
	case IC_SHR:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tempi->ic_data=tempi1->ic_data(U64)>>tempi2->ic_data(U64);
	      else
		tempi->ic_data=tempi1->ic_data(I64)>>tempi2->ic_data(I64);
	      tempi->ic_code=IC_IMM_I64;
	    } else {
	      tempi->ic_data(F64)=tempi1->ic_data(F64) >> tempi2->ic_data(F64);
	      tempi->ic_code=IC_IMM_F64;
	    }
	  } else if (tempi2->ic_code==IC_IMM_I64) {
	    tempi->ic_flags|=tempi2->ic_flags;
	    tempi->ic_data=tempi2->ic_data;
	    tempi->ic_code=IC_SHR_CONST;
	    OptSetNOP1(tempi2);
	  }
	  break;
      sub_switch_end:
	if (!tempi_push->ic_class->ptr_stars_cnt) {
	  if (tempi_push->ic_class->raw_type==RT_F64)
	    tempi_push->ic_class2=cmp.internal_types[RT_F64];
	  else
	    tempi_push->ic_class2=cmp.internal_types[RT_I64];
	}
	break;
      case IC_DEREF:
	if (cc->pass==2) {
	  if (!tempc->size)
	    LexWarn(cc,"Dereference U0 ");
	  if (tempi1->ic_class->raw_type!=RT_F64) {
	    if (tempi1->ic_code==IC__PP) {
	      tempi->ic_code=IC_DEREF_PP;
	      tempi->ic_flags|=tempi1->ic_flags;
	      OptSetNOP1(tempi1);
	    } else if (tempi1->ic_code==IC__MM) {
	      tempi->ic_code=IC_DEREF_MM;
	      tempi->ic_flags|=tempi1->ic_flags;
	      OptSetNOP1(tempi1);
	    }
	  }
	}
	break;
      case IC__PP:
      case IC__MM:
      case IC_PP_:
      case IC_MM_:
	if (cc->pass==2 && !tempc->size)
	  LexWarn(cc,"Dereference U0 ");
	break;
      case IC_POWER:
	tempc=tempi->ic_class=cmp.internal_types[RT_F64];
	if (tempc1->raw_type!=RT_F64)
	  tempi1->ic_flags|=ICF_R_TO_F64;
	if (tempc2->raw_type!=RT_F64)
	  tempi2->ic_flags|=ICF_R_TO_F64;
	tempi_push->ic_class2=cmp.internal_types[RT_F64];
	break;
      sub_switch_start:
	case IC_MUL:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tempi->ic_data=tempi1->ic_data(U64)*tempi2->ic_data(U64);
	      else
		tempi->ic_data=tempi1->ic_data(I64)*tempi2->ic_data(I64);
	      tempi->ic_code=IC_IMM_I64;
	    } else {
	      tempi->ic_data(F64)=tempi1->ic_data(F64)*tempi2->ic_data(F64);
	      tempi->ic_code=IC_IMM_F64;
	    }
	  } else {
	    if (tempi1->ic_code==IC_IMM_I64 && cc->pass==2) {
	      switch (i=tempi1->ic_data) {
		case 0:
		  break;
		case 1:
		  tempi2->ic_flags|=tempi->ic_flags|tempi1->ic_flags;
		  tempi2->ic_class2=tempi->ic_class2;
		  tempi_push=tempi2;
		  OptSetNOP1(tempi1);
		  OptSetNOP1(tempi);
		  break;
		default:
		  if (Bsf(i)==Bsr(i)) {
		    tempi->ic_flags|=tempi1->ic_flags;
		    tempi->t.a1c=tempi->t.a2c;
		    tempi->ic_data=Bsf(i);
		    tempi->ic_code=IC_SHL_CONST;
		    OptSetNOP1(tempi1);
		  }
	      }
	    } else if (tempi2->ic_code==IC_IMM_I64) {
	      switch (i=tempi2->ic_data) {
		case 0:
		  break;
		case 1:
		  tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
		  tempi1->ic_class2=tempi->ic_class2;
		  tempi_push=tempi1;
		  OptSetNOP1(tempi2);
		  OptSetNOP1(tempi);
		  break;
		default:
		  if (Bsf(i)==Bsr(i)) {
		    tempi->ic_flags|=tempi2->ic_flags;
		    tempi->ic_data=Bsf(i);
		    tempi->ic_code=IC_SHL_CONST;
		    OptSetNOP1(tempi2);
		  } else if (tempi1->ic_code==IC_MUL && cc->pass==2) {
		    if (tempi1->t.a1t->ic_code==IC_IMM_I64) {
		      tempi1->ic_flags|=tempi->ic_flags;
		      tempi1->t.a1t->ic_data*=tempi2->ic_data;
		      tempi1->ic_class2=tempi->ic_class2;
		      tempi_push=tempi1;
		      OptSetNOP1(tempi2);
		      OptSetNOP1(tempi);
		    } else if (tempi1->t.a2t->ic_code==IC_IMM_I64) {
		      tempi1->ic_flags|=tempi->ic_flags;
		      tempi1->t.a2t->ic_data*=tempi2->ic_data;
		      tempi1->ic_class2=tempi->ic_class2;
		      tempi_push=tempi1;
		      OptSetNOP1(tempi2);
		      OptSetNOP1(tempi);
		    }
		  }
	      }
	    } else if (tempi2->ic_code==IC_IMM_F64 && cc->pass==2) {
	      if (tempi2->ic_data(F64)==1.0) {
		tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
		tempi1->ic_class2=tempi->ic_class2;
		tempi_push=tempi1;
		OptSetNOP1(tempi2);
		OptSetNOP1(tempi);
	      } else if (tempi1->ic_code==IC_MUL) {
		if (tempi1->t.a1t->ic_code==IC_IMM_F64) {
		  tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
		  tempi1->t.a1t->ic_data(F64)*=tempi2->ic_data(F64);
		  tempi1->ic_class2=tempi->ic_class2;
		  tempi_push=tempi1;
		  OptSetNOP1(tempi2);
		  OptSetNOP1(tempi);
		} else if (tempi1->t.a2t->ic_code==IC_IMM_F64) {
		  tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
		  tempi1->t.a2t->ic_data(F64)*=tempi2->ic_data(F64);
		  tempi1->ic_class2=tempi->ic_class2;
		  tempi_push=tempi1;
		  OptSetNOP1(tempi2);
		  OptSetNOP1(tempi);
		}
	      }
	    }
	  }
	  break;
	case IC_DIV:
	  if ((tempi2->ic_data || tempi2->ic_code!=IC_IMM_I64 &&
		tempi2->ic_code!=IC_IMM_F64) &&
		(i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned))) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tempi->ic_data=tempi1->ic_data(U64)/tempi2->ic_data(U64);
	      else
		tempi->ic_data=tempi1->ic_data(I64)/tempi2->ic_data(I64);
	      tempi->ic_code=IC_IMM_I64;
	    } else {
	      tempi->ic_data(F64)=tempi1->ic_data(F64)/
		    tempi2->ic_data(F64);
	      tempi->ic_code=IC_IMM_F64;
	    }
	  } else {
	    if (tempi2->ic_code==IC_IMM_I64 && (i=tempi2->ic_data)) {
	      if (i==1) {
		tempi1->ic_flags|=tempi2->ic_flags|tempi->ic_flags;
		tempi1->ic_class2=tempi->ic_class2;
		tempi_push=tempi1;
		OptSetNOP1(tempi2);
		OptSetNOP1(tempi);
	      } else if (Bsf(i)==Bsr(i)) {
		tempi->ic_flags|=tempi2->ic_flags;
		tempi->ic_data=Bsf(i);
		tempi->ic_code=IC_SHR_CONST;
		OptSetNOP1(tempi2);
	      }
	    }
	  }
	  break;
	case IC_MOD:
	  if ((tempi2->ic_data || tempi2->ic_code!=IC_IMM_I64 &&
		tempi2->ic_code!=IC_IMM_F64) &&
		(i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned))) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tempi->ic_data=tempi1->ic_data(U64)%tempi2->ic_data(U64);
	      else
		tempi->ic_data=tempi1->ic_data(I64)%tempi2->ic_data(I64);
	      tempi->ic_code=IC_IMM_I64;
	    } else {
	      tempi->ic_data(F64)=tempi1->ic_data(F64)%
		    tempi2->ic_data(F64);
	      tempi->ic_code=IC_IMM_F64;
	    }
	  } else if (cc->pass==2 && tempi2->ic_code==IC_IMM_I64 &&
		(i=tempi2->ic_data) && Bsf(i)==Bsr(i) &&
		tempi_push->ic_class->raw_type!=RT_F64 &&
		tempi_push->ic_class->raw_type&1) {//do only unsigned
	    tempi2->ic_data=i-1;
	    tempi->ic_code=IC_AND;
	  }
	  break;
	case IC_AND:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    tempi->ic_data=tempi1->ic_data&tempi2->ic_data;
	    if (i==FBO1_INT)
	      tempi->ic_code=IC_IMM_I64;
	    else
	      tempi->ic_code=IC_IMM_F64;
	  }
	  break;
	case IC_OR:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    tempi->ic_data=tempi1->ic_data|tempi2->ic_data;
	    if (i==FBO1_INT)
	      tempi->ic_code=IC_IMM_I64;
	    else
	      tempi->ic_code=IC_IMM_F64;
	  }
	  break;
	case IC_XOR:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    tempi->ic_data=tempi1->ic_data^tempi2->ic_data;
	    if (i==FBO1_INT)
	      tempi->ic_code=IC_IMM_I64;
	    else
	      tempi->ic_code=IC_IMM_F64;
	  }
	  break;
	case IC_ADD:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      tempi->ic_data=tempi1->ic_data+tempi2->ic_data;
	      tempi->ic_code=IC_IMM_I64;
	    } else {
	      tempi->ic_data(F64)=tempi1->ic_data(F64)+
		    tempi2->ic_data(F64);
	      tempi->ic_code=IC_IMM_F64;
	    }
	  } else {
	    if (tempi1->ic_code==IC_ABS_ADDR&&tempi2->ic_code==IC_IMM_I64) {
	      tempi->ic_flags|=tempi1->ic_flags|tempi2->ic_flags;
	      tempi->ic_data=tempi1->ic_data+tempi2->ic_data;
	      tempi->ic_code=IC_ABS_ADDR;
	      OptSetNOP1(tempi1);
	      OptSetNOP1(tempi2);
	    } else if (cc->pass==2) {
	      if (tempi1->ic_code==IC_IMM_I64) {
		if (!tempi1->ic_data) {
		  tempi2->ic_flags|=tempi1->ic_flags|tempi->ic_flags;
		  tempi2->ic_class2=tempi->ic_class2;
		  tempi_push=tempi2;
		  OptSetNOP1(tempi1);
		  OptSetNOP1(tempi);
		} else if (tempi2->ic_code==IC_ADD||
		      tempi2->ic_code==IC_SUB) {
		  if (tempi2->t.a1t->ic_code==IC_IMM_I64) {
		    tempi2->ic_flags|=tempi->ic_flags;
		    tempi2->t.a1t->ic_data+=tempi1->ic_data;
		    tempi2->ic_class2=tempi->ic_class2;
		    tempi_push=tempi2;
		    OptSetNOP1(tempi1);
		    OptSetNOP1(tempi);
		  } else if (tempi2->t.a2t->ic_code==IC_IMM_I64) {
		    tempi2->ic_flags|=tempi->ic_flags;
		    if (tempi2->ic_code==IC_ADD)
		      tempi2->t.a2t->ic_data+=tempi1->ic_data;
		    else
		      tempi2->t.a2t->ic_data-=tempi1->ic_data;
		    tempi2->ic_class2=tempi->ic_class2;
		    tempi_push=tempi2;
		    OptSetNOP1(tempi1);
		    OptSetNOP1(tempi);
		  }
		}
	      } else if (tempi2->ic_code==IC_IMM_I64) {
		if (!tempi2->ic_data) {
		  tempi1->ic_flags|=tempi2->ic_flags|tempi->ic_flags;
		  tempi1->ic_class2=tempi->ic_class2;
		  tempi_push=tempi1;
		  OptSetNOP1(tempi2);
		  OptSetNOP1(tempi);
		} else if (tempi1->ic_code==IC_ADD ||
		      tempi1->ic_code==IC_SUB) {
		  if (tempi1->t.a1t->ic_code==IC_IMM_I64) {
		    tempi1->ic_flags|=tempi->ic_flags;
		    tempi1->t.a1t->ic_data+=tempi2->ic_data;
		    tempi1->ic_class2=tempi->ic_class2;
		    tempi_push=tempi1;
		    OptSetNOP1(tempi2);
		    OptSetNOP1(tempi);
		  } else if (tempi1->t.a2t->ic_code==IC_IMM_I64) {
		    tempi1->ic_flags|=tempi->ic_flags;
		    if (tempi1->ic_code==IC_ADD)
		      tempi1->t.a2t->ic_data+=tempi2->ic_data;
		    else
		      tempi1->t.a2t->ic_data-=tempi2->ic_data;
		    tempi1->ic_class2=tempi->ic_class2;
		    tempi_push=tempi1;
		    OptSetNOP1(tempi2);
		    OptSetNOP1(tempi);
		  }
		}
	      } else if (tempi1->ic_code==IC_IMM_F64) {
		if (!tempi1->ic_data) {
		  tempi2->ic_flags|=tempi1->ic_flags|tempi->ic_flags;
		  tempi2->ic_class2=tempi->ic_class2;
		  tempi_push=tempi2;
		  OptSetNOP1(tempi1);
		  OptSetNOP1(tempi);
		} else if (tempi2->ic_code==IC_ADD||
		      tempi2->ic_code==IC_SUB) {
		  if (tempi2->t.a1t->ic_code==IC_IMM_F64) {
		    tempi2->ic_flags|=tempi->ic_flags;
		    tempi2->t.a1t->ic_data(F64)+=tempi1->ic_data(F64);
		    tempi2->ic_class2=tempi->ic_class2;
		    tempi_push=tempi2;
		    OptSetNOP1(tempi1);
		    OptSetNOP1(tempi);
		  } else if (tempi2->t.a2t->ic_code==IC_IMM_F64) {
		    tempi2->ic_flags|=tempi->ic_flags;
		    if (tempi2->ic_code==IC_ADD)
		      tempi2->t.a2t->ic_data(F64)+=tempi1->ic_data(F64);
		    else
		      tempi2->t.a2t->ic_data(F64)-=tempi1->ic_data(F64);
		    tempi2->ic_class2=tempi->ic_class2;
		    tempi_push=tempi2;
		    OptSetNOP1(tempi1);
		    OptSetNOP1(tempi);
		  }
		}
	      } else if (tempi2->ic_code==IC_IMM_F64) {
		if (!tempi2->ic_data) {
		  tempi1->ic_flags|=tempi2->ic_flags|tempi->ic_flags;
		  tempi1->ic_class2=tempi->ic_class2;
		  tempi_push=tempi1;
		  OptSetNOP1(tempi2);
		  OptSetNOP1(tempi);
		} else if (tempi1->ic_code==IC_ADD ||
		      tempi1->ic_code==IC_SUB) {
		  if (tempi1->t.a1t->ic_code==IC_IMM_F64) {
		    tempi1->ic_flags|=tempi->ic_flags;
		    tempi1->t.a1t->ic_data(F64)+=tempi2->ic_data(F64);
		    tempi1->ic_class2=tempi->ic_class2;
		    tempi_push=tempi1;
		    OptSetNOP1(tempi2);
		    OptSetNOP1(tempi);
		  } else if (tempi1->t.a2t->ic_code==IC_IMM_F64) {
		    tempi1->ic_flags|=tempi->ic_flags;
		    if (tempi1->ic_code==IC_ADD)
		      tempi1->t.a2t->ic_data(F64)+=tempi2->ic_data(F64);
		    else
		      tempi1->t.a2t->ic_data(F64)-=tempi2->ic_data(F64);
		    tempi1->ic_class2=tempi->ic_class2;
		    tempi_push=tempi1;
		    OptSetNOP1(tempi2);
		    OptSetNOP1(tempi);
		  }
		}
	      }
	    }
	  }
	  break;
	case IC_SUB:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      tempi->ic_data=tempi1->ic_data-tempi2->ic_data;
	      tempi->ic_code=IC_IMM_I64;
	    } else {
	      tempi->ic_data(F64)=tempi1->ic_data(F64)-
		    tempi2->ic_data(F64);
	      tempi->ic_code=IC_IMM_F64;
	    }
	  } else {
	    if (cc->pass==2) {
	      if (tempi2->ic_code==IC_IMM_I64) {
		if (!tempi2->ic_data) {
		  tempi1->ic_flags|=tempi2->ic_flags|tempi->ic_flags;
		  tempi1->ic_class2=tempi->ic_class2;
		  tempi_push=tempi1;
		  OptSetNOP1(tempi2);
		  OptSetNOP1(tempi);
		} else
		  if (tempi1->ic_code==IC_ADD ||
			tempi1->ic_code==IC_SUB) {
		    if (tempi1->t.a1t->ic_code==IC_IMM_I64) {
		      tempi1->ic_flags|=tempi->ic_flags;
		      tempi1->t.a1t->ic_data-=tempi2->ic_data;
		      tempi1->ic_class2=tempi->ic_class2;
		      tempi_push=tempi1;
		      OptSetNOP1(tempi2);
		      OptSetNOP1(tempi);
		    } else if (tempi1->t.a2t->ic_code==IC_IMM_I64) {
		      tempi1->ic_flags|=tempi->ic_flags;
		      if (tempi1->ic_code==IC_ADD)
			tempi1->t.a2t->ic_data-=tempi2->ic_data;
		      else
			tempi1->t.a2t->ic_data+=tempi2->ic_data;
		      tempi1->ic_class2=tempi->ic_class2;
		      tempi_push=tempi1;
		      OptSetNOP1(tempi2);
		      OptSetNOP1(tempi);
		    }
		  }
	      } else
		if (tempi2->ic_code==IC_IMM_F64) {
		  if (!tempi2->ic_data) {
		    tempi1->ic_flags|=tempi2->ic_flags|tempi->ic_flags;
		    tempi1->ic_class2=tempi->ic_class2;
		    tempi_push=tempi1;
		    OptSetNOP1(tempi2);
		    OptSetNOP1(tempi);
		  } else
		    if (tempi1->ic_code==IC_ADD ||
			  tempi1->ic_code==IC_SUB) {
		      if (tempi1->t.a1t->ic_code==IC_IMM_F64) {
			tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
			tempi1->t.a1t->ic_data(F64)-=tempi2->ic_data(F64);
			tempi1->ic_class2=tempi->ic_class2;
			tempi_push=tempi1;
			OptSetNOP1(tempi2);
			OptSetNOP1(tempi);
		      } else if (tempi1->t.a2t->ic_code==IC_IMM_F64) {
			tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
			if (tempi1->ic_code==IC_ADD)
			  tempi1->t.a2t->ic_data(F64)-=tempi2->ic_data(F64);
			else
			  tempi1->t.a2t->ic_data(F64)+=tempi2->ic_data(F64);
			tempi1->ic_class2=tempi->ic_class2;
			tempi_push=tempi1;
			OptSetNOP1(tempi2);
			OptSetNOP1(tempi);
		      }
		    }
		}
	    }
	  }
	  break;
	case IC_AND_AND:
	  if (OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    tempi->ic_data=tempi1->ic_data&&tempi2->ic_data;
	    tempi->ic_code=IC_IMM_I64;
	  }
	  tempi->ic_class=cmp.internal_types[RT_I64];
	  break;
	case IC_OR_OR:
	  if (OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    tempi->ic_data=tempi1->ic_data||tempi2->ic_data;
	    tempi->ic_code=IC_IMM_I64;
	  }
	  tempi->ic_class=cmp.internal_types[RT_I64];
	  break;
	case IC_XOR_XOR:
	  if (OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    tempi->ic_data=tempi1->ic_data^^tempi2->ic_data;
	    tempi->ic_code=IC_IMM_I64;
	  }
	  tempi->ic_class=cmp.internal_types[RT_I64];
	  break;
      sub_switch_end:
	if (!tempi_push->ic_class->ptr_stars_cnt) {
	  if (tempi_push->ic_class->raw_type==RT_F64)
	    tempi_push->ic_class2=cmp.internal_types[RT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=cmp.internal_types[RT_U64];
	  else
	    tempi_push->ic_class2=cmp.internal_types[RT_I64];
	}
	break;
      sub_switch_start:
	case IC_EQU_EQU:
	  if (OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    tempi->ic_data=tempi1->ic_data==tempi2->ic_data;
	    tempi->ic_code=IC_IMM_I64;
	  } else
	    if (tempi->ic_class->raw_type==RT_F64)
	      tempi->ic_flags|=ICF_USE_F64;
	  break;
	case IC_NOT_EQU:
	  if (OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    tempi->ic_data=tempi1->ic_data!=tempi2->ic_data;
	    tempi->ic_code=IC_IMM_I64;
	  } else
	    if (tempi->ic_class->raw_type==RT_F64)
	      tempi->ic_flags|=ICF_USE_F64;
	  break;
	case IC_LESS:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tempi->ic_data=tempi1->ic_data(U64)<tempi2->ic_data(U64);
	      else
		tempi->ic_data=tempi1->ic_data(I64)<tempi2->ic_data(I64);
	    } else
	      tempi->ic_data=tempi1->ic_data(F64)<tempi2->ic_data(F64);
	    tempi->ic_code=IC_IMM_I64;
	  } else {
	    if (is_unsigned)
	      tempi->ic_flags|=ICF_USE_UNSIGNED;
	    if (tempi->ic_class->raw_type==RT_F64)
	      tempi->ic_flags|=ICF_USE_F64;
	  }
	  break;
	case IC_GREATER_EQU:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tempi->ic_data=tempi1->ic_data(U64)>=tempi2->ic_data(U64);
	      else
		tempi->ic_data=tempi1->ic_data(I64)>=tempi2->ic_data(I64);
	    } else
	      tempi->ic_data=tempi1->ic_data(F64)>=tempi2->ic_data(F64);
	    tempi->ic_code=IC_IMM_I64;
	  } else {
	    if (is_unsigned)
	      tempi->ic_flags|=ICF_USE_UNSIGNED;
	    if (tempi->ic_class->raw_type==RT_F64)
	      tempi->ic_flags|=ICF_USE_F64;
	  }
	  break;
	case IC_GREATER:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tempi->ic_data=tempi1->ic_data(U64)>tempi2->ic_data(U64);
	      else
		tempi->ic_data=tempi1->ic_data(I64)>tempi2->ic_data(I64);
	    } else
	      tempi->ic_data=tempi1->ic_data(F64)>tempi2->ic_data(F64);
	    tempi->ic_code=IC_IMM_I64;
	  } else {
	    if (is_unsigned)
	      tempi->ic_flags|=ICF_USE_UNSIGNED;
	    if (tempi->ic_class->raw_type==RT_F64)
	      tempi->ic_flags|=ICF_USE_F64;
	  }
	  break;
	case IC_LESS_EQU:
	  if (i=OptFixupBinaryOp1(tempi,tempi1,tempi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tempi->ic_data=tempi1->ic_data(U64)<=tempi2->ic_data(U64);
	      else
		tempi->ic_data=tempi1->ic_data(I64)<=tempi2->ic_data(I64);
	    } else
	      tempi->ic_data=tempi1->ic_data(F64)<=tempi2->ic_data(F64);
	    tempi->ic_code=IC_IMM_I64;
	  } else {
	    if (is_unsigned)
	      tempi->ic_flags|=ICF_USE_UNSIGNED;
	    if (tempi->ic_class->raw_type==RT_F64)
	      tempi->ic_flags|=ICF_USE_F64;
	  }
	  break;
      sub_switch_end:
	tempi->ic_flags&=~ICF_R_TO_INT;
	if (!tempi_push->ic_class->ptr_stars_cnt) {
	  if (tempi_push->ic_class->raw_type==RT_F64)
	    tempi_push->ic_class2=cmp.internal_types[RT_F64];
	  else if (is_unsigned)
	    tempi_push->ic_class2=cmp.internal_types[RT_U64];
	  else
	    tempi_push->ic_class2=cmp.internal_types[RT_I64];
	}
	if (tempi_push->ic_flags & ICF_PUSH_CMP)
	  tempi->ic_class=tempi->ic_class2;
	else
	  tempi->ic_class=cmp.internal_types[RT_I64];
	break;
      sub_switch_start:
	if (cc->pass==2 && (!tempc->size||!tempc2->size))
	  LexWarn(cc,"Assign U0 ");
	sub_switch_start:
	  case IC_MUL_EQU:
	    if (tempi2->ic_code==IC_IMM_I64 &&
		  tempc->raw_type!=RT_F64 &&
		  tempc2->raw_type!=RT_F64) {
	      if (i=tempi2->ic_data) {
		if (Bsf(i)==Bsr(i)) {
		  tempi2->ic_data=Bsf(i);
		  tempi->ic_code=IC_SHL_EQU;
		}
	      }
	    }
	    break;
	  case IC_DIV_EQU:
	    if (tempi2->ic_code==IC_IMM_I64 &&
		  tempc->raw_type!=RT_F64 &&
		  tempc2->raw_type!=RT_F64 &&
		  (i=tempi2->ic_data) && Bsf(i)==Bsr(i)) {
	      tempi2->ic_data=Bsf(i);
	      tempi->ic_code=IC_SHR_EQU;
	    }
	    break;
	  case IC_MOD_EQU:
	    if (tempi2->ic_code==IC_IMM_I64 &&
		  tempc->raw_type!=RT_F64 &&
		  tempc2->raw_type!=RT_F64 &&
		  (i=tempi2->ic_data) && Bsf(i)==Bsr(i)) {
	      tempi2->ic_data=i-1;
	      tempi->ic_code=IC_AND_EQU;
	    }
	    break;
	  case IC_ADD_EQU:
	  case IC_SUB_EQU:
	    break;
	sub_switch_end:
	  if (tempi2->ic_class->raw_type==RT_F64)
	    tempi->ic_flags=tempi->ic_flags|ICF_USE_F64;
	  if (tempc->raw_type==RT_F64) {
	    if (tempc2->raw_type!=RT_F64)
	      tempi2->ic_flags|=ICF_R_TO_F64;
	  }
	  break;
	case IC_ASSIGN:
	  if (tempc->raw_type==RT_F64) {
	    if (tempc2->raw_type!=RT_F64)
	      tempi2->ic_flags|=ICF_R_TO_F64;
	  } else {
	    if (tempc2->raw_type==RT_F64)
	      tempi2->ic_flags|=ICF_R_TO_INT;
	  }
	  if (cc->pass==2 && tempi1->ic_class->raw_type!=RT_F64) {
	    if (tempi1->ic_code==IC__PP) {
	      tempi->ic_code=IC_ASSIGN_PP;
	      tempi->ic_flags|=tempi1->ic_flags;
	      tempi->t.class2=tempi1->ic_class;
	      OptSetNOP1(tempi1);
	    } else if (tempi1->ic_code==IC__MM) {
	      tempi->ic_code=IC_ASSIGN_MM;
	      tempi->ic_flags|=tempi1->ic_flags;
	      tempi->t.class2=tempi1->ic_class;
	      OptSetNOP1(tempi1);
	    }
	  }
	  break;
	case IC_SHL_EQU:
	case IC_SHR_EQU:
	case IC_AND_EQU:
	case IC_OR_EQU:
	case IC_XOR_EQU:
	  if (tempc2->raw_type==RT_F64)
	    tempi2->ic_flags|=ICF_R_TO_INT;
	  break;
      sub_switch_end:
	break;
      case IC_ENTER:
      case IC_LEAVE:
	tempi->ic_data=-cc->htc.fun->size;
	break;
      case IC_ADD_RSP:
	if (tempi1=OptLag(tempi)) {
	  if (tempi1->ic_code==IC_ADD_RSP) {
	    tempi->ic_data+=tempi1->ic_data;
	    tempi->ic_flags|=tempi1->ic_flags;
	    OptSetNOP1(tempi1);
	  }
	}
      case IC_ADD_RSP1:
	break;
      case IC_BSF:
	if (tempi1->ic_code==IC_IMM_I64) {
	  tempi1->ic_data=Bsf(tempi1->ic_data);
	  tempi_push=tempi1;
	  OptSetNOP1(OptLag(tempi1));	//CALL_START
	  tempi2=OptLead1(tempi);
	  tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
	  OptSetNOP1(tempi2);		//CALL_END
	  OptSetNOP1(tempi);		//BSF
	}
	break;
      case IC_BSR:
	if (tempi1->ic_code==IC_IMM_I64) {
	  tempi1->ic_data=Bsr(tempi1->ic_data);
	  tempi_push=tempi1;
	  OptSetNOP1(OptLag(tempi1));	//CALL_START
	  tempi2=OptLead1(tempi);
	  tempi1->ic_flags|=tempi->ic_flags|tempi2->ic_flags;
	  OptSetNOP1(tempi2);		//CALL_END
	  OptSetNOP1(tempi);		//BSR
	}
	break;
      case IC_LBTS:
      case IC_LBTR:
      case IC_LBTC:
	tempi->ic_flags|=ICF_LOCK;
	break;
      case IC_TO_I64:
	if (tempi1->ic_code==IC_IMM_F64) {
	  tempi2=tempi1->last;
	  while (tempi2->ic_code!=IC_CALL_START)
	    tempi2=tempi2->last;
	  OptSetNOP1(tempi2);

	  tempi2=tempi->next;
	  while (tempi2->ic_code!=IC_CALL_END)
	    tempi2=tempi2->next;

	  tempi->ic_flags|=tempi1->ic_flags&~ICF_R_TO_INT|tempi2->ic_flags;
	  OptSetNOP1(tempi2);

	  tempi->ic_code=IC_IMM_I64;
	  tempi->ic_data=ToI64(tempi1->ic_data(F64));
	  tempi->ic_class=cmp.internal_types[RT_I64];
	  tempi->ic_class2=cmp.internal_types[RT_I64];
	  OptSetNOP1(tempi1);
	} else if (tempi1->ic_code==IC_IMM_I64) {
	  tempi2=tempi1->last;
	  while (tempi2->ic_code!=IC_CALL_START)
	    tempi2=tempi2->last;
	  OptSetNOP1(tempi2);

	  tempi2=tempi->next;
	  while (tempi2->ic_code!=IC_CALL_END)
	    tempi2=tempi2->next;

	  tempi->ic_flags|=tempi1->ic_flags&~ICF_R_TO_F64|tempi2->ic_flags;
	  OptSetNOP1(tempi2);

	  tempi->ic_code=IC_IMM_I64;
	  tempi->ic_data=tempi1->ic_data;
	  tempi->ic_class=cmp.internal_types[RT_I64];
	  tempi->ic_class2=cmp.internal_types[RT_I64];
	  OptSetNOP1(tempi1);
	}
	if (tempi1->ic_flags&ICF_R_TO_F64) {
	  i=0;
	  tempi2=tempi1->last;
	  while (TRUE) {
	    if (tempi2->ic_code==IC_CALL_START) {
	      if (!i) break;
	      i--;
	    } else if (tempi2->ic_code==IC_CALL_END)
	      i++;
	    tempi2=tempi2->last;
	  }
	  OptSetNOP1(tempi2);

	  tempi2=tempi1->next;
	  while (tempi2->ic_code!=IC_CALL_END)
	    tempi2=tempi2->next;

	  tempi1->ic_flags=tempi->ic_flags|tempi1->ic_flags&
		~(ICF_R_TO_F64|ICF_PUSH_RESULT)|tempi2->ic_flags;
	  OptSetNOP1(tempi2);

	  tempi1->ic_class=cmp.internal_types[RT_I64];
	  tempi1->ic_class2=cmp.internal_types[RT_I64];
	  tempi_push=tempi1;
	  OptSetNOP1(tempi);
	}
	break;
      case IC_TO_F64:
	if (tempi1->ic_code==IC_IMM_I64) {
	  tempi2=tempi1->last;
	  while (tempi2->ic_code!=IC_CALL_START)
	    tempi2=tempi2->last;
	  OptSetNOP1(tempi2);

	  tempi2=tempi->next;
	  while (tempi2->ic_code!=IC_CALL_END)
	    tempi2=tempi2->next;

	  tempi->ic_flags|=tempi1->ic_flags&~ICF_R_TO_F64|tempi2->ic_flags;
	  OptSetNOP1(tempi2);

	  tempi->ic_code=IC_IMM_F64;
	  tempi->ic_data(F64)=ToF64(tempi1->ic_data);
	  tempi->ic_class=cmp.internal_types[RT_F64];
	  tempi->ic_class2=cmp.internal_types[RT_F64];
	  OptSetNOP1(tempi1);
	} else if (tempi1->ic_code==IC_IMM_F64) {
	  tempi2=tempi1->last;
	  while (tempi2->ic_code!=IC_CALL_START)
	    tempi2=tempi2->last;
	  OptSetNOP1(tempi2);

	  tempi2=tempi->next;
	  while (tempi2->ic_code!=IC_CALL_END)
	    tempi2=tempi2->next;

	  tempi->ic_flags|=tempi1->ic_flags&~ICF_R_TO_INT|tempi2->ic_flags;
	  OptSetNOP1(tempi2);

	  tempi->ic_code=IC_IMM_F64;
	  tempi->ic_data=tempi1->ic_data;
	  tempi->ic_class=cmp.internal_types[RT_F64];
	  tempi->ic_class2=cmp.internal_types[RT_F64];
	  OptSetNOP1(tempi1);
	}
	if (tempi1->ic_flags&ICF_R_TO_INT) {
	  i=0;
	  tempi2=tempi1->last;
	  while (TRUE) {
	    if (tempi2->ic_code==IC_CALL_START) {
	      if (!i) break;
	      i--;
	    } else if (tempi2->ic_code==IC_CALL_END)
	      i++;
	    tempi2=tempi2->last;
	  }
	  OptSetNOP1(tempi2);

	  tempi2=tempi1->next;
	  while (tempi2->ic_code!=IC_CALL_END)
	    tempi2=tempi2->next;

	  tempi1->ic_flags=tempi->ic_flags|tempi1->ic_flags&
		~(ICF_R_TO_INT|ICF_PUSH_RESULT)|tempi2->ic_flags;
	  OptSetNOP1(tempi2);

	  tempi1->ic_class=cmp.internal_types[RT_F64];
	  tempi1->ic_class2=cmp.internal_types[RT_F64];
	  tempi_push=tempi1;
	  OptSetNOP1(tempi);
	}
	break;
      case IC_TO_BOOL:
	if (tempi1->ic_code==IC_IMM_I64 || tempi1->ic_code==IC_IMM_F64) {
	  tempi2=tempi1->last;
	  while (tempi2->ic_code!=IC_CALL_START)
	    tempi2=tempi2->last;
	  OptSetNOP1(tempi2);

	  tempi2=tempi->next;
	  while (tempi2->ic_code!=IC_CALL_END)
	    tempi2=tempi2->next;

	  tempi->ic_flags|=tempi1->ic_flags&~ICF_R_TO_F64|tempi2->ic_flags;
	  OptSetNOP1(tempi2);

	  tempi->ic_code=IC_IMM_I64;
	  tempi->ic_data=ToBool(tempi1->ic_data);
	  tempi->ic_class=cmp.internal_types[RT_I64];
	  tempi->ic_class2=cmp.internal_types[RT_I64];
	  OptSetNOP1(tempi1);
	}
	break;
      case IC_BR_ZERO:
	tempi_push=OptBrZero(cc,tempi);
	break;
      case IC_BR_NOT_ZERO:
	tempi_push=OptBrNotZero(cc,tempi);
	break;
      case IC_NOP1:
	if (tempi->ic_flags&ICF_PUSH_RESULT) {
	  tempi1=tempi;
	  do tempi1=tempi1->last;
	  while (tempi1->ic_code==IC_NOP1);
	  tempi1->ic_flags|=ICF_PUSH_RESULT;
	  tempi->ic_flags&=~ICF_PUSH_RESULT;
	}
	break;
      case IC_NOP2:
	ps->ptr+=tempi->ic_data<<1;
	break;
      case IC_LABEL:
	lb=OptLabelFwd(tempi->ic_data);
	lb1=tempi->ic_data;
	while (lb2=lb1->fwd) {
	  lb1->fwd=lb;
	  lb1=lb2;
	}
	if (tempi1=OptLag(tempi)) {
	  if (tempi1->ic_code==IC_JMP) {
	    lb1=tempi1->ic_data;
	    while (lb1->fwd)
	      lb1=lb1->fwd;
	    if (lb1==lb) {
	      tempi->ic_flags|=tempi1->ic_flags;
	      OptSetNOP1(tempi1);
	    }
	  } else if (tempi1->ic_code==IC_LABEL) {
	    lb1=tempi1->ic_data;
	    if (!lb1->fwd)
	      lb1->fwd=lb;
	    if (tempi1=OptLag(tempi1)) {
	      if (tempi1->ic_code==IC_JMP) {
		lb1=tempi1->ic_data;
		while (lb1->fwd)
		  lb1=lb1->fwd;
		if (lb1==lb) {
		  tempi->ic_flags|=tempi1->ic_flags;
		  OptSetNOP1(tempi1);
		}
	      }
	    }
	  }
	}
	break;
      case IC_JMP:
	if (tempi1=OptLag(tempi)) {
	  if (tempi1->ic_code==IC_LABEL) {
	    lb=OptLabelFwd(tempi->ic_data);
	    lb1=OptLabelFwd(tempi1->ic_data);
	    if (lb!=lb1)
	      lb1->fwd=lb;
	  }
	}
	break;
      case IC_STR_CONST:
      case IC_RBP:
      case IC_MOV_FS:
      case IC_MOV_GS:
      case IC_RIP:
      case IC_SIZEOF:
      case IC_SQR:
      case IC_ABS:
      case IC_SQRT:
      case IC_SIN:
      case IC_COS:
      case IC_TAN:
      case IC_ATAN:
      case IC_BR_CARRY:
      case IC_BR_NOT_CARRY:
      case IC_BR_EQU_EQU ...IC_BR_LESS_EQU:
      case IC_BR_EQU_EQU2...IC_BR_LESS_EQU2:
      case IC_BR_OR_OR_NOT_ZERO:
      case IC_BR_OR_OR_ZERO:
      case IC_BR_AND_AND_NOT_ZERO:
      case IC_BR_AND_AND_ZERO:
      case IC_BR_AND_NOT_ZERO:
      case IC_BR_AND_ZERO:
      case IC_BR_MM_NOT_ZERO:
      case IC_BR_MM_ZERO:
      case IC_BR_BT:
      case IC_BR_BTS:
      case IC_BR_BTR:
      case IC_BR_BTC:
      case IC_BR_NOT_BT:
      case IC_BR_NOT_BTS:
      case IC_BR_NOT_BTR:
      case IC_BR_NOT_BTC:
      case IC_END:
      case IC_ADDR:
      case IC_RET:
      case IC_END_EXP:
      case IC_CALL_START:
      case IC_CALL_END:
      case IC_CALL_END2:
      case IC_PUSH_REGS:
      case IC_POP_REGS:
      case IC_SUB_CALL:
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
      case IC_GET_RBP:
      case IC_GET_RSP:
      case IC_GET_RAX:
      case IC_RETURN_VAL:
      case IC_RETURN_VAL2:
      case IC_ABS_ADDR:
      case IC_HEAP_GLBL:
      case IC_ADDR_IMPORT:
      case IC_GET_LABEL:
      case IC_TYPE:
      case IC_RDTSC:
      case IC_HPET:
      case IC_SET_RFLAGS:
      case IC_SET_RBP:
      case IC_SET_RSP:
      case IC_SET_RAX:
      case IC_SIGN_I64:
      case IC_TOUPPER:
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
      case IC_IN_U32:
      case IC_IN_U16:
      case IC_IN_U8:
      case IC_STRLEN:
      case IC_BT:
      case IC_BTS:
      case IC_BTR:
      case IC_BTC:
      case IC_QUE_INIT:
      case IC_QUE_REM:
      case IC_QUE_INS:
      case IC_QUE_INS_REV:
      case IC_OUT_U32:
      case IC_OUT_U16:
      case IC_OUT_U8:
      case IC_NOBOUND_SWITCH:
      case IC_SWITCH:
      case IC_ASM:
	break;
      default:
	"Pass:%d Missing IC handler\n",cc->pass;
	ICPut(cc,tempi);
	LexExcept(cc,"Compiler Optimization Error at ");
    }
    if (intermediate_code_table[code].arg_cnt==IS_2_ARG) {
      if (tempi_push->ic_precedence.u8[0]&~ASSOC_MASK==PREC_ASSIGN)
	OptFixSizeOf(tempi2,tempi_push,tempi1->ic_class-1);
      else {
	OptFixSizeOf(tempi1,tempi_push,tempi2->ic_class);
	OptFixSizeOf(tempi2,tempi_push,tempi1->ic_class);
      }
    }
    if (intermediate_code_table[tempi_push->ic_code].result_cnt) {
      PrsPush(ps,tempi->ic_class2);
      PrsPush(ps,tempi_push);
    }
    if (tempi->ic_class) {
      if (tempi->ic_class->raw_type==RT_F64)
	tempi->ic_flags&=~ICF_R_TO_F64;
      else
	tempi->ic_flags&=~ICF_R_TO_INT;
      if (code>IC_END_EXP)
	last_with_class=tempi;
    }
    tempi=tempi->next;
  }
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",cc->pass,ps->ptr;
    LexExcept(cc,"Compiler Optimization Error at ");
  }
/*This is for determining type conversions
for passing args to funs.
*/
  return last_with_class;
}

