U0 ICFOpEqu(CCmpCtrl *cc,CIntermediateCode *tempi,I64 op,U8 *buf2,I64 ip)
{//for ADD,SUB,DIV,MUL
  CICArg *a1=&tempi->a1,
	*a2=&tempi->a2;
  Bool dont_push_float,dont_pop_float,p1_mem;
  I64 rsp_size=0,builtin2=0,
	t1,r1,d1,t2,r2,d2;

  if (cc->flags&CCF_AOT_COMPILE)
    buf2=cc->aotc->ip;

  CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
  if (dont_pop_float)
    throw('Compiler');

  if (tempi->ic_flags & ICF_BY_VAL) {
    p1_mem=FALSE;
    if (dont_push_float) {
      if (a1->type_pointed_to!=RT_F64) {
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	      a1->type&MDG_MASK+a1->type_pointed_to,a1->reg,a1->disp,ip);
	ICFCvt2(cc,tempi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,FALSE,ip);
	ICPush(tempi,MDF_REG+RT_I64,REG_RAX,0,ip);
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	rsp_size+=8;
      } else {
	if (a1->type_pointed_to>=RT_I64 && a1->type&MDG_DISP_SIB_RIP) {
	  t1=a1->type&MDG_MASK+a1->type_pointed_to;
	  r1=a1->reg;
	  d1=a1->disp;
	  p1_mem=TRUE;
	} else {
	  ICPush(tempi,
		a1->type&MDG_MASK+a1->type_pointed_to,a1->reg,a1->disp,ip);
	  t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	  rsp_size+=8;
	}
      }
    } else {
      if (a1->type_pointed_to!=RT_F64 || a1->type&MDF_STK) {
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip);
	ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
	      a1->type&MDG_MASK+a1->type_pointed_to,a1->reg,a1->disp,ip);
	if (a1->type_pointed_to!=RT_F64)
	  ICFCvt2(cc,tempi,REG_RDX,MDF_REG+RT_I64,REG_RDX,0,FALSE,ip);
	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=8;
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      } else {
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
	if (a1->type_pointed_to>=RT_I64 && a1->type&MDG_DISP_SIB_RIP) {
	  t1=a1->type&MDG_MASK+a1->type_pointed_to;
	  r1=a1->reg;
	  d1=a1->disp;
	  p1_mem=TRUE;
	} else {
	  ICPush(tempi,
		a1->type&MDG_MASK+a1->type_pointed_to,a1->reg,a1->disp,ip);
	  t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	  rsp_size+=8;
	  if (r2==REG_RSP+REG_RSP<<8)
	    d2+=8;
	}
      }
    }
    if (!rsp_size && !(p1_mem && a1->type_pointed_to==RT_F64)) {
      rsp_size=8;
      ICAddRSP(tempi,-8);
    }
    if (!dont_push_float) {
      if (builtin2)
	ICU16(tempi,builtin2);
      else
	ICSlashOp(tempi,t2,r2,d2,SLASH_OP_FLD,ip);
    }
    switch (op.u8[0]) {
      case 4: op=SLASH_OP_FSUBR; break;
      case 6: op=SLASH_OP_FDIVR; break;
    }
    ICSlashOp(tempi,t1,r1,d1,op,ip);
    CmpNoteFloatOp(cc,tempi,TRUE,FALSE,CN_MAIN);
    if (p1_mem && a1->type_pointed_to==RT_F64) {
      ICSlashOp(tempi,t1,r1,d1,SLASH_OP_FSTP,ip);
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
      if (a1->type_pointed_to!=RT_F64) {
	ICPop(tempi,MDF_REG+RT_I64,REG_RAX,0,ip);
	ICFCvt2(cc,tempi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,TRUE,ip);
	ICMov(tempi,a1->type&MDG_MASK+a1->type_pointed_to,a1->reg,a1->disp,
	      MDF_REG+RT_I64,REG_RAX,0,ip);
      } else
	ICPop(tempi,a1->type&MDG_MASK+a1->type_pointed_to,a1->reg,a1->disp,ip);
    }
    if (tempi->r.type.mode)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    a1->type&MDG_MASK+a1->type_pointed_to,a1->reg,a1->disp,ip);
  } else {
    if (a1->type_pointed_to>=RT_I64)
      p1_mem=TRUE;
    else
      p1_mem=FALSE;
    if (dont_push_float) {
      ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,a1->type,a1->reg,a1->disp,ip);
      if (a1->type_pointed_to!=RT_F64) {
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	      MDF_DISP+a1->type_pointed_to,REG_RCX,0,ip);
	ICFCvt2(cc,tempi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,FALSE,ip);
	ICPush(tempi,MDF_REG+RT_I64,REG_RAX,0,ip);
      } else
	ICPush(tempi,MDF_DISP+a1->type_pointed_to,REG_RCX,0,ip);
      t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
      rsp_size+=8;
    } else {
      if (a1->type_pointed_to!=RT_F64 || a1->type&MDF_STK) {
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip);
	ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,a1->type,a1->reg,a1->disp,ip);
	ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
	      MDF_DISP+a1->type_pointed_to,REG_RCX,0,ip);
	if (a1->type_pointed_to!=RT_F64)
	  ICFCvt2(cc,tempi,REG_RDX,MDF_REG+RT_I64,REG_RDX,0,FALSE,ip);
	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=8;
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      } else {
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
	ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,a1->type,a1->reg,a1->disp,ip);
	ICPush(tempi,MDF_DISP+a1->type_pointed_to,REG_RCX,0,ip);
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	rsp_size+=8;
	if (r2==REG_RSP+REG_RSP<<8)
	  d2+=8;
      }
    }
    if (!rsp_size && !(p1_mem && a1->type_pointed_to==RT_F64)) {
      rsp_size=8;
      ICAddRSP(tempi,-8);
    }
    if (!dont_push_float) {
      if (builtin2)
	ICU16(tempi,builtin2);
      else
	ICSlashOp(tempi,t2,r2,d2,SLASH_OP_FLD,ip);
    }

    switch (op.u8[0]) {
      case 4: op=SLASH_OP_FSUBR; break;
      case 6: op=SLASH_OP_FDIVR; break;
    }
    ICSlashOp(tempi,t1,r1,d1,op,ip);
    CmpNoteFloatOp(cc,tempi,TRUE,FALSE,CN_MAIN);
    if (p1_mem && a1->type_pointed_to==RT_F64) {
      ICSlashOp(tempi,MDF_DISP+a1->type_pointed_to,REG_RCX,0,SLASH_OP_FSTP,ip);
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
      ICPop(tempi,MDF_REG+RT_I64,REG_RAX,0,ip);
      if (a1->type_pointed_to!=RT_F64)
	ICFCvt2(cc,tempi,REG_RAX,MDF_REG+RT_I64,REG_RAX,0,TRUE,ip);
      ICMov(tempi,MDF_DISP+a1->type_pointed_to,REG_RCX,0,
	    MDF_REG+RT_I64,REG_RAX,0,ip);
    }
    if (tempi->r.type.mode)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    MDF_REG+RT_I64,REG_RAX,0,ip);
  }
}
 
U0 ICFCmpAndBranch(CCmpCtrl *cc,CIntermediateCode *tempi,I64 ip,
   I64 us,I64 not_us,U8 *buf,I64 ip2)
{
  CICArg *a1,*a2;
  Bool dont_push_float,dont_pop_float,alt,short_jmp;
  I64 i,rsp_size=0,builtin1=0,builtin2=0,t1,r1,d1,t2,r2,d2;
  CCodeMisc *lb;
  U8 *buf2;

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
  else
    buf2=buf;

  CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
  if (dont_push_float) {
    if (tempi->ic_flags&ICF_POP_CMP && alt) {
      t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      rsp_size+=8;
    } else {
      if (a2->type.raw_type>=RT_I64 && a2->type&MDG_DISP_SIB_RIP) {
	t2=a2->type;
	r2=a2->reg;
	d2=a2->disp;
      } else {
	if (a2->type&MDF_IMM) {
	  if (!(builtin2=ICBuiltInFloatConst(a2->disp(F64))) ||
		tempi->ic_flags&ICF_PUSH_CMP) {
	    t2=MDF_RIP_DISP32+RT_I64;
	    r2=REG_RIP;
	    d2=COCFloatConstFind(cc,a2->disp(F64))+buf2;
	  }
	} else {
	  ICPush(tempi,a2->type,a2->reg,a2->disp,ip2);
	  t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	  rsp_size+=8;
	}
      }
    }
  } else {
    if (alt) {
      if (!(a2->type&MDF_STK)) {
	if (tempi->ic_flags&ICF_POP_CMP) {
	  t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	  rsp_size+=8;
	} else {
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
	      ICPush(tempi,a1->type,a1->reg,a1->disp,ip2);
	      t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	      rsp_size+=8;
	    }
	  }
	}
	if (a2->type.raw_type>=RT_I64 && a2->type&MDG_DISP_SIB_RIP) {
	  t2=a2->type;
	  r2=a2->reg;
	  d2=a2->disp;
	} else {
	  if (a2->type&MDF_IMM) {
	    if (!(builtin2=ICBuiltInFloatConst(a2->disp(F64))) ||
		  tempi->ic_flags&ICF_PUSH_CMP) {
	      t2=MDF_RIP_DISP32+RT_I64;
	      r2=REG_RIP;
	      d2=COCFloatConstFind(cc,a2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,a2->type,a2->reg,a2->disp,ip2);
	    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	    rsp_size+=8;
	    if (r1==REG_RSP+REG_RSP<<8)
	      d1+=8;
	  }
	}
      } else {
	if (tempi->ic_flags&ICF_POP_CMP)
	  ICPopRegs(tempi,1<<REG_RDX);
	else
	  ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip2);
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip2);
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
	    if (!(builtin2=ICBuiltInFloatConst(a2->disp(F64))) ||
		  tempi->ic_flags&ICF_PUSH_CMP) {
	      t2=MDF_RIP_DISP32+RT_I64;
	      r2=REG_RIP;
	      d2=COCFloatConstFind(cc,a2->disp(F64))+buf2;
	    }
	  } else {
	    ICPush(tempi,a2->type,a2->reg,a2->disp,ip2);
	    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
	    rsp_size+=8;
	  }
	}
	if (tempi->ic_flags&ICF_POP_CMP) {
	  t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	  rsp_size+=8;
	  if (r2==REG_RSP+REG_RSP<<8)
	    d1+=8;
	} else {
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
	      ICPush(tempi,a1->type,a1->reg,a1->disp,ip2);
	      t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=0;
	      rsp_size+=8;
	      if (r2==REG_RSP+REG_RSP<<8)
		d2+=8;
	    }
	  }
	}
      } else {
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip2);
	if (tempi->ic_flags&ICF_POP_CMP)
	  ICPopRegs(tempi,1<<REG_RDX);
	else
	  ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip2);
	ICU16(tempi,0x5052);	//PUSH EDX PUSH EAX
	rsp_size=16;
	t1=MDF_SIB+RT_I64; r1=REG_RSP+REG_RSP<<8; d1=8;
	t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
      }
    }
  }
  if (!dont_push_float) {
    if (builtin1)
      ICU16(tempi,builtin1);
    else
      ICSlashOp(tempi,t1,r1,d1,SLASH_OP_FLD,ip2);
  }
  if (!alt)
    us=not_us;
  if (builtin2)
    ICU16(tempi,builtin2);
  else
    ICSlashOp(tempi,t2,r2,d2,SLASH_OP_FLD,ip2);
  if (tempi->ic_flags&ICF_PUSH_CMP) {
    t2=MDF_SIB+RT_I64; r2=REG_RSP+REG_RSP<<8; d2=0;
    if (!rsp_size) {
      rsp_size=8;
      ICAddRSP(tempi,-8);
    } else if (rsp_size==16)
      d2=8;

    if (alt) {
      ICU16(tempi,0xF1DF);	  //FCOMIP
      ICSlashOp(tempi,t2,r2,d2,SLASH_OP_FSTP,ip2);
    } else {
      ICU16(tempi,0xF1DB);	  //FCOMI
      ICSlashOp(tempi,t2,r2,d2,SLASH_OP_FSTP,ip2);
      ICU32(tempi,0xF7D9C0DD); //FFREE,FINCSTP
    }
  } else {
    ICU16(tempi,0xF1DF);	  //FCOMIP
    ICU32(tempi,0xF7D9C0DD); //FFREE,FINCSTP
  }
  CmpNoteFloatOp(cc,tempi,TRUE,FALSE,CN_MAIN);

  if (tempi->ic_flags&ICF_PUSH_CMP) {
    if (r2.u8[0]==REG_RSP) {
      while (d2 && rsp_size) {
	ICU8(tempi,0x5B); //POP RBX (Dont change flags)
	rsp_size-=8;
	d2-=8;
      }
    } else {
      while (rsp_size) {
	ICU8(tempi,0x5B); //POP RBX (Dont change flags)
	rsp_size-=8;
      }
      ICPush(tempi,t2,r2,d2,ip2);
    }
  } else {
    while (rsp_size) {
      ICU8(tempi,0x5B); //POP RBX (Dont change flags)
      rsp_size-=8;
    }
  }

  ip+=tempi->ic_cnt;
  lb=OptLabelFwd(tempi->ic_data);
  short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP) {
      if(tempi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tempi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP) {
      if(tempi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    ICU16(tempi,i<<8+us.u8[2]);
  } else {
    tempi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP) {
      if(tempi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    ICU16(tempi,us.u16[0]);
    ICU32(tempi,i);
  }
}

U0 ICFMul(CCmpCtrl *cc,CIntermediateCode *tempi,U8 *buf,I64 ip)
{
  if (tempi->a1.type&MDF_IMM && tempi->a1.type_pointed_to==RT_F64 &&
	tempi->a1.disp(F64)==1.0) {
    CmpNoteFloatOp(cc,tempi,FALSE,FALSE,CN_MAIN);
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
  } else if (tempi->a2.type&MDF_IMM &&
	tempi->a2.type_pointed_to==RT_F64&& tempi->a2.disp(F64)==1.0) {
    CmpNoteFloatOp(cc,tempi,FALSE,FALSE,CN_MAIN);
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
  } else
    ICFOp(cc,tempi,SLASH_OP_FMUL,buf,ip);
}

U0 ICFDiv(CCmpCtrl *cc,CIntermediateCode *tempi,U8 *buf,I64 ip)
{
  if (tempi->a2.type&MDF_IMM && tempi->a2.type_pointed_to==RT_F64 &&
	tempi->a2.disp(F64)==1.0) {
    CmpNoteFloatOp(cc,tempi,FALSE,FALSE,CN_MAIN);
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
  } else
    ICFOp(cc,tempi,SLASH_OP_FDIV,buf,ip);
}

U0 ICFAdd(CCmpCtrl *cc,CIntermediateCode *tempi,U8 *buf,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
  if (tempi->a1.type&MDF_IMM && !tempi->a1.disp) {
    if (dont_push_float) {
      ICCopyTemplate(cc,tempi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    MDF_REG+RT_I64,REG_RAX,0,ip);
    } else if (dont_pop_float) {
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
      ICCopyTemplate(cc,tempi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_MAIN);
    } else {
      CmpNoteFloatOp(cc,tempi,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    }
  } else if (tempi->a2.type&MDF_IMM && !tempi->a2.disp) {
    if (dont_push_float) {
      ICCopyTemplate(cc,tempi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    MDF_REG+RT_I64,REG_RAX,0,ip);
    } else if (dont_pop_float) {
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      ICCopyTemplate(cc,tempi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_MAIN);
    } else {
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      CmpNoteFloatOp(cc,tempi,TRUE,TRUE,CN_MAIN);
    }
  } else
    ICFOp(cc,tempi,SLASH_OP_FADD,buf,ip);
}

U0 ICFSub(CCmpCtrl *cc,CIntermediateCode *tempi,U8 *buf,I64 ip)
{
  Bool dont_push_float,dont_pop_float;
  if (tempi->a2.type&MDF_IMM && !tempi->a2.disp) {
    CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
    if (dont_push_float) {
      ICCopyTemplate(cc,tempi,CMP_TEMPLATE_FSTP,FALSE,TRUE,TRUE,CN_MAIN);
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    MDF_REG+RT_I64,REG_RAX,0,ip);
    } else if (dont_pop_float) {
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      ICCopyTemplate(cc,tempi,CMP_TEMPLATE_FLD,FALSE,TRUE,TRUE,CN_MAIN);
    } else {
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      CmpNoteFloatOp(cc,tempi,TRUE,TRUE,CN_MAIN);
    }
  } else
    ICFOp(cc,tempi,SLASH_OP_FSUB,buf,ip);
}

U0 ICFPreIncDec(CCmpCtrl *cc,CIntermediateCode *tempi,I64 op,I64 ip)
{
  if (tempi->ic_flags & ICF_BY_VAL) {
    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	  tempi->a1.type&MDG_MASK+RT_I64,tempi->a1.reg,tempi->a1.disp,ip);
    ICCopyTemplate(cc,tempi,op,FALSE,TRUE,TRUE,CN_MAIN);
    ICMov(tempi,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
	  MDF_REG+RT_I64,REG_RAX,0,ip);
  } else {
    ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,MDF_DISP+RT_I64,REG_RCX,0,ip);
    ICCopyTemplate(cc,tempi,op,FALSE,TRUE,TRUE,CN_MAIN);
    ICMov(tempi,MDF_DISP+RT_I64,REG_RCX,0,MDF_REG+RT_I64,REG_RAX,0,ip);
  }
  if (tempi->r.type.mode)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICFPostIncDec(CCmpCtrl *cc,CIntermediateCode *tempi,I64 op,I64 ip)
{
  if (tempi->ic_flags & ICF_BY_VAL) {
    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	  tempi->a1.type&MDG_MASK+RT_I64,tempi->a1.reg,tempi->a1.disp,ip);
    if (tempi->r.type.mode)
      ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,MDF_REG+RT_I64,REG_RAX,0,ip);
    ICCopyTemplate(cc,tempi,op,FALSE,TRUE,TRUE,CN_MAIN);
    ICMov(tempi,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
	  MDF_REG+RT_I64,REG_RAX,0,ip);
  } else {
    ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,MDF_DISP+RT_I64,REG_RCX,0,ip);
    if (tempi->r.type.mode)
      ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,MDF_REG+RT_I64,REG_RAX,0,ip);
    ICCopyTemplate(cc,tempi,op,FALSE,TRUE,TRUE,CN_MAIN);
    ICMov(tempi,MDF_DISP+RT_I64,REG_RCX,0,MDF_REG+RT_I64,REG_RAX,0,ip);
  }
  if (tempi->r.type.mode)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  MDF_REG+RT_I64,REG_RDX,0,ip);
}

U0 ICFTemplateFun(CCmpCtrl *cc,CIntermediateCode *tempi,I64 op,I64 ip)
{
  Bool dont_push_float,dont_pop_float;

  CmpSetFloatOpPushPop(cc,tempi,&dont_push_float,&dont_pop_float);
  if (!dont_push_float)
    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);

  ICCopyTemplate(cc,tempi,op,FALSE,TRUE,TRUE,CN_MAIN);
  if (tempi->r.type.mode && !(tempi->ic_flags & ICF_R_TO_F64) &&
	!(tempi->ic_flags & ICF_R_TO_INT))
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  MDF_REG+RT_I64,REG_RAX,0,ip);
}
