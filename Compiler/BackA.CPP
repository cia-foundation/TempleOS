U0 ICAddEct(CIntermediateCode *tempi,CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,CICType t3,I64 r3,I64 d3,I64 op,I64 ip)
{
  I64 i,temp,res_reg=REG_RAX;
  Bool swap=FALSE;
  if (r3!=res_reg) {
    swap^=TRUE;
    SwapI64(&t2,&t3);
    SwapI64(&r2,&r3);
    SwapI64(&d2,&d3);
  }
  if (t2.raw_type>=RT_I64 && r2!=res_reg && t2&MDG_REG_DISP_SIB_RIP) {
    if (t1&MDF_REG && !(r2==r1 && t2&MDG_REG_DISP_SIB))
      res_reg=r1;
    ICMov(tempi,MDF_REG+RT_I64,res_reg,0,t3,r3,d3,ip);
    i=ICModr1(res_reg,t2,r2,d2);
    if (tempi->ic_flags&ICF_LOCK)
      ICU8(tempi,OC_LOCK_PREFIX);
    ICRex(tempi,i.u8[1]);
    ICU16(tempi,i.u8[2]<<8+op);
    ICModr2(tempi,i,d2,ip+4);
  } else {
    if (t2&MDF_REG)
      temp=r2;
    else
      temp=REG_RCX;

    if (t1&MDF_REG)
      res_reg=r1;

    if (temp==res_reg)
      res_reg=REG_RDX;
    if (swap) {
      if (r3==temp && t3&MDG_REG_DISP_SIB)
	temp=REG_RCX;
      ICMov(tempi,MDF_REG+RT_I64,temp,0,t2,r2,d2,ip);
      ICMov(tempi,MDF_REG+RT_I64,res_reg,0,t3,r3,d3,ip);
    } else {
      if (r2==res_reg && t2&MDG_REG_DISP_SIB)
	res_reg=REG_RDX;
      ICMov(tempi,MDF_REG+RT_I64,res_reg,0,t3,r3,d3,ip);
      ICMov(tempi,MDF_REG+RT_I64,temp,0,t2,r2,d2,ip);
    }
    i=0x48;
    if (res_reg>7)
      i+=4;
    if (temp>7)
      i++;
    if (tempi->ic_flags&ICF_LOCK)
      ICU8(tempi,OC_LOCK_PREFIX);
    ICU24(tempi,0xC00000+i+(temp&7)<<16+(res_reg&7)<<19+op<<8);
  }
  ICMov(tempi,t1,r1,d1,MDF_REG+RT_I64,res_reg,0,ip);
}

U0 ICAddSubEctImm(CIntermediateCode *tempi,CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,I64 d,I64 op,I64 ip)
{
  I64 i,r;
  if (op.u8[0]==0x2B) {
    op=0x0003;
    d=-d;
  }
  if (t1&MDF_REG) {
    if (!(t2&MDF_REG)) {
      ICMov(tempi,t1,r1,d1,t2,r2,d2,ip);
      t2=t1;
      r2=r1;
      d2=d1;
    }
    if (r1==r2) {
      if (r1>7)
	i=0x49;
      else
	i=0x48;
      if (!d &&
	    (op.u8[0]==0x03||op.u8[0]==0x2B||op.u8[0]==0x33||op.u8[0]==0x0B))
	return;
      else if (d==1 && op.u8[0]==0x03) {
	ICU24(tempi,0xC0FF00+op.u8[1]<<19+i+(r1&7)<<16);
	return;
      } else if (d==-1 && op.u8[0]==0x03) {
	ICU24(tempi,0xC8FF00+i+(r1&7)<<16);
	return;
      } else if (MIN_I8<=d<=MAX_I8) {
	ICU24(tempi,0xC08300+op.u8[1]<<19+i+(r1&7)<<16);
	ICU8(tempi,d);
	return;
      } else if (MIN_I32<=d<=MAX_I32) {
	ICU24(tempi,0xC08100+op.u8[1]<<19+i+(r1&7)<<16);
	ICU32(tempi,d);
	return;
      }
    }
    if (op.u8[0]==0x03 && MIN_I32<=d<=MAX_I32 &&
	  !Bt(&cmp.non_ptr_vars_mask,r2)) {
      i=ICModr1(r1,MDF_DISP+RT_I64,r2,d);
      i.u8[1]|=0x48;
      ICU24(tempi,i.u8[2]<<16+0x8D00+i.u8[1]);
      ICModr2(tempi,i,d,ip+4);
      return;
    }
  }
  switch (Bsr(t1)) {
    case MDf_REG:
    case MDf_DISP:
    case MDf_SIB:
    case MDf_RIP_DISP32:
      if (t1!=t2 || r1!=r2 || d1!=d2) {
	ICMov(tempi,t1,r1,d1,t2,r2,d2,ip);
	t2=t1;
	r2=r1;
	d2=d1;
      }

      if (!d &&(op.u8[0]==0x03||op.u8[0]==0x2B||op.u8[0]==0x33||op.u8[0]==0x0B))
	return;
      r=op.u8[1]; //not a reg but a slash val
      if (op.u8[0]==0x03 && d==-1) //add -1
	r=1; //Decrement slash val

      if (op.u8[0]==0x03 && (d==1 || d==-1)) {	//Add
	i=ICModr1(r,t1,r1,d1);
	if (!(t1&MDF_REG) && tempi->ic_flags&ICF_LOCK)
	  ICU8(tempi,OC_LOCK_PREFIX);
	switch (t1.raw_type) {
	  case RT_I8:
	  case RT_U8:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xFE);
	    break;
	  case RT_I16:
	  case RT_U16:
	    ICOpSizeRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xFF);
	    break;
	  default:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0xFF);
	}
	ICModr2(tempi,i,d1,ip+4);
	return;
      }
      if (MIN_I8<=d<=MAX_I8 || t1&(RTG_MASK-RTF_UNSIGNED)==RT_I8) {
	i=ICModr1(r,t1,r1,d1);
	if (tempi->ic_flags&ICF_LOCK)
	  ICU8(tempi,OC_LOCK_PREFIX);
	switch (t1.raw_type) {
	  case RT_I8:
	  case RT_U8:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0x80);
	    break;
	  case RT_I16:
	  case RT_U16:
	    ICOpSizeRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0x83);
	    break;
	  default:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0x83);
	}
	ICModr2(tempi,i,d1,ip+5);
	ICU8(tempi,d);
	return;
      }
      if (MIN_I32<=d<=MAX_I32 || t1.raw_type<RT_I64) {
	i=ICModr1(r,t1,r1,d1);
	if (tempi->ic_flags&ICF_LOCK)
	  ICU8(tempi,OC_LOCK_PREFIX);
	switch (t1.raw_type) {
	  case RT_I16:
	  case RT_U16:
	    ICOpSizeRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0x81);
	    break;
	  default:
	    ICRex(tempi,i.u8[1]);
	    ICU16(tempi,i.u8[2]<<8+0x81);
	}
	if (i.u8[0]!=MODR_RIP_REL)
	  ICModr2(tempi,i,d1,);
	else {
	  if (t1&(RTG_MASK-RTF_UNSIGNED)==RT_I16)
	    ICU32(tempi,d1-(ip+tempi->ic_cnt+6));
	  else
	    ICU32(tempi,d1-(ip+tempi->ic_cnt+8));
	  tempi->ic_flags&=~ICF_CODE_FINAL;
	}
	if (t1&(RTG_MASK-RTF_UNSIGNED)==RT_I16)
	  ICU16(tempi,d);
	else
	  ICU32(tempi,d);
	return;
      }
      break;
    case MDf_STK:
      ICAddSubEctImm(tempi,MDF_REG+RT_I64,REG_RAX,0,t2,r2,d2,d,op,ip);
      ICPushRegs(tempi,1<<REG_RAX);
      return;
  }
  ICAddEct(tempi,t1,r1,d1,MDF_IMM+RT_I64,0,d,t2,r2,d2,op.u8[0],ip);
}

U0 ICSub(CIntermediateCode *tempi,CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,CICType t3,I64 r3,I64 d3,I64 ip)
{
  I64 i=0x48,op=0x2B;
  Bool swap=FALSE;
  if (r3!=REG_RAX) {
    swap=TRUE;
    SwapI64(&t2,&t3);
    SwapI64(&r2,&r3);
    SwapI64(&d2,&d3);
  }
  if (t2.raw_type>=RT_I64 && r2.u8[0]!=REG_RAX &&
	(!(t2&MDF_SIB) || r2.u8[1]&15!=REG_RAX) && t2&MDG_REG_DISP_SIB_RIP) {
    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,t3,r3,d3,ip);
    if (!swap) {
      op=0x03;
      ICU24(tempi,0xD8F748);
    }
    i=ICModr1(REG_RAX,t2,r2,d2);
    if (tempi->ic_flags&ICF_LOCK)
      ICU8(tempi,OC_LOCK_PREFIX);
    ICRex(tempi,i.u8[1]);
    ICU16(tempi,i.u8[2]<<8+op);
    ICModr2(tempi,i,d2,ip+4);
    ICMov(tempi,t1,r1,d1,MDF_REG+RT_I64,REG_RAX,0,ip);
  } else {
    if (!(t3&MDF_REG) || t3.raw_type<RT_I64) {
      if (swap) {
	swap=FALSE;
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,t3,r3,d3,ip);
	ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,ip);
	r2=REG_RAX;
	r3=REG_RCX;
      } else {
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,t3,r3,d3,ip);
	ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,ip);
	r3=REG_RAX;
	r2=REG_RCX;
      }
    } else {
      ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,ip);
      r2=REG_RCX;
    }
    if (swap) {
      op=0x03;
      ICU24(tempi,0xD9F748);
    }
    if (r3>7)
      i++;
    if (r2>7)
      i+=4;
    if (tempi->ic_flags&ICF_LOCK)
      ICU8(tempi,OC_LOCK_PREFIX);
    ICU24(tempi,0xC00000+i+(r3&7)<<16+(r2&7)<<19+op<<8);
    ICMov(tempi,t1,r1,d1,MDF_REG+RT_I64,r2,0,ip);
  }
}

U0 ICMul(CIntermediateCode *tempi,I64 ip)
{
  I64 i,r2,r=REG_RAX,j;
  CICArg *a1,*a2;
  Bool alt;
  if (tempi->a1.type&MDF_IMM) {
    a1=&tempi->a2;
    a2=&tempi->a1;
    alt=TRUE;
  } else {
    a1=&tempi->a1;
    a2=&tempi->a2;
    alt=FALSE;
  }
  i=a2->disp;
  if (!(tempi->ic_class->raw_type&RTF_UNSIGNED) &&
	a2->type&MDF_IMM && MIN_I32<=i<=MAX_I32) {
    if (tempi->r.type==MDF_REG+RT_I64) {
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    a1->type,a1->reg,a1->disp,ip);
      r=tempi->r.reg;
    } else
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,a1->type,a1->reg,a1->disp,ip);
    if (r>7)
      j=0xC0004D;
    else
      j=0xC00048;
    if (MIN_I8<=i<=MAX_I8)
      ICU32(tempi,i<<24+0x6B00+j+(r&7)<<16+(r&7)<<19);
    else {
      ICU24(tempi,0x6900+j+(r&7)<<16+(r&7)<<19);
      ICU32(tempi,i);
    }
  } else {
    if (tempi->ic_class->raw_type&RTF_UNSIGNED)
      i=0xE0F748;
    else
      i=0xE8F748;
    if (alt) {
      ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,a1->type,a1->reg,a1->disp,ip);
      r2=REG_RCX;
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip);
    } else {
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip);
      if (!(a1->type&MDF_REG) || a1->type.raw_type<RT_I64) {
	ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,a1->type,a1->reg,a1->disp,ip);
	r2=REG_RCX;
      } else
	r2=a1->reg;
    }
    if (r2>7) {
      i++;
      r2&=7;
    }
    ICU24(tempi,i+r2<<16);
  }
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,MDF_REG+RT_I64,r,0,ip);
}

U0 ICMulEqu(CIntermediateCode *tempi,I64 ip)
{
  I64 i=tempi->a2.disp,r=REG_RAX,j;
  if (!(tempi->ic_class->raw_type&RTF_UNSIGNED) &&
	tempi->a2.type&MDF_IMM && MIN_I32<=i<=MAX_I32) {
    if (tempi->ic_flags & ICF_BY_VAL) {
      if (tempi->a1.type==MDF_REG+RT_I64)
	r=tempi->a1.reg;
      else
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	      tempi->a1.type&MDG_MASK+tempi->a1.type_pointed_to,
	      tempi->a1.reg,tempi->a1.disp,ip);
      if (r>7)
	j=0xC0004D;
      else
	j=0xC00048;
      if (MIN_I8<=i<=MAX_I8)
	ICU32(tempi,i<<24+0x6B00+j+(r&7)<<16+(r&7)<<19);
      else {
	ICU24(tempi,0x6900+j+(r&7)<<16+(r&7)<<19);
	ICU32(tempi,i);
      }
      ICMov(tempi,tempi->a1.type&MDG_MASK+tempi->a1.type_pointed_to,
	    tempi->a1.reg,tempi->a1.disp,MDF_REG+RT_I64,r,0,ip);
    } else {
      ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,
	    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      ICMov(tempi,MDF_REG+RT_I64,REG_RBX,0,
	    MDF_DISP+tempi->a1.type_pointed_to,REG_RCX,0,ip);
      r=REG_RBX;
      if (MIN_I8<=i<=MAX_I8)
	ICU32(tempi,i<<24+0xDB6B48);
      else {
	ICU24(tempi,0xDB6948);
	ICU32(tempi,i);
      }
      ICMov(tempi,MDF_DISP+tempi->a1.type_pointed_to,REG_RCX,0,
	    MDF_REG+RT_I64,REG_RBX,0,ip);
    }
  } else {
    if (tempi->ic_class->raw_type&RTF_UNSIGNED)
      i=0xE3F748;
    else
      i=0xEBF748;
    if (tempi->ic_flags & ICF_BY_VAL) {
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
      ICMov(tempi,MDF_REG+RT_I64,REG_RBX,0,
	    tempi->a1.type&MDG_MASK+tempi->a1.type_pointed_to,
	    tempi->a1.reg,tempi->a1.disp,ip);
      ICU24(tempi,i);
      ICMov(tempi,tempi->a1.type&MDG_MASK+tempi->a1.type_pointed_to,
	    tempi->a1.reg,tempi->a1.disp,MDF_REG+RT_I64,REG_RAX,0,ip);
    } else {
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
      ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,
	    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      ICMov(tempi,MDF_REG+RT_I64,REG_RBX,0,
	    MDF_DISP+tempi->a1.type_pointed_to,REG_RCX,0,ip);
      ICU24(tempi,i);
      ICMov(tempi,MDF_DISP+tempi->a1.type_pointed_to,REG_RCX,0,
	    MDF_REG+RT_I64,REG_RAX,0,ip);
    }
  }
  if (tempi->r.type.mode)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,MDF_REG+RT_I64,r,0,ip);
}

U0 ICDiv(CIntermediateCode *tempi,I64 ip)
{
  ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
  if (tempi->ic_class->raw_type&RTF_UNSIGNED) {
    ICZero(tempi,REG_RDX);
    ICU24(tempi,0xF1F748);
  } else {
    ICU16(tempi,0x9948);
    ICU24(tempi,0xF9F748);
  }
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICDivEqu(CIntermediateCode *tempi,Bool is_mod,I64 ip)
{
  if (tempi->ic_flags & ICF_BY_VAL) {
    ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	  tempi->a1.type&MDG_MASK+tempi->a1.type_pointed_to,
	  tempi->a1.reg,tempi->a1.disp,ip);
    if (tempi->ic_class->raw_type&RTF_UNSIGNED) {
      ICZero(tempi,REG_RDX);
      ICU24(tempi,0xF1F748);
    } else {
      ICU16(tempi,0x9948);
      ICU24(tempi,0xF9F748);
    }
    if (is_mod)
      ICMov(tempi,tempi->a1.type&MDG_MASK+tempi->a1.type_pointed_to,
	    tempi->a1.reg,tempi->a1.disp,MDF_REG+RT_I64,REG_RDX,0,ip);
    else
      ICMov(tempi,tempi->a1.type&MDG_MASK+tempi->a1.type_pointed_to,
	    tempi->a1.reg,tempi->a1.disp,MDF_REG+RT_I64,REG_RAX,0,ip);
  } else {
    ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    ICMov(tempi,MDF_REG+RT_I64,REG_RBX,0,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
//dangerous might clobber RBX in Mov, but it doesn't
    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,MDF_DISP+tempi->a1.type_pointed_to,
	  REG_RBX,0,ip);
    if (tempi->ic_class->raw_type&RTF_UNSIGNED) {
      ICZero(tempi,REG_RDX);
      ICU24(tempi,0xF1F748);
    } else {
      ICU16(tempi,0x9948);
      ICU24(tempi,0xF9F748);
    }
    if (is_mod)
      ICMov(tempi,MDF_DISP+tempi->a1.type_pointed_to,REG_RBX,0,
	    MDF_REG+RT_I64,REG_RDX,0,ip);
    else
      ICMov(tempi,MDF_DISP+tempi->a1.type_pointed_to,REG_RBX,0,
	    MDF_REG+RT_I64,REG_RAX,0,ip);
  }
  if (tempi->r.type.mode) {
    if (is_mod)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    MDF_REG+RT_I64,REG_RDX,0,ip);
    else
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	    MDF_REG+RT_I64,REG_RAX,0,ip);
  }
}

U0 ICMod(CIntermediateCode *tempi,I64 ip)
{
  ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
  if (tempi->ic_class->raw_type&RTF_UNSIGNED) {
    ICZero(tempi,REG_RDX);
    ICU24(tempi,0xF1F748);
  } else {
    ICU16(tempi,0x9948);
    ICU24(tempi,0xF9F748);
  }
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	MDF_REG+RT_I64,REG_RDX,0,ip);
}

U0 ICAddSubEctEqu(CIntermediateCode *tempi,U8 type_pointed_to,
	CICType t1,I64 r1,I64 d1,CICType t2,I64 r2,I64 d2,
	CICType t3,I64 r3,I64 d3,I64 op,I64 ip)
{
  Bool done;
  I64 res_reg,temp,i;
  if (tempi->ic_flags & ICF_BY_VAL) {
    if (t3&MDF_IMM) {
      ICAddSubEctImm(tempi,t2&MDG_MASK+type_pointed_to,r2,d2,t2&MDG_MASK+
	    type_pointed_to,r2,d2,d3,op,ip);
      if (t1.mode)
	ICMov(tempi,t1,r1,d1,t2&MDG_MASK+type_pointed_to,r2,d2,ip);
      return;
    } else {
      done=FALSE;
      if (type_pointed_to>=RT_I64) {
	if (!t1.mode && t2&MDG_REG_DISP_SIB_RIP) {
	  if (t3&MDF_REG)
	    temp=r3;
	  else {
	    temp=REG_RCX;
	    ICMov(tempi,MDF_REG+RT_I64,temp,0,t3,r3,d3,ip);
	  }
	  i=ICModr1(temp,t2&MDG_MASK+type_pointed_to,r2,d2);
	  if (tempi->ic_flags&ICF_LOCK)
	    ICU8(tempi,OC_LOCK_PREFIX);
	  ICRex(tempi,i.u8[1]);
	  ICU16(tempi,i.u8[2]<<8+op.u8[5]);
	  ICModr2(tempi,i,d2,ip+4);
	  return;
	}
	if (t3.raw_type>=RT_I64 && t3&MDG_REG_DISP_SIB_RIP) {
	  if (t2&MDF_REG)
	    res_reg=r2;
	  else {
	    res_reg=REG_RCX;
	    ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,ip);
	  }
	  i=ICModr1(res_reg,t3&MDG_MASK+type_pointed_to,r3,d3);
	  if (tempi->ic_flags&ICF_LOCK)
	    ICU8(tempi,OC_LOCK_PREFIX);
	  ICRex(tempi,i.u8[1]);
	  ICU16(tempi,i.u8[2]<<8+op.u8[0]);
	  ICModr2(tempi,i,d3,ip+4);
	  ICMov(tempi,t2&MDG_MASK+type_pointed_to,r2,d2,
		MDF_REG+RT_I64,res_reg,0,ip);
	  done=TRUE;
	}
      }
      if (!done) {
	ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,t3,r3,d3,ip);
	if (t2&MDF_REG && r2!=REG_RAX)
	  res_reg=r2;
	else {
	  res_reg=REG_RCX;
	  ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,
		t2&MDG_MASK+type_pointed_to,r2,d2,ip);
	}
	if (tempi->ic_flags&ICF_LOCK)
	  ICU8(tempi,OC_LOCK_PREFIX);
	if (res_reg>7)
	  ICU8(tempi,0x4C);
	else
	  ICU8(tempi,0x48);
	ICU16(tempi,0xC000+op.u8[0]+(res_reg&7)<<11);
	ICMov(tempi,t2&MDG_MASK+type_pointed_to,r2,d2,
	      MDF_REG+RT_I64,res_reg,0,ip);
      }
    }
  } else {
    done=FALSE;
    if (t3&MDF_IMM && op.u8[2]) {
      if (!d3.u32[1]) {
	if (tempi->ic_flags&ICF_RES_NOT_USED &&
	      t2&MDF_REG && d3(U64)<=MAX_I8) {
	  ICSlashOp(tempi,MDF_DISP+type_pointed_to,r2,0,0x838000+op.u8[4],ip);
	  ICU8(tempi,d3);
	  done=TRUE;
	} else if (op.u8[2]==0x24) {//AND
	  ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,ip);
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		MDF_DISP+type_pointed_to,REG_RCX,0,ip);
	  res_reg=REG_RAX;
	  if (tempi->ic_flags&ICF_LOCK)
	    ICU8(tempi,OC_LOCK_PREFIX);
	  ICU16(tempi,op.u8[3]<<8+0x40);
	  ICU32(tempi,d3);
	  ICMov(tempi,MDF_DISP+type_pointed_to,REG_RCX,0,
		MDF_REG+RT_I64,res_reg,0,ip);
	  done=TRUE;
	} else if (type_pointed_to<RT_I64) {//OR/XOR
	  ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,ip);
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		MDF_DISP+type_pointed_to,REG_RCX,0,ip);
	  res_reg=REG_RAX;
	  if (tempi->ic_flags&ICF_LOCK)
	    ICU8(tempi,OC_LOCK_PREFIX);
	  if (d3.u16[1]) {
	    ICU16(tempi,op.u8[3]<<8+0x40);
	    ICU32(tempi,d3);
	  } else if (d3.u8[1]) {
	    ICU24(tempi,op.u8[3]<<16+0x4000+OC_OP_SIZE_PREFIX);
	    ICU16(tempi,d3);
	  } else {
	    ICU16(tempi,op.u8[2]<<8+0x40);
	    ICU8(tempi,d3);
	  }
	  ICMov(tempi,MDF_DISP+type_pointed_to,REG_RCX,0,
		MDF_REG+RT_I64,res_reg,0,ip);
	  done=TRUE;
	}
      }
    }
    if (!done) {
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,t3,r3,d3,ip);
      ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,ip);
      ICMov(tempi,MDF_REG+RT_I64,REG_RBX,0,
	    MDF_DISP+type_pointed_to,REG_RCX,0,ip);
      res_reg=REG_RBX;
      if (tempi->ic_flags&ICF_LOCK)
	ICU8(tempi,OC_LOCK_PREFIX);
      ICU8(tempi,0x48);
      ICU16(tempi,0xC000+op.u8[0]+(res_reg&7)<<11);
      ICMov(tempi,MDF_DISP+type_pointed_to,REG_RCX,0,
	    MDF_REG+RT_I64,res_reg,0,ip);
    }
  }
  if (t1.mode)
    ICMov(tempi,t1,r1,d1,MDF_REG+RT_I64,res_reg,0,ip);
}

U0 ICShift(CIntermediateCode *tempi,CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,CICType t3,I64 r3,I64 d3,
	I64 us,I64 is,I64 ip)
{
  I64 i=0x48,res_reg;
  if (tempi->ic_class->raw_type&RTF_UNSIGNED ||
	tempi->ic_flags & ICF_USE_UNSIGNED)
    is=us;
  if (t1&MDF_REG) {
    res_reg=r1;
    if (res_reg>7)
      i++;
  } else
    res_reg=REG_RAX;
  if (t3&MDF_IMM) {
    ICMov(tempi,MDF_REG+RT_I64,res_reg,0,t2,r2,d2,ip);
    if (d3==1)
      ICU24(tempi,i+is.u16[2]<<8+(res_reg&7)<<16);
    else {
      ICU24(tempi,i+is.u16[0]<<8+(res_reg&7)<<16);
      ICU8(tempi,d3);
    }
  } else {
    ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,t3,r3,d3,ip);
    ICMov(tempi,MDF_REG+RT_I64,res_reg,0,t2,r2,d2,ip);
    ICU24(tempi,i+is.u16[1]<<8+(res_reg&7)<<16);
  }
  ICMov(tempi,t1,r1,d1,MDF_REG+RT_I64,res_reg,0,ip);
}

U0 ICShiftEqu(CIntermediateCode *tempi,U8 type_pointed_to,
	CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,
	CICType t3,I64 r3,I64 d3,I64 us,I64 is,I64 ip)
{
  I64 res_reg;
  if (tempi->ic_class->raw_type&RTF_UNSIGNED ||
	tempi->ic_flags & ICF_USE_UNSIGNED)
    is=us;
  if (tempi->ic_flags & ICF_BY_VAL) {
    if (!(t3&MDF_IMM))
      ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,t3,r3,d3,ip);
    if (t2&MDF_REG)
      res_reg=r2;
    else {
      res_reg=REG_RAX;
      ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
	    t2&MDG_MASK+type_pointed_to,r2,d2,ip);
    }
    if (res_reg>7)
      ICU8(tempi,0x49);
    else
      ICU8(tempi,0x48);
    if (t3&MDF_IMM) {
      if (d3==1)
	ICU16(tempi,is.u16[2]+(res_reg&7)<<8);
      else {
	ICU16(tempi,is.u16[0]+(res_reg&7)<<8);
	ICU8(tempi,d3);
      }
    } else
      ICU16(tempi,is.u16[1]+(res_reg&7)<<8);
    ICMov(tempi,t2&MDG_MASK+type_pointed_to,r2,d2,
	  MDF_REG+RT_I64,res_reg,0,ip);
  } else {
    if (!(t3&MDF_IMM))
      ICMov(tempi,MDF_REG+RT_I64,REG_RCX,0,t3,r3,d3,ip);
    ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,t2,r2,d2,ip);
    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,MDF_DISP+type_pointed_to,REG_RDX,0,ip);
    res_reg=REG_RAX;
    ICU8(tempi,0x48);
    if (t3&MDF_IMM) {
      if (d3==1)
	ICU16(tempi,is.u16[2]+(res_reg&7)<<8);
      else {
	ICU16(tempi,is.u16[0]+(res_reg&7)<<8);
	ICU8(tempi,d3);
      }
    } else
      ICU16(tempi,is.u16[1]+(res_reg&7)<<8);
    ICMov(tempi,
	  MDF_DISP+type_pointed_to,REG_RDX,0,MDF_REG+RT_I64,res_reg,0,ip);
  }
  if (t1.mode)
    ICMov(tempi,t1,r1,d1,MDF_REG+RT_I64,res_reg,0,ip);
}
