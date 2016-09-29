U0 ICAndBranch(CIntermediateCode *tmpi,I64 ip,I64 is,U8 *buf,I64 ip2)
{
  U64 i;
  I64 it,t1,r1,d1,r2;
  Bool short_jmp,swap,override;
  CCodeMisc *lb;
  CICArg *a1,*a2;

  if (tmpi->a1.type&MDF_IMM) {
    swap=TRUE;
    a1=&tmpi->a2;
    a2=&tmpi->a1;
  } else {
    swap=FALSE;
    a1=&tmpi->a1;
    a2=&tmpi->a2;
  }

  if (a2->type&MDF_IMM && a2->disp>MAX_U32)
    override=TRUE;
  else
    override=FALSE;

  if (a1->type.raw_type<a2->type.raw_type)
    it=a1->type.raw_type;
  else
    it=a2->type.raw_type;

  i=a2->disp;
  if (a2->type&MDF_IMM && i<=MAX_U32) {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,a1->type,a1->reg,a1->disp,ip2);
    if (i<=MAX_U8)
      ICU16(tmpi,i<<8+0xA8);
    else if (i<=MAX_U16)
      ICU32(tmpi,i<<16+0xA900+OC_OP_SIZE_PREFIX);
    else {
      ICU8(tmpi,0xA9);
      ICU32(tmpi,i);
    }
  } else {
    t1=MDF_REG+it;
    d1=0;
    if (swap && !override) {
      if (a1->type&MDF_REG) {
	r1=a1->reg;
	swap=TRUE;
      } else {
	r1=REG_RCX;
	swap=FALSE;
      }
      if (a2->type&MDF_REG) {
	r2=a2->reg;
	swap=FALSE;
      } else
	r2=REG_RDX;
      if (swap) {
	if (!(a1->type&MDF_REG) || r1!=a1->reg)
	  ICMov(tmpi,MDF_REG+RT_I64,r1,0,a1->type,a1->reg,a1->disp,ip2);
	if (a2->type&MDG_REG_DISP_SIB_RIP) {
	  t1=a2->type&MDG_MASK+it;
	  r2=a2->reg;
	  d1=a2->disp;
	} else
	  ICMov(tmpi,MDF_REG+RT_I64,r2,0,a2->type,a2->reg,a2->disp,ip2);
	i=ICModr1(r1,t1,r2,d1);
      } else {
	if (a1->type&MDG_REG_DISP_SIB_RIP) {
	  t1=a1->type&MDG_MASK+it;
	  r1=a1->reg;
	  d1=a1->disp;
	} else
	  ICMov(tmpi,MDF_REG+RT_I64,r1,0,a1->type,a1->reg,a1->disp,ip2);
	if (!(a2->type&MDF_REG) || r2!=a2->reg)
	  ICMov(tmpi,MDF_REG+RT_I64,r2,0,a2->type,a2->reg,a2->disp,ip2);
	i=ICModr1(r2,t1,r1,d1);
      }
    } else {
      if (a2->type&MDF_REG) {
	r2=a2->reg;
	swap=FALSE;
      } else {
	r2=REG_RDX;
	swap=TRUE;
      }
      if (a1->type&MDF_REG) {
	r1=a1->reg;
	swap=TRUE;
      } else
	r1=REG_RCX;
      if (override)
	swap=FALSE;
      if (swap) {
	if (a2->type&MDG_REG_DISP_SIB_RIP) {
	  t1=a2->type&MDG_MASK+it;
	  r2=a2->reg;
	  d1=a2->disp;
	} else
	  ICMov(tmpi,MDF_REG+RT_I64,r2,0,a2->type,a2->reg,a2->disp,ip2);
	if (!(a1->type&MDF_REG) || r1!=a1->reg)
	  ICMov(tmpi,MDF_REG+RT_I64,r1,0,a1->type,a1->reg,a1->disp,ip2);
	i=ICModr1(r1,t1,r2,d1);
      } else {
	if (!(a2->type&MDF_REG) || r2!=a2->reg)
	  ICMov(tmpi,MDF_REG+RT_I64,r2,0,a2->type,a2->reg,a2->disp,ip2);
	if (a1->type&MDG_REG_DISP_SIB_RIP) {
	  t1=a1->type&MDG_MASK+it;
	  r1=a1->reg;
	  d1=a1->disp;
	} else
	  ICMov(tmpi,MDF_REG+RT_I64,r1,0,a1->type,a1->reg,a1->disp,ip2);
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
    ICModr2(tmpi,i,d1,ip2+4);
  }

  ip+=tmpi->ic_cnt;
  lb=OptLabelFwd(tmpi->ic_data);
  short_jmp=ToBool(tmpi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tmpi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tmpi,i<<8+is.u8[2]);
  } else {
    tmpi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tmpi,is.u16[0]);
    ICU32(tmpi,i);
  }
}

U0 ICAssign(CIntermediateCode *tmpi,I64 ip)
{
  CIntermediateCode *tmpi1;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    ICMov(tmpi,tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	  tmpi->a1.reg,tmpi->a1.disp,
	  tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
    if (tmpi->r.type.mode)
      ICMov(tmpi,tmpi->r.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->r.reg,tmpi->r.disp,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,ip);
  } else {
    if (tmpi->a1.type&MDF_REG) {
      if (!(tmpi1=OptLag1(tmpi)) || tmpi1->ic_code!=IC_ADD_CONST ||
	    tmpi1->r.type!=MDF_REG+RT_I64 || tmpi1->r.reg!=tmpi->a1.reg ||
	    tmpi1->a1.type!=MDF_REG+RT_I64 || tmpi1->a1.reg!=tmpi->a1.reg ||
	    (tmpi->a2.type&MDF_REG || tmpi->a2.type&MDF_DISP) &&
	    tmpi->a2.reg==tmpi->a1.reg ||
	    tmpi->r.type.mode || tmpi1->ic_flags&~ICG_NO_CVT_MASK) {
	ICMov(tmpi,
	      MDF_DISP+tmpi->a1_type_pointed_to,tmpi->a1.reg,tmpi->a1.disp,
	      tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
	if (tmpi->r.type.mode)
	  ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
		tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
      } else {
	tmpi->ic_flags=(tmpi->ic_flags|tmpi1->ic_flags)&
	      ~ICF_CODE_FINAL|ICF_DONT_RESTORE;
	tmpi->a1.disp=tmpi1->ic_data;
	OptSetNOP1(tmpi1); //This better not be last pass!
	ICMov(tmpi,MDF_DISP+tmpi->a1_type_pointed_to,
	      tmpi->a1.reg,tmpi->a1.disp,
	      tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
      }
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
      ICMov(tmpi,MDF_DISP+tmpi->a1_type_pointed_to,REG_RCX,0,
	    tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
      if (tmpi->r.type.mode)
	ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	      tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
    }
  }
}

U0 ICBrBitOps(CIntermediateCode *tmpi,I64 ip,
	I64 op,I64 op_imm,I64 is,U8 *buf,I64 ip2)
{
  I64 i,t,r1,r2,d1,d2,t1,t2;
  CICArg *a1=&tmpi->a1,*a2=&tmpi->a2;
  Bool short_jmp;
  CCodeMisc *lb;

  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (tmpi->ic_flags&ICF_SWAP && !(a2->type&MDF_REG) &&
	  (!(a2->type&MDF_IMM) ||a2->disp>63)||
	  !(tmpi->ic_flags&ICF_SWAP) && a2->type&MDF_IMM &&
	  a2->disp<64 || a2->type&MDF_STK) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,a2->type,a2->reg,a2->disp,ip2);
      t2=MDF_REG+RT_I64;
      r2=REG_RCX;
      d2=0;
    } else {
      t2=a2->type;
      if (t2&MDF_IMM && a2->disp<64)
	r2=0;
      else
	r2=a2->reg;
      d2=a2->disp;
    }
    if (!(tmpi->ic_flags&ICF_SWAP) && !(a1->type&MDF_REG) &&
	  (!(a1->type&MDF_IMM) || a1->disp>63) ||
	  tmpi->ic_flags&ICF_SWAP && a1->type&MDF_IMM &&
	  a1->disp<64 || a1->type&MDF_STK) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip2);
      t1=MDF_REG+RT_I64;
      r1=REG_RDX;
      d1=0;
    } else {
      t1=a1->type;
      if (t1&MDF_IMM && a1->disp<64)
	r1=0;
      else
	r1=a1->reg;
      d1=a1->disp;
    }
  } else {
    t1=MDF_DISP+RT_I64;
    t2=MDF_DISP+RT_I64;
    d1=0;
    d2=0;
    if (a2->type&MDF_REG)
      r2=a2->reg;
    else if (!(tmpi->ic_flags&ICF_SWAP) || !(a2->type&MDF_IMM) ||
	  a2->disp>63) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,a2->type,a2->reg,a2->disp,ip2);
      r2=REG_RCX;
    } else
      r2=0;
    if (a1->type&MDF_REG)
      r1=a1->reg;
    else if (tmpi->ic_flags&ICF_SWAP || !(a1->type&MDF_IMM) || a1->disp>63) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip2);
      r1=REG_RDX;
    } else
      r1=0;
  }
  if (tmpi->ic_flags&ICF_LOCK && op!=0xA30F)
    ICU8(tmpi,OC_LOCK_PREFIX);
  if (tmpi->ic_flags&ICF_SWAP) {
    if (a2->type&MDF_IMM && a2->disp<32) {
      if (op==0xA30F && a2->disp<8) {
	t=t1&MDG_MASK+RT_U8;
	op_imm=0xF6; //TEST
      } else
	t=t1&MDG_MASK+RT_U32;
    } else
      t=t1;
    i=ICModr1(r2,t,r1,d1);
    SwapI64(&a1,&a2);
  } else {
    if (a1->type&MDF_IMM && a1->disp<32) {
      if (op==0xA30F && a1->disp<8) {
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
    ICModr2(tmpi,i,a2->disp,ip2+5);
    ICU8(tmpi,1<<a1->disp);
    if (is==0x72820F)
      is=0x75850F;
    else
      is=0x74840F;
  } else if (a1->type&MDF_IMM && a1->disp<64) {
    ICU24(tmpi,i.u8[2]<<16+op_imm);
    ICModr2(tmpi,i,a2->disp,ip2+5);
    ICU8(tmpi,a1->disp);
  } else {
    ICU24(tmpi,i.u8[2]<<16+op);
    ICModr2(tmpi,i,a2->disp,ip2+4);
  }

  ip+=tmpi->ic_cnt;
  lb=OptLabelFwd(tmpi->ic_data);
  short_jmp=ToBool(tmpi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tmpi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tmpi,i<<8+is.u8[2]);
  } else {
    tmpi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tmpi,is.u16[0]);
    ICU32(tmpi,i);
  }
}

U0 ICQueInit(CIntermediateCode *tmpi,I64 ip2)
{
  I64 r1;
  if (tmpi->a1.type==MDF_REG+RT_I64)
    r1=tmpi->a1.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
    r1=REG_RAX;
  }
  ICMov(tmpi,MDF_DISP+RT_I64,r1,0,MDF_REG+RT_I64,r1,0,ip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r1,sizeof(U8 *),MDF_REG+RT_I64,r1,0,ip2);
}

U0 ICQueIns(CIntermediateCode *tmpi,I64 ip2)
{
  I64 r1,r2;
  if (tmpi->a2.type==MDF_REG+RT_I64 && tmpi->a2.reg!=REG_RDX)
    r2=tmpi->a2.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip2);
    r2=REG_RAX;
  }
  if (tmpi->a1.type==MDF_REG+RT_I64)
    r1=tmpi->a1.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
    r1=REG_RDX;
  }
  ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,MDF_DISP+RT_I64,r1,0,ip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r1,0,MDF_REG+RT_I64,r2,0,ip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r2,0,MDF_REG+RT_I64,REG_RBX,0,ip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r2,sizeof(U8 *),MDF_REG+RT_I64,r1,0,ip2);
  ICMov(tmpi,MDF_DISP+RT_I64,REG_RBX,sizeof(U8 *),MDF_REG+RT_I64,r2,0,ip2);
}

U0 ICQueInsRev(CIntermediateCode *tmpi,I64 ip2)
{
  I64 r1,r2;
  if (tmpi->a2.type==MDF_REG+RT_I64 && tmpi->a2.reg!=REG_RDX)
    r2=tmpi->a2.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip2);
    r2=REG_RAX;
  }
  if (tmpi->a1.type==MDF_REG+RT_I64)
    r1=tmpi->a1.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
    r1=REG_RDX;
  }
  ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,MDF_DISP+RT_I64,r1,sizeof(U8 *),ip2);
  ICMov(tmpi,MDF_DISP+RT_I64,REG_RBX,0,MDF_REG+RT_I64,r2,0,ip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r2,0,MDF_REG+RT_I64,r1,0,ip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r2,sizeof(U8 *),MDF_REG+RT_I64,REG_RBX,0,ip2);
  ICMov(tmpi,MDF_DISP+RT_I64,r1,sizeof(U8 *),MDF_REG+RT_I64,r2,0,ip2);
}

U0 ICQueRem(CIntermediateCode *tmpi,I64 ip2)
{
  I64 r1;
  if (tmpi->a1.type==MDF_REG+RT_I64)
    r1=tmpi->a1.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
    r1=REG_RAX;
  }
  ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,MDF_DISP+RT_I64,r1,0,ip2);
  ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_I64,r1,sizeof(U8 *),ip2);
  ICU24(tmpi,0x1A8948);
  ICU32(tmpi,sizeof(U8 *)<<24+0x538948);
}

U0 ICMinMax(CIntermediateCode *tmpi,I64 op,I64 ip2)
{
  I64 r1,i1=0x48;
  if (tmpi->a2.type==MDF_REG+RT_I64 && tmpi->a2.reg!=REG_RAX) {
    r1=tmpi->a2.reg;
    if (r1>7) {
      i1++;
      r1&=7;
    }
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
  } else {
    if (tmpi->a1.reg==REG_RAX && tmpi->a1.type&MDG_REG_DISP_SIB) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip2);
      r1=REG_RDX;
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip2);
      if (tmpi->a1.type==MDF_REG+RT_I64) {
	r1=tmpi->a1.reg;
	if (r1>7) {
	  i1++;
	  r1&=7;
	}
      } else {
	r1=REG_RDX;
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	      tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
      }
    }
  }
  ICU24(tmpi,0xC03B00+r1<<16+i1);
  ICU32(tmpi,0xC0000F00+op<<16+r1<<24+i1);
}

U0 ICSqr(CIntermediateCode *tmpi,I64 op,I64 ip2)
{
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
  ICSlashOp(tmpi,MDF_REG+RT_I64,REG_RAX,0,op,ip2);
}

U0 ICModU64(CIntermediateCode *tmpi,I64 ip2)
{
  CICType t1;
  I64 r1,d1;
  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip2);
  if (tmpi->a1.reg!=REG_RAX && tmpi->a1.reg!=REG_RDX &&
	tmpi->a1.type&MDG_REG_DISP_SIB &&
	tmpi->a1.type.raw_type>=RT_I64) {
    t1=tmpi->a1.type;
    r1=tmpi->a1.reg;
    d1=tmpi->a1.disp;
  } else {
    t1=MDF_REG+RT_I64;
    r1=REG_RBX;
    d1=0;
    ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
  }
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,MDF_DISP+RT_I64,REG_RCX,0,ip2);
  ICZero(tmpi,REG_RDX);
  ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_DIV,ip2);
  ICMov(tmpi,MDF_DISP+RT_I64,REG_RCX,0,MDF_REG+RT_I64,REG_RAX,0,ip2);
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	MDF_REG+RT_I64,REG_RDX,0,ip2);
}

U0 ICSwap(CIntermediateCode *tmpi,I64 ip2)
{
  I64 r1,r2;
  if (tmpi->a1.type&MDF_REG)
    r1=tmpi->a1.reg;
  else
    r1=REG_RAX;
  if (tmpi->a2.type&MDF_REG)
    r2=tmpi->a2.reg;
  else
    r2=REG_RAX;
  if (r1==r2) {
    if (r1==REG_RAX)
      r1=REG_RBX;
    else
      r2=REG_RAX;
  }
  ICMov(tmpi,MDF_REG+RT_I64,r2,0,
	tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip2);
  ICMov(tmpi,MDF_REG+RT_I64,r1,0,
	tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
  switch (tmpi->ic_code) {
    case IC_SWAP_U8:
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_DISP+RT_U8,r1,0,ip2);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_U8,r2,0,ip2);
      ICMov(tmpi,MDF_DISP+RT_U8,r2,0,MDF_REG+RT_I64,REG_RCX,0,ip2);
      ICMov(tmpi,MDF_DISP+RT_U8,r1,0,MDF_REG+RT_I64,REG_RDX,0,ip2);
      break;
    case IC_SWAP_U16:
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_DISP+RT_U16,r1,0,ip2);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_U16,r2,0,ip2);
      ICMov(tmpi,MDF_DISP+RT_U16,r2,0,MDF_REG+RT_I64,REG_RCX,0,ip2);
      ICMov(tmpi,MDF_DISP+RT_U16,r1,0,MDF_REG+RT_I64,REG_RDX,0,ip2);
      break;
    case IC_SWAP_U32:
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_DISP+RT_U32,r1,0,ip2);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_U32,r2,0,ip2);
      ICMov(tmpi,MDF_DISP+RT_U32,r2,0,MDF_REG+RT_I64,REG_RCX,0,ip2);
      ICMov(tmpi,MDF_DISP+RT_U32,r1,0,MDF_REG+RT_I64,REG_RDX,0,ip2);
      break;
    case IC_SWAP_I64:
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_DISP+RT_U64,r1,0,ip2);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_U64,r2,0,ip2);
      ICMov(tmpi,MDF_DISP+RT_U64,r2,0,MDF_REG+RT_I64,REG_RCX,0,ip2);
      ICMov(tmpi,MDF_DISP+RT_U64,r1,0,MDF_REG+RT_I64,REG_RDX,0,ip2);
      break;
  }
}

U0 ICAndEqu(CIntermediateCode *tmpi,I64 ip2)
{
  I64 i,bit;
  if (tmpi->a2.type&MDF_IMM && !(tmpi->a1.type&MDF_STK) &&
	tmpi->ic_flags&ICF_RES_NOT_USED) {
    i=~tmpi->a2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i)) {
      tmpi->a2.disp=bit;
      tmpi->a2.reg=0;
      tmpi->a1.type=tmpi->a1.type&MDG_MASK+RT_I64;
      ICBitOps(tmpi,&tmpi->a2,&tmpi->a1,tmpi,0xB30F,0x30BA0F,ip2);
      return;
    }
  }
  ICAddSubEctEqu(tmpi,tmpi->a1_type_pointed_to,
	tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,
	tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,0x210425240423,ip2);
}

U0 ICOrEqu(CIntermediateCode *tmpi,I64 ip2)
{
  I64 i,bit;
  if (tmpi->a2.type&MDF_IMM && !(tmpi->a1.type&MDF_STK) &&
	tmpi->ic_flags&ICF_RES_NOT_USED) {
    i=tmpi->a2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i) && i>MAX_I8) {
      tmpi->a2.disp=bit;
      tmpi->a2.reg=0;
      tmpi->a1.type=tmpi->a1.type&MDG_MASK+RT_I64;
      ICBitOps(tmpi,&tmpi->a2,&tmpi->a1,tmpi,0xAB0F,0x28BA0F,ip2);
      return;
    }
  }
  ICAddSubEctEqu(tmpi,tmpi->a1_type_pointed_to,
	tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,
	tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,0x09010D0C010B,ip2);
}

U0 ICXorEqu(CIntermediateCode *tmpi,I64 ip2)
{
  I64 i,bit;
  if (tmpi->a2.type&MDF_IMM && !(tmpi->a1.type&MDF_STK) &&
	tmpi->ic_flags&ICF_RES_NOT_USED) {
    i=tmpi->a2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i)) {
      tmpi->a2.disp=bit;
      tmpi->a2.reg=0;
      tmpi->a1.type=tmpi->a1.type&MDG_MASK+RT_I64;
      ICBitOps(tmpi,&tmpi->a2,&tmpi->a1,tmpi,0xBB0F,0x38BA0F,ip2);
      return;
    }
  }
  ICAddSubEctEqu(tmpi,tmpi->a1_type_pointed_to,
	tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,
	tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,0x310635340633,ip2);
}

U0 ICSwitch(CIntermediateCode *tmpi,I64 ip,
	Bool nobound,CCmpCtrl *cc,U8 *buf,I64 ip2)
{
  I64 i,j,cnt,min,max,begin,r;
  CCodeMisc *lb;
  Bool short_jmp;
  CAOTAbsAddr *tmpa;
  if (!(tmpi->a1.type&MDF_REG) || tmpi->a1.reg&7==REG_RSP)
    r=REG_RDX;
  else
    r=tmpi->a1.reg;
  if (nobound)
    ICMov(tmpi,MDF_REG+RT_I64,r,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
  else {
    if (tmpi->a2.type&MDF_IMM) {
      j=tmpi->a2.disp;
      ICMov(tmpi,MDF_REG+RT_I64,r,0,
	    tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
      if (MIN_I8<=j<=MAX_I8) {
	i=0xF88348+(r&7)<<16;
	if (r>7) i++;
	ICU24(tmpi,i);
	ICU8(tmpi,j);
      } else if (MIN_I32<=j<=MAX_I32) {
	i=0xF88148+(r&7)<<16;
	if (r>7) i++;
	ICU24(tmpi,i);
	ICU32(tmpi,j);
      } else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	      tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip2);
	i=0xC13B48+(r&7)<<19;
	if (r>7) i+=4;
	ICU24(tmpi,i);
      }
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip2);
      ICMov(tmpi,MDF_REG+RT_I64,r,0,
	    tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
      i=0xC13B48+(r&7)<<19;
      if (r>7) i+=4;
      ICU24(tmpi,i);
    }

    ip+=tmpi->ic_cnt;
    lb=tmpi->ic_data(CCodeMisc *)->dft;
    short_jmp=ToBool(tmpi->ic_flags&ICF_SHORT_JMP);
    if (!buf && lb->addr!=INVALID_PTR) {
      i=lb->addr-(ip+2);
      if (MIN_I8<=i<=MAX_I8)
	short_jmp=TRUE;
    }
    if (short_jmp) {
      tmpi->ic_flags|=ICF_SHORT_JMP;
      ICU16(tmpi,(lb->addr-(ip+2))<<8+0x73);
    } else {
      tmpi->ic_flags&=~ICF_SHORT_JMP;
      ICU16(tmpi,0x830F);
      ICU32(tmpi,lb->addr-(ip+6));
    }
  }

  lb=tmpi->ic_data;
  begin=lb->begin->addr;
  if (!buf && begin!=INVALID_PTR) {
    min=MAX_I64;
    max=MIN_I64;
    for (i=0;i<lb->range;i++) {
      if (lb->jmp_table[i]->addr==INVALID_PTR) {
	min=MIN_I64;
	max=MAX_I64;
	break;
      } else {
	j=lb->jmp_table[i]->addr-begin;
	min=MinI64(min,j);
	max=MaxI64(max,j);
      }
    }
    if (MIN_I8<=min<=max<=MAX_I8)
      lb->flags|=CMF_I8_JMP_TABLE;
    else if (MIN_U8<=min<=max<=MAX_U8)
      lb->flags|=CMF_U8_JMP_TABLE;
    else if (MIN_I16<=min<=max<=MAX_I16)
      lb->flags|=CMF_I16_JMP_TABLE;
    else if (MIN_U16<=min<=max<=MAX_U16)
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
    tmpa->ip=ip2+tmpi->ic_cnt;
    ICU32(tmpi,lb->addr+cc->aotc->ip);
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
      tmpa->ip=ip2+tmpi->ic_cnt;
      ICU32(tmpi,begin+cc->aotc->ip);
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
  ICU16(tmpi,0xAAF3);
  ICU16(tmpi,0x5F48);
}
