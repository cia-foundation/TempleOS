#define CN_A2	0
#define CN_A1	1
#define CN_MAIN	2
#define CN_R	3

U0 CmpNoteFloatOp(CCmpCtrl *cc,CIntermediateCode *tempi,
	Bool dont_pushable,Bool dont_popable,I64 pos)
{
  Bool link=FALSE;
  if (cc->pass==7 && cc->last_float_op_ic &&
	cc->last_dont_popable && dont_pushable) {
    switch [pos] {
      case CN_A2:
	if (cc->last_float_op_ic!=tempi && cc->dont_push_float)
	  link=TRUE;
	break;
      case CN_A1:
	if (cc->last_float_op_ic!=tempi && cc->dont_push_float)
	  link=TRUE;
	break;
      case CN_MAIN:
	if (cc->last_float_op_ic!=tempi) {
	  if (cc->dont_push_float) {
	    if (intermediate_code_table[tempi->ic_code].arg_cnt==IS_2_ARG &&
		  cc->last_float_op_ic->r.reg!=REG_R8)
	      tempi->ic_flags|=ICF_ALT_TEMPLATE;
	    else
	      tempi->ic_flags&=~ICF_ALT_TEMPLATE;
	    link=TRUE;
	  }
	} else {
	  if (intermediate_code_table[tempi->ic_code].arg_cnt==IS_2_ARG &&
		cc->last_float_op_pos!=CN_A1)
	    tempi->ic_flags|=ICF_ALT_TEMPLATE;
	  else
	    tempi->ic_flags&=~ICF_ALT_TEMPLATE;
	  link=TRUE;
	}
	break;
      case CN_R:
	if (cc->last_float_op_ic==tempi && cc->last_float_op_pos==CN_MAIN)
	  link=TRUE;
	break;
    }
    if (link) {
      if (!Bts(&cc->last_float_op_ic->ic_flags,
	    ICf_DONT_POP_FLOAT0+cc->last_ic_float_op_num))
	cc->last_float_op_ic->ic_flags&=~ICF_CODE_FINAL;
      if (!Bts(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0+cc->cur_ic_float_op_num))
	tempi->ic_flags&=~ICF_CODE_FINAL;
    }
  }
  cc->last_float_op_ic=tempi;
  cc->last_dont_pushable=dont_pushable;
  cc->last_dont_popable=dont_popable;
  cc->last_ic_float_op_num=cc->cur_ic_float_op_num++;
  cc->last_float_op_pos=pos;
  if (cc->cur_ic_float_op_num>4)
    throw('Compiler');
}

U0 CmpSetFloatOpPushPop(CCmpCtrl *cc,CIntermediateCode *tempi,
	Bool *dont_push_float,Bool *dont_pop_float)
{
  if (cc->pass==7) {
    *dont_push_float=FALSE;
    *dont_pop_float =FALSE;
    tempi->ic_flags&=~ICF_CODE_FINAL;
  } else {
    *dont_push_float=Bt(&tempi->ic_flags,
	  ICf_DONT_PUSH_FLOAT0+cc->cur_ic_float_op_num);
    *dont_pop_float=Bt(&tempi->ic_flags,
	  ICf_DONT_POP_FLOAT0+cc->cur_ic_float_op_num);
  }
}

U0 ICCopyTemplate(CCmpCtrl *cc,CIntermediateCode *tempi,I64 op,
  Bool off_the_record,Bool dont_pushable,Bool dont_popable,I64 pos)
{
  Bool dont_push_float,dont_pop_float,alt;
  U8 *ptr;
  I64 i=0;
  if (!off_the_record) {
    if (tempi->ic_flags&ICF_ALT_TEMPLATE)
      alt=TRUE;
    else
      alt=FALSE;
    CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
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
  MemCpy(&tempi->ic_body[tempi->ic_cnt],ptr,i);
  if (!off_the_record)
    CmpNoteFloatOp(cc,tempi,dont_pushable,dont_popable,pos);
  tempi->ic_cnt+=i;
}

U0 ICFCvt(CCmpCtrl *cc,CIntermediateCode *tempi,I64 r1,
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

  CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
  if (!dont_push_float) {
    if (!(t2.raw_type>=RT_I64 && t2&MDG_DISP_SIB_RIP)) {
      ICPush(tempi,t2,r2,d2,ip);
      t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      rsp_size=8;
    } else {
      if (!dont_pop_float) {
	rsp_size=8;
	ICAddRSP(tempi,-8);
      }
    }
    ICSlashOp(tempi,t2,r2,d2,op1,ip);
  } else {
    if (!dont_pop_float) {
      rsp_size=8;
      ICAddRSP(tempi,-8);
    }
  }
  if (to_int)
    CmpNoteFloatOp(cc,tempi,TRUE,FALSE,pos);
  else
    CmpNoteFloatOp(cc,tempi,FALSE,TRUE,pos);
  if (dont_pop_float) {
    if (rsp_size)
      ICAddRSP(tempi,rsp_size);
  } else {
    ICSlashOp(tempi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,op2,ip);
    ICPop(tempi,MDF_REG+RT_I64,r1,0,ip);
  }
}

U0 ICFCvt2(CCmpCtrl *cc,CIntermediateCode *tempi,I64 r1,
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
    ICPush(tempi,t2,r2,d2,ip);
    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
    rsp_size=8;
  } else {
    rsp_size=8;
    ICAddRSP(tempi,-8);
  }
  ICSlashOp(tempi,t2,r2,d2,op1,ip);
  ICSlashOp(tempi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,op2,ip);
  ICPop(tempi,MDF_REG+RT_I64,r1,0,ip);
  cc->last_dont_pushable=cc->last_dont_popable=FALSE; //TODO: improve this
}

U0 ICFUnaryMinus(CCmpCtrl *cc,CIntermediateCode *tempi,U8 *buf2,I64 ip)
{
  CICArg *a1=&tempi->a1;
  I64 rsp_size=0,builtin1=0,t1,r1,d1;
  Bool dont_push_float,dont_pop_float;

  if (cc->flags&CCF_AOT_COMPILE)
    buf2=cc->aotc->ip;

  CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
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
	ICPush(tempi,a1->type,a1->reg,a1->disp,ip);
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	rsp_size+=8;
      }
    }
    if (builtin1)
      ICU16(tempi,builtin1);
    else
      ICSlashOp(tempi,t1,r1,d1,SLASH_OP_FLD,ip);
  }
  if (!dont_pop_float && !rsp_size) {
    rsp_size=8;
    ICAddRSP(tempi,-8);
  }
  ICU16(tempi,0xE0D9); //FCHS
  CmpNoteFloatOp(cc,tempi,TRUE,TRUE,CN_MAIN);
  if (dont_pop_float) {
    if (rsp_size)
      ICAddRSP(tempi,rsp_size);
  } else {
    ICSlashOp(tempi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,SLASH_OP_FSTP,ip);
    ICPop(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,ip);
  }
}

U0 ICFMod(CCmpCtrl *cc,CIntermediateCode *tempi,I64 ip)
{//for MOD
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (tempi->ic_flags&ICF_ALT_TEMPLATE)
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
    else
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
  } else {
    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
  }
//TODO: unpushable,unpop?  Not sure
  ICCopyTemplate(cc,tempi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_MAIN);
  if (!dont_pop_float)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICFPow(CCmpCtrl *cc,CIntermediateCode *tempi,U8 *buf,I64 ip)
{//for POW
  I64 i;
  CAOTImportExport *tempie;
  CHashExport *tempex=HashFind("SYS_POW",
	cc->htc.hash_table_lst,HTT_EXPORT_SYS_SYM);

  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
  ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
  if (cc->flags&CCF_AOT_COMPILE) {
    if (!tempex) {
      tempex=CAlloc(sizeof(CHashExport));
      tempex->str=StrNew("SYS_POW");
      tempex->type=HTT_EXPORT_SYS_SYM|HTF_UNRESOLVED|HTF_IMPORT;
      HashAdd(tempex,cc->htc.glbl_hash_table);
    }
    if (tempex->type&HTF_IMPORT) {
      if (GetOption(OPTf_USE_IMM64)) {
	ICU16(tempi,0xBB48);
	ICU64(tempi,0);
	if (buf) {
	  tempie=CAlloc(sizeof(CAOTImportExport));
	  tempie->type=IET_IMM_I64;
	  tempie->ip=ip+tempi->ic_cnt-8;
	  tempie->next=tempex->ie_lst;
	  tempex->ie_lst=tempie;
	}
	ICU16(tempi,0xD3FF);
      } else {
	ICU8(tempi,0xE8);
	ICU32(tempi,-(ip+tempi->ic_cnt+4));
	if (buf) {
	  tempie=CAlloc(sizeof(CAOTImportExport));
	  tempie->type=IET_REL_I32;
	  tempie->ip=ip+tempi->ic_cnt-4;
	  tempie->next=tempex->ie_lst;
	  tempex->ie_lst=tempie;
	}
      }
    } else {//Kernel
      if (tempex->type&HTF_UNRESOLVED)
	throw('Compiler');
      else {
	i=tempex->val-(ip+tempi->ic_cnt+5);
	if (!(MIN_I32<=i<=MAX_I32)) {
	  throw('Compiler');
//	    ICU16(tempi,0xBB48);
	  //	    ICU64(tempi,tempex->val);
	  //	    ICU16(tempi,0xD3FF);
	} else {
	  ICU8(tempi,0xE8);
	  ICU32(tempi,i);
	}
      }
    }
  } else {
    i=tempex->val-(ip+tempi->ic_cnt+5);
    if (!(MIN_I32<=i<=MAX_I32)) {
      ICU16(tempi,0xBB48);
      ICU64(tempi,tempex->val);
      ICU16(tempi,0xD3FF);
    } else {
      ICU8(tempi,0xE8);
      ICU32(tempi,i);
    }
  }
  tempi->ic_flags&=~ICF_CODE_FINAL;
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICFOp(CCmpCtrl *cc,CIntermediateCode *tempi,I64 op,U8 *buf2,I64 ip)
{//for ADD,SUB,DIV,MUL
  CICArg *a1,*a2;
  Bool dont_push_float,dont_pop_float,alt;
  I64 rsp_size=0,builtin1=0,builtin2=0,t1,r1,d1,t2,r2,d2;

  if (tempi->ic_flags&ICF_ALT_TEMPLATE) {
    a1=&tempi->a2;
    a2=&tempi->a1;
    alt=TRUE;
  } else {
    a1=&tempi->a1;
    a2=&tempi->a2;
    alt=FALSE;
  }

  if (cc->flags&CCF_AOT_COMPILE)
    buf2=cc->aotc->ip;

  CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
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
	ICPush(tempi,a2->type,a2->reg,a2->disp,ip);
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
	    ICPush(tempi,a1->type,a1->reg,a1->disp,ip);
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
	    ICPush(tempi,a2->type,a2->reg,a2->disp,ip);
	    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	    rsp_size+=8;
	    if (r1==REG_RSP+REG_RSP<<8)
	      d1+=8;
	  }
	}
      } else {
	ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip);
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip);
	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
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
	    ICPush(tempi,a2->type,a2->reg,a2->disp,ip);
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
	    ICPush(tempi,a1->type,a1->reg,a1->disp,ip);
	    t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	    rsp_size+=8;
	    if (r2==REG_RSP+REG_RSP<<8)
	      d2+=8;
	  }
	}
      } else {
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip);
	ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip);
	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=8;
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      }
    }
  }
  if (!dont_pop_float && !rsp_size) {
    rsp_size=8;
    ICAddRSP(tempi,-8);
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
      ICU16(tempi,builtin1);
    else
      ICSlashOp(tempi,t1,r1,d1,SLASH_OP_FLD,ip);
  }
  if (alt)
    switch (op.u8[0]) {
      case 4: op=SLASH_OP_FSUBR; break;
      case 6: op=SLASH_OP_FDIVR; break;
    }
  if (builtin2) {
    ICU16(tempi,builtin2);
    ICU16(tempi,op.u16[2]);
  } else
    ICSlashOp(tempi,t2,r2,d2,op,ip);
  CmpNoteFloatOp(cc,tempi,TRUE,TRUE,CN_MAIN);
  if (dont_pop_float) {
    if (rsp_size)
      ICAddRSP(tempi,rsp_size);
  } else {
    if (rsp_size==8)
      ICSlashOp(tempi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,0,SLASH_OP_FSTP,ip);
    else if (rsp_size>8) {
      ICSlashOp(tempi,MDF_SIB+RT_I64,REG_RSP+REG_RSP<<8,rsp_size-8,
	    SLASH_OP_FSTP,ip);
      ICAddRSP(tempi,rsp_size-8);
    }
    ICPop(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,ip);
  }
}

U0 ICFCmp(CCmpCtrl *cc,CIntermediateCode *tempi,I64 op,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (tempi->ic_flags&ICF_ALT_TEMPLATE) {
      if (tempi->ic_flags&ICF_POP_CMP)
	ICPopRegs(tempi,1<<REG_RAX);
      else
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	      tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
    } else
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
  } else {
    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    if (tempi->ic_flags&ICF_POP_CMP)
      ICPopRegs(tempi,1<<REG_RDX);
    else
      ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
	    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
  }
  if (tempi->ic_flags&ICF_PUSH_CMP)
    ICPushRegs(tempi,1<<REG_RAX);
  ICCopyTemplate(cc,tempi,op,FALSE,TRUE,FALSE,CN_MAIN);
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICFModEqu(CCmpCtrl *cc,CIntermediateCode *tempi,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
  if (tempi->ic_flags & ICF_BY_VAL) {
    if (dont_push_float) {
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    tempi->a1.type&MDG_MASK+tempi->a1.type_pointed_to,
	    tempi->a1.reg,tempi->a1.disp,ip);
      if (tempi->a1.type_pointed_to!=RT_F64)
	ICFCvt2(cc,tempi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,FALSE,ip);
    } else {
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
      ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
	    tempi->a1.type&MDG_MASK+tempi->a1.type_pointed_to,
	    tempi->a1.reg,tempi->a1.disp,ip);
      if (tempi->a1.type_pointed_to!=RT_F64)
	ICFCvt2(cc,tempi,REG_RDX,MDF_REG+RT_I64,REG_RDX,0,FALSE,ip);
    }
//TODO: unpushable,unpop?  Not sure
    ICCopyTemplate(cc,tempi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_MAIN);
    if (tempi->a1.type_pointed_to!=RT_F64)
      ICFCvt2(cc,tempi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,TRUE,ip);
    ICMov(tempi,tempi->a1.type&MDG_MASK+tempi->a1.type_pointed_to,
	  tempi->a1.reg,tempi->a1.disp,MDF_REG+RT_I64,REG_RAX,0,ip);
    if (tempi->r.type.mode)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    tempi->a1.type&MDG_MASK+tempi->a1.type_pointed_to,
	    tempi->a1.reg,tempi->a1.disp,ip);
  } else {
    if (dont_push_float) {
      ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,tempi->a1.type,
	    tempi->a1.reg,tempi->a1.disp,ip);
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    MDF_DISP+tempi->a1.type_pointed_to,REG_RCX,0,ip);
      if (tempi->a1.type_pointed_to!=RT_F64)
	ICFCvt2(cc,tempi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,FALSE,ip);
    } else {
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
      ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,tempi->a1.type,
	    tempi->a1.reg,tempi->a1.disp,ip);
      ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
	    MDF_DISP+tempi->a1.type_pointed_to,REG_RCX,0,ip);
      if (tempi->a1.type_pointed_to!=RT_F64)
	ICFCvt2(cc,tempi,REG_RDX,MDF_REG+RT_I64,REG_RDX,0,FALSE,ip);
    }
//TODO: unpushable,unpop?  Not sure
    ICCopyTemplate(cc,tempi,CMP_TEMPLATE_MOD,FALSE,FALSE,FALSE,CN_MAIN);
    if (tempi->a1.type_pointed_to!=RT_F64)
      ICFCvt2(cc,tempi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,TRUE,ip);
    ICMov(tempi,MDF_DISP+tempi->a1.type_pointed_to,REG_RCX,0,
	  MDF_REG+RT_I64,REG_RAX,0,ip);
    if (tempi->r.type.mode)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    MDF_REG+RT_I64,REG_RAX,0,ip);
  }
}
