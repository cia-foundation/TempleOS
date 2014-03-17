U0 ICAndBranch(CIntermediateCode *tempi,I64 ip,I64 is,U8 *buf,I64 ip2)
{
  U64 i;
  I64 it,t1,r1,d1,r2;
  Bool short_jmp,swap,override;
  CLexCodeMisc *lb;
  CICArg *a1,*a2;

  if (tempi->a1.type&TY_MASK==TY_IMM) {
    swap=TRUE;
    a1=&tempi->a2;
    a2=&tempi->a1;
  } else {
    swap=FALSE;
    a1=&tempi->a1;
    a2=&tempi->a2;
  }

  if (a2->type&TY_MASK==TY_IMM && a2->disp>MAX_U32)
    override=TRUE;
  else
    override=FALSE;

  if (a1->type&RT_MASK<a2->type&RT_MASK)
    it=a1->type&RT_MASK;
  else
    it=a2->type&RT_MASK;

  i=a2->disp;
  if (a2->type&TY_MASK==TY_IMM && i<=MAX_U32) {
    ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,a1->type,a1->reg,a1->disp,ip2);
    if (i<=MAX_U8)
      ICU16(tempi,i<<8+0xA8);
    else if (i<=MAX_U16)
      ICU32(tempi,i<<16+0xA900+OC_OP_SIZE_PREFIX);
    else {
      ICU8(tempi,0xA9);
      ICU32(tempi,i);
    }
  } else {
    t1=TY_REG+it;
    d1=0;
    if ((tempi->ic_flags&ICF_A1_FIRST ^^swap)&&!override) {
      if (a1->type&TY_MASK==TY_REG) {
	r1=a1->reg;
	swap=TRUE;
      } else {
	r1=REG_RCX;
	swap=FALSE;
      }
      if (a2->type&TY_MASK==TY_REG) {
	r2=a2->reg;
	swap=FALSE;
      } else
	r2=REG_RDX;
      if (swap) {
	if (a1->type&TY_MASK!=TY_REG || r1!=a1->reg)
	  ICMov(tempi,TY_REG+RT_I64,r1,0,a1->type,a1->reg,a1->disp,ip2);
	if (TY_REG<=a2->type&TY_MASK<=TY_RIP_DISP32) {
	  t1=a2->type&TY_MASK+it;
	  r2=a2->reg;
	  d1=a2->disp;
	} else
	  ICMov(tempi,TY_REG+RT_I64,r2,0,a2->type,a2->reg,a2->disp,ip2);
	i=ICModr1(r1,t1,r2,d1);
      } else {
	if (TY_REG<=a1->type&TY_MASK<=TY_RIP_DISP32) {
	  t1=a1->type&TY_MASK+it;
	  r1=a1->reg;
	  d1=a1->disp;
	} else
	  ICMov(tempi,TY_REG+RT_I64,r1,0,a1->type,a1->reg,a1->disp,ip2);
	if (a2->type&TY_MASK!=TY_REG || r2!=a2->reg)
	  ICMov(tempi,TY_REG+RT_I64,r2,0,a2->type,a2->reg,a2->disp,ip2);
	i=ICModr1(r2,t1,r1,d1);
      }
    } else {
      if (a2->type&TY_MASK==TY_REG) {
	r2=a2->reg;
	swap=FALSE;
      } else {
	r2=REG_RDX;
	swap=TRUE;
      }
      if (a1->type&TY_MASK==TY_REG) {
	r1=a1->reg;
	swap=TRUE;
      } else
	r1=REG_RCX;
      if (override)
	swap=FALSE;
      if (swap) {
	if (TY_REG<=a2->type&TY_MASK<=TY_RIP_DISP32) {
	  t1=a2->type&TY_MASK+it;
	  r2=a2->reg;
	  d1=a2->disp;
	} else
	  ICMov(tempi,TY_REG+RT_I64,r2,0,a2->type,a2->reg,a2->disp,ip2);
	if (a1->type&TY_MASK!=TY_REG || r1!=a1->reg)
	  ICMov(tempi,TY_REG+RT_I64,r1,0,a1->type,a1->reg,a1->disp,ip2);
	i=ICModr1(r1,t1,r2,d1);
      } else {
	if (a2->type&TY_MASK!=TY_REG || r2!=a2->reg)
	  ICMov(tempi,TY_REG+RT_I64,r2,0,a2->type,a2->reg,a2->disp,ip2);
	if (TY_REG<=a1->type&TY_MASK<=TY_RIP_DISP32) {
	  t1=a1->type&TY_MASK+it;
	  r1=a1->reg;
	  d1=a1->disp;
	} else
	  ICMov(tempi,TY_REG+RT_I64,r1,0,a1->type,a1->reg,a1->disp,ip2);
	i=ICModr1(r2,t1,r1,d1);
      }
    }
    switch (it) {
      case RT_I8:
      case RT_U8:
	ICRex(tempi,i.u8[1]);
	ICU16(tempi,i.u8[2]<<8+0x84);
	break;
      case RT_U16:
      case RT_I16:
	ICOpSizeRex(tempi,i.u8[1]);
	ICU16(tempi,i.u8[2]<<8+0x85);
	break;
      default:
	ICRex(tempi,i.u8[1]);
	ICU16(tempi,i.u8[2]<<8+0x85);
    }
    ICModr2(tempi,i,d1,ip2+4);
  }

  ip+=tempi->ic_cnt;
  lb=OptLabelFwd(tempi->ic_data);
  short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tempi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,i<<8+is.u8[2]);
  } else {
    tempi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,is.u16[0]);
    ICU32(tempi,i);
  }
}

U0 ICAssign(CIntermediateCode *tempi,I64 ip)
{
  CIntermediateCode *tempi1;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    ICMov(tempi,tempi->a1.type&TY_MASK+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    if (tempi->r.type&TY_MASK) {
      if (tempi->a1.type&TY_MASK==TY_REG)
	ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->a1.type&TY_MASK+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,ip);
      else
	ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    }
  } else {
    if (tempi->a1.type&TY_MASK==TY_REG) {
      if (!(tempi1=OptLag1(tempi)) ||
	  tempi1->ic_code!=IC_ADD_CONST ||
	  tempi1->r.type!=TY_REG+RT_I64 || tempi1->r.reg!=tempi->a1.reg ||
	  tempi1->a1.type!=TY_REG+RT_I64 || tempi1->a1.reg!=tempi->a1.reg ||
	  (tempi->a2.type&TY_MASK==TY_REG||tempi->a2.type&TY_MASK==TY_DISP) &&
	  tempi->a2.reg==tempi->a1.reg ||
	  tempi->r.type&TY_MASK ||
	  tempi1->ic_flags&~ICG_NO_CVT_MASK) {
	ICMov(tempi,TY_DISP+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,
	      tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
	if (tempi->r.type&TY_MASK)
	  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
      } else {
	tempi->ic_flags=(tempi->ic_flags|tempi1->ic_flags)&~ICF_CODE_FINAL|ICF_DONT_RESTORE;
	tempi->a1.disp=tempi1->ic_data;
	OptSetNOP1(tempi1); //This better not be last pass!
	ICMov(tempi,TY_DISP+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,
	      tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
      }
    } else {
      ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,
	    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      ICMov(tempi,TY_DISP+tempi->a1.type_pointed,REG_RCX,0,
	    tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
      if (tempi->r.type&TY_MASK)
	ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    }
  }
}


U0 ICBrBitOps(CIntermediateCode *tempi,I64 ip,I64 op,I64 op_imm,I64 is,U8 *buf,I64 ip2)
{
  I64 i,t,r1,r2,d1,d2,t1,t2;
  CICArg *a1=&tempi->a1,*a2=&tempi->a2;
  Bool short_jmp;
  CLexCodeMisc *lb;

  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempi->ic_flags & ICF_A1_FIRST) {
      if (!(tempi->ic_flags&ICF_SWAP) && a1->type&TY_MASK!=TY_REG && (a1->type&TY_MASK!=TY_IMM || a1->disp>63) ||
	    tempi->ic_flags&ICF_SWAP && a1->type&TY_MASK==TY_IMM && a1->disp<64 ||
	    a1->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,a1->type,a1->reg,a1->disp,ip2);
	t1=TY_REG+RT_I64;
	r1=REG_RCX;
	d1=0;
      } else {
	t1=a1->type;
	if (t1&TY_MASK==TY_IMM && a1->disp<64)
	  r1=0;
	else
	  r1=a1->reg;
	d1=a1->disp;
      }
      if (tempi->ic_flags&ICF_SWAP && a2->type&TY_MASK!=TY_REG && (a2->type&TY_MASK!=TY_IMM || a2->disp>63) ||
	    !(tempi->ic_flags&ICF_SWAP) && a2->type&TY_MASK==TY_IMM && a2->disp<64 ||
	    a2->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,a2->type,a2->reg,a2->disp,ip2);
	t2=TY_REG+RT_I64;
	r2=REG_RDX;
	d2=0;
      } else {
	t2=a2->type;
	if (t2&TY_MASK==TY_IMM && a2->disp<64)
	  r2=0;
	else
	  r2=a2->reg;
	d2=a2->disp;
      }
    } else {
      if (tempi->ic_flags&ICF_SWAP && a2->type&TY_MASK!=TY_REG && (a2->type&TY_MASK!=TY_IMM ||a2->disp>63)||
	    !(tempi->ic_flags&ICF_SWAP) && a2->type&TY_MASK==TY_IMM && a2->disp<64 ||
	    a2->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,a2->type,a2->reg,a2->disp,ip2);
	t2=TY_REG+RT_I64;
	r2=REG_RCX;
	d2=0;
      } else {
	t2=a2->type;
	if (t2&TY_MASK==TY_IMM && a2->disp<64)
	  r2=0;
	else
	  r2=a2->reg;
	d2=a2->disp;
      }
      if (!(tempi->ic_flags&ICF_SWAP) && a1->type&TY_MASK!=TY_REG && (a1->type&TY_MASK!=TY_IMM || a1->disp>63) ||
	    tempi->ic_flags&ICF_SWAP && a1->type&TY_MASK==TY_IMM && a1->disp<64||
	    a1->type&TY_MASK==TY_STK) {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip2);
	t1=TY_REG+RT_I64;
	r1=REG_RDX;
	d1=0;
      } else {
	t1=a1->type;
	if (t1&TY_MASK==TY_IMM && a1->disp<64)
	  r1=0;
	else
	  r1=a1->reg;
	d1=a1->disp;
      }
    }
  } else {
    t1=TY_DISP+RT_I64;
    t2=TY_DISP+RT_I64;
    d1=0;
    d2=0;
    if (tempi->ic_flags & ICF_A1_FIRST) {
      if (a1->type&TY_MASK==TY_REG)
	r1=a1->reg;
      else if (tempi->ic_flags&ICF_SWAP || a1->type&TY_MASK!=TY_IMM || a1->disp>63) {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,a1->type,a1->reg,a1->disp,ip2);
	r1=REG_RCX;
      } else
	r1=0;
      if (a2->type&TY_MASK==TY_REG)
	r2=a2->reg;
      else if (!(tempi->ic_flags&ICF_SWAP) || a2->type&TY_MASK!=TY_IMM || a2->disp>63) {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,a2->type,a2->reg,a2->disp,ip2);
	r2=REG_RDX;
      } else
	r2=0;
    } else {
      if (a2->type&TY_MASK==TY_REG)
	r2=a2->reg;
      else if (!(tempi->ic_flags&ICF_SWAP) || a2->type&TY_MASK!=TY_IMM || a2->disp>63) {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,a2->type,a2->reg,a2->disp,ip2);
	r2=REG_RCX;
      } else
	r2=0;
      if (a1->type&TY_MASK==TY_REG)
	r1=a1->reg;
      else if (tempi->ic_flags&ICF_SWAP || a1->type&TY_MASK!=TY_IMM || a1->disp>63) {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip2);
	r1=REG_RDX;
      } else
	r1=0;
    }
  }
  if (tempi->ic_flags&ICF_LOCK && op!=0xA30F)
    ICU8(tempi,OC_LOCK_PREFIX);
  if (tempi->ic_flags&ICF_SWAP) {
    if (a2->type&TY_MASK==TY_IMM && a2->disp<32) {
      if (op==0xA30F && a2->disp<8) {
	t=t1&TY_MASK+RT_U8;
	op_imm=0xF6; //TEST
      } else
	t=t1&TY_MASK+RT_U32;
    } else
      t=t1;
    i=ICModr1(r2,t,r1,d1);
    SwapU64(&a1,&a2);
  } else {
    if (a1->type&TY_MASK==TY_IMM && a1->disp<32) {
      if (op==0xA30F && a1->disp<8) {
	t=t2&TY_MASK+RT_U8;
	op_imm=0xF6; //TEST
      } else
	t=t2&TY_MASK+RT_U32;
    } else
      t=t2;
    i=ICModr1(r1,t,r2,d2);
  }
  ICRex(tempi,i.u8[1]);
  if (op_imm==0xF6) { //TEST
    ICU16(tempi,i.u8[2]<<8+op_imm);
    ICModr2(tempi,i,a2->disp,ip2+5);
    ICU8(tempi,1<<a1->disp);
    if (is==0x72820F)
      is=0x75850F;
    else
      is=0x74840F;
  } else if (a1->type&TY_MASK==TY_IMM && a1->disp<64) {
    ICU24(tempi,i.u8[2]<<16+op_imm);
    ICModr2(tempi,i,a2->disp,ip2+5);
    ICU8(tempi,a1->disp);
  } else {
    ICU24(tempi,i.u8[2]<<16+op);
    ICModr2(tempi,i,a2->disp,ip2+4);
  }

  ip+=tempi->ic_cnt;
  lb=OptLabelFwd(tempi->ic_data);
  short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tempi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,i<<8+is.u8[2]);
  } else {
    tempi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP)
      i+=8;
    ICU16(tempi,is.u16[0]);
    ICU32(tempi,i);
  }
}

U0 ICQueInit(CIntermediateCode *tempi,I64 ip2)
{
  I64 r1;
  if (tempi->a1.type==TY_REG+RT_I64)
    r1=tempi->a1.reg;
  else {
    ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
      tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
    r1=REG_RAX;
  }
  ICMov(tempi,TY_DISP+RT_I64,r1,0,TY_REG+RT_I64,r1,0,ip2);
  ICMov(tempi,TY_DISP+RT_I64,r1,sizeof(U8 *),TY_REG+RT_I64,r1,0,ip2);
}

U0 ICQueIns(CIntermediateCode *tempi,I64 ip2)
{
  I64 r1,r2;
  if (tempi->ic_flags & ICF_A1_FIRST) {
    if (tempi->a1.type==TY_REG+RT_I64 && tempi->a1.reg!=REG_RAX)
      r1=tempi->a1.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
      r1=REG_RDX;
    }
    if (tempi->a2.type==TY_REG+RT_I64)
      r2=tempi->a2.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
      r2=REG_RAX;
    }
  } else {
    if (tempi->a2.type==TY_REG+RT_I64 && tempi->a2.reg!=REG_RDX)
      r2=tempi->a2.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
      r2=REG_RAX;
    }
    if (tempi->a1.type==TY_REG+RT_I64)
      r1=tempi->a1.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
      r1=REG_RDX;
    }
  }
  ICMov(tempi,TY_REG+RT_I64,REG_RBX,0,TY_DISP+RT_I64,r1,0,ip2);
  ICMov(tempi,TY_DISP+RT_I64,r1,0,TY_REG+RT_I64,r2,0,ip2);
  ICMov(tempi,TY_DISP+RT_I64,r2,0,TY_REG+RT_I64,REG_RBX,0,ip2);
  ICMov(tempi,TY_DISP+RT_I64,r2,sizeof(U8 *),TY_REG+RT_I64,r1,0,ip2);
  ICMov(tempi,TY_DISP+RT_I64,REG_RBX,sizeof(U8 *),TY_REG+RT_I64,r2,0,ip2);
}

U0 ICQueInsRev(CIntermediateCode *tempi,I64 ip2)
{
  I64 r1,r2;
  if (tempi->ic_flags & ICF_A1_FIRST) {
    if (tempi->a1.type==TY_REG+RT_I64 && tempi->a1.reg!=REG_RAX)
      r1=tempi->a1.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
      r1=REG_RDX;
    }
    if (tempi->a2.type==TY_REG+RT_I64)
      r2=tempi->a2.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
      r2=REG_RAX;
    }
  } else {
    if (tempi->a2.type==TY_REG+RT_I64 && tempi->a2.reg!=REG_RDX)
      r2=tempi->a2.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
      r2=REG_RAX;
    }
    if (tempi->a1.type==TY_REG+RT_I64)
      r1=tempi->a1.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
      r1=REG_RDX;
    }
  }
  ICMov(tempi,TY_REG+RT_I64,REG_RBX,0,TY_DISP+RT_I64,r1,sizeof(U8 *),ip2);
  ICMov(tempi,TY_DISP+RT_I64,REG_RBX,0,TY_REG+RT_I64,r2,0,ip2);
  ICMov(tempi,TY_DISP+RT_I64,r2,0,TY_REG+RT_I64,r1,0,ip2);
  ICMov(tempi,TY_DISP+RT_I64,r2,sizeof(U8 *),TY_REG+RT_I64,REG_RBX,0,ip2);
  ICMov(tempi,TY_DISP+RT_I64,r1,sizeof(U8 *),TY_REG+RT_I64,r2,0,ip2);
}

U0 ICQueRem(CIntermediateCode *tempi,I64 ip2)
{
  I64 r1;
  if (tempi->a1.type==TY_REG+RT_I64)
    r1=tempi->a1.reg;
  else {
    ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
      tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
    r1=REG_RAX;
  }
  ICMov(tempi,TY_REG+RT_I64,REG_RBX,0,TY_DISP+RT_I64,r1,0,ip2);
  ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,TY_DISP+RT_I64,r1,sizeof(U8 *),ip2);
  ICU24(tempi,0x1A8948);
  ICU32(tempi,sizeof(U8 *)<<24+0x538948);
}

U0 ICMinMax(CIntermediateCode *tempi,I64 op,I64 ip2)
{
  I64 r1,i1=0x48;
  if (tempi->ic_flags & ICF_A1_FIRST) {
    if (tempi->a1.type==TY_REG+RT_I64 && tempi->a1.reg!=REG_RAX) {
      r1=tempi->a1.reg;
      if (r1>7) {
	i1++;
	r1&=7;
      }
      ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
    } else {
      if (tempi->a2.reg==REG_RAX && TY_REG<=tempi->a2.type&TY_MASK<=TY_SIB) {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	r1=REG_RDX;
	ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
      } else {
	ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	if (tempi->a2.type==TY_REG+RT_I64) {
	  r1=tempi->a2.reg;
	  if (r1>7) {
	    i1++;
	    r1&=7;
	  }
	} else {
	  r1=REG_RDX;
	  ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,
	    tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
	}
      }
    }
  } else {
    if (tempi->a2.type==TY_REG+RT_I64 && tempi->a2.reg!=REG_RAX) {
      r1=tempi->a2.reg;
      if (r1>7) {
	i1++;
	r1&=7;
      }
      ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
    } else {
      if (tempi->a1.reg==REG_RAX && TY_REG<=tempi->a1.type&TY_MASK<=TY_SIB) {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
	r1=REG_RDX;
	ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
      } else {
	ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
	if (tempi->a1.type==TY_REG+RT_I64) {
	  r1=tempi->a1.reg;
	  if (r1>7) {
	    i1++;
	    r1&=7;
	  }
	} else {
	  r1=REG_RDX;
	  ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,
	    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	}
      }
    }
  }
  ICU24(tempi,0xC03B00+r1<<16+i1);
  ICU32(tempi,0xC0000F00+op<<16+r1<<24+i1);
}

U0 ICSqr(CIntermediateCode *tempi,I64 op,I64 ip2)
{
  ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
  ICSlashOp(tempi,TY_REG+RT_I64,REG_RAX,0,op,ip2);
}

U0 ICModU64(CIntermediateCode *tempi,I64 ip2)
{
  I64 t1,r1,d1;
  if (tempi->ic_flags & ICF_A1_FIRST) {
    if (tempi->a1.reg!=REG_RAX && tempi->a1.reg!=REG_RDX && TY_REG<=tempi->a1.type&TY_MASK<=TY_SIB &&
	tempi->a1.type&RT_MASK>=RT_I64) {
      t1=tempi->a1.type;
      r1=tempi->a1.reg;
      d1=tempi->a1.disp;
    } else {
      t1=TY_REG+RT_I64;
      r1=REG_RBX;
      d1=0;
      ICMov(tempi,TY_REG+RT_I64,REG_RBX,0,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
    }
    ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
  } else {
    ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
    if (tempi->a1.reg!=REG_RAX && tempi->a1.reg!=REG_RDX && TY_REG<=tempi->a1.type&TY_MASK<=TY_SIB &&
	tempi->a1.type&RT_MASK>=RT_I64) {
      t1=tempi->a1.type;
      r1=tempi->a1.reg;
      d1=tempi->a1.disp;
    } else {
      t1=TY_REG+RT_I64;
      r1=REG_RBX;
      d1=0;
      ICMov(tempi,TY_REG+RT_I64,REG_RBX,0,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
    }
  }
  ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,TY_DISP+RT_I64,REG_RCX,0,ip2);
  ICZero(tempi,REG_RDX);
  ICSlashOp(tempi,t1,r1,d1,SLASH_OP_DIV,ip2);
  ICMov(tempi,TY_DISP+RT_I64,REG_RCX,0,TY_REG+RT_I64,REG_RAX,0,ip2);
  ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
    TY_REG+RT_I64,REG_RDX,0,ip2);
}

U0 ICSwap(CIntermediateCode *tempi,I64 ip2)
{
  I64 r1,r2;
  if (tempi->a1.type&TY_MASK==TY_REG)
    r1=tempi->a1.reg;
  else
    r1=REG_RAX;
  if (tempi->a2.type&TY_MASK==TY_REG)
    r2=tempi->a2.reg;
  else
    r2=REG_RAX;
  if (tempi->ic_flags & ICF_A1_FIRST) {
    if (r1==r2) {
      if (r2==REG_RAX)
	r2=REG_RBX;
      else
	r1=REG_RAX;
    }
    ICMov(tempi,TY_REG+RT_I64,r1,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
    ICMov(tempi,TY_REG+RT_I64,r2,0,tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
  } else {
    if (r1==r2) {
      if (r1==REG_RAX)
	r1=REG_RBX;
      else
	r2=REG_RAX;
    }
    ICMov(tempi,TY_REG+RT_I64,r2,0,tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
    ICMov(tempi,TY_REG+RT_I64,r1,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
  }
  switch (tempi->ic_code) {
    case IC_SWAP_U8:
      ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,TY_DISP+RT_U8,r1,0,ip2);
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,TY_DISP+RT_U8,r2,0,ip2);
      ICMov(tempi,TY_DISP+RT_U8,r2,0,TY_REG+RT_I64,REG_RCX,0,ip2);
      ICMov(tempi,TY_DISP+RT_U8,r1,0,TY_REG+RT_I64,REG_RDX,0,ip2);
      break;
    case IC_SWAP_U16:
      ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,TY_DISP+RT_U16,r1,0,ip2);
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,TY_DISP+RT_U16,r2,0,ip2);
      ICMov(tempi,TY_DISP+RT_U16,r2,0,TY_REG+RT_I64,REG_RCX,0,ip2);
      ICMov(tempi,TY_DISP+RT_U16,r1,0,TY_REG+RT_I64,REG_RDX,0,ip2);
      break;
    case IC_SWAP_U32:
      ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,TY_DISP+RT_U32,r1,0,ip2);
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,TY_DISP+RT_U32,r2,0,ip2);
      ICMov(tempi,TY_DISP+RT_U32,r2,0,TY_REG+RT_I64,REG_RCX,0,ip2);
      ICMov(tempi,TY_DISP+RT_U32,r1,0,TY_REG+RT_I64,REG_RDX,0,ip2);
      break;
    case IC_SWAP_U64:
      ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,TY_DISP+RT_U64,r1,0,ip2);
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,TY_DISP+RT_U64,r2,0,ip2);
      ICMov(tempi,TY_DISP+RT_U64,r2,0,TY_REG+RT_I64,REG_RCX,0,ip2);
      ICMov(tempi,TY_DISP+RT_U64,r1,0,TY_REG+RT_I64,REG_RDX,0,ip2);
      break;
  }
}

U0 ICAndEqual(CIntermediateCode *tempi,I64 ip2)
{
  I64 i,bit;
  if (tempi->a2.type&TY_MASK==TY_IMM &&
      tempi->a1.type&TY_MASK!=TY_STK &&
      tempi->ic_flags&ICF_NO_DEPEND_RESULT) {
    i=~tempi->a2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i)) {
      tempi->a2.disp=bit;
      tempi->a2.reg=0;
      tempi->a1.type=tempi->a1.type&TY_MASK+RT_I64;
      ICBitOps(tempi,&tempi->a2,&tempi->a1,tempi,
	0xB30F,0x30BA0F,ip2);
      return;
    }
  }
  ICAddSubEctEqual(tempi,tempi->a1.type_pointed,
	tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,0x210425240423,ip2);
}

U0 ICOrEqual(CIntermediateCode *tempi,I64 ip2)
{
  I64 i,bit;
  if (tempi->a2.type&TY_MASK==TY_IMM &&
      tempi->a1.type&TY_MASK!=TY_STK &&
      tempi->ic_flags&ICF_NO_DEPEND_RESULT) {
    i=tempi->a2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i) && i>MAX_I8) {
      tempi->a2.disp=bit;
      tempi->a2.reg=0;
      tempi->a1.type=tempi->a1.type&TY_MASK+RT_I64;
      ICBitOps(tempi,&tempi->a2,&tempi->a1,tempi,
	0xAB0F,0x28BA0F,ip2);
      return;
    }
  }
  ICAddSubEctEqual(tempi,tempi->a1.type_pointed,
	tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,0x09010D0C010B,ip2);
}

U0 ICXorEqual(CIntermediateCode *tempi,I64 ip2)
{
  I64 i,bit;
  if (tempi->a2.type&TY_MASK==TY_IMM &&
      tempi->a1.type&TY_MASK!=TY_STK &&
      tempi->ic_flags&ICF_NO_DEPEND_RESULT) {
    i=tempi->a2.disp;
    bit=Bsf(i);
    if (0<=bit==Bsr(i)) {
      tempi->a2.disp=bit;
      tempi->a2.reg=0;
      tempi->a1.type=tempi->a1.type&TY_MASK+RT_I64;
      ICBitOps(tempi,&tempi->a2,&tempi->a1,tempi,
	0xBB0F,0x38BA0F,ip2);
      return;
    }
  }
  ICAddSubEctEqual(tempi,tempi->a1.type_pointed,
	tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,0x310635340633,ip2);
}

U0 ICSwitch(CIntermediateCode *tempi,I64 ip,Bool nobound,CLex *lx,U8 *buf,I64 ip2)
{
  I64 i,j,cnt,min,max,begin,r;
  CLexCodeMisc *lb;
  Bool short_jmp;
  CExeAbsAddr *tempa;
  if (tempi->a1.type&TY_MASK!=TY_REG || tempi->a1.reg&7==REG_RSP)
    r=REG_RDX;
  else
    r=tempi->a1.reg;
  if (nobound)
    ICMov(tempi,TY_REG+RT_I64,r,0,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
  else {
    if (tempi->a2.type&TY_MASK==TY_IMM) {
      j=tempi->a2.disp;
      ICMov(tempi,TY_REG+RT_I64,r,0,
	    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
      if (MIN_I8<=j<=MAX_I8) {
	i=0xF88348+(r&7)<<16;
	if (r>7) i++;
	ICU24(tempi,i);
	ICU8(tempi,j);
      } else if (MIN_I32<=j<=MAX_I32) {
	i=0xF88148+(r&7)<<16;
	if (r>7) i++;
	ICU24(tempi,i);
	ICU32(tempi,j);
      } else {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,
	      tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
	i=0xC13B48+(r&7)<<19;
	if (r>7) i+=4;
	ICU24(tempi,i);
      }
    } else {
      if (tempi->ic_flags & ICF_A1_FIRST) {
	ICMov(tempi,TY_REG+RT_I64,r,0,
	      tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,
	      tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
      } else {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,
	      tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
	ICMov(tempi,TY_REG+RT_I64,r,0,
	      tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
      }
      i=0xC13B48+(r&7)<<19;
      if (r>7) i+=4;
      ICU24(tempi,i);
    }

    ip+=tempi->ic_cnt;
    lb=tempi->ic_data(CLexCodeMisc *)->dft;
    short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
    if (!buf && lb->addr!=INVALID_PTR) {
      i=lb->addr-(ip+2);
      if (MIN_I8<=i<=MAX_I8)
	short_jmp=TRUE;
    }
    if (short_jmp) {
      tempi->ic_flags|=ICF_SHORT_JMP;
      ICU16(tempi,(lb->addr-(ip+2))<<8+0x73);
    } else {
      tempi->ic_flags&=~ICF_SHORT_JMP;
      ICU16(tempi,0x830F);
      ICU32(tempi,lb->addr-(ip+6));
    }
  }

  lb=tempi->ic_data;
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
      ICU8(tempi,0x48);
    else
      ICU8(tempi,0x49);
    ICU24(tempi,0x98BE0F+(r&7)<<16);
    cnt=1;
  } else if (lb->flags&CMF_U8_JMP_TABLE) {
    if (r<8)
      cnt=2;
    else {
      ICU8(tempi,0x49);
      cnt=1;
    }
    ICU24(tempi,0x98B60F+(r&7)<<16);
  } else if (lb->flags&CMF_I16_JMP_TABLE) {
    if (r<8)
      ICU8(tempi,0x48);
    else
      ICU8(tempi,0x4A);
    ICU32(tempi,0x451CBF0F+(r&7)<<27);
    cnt=0;
  } else if (lb->flags&CMF_U16_JMP_TABLE) {
    if (r<8)
      cnt=1;
    else {
      ICU8(tempi,0x4A);
      cnt=0;
    }
    ICU32(tempi,0x451CB70F+(r&7)<<27);
  } else {
    if (r<8)
      cnt=2;
    else {
      ICU8(tempi,0x42);
      cnt=1;
    }
    ICU24(tempi,0x851C8B+(r&7)<<19);
  }
  if (buf && lx->flags&LF_AOT_COMPILE) {
    tempa=CAlloc(sizeof(CExeAbsAddr));
    tempa->next=lx->a->abss;
    tempa->type=EAT_ADD_U32;
    lx->a->abss=tempa;
    tempa->ip=ip2+tempi->ic_cnt;
    ICU32(tempi,lb->addr+lx->a->ip);
  } else
    ICU32(tempi,lb->addr+buf);

  if (lb->flags&(CMF_I8_JMP_TABLE|CMF_U8_JMP_TABLE|CMF_I16_JMP_TABLE|CMF_U16_JMP_TABLE)) {
    ICU16(tempi,0xC381); //ADD EBX,0x12345678
    if (buf && lx->flags&LF_AOT_COMPILE) {
      tempa=CAlloc(sizeof(CExeAbsAddr));
      tempa->next=lx->a->abss;
      tempa->type=EAT_ADD_U32;
      lx->a->abss=tempa;
      tempa->ip=ip2+tempi->ic_cnt;
      ICU32(tempi,begin+lx->a->ip);
    } else
      ICU32(tempi,begin+buf);
  } else
    cnt+=6;
  ICU16(tempi,0xE3FF); //JMP EBX
  for (i=0;i<cnt;i++) //Code must always shrink, not expand
    ICU8(tempi,OC_NOP);
  tempi->ic_flags&=~ICF_CODE_FINAL;
}

U0 ICLocalVarInit(CIntermediateCode *tempi)
{
  ICU24(tempi,0xC48B48);
  ICU16(tempi,0x5748);
  ICU24(tempi,0xF88B48);
  ICU24(tempi,0xC1C748);
  ICU32(tempi,tempi->ic_data);
  ICU16(tempi,sys_var_init_val<<8+0xB0);
  ICU16(tempi,0xAAF3);
  ICU16(tempi,0x5F48);
}
