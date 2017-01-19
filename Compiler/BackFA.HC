#define CN_A2	0
#define CN_A1	1
#define CN_INST	2
#define CN_RES	3

U0 CmpNoteFloatOp(CCmpCtrl *cc,CIntermediateCode *tmpi,
	Bool dont_pushable,Bool dont_popable,I64 pos)
{
  Bool link=FALSE;
  if (cc->pass==7 && cc->last_float_op_ic &&
	cc->last_dont_popable && dont_pushable) {
    switch [pos] {
      case CN_A2:
	if (cc->last_float_op_ic!=tmpi && cc->dont_push_float)
	  link=TRUE;
	break;
      case CN_A1:
	if (cc->last_float_op_ic!=tmpi && cc->dont_push_float)
	  link=TRUE;
	break;
      case CN_INST:
	if (cc->last_float_op_ic!=tmpi) {
	  if (cc->dont_push_float) {
	    if (intermediate_code_table[tmpi->ic_code].arg_cnt==IS_2_ARG &&
		  cc->last_float_op_ic->res.reg!=REG_R8)
	      tmpi->ic_flags|=ICF_ALT_TEMPLATE;
	    else
	      tmpi->ic_flags&=~ICF_ALT_TEMPLATE;
	    link=TRUE;
	  }
	} else {
	  if (intermediate_code_table[tmpi->ic_code].arg_cnt==IS_2_ARG &&
		cc->last_float_op_pos!=CN_A1)
	    tmpi->ic_flags|=ICF_ALT_TEMPLATE;
	  else
	    tmpi->ic_flags&=~ICF_ALT_TEMPLATE;
	  link=TRUE;
	}
	break;
      case CN_RES:
	if (cc->last_float_op_ic==tmpi && cc->last_float_op_pos==CN_INST)
	  link=TRUE;
	break;
    }
    if (link) {
      if (!Bts(&cc->last_float_op_ic->ic_flags,
	    ICf_DONT_POP_FLOAT0+cc->last_ic_float_op_num))
	cc->last_float_op_ic->ic_flags&=~ICF_CODE_FINAL;
      if (!Bts(&tmpi->ic_flags,ICf_DONT_PUSH_FLOAT0+cc->cur_ic_float_op_num))
	tmpi->ic_flags&=~ICF_CODE_FINAL;
    }
  }
  cc->last_float_op_ic=tmpi;
  cc->last_dont_pushable=dont_pushable;
  cc->last_dont_popable=dont_popable;
  cc->last_ic_float_op_num=cc->cur_ic_float_op_num++;
  cc->last_float_op_pos=pos;
  if (cc->cur_ic_float_op_num>4)
    throw('Compiler');
}

U0 CmpSetFloatOpPushPop(CCmpCtrl *cc,CIntermediateCode *tmpi,
	Bool *dont_push_float,Bool *dont_pop_float)
{
  if (cc->pass==7) {
    *dont_push_float=FALSE;
    *dont_pop_float =FALSE;
    tmpi->ic_flags&=~ICF_CODE_FINAL;
  } else {
    *dont_push_float=Bt(&tmpi->ic_flags,
	  ICf_DONT_PUSH_FLOAT0+cc->cur_ic_float_op_num);
    *dont_pop_float=Bt(&tmpi->ic_flags,
	  ICf_DONT_POP_FLOAT0+cc->cur_ic_float_op_num);
  }
}

U0 ICCopyTemplate(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 op,
  Bool off_the_record,Bool dont_pushable,Bool dont_popable,I64 pos)
{
  Bool dont_push_float,dont_pop_float,alt;
  U8 *ptr;
  I64 i=0;
  if (!off_the_record) {
    if (tmpi->ic_flags&ICF_ALT_TEMPLATE)
      alt=TRUE;
    else
      alt=FALSE;
    CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  } else {
    dont_push_float=FALSE;
    dont_pop_float=FALSE;
    alt=FALSE;
  }
  if (alt && dont_push_float && !dont_pop_float) {
    ptr=cmp_templates_dont_push2[op];
    i=cmp_templates_dont_push2[op+1]-ptr;
  }
  if (!i) {
    if (dont_push_float) {
      if (dont_pop_float) {
	ptr=cmp_templates_dont_push_pop[op];
	i=cmp_templates_dont_push_pop[op+1]-ptr;
      } else {
	ptr=cmp_templates_dont_push[op];
	i=cmp_templates_dont_push[op+1]-ptr;
      }
    } else {
      if (dont_pop_float) {
	ptr=cmp_templates_dont_pop[op];
	i=cmp_templates_dont_pop[op+1]-ptr;
      } else {
	ptr=cmp_templates[op];
	i=cmp_templates[op+1]-ptr;
      }
    }
  }
  MemCpy(&tmpi->ic_body[tmpi->ic_cnt],ptr,i);
  if (!off_the_record)
    CmpNoteFloatOp(cc,tmpi,dont_pushable,dont_popable,pos);
  tmpi->ic_cnt+=i;
}

U0 ICFCvt(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 r1,
	CICType t2,I64 r2,I64 d2,Bool to_int,I64 pos,I64 rip)
{
  I64 rsp_size=0,op1,op2;
  Bool dont_push_float,dont_pop_float;

  if (to_int) {
    op1=SLASH_OP_FLD;
    op2=SLASH_OP_FISTTP;
  } else {
    op1=SLASH_OP_FILD;
    op2=SLASH_OP_FSTP;
  }

  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (!dont_push_float) {
    if (!(t2.raw_type>=RT_I64 && t2&MDG_DISP_SIB_RIP)) {
      ICPush(tmpi,t2,r2,d2,rip);
      t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      rsp_size=8;
    } else {
      if (!dont_pop_float) {
	rsp_size=8;
	ICAddRSP(tmpi,-8);
      }
    }
    ICSlashOp(tmpi,t2,r2,d2,op1,rip);
  } else {
    if (!dont_pop_float) {
      rsp_size=8;
      ICAddRSP(tmpi,-8);
    }
  }
  if (to_int)
    CmpNoteFloatOp(cc,tmpi,TRUE,FALSE,pos);
  else
    CmpNoteFloatOp(cc,tmpi,FALSE,TRUE,pos);
  if (dont_pop_float) {
    if (rsp_size)
      ICAddRSP(tmpi,rsp_size);
  } else {
    ICSlashOp(tmpi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,op2,rip);
    ICPop(tmpi,MDF_REG+RT_I64,r1,0,rip);
  }
}

U0 ICFCvt2(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 r1,
	CICType t2,I64 r2,I64 d2,Bool to_int,I64 rip)
{
  I64 rsp_size=0,op1,op2;
  if (to_int) {
    op1=SLASH_OP_FLD;
    op2=SLASH_OP_FISTTP;
  } else {
    op1=SLASH_OP_FILD;
    op2=SLASH_OP_FSTP;
  }
  if (!(t2.raw_type>=RT_I64 && t2&MDG_DISP_SIB_RIP)) {
    ICPush(tmpi,t2,r2,d2,rip);
    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
    rsp_size=8;
  } else {
    rsp_size=8;
    ICAddRSP(tmpi,-8);
  }
  ICSlashOp(tmpi,t2,r2,d2,op1,rip);
  ICSlashOp(tmpi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,op2,rip);
  ICPop(tmpi,MDF_REG+RT_I64,r1,0,rip);
  cc->last_dont_pushable=cc->last_dont_popable=FALSE; //TODO: improve this
}

U0 ICFUnaryMinus(CCmpCtrl *cc,CIntermediateCode *tmpi,U8 *buf2,I64 rip)
{
  CICArg *arg1=&tmpi->arg1;
  I64 rsp_size=0,builtin1=0,t1,r1,d1;
  Bool dont_push_float,dont_pop_float;

  if (cc->flags&CCF_AOT_COMPILE)
    buf2=cc->aotc->rip;

  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (!dont_push_float) {
    if (arg1->type.raw_type>=RT_I64 && arg1->type&MDG_DISP_SIB_RIP) {
      t1=arg1->type;
      r1=arg1->reg;
      d1=arg1->disp;
    } else {
      if (arg1->type&MDF_IMM) {
	if (!(builtin1=ICBuiltInFloatConst(arg1->disp(F64)))) {
	  t1=MDF_RIP_DISP32+RT_I64;
	  r1=REG_RIP;
	  d1=COCFloatConstFind(cc,arg1->disp(F64))+buf2;
	}
      } else {
	ICPush(tmpi,arg1->type,arg1->reg,arg1->disp,rip);
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	rsp_size+=8;
      }
    }
    if (builtin1)
      ICU16(tmpi,builtin1);
    else
      ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_FLD,rip);
  }
  if (!dont_pop_float && !rsp_size) {
    rsp_size=8;
    ICAddRSP(tmpi,-8);
  }
  ICU16(tmpi,0xE0D9); //FCHS
  CmpNoteFloatOp(cc,tmpi,TRUE,TRUE,CN_INST);
  if (dont_pop_float) {
    if (rsp_size)
      ICAddRSP(tmpi,rsp_size);
  } else {
    ICSlashOp(tmpi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,SLASH_OP_FSTP,rip);
    ICPop(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,rip);
  }
}

U0 ICFMod(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 rip)
{//for MOD
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (tmpi->ic_flags&ICF_ALT_TEMPLATE)
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    else
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
  }
//TODO: unpushable,unpop?  Not sure
  ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_INST);
  if (!dont_pop_float)
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_REG+RT_I64,REG_RAX,0,rip);
}

U0 ICFPow(CCmpCtrl *cc,CIntermediateCode *tmpi,U8 *buf,I64 rip)
{//for POW
  I64 i;
  CAOTImportExport *tmpie;
  CHashExport *tmpex=HashFind("SYS_POW",
	cc->htc.hash_table_lst,HTT_EXPORT_SYS_SYM);

  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
  ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
  if (cc->flags&CCF_AOT_COMPILE) {
    if (!tmpex) {
      tmpex=CAlloc(sizeof(CHashExport));
      tmpex->str=StrNew("SYS_POW");
      tmpex->type=HTT_EXPORT_SYS_SYM|HTF_UNRESOLVED|HTF_IMPORT;
      HashAdd(tmpex,cc->htc.glbl_hash_table);
    }
    if (tmpex->type&HTF_IMPORT) {
      if (GetOption(OPTf_USE_IMM64)) {
	ICU16(tmpi,0xBB48);
	ICU64(tmpi,0);
	if (buf) {
	  tmpie=CAlloc(sizeof(CAOTImportExport));
	  tmpie->type=IET_IMM_I64;
	  tmpie->rip=rip+tmpi->ic_cnt-8;
	  tmpie->next=tmpex->ie_lst;
	  tmpex->ie_lst=tmpie;
	}
	ICU16(tmpi,0xD3FF);
      } else {
	ICU8(tmpi,0xE8);
	ICU32(tmpi,-(rip+tmpi->ic_cnt+4));
	if (buf) {
	  tmpie=CAlloc(sizeof(CAOTImportExport));
	  tmpie->type=IET_REL_I32;
	  tmpie->rip=rip+tmpi->ic_cnt-4;
	  tmpie->next=tmpex->ie_lst;
	  tmpex->ie_lst=tmpie;
	}
      }
    } else {//Kernel
      if (tmpex->type&HTF_UNRESOLVED)
	throw('Compiler');
      else {
	i=tmpex->val-(rip+tmpi->ic_cnt+5);
	if (!(I32_MIN<=i<=I32_MAX)) {
	  throw('Compiler');
//	    ICU16(tmpi,0xBB48);
	  //	    ICU64(tmpi,tmpex->val);
	  //	    ICU16(tmpi,0xD3FF);
	} else {
	  ICU8(tmpi,0xE8);
	  ICU32(tmpi,i);
	}
      }
    }
  } else {
    i=tmpex->val-(rip+tmpi->ic_cnt+5);
    if (!(I32_MIN<=i<=I32_MAX)) {
      ICU16(tmpi,0xBB48);
      ICU64(tmpi,tmpex->val);
      ICU16(tmpi,0xD3FF);
    } else {
      ICU8(tmpi,0xE8);
      ICU32(tmpi,i);
    }
  }
  tmpi->ic_flags&=~ICF_CODE_FINAL;
  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	MDF_REG+RT_I64,REG_RAX,0,rip);
}

U0 ICFOp(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 op,U8 *buf2,I64 rip)
{//for ADD,SUB,DIV,MUL
  CICArg *arg1,*arg2;
  Bool dont_push_float,dont_pop_float,alt;
  I64 rsp_size=0,builtin1=0,builtin2=0,t1,r1,d1,t2,r2,d2;

  if (tmpi->ic_flags&ICF_ALT_TEMPLATE) {
    arg1=&tmpi->arg2;
    arg2=&tmpi->arg1;
    alt=TRUE;
  } else {
    arg1=&tmpi->arg1;
    arg2=&tmpi->arg2;
    alt=FALSE;
  }

  if (cc->flags&CCF_AOT_COMPILE)
    buf2=cc->aotc->rip;

  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (arg2->type.raw_type>=RT_I64 && arg2->type&MDG_DISP_SIB_RIP) {
      t2=arg2->type;
      r2=arg2->reg;
      d2=arg2->disp;
    } else {
      if (arg2->type&MDF_IMM) {
	if (!(builtin2=ICBuiltInFloatConst(arg2->disp(F64)))) {
	  t2=MDF_RIP_DISP32+RT_I64;
	  r2=REG_RIP;
	  d2=COCFloatConstFind(cc,arg2->disp(F64))+buf2;
	}
      } else {
	ICPush(tmpi,arg2->type,arg2->reg,arg2->disp,rip);
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	rsp_size+=8;
      }
    }
  } else {
    if (alt) {
      if (!(arg2->type&MDF_STK)) {
	if (arg1->type.raw_type>=RT_I64 && arg1->type&MDG_DISP_SIB_RIP) {
	  t1=arg1->type;
	  r1=arg1->reg;
	  d1=arg1->disp;
	} else {
	  if (arg1->type&MDF_IMM) {
	    if (!(builtin1=ICBuiltInFloatConst(arg1->disp(F64)))) {
	      t1=MDF_RIP_DISP32+RT_I64;
	      r1=REG_RIP;
	      d1=COCFloatConstFind(cc,arg1->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tmpi,arg1->type,arg1->reg,arg1->disp,rip);
	    t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	    rsp_size+=8;
	  }
	}
	if (arg2->type.raw_type>=RT_I64 && arg2->type&MDG_DISP_SIB_RIP) {
	  t2=arg2->type;
	  r2=arg2->reg;
	  d2=arg2->disp;
	} else {
	  if (arg2->type&MDF_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(arg2->disp(F64)))) {
	      t2=MDF_RIP_DISP32+RT_I64;
	      r2=REG_RIP;
	      d2=COCFloatConstFind(cc,arg2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tmpi,arg2->type,arg2->reg,arg2->disp,rip);
	    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	    rsp_size+=8;
	    if (r1==REG_RSP+REG_RSP<<8)
	      d1+=8;
	  }
	}
      } else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,arg1->type,
	      arg1->reg,arg1->disp,rip);
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg2->type,
	      arg2->reg,arg2->disp,rip);
	ICU16(tmpi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=8;
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      }
    } else {
      if (!(arg1->type&MDF_STK)) {
	if (arg2->type.raw_type>=RT_I64 && arg2->type&MDG_DISP_SIB_RIP) {
	  t2=arg2->type;
	  r2=arg2->reg;
	  d2=arg2->disp;
	} else {
	  if (arg2->type&MDF_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(arg2->disp(F64)))) {
	      t2=MDF_RIP_DISP32+RT_I64;
	      r2=REG_RIP;
	      d2=COCFloatConstFind(cc,arg2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tmpi,arg2->type,arg2->reg,arg2->disp,rip);
	    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	    rsp_size+=8;
	  }
	}
	if (arg1->type.raw_type>=RT_I64 && arg1->type&MDG_DISP_SIB_RIP) {
	  t1=arg1->type;
	  r1=arg1->reg;
	  d1=arg1->disp;
	} else {
	  if (arg1->type&MDF_IMM) {
	    if (!(builtin1=ICBuiltInFloatConst(arg1->disp(F64)))) {
	      t1=MDF_RIP_DISP32+RT_I64;
	      r1=REG_RIP;
	      d1=COCFloatConstFind(cc,arg1->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tmpi,arg1->type,arg1->reg,arg1->disp,rip);
	    t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	    rsp_size+=8;
	    if (r2==REG_RSP+REG_RSP<<8)
	      d2+=8;
	  }
	}
      } else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg2->type,
	      arg2->reg,arg2->disp,rip);
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,arg1->type,
	      arg1->reg,arg1->disp,rip);
	ICU16(tmpi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=8;
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      }
    }
  }
  if (!dont_pop_float && !rsp_size) {
    rsp_size=8;
    ICAddRSP(tmpi,-8);
  }
  if (!dont_push_float) {
    if (builtin2 && !builtin1) {
      alt=!alt;
      SwapI64(&t1,&t2);
      SwapI64(&r1,&r2);
      SwapI64(&d1,&d2);
      SwapI64(&builtin1,&builtin2);
    }
    if (builtin1)
      ICU16(tmpi,builtin1);
    else
      ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_FLD,rip);
  }
  if (alt)
    switch (op.u8[0]) {
      case 4: op=SLASH_OP_FSUBR; break;
      case 6: op=SLASH_OP_FDIVR; break;
    }
  if (builtin2) {
    ICU16(tmpi,builtin2);
    ICU16(tmpi,op.u16[2]);
  } else
    ICSlashOp(tmpi,t2,r2,d2,op,rip);
  CmpNoteFloatOp(cc,tmpi,TRUE,TRUE,CN_INST);
  if (dont_pop_float) {
    if (rsp_size)
      ICAddRSP(tmpi,rsp_size);
  } else {
    if (rsp_size==8)
      ICSlashOp(tmpi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,SLASH_OP_FSTP,rip);
    else if (rsp_size>8) {
      ICSlashOp(tmpi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,rsp_size-8,
	    SLASH_OP_FSTP,rip);
      ICAddRSP(tmpi,rsp_size-8);
    }
    ICPop(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,rip);
  }
}

U0 ICFCmp(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 op,I64 rip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (tmpi->ic_flags&ICF_ALT_TEMPLATE) {
      if (tmpi->ic_flags&ICF_POP_CMP)
	ICPopRegs(tmpi,1<<REG_RAX);
      else
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	      tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    } else
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
    if (tmpi->ic_flags&ICF_POP_CMP)
      ICPopRegs(tmpi,1<<REG_RDX);
    else
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
  }
  if (tmpi->ic_flags&ICF_PUSH_CMP)
    ICPushRegs(tmpi,1<<REG_RAX);
  ICCopyTemplate(cc,tmpi,op,FALSE,TRUE,FALSE,CN_INST);
  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	MDF_REG+RT_I64,REG_RAX,0,rip);
}

U0 ICFModEqu(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 rip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (dont_push_float) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,rip);
      if (tmpi->arg1_type_pointed_to!=RT_F64)
	ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,FALSE,rip);
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,rip);
      if (tmpi->arg1_type_pointed_to!=RT_F64)
	ICFCvt2(cc,tmpi,REG_RDX,MDF_REG+RT_I64,REG_RDX,0,FALSE,rip);
    }
//TODO: unpushable,unpop?  Not sure
    ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_INST);
    if (tmpi->arg1_type_pointed_to!=RT_F64)
      ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,TRUE,rip);
    ICMov(tmpi,tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	  tmpi->arg1.reg,tmpi->arg1.disp,MDF_REG+RT_I64,REG_RAX,0,rip);
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,rip);
  } else {
    if (dont_push_float) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,tmpi->arg1.type,
	    tmpi->arg1.reg,tmpi->arg1.disp,rip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,rip);
      if (tmpi->arg1_type_pointed_to!=RT_F64)
	ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,FALSE,rip);
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,tmpi->arg1.type,
	    tmpi->arg1.reg,tmpi->arg1.disp,rip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,rip);
      if (tmpi->arg1_type_pointed_to!=RT_F64)
	ICFCvt2(cc,tmpi,REG_RDX,MDF_REG+RT_I64,REG_RDX,0,FALSE,rip);
    }
//TODO: unpushable,unpop?  Not sure
    ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_INST);
    if (tmpi->arg1_type_pointed_to!=RT_F64)
      ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,TRUE,rip);
    ICMov(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,
	  MDF_REG+RT_I64,REG_RAX,0,rip);
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    MDF_REG+RT_I64,REG_RAX,0,rip);
  }
}
