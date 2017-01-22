U0 ICAddEct(CIntermediateCode *tmpi,CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,CICType t3,I64 r3,I64 d3,I64 op,I64 rip)
{
  I64 i,tmp,res_reg=REG_RAX;
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
    ICMov(tmpi,MDF_REG+RT_I64,res_reg,0,t3,r3,d3,rip);
    i=ICModr1(res_reg,t2,r2,d2);
    if (tmpi->ic_flags&ICF_LOCK)
      ICU8(tmpi,OC_LOCK_PREFIX);
    ICRex(tmpi,i.u8[1]);
    ICU16(tmpi,i.u8[2]<<8+op);
    ICModr2(tmpi,i,,d2,rip);
  } else {
    if (t2&MDF_REG)
      tmp=r2;
    else
      tmp=REG_RCX;

    if (t1&MDF_REG)
      res_reg=r1;

    if (tmp==res_reg)
      res_reg=REG_RDX;
    if (swap) {
      if (r3==tmp && t3&MDG_REG_DISP_SIB)
	tmp=REG_RCX;
      ICMov(tmpi,MDF_REG+RT_I64,tmp,0,t2,r2,d2,rip);
      ICMov(tmpi,MDF_REG+RT_I64,res_reg,0,t3,r3,d3,rip);
    } else {
      if (r2==res_reg && t2&MDG_REG_DISP_SIB)
	res_reg=REG_RDX;
      ICMov(tmpi,MDF_REG+RT_I64,res_reg,0,t3,r3,d3,rip);
      ICMov(tmpi,MDF_REG+RT_I64,tmp,0,t2,r2,d2,rip);
    }
    i=0x48;
    if (res_reg>7)
      i+=4;
    if (tmp>7)
      i++;
    if (tmpi->ic_flags&ICF_LOCK)
      ICU8(tmpi,OC_LOCK_PREFIX);
    ICU24(tmpi,0xC00000+i+(tmp&7)<<16+(res_reg&7)<<19+op<<8);
  }
  ICMov(tmpi,t1,r1,d1,MDF_REG+RT_I64,res_reg,0,rip);
}

U0 ICAddSubEctImm(CIntermediateCode *tmpi,CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,I64 d,I64 op,I64 rip)
{
  I64 i;
  if (op.u8[0]==0x2B) {
    op=0x0003;
    d=-d;
  }
  if (t1&MDF_REG) {
    if (!(t2&MDF_REG)) {
      ICMov(tmpi,t1,r1,d1,t2,r2,d2,rip);
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
	ICU24(tmpi,0xC0FF00+op.u8[1]<<19+i+(r1&7)<<16);
	return;
      } else if (d==-1 && op.u8[0]==0x03) {
	ICU24(tmpi,0xC8FF00+i+(r1&7)<<16);
	return;
      } else if (I8_MIN<=d<=I8_MAX) {
	ICU24(tmpi,0xC08300+op.u8[1]<<19+i+(r1&7)<<16);
	ICU8(tmpi,d);
	return;
      } else if (I32_MIN<=d<=I32_MAX) {
	ICU24(tmpi,0xC08100+op.u8[1]<<19+i+(r1&7)<<16);
	ICU32(tmpi,d);
	return;
      }
    }
    if (op.u8[0]==0x03 && I32_MIN<=d<=I32_MAX &&
	  !Bt(&cmp.non_ptr_vars_mask,r2)) {
      i=ICModr1(r1,MDF_DISP+RT_I64,r2,d);
      i.u8[1]|=0x48;
      ICU24(tmpi,i.u8[2]<<16+0x8D00+i.u8[1]);
      ICModr2(tmpi,i,,d,rip);
      return;
    }
  }
  switch (Bsr(t1)) {
    case MDf_REG:
    case MDf_DISP:
    case MDf_SIB:
    case MDf_RIP_DISP32:
      if (t1!=t2 || r1!=r2 || d1!=d2) {
	ICMov(tmpi,t1,r1,d1,t2,r2,d2,rip);
	t2=t1;
	r2=r1;
	d2=d1;
      }

      if (!d &&(op.u8[0]==0x03||op.u8[0]==0x2B||op.u8[0]==0x33||op.u8[0]==0x0B))
	return;

      if (op.u8[0]==0x03 && d==-1) //add -1
	op.u8[1]=1; //Decrement slash val

      if (op.u8[0]==0x03 && (d==1 || d==-1)) {	//Add
	i=ICModr1(op.u8[1],t1,r1,d1);
	if (!(t1&MDF_REG) && tmpi->ic_flags&ICF_LOCK)
	  ICU8(tmpi,OC_LOCK_PREFIX);
	switch (t1.raw_type) {
	  case RT_I8:
	  case RT_U8:
	    ICRex(tmpi,i.u8[1]);
	    ICU16(tmpi,i.u8[2]<<8+0xFE);
	    break;
	  case RT_I16:
	  case RT_U16:
	    ICOpSizeRex(tmpi,i.u8[1]);
	    ICU16(tmpi,i.u8[2]<<8+0xFF);
	    break;
	  default:
	    ICRex(tmpi,i.u8[1]);
	    ICU16(tmpi,i.u8[2]<<8+0xFF);
	}
	ICModr2(tmpi,i,,d1,rip);
	return;
      }
      if (I8_MIN<=d<=I8_MAX || t1&(RTG_MASK-RTF_UNSIGNED)==RT_I8) {
	ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_IMM_U8+op.u8[1],rip+1);
	ICU8(tmpi,d);
	return;
      }
      if (I32_MIN<=d<=I32_MAX || t1.raw_type<RT_I64) {
	ICSlashOp(tmpi,t1,r1,d1,SLASH_OP_IMM_U32+op.u8[1],rip);
	if (t1&(RTG_MASK-RTF_UNSIGNED)==RT_I16)
	  ICU16(tmpi,d);
	else
	  ICU32(tmpi,d);
	return;
      }
      break;
    case MDf_STK:
      ICAddSubEctImm(tmpi,MDF_REG+RT_I64,REG_RAX,0,t2,r2,d2,d,op,rip);
      ICPushRegs(tmpi,1<<REG_RAX);
      return;
  }
  ICAddEct(tmpi,t1,r1,d1,MDF_IMM+RT_I64,0,d,t2,r2,d2,op.u8[0],rip);
}

U0 ICSub(CIntermediateCode *tmpi,CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,CICType t3,I64 r3,I64 d3,I64 rip)
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
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,t3,r3,d3,rip);
    if (!swap) {
      op=0x03;
      ICU24(tmpi,0xD8F748);
    }
    i=ICModr1(REG_RAX,t2,r2,d2);
    if (tmpi->ic_flags&ICF_LOCK)
      ICU8(tmpi,OC_LOCK_PREFIX);
    ICRex(tmpi,i.u8[1]);
    ICU16(tmpi,i.u8[2]<<8+op);
    ICModr2(tmpi,i,,d2,rip);
    ICMov(tmpi,t1,r1,d1,MDF_REG+RT_I64,REG_RAX,0,rip);
  } else {
    if (!(t3&MDF_REG) || t3.raw_type<RT_I64) {
      if (swap) {
	swap=FALSE;
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,t3,r3,d3,rip);
	ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,rip);
	r2=REG_RAX;
	r3=REG_RCX;
      } else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,t3,r3,d3,rip);
	ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,rip);
	r3=REG_RAX;
	r2=REG_RCX;
      }
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,rip);
      r2=REG_RCX;
    }
    if (swap) {
      op=0x03;
      ICU24(tmpi,0xD9F748);
    }
    if (r3>7)
      i++;
    if (r2>7)
      i+=4;
    if (tmpi->ic_flags&ICF_LOCK)
      ICU8(tmpi,OC_LOCK_PREFIX);
    ICU24(tmpi,0xC00000+i+(r3&7)<<16+(r2&7)<<19+op<<8);
    ICMov(tmpi,t1,r1,d1,MDF_REG+RT_I64,r2,0,rip);
  }
}

U0 ICMul(CIntermediateCode *tmpi,I64 rip)
{
  I64 i,r2,r=REG_RAX,j;
  CICArg *arg1,*arg2;
  Bool alt;
  if (tmpi->arg1.type&MDF_IMM) {
    arg1=&tmpi->arg2;
    arg2=&tmpi->arg1;
    alt=TRUE;
  } else {
    arg1=&tmpi->arg1;
    arg2=&tmpi->arg2;
    alt=FALSE;
  }
  i=arg2->disp;
  if (!(tmpi->ic_class->raw_type&RTF_UNSIGNED) &&
	arg2->type&MDF_IMM && I32_MIN<=i<=I32_MAX) {
    if (tmpi->res.type==MDF_REG+RT_I64) {
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    arg1->type,arg1->reg,arg1->disp,rip);
      r=tmpi->res.reg;
    } else
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg1->type,arg1->reg,arg1->disp,rip);
    if (r>7)
      j=0xC0004D;
    else
      j=0xC00048;
    if (I8_MIN<=i<=I8_MAX)
      ICU32(tmpi,i<<24+0x6B00+j+(r&7)<<16+(r&7)<<19);
    else {
      ICU24(tmpi,0x6900+j+(r&7)<<16+(r&7)<<19);
      ICU32(tmpi,i);
    }
  } else {
    if (tmpi->ic_class->raw_type&RTF_UNSIGNED)
      i=0xE0F748;
    else
      i=0xE8F748;
    if (alt) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,arg1->type,arg1->reg,arg1->disp,rip);
      r2=REG_RCX;
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg2->type,arg2->reg,arg2->disp,rip);
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg2->type,arg2->reg,arg2->disp,rip);
      if (!(arg1->type&MDF_REG) || arg1->type.raw_type<RT_I64) {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	      arg1->type,arg1->reg,arg1->disp,rip);
	r2=REG_RCX;
      } else
	r2=arg1->reg;
    }
    if (r2>7) {
      i++;
      r2&=7;
    }
    ICU24(tmpi,i+r2<<16);
  }
  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	MDF_REG+RT_I64,r,0,rip);
}

U0 ICMulEqu(CIntermediateCode *tmpi,I64 rip)
{
  I64 i=tmpi->arg2.disp,r=REG_RAX,j;
  if (!(tmpi->ic_class->raw_type&RTF_UNSIGNED) &&
	tmpi->arg2.type&MDF_IMM && I32_MIN<=i<=I32_MAX) {
    if (tmpi->ic_flags & ICF_BY_VAL) {
      if (tmpi->arg1.type==MDF_REG+RT_I64)
	r=tmpi->arg1.reg;
      else
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	      tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	      tmpi->arg1.reg,tmpi->arg1.disp,rip);
      if (r>7)
	j=0xC0004D;
      else
	j=0xC00048;
      if (I8_MIN<=i<=I8_MAX)
	ICU32(tmpi,i<<24+0x6B00+j+(r&7)<<16+(r&7)<<19);
      else {
	ICU24(tmpi,0x6900+j+(r&7)<<16+(r&7)<<19);
	ICU32(tmpi,i);
      }
      ICMov(tmpi,tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,MDF_REG+RT_I64,r,0,rip);
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,
	    MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,rip);
      r=REG_RBX;
      if (I8_MIN<=i<=I8_MAX)
	ICU32(tmpi,i<<24+0xDB6B48);
      else {
	ICU24(tmpi,0xDB6948);
	ICU32(tmpi,i);
      }
      ICMov(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,
	    MDF_REG+RT_I64,REG_RBX,0,rip);
    }
  } else {
    if (tmpi->ic_class->raw_type&RTF_UNSIGNED)
      i=0xE3F748;
    else
      i=0xEBF748;
    if (tmpi->ic_flags & ICF_BY_VAL) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,rip);
      ICU24(tmpi,i);
      ICMov(tmpi,tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,MDF_REG+RT_I64,REG_RAX,0,rip);
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,
	    MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,rip);
      ICU24(tmpi,i);
      ICMov(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,
	    MDF_REG+RT_I64,REG_RAX,0,rip);
    }
  }
  if (tmpi->res.type.mode)
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_REG+RT_I64,r,0,rip);
}

U0 ICDiv(CIntermediateCode *tmpi,I64 rip)
{
  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
  if (tmpi->ic_class->raw_type&RTF_UNSIGNED) {
    ICZero(tmpi,REG_RDX);
    ICU24(tmpi,0xF1F748);
  } else {
    ICU16(tmpi,0x9948);
    ICU24(tmpi,0xF9F748);
  }
  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	MDF_REG+RT_I64,REG_RAX,0,rip);
}

U0 ICDivEqu(CIntermediateCode *tmpi,Bool is_mod,I64 rip)
{
  if (tmpi->ic_flags & ICF_BY_VAL) {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	  tmpi->arg1.reg,tmpi->arg1.disp,rip);
    if (tmpi->ic_class->raw_type&RTF_UNSIGNED) {
      ICZero(tmpi,REG_RDX);
      ICU24(tmpi,0xF1F748);
    } else {
      ICU16(tmpi,0x9948);
      ICU24(tmpi,0xF9F748);
    }
    if (is_mod)
      ICMov(tmpi,tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,MDF_REG+RT_I64,REG_RDX,0,rip);
    else
      ICMov(tmpi,tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,MDF_REG+RT_I64,REG_RAX,0,rip);
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
    ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
//dangerous might clobber RBX in Mov, but it doesn't
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,MDF_DISP+tmpi->arg1_type_pointed_to,
	  REG_RBX,0,rip);
    if (tmpi->ic_class->raw_type&RTF_UNSIGNED) {
      ICZero(tmpi,REG_RDX);
      ICU24(tmpi,0xF1F748);
    } else {
      ICU16(tmpi,0x9948);
      ICU24(tmpi,0xF9F748);
    }
    if (is_mod)
      ICMov(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,REG_RBX,0,
	    MDF_REG+RT_I64,REG_RDX,0,rip);
    else
      ICMov(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,REG_RBX,0,
	    MDF_REG+RT_I64,REG_RAX,0,rip);
  }
  if (tmpi->res.type.mode) {
    if (is_mod)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    MDF_REG+RT_I64,REG_RDX,0,rip);
    else
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    MDF_REG+RT_I64,REG_RAX,0,rip);
  }
}

U0 ICMod(CIntermediateCode *tmpi,I64 rip)
{
  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
  if (tmpi->ic_class->raw_type&RTF_UNSIGNED) {
    ICZero(tmpi,REG_RDX);
    ICU24(tmpi,0xF1F748);
  } else {
    ICU16(tmpi,0x9948);
    ICU24(tmpi,0xF9F748);
  }
  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	MDF_REG+RT_I64,REG_RDX,0,rip);
}

U0 ICAddSubEctEqu(CIntermediateCode *tmpi,U8 type_pointed_to,
	CICType t1,I64 r1,I64 d1,CICType t2,I64 r2,I64 d2,
	CICType t3,I64 r3,I64 d3,I64 op,I64 rip)
{
  Bool done;
  I64 res_reg,tmp,i;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (t3&MDF_IMM) {
      ICAddSubEctImm(tmpi,t2&MDG_MASK+type_pointed_to,r2,d2,t2&MDG_MASK+
	    type_pointed_to,r2,d2,d3,op,rip);
      if (t1.mode)
	ICMov(tmpi,t1,r1,d1,t2&MDG_MASK+type_pointed_to,r2,d2,rip);
      return;
    } else {
      done=FALSE;
      if (type_pointed_to>=RT_I64) {
	if (!t1.mode && t2&MDG_REG_DISP_SIB_RIP) {
	  if (t3&MDF_REG)
	    tmp=r3;
	  else {
	    tmp=REG_RCX;
	    ICMov(tmpi,MDF_REG+RT_I64,tmp,0,t3,r3,d3,rip);
	  }
	  i=ICModr1(tmp,t2&MDG_MASK+type_pointed_to,r2,d2);
	  if (tmpi->ic_flags&ICF_LOCK)
	    ICU8(tmpi,OC_LOCK_PREFIX);
	  ICRex(tmpi,i.u8[1]);
	  ICU16(tmpi,i.u8[2]<<8+op.u8[5]);
	  ICModr2(tmpi,i,,d2,rip);
	  return;
	}
	if (t3.raw_type>=RT_I64 && t3&MDG_REG_DISP_SIB_RIP) {
	  if (t2&MDF_REG)
	    res_reg=r2;
	  else {
	    res_reg=REG_RCX;
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,rip);
	  }
	  i=ICModr1(res_reg,t3&MDG_MASK+type_pointed_to,r3,d3);
	  if (tmpi->ic_flags&ICF_LOCK)
	    ICU8(tmpi,OC_LOCK_PREFIX);
	  ICRex(tmpi,i.u8[1]);
	  ICU16(tmpi,i.u8[2]<<8+op.u8[0]);
	  ICModr2(tmpi,i,,d3,rip);
	  ICMov(tmpi,t2&MDG_MASK+type_pointed_to,r2,d2,
		MDF_REG+RT_I64,res_reg,0,rip);
	  done=TRUE;
	}
      }
      if (!done) {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,t3,r3,d3,rip);
	if (t2&MDF_REG && r2!=REG_RAX)
	  res_reg=r2;
	else {
	  res_reg=REG_RCX;
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
		t2&MDG_MASK+type_pointed_to,r2,d2,rip);
	}
	if (tmpi->ic_flags&ICF_LOCK)
	  ICU8(tmpi,OC_LOCK_PREFIX);
	if (res_reg>7)
	  ICU8(tmpi,0x4C);
	else
	  ICU8(tmpi,0x48);
	ICU16(tmpi,0xC000+op.u8[0]+(res_reg&7)<<11);
	ICMov(tmpi,t2&MDG_MASK+type_pointed_to,r2,d2,
	      MDF_REG+RT_I64,res_reg,0,rip);
      }
    }
  } else {
    done=FALSE;
    if (t3&MDF_IMM && op.u8[2]) {
      if (!d3.u32[1]) {
	if (tmpi->ic_flags&ICF_RES_NOT_USED &&
	      t2&MDF_REG && d3(U64)<=I8_MAX) {
	  ICSlashOp(tmpi,MDF_DISP+type_pointed_to,r2,0,0x838000+op.u8[4],rip);
	  ICU8(tmpi,d3);
	  done=TRUE;
	} else if (op.u8[2]==0x24) {//AND
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,rip);
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		MDF_DISP+type_pointed_to,REG_RCX,0,rip);
	  res_reg=REG_RAX;
	  if (tmpi->ic_flags&ICF_LOCK)
	    ICU8(tmpi,OC_LOCK_PREFIX);
	  ICU16(tmpi,op.u8[3]<<8+0x40);
	  ICU32(tmpi,d3);
	  ICMov(tmpi,MDF_DISP+type_pointed_to,REG_RCX,0,
		MDF_REG+RT_I64,res_reg,0,rip);
	  done=TRUE;
	} else if (type_pointed_to<RT_I64) {//OR/XOR
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,rip);
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		MDF_DISP+type_pointed_to,REG_RCX,0,rip);
	  res_reg=REG_RAX;
	  if (tmpi->ic_flags&ICF_LOCK)
	    ICU8(tmpi,OC_LOCK_PREFIX);
	  if (d3.u16[1]) {
	    ICU16(tmpi,op.u8[3]<<8+0x40);
	    ICU32(tmpi,d3);
	  } else if (d3.u8[1]) {
	    ICU24(tmpi,op.u8[3]<<16+0x4000+OC_OP_SIZE_PREFIX);
	    ICU16(tmpi,d3);
	  } else {
	    ICU16(tmpi,op.u8[2]<<8+0x40);
	    ICU8(tmpi,d3);
	  }
	  ICMov(tmpi,MDF_DISP+type_pointed_to,REG_RCX,0,
		MDF_REG+RT_I64,res_reg,0,rip);
	  done=TRUE;
	}
      }
    }
    if (!done) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,t3,r3,d3,rip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,rip);
      ICMov(tmpi,MDF_REG+RT_I64,REG_RBX,0,
	    MDF_DISP+type_pointed_to,REG_RCX,0,rip);
      res_reg=REG_RBX;
      if (tmpi->ic_flags&ICF_LOCK)
	ICU8(tmpi,OC_LOCK_PREFIX);
      ICU8(tmpi,0x48);
      ICU16(tmpi,0xC000+op.u8[0]+(res_reg&7)<<11);
      ICMov(tmpi,MDF_DISP+type_pointed_to,REG_RCX,0,
	    MDF_REG+RT_I64,res_reg,0,rip);
    }
  }
  if (t1.mode)
    ICMov(tmpi,t1,r1,d1,MDF_REG+RT_I64,res_reg,0,rip);
}

U0 ICShift(CIntermediateCode *tmpi,CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,CICType t3,I64 r3,I64 d3,
	I64 us,I64 is,I64 rip)
{
  I64 i=0x48,res_reg;
  if (tmpi->ic_class->raw_type&RTF_UNSIGNED ||
	tmpi->ic_flags & ICF_USE_UNSIGNED)
    is=us;
  if (t1&MDF_REG) {
    res_reg=r1;
    if (res_reg>7)
      i++;
  } else
    res_reg=REG_RAX;
  if (t3&MDF_IMM) {
    ICMov(tmpi,MDF_REG+RT_I64,res_reg,0,t2,r2,d2,rip);
    if (d3==1)
      ICU24(tmpi,i+is.u16[2]<<8+(res_reg&7)<<16);
    else {
      ICU24(tmpi,i+is.u16[0]<<8+(res_reg&7)<<16);
      ICU8(tmpi,d3);
    }
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t3,r3,d3,rip);
    ICMov(tmpi,MDF_REG+RT_I64,res_reg,0,t2,r2,d2,rip);
    ICU24(tmpi,i+is.u16[1]<<8+(res_reg&7)<<16);
  }
  ICMov(tmpi,t1,r1,d1,MDF_REG+RT_I64,res_reg,0,rip);
}

U0 ICShiftEqu(CIntermediateCode *tmpi,U8 type_pointed_to,
	CICType t1,I64 r1,I64 d1,
	CICType t2,I64 r2,I64 d2,
	CICType t3,I64 r3,I64 d3,I64 us,I64 is,I64 rip)
{
  I64 res_reg;
  if (tmpi->ic_class->raw_type&RTF_UNSIGNED ||
	tmpi->ic_flags & ICF_USE_UNSIGNED)
    is=us;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (!(t3&MDF_IMM))
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t3,r3,d3,rip);
    if (t2&MDF_REG)
      res_reg=r2;
    else {
      res_reg=REG_RAX;
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    t2&MDG_MASK+type_pointed_to,r2,d2,rip);
    }
    if (res_reg>7)
      ICU8(tmpi,0x49);
    else
      ICU8(tmpi,0x48);
    if (t3&MDF_IMM) {
      if (d3==1)
	ICU16(tmpi,is.u16[2]+(res_reg&7)<<8);
      else {
	ICU16(tmpi,is.u16[0]+(res_reg&7)<<8);
	ICU8(tmpi,d3);
      }
    } else
      ICU16(tmpi,is.u16[1]+(res_reg&7)<<8);
    ICMov(tmpi,t2&MDG_MASK+type_pointed_to,r2,d2,
	  MDF_REG+RT_I64,res_reg,0,rip);
  } else {
    if (!(t3&MDF_IMM))
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t3,r3,d3,rip);
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,t2,r2,d2,rip);
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,MDF_DISP+type_pointed_to,REG_RDX,0,rip);
    res_reg=REG_RAX;
    ICU8(tmpi,0x48);
    if (t3&MDF_IMM) {
      if (d3==1)
	ICU16(tmpi,is.u16[2]+(res_reg&7)<<8);
      else {
	ICU16(tmpi,is.u16[0]+(res_reg&7)<<8);
	ICU8(tmpi,d3);
      }
    } else
      ICU16(tmpi,is.u16[1]+(res_reg&7)<<8);
    ICMov(tmpi,
	  MDF_DISP+type_pointed_to,REG_RDX,0,MDF_REG+RT_I64,res_reg,0,rip);
  }
  if (t1.mode)
    ICMov(tmpi,t1,r1,d1,MDF_REG+RT_I64,res_reg,0,rip);
}
