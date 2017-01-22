U0 ICUnaries(CIntermediateCode *tmpi,I64 op,I64 rip)
{
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
  ICSlashOp(tmpi,MDF_REG+RT_I64,REG_RAX,0,op,rip);
  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	MDF_REG+RT_I64,REG_RAX,0,rip);
}

U0 ICNot(CIntermediateCode *tmpi,I64 rip)
{
  I64 i;
  if (tmpi->arg1.type.raw_type<=RT_U8 && tmpi->arg1.type&MDG_DISP_SIB_RIP) {
    i=ICModr1(tmpi,tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp);
    ICRex(tmpi,i.u8[1]);
    ICU16(tmpi,i.u8[2]<<8+0xF6); //TEST ?,0xFF
    ICModr2(tmpi,i,,tmpi->arg1.disp,rip+1);
    ICU8(tmpi,0xFF);
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    ICTest(tmpi,REG_RAX);
  }
  ICU24(tmpi,0xC0940F);   //SETZ AL
  ICU32(tmpi,0xC0B60F48); //MOVZX RAX,AL
  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	MDF_REG+RT_U64,REG_RAX,0,rip);
}

U0 ICAndAnd(CIntermediateCode *tmpi,I64 rip)
{
  I64 r2;
  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
  if (!(tmpi->arg1.type&MDF_REG) || tmpi->arg1.reg==REG_RAX) {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    r2=REG_RDX;
  } else
    r2=tmpi->arg1.reg;
  ICZero(tmpi,REG_RAX);
  ICTest(tmpi,r2);
  ICU16(tmpi,0x0874);
  ICTest(tmpi,REG_RCX);
  ICU16(tmpi,0x0374);
  ICU24(tmpi,0xC0FF48);
  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	MDF_REG+RT_I64,REG_RAX,0,rip);
}

U0 ICOrOr(CIntermediateCode *tmpi,I64 rip)
{
  I64 i=0x48,r2;
  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
  if (!(tmpi->arg1.type&MDF_REG) || tmpi->arg1.reg==REG_RAX) {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    r2=REG_RDX;
  } else
    r2=tmpi->arg1.reg;

  if (r2>7) {
    i++;
    r2&=7;
  }
  ICZero(tmpi,REG_RAX);
  ICU24(tmpi,0xC80B00+i+r2<<16);
  ICU16(tmpi,0x0374);
  ICU24(tmpi,0xC0FF48);
  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	MDF_REG+RT_I64,REG_RAX,0,rip);
}

U0 ICXorXor(CIntermediateCode *tmpi,I64 rip)
{
  I64 r2;
  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
  if (!(tmpi->arg1.type&MDF_REG)) {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    r2=REG_RDX;
  } else
    r2=tmpi->arg1.reg;
  ICZero(tmpi,REG_RBX);
  ICTest(tmpi,r2);
  ICU16(tmpi,0x0374);
  ICU24(tmpi,0xC3FF48);

  ICZero(tmpi,REG_RAX);
  ICTest(tmpi,REG_RCX);
  ICU16(tmpi,0x0374);
  ICU24(tmpi,0xC0FF48);

  ICU24(tmpi,0xC33348);

  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	MDF_REG+RT_I64,REG_RAX,0,rip);
}

U0 ICCmp(CIntermediateCode *tmpi,I64 us,I64 is,I64 rip)
{
  I64 r1,d1,r2,i=0x48,j=tmpi->arg2.disp;
  if (tmpi->arg2.type&MDF_IMM && I32_MIN<=j<=I32_MAX) {
    if (!(tmpi->ic_flags&(ICF_POP_CMP|ICF_PUSH_CMP)) &&
	  tmpi->arg1.type&MDF_DISP && //TODO
	  tmpi->arg1.type.raw_type>=RT_I64 && tmpi->arg1.reg!=REG_RAX) {
      r1=tmpi->arg1.reg;
      d1=tmpi->arg1.disp;
      ICZero(tmpi,REG_RAX);
      if (r1>7)
	i++;
      if (I8_MIN<=j<=I8_MAX)
	i+=0x388300;
      else
	i+=0x388100;
      if (!d1) {
	ICU24(tmpi,0x000000+i+(r1&7)<<16);
      } else if (I8_MIN<=d1<=I8_MAX) {
	ICU24(tmpi,0x400000+i+(r1&7)<<16);
	ICU8(tmpi,d1);
      } else {
	ICU24(tmpi,0x800000+i+(r1&7)<<16);
	ICU32(tmpi,d1);
      }
      if (I8_MIN<=j<=I8_MAX)
	ICU8(tmpi,j);
      else
	ICU32(tmpi,j);
    } else {
      if (tmpi->ic_flags & ICF_POP_CMP) {
	ICPopRegs(tmpi,1<<REG_RCX);
	r1=REG_RCX;
      } else {
	if (tmpi->arg1.type&MDF_REG && tmpi->arg1.reg!=REG_RAX)
	  r1=tmpi->arg1.reg;
	else {
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
	  r1=REG_RCX;
	}
      }
      ICZero(tmpi,REG_RAX);
      if (r1>7)
	i++;
      if (I8_MIN<=j<=I8_MAX) {
	ICU24(tmpi,0xF88300+i+(r1&7)<<16);
	ICU8(tmpi,j);
      } else {
	ICU24(tmpi,0xF88100+i+(r1&7)<<16);
	ICU32(tmpi,j);
      }
    }
    if (tmpi->ic_flags & ICF_PUSH_CMP)
      ICPush(tmpi,MDF_IMM+RT_I64,0,j,rip);
    if (tmpi->ic_class->raw_type&RTF_UNSIGNED ||
	  tmpi->ic_flags & ICF_USE_UNSIGNED)
      is=us;
    ICU16(tmpi,0x300+is);
    ICU24(tmpi,0xC0FF48);
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_REG+RT_I64,REG_RAX,0,rip);
  } else {
    if (tmpi->arg2.type&MDF_REG && tmpi->arg2.reg!=REG_RAX)
      r2=tmpi->arg2.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
      r2=REG_RCX;
    }
    if (tmpi->ic_flags & ICF_POP_CMP) {
      ICPopRegs(tmpi,1<<REG_RDX);
      r1=REG_RDX;
    } else {
      if (tmpi->arg1.type&MDF_REG && tmpi->arg1.reg!=REG_RAX)
	r1=tmpi->arg1.reg;
      else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	      tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
	r1=REG_RDX;
      }
    }
    ICZero(tmpi,REG_RAX);
    if (r2>7)
      i++;
    if (r1>7)
      i+=4;
    if (tmpi->ic_flags & ICF_PUSH_CMP)
      ICPushRegs(tmpi,1<<r2);
    ICU24(tmpi,0xC03B00+i+(r2&7)<<16+(r1&7)<<19);
    if (tmpi->ic_class->raw_type&RTF_UNSIGNED ||
	  tmpi->ic_flags & ICF_USE_UNSIGNED)
      is=us;
    ICU16(tmpi,0x300+is);
    ICU24(tmpi,0xC0FF48);
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_REG+RT_I64,REG_RAX,0,rip);
  }
}

U0 ICBitOps(CIntermediateCode *tmpi,CICArg *arg1,CICArg *arg2,
	CIntermediateCode *tmpi2,I64 op,I64 op_imm,I64 rip)
{//TODO:not fully utilizing Modr
  Bool res_not_used=ToBool(tmpi2->ic_flags&ICF_RES_NOT_USED);
  I64 r1,t2,r2,d2,i=0x48;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    t2=arg2->type&MDG_MASK+RT_I64; //TODO: check overflow
    r2=arg2->reg;
    d2=arg2->disp;
    if (!(t2&MDG_REG_DISP_SIB_RIP) || !(r2.u8[0]!=REG_RAX&&(!(t2&MDF_SIB) ||
	r2.u8[1]&15!=REG_RAX) || res_not_used)) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,rip);
      t2=MDF_REG+RT_I64;
      r2=REG_RCX;
      d2=0;
    }
    if (arg1->type&MDF_REG && (arg1->reg!=REG_RAX||res_not_used))
      r1=arg1->reg;
    else if (!(arg1->type&MDF_IMM) || arg1->disp>63) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,arg1->type,arg1->reg,arg1->disp,rip);
      r1=REG_RDX;
    } else
      r1=0;
  } else {
    t2=MDF_DISP+RT_I64;
    d2=0;
    if (arg2->type&MDF_REG && (arg2->reg!=REG_RAX||res_not_used))
      r2=arg2->reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,arg2->type,arg2->reg,arg2->disp,rip);
      r2=REG_RCX;
    }
    if (arg1->type&MDF_REG && (arg1->reg!=REG_RAX||res_not_used))
      r1=arg1->reg;
    else if (!(arg1->type&MDF_IMM) || arg1->disp>63) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,arg1->type,arg1->reg,arg1->disp,rip);
      r1=REG_RDX;
    } else
      r1=0;
  }
  if (!res_not_used)
    ICZero(tmpi,REG_RAX);
  if (tmpi->ic_flags&ICF_LOCK && op!=0xA30F)
    ICU8(tmpi,OC_LOCK_PREFIX);
  if (arg1->type&MDF_IMM && arg1->disp<32)
    t2=t2&MDG_MASK+RT_U32;
  i=ICModr1(r1,t2,r2,d2);
  ICRex(tmpi,i.u8[1]);
  if (arg1->type&MDF_IMM && arg1->disp<64) {
    ICU24(tmpi,i.u8[2]<<16+op_imm);
    ICModr2(tmpi,i,,d2,rip+1);
    ICU8(tmpi,arg1->disp);
  } else {
    ICU24(tmpi,i.u8[2]<<16+op);
    ICModr2(tmpi,i,,d2,rip);
  }
  if (!res_not_used) {
    ICU24(tmpi,0xC0920F); //SETC AL
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    MDF_REG+RT_I64,REG_RAX,0,rip);
  }
}

U0 ICToUpper(CIntermediateCode *tmpi,I64 rip)
{
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
  ICU32(tmpi,0x61F88348);
  ICU16(tmpi,0x0A7C);
  ICU32(tmpi,0x7AF88348);
  ICU16(tmpi,0x047F);
  ICU32(tmpi,0xE0C08348);
}

U0 ICToI64(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 rip)
{
  ICFCvt(cc,tmpi,REG_RAX,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,TRUE,CN_INST,rip);
}

U0 ICToF64(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 rip)
{
  ICFCvt(cc,tmpi,REG_RAX,
	tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,FALSE,CN_INST,rip);
}

U0 ICToBool(CCmpCtrl *,CIntermediateCode *tmpi,I64 rip)
{
  I64 r;
  if (tmpi->arg1.type&MDF_REG)
    r=tmpi->arg1.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    r=REG_RAX;
  }
  ICTest(tmpi,r);
  ICU24(tmpi,0xC0950F);   //SETNZ AL
  ICU32(tmpi,0xC0B60F48); //MOVZX RAX,AL
}

U0 ICPreIncDec(CIntermediateCode *tmpi,I64 op,I64 rip)
{
  I64 r;
  CHashClass *tmpc=tmpi->ic_class,*tmpc1=tmpc-1;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (tmpc->ptr_stars_cnt && tmpc1->size!=1) {
      ICAddSubEctImm(tmpi,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,tmpc1->size,op.u16[3],rip);
    } else
      ICSlashOp(tmpi,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,op,rip);
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,rip);
  } else {
    if (tmpi->arg1.type&MDF_REG)
      r=tmpi->arg1.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      r=REG_RCX;
    }
    if (tmpc->ptr_stars_cnt &&
	  tmpc1->size!=1) {
      ICAddSubEctImm(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,r,0,
	    MDF_DISP+tmpi->arg1_type_pointed_to,r,0,
	    tmpc1->size,op.u16[3],rip);
    } else
      ICSlashOp(tmpi,
	    MDF_DISP+tmpi->arg1_type_pointed_to,r,0,op,rip);
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    MDF_DISP+tmpi->arg1_type_pointed_to,r,0,rip);
  }
}

U0 ICPostIncDec(CIntermediateCode *tmpi,I64 op,I64 rip)
{
  I64 r;
  CHashClass *tmpc=tmpi->ic_class,*tmpc1=tmpc-1;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,rip);
    if (tmpc->ptr_stars_cnt &&	tmpc1->size!=1)
      ICAddSubEctImm(tmpi,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,tmpc1->size,op.u16[3],rip);
    else
      ICSlashOp(tmpi,
	    tmpi->arg1.type&MDG_MASK+tmpi->arg1_type_pointed_to,
	    tmpi->arg1.reg,tmpi->arg1.disp,op,rip);
  } else {
    if (tmpi->arg1.type&MDF_REG &&
	  !(tmpi->res.type&MDF_REG && tmpi->res.reg==tmpi->arg1.reg))
      r=tmpi->arg1.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      r=REG_RCX;
    }
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    MDF_DISP+tmpi->arg1_type_pointed_to,r,0,rip);
    if (tmpc->ptr_stars_cnt &&	tmpc1->size!=1)
      ICAddSubEctImm(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,r,0,
	    MDF_DISP+tmpi->arg1_type_pointed_to,r,0,tmpc1->size,op.u16[3],rip);
    else
      ICSlashOp(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,r,0,op,rip);
  }
}

U0 ICDerefPostIncDec(CIntermediateCode *tmpi,I64 op,I64 rip)
{
  CICType t;
  I64 r;
  CHashClass *tmpc1=tmpi->ic_class;
  t=tmpi->res.type.raw_type;
  if (t>tmpi->arg1_type_pointed_to)
    t=tmpi->arg1_type_pointed_to;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (tmpi->arg1.type&MDF_REG)
      r=tmpi->arg1.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    tmpi->arg1.type&MDG_MASK+RT_I64,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      r=REG_RDX;
    }
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_DISP+t,r,0,rip);
    if (tmpc1->size!=1)
      ICAddSubEctImm(tmpi,
	    tmpi->arg1.type&MDG_MASK+RT_I64,tmpi->arg1.reg,tmpi->arg1.disp,
	    tmpi->arg1.type&MDG_MASK+RT_I64,tmpi->arg1.reg,tmpi->arg1.disp,
	    tmpc1->size,op.u16[3],rip);
    else
      ICSlashOp(tmpi,tmpi->arg1.type&MDG_MASK+RT_I64,
	    tmpi->arg1.reg,tmpi->arg1.disp,op,rip);
  } else {
    if (tmpi->arg1.type&MDF_REG)
      r=tmpi->arg1.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->arg1.type&MDG_MASK+RT_I64,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      r=REG_RCX;
    }
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_I64,r,0,rip);
    if (tmpc1->size!=1)
      ICAddSubEctImm(tmpi,MDF_DISP+RT_I64,r,0,MDF_DISP+RT_I64,r,0,
	    tmpc1->size,op.u16[3],rip);
    else
      ICSlashOp(tmpi,MDF_DISP+RT_I64,r,0,op,rip);
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_DISP+t,REG_RDX,0,rip);
  }
}

U0 ICAssignPostIncDec(CIntermediateCode *tmpi,I64 op,I64 rip)
{
  CHashClass *tmpc1=tmpi->ic_class2-1;
  I64 r;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (tmpi->arg1.type&MDF_REG)
      r=tmpi->arg1.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    tmpi->arg1.type&MDG_MASK+RT_I64,tmpi->arg1.reg,tmpi->arg1.disp,rip);
      r=REG_RDX;
    }
    ICMov(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,r,0,
	  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
    if (tmpc1->size!=1 || tmpi->arg1.type&MDF_STK)
      ICAddSubEctImm(tmpi,tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
	    MDF_REG+RT_I64,r,0,tmpc1->size,op.u16[3],rip);
    else
      ICSlashOp(tmpi,tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,op,rip);
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip);
    ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_DISP+RT_I64,REG_RDX,0,rip);
    ICMov(tmpi,MDF_DISP+tmpi->arg1_type_pointed_to,REG_RCX,0,
	  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
    if (tmpi->res.type.mode)
      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	    tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip);
    if (tmpc1->size!=1)
      ICAddSubEctImm(tmpi,MDF_DISP+RT_I64,REG_RDX,0,
	    MDF_REG+RT_I64,REG_RCX,0,tmpc1->size,op.u16[3],rip);
    else
      ICSlashOp(tmpi,MDF_DISP+RT_I64,REG_RDX,0,op,rip);
  }
}

U0 ICCmpAndBranch(CIntermediateCode *tmpi,Bool has_res,I64 rip,
   I64 us,I64 is,I64 not_us,I64 not_is,U8 *buf,I64 rip2)
{
  I64 r1,r2,i=0x48,j,res_reg;
  CICType t1,t2;
  Bool short_jmp,swap,done;
  CCodeMisc *lb;
  CICArg *arg1=&tmpi->arg1,*arg2=&tmpi->arg2;

  j=arg1->disp;
  if (arg1->type&MDF_IMM && I32_MIN<=j<=I32_MAX) {
    SwapI64(&arg1,&arg2);
    swap=TRUE;
    us=not_us;
    is=not_is;
  } else
    swap=FALSE;
  if (tmpi->ic_class->raw_type&RTF_UNSIGNED ||
	tmpi->ic_flags & ICF_USE_UNSIGNED)
    is=us;

  j=arg2->disp;
  if (arg2->type&MDF_IMM && I32_MIN<=j<=I32_MAX) {
    if (!has_res && arg1->type&MDG_REG_DISP_SIB_RIP)
      ICAddSubEctImm(tmpi,arg1->type,arg1->reg,arg1->disp,
	    arg1->type,arg1->reg,arg1->disp,j,0x073B,rip2);
    else {
      if (arg1->type&MDF_REG)
	r1=arg1->reg;
      else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,arg1->type,
	      arg1->reg,arg1->disp,rip2);
	r1=REG_RDX;
      }
      if (!j) {
	if (is.u8[2]==0x7C) {
	  ICTest(tmpi,r1);
	  is=0x78880F;
	} else if (is.u8[2]==0x7D) {
	  ICTest(tmpi,r1);
	  is=0x79890F;
	} else if (is.u8[2]==0x74 || is.u8[2]==0x75)
	  ICTest(tmpi,r1);
	else {
	  if (r1>7)
	    i++;
	  ICU24(tmpi,0xF88300+i+(r1&7)<<16);
	  ICU8(tmpi,j);
	}
      } else {
	if (r1>7)
	  i++;
	if (I8_MIN<=j<=I8_MAX) {
	  ICU24(tmpi,0xF88300+i+(r1&7)<<16);
	  ICU8(tmpi,j);
	} else {
	  ICU24(tmpi,0xF88100+i+(r1&7)<<16);
	  ICU32(tmpi,j);
	}
      }
    }
    if (has_res) {
      if (!swap) {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_IMM+RT_I64,0,j,rip2);
	res_reg=REG_RCX;
      } else
	res_reg=r1;
    }
  } else {
    done=FALSE;
    t1=arg1->type;
    r1=arg1->reg;
    r2=arg2->reg;
    t2=arg2->type;
    if (t2.raw_type>=RT_I64 && !has_res && t2&MDG_DISP_SIB_RIP) {
      if (!(t1&MDF_REG) || t1.raw_type<RT_I64) {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg1->type,
	      arg1->reg,arg1->disp,rip2);
	r1=REG_RAX;
      }
      i=ICModr1(r1,t2,r2,arg2->disp);
      ICRex(tmpi,i.u8[1]);
      ICU16(tmpi,i.u8[2]<<8+0x3B);
      ICModr2(tmpi,i,,arg2->disp,rip2);
      done=TRUE;
    } else if (t1.raw_type>=RT_I64 && t1&MDG_REG_DISP_SIB_RIP) {
      if (!(t2&MDF_REG) || t2.raw_type<RT_I64) {
	if (t1&MDF_REG && r1==REG_RAX) {
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,arg2->type,
		arg2->reg,arg2->disp,rip2);
	  r2=REG_RCX;
	} else {
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg2->type,
		arg2->reg,arg2->disp,rip2);
	  r2=REG_RAX;
	}
      }
      i=ICModr1(r2,t1,r1,arg1->disp);
      ICRex(tmpi,i.u8[1]);
      ICU16(tmpi,i.u8[2]<<8+0x39);
      ICModr2(tmpi,i,,arg1->disp,rip2);
      if (has_res)
	res_reg=r2;
      done=TRUE;
    }
    if (!done) {
      if (arg2->type&MDF_REG)
	r2=arg2->reg;
      else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,arg2->type,
	      arg2->reg,arg2->disp,rip2);
	r2=REG_RAX;
      }
      if (arg1->type&MDF_REG)
	r1=arg1->reg;
      else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,arg1->type,
	      arg1->reg,arg1->disp,rip2);
	r1=REG_RCX;
      }
      if (r2>7)
	i++;
      if (r1>7)
	i+=4;
      ICU24(tmpi,0xC03B00+i+(r2&7)<<16+(r1&7)<<19);
      if (has_res)
	res_reg=r2;
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
    ICU16(tmpi,i<<8+is.u8[2]);
  } else {
    tmpi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(rip+6);
    if (lb->flags&CMF_POP_CMP) {
      if(tmpi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    ICU16(tmpi,is.u16[0]);
    ICU32(tmpi,i);
  }
  if (has_res)
    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
	  MDF_REG+RT_I64,res_reg,0,rip2);
}

U0 ICTestAndBranch(CIntermediateCode *tmpi,I64 rip,I64 is,U8 *buf,I64 rip2)
{
  I64 i;
  Bool short_jmp;
  CCodeMisc *lb;

  if (!(tmpi->arg1.type&MDF_REG)) {
    if (tmpi->arg1.type.raw_type<=RT_U8 && tmpi->arg1.type&MDG_DISP_SIB_RIP) {
      i=ICModr1(tmpi,tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp);
      ICRex(tmpi,i.u8[1]);
      ICU16(tmpi,i.u8[2]<<8+0xF6);
      ICModr2(tmpi,i,,tmpi->arg1.disp,rip2+1);
      ICU8(tmpi,0xFF);
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
      ICTest(tmpi,REG_RAX);
    }
  } else
    ICTest(tmpi,tmpi->arg1.reg);

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

U0 ICFlagBranch(CIntermediateCode *tmpi,I64 rip,I64 is,U8 *buf)
{
  I64 i;
  Bool short_jmp;
  CCodeMisc *lb;

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
