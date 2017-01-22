/*OptPass012

Pass#0
When parsing the arg expressions to a function
call, there is a call to OptPass012 to determine
the type of the expression. $LK,"OptPass012",A="FF:::/Compiler/PrsExp.HC,OptPass012"$

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
  CHashClass *tmpc,*tmpc1,*tmpc2;
  CIntermediateCode *tmpi,*tmpi1,*tmpi2,*tmpi3,*tmpi_push,
	*last_with_class=NULL;
  CCodeMisc *lb,*lb1,*lb2;
  CPrsStk *ps;
  if (!(ps=cc->ps))
    ps=cc->ps=MAlloc(sizeof(CPrsStk));
  ps->ptr=0;
  ps->ptr2=0;
  tmpi=cc->coc.coc_head.next;
  while (code=tmpi->ic_code) {
    tmpc=tmpi->ic_class;
    tmpi->ic_class2=tmpc;
    tmpi_push=tmpi;
    MemSet(&tmpi->arg1,0,3*sizeof(CICArg));
    tmpi->arg1_type_pointed_to=0;
    switch [intermediate_code_table[code].arg_cnt] {
      case IS_V_ARG:
	ps->ptr-=tmpi->ic_data>>2;
	break;
      case IS_2_ARG:
	tmpi2=PrsPop(ps);
	tmpc2=tmpi2->ic_class;
	tmpi->t.arg2_tree=tmpi2;
	tmpi->t.arg2_class=PrsPop(ps);
      case IS_1_ARG:
	tmpi1=PrsPop(ps);
	tmpc1=tmpi1->ic_class;
	tmpi->t.arg1_tree=tmpi1;
	tmpi->t.arg1_class=PrsPop(ps);
	break;
      case IS_0_ARG: //nobound switch
	break;
    }
    if (intermediate_code_table[code].not_const)
      cc->flags|=CCF_NOT_CONST;
    switch [code] {
      case IC_IMM_F64:
	tmpi->ic_flags&=~ICF_RES_TO_F64;
	if (cc->pass==2 && tmpi->ic_flags&ICF_RES_TO_INT) {
	  tmpi->ic_data=ToI64(tmpi->ic_data(F64));
	  tmpi->ic_flags&=~ICF_RES_TO_INT;
	  tmpi->ic_code=IC_IMM_I64;
	  tmpi->ic_class=cmp.internal_types[RT_I64];
	}
	break;
      case IC_IMM_I64:
	tmpi->ic_flags&=~ICF_RES_TO_INT;
	if (cc->pass==2 && tmpi->ic_flags&ICF_RES_TO_F64) {
	  tmpi->ic_data(F64)=ToF64(tmpi->ic_data);
	  tmpi->ic_flags&=~ICF_RES_TO_F64;
	  tmpi->ic_code=IC_IMM_F64;
	  tmpi->ic_class=cmp.internal_types[RT_F64];
	}
	break;
      case IC_HOLYC_TYPECAST:
	if (tmpi1->ic_code==IC_IMM_I64 || tmpi1->ic_code==IC_IMM_F64) {
	  if (tmpi->ic_class->raw_type==RT_F64)
	    tmpi1->ic_code=IC_IMM_F64;
	  else
	    tmpi1->ic_code=IC_IMM_I64;
	  tmpi1->ic_class=tmpi->ic_class;
	  tmpi1->ic_flags|=tmpi->ic_flags;
	  tmpi_push=tmpi1;
	  OptSetNOP1(tmpi);
	} else {
	  if (tmpi->ic_data) {//was paren
	    if (!tmpi_push->ic_class->ptr_stars_cnt) {
	      if (tmpi_push->ic_class->raw_type==RT_F64)
		tmpi_push->ic_class2=cmp.internal_types[RT_F64];
	      else
		tmpi_push->ic_class2=cmp.internal_types[RT_I64];
	    }
	  } else {
	    tmpi1->ic_class=tmpi->ic_class;
	    tmpi1->ic_flags|=tmpi->ic_flags;
	    tmpi_push=tmpi1;
	    OptSetNOP1(tmpi);
	  }
	}
	break;
      case IC_FS:
      case IC_GS:
//CALL,FS/GS,CALL_END,IMM,ADD,DEREF-->MOV_FS/GS
	tmpi1=tmpi->next->next; //IMM
	tmpi2=tmpi1->next; //ADD
	tmpi3=tmpi2->next; //DEREF
	if (tmpi1->ic_code==IC_IMM_I64 && tmpi2->ic_code==IC_ADD &&
	      tmpi3->ic_code==IC_DEREF &&
	      !(tmpi3->ic_flags&~ICG_NO_CVT_MASK)) {
	  tmpi->ic_flags|=tmpi1->ic_flags|tmpi2->ic_flags|tmpi3->ic_flags;
	  if (tmpi->ic_code==IC_FS)
	    tmpi->ic_code=IC_MOV_FS;
	  else
	    tmpi->ic_code=IC_MOV_GS;
	  tmpi->ic_data=tmpi1->ic_data;
	  tmpi->ic_class =tmpi3->ic_class;
	  tmpi->ic_class2=tmpi3->ic_class2;
	  OptSetNOP1(tmpi1);
	  OptSetNOP1(tmpi2);
	  OptSetNOP1(tmpi3);

	  tmpi1=tmpi->last; //CALL
	  tmpi2=tmpi->next; //CALL_END
	  tmpi->ic_flags|=tmpi1->ic_flags|tmpi2->ic_flags;
	  OptSetNOP1(tmpi1);
	  OptSetNOP1(tmpi2);
	}
	break;
      case IC_PUSH_CMP:
	if (tmpi1=OptLag(tmpi)) {
	  if (tmpi1->ic_code==IC_AND_AND)
	    tmpi1=OptLag(tmpi1);
	  if (tmpi1)
	    tmpi->ic_class=tmpi1->ic_class;
	}
	tmpi->ic_class2=tmpi->ic_class;
	tmpi->ic_data=0;
	if (tmpi->ic_class->raw_type==RT_F64)
	  tmpi->ic_flags|=ICF_USE_F64;
	break;
      case IC_COM:
	if (tmpi1->ic_code==IC_IMM_I64) {
	  tmpi->ic_data=~tmpi1->ic_data;
	  tmpi->ic_code=IC_IMM_I64;
	  tmpi->ic_flags|=tmpi1->ic_flags;
	  OptSetNOP1(tmpi1);
	}
	tmpi_push->ic_class2=cmp.internal_types[RT_I64];
	break;
      start:
	case IC_NOT:
	  if (tmpc->raw_type==RT_F64) {
	    if (tmpi1->ic_code==IC_IMM_F64) {
	      tmpi->ic_data(F64)=!tmpi1->ic_data(F64);
	      tmpi->ic_code=IC_IMM_F64;
	      tmpi->ic_flags|=tmpi1->ic_flags;
	      OptSetNOP1(tmpi1);
	    }
	    break;
	  }
	  if (tmpi1->ic_code==IC_IMM_I64) {
	    tmpi->ic_data=!tmpi1->ic_data;
	    tmpi->ic_code=IC_IMM_I64;
	    tmpi->ic_flags|=tmpi1->ic_flags;
	    OptSetNOP1(tmpi1);
	  }
	  break;
	case IC_UNARY_MINUS:
	  if (i=OptFixupUnaryOp(tmpi,tmpi1,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      tmpi->ic_data=-tmpi1->ic_data(I64);
	      tmpi->ic_code=IC_IMM_I64;
	    } else {
	      tmpi->ic_data(F64)=-tmpi1->ic_data(F64);
	      tmpi->ic_code=IC_IMM_F64;
	    }
	  }
	  if (tmpc1->type&HTT_INTERNAL_TYPE && tmpc1->raw_type&RTF_UNSIGNED)
	    tmpi->ic_class=cmp.internal_types[tmpc1->raw_type-1];
	  break;
	case IC_SHL_CONST:
	  if (i=OptFixupUnaryOp(tmpi,tmpi1,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tmpi->ic_data=tmpi1->ic_data(U64)<<tmpi->ic_data(U64);
	      else
		tmpi->ic_data=tmpi1->ic_data(I64)<<tmpi->ic_data(I64);
	      tmpi->ic_code=IC_IMM_I64;
	    } else {
	      tmpi->ic_data(F64)=tmpi1->ic_data(F64)<<tmpi->ic_data;
	      tmpi->ic_code=IC_IMM_F64;
	    }
	  } else if (tmpi1->ic_code==IC_SHL_CONST) {
	    tmpi->ic_flags|=tmpi1->ic_flags;
	    tmpi->ic_data+=tmpi1->ic_data;
	    OptSetNOP1(tmpi1);
	  }
	  break;
	case IC_SHR_CONST:
	  if (i=OptFixupUnaryOp(tmpi,tmpi1,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tmpi->ic_data=tmpi1->ic_data(U64)>>tmpi->ic_data(U64);
	      else
		tmpi->ic_data=tmpi1->ic_data(I64)>>tmpi->ic_data(I64);
	      tmpi->ic_code=IC_IMM_I64;
	    } else {
	      tmpi->ic_data(F64)=tmpi1->ic_data(F64)>>tmpi->ic_data;
	      tmpi->ic_code=IC_IMM_F64;
	    }
	  } else if (tmpi1->ic_code==IC_SHR_CONST) {
	    tmpi->ic_flags|=tmpi1->ic_flags;
	    tmpi->ic_data+=tmpi1->ic_data;
	    OptSetNOP1(tmpi1);
	  }
	  break;
	case IC_SHL:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tmpi->ic_data=tmpi1->ic_data(U64)<<tmpi2->ic_data(U64);
	      else
		tmpi->ic_data=tmpi1->ic_data(I64)<<tmpi2->ic_data(I64);
	      tmpi->ic_code=IC_IMM_I64;
	    } else {
	      tmpi->ic_data(F64)=tmpi1->ic_data(F64) << tmpi2->ic_data(F64);
	      tmpi->ic_code=IC_IMM_F64;
	    }
	  } else if (tmpi2->ic_code==IC_IMM_I64) {
	    tmpi->ic_flags|=tmpi2->ic_flags;
	    tmpi->ic_data=tmpi2->ic_data;
	    tmpi->ic_code=IC_SHL_CONST;
	    OptSetNOP1(tmpi2);
	  }
	  break;
	case IC_SHR:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tmpi->ic_data=tmpi1->ic_data(U64)>>tmpi2->ic_data(U64);
	      else
		tmpi->ic_data=tmpi1->ic_data(I64)>>tmpi2->ic_data(I64);
	      tmpi->ic_code=IC_IMM_I64;
	    } else {
	      tmpi->ic_data(F64)=tmpi1->ic_data(F64) >> tmpi2->ic_data(F64);
	      tmpi->ic_code=IC_IMM_F64;
	    }
	  } else if (tmpi2->ic_code==IC_IMM_I64) {
	    tmpi->ic_flags|=tmpi2->ic_flags;
	    tmpi->ic_data=tmpi2->ic_data;
	    tmpi->ic_code=IC_SHR_CONST;
	    OptSetNOP1(tmpi2);
	  }
	  break;
      end:
	if (!tmpi_push->ic_class->ptr_stars_cnt) {
	  if (tmpi_push->ic_class->raw_type==RT_F64)
	    tmpi_push->ic_class2=cmp.internal_types[RT_F64];
	  else
	    tmpi_push->ic_class2=cmp.internal_types[RT_I64];
	}
	break;
      case IC_DEREF:
	if (cc->pass==2) {
	  if (!tmpc->size)
	    LexWarn(cc,"Dereference U0 ");
	  if (tmpi1->ic_class->raw_type!=RT_F64) {
	    if (tmpi1->ic_code==IC__PP) {
	      tmpi->ic_code=IC_DEREF_PP;
	      tmpi->ic_flags|=tmpi1->ic_flags;
	      OptSetNOP1(tmpi1);
	    } else if (tmpi1->ic_code==IC__MM) {
	      tmpi->ic_code=IC_DEREF_MM;
	      tmpi->ic_flags|=tmpi1->ic_flags;
	      OptSetNOP1(tmpi1);
	    }
	  }
	}
	break;
      case IC__PP:
      case IC__MM:
      case IC_PP_:
      case IC_MM_:
	if (cc->pass==2 && !tmpc->size)
	  LexWarn(cc,"Dereference U0 ");
	break;
      case IC_POWER:
	tmpc=tmpi->ic_class=cmp.internal_types[RT_F64];
	if (tmpc1->raw_type!=RT_F64)
	  tmpi1->ic_flags|=ICF_RES_TO_F64;
	if (tmpc2->raw_type!=RT_F64)
	  tmpi2->ic_flags|=ICF_RES_TO_F64;
	tmpi_push->ic_class2=cmp.internal_types[RT_F64];
	break;
      start:
	case IC_MUL:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tmpi->ic_data=tmpi1->ic_data(U64)*tmpi2->ic_data(U64);
	      else
		tmpi->ic_data=tmpi1->ic_data(I64)*tmpi2->ic_data(I64);
	      tmpi->ic_code=IC_IMM_I64;
	    } else {
	      tmpi->ic_data(F64)=tmpi1->ic_data(F64)*tmpi2->ic_data(F64);
	      tmpi->ic_code=IC_IMM_F64;
	    }
	  } else {
	    if (tmpi1->ic_code==IC_IMM_I64 && cc->pass==2) {
	      switch (i=tmpi1->ic_data) {
		case 0:
		  break;
		case 1:
		  tmpi2->ic_flags|=tmpi->ic_flags|tmpi1->ic_flags;
		  tmpi2->ic_class2=tmpi->ic_class2;
		  tmpi_push=tmpi2;
		  OptSetNOP1(tmpi1);
		  OptSetNOP1(tmpi);
		  break;
		default:
		  if (Bsf(i)==Bsr(i)) {
		    tmpi->ic_flags|=tmpi1->ic_flags;
		    tmpi->t.arg1_class=tmpi->t.arg2_class;
		    tmpi->ic_data=Bsf(i);
		    tmpi->ic_code=IC_SHL_CONST;
		    OptSetNOP1(tmpi1);
		  }
	      }
	    } else if (tmpi2->ic_code==IC_IMM_I64) {
	      switch (i=tmpi2->ic_data) {
		case 0:
		  break;
		case 1:
		  tmpi1->ic_flags|=tmpi->ic_flags|tmpi2->ic_flags;
		  tmpi1->ic_class2=tmpi->ic_class2;
		  tmpi_push=tmpi1;
		  OptSetNOP1(tmpi2);
		  OptSetNOP1(tmpi);
		  break;
		default:
		  if (Bsf(i)==Bsr(i)) {
		    tmpi->ic_flags|=tmpi2->ic_flags;
		    tmpi->ic_data=Bsf(i);
		    tmpi->ic_code=IC_SHL_CONST;
		    OptSetNOP1(tmpi2);
		  } else if (tmpi1->ic_code==IC_MUL && cc->pass==2) {
		    if (tmpi1->t.arg1_tree->ic_code==IC_IMM_I64) {
		      tmpi1->ic_flags|=tmpi->ic_flags;
		      tmpi1->t.arg1_tree->ic_data*=tmpi2->ic_data;
		      tmpi1->ic_class2=tmpi->ic_class2;
		      tmpi_push=tmpi1;
		      OptSetNOP1(tmpi2);
		      OptSetNOP1(tmpi);
		    } else if (tmpi1->t.arg2_tree->ic_code==IC_IMM_I64) {
		      tmpi1->ic_flags|=tmpi->ic_flags;
		      tmpi1->t.arg2_tree->ic_data*=tmpi2->ic_data;
		      tmpi1->ic_class2=tmpi->ic_class2;
		      tmpi_push=tmpi1;
		      OptSetNOP1(tmpi2);
		      OptSetNOP1(tmpi);
		    }
		  }
	      }
	    } else if (tmpi2->ic_code==IC_IMM_F64 && cc->pass==2) {
	      if (tmpi2->ic_data(F64)==1.0) {
		tmpi1->ic_flags|=tmpi->ic_flags|tmpi2->ic_flags;
		tmpi1->ic_class2=tmpi->ic_class2;
		tmpi_push=tmpi1;
		OptSetNOP1(tmpi2);
		OptSetNOP1(tmpi);
	      } else if (tmpi1->ic_code==IC_MUL) {
		if (tmpi1->t.arg1_tree->ic_code==IC_IMM_F64) {
		  tmpi1->ic_flags|=tmpi->ic_flags|tmpi2->ic_flags;
		  tmpi1->t.arg1_tree->ic_data(F64)*=tmpi2->ic_data(F64);
		  tmpi1->ic_class2=tmpi->ic_class2;
		  tmpi_push=tmpi1;
		  OptSetNOP1(tmpi2);
		  OptSetNOP1(tmpi);
		} else if (tmpi1->t.arg2_tree->ic_code==IC_IMM_F64) {
		  tmpi1->ic_flags|=tmpi->ic_flags|tmpi2->ic_flags;
		  tmpi1->t.arg2_tree->ic_data(F64)*=tmpi2->ic_data(F64);
		  tmpi1->ic_class2=tmpi->ic_class2;
		  tmpi_push=tmpi1;
		  OptSetNOP1(tmpi2);
		  OptSetNOP1(tmpi);
		}
	      }
	    }
	  }
	  break;
	case IC_DIV:
	  if ((tmpi2->ic_data || tmpi2->ic_code!=IC_IMM_I64 &&
		tmpi2->ic_code!=IC_IMM_F64) &&
		(i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned))) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tmpi->ic_data=tmpi1->ic_data(U64)/tmpi2->ic_data(U64);
	      else
		tmpi->ic_data=tmpi1->ic_data(I64)/tmpi2->ic_data(I64);
	      tmpi->ic_code=IC_IMM_I64;
	    } else {
	      tmpi->ic_data(F64)=tmpi1->ic_data(F64)/
		    tmpi2->ic_data(F64);
	      tmpi->ic_code=IC_IMM_F64;
	    }
	  } else {
	    if (tmpi2->ic_code==IC_IMM_I64 && (i=tmpi2->ic_data)) {
	      if (i==1) {
		tmpi1->ic_flags|=tmpi2->ic_flags|tmpi->ic_flags;
		tmpi1->ic_class2=tmpi->ic_class2;
		tmpi_push=tmpi1;
		OptSetNOP1(tmpi2);
		OptSetNOP1(tmpi);
	      } else if (Bsf(i)==Bsr(i)) {
		tmpi->ic_flags|=tmpi2->ic_flags;
		tmpi->ic_data=Bsf(i);
		tmpi->ic_code=IC_SHR_CONST;
		OptSetNOP1(tmpi2);
	      }
	    }
	  }
	  break;
	case IC_MOD:
	  if ((tmpi2->ic_data || tmpi2->ic_code!=IC_IMM_I64 &&
		tmpi2->ic_code!=IC_IMM_F64) &&
		(i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned))) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tmpi->ic_data=tmpi1->ic_data(U64)%tmpi2->ic_data(U64);
	      else
		tmpi->ic_data=tmpi1->ic_data(I64)%tmpi2->ic_data(I64);
	      tmpi->ic_code=IC_IMM_I64;
	    } else {
	      tmpi->ic_data(F64)=tmpi1->ic_data(F64)%
		    tmpi2->ic_data(F64);
	      tmpi->ic_code=IC_IMM_F64;
	    }
	  } else if (cc->pass==2 && tmpi2->ic_code==IC_IMM_I64 &&
		(i=tmpi2->ic_data) && Bsf(i)==Bsr(i) &&
		tmpi_push->ic_class->raw_type!=RT_F64 &&
		tmpi_push->ic_class->raw_type&RTF_UNSIGNED) {//do only unsigned
	    tmpi2->ic_data=i-1;
	    tmpi->ic_code=IC_AND;
	  }
	  break;
	case IC_AND:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    tmpi->ic_data=tmpi1->ic_data&tmpi2->ic_data;
	    if (i==FBO1_INT)
	      tmpi->ic_code=IC_IMM_I64;
	    else
	      tmpi->ic_code=IC_IMM_F64;
	  }
	  break;
	case IC_OR:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    tmpi->ic_data=tmpi1->ic_data|tmpi2->ic_data;
	    if (i==FBO1_INT)
	      tmpi->ic_code=IC_IMM_I64;
	    else
	      tmpi->ic_code=IC_IMM_F64;
	  }
	  break;
	case IC_XOR:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    tmpi->ic_data=tmpi1->ic_data^tmpi2->ic_data;
	    if (i==FBO1_INT)
	      tmpi->ic_code=IC_IMM_I64;
	    else
	      tmpi->ic_code=IC_IMM_F64;
	  }
	  break;
	case IC_ADD:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      tmpi->ic_data=tmpi1->ic_data+tmpi2->ic_data;
	      tmpi->ic_code=IC_IMM_I64;
	    } else {
	      tmpi->ic_data(F64)=tmpi1->ic_data(F64)+
		    tmpi2->ic_data(F64);
	      tmpi->ic_code=IC_IMM_F64;
	    }
	  } else {
	    if (tmpi1->ic_code==IC_ABS_ADDR&&tmpi2->ic_code==IC_IMM_I64) {
	      tmpi->ic_flags|=tmpi1->ic_flags|tmpi2->ic_flags;
	      tmpi->ic_data=tmpi1->ic_data+tmpi2->ic_data;
	      tmpi->ic_code=IC_ABS_ADDR;
	      OptSetNOP1(tmpi1);
	      OptSetNOP1(tmpi2);
	    } else if (cc->pass==2) {
	      if (tmpi1->ic_code==IC_IMM_I64) {
		if (!tmpi1->ic_data) {
		  tmpi2->ic_flags|=tmpi1->ic_flags|tmpi->ic_flags;
		  tmpi2->ic_class2=tmpi->ic_class2;
		  tmpi_push=tmpi2;
		  OptSetNOP1(tmpi1);
		  OptSetNOP1(tmpi);
		} else if (tmpi2->ic_code==IC_ADD||
		      tmpi2->ic_code==IC_SUB) {
		  if (tmpi2->t.arg1_tree->ic_code==IC_IMM_I64) {
		    tmpi2->ic_flags|=tmpi->ic_flags;
		    tmpi2->t.arg1_tree->ic_data+=tmpi1->ic_data;
		    tmpi2->ic_class2=tmpi->ic_class2;
		    tmpi_push=tmpi2;
		    OptSetNOP1(tmpi1);
		    OptSetNOP1(tmpi);
		  } else if (tmpi2->t.arg2_tree->ic_code==IC_IMM_I64) {
		    tmpi2->ic_flags|=tmpi->ic_flags;
		    if (tmpi2->ic_code==IC_ADD)
		      tmpi2->t.arg2_tree->ic_data+=tmpi1->ic_data;
		    else
		      tmpi2->t.arg2_tree->ic_data-=tmpi1->ic_data;
		    tmpi2->ic_class2=tmpi->ic_class2;
		    tmpi_push=tmpi2;
		    OptSetNOP1(tmpi1);
		    OptSetNOP1(tmpi);
		  }
		}
	      } else if (tmpi2->ic_code==IC_IMM_I64) {
		if (!tmpi2->ic_data) {
		  tmpi1->ic_flags|=tmpi2->ic_flags|tmpi->ic_flags;
		  tmpi1->ic_class2=tmpi->ic_class2;
		  tmpi_push=tmpi1;
		  OptSetNOP1(tmpi2);
		  OptSetNOP1(tmpi);
		} else if (tmpi1->ic_code==IC_ADD ||
		      tmpi1->ic_code==IC_SUB) {
		  if (tmpi1->t.arg1_tree->ic_code==IC_IMM_I64) {
		    tmpi1->ic_flags|=tmpi->ic_flags;
		    tmpi1->t.arg1_tree->ic_data+=tmpi2->ic_data;
		    tmpi1->ic_class2=tmpi->ic_class2;
		    tmpi_push=tmpi1;
		    OptSetNOP1(tmpi2);
		    OptSetNOP1(tmpi);
		  } else if (tmpi1->t.arg2_tree->ic_code==IC_IMM_I64) {
		    tmpi1->ic_flags|=tmpi->ic_flags;
		    if (tmpi1->ic_code==IC_ADD)
		      tmpi1->t.arg2_tree->ic_data+=tmpi2->ic_data;
		    else
		      tmpi1->t.arg2_tree->ic_data-=tmpi2->ic_data;
		    tmpi1->ic_class2=tmpi->ic_class2;
		    tmpi_push=tmpi1;
		    OptSetNOP1(tmpi2);
		    OptSetNOP1(tmpi);
		  }
		}
	      } else if (tmpi1->ic_code==IC_IMM_F64) {
		if (!tmpi1->ic_data) {
		  tmpi2->ic_flags|=tmpi1->ic_flags|tmpi->ic_flags;
		  tmpi2->ic_class2=tmpi->ic_class2;
		  tmpi_push=tmpi2;
		  OptSetNOP1(tmpi1);
		  OptSetNOP1(tmpi);
		} else if (tmpi2->ic_code==IC_ADD||
		      tmpi2->ic_code==IC_SUB) {
		  if (tmpi2->t.arg1_tree->ic_code==IC_IMM_F64) {
		    tmpi2->ic_flags|=tmpi->ic_flags;
		    tmpi2->t.arg1_tree->ic_data(F64)+=tmpi1->ic_data(F64);
		    tmpi2->ic_class2=tmpi->ic_class2;
		    tmpi_push=tmpi2;
		    OptSetNOP1(tmpi1);
		    OptSetNOP1(tmpi);
		  } else if (tmpi2->t.arg2_tree->ic_code==IC_IMM_F64) {
		    tmpi2->ic_flags|=tmpi->ic_flags;
		    if (tmpi2->ic_code==IC_ADD)
		      tmpi2->t.arg2_tree->ic_data(F64)+=tmpi1->ic_data(F64);
		    else
		      tmpi2->t.arg2_tree->ic_data(F64)-=tmpi1->ic_data(F64);
		    tmpi2->ic_class2=tmpi->ic_class2;
		    tmpi_push=tmpi2;
		    OptSetNOP1(tmpi1);
		    OptSetNOP1(tmpi);
		  }
		}
	      } else if (tmpi2->ic_code==IC_IMM_F64) {
		if (!tmpi2->ic_data) {
		  tmpi1->ic_flags|=tmpi2->ic_flags|tmpi->ic_flags;
		  tmpi1->ic_class2=tmpi->ic_class2;
		  tmpi_push=tmpi1;
		  OptSetNOP1(tmpi2);
		  OptSetNOP1(tmpi);
		} else if (tmpi1->ic_code==IC_ADD ||
		      tmpi1->ic_code==IC_SUB) {
		  if (tmpi1->t.arg1_tree->ic_code==IC_IMM_F64) {
		    tmpi1->ic_flags|=tmpi->ic_flags;
		    tmpi1->t.arg1_tree->ic_data(F64)+=tmpi2->ic_data(F64);
		    tmpi1->ic_class2=tmpi->ic_class2;
		    tmpi_push=tmpi1;
		    OptSetNOP1(tmpi2);
		    OptSetNOP1(tmpi);
		  } else if (tmpi1->t.arg2_tree->ic_code==IC_IMM_F64) {
		    tmpi1->ic_flags|=tmpi->ic_flags;
		    if (tmpi1->ic_code==IC_ADD)
		      tmpi1->t.arg2_tree->ic_data(F64)+=tmpi2->ic_data(F64);
		    else
		      tmpi1->t.arg2_tree->ic_data(F64)-=tmpi2->ic_data(F64);
		    tmpi1->ic_class2=tmpi->ic_class2;
		    tmpi_push=tmpi1;
		    OptSetNOP1(tmpi2);
		    OptSetNOP1(tmpi);
		  }
		}
	      }
	    }
	  }
	  break;
	case IC_SUB:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      tmpi->ic_data=tmpi1->ic_data-tmpi2->ic_data;
	      tmpi->ic_code=IC_IMM_I64;
	    } else {
	      tmpi->ic_data(F64)=tmpi1->ic_data(F64)-
		    tmpi2->ic_data(F64);
	      tmpi->ic_code=IC_IMM_F64;
	    }
	  } else {
	    if (cc->pass==2) {
	      if (tmpi2->ic_code==IC_IMM_I64) {
		if (!tmpi2->ic_data) {
		  tmpi1->ic_flags|=tmpi2->ic_flags|tmpi->ic_flags;
		  tmpi1->ic_class2=tmpi->ic_class2;
		  tmpi_push=tmpi1;
		  OptSetNOP1(tmpi2);
		  OptSetNOP1(tmpi);
		} else
		  if (tmpi1->ic_code==IC_ADD ||
			tmpi1->ic_code==IC_SUB) {
		    if (tmpi1->t.arg1_tree->ic_code==IC_IMM_I64) {
		      tmpi1->ic_flags|=tmpi->ic_flags;
		      tmpi1->t.arg1_tree->ic_data-=tmpi2->ic_data;
		      tmpi1->ic_class2=tmpi->ic_class2;
		      tmpi_push=tmpi1;
		      OptSetNOP1(tmpi2);
		      OptSetNOP1(tmpi);
		    } else if (tmpi1->t.arg2_tree->ic_code==IC_IMM_I64) {
		      tmpi1->ic_flags|=tmpi->ic_flags;
		      if (tmpi1->ic_code==IC_ADD)
			tmpi1->t.arg2_tree->ic_data-=tmpi2->ic_data;
		      else
			tmpi1->t.arg2_tree->ic_data+=tmpi2->ic_data;
		      tmpi1->ic_class2=tmpi->ic_class2;
		      tmpi_push=tmpi1;
		      OptSetNOP1(tmpi2);
		      OptSetNOP1(tmpi);
		    }
		  }
	      } else
		if (tmpi2->ic_code==IC_IMM_F64) {
		  if (!tmpi2->ic_data) {
		    tmpi1->ic_flags|=tmpi2->ic_flags|tmpi->ic_flags;
		    tmpi1->ic_class2=tmpi->ic_class2;
		    tmpi_push=tmpi1;
		    OptSetNOP1(tmpi2);
		    OptSetNOP1(tmpi);
		  } else
		    if (tmpi1->ic_code==IC_ADD || tmpi1->ic_code==IC_SUB) {
		      if (tmpi1->t.arg1_tree->ic_code==IC_IMM_F64) {
			tmpi1->ic_flags|=tmpi->ic_flags|tmpi2->ic_flags;
			tmpi1->t.arg1_tree->ic_data(F64)-=tmpi2->ic_data(F64);
			tmpi1->ic_class2=tmpi->ic_class2;
			tmpi_push=tmpi1;
			OptSetNOP1(tmpi2);
			OptSetNOP1(tmpi);
		      } else if (tmpi1->t.arg2_tree->ic_code==IC_IMM_F64) {
			tmpi1->ic_flags|=tmpi->ic_flags|tmpi2->ic_flags;
			if (tmpi1->ic_code==IC_ADD)
			  tmpi1->t.arg2_tree->ic_data(F64)-=tmpi2->ic_data(F64);
			else
			  tmpi1->t.arg2_tree->ic_data(F64)+=tmpi2->ic_data(F64);
			tmpi1->ic_class2=tmpi->ic_class2;
			tmpi_push=tmpi1;
			OptSetNOP1(tmpi2);
			OptSetNOP1(tmpi);
		      }
		    }
		}
	    }
	  }
	  break;
	case IC_AND_AND:
	  if (OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    tmpi->ic_data=tmpi1->ic_data&&tmpi2->ic_data;
	    tmpi->ic_code=IC_IMM_I64;
	  }
	  tmpi->ic_class=cmp.internal_types[RT_I64];
	  break;
	case IC_OR_OR:
	  if (OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    tmpi->ic_data=tmpi1->ic_data||tmpi2->ic_data;
	    tmpi->ic_code=IC_IMM_I64;
	  }
	  tmpi->ic_class=cmp.internal_types[RT_I64];
	  break;
	case IC_XOR_XOR:
	  if (OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    tmpi->ic_data=tmpi1->ic_data^^tmpi2->ic_data;
	    tmpi->ic_code=IC_IMM_I64;
	  }
	  tmpi->ic_class=cmp.internal_types[RT_I64];
	  break;
      end:
	if (!tmpi_push->ic_class->ptr_stars_cnt) {
	  if (tmpi_push->ic_class->raw_type==RT_F64)
	    tmpi_push->ic_class2=cmp.internal_types[RT_F64];
	  else if (is_unsigned)
	    tmpi_push->ic_class2=cmp.internal_types[RT_U64];
	  else
	    tmpi_push->ic_class2=cmp.internal_types[RT_I64];
	}
	break;
      start:
	case IC_EQU_EQU:
	  if (OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    tmpi->ic_data=tmpi1->ic_data==tmpi2->ic_data;
	    tmpi->ic_code=IC_IMM_I64;
	  } else
	    if (tmpi->ic_class->raw_type==RT_F64)
	      tmpi->ic_flags|=ICF_USE_F64;
	  break;
	case IC_NOT_EQU:
	  if (OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    tmpi->ic_data=tmpi1->ic_data!=tmpi2->ic_data;
	    tmpi->ic_code=IC_IMM_I64;
	  } else
	    if (tmpi->ic_class->raw_type==RT_F64)
	      tmpi->ic_flags|=ICF_USE_F64;
	  break;
	case IC_LESS:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tmpi->ic_data=tmpi1->ic_data(U64)<tmpi2->ic_data(U64);
	      else
		tmpi->ic_data=tmpi1->ic_data(I64)<tmpi2->ic_data(I64);
	    } else
	      tmpi->ic_data=tmpi1->ic_data(F64)<tmpi2->ic_data(F64);
	    tmpi->ic_code=IC_IMM_I64;
	  } else {
	    if (is_unsigned)
	      tmpi->ic_flags|=ICF_USE_UNSIGNED;
	    if (tmpi->ic_class->raw_type==RT_F64)
	      tmpi->ic_flags|=ICF_USE_F64;
	  }
	  break;
	case IC_GREATER_EQU:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tmpi->ic_data=tmpi1->ic_data(U64)>=tmpi2->ic_data(U64);
	      else
		tmpi->ic_data=tmpi1->ic_data(I64)>=tmpi2->ic_data(I64);
	    } else
	      tmpi->ic_data=tmpi1->ic_data(F64)>=tmpi2->ic_data(F64);
	    tmpi->ic_code=IC_IMM_I64;
	  } else {
	    if (is_unsigned)
	      tmpi->ic_flags|=ICF_USE_UNSIGNED;
	    if (tmpi->ic_class->raw_type==RT_F64)
	      tmpi->ic_flags|=ICF_USE_F64;
	  }
	  break;
	case IC_GREATER:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tmpi->ic_data=tmpi1->ic_data(U64)>tmpi2->ic_data(U64);
	      else
		tmpi->ic_data=tmpi1->ic_data(I64)>tmpi2->ic_data(I64);
	    } else
	      tmpi->ic_data=tmpi1->ic_data(F64)>tmpi2->ic_data(F64);
	    tmpi->ic_code=IC_IMM_I64;
	  } else {
	    if (is_unsigned)
	      tmpi->ic_flags|=ICF_USE_UNSIGNED;
	    if (tmpi->ic_class->raw_type==RT_F64)
	      tmpi->ic_flags|=ICF_USE_F64;
	  }
	  break;
	case IC_LESS_EQU:
	  if (i=OptFixupBinaryOp1(tmpi,tmpi1,tmpi2,&is_unsigned)) {
	    if (i==FBO1_INT) {
	      if (is_unsigned)
		tmpi->ic_data=tmpi1->ic_data(U64)<=tmpi2->ic_data(U64);
	      else
		tmpi->ic_data=tmpi1->ic_data(I64)<=tmpi2->ic_data(I64);
	    } else
	      tmpi->ic_data=tmpi1->ic_data(F64)<=tmpi2->ic_data(F64);
	    tmpi->ic_code=IC_IMM_I64;
	  } else {
	    if (is_unsigned)
	      tmpi->ic_flags|=ICF_USE_UNSIGNED;
	    if (tmpi->ic_class->raw_type==RT_F64)
	      tmpi->ic_flags|=ICF_USE_F64;
	  }
	  break;
      end:
	tmpi->ic_flags&=~ICF_RES_TO_INT;
	if (!tmpi_push->ic_class->ptr_stars_cnt) {
	  if (tmpi_push->ic_class->raw_type==RT_F64)
	    tmpi_push->ic_class2=cmp.internal_types[RT_F64];
	  else if (is_unsigned)
	    tmpi_push->ic_class2=cmp.internal_types[RT_U64];
	  else
	    tmpi_push->ic_class2=cmp.internal_types[RT_I64];
	}
	if (tmpi_push->ic_flags & ICF_PUSH_CMP)
	  tmpi->ic_class=tmpi->ic_class2;
	else
	  tmpi->ic_class=cmp.internal_types[RT_I64];
	break;
      start:
	if (cc->pass==2 && (!tmpc->size||!tmpc2->size))
	  LexWarn(cc,"Assign U0 ");
	start:
	  case IC_MUL_EQU:
	    if (tmpi2->ic_code==IC_IMM_I64 && tmpc->raw_type!=RT_F64 &&
		  tmpc2->raw_type!=RT_F64) {
	      if (i=tmpi2->ic_data) {
		if (Bsf(i)==Bsr(i)) {
		  tmpi2->ic_data=Bsf(i);
		  tmpi->ic_code=IC_SHL_EQU;
		}
	      }
	    }
	    break;
	  case IC_DIV_EQU:
	    if (tmpi2->ic_code==IC_IMM_I64 && tmpc->raw_type!=RT_F64 &&
		  tmpc2->raw_type!=RT_F64 &&
		  (i=tmpi2->ic_data) && Bsf(i)==Bsr(i)) {
	      tmpi2->ic_data=Bsf(i);
	      tmpi->ic_code=IC_SHR_EQU;
	    }
	    break;
	  case IC_MOD_EQU:
	    if (tmpi2->ic_code==IC_IMM_I64 && tmpc->raw_type!=RT_F64 &&
		  tmpc2->raw_type!=RT_F64 &&
		  (i=tmpi2->ic_data) && Bsf(i)==Bsr(i)) {
	      tmpi2->ic_data=i-1;
	      tmpi->ic_code=IC_AND_EQU;
	    }
	    break;
	  case IC_ADD_EQU:
	  case IC_SUB_EQU:
	    break;
	end:
	  if (tmpi2->ic_class->raw_type==RT_F64)
	    tmpi->ic_flags=tmpi->ic_flags|ICF_USE_F64;
	  if (tmpc->raw_type==RT_F64) {
	    if (tmpc2->raw_type!=RT_F64)
	      tmpi2->ic_flags|=ICF_RES_TO_F64;
	  }
	  break;
	case IC_ASSIGN:
	  if (tmpc->raw_type==RT_F64) {
	    if (tmpc2->raw_type!=RT_F64)
	      tmpi2->ic_flags|=ICF_RES_TO_F64;
	  } else {
	    if (tmpc2->raw_type==RT_F64)
	      tmpi2->ic_flags|=ICF_RES_TO_INT;
	  }
	  if (cc->pass==2 && tmpi1->ic_class->raw_type!=RT_F64) {
	    if (tmpi1->ic_code==IC__PP) {
	      tmpi->ic_code=IC_ASSIGN_PP;
	      tmpi->ic_flags|=tmpi1->ic_flags;
	      tmpi->t.class2=tmpi1->ic_class;
	      OptSetNOP1(tmpi1);
	    } else if (tmpi1->ic_code==IC__MM) {
	      tmpi->ic_code=IC_ASSIGN_MM;
	      tmpi->ic_flags|=tmpi1->ic_flags;
	      tmpi->t.class2=tmpi1->ic_class;
	      OptSetNOP1(tmpi1);
	    }
	  }
	  break;
	case IC_SHL_EQU:
	case IC_SHR_EQU:
	case IC_AND_EQU:
	case IC_OR_EQU:
	case IC_XOR_EQU:
	  if (tmpc2->raw_type==RT_F64)
	    tmpi2->ic_flags|=ICF_RES_TO_INT;
	  break;
      end:
	break;
      case IC_ENTER:
      case IC_LEAVE:
	tmpi->ic_data=-cc->htc.fun->size;
	break;
      case IC_ADD_RSP:
	if (tmpi1=OptLag(tmpi)) {
	  if (tmpi1->ic_code==IC_ADD_RSP) {
	    tmpi->ic_data+=tmpi1->ic_data;
	    tmpi->ic_flags|=tmpi1->ic_flags;
	    OptSetNOP1(tmpi1);
	  }
	}
      case IC_ADD_RSP1:
	break;
      case IC_BSF:
	if (tmpi1->ic_code==IC_IMM_I64) {
	  tmpi1->ic_data=Bsf(tmpi1->ic_data);
	  tmpi_push=tmpi1;
	  OptSetNOP1(OptLag(tmpi1));	//CALL_START
	  tmpi2=OptLead1(tmpi);
	  tmpi1->ic_flags|=tmpi->ic_flags|tmpi2->ic_flags;
	  OptSetNOP1(tmpi2);		//CALL_END
	  OptSetNOP1(tmpi);		//BSF
	}
	break;
      case IC_BSR:
	if (tmpi1->ic_code==IC_IMM_I64) {
	  tmpi1->ic_data=Bsr(tmpi1->ic_data);
	  tmpi_push=tmpi1;
	  OptSetNOP1(OptLag(tmpi1));	//CALL_START
	  tmpi2=OptLead1(tmpi);
	  tmpi1->ic_flags|=tmpi->ic_flags|tmpi2->ic_flags;
	  OptSetNOP1(tmpi2);		//CALL_END
	  OptSetNOP1(tmpi);		//BSR
	}
	break;
      case IC_LBTS:
      case IC_LBTR:
      case IC_LBTC:
	tmpi->ic_flags|=ICF_LOCK;
	break;
      case IC_TO_I64:
	if (tmpi1->ic_code==IC_IMM_F64) {
	  tmpi2=tmpi1->last;
	  while (tmpi2->ic_code!=IC_CALL_START)
	    tmpi2=tmpi2->last;
	  OptSetNOP1(tmpi2);

	  tmpi2=tmpi->next;
	  while (tmpi2->ic_code!=IC_CALL_END)
	    tmpi2=tmpi2->next;

	  tmpi->ic_flags|=tmpi1->ic_flags&~ICF_RES_TO_INT|tmpi2->ic_flags;
	  OptSetNOP1(tmpi2);

	  tmpi->ic_code=IC_IMM_I64;
	  tmpi->ic_data=ToI64(tmpi1->ic_data(F64));
	  tmpi->ic_class=cmp.internal_types[RT_I64];
	  tmpi->ic_class2=cmp.internal_types[RT_I64];
	  OptSetNOP1(tmpi1);
	} else if (tmpi1->ic_code==IC_IMM_I64) {
	  tmpi2=tmpi1->last;
	  while (tmpi2->ic_code!=IC_CALL_START)
	    tmpi2=tmpi2->last;
	  OptSetNOP1(tmpi2);

	  tmpi2=tmpi->next;
	  while (tmpi2->ic_code!=IC_CALL_END)
	    tmpi2=tmpi2->next;

	  tmpi->ic_flags|=tmpi1->ic_flags&~ICF_RES_TO_F64|tmpi2->ic_flags;
	  OptSetNOP1(tmpi2);

	  tmpi->ic_code=IC_IMM_I64;
	  tmpi->ic_data=tmpi1->ic_data;
	  tmpi->ic_class=cmp.internal_types[RT_I64];
	  tmpi->ic_class2=cmp.internal_types[RT_I64];
	  OptSetNOP1(tmpi1);
	}
	if (tmpi1->ic_flags&ICF_RES_TO_F64) {
	  i=0;
	  tmpi2=tmpi1->last;
	  while (TRUE) {
	    if (tmpi2->ic_code==IC_CALL_START) {
	      if (!i) break;
	      i--;
	    } else if (tmpi2->ic_code==IC_CALL_END)
	      i++;
	    tmpi2=tmpi2->last;
	  }
	  OptSetNOP1(tmpi2);

	  tmpi2=tmpi1->next;
	  while (tmpi2->ic_code!=IC_CALL_END)
	    tmpi2=tmpi2->next;

	  tmpi1->ic_flags=tmpi->ic_flags|tmpi1->ic_flags&
		~(ICF_RES_TO_F64|ICF_PUSH_RES)|tmpi2->ic_flags;
	  OptSetNOP1(tmpi2);

	  tmpi1->ic_class=cmp.internal_types[RT_I64];
	  tmpi1->ic_class2=cmp.internal_types[RT_I64];
	  tmpi_push=tmpi1;
	  OptSetNOP1(tmpi);
	}
	break;
      case IC_TO_F64:
	if (tmpi1->ic_code==IC_IMM_I64) {
	  tmpi2=tmpi1->last;
	  while (tmpi2->ic_code!=IC_CALL_START)
	    tmpi2=tmpi2->last;
	  OptSetNOP1(tmpi2);

	  tmpi2=tmpi->next;
	  while (tmpi2->ic_code!=IC_CALL_END)
	    tmpi2=tmpi2->next;

	  tmpi->ic_flags|=tmpi1->ic_flags&~ICF_RES_TO_F64|tmpi2->ic_flags;
	  OptSetNOP1(tmpi2);

	  tmpi->ic_code=IC_IMM_F64;
	  tmpi->ic_data(F64)=ToF64(tmpi1->ic_data);
	  tmpi->ic_class=cmp.internal_types[RT_F64];
	  tmpi->ic_class2=cmp.internal_types[RT_F64];
	  OptSetNOP1(tmpi1);
	} else if (tmpi1->ic_code==IC_IMM_F64) {
	  tmpi2=tmpi1->last;
	  while (tmpi2->ic_code!=IC_CALL_START)
	    tmpi2=tmpi2->last;
	  OptSetNOP1(tmpi2);

	  tmpi2=tmpi->next;
	  while (tmpi2->ic_code!=IC_CALL_END)
	    tmpi2=tmpi2->next;

	  tmpi->ic_flags|=tmpi1->ic_flags&~ICF_RES_TO_INT|tmpi2->ic_flags;
	  OptSetNOP1(tmpi2);

	  tmpi->ic_code=IC_IMM_F64;
	  tmpi->ic_data=tmpi1->ic_data;
	  tmpi->ic_class=cmp.internal_types[RT_F64];
	  tmpi->ic_class2=cmp.internal_types[RT_F64];
	  OptSetNOP1(tmpi1);
	}
	if (tmpi1->ic_flags&ICF_RES_TO_INT) {
	  i=0;
	  tmpi2=tmpi1->last;
	  while (TRUE) {
	    if (tmpi2->ic_code==IC_CALL_START) {
	      if (!i) break;
	      i--;
	    } else if (tmpi2->ic_code==IC_CALL_END)
	      i++;
	    tmpi2=tmpi2->last;
	  }
	  OptSetNOP1(tmpi2);

	  tmpi2=tmpi1->next;
	  while (tmpi2->ic_code!=IC_CALL_END)
	    tmpi2=tmpi2->next;

	  tmpi1->ic_flags=tmpi->ic_flags|tmpi1->ic_flags&
		~(ICF_RES_TO_INT|ICF_PUSH_RES)|tmpi2->ic_flags;
	  OptSetNOP1(tmpi2);

	  tmpi1->ic_class=cmp.internal_types[RT_F64];
	  tmpi1->ic_class2=cmp.internal_types[RT_F64];
	  tmpi_push=tmpi1;
	  OptSetNOP1(tmpi);
	}
	break;
      case IC_TO_BOOL:
	if (tmpi1->ic_code==IC_IMM_I64 || tmpi1->ic_code==IC_IMM_F64) {
	  tmpi2=tmpi1->last;
	  while (tmpi2->ic_code!=IC_CALL_START)
	    tmpi2=tmpi2->last;
	  OptSetNOP1(tmpi2);

	  tmpi2=tmpi->next;
	  while (tmpi2->ic_code!=IC_CALL_END)
	    tmpi2=tmpi2->next;

	  tmpi->ic_flags|=tmpi1->ic_flags&~ICF_RES_TO_F64|tmpi2->ic_flags;
	  OptSetNOP1(tmpi2);

	  tmpi->ic_code=IC_IMM_I64;
	  tmpi->ic_data=ToBool(tmpi1->ic_data);
	  tmpi->ic_class=cmp.internal_types[RT_I64];
	  tmpi->ic_class2=cmp.internal_types[RT_I64];
	  OptSetNOP1(tmpi1);
	}
	break;
      case IC_BR_ZERO:
	tmpi_push=OptBrZero(cc,tmpi);
	break;
      case IC_BR_NOT_ZERO:
	tmpi_push=OptBrNotZero(cc,tmpi);
	break;
      case IC_NOP1:
	if (tmpi->ic_flags&ICF_PUSH_RES) {
	  tmpi1=tmpi;
	  do tmpi1=tmpi1->last;
	  while (tmpi1->ic_code==IC_NOP1);
	  tmpi1->ic_flags|=ICF_PUSH_RES;
	  tmpi->ic_flags&=~ICF_PUSH_RES;
	}
	break;
      case IC_NOP2:
	ps->ptr+=tmpi->ic_data<<1;
	break;
      case IC_LABEL:
	lb=OptLabelFwd(tmpi->ic_data);
	lb1=tmpi->ic_data;
	while (lb2=lb1->fwd) {
	  lb1->fwd=lb;
	  lb1=lb2;
	}
	if (tmpi1=OptLag(tmpi)) {
	  if (tmpi1->ic_code==IC_JMP) {
	    lb1=tmpi1->ic_data;
	    while (lb1->fwd)
	      lb1=lb1->fwd;
	    if (lb1==lb) {
	      tmpi->ic_flags|=tmpi1->ic_flags;
	      OptSetNOP1(tmpi1);
	    }
	  } else if (tmpi1->ic_code==IC_LABEL) {
	    lb1=tmpi1->ic_data;
	    if (!lb1->fwd)
	      lb1->fwd=lb;
	    if (tmpi1=OptLag(tmpi1)) {
	      if (tmpi1->ic_code==IC_JMP) {
		lb1=tmpi1->ic_data;
		while (lb1->fwd)
		  lb1=lb1->fwd;
		if (lb1==lb) {
		  tmpi->ic_flags|=tmpi1->ic_flags;
		  OptSetNOP1(tmpi1);
		}
	      }
	    }
	  }
	}
	break;
      case IC_JMP:
	if (tmpi1=OptLag(tmpi)) {
	  if (tmpi1->ic_code==IC_LABEL) {
	    lb=OptLabelFwd(tmpi->ic_data);
	    lb1=OptLabelFwd(tmpi1->ic_data);
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
	"Pass:%d Missing IC hndlr\n",cc->pass;
	ICPut(cc,tmpi);
	LexExcept(cc,"Compiler Optimization Error at ");
    }
    if (intermediate_code_table[code].arg_cnt==IS_2_ARG) {
      if (tmpi_push->ic_precedence&~ASSOC_MASK==PREC_ASSIGN)
	OptFixSizeOf(tmpi2,tmpi_push,tmpi1->ic_class-1);
      else {
	OptFixSizeOf(tmpi1,tmpi_push,tmpi2->ic_class);
	OptFixSizeOf(tmpi2,tmpi_push,tmpi1->ic_class);
      }
    }
    if (intermediate_code_table[tmpi_push->ic_code].res_cnt) {
      PrsPush(ps,tmpi->ic_class2);
      PrsPush(ps,tmpi_push);
    }
    if (tmpi->ic_class) {
      if (tmpi->ic_class->raw_type==RT_F64)
	tmpi->ic_flags&=~ICF_RES_TO_F64;
      else
	tmpi->ic_flags&=~ICF_RES_TO_INT;
      if (code>IC_END_EXP)
	last_with_class=tmpi;
    }
    tmpi=tmpi->next;
  }
  if (ps->ptr>2) {
    "Pass:%d Stk:%08X\n",cc->pass,ps->ptr;
    LexExcept(cc,"Compiler Optimization Error at ");
  }
//This is for determining type conversions for passing args to funs.
  return last_with_class;
}

