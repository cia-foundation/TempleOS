U0 ICUnaries(CIntermediateCode *tmpi,I64 op,I64 ip)
{
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
  ICSlashOp(tmpi,MDF_REG+RT_I64,REG_RAX,0,op,ip);
  ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICNot(CIntermediateCode *tmpi,I64 ip)
{
  I64 i;
  if (tmpi->a1.type.raw_type<=RT_U8 && tmpi->a1.type&MDG_DISP_SIB_RIP) {
    i=ICModr1(tmpi,tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp);
    ICRex(tmpi,i.u8[1]);
    ICU16(tmpi,i.u8[2]<<8+0xF6); //TEST ?,0xFF
    ICModr2(tmpi,i,tmpi->a1.disp,ip+5);
    ICU8(tmpi,0xFF);
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
    ICTest(tmpi,REG_RAX);
  }
  ICU24(tmpi,0xC0940F);   //SETZ AL
  ICU32(tmpi,0xC0B60F48); //MOVZX RAX,AL
  ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	MDF_REG+RT_U64,REG_RAX,0,ip);
}

U0 ICAndAnd(CIntermediateCode *tmpi,I64 ip)
{
  I64 r2;
  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
  if (!(tmpi->a1.type&MDF_REG) || tmpi->a1.reg==REG_RAX) {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
    r2=REG_RDX;
  } else
    r2=tmpi->a1.reg;
  ICZero(tmpi,REG_RAX);
  ICTest(tmpi,r2);
  ICU16(tmpi,0x0874);
  ICTest(tmpi,REG_RCX);
  ICU16(tmpi,0x0374);
  ICU24(tmpi,0xC0FF48);
  ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICOrOr(CIntermediateCode *tmpi,I64 ip)
{
  I64 i=0x48,r2;
  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
  if (!(tmpi->a1.type&MDF_REG) || tmpi->a1.reg==REG_RAX) {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
    r2=REG_RDX;
  } else
    r2=tmpi->a1.reg;

  if (r2>7) {
    i++;
    r2&=7;
  }
  ICZero(tmpi,REG_RAX);
  ICU24(tmpi,0xC80B00+i+r2<<16);
  ICU16(tmpi,0x0374);
  ICU24(tmpi,0xC0FF48);
  ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICXorXor(CIntermediateCode *tmpi,I64 ip)
{
  I64 r2;
  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
  if (!(tmpi->a1.type&MDF_REG)) {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
    r2=REG_RDX;
  } else
    r2=tmpi->a1.reg;
  ICZero(tmpi,REG_RBX);
  ICTest(tmpi,r2);
  ICU16(tmpi,0x0374);
  ICU24(tmpi,0xC3FF48);

  ICZero(tmpi,REG_RAX);
  ICTest(tmpi,REG_RCX);
  ICU16(tmpi,0x0374);
  ICU24(tmpi,0xC0FF48);

  ICU24(tmpi,0xC33348);

  ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	MDF_REG+RT_I64,REG_RAX,0,ip);
}

U0 ICCmp(CIntermediateCode *tmpi,I64 us,I64 is,I64 ip)
{
  I64 r1,d1,r2,i=0x48,j=tmpi->a2.disp;
  if (tmpi->a2.type&MDF_IMM && MIN_I32<=j<=MAX_I32) {
    if (!(tmpi->ic_flags&(ICF_POP_CMP|ICF_PUSH_CMP)) &&
	  tmpi->a1.type&MDF_DISP && //TODO
	  tmpi->a1.type.raw_type>=RT_I64 && tmpi->a1.reg!=REG_RAX) {
      r1=tmpi->a1.reg;
      d1=tmpi->a1.disp;
      ICZero(tmpi,REG_RAX);
      if (r1>7)
	i++;
      if (MIN_I8<=j<=MAX_I8)
	i+=0x388300;
      else
	i+=0x388100;
      if (!d1) {
	ICU24(tmpi,0x000000+i+(r1&7)<<16);
      } else if (MIN_I8<=d1<=MAX_I8) {
	ICU24(tmpi,0x400000+i+(r1&7)<<16);
	ICU8(tmpi,d1);
      } else {
	ICU24(tmpi,0x800000+i+(r1&7)<<16);
	ICU32(tmpi,d1);
      }
      if (MIN_I8<=j<=MAX_I8)
	ICU8(tmpi,j);
      else
	ICU32(tmpi,j);
    } else {
      if (tmpi->ic_flags & ICF_POP_CMP) {
	ICPopRegs(tmpi,1<<REG_RCX);
	r1=REG_RCX;
      } else {
	if (tmpi->a1.type&MDF_REG && tmpi->a1.reg!=REG_RAX)
	  r1=tmpi->a1.reg;
	else {
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
		tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
	  r1=REG_RCX;
	}
      }
      ICZero(tmpi,REG_RAX);
      if (r1>7)
	i++;
      if (MIN_I8<=j<=MAX_I8) {
	ICU24(tmpi,0xF88300+i+(r1&7)<<16);
	ICU8(tmpi,j);
      } else {
	ICU24(tmpi,0xF88100+i+(r1&7)<<16);
	ICU32(tmpi,j);
      }
    }
    if (tmpi->ic_flags & ICF_PUSH_CMP)
      ICPush(tmpi,MDF_IMM+RT_I64,0,j,ip);
    if (tmpi->ic_class->raw_type&RTF_UNSIGNED ||
	  tmpi->ic_flags & ICF_USE_UNSIGNED)
      is=us;
    ICU16(tmpi,0x300+is);
    ICU24(tmpi,0xC0FF48);
    ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	  MDF_REG+RT_I64,REG_RAX,0,ip);
  } else {
    if (tmpi->a2.type&MDF_REG && tmpi->a2.reg!=REG_RAX)
      r2=tmpi->a2.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
      r2=REG_RCX;
    }
    if (tmpi->ic_flags & ICF_POP_CMP) {
      ICPopRegs(tmpi,1<<REG_RDX);
      r1=REG_RDX;
    } else {
      if (tmpi->a1.type&MDF_REG && tmpi->a1.reg!=REG_RAX)
	r1=tmpi->a1.reg;
      else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	      tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
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
    ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	  MDF_REG+RT_I64,REG_RAX,0,ip);
  }
}

U0 ICBitOps(CIntermediateCode *tmpi,CICArg *a1,CICArg *a2,
	CIntermediateCode *tmpi2,I64 op,I64 op_imm,I64 ip)
{//TODO:not fully utilizing Modr
  Bool res_not_used=ToBool(tmpi2->ic_flags&ICF_RES_NOT_USED);
  I64 r1,t2,r2,d2,i=0x48;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    t2=a2->type&MDG_MASK+RT_I64; //TODO: check overflow
    r2=a2->reg;
    d2=a2->disp;
    if (!(t2&MDG_REG_DISP_SIB_RIP) || !(r2.u8[0]!=REG_RAX&&(!(t2&MDF_SIB) ||
	r2.u8[1]&15!=REG_RAX) || res_not_used)) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,t2,r2,d2,ip);
      t2=MDF_REG+RT_I64;
      r2=REG_RCX;
      d2=0;
    }
    if (a1->type&MDF_REG && (a1->reg!=REG_RAX||res_not_used))
      r1=a1->reg;
    else if (!(a1->type&MDF_IMM) || a1->disp>63) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip);
      r1=REG_RDX;
    } else
      r1=0;
  } else {
    t2=MDF_DISP+RT_I64;
    d2=0;
    if (a2->type&MDF_REG && (a2->reg!=REG_RAX||res_not_used))
      r2=a2->reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,a2->type,a2->reg,a2->disp,ip);
      r2=REG_RCX;
    }
    if (a1->type&MDF_REG && (a1->reg!=REG_RAX||res_not_used))
      r1=a1->reg;
    else if (!(a1->type&MDF_IMM) || a1->disp>63) {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip);
      r1=REG_RDX;
    } else
      r1=0;
  }
  if (!res_not_used)
    ICZero(tmpi,REG_RAX);
  if (tmpi->ic_flags&ICF_LOCK && op!=0xA30F)
    ICU8(tmpi,OC_LOCK_PREFIX);
  if (a1->type&MDF_IMM && a1->disp<32)
    t2=t2&MDG_MASK+RT_U32;
  i=ICModr1(r1,t2,r2,d2);
  ICRex(tmpi,i.u8[1]);
  if (a1->type&MDF_IMM && a1->disp<64) {
    ICU24(tmpi,i.u8[2]<<16+op_imm);
    ICModr2(tmpi,i,d2,ip+5);
    ICU8(tmpi,a1->disp);
  } else {
    ICU24(tmpi,i.u8[2]<<16+op);
    ICModr2(tmpi,i,d2,ip+4);
  }
  if (!res_not_used) {
    ICU24(tmpi,0xC0920F); //SETC AL
    if (tmpi->r.type.mode)
      ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	    MDF_REG+RT_I64,REG_RAX,0,ip);
  }
}

U0 ICToUpper(CIntermediateCode *tmpi,I64 ip)
{
  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
  ICU32(tmpi,0x61F88348);
  ICU16(tmpi,0x0A7C);
  ICU32(tmpi,0x7AF88348);
  ICU16(tmpi,0x047F);
  ICU32(tmpi,0xE0C08348);
}

U0 ICToI64(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 ip)
{
  ICFCvt(cc,tmpi,REG_RAX,
	tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,TRUE,CN_MAIN,ip);
}

U0 ICToF64(CCmpCtrl *cc,CIntermediateCode *tmpi,I64 ip)
{
  ICFCvt(cc,tmpi,REG_RAX,
	tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,FALSE,CN_MAIN,ip);
}

U0 ICToBool(CCmpCtrl *,CIntermediateCode *tmpi,I64 ip)
{
  I64 r;
  if (tmpi->a1.type&MDF_REG)
    r=tmpi->a1.reg;
  else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
    r=REG_RAX;
  }
  ICTest(tmpi,r);
  ICU24(tmpi,0xC0950F);   //SETNZ AL
  ICU32(tmpi,0xC0B60F48); //MOVZX RAX,AL
}

U0 ICPreIncDec(CIntermediateCode *tmpi,I64 op,I64 ip)
{
  I64 r;
  CHashClass *tmpc=tmpi->ic_class,*tmpc1=tmpc-1;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (tmpc->ptr_stars_cnt && tmpc1->size!=1) {
      ICAddSubEctImm(tmpi,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,tmpc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tmpi,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,op,ip);
    if (tmpi->r.type.mode)
      ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,ip);
  } else {
    if (tmpi->a1.type&MDF_REG)
      r=tmpi->a1.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
      r=REG_RCX;
    }
    if (tmpc->ptr_stars_cnt &&
	  tmpc1->size!=1) {
      ICAddSubEctImm(tmpi,MDF_DISP+tmpi->a1_type_pointed_to,r,0,
	    MDF_DISP+tmpi->a1_type_pointed_to,r,0,
	    tmpc1->size,op.u16[3],ip);
    } else
      ICSlashOp(tmpi,
	    MDF_DISP+tmpi->a1_type_pointed_to,r,0,op,ip);
    if (tmpi->r.type.mode)
      ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	    MDF_DISP+tmpi->a1_type_pointed_to,r,0,ip);
  }
}

U0 ICPostIncDec(CIntermediateCode *tmpi,I64 op,I64 ip)
{
  I64 r;
  CHashClass *tmpc=tmpi->ic_class,*tmpc1=tmpc-1;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (tmpi->r.type.mode)
      ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,ip);
    if (tmpc->ptr_stars_cnt &&	tmpc1->size!=1)
      ICAddSubEctImm(tmpi,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,tmpc1->size,op.u16[3],ip);
    else
      ICSlashOp(tmpi,
	    tmpi->a1.type&MDG_MASK+tmpi->a1_type_pointed_to,
	    tmpi->a1.reg,tmpi->a1.disp,op,ip);
  } else {
    if (tmpi->a1.type&MDF_REG &&
	  !(tmpi->r.type&MDF_REG && tmpi->r.reg==tmpi->a1.reg))
      r=tmpi->a1.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
      r=REG_RCX;
    }
    if (tmpi->r.type.mode)
      ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	    MDF_DISP+tmpi->a1_type_pointed_to,r,0,ip);
    if (tmpc->ptr_stars_cnt &&	tmpc1->size!=1)
      ICAddSubEctImm(tmpi,MDF_DISP+tmpi->a1_type_pointed_to,r,0,
	    MDF_DISP+tmpi->a1_type_pointed_to,r,0,tmpc1->size,op.u16[3],ip);
    else
      ICSlashOp(tmpi,MDF_DISP+tmpi->a1_type_pointed_to,r,0,op,ip);
  }
}

U0 ICDerefPostIncDec(CIntermediateCode *tmpi,I64 op,I64 ip)
{
  CICType t;
  I64 r;
  CHashClass *tmpc1=tmpi->ic_class;
  t=tmpi->r.type.raw_type;
  if (t>tmpi->a1_type_pointed_to)
    t=tmpi->a1_type_pointed_to;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (tmpi->a1.type&MDF_REG)
      r=tmpi->a1.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    tmpi->a1.type&MDG_MASK+RT_I64,tmpi->a1.reg,tmpi->a1.disp,ip);
      r=REG_RDX;
    }
    ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	  MDF_DISP+t,r,0,ip);
    if (tmpc1->size!=1)
      ICAddSubEctImm(tmpi,
	    tmpi->a1.type&MDG_MASK+RT_I64,tmpi->a1.reg,tmpi->a1.disp,
	    tmpi->a1.type&MDG_MASK+RT_I64,tmpi->a1.reg,tmpi->a1.disp,
	    tmpc1->size,op.u16[3],ip);
    else
      ICSlashOp(tmpi,
	    tmpi->a1.type&MDG_MASK+RT_I64,tmpi->a1.reg,tmpi->a1.disp,op,ip);
  } else {
    if (tmpi->a1.type&MDF_REG)
      r=tmpi->a1.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,
	    tmpi->a1.type&MDG_MASK+RT_I64,tmpi->a1.reg,tmpi->a1.disp,ip);
      r=REG_RCX;
    }
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,MDF_DISP+RT_I64,r,0,ip);
    if (tmpc1->size!=1)
      ICAddSubEctImm(tmpi,MDF_DISP+RT_I64,r,0,MDF_DISP+RT_I64,r,0,
	    tmpc1->size,op.u16[3],ip);
    else
      ICSlashOp(tmpi,MDF_DISP+RT_I64,r,0,op,ip);
    ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	  MDF_DISP+t,REG_RDX,0,ip);
  }
}

U0 ICAssignPostIncDec(CIntermediateCode *tmpi,I64 op,I64 ip)
{
  CHashClass *tmpc1=tmpi->ic_class2-1;
  I64 r;
  if (tmpi->ic_flags & ICF_BY_VAL) {
    if (tmpi->a1.type&MDF_REG)
      r=tmpi->a1.reg;
    else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	    tmpi->a1.type&MDG_MASK+RT_I64,tmpi->a1.reg,tmpi->a1.disp,ip);
      r=REG_RDX;
    }
    ICMov(tmpi,MDF_DISP+tmpi->a1_type_pointed_to,r,0,
	  tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
    if (tmpi->r.type.mode)
      ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	    tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
    if (tmpc1->size!=1 || tmpi->a1.type&MDF_STK)
      ICAddSubEctImm(tmpi,tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,
	    MDF_REG+RT_I64,r,0,tmpc1->size,op.u16[3],ip);
    else
      ICSlashOp(tmpi,tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,op,ip);
  } else {
    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
	  tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip);
    ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_DISP+RT_I64,REG_RDX,0,ip);
    ICMov(tmpi,MDF_DISP+tmpi->a1_type_pointed_to,REG_RCX,0,
	  tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
    if (tmpi->r.type.mode)
      ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	    tmpi->a2.type,tmpi->a2.reg,tmpi->a2.disp,ip);
    if (tmpc1->size!=1)
      ICAddSubEctImm(tmpi,MDF_DISP+RT_I64,REG_RDX,0,
	    MDF_REG+RT_I64,REG_RCX,0,tmpc1->size,op.u16[3],ip);
    else
      ICSlashOp(tmpi,MDF_DISP+RT_I64,REG_RDX,0,op,ip);
  }
}

U0 ICCmpAndBranch(CIntermediateCode *tmpi,Bool has_res,I64 ip,
   I64 us,I64 is,I64 not_us,I64 not_is,U8 *buf,I64 ip2)
{
  I64 r1,r2,i=0x48,j,res_reg;
  CICType t1,t2;
  Bool short_jmp,swap,done;
  CCodeMisc *lb;
  CICArg *a1=&tmpi->a1,*a2=&tmpi->a2;

  j=a1->disp;
  if (a1->type&MDF_IMM && MIN_I32<=j<=MAX_I32) {
    SwapI64(&a1,&a2);
    swap=TRUE;
    us=not_us;
    is=not_is;
  } else
    swap=FALSE;
  if (tmpi->ic_class->raw_type&RTF_UNSIGNED ||
	tmpi->ic_flags & ICF_USE_UNSIGNED)
    is=us;

  j=a2->disp;
  if (a2->type&MDF_IMM && MIN_I32<=j<=MAX_I32) {
    if (!has_res && a1->type&MDG_REG_DISP_SIB_RIP)
      ICAddSubEctImm(tmpi,a1->type,a1->reg,a1->disp,
	    a1->type,a1->reg,a1->disp,j,0x073B,ip2);
    else {
      if (a1->type&MDF_REG)
	r1=a1->reg;
      else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,a1->type,a1->reg,a1->disp,ip2);
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
	if (MIN_I8<=j<=MAX_I8) {
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
	ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,MDF_IMM+RT_I64,0,j,ip2);
	res_reg=REG_RCX;
      } else
	res_reg=r1;
    }
  } else {
    done=FALSE;
    t1=a1->type;
    r1=a1->reg;
    r2=a2->reg;
    t2=a2->type;
    if (t2.raw_type>=RT_I64 && !has_res && t2&MDG_DISP_SIB_RIP) {
      if (!(t1&MDF_REG) || t1.raw_type<RT_I64) {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,a1->type,a1->reg,a1->disp,ip2);
	r1=REG_RAX;
      }
      i=ICModr1(r1,t2,r2,a2->disp);
      ICRex(tmpi,i.u8[1]);
      ICU16(tmpi,i.u8[2]<<8+0x3B);
      ICModr2(tmpi,i,a2->disp,ip2+4);
      done=TRUE;
    } else if (t1.raw_type>=RT_I64 && t1&MDG_REG_DISP_SIB_RIP) {
      if (!(t2&MDF_REG) || t2.raw_type<RT_I64) {
	if (t1&MDF_REG && r1==REG_RAX) {
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,a2->type,a2->reg,a2->disp,ip2);
	  r2=REG_RCX;
	} else {
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip2);
	  r2=REG_RAX;
	}
      }
      i=ICModr1(r2,t1,r1,a1->disp);
      ICRex(tmpi,i.u8[1]);
      ICU16(tmpi,i.u8[2]<<8+0x39);
      ICModr2(tmpi,i,a1->disp,ip2+4);
      if (has_res)
	res_reg=r2;
      done=TRUE;
    }
    if (!done) {
      if (a2->type&MDF_REG)
	r2=a2->reg;
      else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,a2->type,a2->reg,a2->disp,ip2);
	r2=REG_RAX;
      }
      if (a1->type&MDF_REG)
	r1=a1->reg;
      else {
	ICMov(tmpi,MDF_REG+RT_I64,REG_RCX,0,a1->type,a1->reg,a1->disp,ip2);
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

  ip+=tmpi->ic_cnt;
  lb=OptLabelFwd(tmpi->ic_data);
  short_jmp=ToBool(tmpi->ic_flags&ICF_SHORT_JMP);
  if (!buf && lb->addr!=INVALID_PTR) {
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP) {
      if(tmpi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    if (MIN_I8<=i<=MAX_I8)
      short_jmp=TRUE;
  }

  if (short_jmp) {
    tmpi->ic_flags|=ICF_SHORT_JMP;
    i=lb->addr-(ip+2);
    if (lb->flags&CMF_POP_CMP) {
      if(tmpi->ic_flags&ICF_PUSH_CMP)
	i+=4;
      else
	i+=8;
    }
    ICU16(tmpi,i<<8+is.u8[2]);
  } else {
    tmpi->ic_flags&=~ICF_SHORT_JMP;
    i=lb->addr-(ip+6);
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
    ICMov(tmpi,tmpi->r.type,tmpi->r.reg,tmpi->r.disp,
	  MDF_REG+RT_I64,res_reg,0,ip2);
}

U0 ICTestAndBranch(CIntermediateCode *tmpi,I64 ip,I64 is,U8 *buf,I64 ip2)
{
  I64 i;
  Bool short_jmp;
  CCodeMisc *lb;

  if (!(tmpi->a1.type&MDF_REG)) {
    if (tmpi->a1.type.raw_type<=RT_U8 && tmpi->a1.type&MDG_DISP_SIB_RIP) {
      i=ICModr1(tmpi,tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp);
      ICRex(tmpi,i.u8[1]);
      ICU16(tmpi,i.u8[2]<<8+0xF6);
      ICModr2(tmpi,i,tmpi->a1.disp,ip2+5);
      ICU8(tmpi,0xFF);
    } else {
      ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
	    tmpi->a1.type,tmpi->a1.reg,tmpi->a1.disp,ip2);
      ICTest(tmpi,REG_RAX);
    }
  } else
    ICTest(tmpi,tmpi->a1.reg);

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

U0 ICFlagBranch(CIntermediateCode *tmpi,I64 ip,I64 is,U8 *buf)
{
  I64 i;
  Bool short_jmp;
  CCodeMisc *lb;

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
