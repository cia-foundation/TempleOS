U0 ICFOpEqu(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 op,U8 *buf2,I64 rip)
{//for ADD,SUB,DIV,MUL
  CICArg *arg1=&tmpi->arg1,
	*arg2=&tmpi->arg2;
  Bool dont_push_float,dont_pop_float,p1_mem;
  I64 rsp_size=0,builtin2=0,
	t1,r1,d1,t2,r2,d2;

  if (cc->flags&CCF_AOT_COMPILE)
    buf2=cc->aotc->rip;

  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (dont_pop_float)
    throw('Compiler');

  if (tmpi->ic_flags & ICF_BY_VAL) {
    p1_mem=FALSE;
    if (dont_push_float) {
      if (tmpi->arg1_type_pointed_to!=RT_F64) {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	      arg1->type&MDG_MASK+tmpi->arg1_type_pointed_to,
	      arg1->reg,arg1->disp,rip);
	ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,FALSE,rip);
	ICPush(tmpi,MDF_REG+RT_I64,REG_RAX,0,rip);
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	rsp_size+=8;
      } else {
	if (tmpi->arg1_type_pointed_to>=RT_I64 && arg1->type&MDG_DISP_SIB_RIP) {
	  t1=arg1->type&MDG_MASK+tmpi->arg1_type_pointed_to;
	  r1=arg1->reg;
	  d1=arg1->disp;
	  p1_mem=TRUE;
	} else {
	  ICPush(tmpi,arg1->type&MDG_MASK+tmpi->arg1_type_pointed_to,
		arg1->reg,arg1->disp,rip);
	  t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	  rsp_size+=8;
	}
      }
    } else {
      if (tmpi->arg1_type_pointed_to!=RT_F64 || arg1->type&MDF_STK) {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg2->type,
	      arg2->reg,arg2->disp,rip);
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	      arg1->type&MDG_MASK+tmpi->arg1_type_pointed_to,
	      arg1->reg,arg1->disp,rip);
	if (tmpi->arg1_type_pointed_to!=RT_F64)
	  ICFCvt2(cc,tmpi,REG_RDX,MDF_REG+RT_I64,REG_RDX,0,FALSE,rip);
	ICU16(tmpi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=8;
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      } else {
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
	if (tmpi->arg1_type_pointed_to>=RT_I64 && arg1->type&MDG_DISP_SIB_RIP) {
	  t1=arg1->type&MDG_MASK+tmpi->arg1_type_pointed_to;
	  r1=arg1->reg;
	  d1=arg1->disp;
	  p1_mem=TRUE;
	} else {
	  ICPush(tmpi,arg1->type&MDG_MASK+tmpi->arg1_type_pointed_to,
		arg1->reg,arg1->disp,rip);
	  t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	  rsp_size+=8;
	  if (r2==REG_RSP+REG_RSP<<8)
	    d2+=8;
	}
      }
    }
    if (!rsp_size && !(p1_mem && tmpi->arg1_type_pointed_to==RT_F64)) {
      rsp_size=8;
      ICAddRSP(tmpi,-8);
    }
    if (!dont_push_float) {
      if (builtin2)
	ICU16(tmpi,builtin2);
      else
	ICSlashOp(tmpi,t2,r2,d2,SLASH_OP_FLD,rip);
    }
    switch (op.u8[0]) {
      case 4: op=SLASH_OP_FSUBR; break;
      case 6: op=SLASH_OP_FDIVR; break;
    }
    ICSlashOp(tmpi,t1,r1,d1,op,rip);
    CmpNoteFloatOp(cc,tmpi,TRUE,FALSE,CN_INST);
    if (p1_mem && tmpi->arg1_type_pointed_to==RT_F64) {
      ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_FSTP,rip);
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
      if (tmpi->arg1_type_pointed_to!=RT_F64) {
	ICPop(tmpi,MDF_REG+RT_I64,REG_RAX,0,rip);
	ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,TRUE,rip);
	ICMov(tmpi,arg1->type&MDG_MASK+tmpi->arg1_type_pointed_to,arg1->reg,
	      arg1->disp,MDF_REG+RT_I64,REG_RAX,0,rip);
      } else
	ICPop(tmpi,arg1->type&MDG_MASK+tmpi->arg1_type_pointed_to,
	      arg1->reg,arg1->disp,rip);
    }
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    arg1->type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    arg1->reg,arg1->disp,rip);
  } else {
    if (tmpi->arg1_type_pointed_to>=RT_I64)
      p1_mem=TRUE;
    else
      p1_mem=FALSE;
    if (dont_push_float) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,arg1->type,arg1->reg,arg1->disp,rip);
      if (tmpi->arg1_type_pointed_to!=RT_F64) {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	      MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,rip);
	ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,FALSE,rip);
	ICPush(tmpi,MDF_REG+RT_I64,REG_RAX,0,rip);
      } else
	ICPush(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,rip);
      t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
      rsp_size+=8;
    } else {
      if (tmpi->arg1_type_pointed_to!=RT_F64 || arg1->type&MDF_STK) {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg2->type,
	      arg2->reg,arg2->disp,rip);
	ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,arg1->type,
	      arg1->reg,arg1->disp,rip);
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	      MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,rip);
	if (tmpi->arg1_type_pointed_to!=RT_F64)
	  ICFCvt2(cc,tmpi,REG_RDX,MDF_REG+RT_I64,REG_RDX,0,FALSE,rip);
	ICU16(tmpi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=8;
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      } else {
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
	ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,arg1->type,
	      arg1->reg,arg1->disp,rip);
	ICPush(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,rip);
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	rsp_size+=8;
	if (r2==REG_RSP+REG_RSP<<8)
	  d2+=8;
      }
    }
    if (!rsp_size && !(p1_mem && tmpi->arg1_type_pointed_to==RT_F64)) {
      rsp_size=8;
      ICAddRSP(tmpi,-8);
    }
    if (!dont_push_float) {
      if (builtin2)
	ICU16(tmpi,builtin2);
      else
	ICSlashOp(tmpi,t2,r2,d2,SLASH_OP_FLD,rip);
    }

    switch (op.u8[0]) {
      case 4: op=SLASH_OP_FSUBR; break;
      case 6: op=SLASH_OP_FDIVR; break;
    }
    ICSlashOp(tmpi,t1,r1,d1,op,rip);
    CmpNoteFloatOp(cc,tmpi,TRUE,FALSE,CN_INST);
    if (p1_mem && tmpi->arg1_type_pointed_to==RT_F64) {
      ICSlashOp(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,
	    REG_RCX,0,SLASH_OP_FSTP,rip);
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
      ICPop(tmpi,MDF_REG+RT_I64,REG_RAX,0,rip);
      if (tmpi->arg1_type_pointed_to!=RT_F64)
	ICFCvt2(cc,tmpi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,TRUE,rip);
      ICMov(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,
	    MDF_REG+RT_I64,REG_RAX,0,rip);
    }
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    MDF_REG+RT_I64,REG_RAX,0,rip);
  }
}
 
U0 ICFCmpAndBranch(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 rip,
   I64 us,I64 not_us,U8 *buf,I64 rip2)
{
  CICArg *arg1,*arg2;
  Bool dont_push_float,dont_pop_float,alt,short_jmp;
  I64 i,rsp_size=0,builtin1=0,builtin2=0,t1,r1,d1,t2,r2,d2;
  CCodeMisc *lb;
  U8 *buf2;

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
  else
    buf2=buf;

  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (tmpi->ic_flags&ICF_POP_CMP && alt) {
      t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      rsp_size+=8;
    } else {
      if (arg2->type.raw_type>=RT_I64 && arg2->type&MDG_DISP_SIB_RIP) {
	t2=arg2->type;
	r2=arg2->reg;
	d2=arg2->disp;
      } else {
	if (arg2->type&MDF_IMM) {
	  if (!(builtin2=ICBuiltInFloatConst(arg2->disp(F64))) ||
		tmpi->ic_flags&ICF_PUSH_CMP) {
	    t2=MDF_RIP_DISP32+RT_I64;
	    r2=REG_RIP;
	    d2=COCFloatConstFind(cc,arg2->disp(F64))+buf2;
	  }
	} else {
	  ICPush(tmpi,arg2->type,arg2->reg,arg2->disp,rip2);
	  t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	  rsp_size+=8;
	}
      }
    }
  } else {
    if (alt) {
      if (!(arg2->type&MDF_STK)) {
	if (tmpi->ic_flags&ICF_POP_CMP) {
	  t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	  rsp_size+=8;
	} else {
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
	      ICPush(tmpi,arg1->type,arg1->reg,arg1->disp,rip2);
	      t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	      rsp_size+=8;
	    }
	  }
	}
	if (arg2->type.raw_type>=RT_I64 && arg2->type&MDG_DISP_SIB_RIP) {
	  t2=arg2->type;
	  r2=arg2->reg;
	  d2=arg2->disp;
	} else {
	  if (arg2->type&MDF_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(arg2->disp(F64))) ||
		  tmpi->ic_flags&ICF_PUSH_CMP) {
	      t2=MDF_RIP_DISP32+RT_I64;
	      r2=REG_RIP;
	      d2=COCFloatConstFind(cc,arg2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tmpi,arg2->type,arg2->reg,arg2->disp,rip2);
	    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	    rsp_size+=8;
	    if (r1==REG_RSP+REG_RSP<<8)
	      d1+=8;
	  }
	}
      } else {
	if (tmpi->ic_flags&ICF_POP_CMP)
	  ICPopRegs(tmpi,1<<REG_RDX);
	else
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,arg1->type,
		arg1->reg,arg1->disp,rip2);
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg2->type,
	      arg2->reg,arg2->disp,rip2);
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
	    if (!(builtin2=ICBuiltInFloatConst(arg2->disp(F64))) ||
		  tmpi->ic_flags&ICF_PUSH_CMP) {
	      t2=MDF_RIP_DISP32+RT_I64;
	      r2=REG_RIP;
	      d2=COCFloatConstFind(cc,arg2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tmpi,arg2->type,arg2->reg,arg2->disp,rip2);
	    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	    rsp_size+=8;
	  }
	}
	if (tmpi->ic_flags&ICF_POP_CMP) {
	  t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	  rsp_size+=8;
	  if (r2==REG_RSP+REG_RSP<<8)
	    d1+=8;
	} else {
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
	      ICPush(tmpi,arg1->type,arg1->reg,arg1->disp,rip2);
	      t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	      rsp_size+=8;
	      if (r2==REG_RSP+REG_RSP<<8)
		d2+=8;
	    }
	  }
	}
      } else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg2->type,
	      arg2->reg,arg2->disp,rip2);
	if (tmpi->ic_flags&ICF_POP_CMP)
	  ICPopRegs(tmpi,1<<REG_RDX);
	else
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,arg1->type,
		arg1->reg,arg1->disp,rip2);
	ICU16(tmpi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=8;
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      }
    }
  }
  if (!dont_push_float) {
    if (builtin1)
      ICU16(tmpi,builtin1);
    else
      ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_FLD,rip2);
  }
  if (!alt)
    us=not_us;
  if (builtin2)
    ICU16(tmpi,builtin2);
  else
    ICSlashOp(tmpi,t2,r2,d2,SLASH_OP_FLD,rip2);
  if (tmpi->ic_flags&ICF_PUSH_CMP) {
    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
    if (!rsp_size) {
      rsp_size=8;
      ICAddRSP(tmpi,-8);
    } else if (rsp_size==16)
      d2=8;

    if (alt) {
      ICU16(tmpi,0xF1DF);	  //FCOMIP
      ICSlashOp(tmpi,t2,r2,d2,SLASH_OP_FSTP,rip2);
    } else {
      ICU16(tmpi,0xF1DB);	  //FCOMI
      ICSlashOp(tmpi,t2,r2,d2,SLASH_OP_FSTP,rip2);
      ICU32(tmpi,0xF7D9C0DD); //FFREE,FINCSTP
    }
  } else {
    ICU16(tmpi,0xF1DF);	  //FCOMIP
    ICU32(tmpi,0xF7D9C0DD); //FFREE,FINCSTP
  }
  CmpNoteFloatOp(cc,tmpi,TRUE,FALSE,CN_INST);

  if (tmpi->ic_flags&ICF_PUSH_CMP) {
    if (r2.u8[0]==REG_RSP) {
      while (d2 && rsp_size) {
	ICU8(tmpi,0x5B); //POP RBX (Dont change flags)
	rsp_size-=8;
	d2-=8;
      }
    } else {
      while (rsp_size) {
	ICU8(tmpi,0x5B); //POP RBX (Dont change flags)
	rsp_size-=8;
      }
      ICPush(tmpi,t2,r2,d2,rip2);
    }
  } else {
    while (rsp_size) {
      ICU8(tmpi,0x5B); //POP RBX (Dont change flags)
      rsp_size-=8;
    }
  }

  rip+=tmpi->ic_cnt;
  lb=OptLabelFwd(tmpi->ic_data);
  short_jmp=ToBool(tmpi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(rip+2);
    if (lb->flags&CMF_POP_CMP) {
      if(tmpi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    if (I8_MIN<=i<=I8_MAX)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tmpi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(rip+2);
    if (lb->flags&CMF_POP_CMP) {
      if(tmpi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    ICU16(tmpi,i<<8+us.u8[2]);
  } else {
    tmpi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(rip+6);
    if (lb->flags&CMF_POP_CMP) {
      if(tmpi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    ICU16(tmpi,us.u16[0]);
    ICU32(tmpi,i);
  }
}

U0 ICFMul(CCmpCtrl *cc,CIntermediateCode *tmpi,U8 *buf,I64 rip)
{
  if (tmpi->arg1.type&MDF_IMM && tmpi->arg1.type&RTG_MASK==RT_F64 &&
	tmpi->arg1.disp(F64)==1.0) {
    CmpNoteFloatOp(cc,tmpi,FALSE,FALSE,CN_INST);
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
  } else if (tmpi->arg2.type&MDF_IMM &&
	tmpi->arg2.type&RTG_MASK==RT_F64&& tmpi->arg2.disp(F64)==1.0) {
    CmpNoteFloatOp(cc,tmpi,FALSE,FALSE,CN_INST);
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
  } else
    ICFOp(cc,tmpi,SLASH_OP_FMUL,buf,rip);
}

U0 ICFDiv(CCmpCtrl *cc,CIntermediateCode *tmpi,U8 *buf,I64 rip)
{
  if (tmpi->arg2.type&MDF_IMM && tmpi->arg2.type&RTG_MASK==RT_F64 &&
	tmpi->arg2.disp(F64)==1.0) {
    CmpNoteFloatOp(cc,tmpi,FALSE,FALSE,CN_INST);
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
  } else
    ICFOp(cc,tmpi,SLASH_OP_FDIV,buf,rip);
}

U0 ICFAdd(CCmpCtrl *cc,CIntermediateCode *tmpi,U8 *buf,I64 rip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (tmpi->arg1.type&MDF_IMM && !tmpi->arg1.disp) {
    if (dont_push_float) {
      ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_INST);
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    MDF_REG+RT_I64,REG_RAX,0,rip);
    } else if (dont_pop_float) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
      ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_INST);
    } else {
      CmpNoteFloatOp(cc,tmpi,TRUE,TRUE,CN_INST);
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
    }
  } else if (tmpi->arg2.type&MDF_IMM && !tmpi->arg2.disp) {
    if (dont_push_float) {
      ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_INST);
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    MDF_REG+RT_I64,REG_RAX,0,rip);
    } else if (dont_pop_float) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_INST);
    } else {
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      CmpNoteFloatOp(cc,tmpi,TRUE,TRUE,CN_INST);
    }
  } else
    ICFOp(cc,tmpi,SLASH_OP_FADD,buf,rip);
}

U0 ICFSub(CCmpCtrl *cc,CIntermediateCode *tmpi,U8 *buf,I64 rip)
{
  Bool dont_push_float,dont_pop_float;
  if (tmpi->arg2.type&MDF_IMM && !tmpi->arg2.disp) {
    CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
    if (dont_push_float) {
      ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_INST);
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    MDF_REG+RT_I64,REG_RAX,0,rip);
    } else if (dont_pop_float) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_INST);
    } else {
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      CmpNoteFloatOp(cc,tmpi,TRUE,TRUE,CN_INST);
    }
  } else
    ICFOp(cc,tmpi,SLASH_OP_FSUB,buf,rip);
}

U0 ICFPreIncDec(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 op,I64 rip)
{
  if (tmpi->ic_flags & ICF_BY_VAL) {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg1.type&MDG_MASK+RT_I64,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    ICCopyTemplate(cc,tmpi,op,FALSE,TRUE,TRUE,CN_INST);
    ICMov(tmpi,tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
	  MDF_REG+RT_I64,REG_RAX,0,rip);
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,MDF_DISP+RT_I64,REG_RCX,0,rip);
    ICCopyTemplate(cc,tmpi,op,FALSE,TRUE,TRUE,CN_INST);
    ICMov(tmpi,MDF_DISP+RT_I64,REG_RCX,0,MDF_REG+RT_I64,REG_RAX,0,rip);
  }
  if (tmpi->res.type.mode)
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_REG+RT_I64,REG_RAX,0,rip);
}

U0 ICFPostIncDec(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 op,I64 rip)
{
  if (tmpi->ic_flags & ICF_BY_VAL) {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg1.type&MDG_MASK+RT_I64,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    if (tmpi->res.type.mode)
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_REG+RT_I64,REG_RAX,0,rip);
    ICCopyTemplate(cc,tmpi,op,FALSE,TRUE,TRUE,CN_INST);
    ICMov(tmpi,tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
	  MDF_REG+RT_I64,REG_RAX,0,rip);
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,MDF_DISP+RT_I64,REG_RCX,0,rip);
    if (tmpi->res.type.mode)
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_REG+RT_I64,REG_RAX,0,rip);
    ICCopyTemplate(cc,tmpi,op,FALSE,TRUE,TRUE,CN_INST);
    ICMov(tmpi,MDF_DISP+RT_I64,REG_RCX,0,MDF_REG+RT_I64,REG_RAX,0,rip);
  }
  if (tmpi->res.type.mode)
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_REG+RT_I64,REG_RDX,0,rip);
}

U0 ICFTemplateFun(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 op,I64 rip)
{
  Bool dont_push_float,dont_pop_float;

  CmpSetFloatOpPushPop(cc,tmpi,&dont_push_float,&dont_pop_float);
  if (!dont_push_float)
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);

  ICCopyTemplate(cc,tmpi,op,FALSE,TRUE,TRUE,CN_INST);
  if (tmpi->res.type.mode && !(tmpi->ic_flags & ICF_RES_TO_F64) &&
	!(tmpi->ic_flags & ICF_RES_TO_INT))
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_REG+RT_I64,REG_RAX,0,rip);
}
