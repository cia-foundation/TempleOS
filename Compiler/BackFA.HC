#define CN_A2	0
#define CN_A1	1
#define CN_MAIN	2
#define CN_R	3

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
      case CN_MAIN:
	if (cc->last_float_op_ic!=tmpi) {
	  if (cc->dont_push_float) {
	    if (intermediate_code_table[tmpi->ic_code].arg_cnt==IS_2_ARG &&
		  cc->last_float_op_ic->r.reg!=REG_R8)
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
      case CN_R:
	if (cc->last_float_op_ic==tmpi && cc->last_float_op_pos==CN_MAIN)
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
	CICType t2,I64 r2,I64 d2,Bool to_int,I64 pos,I64 ip)
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
      ICPush(tmpi,t2,r2,d2,ip);
      t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      rsp_size=8;
    } else {
      if (!dont_pop_float) {
	rsp_size=8;
	ICAddRSP(tmpi,-8);
      }
    }
    ICSlashOp(tmpi,t2,r2,d2,op1,ip);
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
    ICSlashOp(tmpi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,op2,ip);
    ICPop(tmpi,MDF_REG+RT_I64,r1,0,ip);
  }
}

U0 ICFCvt2(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 r1,
	CICType t2,I64 r2,I64 d2,Bool to_int,I64 ip)
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
    ICPush(tmpi,t2,r2,d2,ip);
    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
    rsp_size=8;
  } else {
    rsp_size=8;
    ICAddRSP(tmpi,-8);
  }
  ICSlashOp(tmpi,t2,r2,d2,op1,ip);
  ICSlashOp(tmpi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,op2,ip);
  ICPop(tmpi,MDF_REG+RT_I64,r1,0,ip);
  cc->last_dont_pushable=cc->last_dont_popable=FALSE; //TODO: improve this
}

U0 ICFUnaryMinus(CCmpCtrl *cc,CIntermediateCode *tmpi,U8 *buf2,I64 ip)
{
  CICArg *a1=&tmpi->a1;
  I64 rsp_size=0,builtin1=0,t1,r1,d1;
  Bool dont_push_float,dont_pop_float;

  if (cc->flags&CCF_AOT_COMPILE)
    buf2=cc->aotc->ip;

  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (!dont_push_float) {
    if (a1->type.raw_type>=RT_I64 && a1->type&MDG_DISP_SIB_RIP) {
      t1=a1->type;
      r1=a1->reg;
      d1=a1->disp;
    } else {
      if (a1->type&MDF_IMM) {
	if (!(builtin1=ICBuiltInFloatConst(a1->disp(F64)))) {
	  t1=MDF_RIP_DISP32+RT_I64;
	  r1=REG_RIP;
	  d1=COCFloatConstFind(cc,a1->disp(F64))+buf2;
	}
      } else {
	ICPush(tmpi,a1->type,a1->reg,a1->disp,ip);
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	rsp_size+=8;
      }
    }
    if (builtin1)
      ICU16(tmpi,builtin1);
    else
      ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_FLD,ip);
  }
  if (!dont_pop_float && !rsp_size) {
    rsp_size=8;
    ICAddRSP(tmpi,-8);
  }
  ICU16(tmpi,0xE0D9); //FCHS
  CmpNoteFloatOp(cc,tmpi,TRUE,TRUE,CN_MAIN);
  if (dont_pop_float) {
    if (rsp_size)
      ICAddRSP(tmpi,rsp_size);
  } else {
    ICSlashOp(tmpi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,SLASH_OP_FSTP,ip);
    ICPop(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,ip);
  }
}

U0 ICFMod(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 ip)
{//for MOD
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (tmpi->ic_flags&ICF_ALT_TEMPLATE)
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
    else
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
  }
//TODO: unpushable,unpop?  Not sure
  ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_MAIN);
  if (!dont_pop_float)
    ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	  MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICFPow(CCmpCtrl *cc,CIntermediateCode *tmpi,U8 *buf,I64 ip)
{//for POW
  I64 i;
  CAOTImportExport *tmpie;
  CHashExport *tmpex=HashFind("SYS_POW",
	cc->htc.hash_table_lst,HTT_EXPORT_SYS_SYM);

  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
  ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
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
	  tmpie->ip=ip+tmpi->ic_cnt-8;
	  tmpie->next=tmpex->ie_lst;
	  tmpex->ie_lst=tmpie;
	}
	ICU16(tmpi,0xD3FF);
      } else {
	ICU8(tmpi,0xE8);
	ICU32(tmpi,-(ip+tmpi->ic_cnt+4));
	if (buf) {
	  tmpie=CAlloc(sizeof(CAOTImportExport));
	  tmpie->type=IET_REL_I32;
	  tmpie->ip=ip+tmpi->ic_cnt-4;
	  tmpie->next=tmpex->ie_lst;
	  tmpex->ie_lst=tmpie;
	}
      }
    } else {//Kernel
      if (tmpex->type&HTF_UNRESOLVED)
	throw('Compiler');
      else {
	i=tmpex->val-(ip+tmpi->ic_cnt+5);
	if (!(MIN_I32<=i<=MAX_I32)) {
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
    i=tmpex->val-(ip+tmpi->ic_cnt+5);
    if (!(MIN_I32<=i<=MAX_I32)) {
      ICU16(tmpi,0xBB48);
      ICU64(tmpi,tmpex->val);
      ICU16(tmpi,0xD3FF);
    } else {
      ICU8(tmpi,0xE8);
      ICU32(tmpi,i);
    }
  }
  tmpi->ic_flags&=~ICF_CODE_FINAL;
  ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICFOp(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 op,U8 *buf2,I64 ip)
{//for ADD,SUB,DIV,MUL
  CICArg *a1,*a2;
  Bool dont_push_float,dont_pop_float,alt;
  I64 rsp_size=0,builtin1=0,builtin2=0,t1,r1,d1,t2,r2,d2;

  if (tmpi->ic_flags&ICF_ALT_TEMPLATE) {
    a1=&tmpi->a2;
    a2=&tmpi->a1;
    alt=TRUE;
  } else {
    a1=&tmpi->a1;
    a2=&tmpi->a2;
    alt=FALSE;
  }

  if (cc->flags&CCF_AOT_COMPILE)
    buf2=cc->aotc->ip;

  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (a2->type.raw_type>=RT_I64 && a2->type&MDG_DISP_SIB_RIP) {
      t2=a2->type;
      r2=a2->reg;
      d2=a2->disp;
    } else {
      if (a2->type&MDF_IMM) {
	if (!(builtin2=ICBuiltInFloatConst(a2->disp(F64)))) {
	  t2=MDF_RIP_DISP32+RT_I64;
	  r2=REG_RIP;
	  d2=COCFloatConstFind(cc,a2->disp(F64))+buf2;
	}
      } else {
	ICPush(tmpi,a2->type,a2->reg,a2->disp,ip);
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	rsp_size+=8;
      }
    }
  } else {
    if (alt) {
      if (!(a2->type&MDF_STK)) {
	if (a1->type.raw_type>=RT_I64 && a1->type&MDG_DISP_SIB_RIP) {
	  t1=a1->type;
	  r1=a1->reg;
	  d1=a1->disp;
	} else {
	  if (a1->type&MDF_IMM) {
	    if (!(builtin1=ICBuiltInFloatConst(a1->disp(F64)))) {
	      t1=MDF_RIP_DISP32+RT_I64;
	      r1=REG_RIP;
	      d1=COCFloatConstFind(cc,a1->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tmpi,a1->type,a1->reg,a1->disp,ip);
	    t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	    rsp_size+=8;
	  }
	}
	if (a2->type.raw_type>=RT_I64 && a2->type&MDG_DISP_SIB_RIP) {
	  t2=a2->type;
	  r2=a2->reg;
	  d2=a2->disp;
	} else {
	  if (a2->type&MDF_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(a2->disp(F64)))) {
	      t2=MDF_RIP_DISP32+RT_I64;
	      r2=REG_RIP;
	      d2=COCFloatConstFind(cc,a2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tmpi,a2->type,a2->reg,a2->disp,ip);
	    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	    rsp_size+=8;
	    if (r1==REG_RSP+REG_RSP<<8)
	      d1+=8;
	  }
	}
      } else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip);
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip);
	ICU16(tmpi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=8;
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      }
    } else {
      if (!(a1->type&MDF_STK)) {
	if (a2->type.raw_type>=RT_I64 && a2->type&MDG_DISP_SIB_RIP) {
	  t2=a2->type;
	  r2=a2->reg;
	  d2=a2->disp;
	} else {
	  if (a2->type&MDF_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(a2->disp(F64)))) {
	      t2=MDF_RIP_DISP32+RT_I64;
	      r2=REG_RIP;
	      d2=COCFloatConstFind(cc,a2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tmpi,a2->type,a2->reg,a2->disp,ip);
	    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	    rsp_size+=8;
	  }
	}
	if (a1->type.raw_type>=RT_I64 && a1->type&MDG_DISP_SIB_RIP) {
	  t1=a1->type;
	  r1=a1->reg;
	  d1=a1->disp;
	} else {
	  if (a1->type&MDF_IMM) {
	    if (!(builtin1=ICBuiltInFloatConst(a1->disp(F64)))) {
	      t1=MDF_RIP_DISP32+RT_I64;
	      r1=REG_RIP;
	      d1=COCFloatConstFind(cc,a1->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tmpi,a1->type,a1->reg,a1->disp,ip);
	    t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	    rsp_size+=8;
	    if (r2==REG_RSP+REG_RSP<<8)
	      d2+=8;
	  }
	}
      } else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip);
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip);
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
      ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_FLD,ip);
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
    ICSlashOp(tmpi,t2,r2,d2,op,ip);
  CmpNoteFloatOp(cc,tmpi,TRUE,TRUE,CN_MAIN);
  if (dont_pop_float) {
    if (rsp_size)
      ICAddRSP(tmpi,rsp_size);
  } else {
    if (rsp_size==8)
      ICSlashOp(tmpi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,SLASH_OP_FSTP,ip);
    else if (rsp_size>8) {
      ICSlashOp(tmpi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,rsp_size-8,
	    SLASH_OP_FSTP,ip);
      ICAddRSP(tmpi,rsp_size-8);
    }
    ICPop(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,ip);
  }
}

U0 ICFCmp(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 op,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (tmpi->ic_flags&ICF_ALT_TEMPLATE) {
      if (tmpi->ic_flags&ICF_POP_CMP)
	ICPopRegs(tmpi,1<<REG_RAX);
      else
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	      tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
    } else
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
    if (tmpi->ic_flags&ICF_POP_CMP)
      ICPopRegs(tmpi,1<<REG_RDX);
    else
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
  }
  if (tmpi->ic_flags&ICF_PUSH_CMP)
    ICPushRegs(tmpi,1<<REG_RAX);
  ICCopyTemplate(cc,tmpi,op,FALSE,TRUE,FALSE,CN_MAIN);
  ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICFModEqu(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (dont_push_float) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,ip);
      if (tmpi->a1_type_pointed_to!=RT_F64)
	ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,FALSE,ip);
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,ip);
      if (tmpi->a1_type_pointed_to!=RT_F64)
	ICFCvt2(cc,tmpi,REG_RDX,MDF_REG+RT_I64,REG_RDX,0,FALSE,ip);
    }
//TODO: unpushable,unpop?  Not sure
    ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_MAIN);
    if (tmpi->a1_type_pointed_to!=RT_F64)
      ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,TRUE,ip);
    ICMov(tmpi,tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	  tmpi->a1.reg,tmpi->a1.disp,MDF_REG+RT_I64,REG_RAX,0,ip);
    if (tmpi->r.type.mode)
      ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,ip);
  } else {
    if (dont_push_float) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,tmpi->a1.type,
	    tmpi->a1.reg,tmpi->a1.disp,ip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    MDF_DISP+tmpi->a1_type_pointed_to,REG_RCX,0,ip);
      if (tmpi->a1_type_pointed_to!=RT_F64)
	ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,FALSE,ip);
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,tmpi->a1.type,
	    tmpi->a1.reg,tmpi->a1.disp,ip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    MDF_DISP+tmpi->a1_type_pointed_to,REG_RCX,0,ip);
      if (tmpi->a1_type_pointed_to!=RT_F64)
	ICFCvt2(cc,tmpi,REG_RDX,MDF_REG+RT_I64,REG_RDX,0,FALSE,ip);
    }
//TODO: unpushable,unpop?  Not sure
    ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_MAIN);
    if (tmpi->a1_type_pointed_to!=RT_F64)
      ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,TRUE,ip);
    ICMov(tmpi,MDF_DISP+tmpi->a1_type_pointed_to,REG_RCX,0,
	  MDF_REG+RT_I64,REG_RAX,0,ip);
    if (tmpi->r.type.mode)
      ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	    MDF_REG+RT_I64,REG_RAX,0,ip);
  }
}
