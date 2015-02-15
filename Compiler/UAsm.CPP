I64 InstEntriesCompare(CInstruction *tempi1,CInstruction *tempi2)
{
  I64 i1,i2,j=0,result=0,oc_cnt1=tempi1->opcode_cnt,oc_cnt2=tempi2->opcode_cnt;
  if (tempi1->flags&IEF_STI)
    oc_cnt1--;
  if (tempi2->flags&IEF_STI)
    oc_cnt2--;
  while (TRUE) {
    if (j<oc_cnt1 && j<oc_cnt2) {
      if (result=tempi1->opcode[j]-tempi2->opcode[j])
	return result;
      j++;
    } else {
      if (result=oc_cnt1-oc_cnt2)
	return result;

      if (tempi1->flags&IEF_STI)
	i1=tempi1->opcode[j]>>3&7;
      else
	i1=tempi1->slash_val;
      if (tempi2->flags&IEF_STI)
	i2=tempi2->opcode[j]>>3&7;
      else
	i2=tempi2->slash_val;
	if (result=i1-i2) return result;

      if (tempi1->flags&IEF_STI && tempi2->flags&IEF_STI) {
	i1=tempi1->opcode[j]&7;
	i2=tempi2->opcode[j]&7;
	  if (result=i1-i2) return result;
      }

      if (tempi1->flags&IEF_STI)
	return 1;

      if (tempi2->flags&IEF_STI)
	return -1;

      i1=tempi1->flags & IEF_OP_SIZE32;
      i2=tempi2->flags & IEF_OP_SIZE32;
	if (result=i1-i2) return result;

      i1=Bt(&uasm.ins64_mask,tempi1->arg1) || Bt(&uasm.ins64_mask,tempi1->arg2);
      i2=Bt(&uasm.ins64_mask,tempi2->arg1) || Bt(&uasm.ins64_mask,tempi2->arg2);
	if (result=i1-i2) return result;

      i1=tempi1->flags & IEF_48_REX;
      i2=tempi2->flags & IEF_48_REX;
	if (result=i1-i2) return result;

      i1=tempi1->arg2==ARGT_IMM64 || tempi1->arg2==ARGT_UIMM64;
      i2=tempi2->arg2==ARGT_IMM64 || tempi2->arg2==ARGT_UIMM64;
      return i1-i2;
    }
  }
}

/*
U0 DumpUasmIns(CInstruction *tempi)
{
  CHashOpcode *tempo=tempi(U8 *)-tempi->ins_entry_num*sizeof(CInstruction)-offset(CHashOpcode.ins);
  "%10s:%02d,%02d SV:%02d\n",tempo->str,tempi->arg1,tempi->arg2,tempi->slash_val;
}
U0 DumpUasmTable()
{
  I64 k;
  for (k=0;k<uasm.table_entries;k++)
    DumpUasmIns(uasm.table[k]);
}
*/

CInstruction *InstEntryFind(U8 *ip,I64 opsize)
{//Binary Search
  I64 i,j,n,m,k,a1,a2,o1,o2,oc_cnt,sv;
  CInstruction *tempi;
  i=0;
  j=uasm.table_entries-1;
  while (TRUE) {
    k=(i+j)>>1;  //binary search
    tempi=uasm.table[k];
//DumpUasmIns(tempi);
    m=0;
    n=0;
    while (TRUE) {
      a1=tempi->arg1;
      a2=tempi->arg2;
      oc_cnt=tempi->opcode_cnt;
      if (tempi->flags&IEF_STI)
	oc_cnt--;
      if (n<oc_cnt) {
	o1=ip[n];
	if (n==tempi->opcode_cnt-1 && tempi->flags & IEF_PLUS_OPCODE)
	  o1&=-8;
	o2=tempi->opcode[n++];
	if (m=o1-o2)
	  break;
      } else {
	if (tempi->flags&IEF_STI)
	  sv=tempi->opcode[tempi->opcode_cnt-1]>>3&7;
	else
	  sv=tempi->slash_val;
	if (sv<8) {
	  if (!(m=ip[n]>>3&7-sv)) {
	    if ((Bt(&uasm.mem_mask,a1) || Bt(&uasm.mem_mask,a2)) &&
		ip[n]&0xC0==0xC0) {
	      m=1;
	      break;
	    } else if (tempi->flags&IEF_STI) {
	      if (ip[n]&0xC0!=0xC0) {
		m=-1;
		break;
	      }
	      if (a1!=ARGT_STI && a2!=ARGT_STI)
		m=ip[n]&0x7-tempi->opcode[tempi->opcode_cnt-1]&7;
	      break;
	    }
	    if (opsize==16) {
	      if (tempi->flags & IEF_OP_SIZE32) {
		m=-1;
		break;
	      }
	    } else {
	      if (tempi->flags & IEF_OP_SIZE16) {
		m=1;
		break;
	      }
	    }
	    if (opsize==64||a1==ARGT_M64||a2==ARGT_M64||tempi->flags&IEF_NO_REX) {
	      if (!Bt(&uasm.ins64_mask,a1) && !Bt(&uasm.ins64_mask,a2) && !(tempi->flags&IEF_48_REX))
		m=1;
	    } else {
	      if (Bt(&uasm.ins64_mask,a1) || Bt(&uasm.ins64_mask,a2) || tempi->flags&IEF_48_REX)
		m=-1;
	    }
	  }
	} else {
	  m=0;
	  if (opsize==16) {
	    if (tempi->flags & IEF_OP_SIZE32) {
	      m=-1;
	      break;
	    }
	  } else {
	    if (tempi->flags & IEF_OP_SIZE16) {
	      m=1;
	      break;
	    }
	  }
	  if (opsize==64 || a1==ARGT_M64 || a2==ARGT_M64) {
	    if (!Bt(&uasm.ins64_mask,a1) && !Bt(&uasm.ins64_mask,a2) && !(tempi->flags&IEF_48_REX)&&
		!(a2==ARGT_NO &&
	    (ARGT_UIMM8<=a1<=ARGT_UIMM64 ||
		ARGT_IMM8<=a1<=ARGT_IMM64 ) ))
	      m=1;
	    else if (tempi->arg2==ARGT_IMM64 || tempi->arg2==ARGT_UIMM64) {
	      if (a2!=ARGT_IMM64&&a2!=ARGT_UIMM64)
		m=1;
	    } else if (a2==ARGT_IMM64||a2==ARGT_UIMM64)
		m=-1;
	  } else {
	    if (Bt(&uasm.ins64_mask,a1) || Bt(&uasm.ins64_mask,a2) || tempi->flags&IEF_48_REX)
	      m=-1;
	  }
	}
	break;
      }
    }

    if (m>0) {
      if (k==i) {
	k=j;
	break;
      } else
	i=k;
    } else if (m<0) {
      if (k-i<=1) {
	k=i;
	break;
      } else
	j=k;
    } else
      break;
  }
  return uasm.table[k];
}

U0 UasmHashLoad()
{
  CHashOpcode *temph;
  CInstruction *tempi;
  I64 i,j,k,cnt=0;

  uasm.table=NULL;
  uasm.table_entries=0;
  uasm.ins64_mask=0x0880888880+1<<ARGT_ST0+1<<ARGT_STI;
  uasm.signed_arg_mask=1<<ARGT_REL8+1<<ARGT_REL16+1<<ARGT_REL32+
	1<<ARGT_IMM8+1<<ARGT_IMM16+1<<ARGT_IMM32+1<<ARGT_IMM64;
  uasm.mem_mask=1<<ARGT_M8+1<<ARGT_M16+1<<ARGT_M32+1<<ARGT_M64;

  for (i=0;i<=cmp.asm_hash->mask;i++) {
    temph=cmp.asm_hash->body[i];
    while (temph) {
      if (temph->type==HTT_OPCODE)
	cnt+=temph->instruction_entry_cnt;
      temph=temph->next;
    }
  }
  uasm.table_entries=cnt;

  j=0;
  uasm.table=MAlloc(cnt*sizeof(U8 *));
  for (i=0;i<=cmp.asm_hash->mask;i++) {
    temph=cmp.asm_hash->body[i];
    while (temph) {
      if (temph->type==HTT_OPCODE) {
	tempi=&temph->ins;
	for (k=0;k<temph->instruction_entry_cnt;k++) {
	  uasm.table[j++]=tempi;
	  tempi++;
	}
      }
      temph=temph->next;
    }
  }

  QSortI64(uasm.table,cnt,&InstEntriesCompare);
}

I64 StrMatchBits(U8 *st)
{
  if (StrMatch("8",st))
    return 8;
  if (StrMatch("16",st))
    return 16;
  if (StrMatch("32",st))
    return 32;
  if (StrMatch("64",st))
    return 64;
  return 0;
}

U0 Ui(U8 *buf,U8 **_ip,I64 seg_size=64,I64 *_jmp_dst=NULL,Bool just_ins=FALSE)
{//Unassembles one instruction
  I64 i,disp,imm,opsize,opadd,
	a1,a2,reloced_a1,reloced_a2,
	a1_size=0,a2_size=0,reloced_a1_size,reloced_a2_size,
	ModrM=-1,SIB=-1,scale,r1,r2,
	Mod=-1,RM1=-1,RM2=-1,REX=-1,REX_r=0,REX_x=0,REX_b=0;
  Bool cont;
  CInstruction *tempi,*tempi2;
  CHashOpcode *tempo;
  U8 *ip=*_ip,*ptr,*reloced_a1_st,*reloced_a2_st,*bin_data_area1,*bin_data_area2,
	line1[512],line2[512],buf2[512],a1_st[512],a2_st[512],seg_overrides[32];

  if (_jmp_dst) *_jmp_dst=-1;
  if (seg_size==16) {
    opsize=16;
    opadd=16;
  } else if (seg_size==32) {
    opsize=32;
    opadd=32;
  } else {
    opsize=32;
    opadd=64;
  }
  *a1_st=0;
  *a2_st=0;
  if (!IsRaw && PutSrcLink(ip,1,line1))
    CatPrint(line1,"\n");
  else
    *line1=0;

  StrPrint(line1+StrLen(line1),"%24tp ",ip);
  bin_data_area1=line1+StrLen(line1);
  for (i=0;i<6;i++)
    CatPrint(line1,"%02X",ip[i]);
  CatPrint(line1," ");

  StrPrint(line2,"%24tp ",ip+6);
  bin_data_area2=line2+StrLen(line2);
  for (i=6;i<12;i++)
    CatPrint(line2,"%02X",ip[i]);

  *seg_overrides=0;
  cont=TRUE;
  do {
    switch (*ip++) {
      case 0x2E: if (StrLen(seg_overrides)<24) CatPrint(seg_overrides,"CS:"); break;
      case 0x36: if (StrLen(seg_overrides)<24) CatPrint(seg_overrides,"SS:"); break;
      case 0x3E: if (StrLen(seg_overrides)<24) CatPrint(seg_overrides,"DS:"); break;
      case 0x26: if (StrLen(seg_overrides)<24) CatPrint(seg_overrides,"ES:"); break;
      case 0x64: if (StrLen(seg_overrides)<24) CatPrint(seg_overrides,"FS:"); break;
      case 0x65: if (StrLen(seg_overrides)<24) CatPrint(seg_overrides,"GS:"); break;
      case OC_OP_SIZE_PREFIX:
	if (opsize==16)
	  opsize=32;
	else
	  opsize=16;
	break;
      case OC_ADDR_SIZE_PREFIX:
	if (opadd==16)
	  opadd=32;
	else
	  opadd=16;
	break;
      sub_switch_start:
	case 0x48...0x4F:
	  opsize=64;
	case 0x40...0x47:
      sub_switch_end:
	REX=*(ip-1);
	REX_b=Bt(&REX,0)<<3;
	REX_x=Bt(&REX,1)<<3;
	REX_r=Bt(&REX,2)<<3;
	break;
      default:
	cont=FALSE;
    }
  } while (cont);
  ip--;

  tempi=InstEntryFind(ip,opsize);
  if (opsize==32 && seg_size==64) {
    tempi2=InstEntryFind(ip,64);
    if (tempi2!=tempi &&
	  tempi2->flags&IEF_REX_ONLY_R8_R15 ||
	  tempi2->flags&IEF_REX_XOR_LIKE&& ip[1]>>3&7==ip[1]&7)
      tempi=tempi2;
  }

  ip+=tempi->opcode_cnt;
  tempo=tempi(U8 *)-tempi->ins_entry_num*sizeof(CInstruction)-offset(CHashOpcode.ins);
  if (just_ins)
    *line1=0;
  CatPrint(line1,tempo->str);

  a1=tempi->arg1;
  a2=tempi->arg2;

  if (a1_size=StrMatchBits(DefineSub(a1,"ST_ARG_TYPES"))) {
    if (Bt(&uasm.signed_arg_mask,a1))
      CatPrint(a1_st,"I%d ",a1_size);
    else
      CatPrint(a1_st,"U%d ",a1_size);
  }

  if (a2_size=StrMatchBits(DefineSub(a2,"ST_ARG_TYPES"))) {
    if (Bt(&uasm.signed_arg_mask,a2))
      CatPrint(a2_st,"I%d ",a2_size);
    else
      CatPrint(a2_st,"U%d ",a2_size);
  }

  if (tempi->flags & IEF_PLUS_OPCODE) {
    ip--;
    RM1=*ip++ - tempi->opcode[tempi->opcode_cnt-1]+REX_b;
    ptr=NULL;
    if (ARGT_R8<=a1<=ARGT_R64) {
      if (a1_size==8) {
	if (REX!=-1)
	  ptr="ST_U8_REX_REGS";
	else
	  ptr="ST_U8_REGS";
      } else if (a1_size==16)
	ptr="ST_U16_REGS";
      else if (a1_size==32)
	ptr="ST_U32_REGS";
      else if (a1_size==64)
	ptr="ST_U64_REGS";
      if (ptr)
	CatPrint(a1_st,"%Z",RM1,ptr);
    } else {
      if (a2_size==8) {
	if (REX!=-1)
	  ptr="ST_U8_REX_REGS";
	else
	  ptr="ST_U8_REGS";
      } else if (a2_size==16)
	ptr="ST_U16_REGS";
      else if (a2_size==32)
	ptr="ST_U32_REGS";
      else if (a2_size==64)
	ptr="ST_U64_REGS";
      if (ptr)
	CatPrint(a2_st,"%Z",RM1,ptr);
    }
  }

  if (ARGT_RM8<=a1<=ARGT_RM64 ||
	ARGT_M8<=a1<=ARGT_M64 ||
	ARGT_RM8<=a2<=ARGT_RM64 ||
	ARGT_M8<=a2<=ARGT_M64) {
    if (ARGT_RM8<=a2<=ARGT_RM64 ||
	  ARGT_M8<=a2<=ARGT_M64) {
      reloced_a1=a2;
      reloced_a2=a1;
      reloced_a1_size=a2_size;
      reloced_a2_size=a1_size;
      reloced_a1_st=a2_st;
      reloced_a2_st=a1_st;
    } else {
      reloced_a1=a1;
      reloced_a2=a2;
      reloced_a1_size=a1_size;
      reloced_a2_size=a2_size;
      reloced_a1_st=a1_st;
      reloced_a2_st=a2_st;
    }

    CatPrint(reloced_a1_st,seg_overrides);
    ModrM=*ip++;
    Mod=ModrM>>6 & 3;
    RM1=ModrM & 7+REX_b;
    RM2=ModrM>>3 & 7+REX_r;
    if (Mod<3 && RM1&7==4)
      SIB=*ip++;
    if (Mod==1) {
      disp=*ip(U8 *)++;
      CatPrint(reloced_a1_st,"%02X",disp);
    } else if (Mod==2) {
      disp=*ip(U32 *)++;
      CatPrint(reloced_a1_st,"%08X",disp);
    }
    if (tempi->slash_val<8)
      RM2=-1;
    else {
      ptr=NULL;
      if (reloced_a2==ARGT_SREG) {
	if (RM2<=5)
	  ptr="ST_SEG_REGS";
      } else if (!(ARGT_IMM8<=reloced_a2<=ARGT_IMM64) &&
	    !(ARGT_UIMM8<=reloced_a2<=ARGT_UIMM64)) {
	if (reloced_a2_size==8) {
	  if (REX!=-1)
	    ptr="ST_U8_REX_REGS";
	  else
	    ptr="ST_U8_REGS";
	} else if (reloced_a2_size==16)
	  ptr="ST_U16_REGS";
	else if (reloced_a2_size==32)
	  ptr="ST_U32_REGS";
	else if (reloced_a2_size==64)
	  ptr="ST_U64_REGS";
      }
      if (ptr)
	CatPrint(reloced_a2_st,"%Z",RM2,ptr);
    }
    if (RM1&7==5 && !Mod) {
      disp=*ip(I32 *)++;
      if (seg_size==64) {
	disp+=ip;
	if (reloced_a2==ARGT_IMM8 || reloced_a2==ARGT_UIMM8)
	  disp++;
	else if (reloced_a2==ARGT_IMM16 || reloced_a2==ARGT_UIMM16)
	  disp+=2;
	else if (reloced_a2==ARGT_IMM32 || reloced_a2==ARGT_UIMM32)
	  disp+=4;
	else if (reloced_a2==ARGT_IMM64 || reloced_a2==ARGT_UIMM64)
	  disp+=8;
      }
      CatPrint(reloced_a1_st,"[%X]",disp);
      RM1=-1;
    } else {
      if (Mod<3) {
	if (RM1&7==4) {
	  RM1=-1;
	  r1=SIB & 7+REX_b;
	  r2=SIB>>3 & 7+REX_x;
	  scale=SIB>>6 &3;
	  if (scale==3)
	    scale=8;
	  else if (scale==2)
	    scale=4;
	  else if (scale==1)
	    scale=2;
	  else
	    scale=1;
	  if (seg_size==64)
	    ptr="ST_U64_REGS";
	  else
	    ptr="ST_U32_REGS";
	  if (r1==REG_RBP && !Mod) {
	    disp=*ip(U32 *)++;
	    CatPrint(reloced_a1_st,"%08X[%Z*%d]",disp,r2,ptr,scale);
	  } else if (r2==4)
	    CatPrint(reloced_a1_st,"[%Z]",r1,ptr);
	  else
	    CatPrint(reloced_a1_st,"[%Z+%Z*%d]",r1,ptr,r2,ptr,scale);
	} else {
	  if (opadd==16)
	    ptr="ST_U16_REGS";
	  else if (opadd==32)
	    ptr="ST_U32_REGS";
	  else
	    ptr="ST_U64_REGS";
	  CatPrint(reloced_a1_st,"[%Z]",RM1,ptr);
	}
      } else {
	ptr=NULL;
	if (reloced_a1_size==8) {
	  if (REX!=-1)
	    ptr="ST_U8_REX_REGS";
	  else
	    ptr="ST_U8_REGS";
	} else if (reloced_a1_size==16)
	  ptr="ST_U16_REGS";
	else if (reloced_a1_size==32)
	  ptr="ST_U32_REGS";
	else if (reloced_a1_size==64)
	  ptr="ST_U64_REGS";
	if (ptr)
	  CatPrint(reloced_a1_st,DefineSub(RM1,ptr));
      }
    }
  }

  switch (a1) {
    case ARGT_IMM8:
    case ARGT_UIMM8:
      imm=*ip(U8 *)++;
      CatPrint(a1_st,"%02X",imm);
      if (tempi->opcode[0]==0xCD && (ptr=DefineSub(imm,"ST_INT_NAMES")))
	CatPrint(a1_st," %s",ptr);
      break;
    case ARGT_IMM16:
    case ARGT_UIMM16:
      CatPrint(a1_st,"%04X",*ip(U16 *)++);
      break;
    case ARGT_IMM32:
    case ARGT_UIMM32:
      CatPrint(a1_st,"%08X",*ip(U32 *)++);
      break;
    case ARGT_IMM64:
    case ARGT_UIMM64:
      CatPrint(a1_st,"%016X",*ip(I64 *)++);
      break;
    sub_switch_start:
      case ARGT_REL8:
	disp=*ip(I8 *)++;
	break;
      case ARGT_REL16:
	disp=*ip(I16 *)++;
	break;
      case ARGT_REL32:
	disp=*ip(I32 *)++;
	break;
    sub_switch_end:
      disp+=ip;
      if (IsDbg)
	CatPrint(a1_st,"%p ",disp);
      else if (PutSrcLink(disp,512,buf2))
	CatPrint(a1_st,"%s ",buf2);
      else
	CatPrint(a1_st,"%P ",disp);
      if (_jmp_dst) *_jmp_dst=disp;
      break;
    case ARGT_MOFFS8...ARGT_MOFFS64:
      CatPrint(a1_st,seg_overrides);
      if (a1_size==8)
	disp=*ip(U8 *)++;
      else if (opadd==16)
	disp=*ip(U16 *)++;
      else
	disp=*ip(U32 *)++;
      CatPrint(a1_st,"[%X]",disp);
      break;
    case ARGT_AL ... ARGT_DX:
    case ARGT_SS ... ARGT_ST0:
      CatPrint(a1_st,"%z",a1-ARGT_AL,
	    "AL\0AX\0EAX\0RAX\0CL\0DX\0 \0 \0SS\0DS\0ES\0FS\0GS\0CS\0ST0\0");
      break;
    case ARGT_STI:
      ip--;
      CatPrint(a1_st,"%Z",*ip++ - tempi->opcode[tempi->opcode_cnt-1],"ST_FSTK_REGS");
      break;
  }

  switch (a2) {
    case ARGT_IMM8:
    case ARGT_UIMM8:
      CatPrint(a2_st,"%02X",*ip(U8 *)++);
      break;
    case ARGT_IMM16:
    case ARGT_UIMM16:
      CatPrint(a2_st,"%04X",*ip(U16 *)++);
      break;
    case ARGT_IMM32:
    case ARGT_UIMM32:
      CatPrint(a2_st,"%08X",*ip(U32 *)++);
      break;
    case ARGT_IMM64:
    case ARGT_UIMM64:
      CatPrint(a2_st,"%016X",*ip(I64 *)++);
      break;
    case ARGT_MOFFS8...ARGT_MOFFS64:
      CatPrint(a2_st,seg_overrides);
      if (a2_size==8)
	disp=*ip(U8 *)++;
      else if (opadd==16)
	disp=*ip(U16 *)++;
      else
	disp=*ip(U32 *)++;
      CatPrint(a2_st,"[%X]",disp);
      break;
    case ARGT_AL ... ARGT_DX:
    case ARGT_SS ... ARGT_ST0:
      CatPrint(a2_st,"%z",a2-ARGT_AL,
	    "AL\0AX\0EAX\0RAX\0CL\0DX\0 \0 \0SS\0DS\0ES\0FS\0GS\0CS\0ST0\0");
      break;
    case ARGT_STI:
      ip--;
      CatPrint(a2_st,"%Z",*ip++ - tempi->opcode[tempi->opcode_cnt-1],"ST_FSTK_REGS");
      break;
  }
  if (tempi->flags&IEF_ENDING_ZERO)
    ip++;

  if (*a1_st)
    CatPrint(line1,"\t%s",a1_st);
  if (*a2_st)
    CatPrint(line1,",%s",a2_st);
  CatPrint(line1,"\n");
  CatPrint(line2,"\n");
  if (!just_ins) {
    for (i=ip-(*_ip)(I64);i<6;i++) {
      bin_data_area1[i<<1]=CH_SPACE;
      bin_data_area1[i<<1+1]=CH_SPACE;
    }
    for (i=ip-(*_ip)(I64);i<12;i++) {
      bin_data_area2[(i-6)<<1]=CH_SPACE;
      bin_data_area2[(i-6)<<1+1]=CH_SPACE;
    }
  }
  StrCpy(buf,line1);
  if (!just_ins && ip-(*_ip)(I64)>6)
    CatPrint(buf,line2);
  *_ip=ip;
}

U8 *U(U8 *ip,I64 cnt=20,I64 seg_size=64)
{//Unassembles a num of instructions.
  I64 i;
  U8 buf[1024];
  if (seg_size==16)
    PrintWarn("16-bit unassembly is not well supported.\n");
  "$$HL,1$$";
  for (i=0;i<cnt;i++) {
    Ui(buf,&ip,seg_size);
    "%s",buf;
  }
  "$$HL,0$$";
  return ip;
}

I64 Un(U8 *ip,I64 cnt=0x80,I64 seg_size=64)
{//Unassembles a num of bytes
  I64 i=0;
  U8 buf[1024],*end_ip=ip(I64)+cnt;
  if (seg_size==16)
    PrintWarn("16-bit unassembly is not well supported.\n");
  "$$HL,1$$";
  while (ip<end_ip) {
    Ui(buf,&ip,seg_size);
    "%s",buf;
    i++;
  }
  "$$HL,0$$";
  return i;
}
