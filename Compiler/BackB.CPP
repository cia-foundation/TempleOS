U0 ICUnaries(CIntermediateCode *tempi,I64 op,I64 ip)
{
  ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
  ICSlashOp(tempi,TY_REG+RT_I64,REG_RAX,0,op,ip);
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
    TY_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICNot(CIntermediateCode *tempi,I64 ip)
{
  I64 i;
  if (tempi->a1.type&RT_MASK<=RT_U8 &&
    TY_DISP<=tempi->a1.type&TY_MASK<=TY_RIP_DISP32) {
    i=ICModr1(tempi,tempi->a1.type,tempi->a1.reg,tempi->a1.disp);
    ICRex(tempi,i.u8[1]);
    ICU16(tempi,i.u8[2]<<8+0xF6); //TEST ?,0xFF
    ICModr2(tempi,i,tempi->a1.disp,ip+5);
    ICU8(tempi,0xFF);
  } else {
    ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
    ICTest(tempi,REG_RAX);
  }
  ICU24(tempi,0xC0940F);   //SETZ AL
  ICU32(tempi,0xC0B60F48); //MOVZX RAX,AL
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
    TY_REG+RT_U64,REG_RAX,0,ip);
}

U0 ICAndAnd(CIntermediateCode *tempi,I64 ip)
{
  I64 r2;
  if (tempi->ic_flags & ICF_A1_FIRST) {
    if (tempi->a1.type&TY_MASK!=TY_REG ||
	tempi->a1.reg==REG_RAX) {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      r2=REG_RDX;
    } else
      r2=tempi->a1.reg;
    ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
  } else {
    ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    if (tempi->a1.type&TY_MASK!=TY_REG ||
	tempi->a1.reg==REG_RAX) {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      r2=REG_RDX;
    } else
      r2=tempi->a1.reg;
  }
  ICZero(tempi,REG_RAX);
  ICTest(tempi,r2);
  ICU16(tempi,0x0874);
  ICTest(tempi,REG_RCX);
  ICU16(tempi,0x0374);
  ICU24(tempi,0xC0FF48);
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICOrOr(CIntermediateCode *tempi,I64 ip)
{
  I64 i=0x48,r2;
  if (tempi->ic_flags & ICF_A1_FIRST) {
    if (tempi->a1.type&TY_MASK!=TY_REG ||
	tempi->a1.reg==REG_RAX) {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      r2=REG_RDX;
    } else
      r2=tempi->a1.reg;
    ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
  } else {
    ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    if (tempi->a1.type&TY_MASK!=TY_REG ||
	tempi->a1.reg==REG_RAX) {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      r2=REG_RDX;
    } else
      r2=tempi->a1.reg;
  }
 
  if (r2>7) {
    i++;
    r2&=7;
  }
  ICZero(tempi,REG_RAX);
  ICU24(tempi,0xC80B00+i+r2<<16);
  ICU16(tempi,0x0374);
  ICU24(tempi,0xC0FF48);
  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICXorXor(CIntermediateCode *tempi,I64 ip)
{
  I64 r2;
  if (tempi->ic_flags & ICF_A1_FIRST) {
    if (tempi->a1.type&TY_MASK!=TY_REG) {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      r2=REG_RDX;
    } else
      r2=tempi->a1.reg;
    ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
  } else {
    ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    if (tempi->a1.type&TY_MASK!=TY_REG) {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      r2=REG_RDX;
    } else
      r2=tempi->a1.reg;
  }
  ICZero(tempi,REG_RBX);
  ICTest(tempi,r2);
  ICU16(tempi,0x0374);
  ICU24(tempi,0xC3FF48);

  ICZero(tempi,REG_RAX);
  ICTest(tempi,REG_RCX);
  ICU16(tempi,0x0374);
  ICU24(tempi,0xC0FF48);
 
  ICU24(tempi,0xC33348);

  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICCmp(CIntermediateCode *tempi,I64 us,I64 is,I64 ip)
{
  I64 r1,d1,r2,i=0x48,j=tempi->a2.disp;
  if (tempi->a2.type&TY_MASK==TY_IMM && MIN_I32<=j<=MAX_I32) {
    if (!(tempi->ic_flags&(ICF_POP_CMP|ICF_PUSH_CMP)) &&
	tempi->a1.type&TY_MASK==TY_DISP && //TODO
	tempi->a1.type&RT_MASK>=RT_I64 && tempi->a1.reg!=REG_RAX) {
      r1=tempi->a1.reg;
      d1=tempi->a1.disp;
      ICZero(tempi,REG_RAX);
      if (r1>7)
	i++;
      if (MIN_I8<=j<=MAX_I8)
	i+=0x388300;
      else
	i+=0x388100;
      if (!d1) {
	ICU24(tempi,0x000000+i+(r1&7)<<16);
      } else if (MIN_I8<=d1<=MAX_I8) {
	ICU24(tempi,0x400000+i+(r1&7)<<16);
	ICU8(tempi,d1);
      } else {
	ICU24(tempi,0x800000+i+(r1&7)<<16);
	ICU32(tempi,d1);
      }
      if (MIN_I8<=j<=MAX_I8)
	ICU8(tempi,j);
      else
	ICU32(tempi,j);
    } else {
      if (tempi->ic_flags & ICF_POP_CMP) {
	ICPopRegs(tempi,1<<REG_RCX);
	r1=REG_RCX;
      } else {
	if (tempi->a1.type&TY_MASK==TY_REG &&
	    tempi->a1.reg!=REG_RAX)
	  r1=tempi->a1.reg;
	else {
	  ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
	  r1=REG_RCX;
	}
      }
      ICZero(tempi,REG_RAX);
      if (r1>7)
	i++;
      if (MIN_I8<=j<=MAX_I8) {
	ICU24(tempi,0xF88300+i+(r1&7)<<16);
	ICU8(tempi,j);
      } else {
	ICU24(tempi,0xF88100+i+(r1&7)<<16);
	ICU32(tempi,j);
      }
    }
    if (tempi->ic_flags & ICF_PUSH_CMP)
      ICPush(tempi,TY_IMM+RT_I64,0,j,ip);
    if (tempi->ic_class->raw_type&1 || tempi->ic_flags & ICF_USE_UNSIGNED)
      is=us;
    ICU16(tempi,0x300+is);
    ICU24(tempi,0xC0FF48);
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
      TY_REG+RT_I64,REG_RAX,0,ip);
  } else {
    if (tempi->ic_flags & ICF_A1_FIRST) {
      if (tempi->ic_flags & ICF_POP_CMP) {
	ICPopRegs(tempi,1<<REG_RCX);
	r1=REG_RCX;
      } else {
	if (tempi->a1.type&TY_MASK==TY_REG &&
	    tempi->a1.reg!=REG_RAX)
	  r1=tempi->a1.reg;
	else {
	  ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
	  r1=REG_RCX;
	}
      }
      if (tempi->a2.type&TY_MASK==TY_REG &&
	  tempi->a2.reg!=REG_RAX)
	r2=tempi->a2.reg;
      else {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
	r2=REG_RDX;
      }
    } else {
      if (tempi->a2.type&TY_MASK==TY_REG &&
	  tempi->a2.reg!=REG_RAX)
	r2=tempi->a2.reg;
      else {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
	r2=REG_RCX;
      }
      if (tempi->ic_flags & ICF_POP_CMP) {
	ICPopRegs(tempi,1<<REG_RDX);
	r1=REG_RDX;
      } else {
	if (tempi->a1.type&TY_MASK==TY_REG &&
	    tempi->a1.reg!=REG_RAX)
	  r1=tempi->a1.reg;
	else {
	  ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
	  r1=REG_RDX;
	}
      }
    }
    ICZero(tempi,REG_RAX);
    if (r2>7)
      i++;
    if (r1>7)
      i+=4;
    if (tempi->ic_flags & ICF_PUSH_CMP)
      ICPushRegs(tempi,1<<r2);
    ICU24(tempi,0xC03B00+i+(r2&7)<<16+(r1&7)<<19);
    if (tempi->ic_class->raw_type&1 || tempi->ic_flags & ICF_USE_UNSIGNED)
      is=us;
    ICU16(tempi,0x300+is);
    ICU24(tempi,0xC0FF48);
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
      TY_REG+RT_I64,REG_RAX,0,ip);
  }
}

U0 ICBitOps(CIntermediateCode *tempi,CICArg *a1,CICArg *a2,CIntermediateCode *tempi2,
	      I64 op,I64 op_imm,I64 ip)
{//TODO:not fully utilizing Modr
  Bool no_depend=ToBool(tempi2->ic_flags&ICF_NO_DEPEND_RESULT);
  I64 r1,t2,r2,d2,i=0x48;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    t2=a2->type&TY_MASK+RT_I64; //TODO: check overflow
    r2=a2->reg;
    d2=a2->disp;
    if (tempi->ic_flags & ICF_A1_FIRST) {
      if (a1->type&TY_MASK==TY_REG &&
	  (a1->reg!=REG_RAX||no_depend))
	r1=a1->reg;
      else if (a1->type&TY_MASK!=TY_IMM || a1->disp>63) {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,a1->type,a1->reg,a1->disp,ip);
	r1=REG_RCX;
      } else
	r1=0;
      if (!(TY_REG<=t2&TY_MASK<=TY_RIP_DISP32) ||
	  !(r2.u8[0]!=REG_RAX&&(t2&TY_MASK!=TY_SIB||r2.u8[1]&15!=REG_RAX)||no_depend)) {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,t2,r2,d2,ip);
	t2=TY_REG+RT_I64;
	r2=REG_RDX;
	d2=0;
      }
    } else {
      if (!(TY_REG<=t2&TY_MASK<=TY_RIP_DISP32) ||
	  !(r2.u8[0]!=REG_RAX&&(t2&TY_MASK!=TY_SIB||r2.u8[1]&15!=REG_RAX)||no_depend)) {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,t2,r2,d2,ip);
	t2=TY_REG+RT_I64;
	r2=REG_RCX;
	d2=0;
      }
      if (a1->type&TY_MASK==TY_REG &&
	  (a1->reg!=REG_RAX||no_depend))
	r1=a1->reg;
      else if (a1->type&TY_MASK!=TY_IMM || a1->disp>63) {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip);
	r1=REG_RDX;
      } else
	r1=0;
    }
  } else {
    t2=TY_DISP+RT_I64;
    d2=0;
    if (tempi->ic_flags & ICF_A1_FIRST) {
      if (a1->type&TY_MASK==TY_REG &&
	  (a1->reg!=REG_RAX||no_depend))
	r1=a1->reg;
      else if (a1->type&TY_MASK!=TY_IMM || a1->disp>63) {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,a1->type,a1->reg,a1->disp,ip);
	r1=REG_RCX;
      } else
	r1=0;
      if (a2->type&TY_MASK==TY_REG &&
	  (a2->reg!=REG_RAX||no_depend))
	r2=a2->reg;
      else {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,a2->type,a2->reg,a2->disp,ip);
	r2=REG_RDX;
      }
    } else {
      if (a2->type&TY_MASK==TY_REG &&
	  (a2->reg!=REG_RAX||no_depend))
	r2=a2->reg;
      else {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,a2->type,a2->reg,a2->disp,ip);
	r2=REG_RCX;
      }
      if (a1->type&TY_MASK==TY_REG &&
	  (a1->reg!=REG_RAX||no_depend))
	r1=a1->reg;
      else if (a1->type&TY_MASK!=TY_IMM || a1->disp>63) {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip);
	r1=REG_RDX;
      } else
	r1=0;
    }
  }
  if (!no_depend)
    ICZero(tempi,REG_RAX);
  if (tempi->ic_flags&ICF_LOCK) {
    if (op!=0xA30F)
      ICU8(tempi,OC_LOCK_PREFIX);
  }
  if (a1->type&TY_MASK==TY_IMM && a1->disp<32)
    t2=t2&TY_MASK+RT_U32;
  i=ICModr1(r1,t2,r2,d2);
  ICRex(tempi,i.u8[1]);
  if (a1->type&TY_MASK==TY_IMM && a1->disp<64) {
    ICU24(tempi,i.u8[2]<<16+op_imm);
    ICModr2(tempi,i,d2,ip+5);
    ICU8(tempi,a1->disp);
  } else {
    ICU24(tempi,i.u8[2]<<16+op);
    ICModr2(tempi,i,d2,ip+4);
  }
  if (!no_depend) {
    ICU32(tempi,0xD08348);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+RT_I64,REG_RAX,0,ip);
  }
}

U0 ICPreempt(CIntermediateCode *tempi,I64 ip)
{
  CIntermediateCode *tempi2=tempi->next;
  if (tempi->a1.type&TY_MASK==TY_IMM) {
    ICZero(tempi,REG_RBX);
    ICU32(tempi,0x1B8B4864);
    if (!(tempi2->ic_flags&ICF_NO_DEPEND_RESULT))
      ICZero(tempi,REG_RAX);
    if (tempi->a1.disp)
      ICU24(tempi,0x6BBA0F);
    else
      ICU24(tempi,0x73BA0F);
    ICU16(tempi,CTask.task_flags+TASKf_PREEMPT<<8);
  } else {
    ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
    if (!(tempi2->ic_flags&ICF_NO_DEPEND_RESULT))
      ICZero(tempi,REG_RAX);
    ICZero(tempi,REG_RBX);
    ICU32(tempi,0x1B8B4864);
    ICTest(tempi,REG_RCX);
    ICU16(tempi,0x0774);
    ICU24(tempi,0x6BBA0F);
    ICU16(tempi,CTask.task_flags+TASKf_PREEMPT<<8);
    ICU16(tempi,0x05EB);
    ICU24(tempi,0x73BA0F);
    ICU16(tempi,CTask.task_flags+TASKf_PREEMPT<<8);
  }
  if (!(tempi2->ic_flags&ICF_NO_DEPEND_RESULT))
    ICU32(tempi,0xD08348);
}

U0 ICToUpper(CIntermediateCode *tempi,I64 ip)
{
  ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
    tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
  ICU32(tempi,0x61F88348);
  ICU16(tempi,0x0A7C);
  ICU32(tempi,0x7AF88348);
  ICU16(tempi,0x047F);
  ICU32(tempi,0xE0C08348);
}

U0 ICToI64(CLex *lx,CIntermediateCode *tempi,I64 ip)
{
  ICFCvt(lx,tempi,REG_RAX,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,TRUE,CN_MAIN,ip);
}

U0 ICToF64(CLex *lx,CIntermediateCode *tempi,I64 ip)
{
  ICFCvt(lx,tempi,REG_RAX,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,FALSE,CN_MAIN,ip);
}

U0 ICToBool(CLex *,CIntermediateCode *tempi,I64 ip)
{
  I64 r;
  if (tempi->a1.type&TY_MASK==TY_REG)
    r=tempi->a1.reg;
  else {
    ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,
      tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
    r=REG_RAX;
  }
  ICTest(tempi,r);
  ICU24(tempi,0xC0950F);   //SETNZ AL
  ICU32(tempi,0xC0B60F48); //MOVZX RAX,AL
}

U0 ICPreIncDec(CIntermediateCode *tempi,I64 op,I64 ip)
{
  I64 r;
  CHashClass *tempc=tempi->ic_class,*tempc1=tempc-1;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempc->ptr_stars_cnt && tempc1->size!=1) {
      ICAddSubEctImm(tempi,tempi->a1.type&TY_MASK+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,
	tempi->a1.type&TY_MASK+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,
	tempi->a1.type&TY_MASK+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,op,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->a1.type&TY_MASK+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,ip);
  } else {
    if (tempi->a1.type&TY_MASK==TY_REG)
      r=tempi->a1.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      r=REG_RCX;
    }
    if (tempc->ptr_stars_cnt &&
	tempc1->size!=1) {
      ICAddSubEctImm(tempi,TY_DISP+tempi->a1.type_pointed,r,0,
	TY_DISP+tempi->a1.type_pointed,r,0,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,
	TY_DISP+tempi->a1.type_pointed,r,0,op,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	TY_DISP+tempi->a1.type_pointed,r,0,ip);
  }
}

U0 ICPostIncDec(CIntermediateCode *tempi,I64 op,I64 ip)
{
  I64 r;
  CHashClass *tempc=tempi->ic_class,*tempc1=tempc-1;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempi->r.type&TY_MASK) {
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->a1.type&TY_MASK+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,ip);
    }
    if (tempc->ptr_stars_cnt &&
	tempc1->size!=1) {
      ICAddSubEctImm(tempi,tempi->a1.type&TY_MASK+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,
	tempi->a1.type&TY_MASK+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,
	tempi->a1.type&TY_MASK+tempi->a1.type_pointed,tempi->a1.reg,tempi->a1.disp,op,ip);
  } else {
    if (tempi->a1.type&TY_MASK==TY_REG &&
	!(tempi->r.type&TY_MASK==TY_REG && tempi->r.reg==tempi->a1.reg))
      r=tempi->a1.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
      r=REG_RCX;
    }
    if (tempi->r.type&TY_MASK) {
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	TY_DISP+tempi->a1.type_pointed,r,0,ip);
    }
    if (tempc->ptr_stars_cnt &&
	tempc1->size!=1) {
      ICAddSubEctImm(tempi,TY_DISP+tempi->a1.type_pointed,r,0,
	TY_DISP+tempi->a1.type_pointed,r,0,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,
	TY_DISP+tempi->a1.type_pointed,r,0,op,ip);
  }
}

U0 ICDerefPostIncDec(CIntermediateCode *tempi,I64 op,I64 ip)
{
  I64 t,r;
  CHashClass *tempc1=tempi->ic_class;
  t=tempi->r.type&RT_MASK;
  if (t>tempi->a1.type_pointed)
    t=tempi->a1.type_pointed;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempi->a1.type&TY_MASK==TY_REG)
      r=tempi->a1.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,tempi->a1.type&TY_MASK+RT_I64,tempi->a1.reg,tempi->a1.disp,ip);
      r=REG_RDX;
    }
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
       TY_DISP+t,r,0,ip);
    if (tempc1->size!=1) {
      ICAddSubEctImm(tempi,tempi->a1.type&TY_MASK+RT_I64,tempi->a1.reg,tempi->a1.disp,
	tempi->a1.type&TY_MASK+RT_I64,tempi->a1.reg,tempi->a1.disp,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,
	tempi->a1.type&TY_MASK+RT_I64,tempi->a1.reg,tempi->a1.disp,op,ip);
  } else {
    if (tempi->a1.type&TY_MASK==TY_REG)
      r=tempi->a1.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,tempi->a1.type&TY_MASK+RT_I64,tempi->a1.reg,tempi->a1.disp,ip);
      r=REG_RCX;
    }
    ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,
      TY_DISP+RT_I64,r,0,ip);
    if (tempc1->size!=1) {
      ICAddSubEctImm(tempi,TY_DISP+RT_I64,r,0,
	TY_DISP+RT_I64,r,0,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,TY_DISP+RT_I64,r,0,op,ip);
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
       TY_DISP+t,REG_RDX,0,ip);
  }
}

U0 ICAssignPostIncDec(CIntermediateCode *tempi,I64 op,I64 ip)
{
  CHashClass *tempc1=tempi->ic_class2-1;
  I64 r;
  if (tempi->ic_flags & ICF_NOT_ADDR) {
    if (tempi->a1.type&TY_MASK==TY_REG)
      r=tempi->a1.reg;
    else {
      ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,tempi->a1.type&TY_MASK+RT_I64,tempi->a1.reg,tempi->a1.disp,ip);
      r=REG_RDX;
    }
    ICMov(tempi,TY_DISP+tempi->a1.type_pointed,r,0,
      tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    if (tempc1->size!=1 || tempi->a1.type&TY_MASK==TY_STK) {
      ICAddSubEctImm(tempi,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
	TY_REG+RT_I64,r,0,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,op,ip);
  } else {
    ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,
	  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip);
    ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,
	  TY_DISP+RT_I64,REG_RDX,0,ip);
    ICMov(tempi,TY_DISP+tempi->a1.type_pointed,REG_RCX,0,
	  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    if (tempi->r.type&TY_MASK)
      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
	tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip);
    if (tempc1->size!=1) {
      ICAddSubEctImm(tempi,TY_DISP+RT_I64,REG_RDX,0,
	TY_REG+RT_I64,REG_RCX,0,
	tempc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tempi,TY_DISP+RT_I64,REG_RDX,0,op,ip);
  }
}

U0 ICCmpAndBranch(CIntermediateCode *tempi,Bool has_result,I64 ip,
   I64 us,I64 is,I64 not_us,I64 not_is,
   U8 *buf,I64 ip2)
{
  I64 r1,r2,i=0x48,j,t1,t2,result_reg;
  Bool short_jmp,swap,done;
  CLexCodeMisc *lb;
  CICArg *a1=&tempi->a1,*a2=&tempi->a2;

  j=a1->disp;
  if (a1->type&TY_MASK==TY_IMM && MIN_I32<=j<=MAX_I32) {
    SwapI64(&a1,&a2);
    swap=TRUE;
    us=not_us;
    is=not_is;
  } else
    swap=FALSE;
  if (tempi->ic_class->raw_type&1 || tempi->ic_flags & ICF_USE_UNSIGNED)
    is=us;

  j=a2->disp;
  if (a2->type&TY_MASK==TY_IMM && MIN_I32<=j<=MAX_I32) {
    if (!has_result && TY_REG<=a1->type&TY_MASK<=TY_RIP_DISP32)
      ICAddSubEctImm(tempi,a1->type,a1->reg,a1->disp,
			   a1->type,a1->reg,a1->disp,j,0x073B,ip2);
    else {
      if (a1->type&TY_MASK==TY_REG)
	r1=a1->reg;
      else {
	ICMov(tempi,TY_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip2);
	r1=REG_RDX;
      }
      if (!j) {
	if (is.u8[2]==0x7C) {
	  ICTest(tempi,r1);
	  is=0x78880F;
	} else if (is.u8[2]==0x7D) {
	  ICTest(tempi,r1);
	  is=0x79890F;
	} else if (is.u8[2]==0x74 || is.u8[2]==0x75)
	  ICTest(tempi,r1);
	else {
	  if (r1>7)
	    i++;
	  ICU24(tempi,0xF88300+i+(r1&7)<<16);
	  ICU8(tempi,j);
	}
      } else {
	if (r1>7)
	  i++;
	if (MIN_I8<=j<=MAX_I8) {
	  ICU24(tempi,0xF88300+i+(r1&7)<<16);
	  ICU8(tempi,j);
	} else {
	  ICU24(tempi,0xF88100+i+(r1&7)<<16);
	  ICU32(tempi,j);
	}
      }
    }
    if (has_result) {
      if (!swap) {
	ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,TY_IMM+RT_I64,0,j,ip2);
	result_reg=REG_RCX;
      } else
	result_reg=r1;
    }
  } else {
    done=FALSE;
    t1=a1->type;
    r1=a1->reg;
    r2=a2->reg;
    t2=a2->type;
    if (t2&RT_MASK>=RT_I64 && !has_result &&
	  TY_DISP<=t2&TY_MASK<=TY_RIP_DISP32) {
      if (t1&TY_MASK!=TY_REG || t1&RT_MASK<RT_I64) {
	ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,a1->type,a1->reg,a1->disp,ip2);
	r1=REG_RAX;
      }
      i=ICModr1(r1,t2,r2,a2->disp);
      ICRex(tempi,i.u8[1]);
      ICU16(tempi,i.u8[2]<<8+0x3B);
      ICModr2(tempi,i,a2->disp,ip2+4);
      done=TRUE;
    } else if (t1&RT_MASK>=RT_I64 &&
	  TY_REG<=t1&TY_MASK<=TY_RIP_DISP32) {
      if (t2&TY_MASK!=TY_REG || t2&RT_MASK<RT_I64) {
	if (t1&TY_MASK==TY_REG && r1==REG_RAX) {
	  ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,a2->type,a2->reg,a2->disp,ip2);
	  r2=REG_RCX;
	} else {
	  ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip2);
	  r2=REG_RAX;
	}
      }
      i=ICModr1(r2,t1,r1,a1->disp);
      ICRex(tempi,i.u8[1]);
      ICU16(tempi,i.u8[2]<<8+0x39);
      ICModr2(tempi,i,a1->disp,ip2+4);
      if (has_result)
	result_reg=r2;
      done=TRUE;
    }
    if (!done) {
      if (tempi->ic_flags & ICF_A1_FIRST) {
	if (a1->type&TY_MASK==TY_REG)
	  r1=a1->reg;
	else {
	  ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,a1->type,a1->reg,a1->disp,ip2);
	  r1=REG_RAX;
	}
	if (a2->type&TY_MASK==TY_REG)
	  r2=a2->reg;
	else {
	  ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,a2->type,a2->reg,a2->disp,ip2);
	  r2=REG_RCX;
	}
      } else {
	if (a2->type&TY_MASK==TY_REG)
	  r2=a2->reg;
	else {
	  ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip2);
	  r2=REG_RAX;
	}
	if (a1->type&TY_MASK==TY_REG)
	  r1=a1->reg;
	else {
	  ICMov(tempi,TY_REG+RT_I64,REG_RCX,0,a1->type,a1->reg,a1->disp,ip2);
	  r1=REG_RCX;
	}
      }
      if (r2>7)
	i++;
      if (r1>7)
	i+=4;
      ICU24(tempi,0xC03B00+i+(r2&7)<<16+(r1&7)<<19);
      if (has_result)
	result_reg=r2;
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
    ICU16(tempi,i<<8+is.u8[2]);
  } else {
    tempi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
    if (lb->flags&CMF_POP_CMP) {
      if(tempi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    ICU16(tempi,is.u16[0]);
    ICU32(tempi,i);
  }
  if (has_result)
    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,TY_REG+RT_I64,result_reg,0,ip2);
}

U0 ICTestAndBranch(CIntermediateCode *tempi,I64 ip,I64 is,U8 *buf,I64 ip2)
{
  I64 i;
  Bool short_jmp;
  CLexCodeMisc *lb;

  if (tempi->a1.type&TY_MASK!=TY_REG) {
    if (tempi->a1.type&RT_MASK<=RT_U8 &&
	  TY_DISP<=tempi->a1.type&TY_MASK<=TY_RIP_DISP32) {
      i=ICModr1(tempi,tempi->a1.type,tempi->a1.reg,tempi->a1.disp);
      ICRex(tempi,i.u8[1]);
      ICU16(tempi,i.u8[2]<<8+0xF6);
      ICModr2(tempi,i,tempi->a1.disp,ip2+5);
      ICU8(tempi,0xFF);
    } else {
      ICMov(tempi,TY_REG+RT_I64,REG_RAX,0,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
      ICTest(tempi,REG_RAX);
    }
  } else
    ICTest(tempi,tempi->a1.reg);

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

U0 ICFlagBranch(CIntermediateCode *tempi,I64 ip,I64 is,U8 *buf)
{
  I64 i;
  Bool short_jmp;
  CLexCodeMisc *lb;

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
