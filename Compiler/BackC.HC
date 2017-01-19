U0 ICAndBranch(CIntermediateCode *tmpi,I64 rip,I64 is,U8 *buf,I64 rip2)
{
  U64 i;
  I64 it,t1,r1,d1,r2;
  Bool short_jmp,swap,override;
  CCodeMisc *lb;
  CICArg *arg1,*arg2;

  if (tmpi->arg1.type&MDF_IMM) {
    swap=TRUE;
    arg1=&tmpi->arg2;
    arg2=&tmpi->arg1;
  } else {
    swap=FALSE;
    arg1=&tmpi->arg1;
    arg2=&tmpi->arg2;
  }

  if (arg2->type&MDF_IMM && arg2->disp>U32_MAX)
    override=TRUE;
  else
    override=FALSE;

  if (arg1->type.raw_type<arg2->type.raw_type)
    it=arg1->type.raw_type;
  else
    it=arg2->type.raw_type;

  i=arg2->disp;
  if (arg2->type&MDF_IMM && i<=U32_MAX) {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg1->type,arg1->reg,arg1->disp,rip2);
    if (i<=U8_MAX)
      ICU16(tmpi,i<<8+0xA8);
    else if (i<=U16_MAX)
      ICU32(tmpi,i<<16+0xA900+OC_OP_SIZE_PREFIX);
    else {
      ICU8(tmpi,0xA9);
      ICU32(tmpi,i);
    }
  } else {
    t1=MDF_REG+it;
    d1=0;
    if (swap && !override) {
      if (arg1->type&MDF_REG) {
	r1=arg1->reg;
	swap=TRUE;
      } else {
	r1=REG_RCX;
	swap=FALSE;
      }
      if (arg2->type&MDF_REG) {
	r2=arg2->reg;
	swap=FALSE;
      } else
	r2=REG_RDX;
      if (swap) {
	if (!(arg1->type&MDF_REG) || r1!=arg1->reg)
	  ICMov(tmpi,MDF_REG+RT_I64,r1,0,arg1->type,arg1->reg,arg1->disp,rip2);
	if (arg2->type&MDG_REG_DISP_SIB_RIP) {
	  t1=arg2->type&MDG_MASK+it;
	  r2=arg2->reg;
	  d1=arg2->disp;
	} else
	  ICMov(tmpi,MDF_REG+RT_I64,r2,0,arg2->type,arg2->reg,arg2->disp,rip2);
	i=ICModr1(r1,t1,r2,d1);
      } else {
	if (arg1->type&MDG_REG_DISP_SIB_RIP) {
	  t1=arg1->type&MDG_MASK+it;
	  r1=arg1->reg;
	  d1=arg1->disp;
	} else
	  ICMov(tmpi,MDF_REG+RT_I64,r1,0,arg1->type,arg1->reg,arg1->disp,rip2);
	if (!(arg2->type&MDF_REG) || r2!=arg2->reg)
	  ICMov(tmpi,MDF_REG+RT_I64,r2,0,arg2->type,arg2->reg,arg2->disp,rip2);
	i=ICModr1(r2,t1,r1,d1);
      }
    } else {
      if (arg2->type&MDF_REG) {
	r2=arg2->reg;
	swap=FALSE;
      } else {
	r2=REG_RDX;
	swap=TRUE;
      }
      if (arg1->type&MDF_REG) {
	r1=arg1->reg;
	swap=TRUE;
      } else
	r1=REG_RCX;
      if (override)
	swap=FALSE;
      if (swap) {
	if (arg2->type&MDG_REG_DISP_SIB_RIP) {
	  t1=arg2->type&MDG_MASK+it;
	  r2=arg2->reg;
	  d1=arg2->disp;
	} else
	  ICMov(tmpi,MDF_REG+RT_I64,r2,0,arg2->type,arg2->reg,arg2->disp,rip2);
	if (!(arg1->type&MDF_REG) || r1!=arg1->reg)
	  ICMov(tmpi,MDF_REG+RT_I64,r1,0,arg1->type,arg1->reg,arg1->disp,rip2);
	i=ICModr1(r1,t1,r2,d1);
      } else {
	if (!(arg2->type&MDF_REG) || r2!=arg2->reg)
	  ICMov(tmpi,MDF_REG+RT_I64,r2,0,arg2->type,arg2->reg,arg2->disp,rip2);
	if (arg1->type&MDG_REG_DISP_SIB_RIP) {
	  t1=arg1->type&MDG_MASK+it;
	  r1=arg1->reg;
	  d1=arg1->disp;
	} else
	  ICMov(tmpi,MDF_REG+RT_I64,r1,0,arg1->type,arg1->reg,arg1->disp,rip2);
	i=ICModr1(r2,t1,r1,d1);
      }
    }
    switch (it) {
      case RT_I8:
      case RT_U8:
	ICRex(tmpi,i.u8[1]);
	ICU16(tmpi,i.u8[2]<<8+0x84);
	break;
      case RT_U16:
      case RT_I16:
	ICOpSizeRex(tmpi,i.u8[1]);
	ICU16(tmpi,i.u8[2]<<8+0x85);
	break;
      default:
	ICRex(tmpi,i.u8[1]);
	ICU16(tmpi,i.u8[2]<<8+0x85);
    }
    ICModr2(tmpi,i,,d1,rip2);
  }

  rip+=tmpi->ic_cnt;
  lb=OptLabelFwd(tmpi->ic_data);
  short_jmp=ToBool(tmpi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(rip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    if (I8_MIN<=i<=I8_MAX)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tmpi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(rip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tmpi,i<<8+is.u8[2]);
  } else {
    tmpi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(rip+6);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tmpi,is.u16[0]);
    ICU32(tmpi,i);
  }
}

U0 ICAssign(CIntermediateCode *tmpi,I64 rip)
{
  CIntermediateCode *tmpi1;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    ICMov(tmpi,tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	  tmpi->arg1.reg,tmpi->arg1.disp,
	  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->res.reg,tmpi->res.disp,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,rip);
  } else {
    if (tmpi->arg1.type&MDF_REG) {
      if (!(tmpi1=OptLag1(tmpi)) || tmpi1->ic_code!=IC_ADD_CONST ||
	    tmpi1->res.type!=MDF_REG+RT_I64 || tmpi1->res.reg!=tmpi->arg1.reg ||
	    tmpi1->arg1.type!=MDF_REG+RT_I64 ||
	    tmpi1->arg1.reg!=tmpi->arg1.reg ||
	    (tmpi->arg2.type&MDF_REG || tmpi->arg2.type&MDF_DISP) &&
	    tmpi->arg2.reg==tmpi->arg1.reg ||
	    tmpi->res.type.mode || tmpi1->ic_flags&~ICG_NO_CVT_MASK) {
	ICMov(tmpi,
	      MDF_DISP+tmpi->arg1_type_pointed_to,
	      tmpi->arg1.reg,tmpi->arg1.disp,
	      tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
	if (tmpi->res.type.mode)
	  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
      } else {
	tmpi->ic_flags=(tmpi->ic_flags|tmpi1->ic_flags)&
	      ~ICF_CODE_FINAL|ICF_DONT_RESTORE;
	tmpi->arg1.disp=tmpi1->ic_data;
	OptSetNOP1(tmpi1); //This better not be last pass!
	ICMov(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,
	      tmpi->arg1.reg,tmpi->arg1.disp,
	      tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
      }
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      ICMov(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
      if (tmpi->res.type.mode)
	ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	      tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
    }
  }
}

U0 ICBrBitOps(CIntermediateCode *tmpi,I64 rip,
	I64 op,I64 op_imm,I64 is,U8 *buf,I64 rip2)
{
  I64 i,t,r1,r2,d1,d2,t1,t2;
  CICArg *arg1=&tmpi->arg1,*arg2=&tmpi->arg2;
  Bool short_jmp;
  CCodeMisc *lb;

  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (tmpi->ic_flags&ICF_SWAP && !(arg2->type&MDF_REG) &&
	  (!(arg2->type&MDF_IMM) ||arg2->disp>63)||
	  !(tmpi->ic_flags&ICF_SWAP) && arg2->type&MDF_IMM &&
	  arg2->disp<64 || arg2->type&MDF_STK) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,arg2->type,arg2->reg,arg2->disp,rip2);
      t2=MDF_REG+RT_I64;
      r2=REG_RCX;
      d2=0;
    } else {
      t2=arg2->type;
      if (t2&MDF_IMM && arg2->disp<64)
	r2=0;
      else
	r2=arg2->reg;
      d2=arg2->disp;
    }
    if (!(tmpi->ic_flags&ICF_SWAP) && !(arg1->type&MDF_REG) &&
	  (!(arg1->type&MDF_IMM) || arg1->disp>63) ||
	  tmpi->ic_flags&ICF_SWAP && arg1->type&MDF_IMM &&
	  arg1->disp<64 || arg1->type&MDF_STK) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,arg1->type,arg1->reg,arg1->disp,rip2);
      t1=MDF_REG+RT_I64;
      r1=REG_RDX;
      d1=0;
    } else {
      t1=arg1->type;
      if (t1&MDF_IMM && arg1->disp<64)
	r1=0;
      else
	r1=arg1->reg;
      d1=arg1->disp;
    }
  } else {
    t1=MDF_DISP+RT_I64;
    t2=MDF_DISP+RT_I64;
    d1=0;
    d2=0;
    if (arg2->type&MDF_REG)
      r2=arg2->reg;
    else if (!(tmpi->ic_flags&ICF_SWAP) || !(arg2->type&MDF_IMM) ||
	  arg2->disp>63) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,arg2->type,arg2->reg,arg2->disp,rip2);
      r2=REG_RCX;
    } else
      r2=0;
    if (arg1->type&MDF_REG)
      r1=arg1->reg;
    else if (tmpi->ic_flags&ICF_SWAP ||
	  !(arg1->type&MDF_IMM) || arg1->disp>63) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,arg1->type,arg1->reg,arg1->disp,rip2);
      r1=REG_RDX;
    } else
      r1=0;
  }
  if (tmpi->ic_flags&ICF_LOCK && op!=0xA30F)
    ICU8(tmpi,OC_LOCK_PREFIX);
  if (tmpi->ic_flags&ICF_SWAP) {
    if (arg2->type&MDF_IMM && arg2->disp<32) {
      if (op==0xA30F && arg2->disp<8) {
	t=t1&MDG_MASK+RT_U8;
	op_imm=0xF6; //TEST
      } else
	t=t1&MDG_MASK+RT_U32;
    } else
      t=t1;
    i=ICModr1(r2,t,r1,d1);
    SwapI64(&arg1,&arg2);
  } else {
    if (arg1->type&MDF_IMM && arg1->disp<32) {
      if (op==0xA30F && arg1->disp<8) {
	t=t2&MDG_MASK+RT_U8;
	op_imm=0xF6; //TEST
      } else
	t=t2&MDG_MASK+RT_U32;
    } else
      t=t2;
    i=ICModr1(r1,t,r2,d2);
  }
  ICRex(tmpi,i.u8[1]);
  if (op_imm==0xF6) {//TEST
    ICU16(tmpi,i.u8[2]<<8+op_imm);
    ICModr2(tmpi,i,,arg2->disp,rip2+1);
    ICU8(tmpi,1<<arg1->disp);
    if (is==0x72820F)
      is=0x75850F;
    else
      is=0x74840F;
  } else if (arg1->type&MDF_IMM && arg1->disp<64) {
    ICU24(tmpi,i.u8[2]<<16+op_imm);
    ICModr2(tmpi,i,,arg2->disp,rip2+1);
    ICU8(tmpi,arg1->disp);
  } else {
    ICU24(tmpi,i.u8[2]<<16+op);
    ICModr2(tmpi,i,,arg2->disp,rip2);
  }

  rip+=tmpi->ic_cnt;
  lb=OptLabelFwd(tmpi->ic_data);
  short_jmp=ToBool(tmpi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(rip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    if (I8_MIN<=i<=I8_MAX)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tmpi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(rip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tmpi,i<<8+is.u8[2]);
  } else {
    tmpi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(rip+6);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tmpi,is.u16[0]);
    ICU32(tmpi,i);
  }
}

U0 ICQueInit(CIntermediateCode *tmpi,I64 rip2)
{
  I64 r1;
  if (tmpi->arg1.type==MDF_REG+RT_I64)
    r1=tmpi->arg1.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
    r1=REG_RAX;
  }
  ICMov(tmpi,MDF_DISP+RT_I64,r1,0,MDF_REG+RT_I64,r1,0,rip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r1,sizeof(U8 *),MDF_REG+RT_I64,r1,0,rip2);
}

U0 ICQueIns(CIntermediateCode *tmpi,I64 rip2)
{
  I64 r1,r2;
  if (tmpi->arg2.type==MDF_REG+RT_I64 && tmpi->arg2.reg!=REG_RDX)
    r2=tmpi->arg2.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
    r2=REG_RAX;
  }
  if (tmpi->arg1.type==MDF_REG+RT_I64)
    r1=tmpi->arg1.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
    r1=REG_RDX;
  }
  ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,MDF_DISP+RT_I64,r1,0,rip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r1,0,MDF_REG+RT_I64,r2,0,rip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r2,0,MDF_REG+RT_I64,REG_RBX,0,rip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r2,sizeof(U8 *),MDF_REG+RT_I64,r1,0,rip2);
  ICMov(tmpi,MDF_DISP+RT_I64,REG_RBX,sizeof(U8 *),MDF_REG+RT_I64,r2,0,rip2);
}

U0 ICQueInsRev(CIntermediateCode *tmpi,I64 rip2)
{
  I64 r1,r2;
  if (tmpi->arg2.type==MDF_REG+RT_I64 && tmpi->arg2.reg!=REG_RDX)
    r2=tmpi->arg2.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
    r2=REG_RAX;
  }
  if (tmpi->arg1.type==MDF_REG+RT_I64)
    r1=tmpi->arg1.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
    r1=REG_RDX;
  }
  ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,MDF_DISP+RT_I64,r1,sizeof(U8 *),rip2);
  ICMov(tmpi,MDF_DISP+RT_I64,REG_RBX,0,MDF_REG+RT_I64,r2,0,rip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r2,0,MDF_REG+RT_I64,r1,0,rip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r2,sizeof(U8 *),MDF_REG+RT_I64,REG_RBX,0,rip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r1,sizeof(U8 *),MDF_REG+RT_I64,r2,0,rip2);
}

U0 ICQueRem(CIntermediateCode *tmpi,I64 rip2)
{
  I64 r1;
  if (tmpi->arg1.type==MDF_REG+RT_I64)
    r1=tmpi->arg1.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
    r1=REG_RAX;
  }
  ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,MDF_DISP+RT_I64,r1,0,rip2);
  ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_I64,r1,sizeof(U8 *),rip2);
  ICU24(tmpi,0x1A8948);
  ICU32(tmpi,sizeof(U8 *)<<24+0x538948);
}

U0 ICMinMax(CIntermediateCode *tmpi,I64 op,I64 rip2)
{
  I64 r1,i1=0x48;
  if (tmpi->arg2.type==MDF_REG+RT_I64 && tmpi->arg2.reg!=REG_RAX) {
    r1=tmpi->arg2.reg;
    if (r1>7) {
      i1++;
      r1&=7;
    }
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
  } else {
    if (tmpi->arg1.reg==REG_RAX && tmpi->arg1.type&MDG_REG_DISP_SIB) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
      r1=REG_RDX;
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
      if (tmpi->arg1.type==MDF_REG+RT_I64) {
	r1=tmpi->arg1.reg;
	if (r1>7) {
	  i1++;
	  r1&=7;
	}
      } else {
	r1=REG_RDX;
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	      tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
      }
    }
  }
  ICU24(tmpi,0xC03B00+r1<<16+i1);
  ICU32(tmpi,0xC0000F00+op<<16+r1<<24+i1);
}

U0 ICSqr(CIntermediateCode *tmpi,I64 op,I64 rip2)
{
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
  ICSlashOp(tmpi,MDF_REG+RT_I64,REG_RAX,0,op,rip2);
}

U0 ICModU64(CIntermediateCode *tmpi,I64 rip2)
{
  CICType t1;
  I64 r1,d1;
  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
  if (tmpi->arg1.reg!=REG_RAX && tmpi->arg1.reg!=REG_RDX &&
	tmpi->arg1.type&MDG_REG_DISP_SIB &&
	tmpi->arg1.type.raw_type>=RT_I64) {
    t1=tmpi->arg1.type;
    r1=tmpi->arg1.reg;
    d1=tmpi->arg1.disp;
  } else {
    t1=MDF_REG+RT_I64;
    r1=REG_RBX;
    d1=0;
    ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
  }
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,MDF_DISP+RT_I64,REG_RCX,0,rip2);
  ICZero(tmpi,REG_RDX);
  ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_DIV,rip2);
  ICMov(tmpi,MDF_DISP+RT_I64,REG_RCX,0,MDF_REG+RT_I64,REG_RAX,0,rip2);
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	MDF_REG+RT_I64,REG_RDX,0,rip2);
}

U0 ICSwap(CIntermediateCode *tmpi,I64 rip2)
{
  I64 r1,r2;
  if (tmpi->arg1.type&MDF_REG)
    r1=tmpi->arg1.reg;
  else
    r1=REG_RAX;
  if (tmpi->arg2.type&MDF_REG)
    r2=tmpi->arg2.reg;
  else
    r2=REG_RAX;
  if (r1==r2) {
    if (r1==REG_RAX)
      r1=REG_RBX;
    else
      r2=REG_RAX;
  }
  ICMov(tmpi,MDF_REG+RT_I64,r2,0,
	tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
  ICMov(tmpi,MDF_REG+RT_I64,r1,0,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
  switch (tmpi->ic_code) {
    case IC_SWAP_U8:
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_DISP+RT_U8,r1,0,rip2);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_U8,r2,0,rip2);
      ICMov(tmpi,MDF_DISP+RT_U8,r2,0,MDF_REG+RT_I64,REG_RCX,0,rip2);
      ICMov(tmpi,MDF_DISP+RT_U8,r1,0,MDF_REG+RT_I64,REG_RDX,0,rip2);
      break;
    case IC_SWAP_U16:
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_DISP+RT_U16,r1,0,rip2);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_U16,r2,0,rip2);
      ICMov(tmpi,MDF_DISP+RT_U16,r2,0,MDF_REG+RT_I64,REG_RCX,0,rip2);
      ICMov(tmpi,MDF_DISP+RT_U16,r1,0,MDF_REG+RT_I64,REG_RDX,0,rip2);
      break;
    case IC_SWAP_U32:
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_DISP+RT_U32,r1,0,rip2);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_U32,r2,0,rip2);
      ICMov(tmpi,MDF_DISP+RT_U32,r2,0,MDF_REG+RT_I64,REG_RCX,0,rip2);
      ICMov(tmpi,MDF_DISP+RT_U32,r1,0,MDF_REG+RT_I64,REG_RDX,0,rip2);
      break;
    case IC_SWAP_I64:
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_DISP+RT_U64,r1,0,rip2);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_U64,r2,0,rip2);
      ICMov(tmpi,MDF_DISP+RT_U64,r2,0,MDF_REG+RT_I64,REG_RCX,0,rip2);
      ICMov(tmpi,MDF_DISP+RT_U64,r1,0,MDF_REG+RT_I64,REG_RDX,0,rip2);
      break;
  }
}

U0 ICAndEqu(CIntermediateCode *tmpi,I64 rip2)
{
  I64 i,bit;
  if (tmpi->arg2.type&MDF_IMM && !(tmpi->arg1.type&MDF_STK) &&
	tmpi->ic_flags&ICF_RES_NOT_USED) {
    i=~tmpi->arg2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i)) {
      tmpi->arg2.disp=bit;
      tmpi->arg2.reg=0;
      tmpi->arg1.type=tmpi->arg1.type&MDG_MASK+RT_I64;
      ICBitOps(tmpi,&tmpi->arg2,&tmpi->arg1,tmpi,0xB30F,0x30BA0F,rip2);
      return;
    }
  }
  ICAddSubEctEqu(tmpi,tmpi->arg1_type_pointed_to,
	tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
	tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,0x210425240423,rip2);
}

U0 ICOrEqu(CIntermediateCode *tmpi,I64 rip2)
{
  I64 i,bit;
  if (tmpi->arg2.type&MDF_IMM && !(tmpi->arg1.type&MDF_STK) &&
	tmpi->ic_flags&ICF_RES_NOT_USED) {
    i=tmpi->arg2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i) && i>I8_MAX) {
      tmpi->arg2.disp=bit;
      tmpi->arg2.reg=0;
      tmpi->arg1.type=tmpi->arg1.type&MDG_MASK+RT_I64;
      ICBitOps(tmpi,&tmpi->arg2,&tmpi->arg1,tmpi,0xAB0F,0x28BA0F,rip2);
      return;
    }
  }
  ICAddSubEctEqu(tmpi,tmpi->arg1_type_pointed_to,
	tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
	tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,0x09010D0C010B,rip2);
}

U0 ICXorEqu(CIntermediateCode *tmpi,I64 rip2)
{
  I64 i,bit;
  if (tmpi->arg2.type&MDF_IMM && !(tmpi->arg1.type&MDF_STK) &&
	tmpi->ic_flags&ICF_RES_NOT_USED) {
    i=tmpi->arg2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i)) {
      tmpi->arg2.disp=bit;
      tmpi->arg2.reg=0;
      tmpi->arg1.type=tmpi->arg1.type&MDG_MASK+RT_I64;
      ICBitOps(tmpi,&tmpi->arg2,&tmpi->arg1,tmpi,0xBB0F,0x38BA0F,rip2);
      return;
    }
  }
  ICAddSubEctEqu(tmpi,tmpi->arg1_type_pointed_to,
	tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
	tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,0x310635340633,rip2);
}

U0 ICSwitch(CIntermediateCode *tmpi,I64 rip,
	Bool nobound,CCmpCtrl *cc,U8 *buf,I64 rip2)
{
  I64 i,j,cnt,min,max,begin,r;
  CCodeMisc *lb;
  Bool short_jmp;
  CAOTAbsAddr *tmpa;
  if (!(tmpi->arg1.type&MDF_REG) || tmpi->arg1.reg&7==REG_RSP)
    r=REG_RDX;
  else
    r=tmpi->arg1.reg;
  if (nobound)
    ICMov(tmpi,MDF_REG+RT_I64,r,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
  else {
    if (tmpi->arg2.type&MDF_IMM) {
      j=tmpi->arg2.disp;
      ICMov(tmpi,MDF_REG+RT_I64,r,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
      if (I8_MIN<=j<=I8_MAX) {
	i=0xF88348+(r&7)<<16;
	if (r>7) i++;
	ICU24(tmpi,i);
	ICU8(tmpi,j);
      } else if (I32_MIN<=j<=I32_MAX) {
	i=0xF88148+(r&7)<<16;
	if (r>7) i++;
	ICU24(tmpi,i);
	ICU32(tmpi,j);
      } else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	      tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
	i=0xC13B48+(r&7)<<19;
	if (r>7) i+=4;
	ICU24(tmpi,i);
      }
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
      ICMov(tmpi,MDF_REG+RT_I64,r,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
      i=0xC13B48+(r&7)<<19;
      if (r>7) i+=4;
      ICU24(tmpi,i);
    }

    rip+=tmpi->ic_cnt;
    lb=tmpi->ic_data(CCodeMisc *)->dft;
    short_jmp=ToBool(tmpi->ic_flags&ICF_SHORT_JMP);
    if (!buf && lb->addr!=INVALID_PTR) {
      i=lb->addr-(rip+2);
      if (I8_MIN<=i<=I8_MAX)
	short_jmp=TRUE;
    }
    if (short_jmp) {
      tmpi->ic_flags|=ICF_SHORT_JMP;
      ICU16(tmpi,(lb->addr-(rip+2))<<8+0x73);
    } else {
      tmpi->ic_flags&=~ICF_SHORT_JMP;
      ICU16(tmpi,0x830F);
      ICU32(tmpi,lb->addr-(rip+6));
    }
  }

  lb=tmpi->ic_data;
  begin=lb->begin->addr;
  if (!buf && begin!=INVALID_PTR) {
    min=I64_MAX;
    max=I64_MIN;
    for (i=0;i<lb->range;i++) {
      if (lb->jmp_table[i]->addr==INVALID_PTR) {
	min=I64_MIN;
	max=I64_MAX;
	break;
      } else {
	j=lb->jmp_table[i]->addr-begin;
	min=MinI64(min,j);
	max=MaxI64(max,j);
      }
    }
    if (I8_MIN<=min<=max<=I8_MAX)
      lb->flags|=CMF_I8_JMP_TABLE;
    else if (U8_MIN<=min<=max<=U8_MAX)
      lb->flags|=CMF_U8_JMP_TABLE;
    else if (I16_MIN<=min<=max<=I16_MAX)
      lb->flags|=CMF_I16_JMP_TABLE;
    else if (U16_MIN<=min<=max<=U16_MAX)
      lb->flags|=CMF_U16_JMP_TABLE;
  }

  if (lb->flags&CMF_I8_JMP_TABLE) {
    if (r<8)
      ICU8(tmpi,0x48);
    else
      ICU8(tmpi,0x49);
    ICU24(tmpi,0x98BE0F+(r&7)<<16);
    cnt=1;
  } else if (lb->flags&CMF_U8_JMP_TABLE) {
    if (r<8)
      cnt=2;
    else {
      ICU8(tmpi,0x49);
      cnt=1;
    }
    ICU24(tmpi,0x98B60F+(r&7)<<16);
  } else if (lb->flags&CMF_I16_JMP_TABLE) {
    if (r<8)
      ICU8(tmpi,0x48);
    else
      ICU8(tmpi,0x4A);
    ICU32(tmpi,0x451CBF0F+(r&7)<<27);
    cnt=0;
  } else if (lb->flags&CMF_U16_JMP_TABLE) {
    if (r<8)
      cnt=1;
    else {
      ICU8(tmpi,0x4A);
      cnt=0;
    }
    ICU32(tmpi,0x451CB70F+(r&7)<<27);
  } else {
    if (r<8)
      cnt=2;
    else {
      ICU8(tmpi,0x42);
      cnt=1;
    }
    ICU24(tmpi,0x851C8B+(r&7)<<19);
  }
  if (buf && cc->flags&CCF_AOT_COMPILE) {
    tmpa=CAlloc(sizeof(CAOTAbsAddr));
    tmpa->next=cc->aotc->abss;
    tmpa->type=AAT_ADD_U32;
    cc->aotc->abss=tmpa;
    tmpa->rip=rip2+tmpi->ic_cnt;
    ICU32(tmpi,lb->addr+cc->aotc->rip);
  } else
    ICU32(tmpi,lb->addr+buf);

  if (lb->flags&(CMF_I8_JMP_TABLE|CMF_U8_JMP_TABLE|
	CMF_I16_JMP_TABLE|CMF_U16_JMP_TABLE)) {
    ICU16(tmpi,0xC381); //ADD EBX,0x12345678
    if (buf && cc->flags&CCF_AOT_COMPILE) {
      tmpa=CAlloc(sizeof(CAOTAbsAddr));
      tmpa->next=cc->aotc->abss;
      tmpa->type=AAT_ADD_U32;
      cc->aotc->abss=tmpa;
      tmpa->rip=rip2+tmpi->ic_cnt;
      ICU32(tmpi,begin+cc->aotc->rip);
    } else
      ICU32(tmpi,begin+buf);
  } else
    cnt+=6;
  ICU16(tmpi,0xE3FF); //JMP EBX
  for (i=0;i<cnt;i++) //Code must always shrink, not expand
    ICU8(tmpi,OC_NOP);
  tmpi->ic_flags&=~ICF_CODE_FINAL;
}

U0 ICLocalVarInit(CIntermediateCode *tmpi)
{
  ICU24(tmpi,0xC48B48);
  ICU16(tmpi,0x5748);
  ICU24(tmpi,0xF88B48);
  ICU24(tmpi,0xC1C748);
  ICU32(tmpi,tmpi->ic_data);
  ICU16(tmpi,sys_var_init_val<<8+0xB0);
  ICU24(tmpi,0xAA48F3);
  ICU16(tmpi,0x5F48);
}
