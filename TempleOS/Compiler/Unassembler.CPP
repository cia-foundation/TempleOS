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

      i1=Bt(&uasm_ins64_mask,tempi1->arg1) ||
      Bt(&uasm_ins64_mask,tempi1->arg2);
      i2=Bt(&uasm_ins64_mask,tempi2->arg1) ||
      Bt(&uasm_ins64_mask,tempi2->arg2);
	if (result=i1-i2) return result;

      i1=tempi1->flags & IEF_48_REX;
      i2=tempi2->flags & IEF_48_REX;
      return i1-i2;
    }
  }
}

/*
U0 DumpUnasmIns(CInstruction *tempi)
{
  CHashOpcode *tempo=tempi(U8 *)-tempi->ins_entry_num*sizeof(CInstruction)-offset(CHashOpcode.ins);
  "%10s:%02d,%02d SV:%02d\n",tempo->str,tempi->arg1,tempi->arg2,tempi->slash_val;
}

U0 DumpUnasmTable()
{
  I64 k;
  for (k=0;k<unasm_table_entries;k++)
    DumpUnasmIns(unasm_table[k]);
}
*/

CInstruction *InstEntryFind(U8 *ip,I64 opsize)
{//Binary Search
  I64 i,j,n,m,k,a1,a2,o1,o2,oc_cnt,sv;
  CInstruction *tempi;
  i=0;
  j=unasm_table_entries-1;
  while (TRUE) {
    k=(i+j)>>1;  //binary search
    tempi=unasm_table[k];
//DumpUnasmIns(tempi);
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
	    if ((Bt(&uasm_mem_mask,a1) || Bt(&uasm_mem_mask,a2)) &&
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
	      if (!Bt(&uasm_ins64_mask,a1) && !Bt(&uasm_ins64_mask,a2) && !(tempi->flags&IEF_48_REX))
		m=1;
	    } else {
	      if (Bt(&uasm_ins64_mask,a1) || Bt(&uasm_ins64_mask,a2) || tempi->flags&IEF_48_REX)
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
	    if (!Bt(&uasm_ins64_mask,a1) && !Bt(&uasm_ins64_mask,a2) && !(tempi->flags&IEF_48_REX)&&
		!(a2==ARGT_NO &&
	    (ARGT_UIMM8<=a1<=ARGT_UIMM64 ||
		ARGT_IMM8<=a1<=ARGT_IMM64 ) ))
	      m=1;
	  } else {
	    if (Bt(&uasm_ins64_mask,a1) || Bt(&uasm_ins64_mask,a2) || tempi->flags&IEF_48_REX)
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
  return unasm_table[k];
}

U0 UnasmHashLoad()
{
  CHashTable *h=asm_hash;
  CHashOpcode *temph;
  CInstruction *tempi;
  I64 cnt=0,j,k;

  unasm_table=NULL;
  unasm_table_entries=0;
  uasm_ins64_mask=0x0880888880+1<<ARGT_ST0+1<<ARGT_STI;
  uasm_signed_arg_mask=1<<ARGT_REL8+1<<ARGT_REL16+1<<ARGT_REL32+
    1<<ARGT_IMM8+1<<ARGT_IMM16+1<<ARGT_IMM32+1<<ARGT_IMM64;
  uasm_mem_mask=1<<ARGT_M8+1<<ARGT_M16+1<<ARGT_M32+1<<ARGT_M64;

  temph=h->next_added;
  while (temph!=h) {
    if (temph->type==HTT_OPCODE)
      cnt+=temph->instruction_entry_cnt;
    temph=temph->next_added;
  }
  unasm_table_entries=cnt;

  j=0;
  unasm_table=MAlloc(cnt*sizeof(U8 *));
  temph=h->next_added;
  while (temph!=h) {
    if (temph->type==HTT_OPCODE) {
      tempi=&temph->ins;
      for (k=0;k<temph->instruction_entry_cnt;k++) {
	unasm_table[j++]=tempi;
	tempi++;
      }
      }
    temph=temph->next_added;
  }

  QSortI64(unasm_table,cnt,&InstEntriesCompare);
}

I64 StrStrBits(U8 *st)
{
  if (StrStr(st,"8"))
    return 8;
  if (StrStr(st,"16"))
    return 16;
  if (StrStr(st,"32"))
    return 32;
  if (StrStr(st,"64"))
    return 64;
  return 0;
}

U0 Ui(U8 *buf,U8 **_ip,I64 seg_size=64,I64 *_jmp_dst=NULL,Bool just_ins=FALSE)
//Unassembles one instruction
{
  I64 i,disp,imm,opsize,opadd,
	a1,a2,reloced_a1,reloced_a2,
	a1_size=0,a2_size=0,reloced_a1_size,reloced_a2_size,
	ModrM=-1,SIB=-1,scale,r1,r2,
	Mod=-1,RM1=-1,RM2=-1,REX=-1,REX_r=0,REX_x=0,REX_b=0;
  Bool cont;
  CInstruction *tempi,*tempi2;
  CHashOpcode *tempo;
  U8 *ip=*_ip,*ptr,*reloced_a1_st,*reloced_a2_st,*bin_data_area1,*bin_data_area2,
     buf2[256],line1[256],line2[256],a1_st[256],a2_st[256],seg_overrides[32];

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
    StrCat(line1,"\n");
  else
    *line1=0;

  SPrintF(line1+StrLen(line1),"%24tP ",ip);
  bin_data_area1=line1+StrLen(line1);
  for (i=0;i<6;i++) {
    SPrintF(buf2,"%02X",ip[i]);
    StrCat(line1,buf2);
  }
  StrCat(line1," ");

  SPrintF(line2,"%24tP ",ip+6);
  bin_data_area2=line2+StrLen(line2);
  for (i=6;i<12;i++) {
    SPrintF(buf2,"%02X",ip[i]);
    StrCat(line2,buf2);
  }

  *seg_overrides=0;
  cont=TRUE;
  do {
    switch (*ip++) {
      case 0x2E: if (StrLen(seg_overrides)<24) StrCat(seg_overrides,"CS:"); break;
      case 0x36: if (StrLen(seg_overrides)<24) StrCat(seg_overrides,"SS:"); break;
      case 0x3E: if (StrLen(seg_overrides)<24) StrCat(seg_overrides,"DS:"); break;
      case 0x26: if (StrLen(seg_overrides)<24) StrCat(seg_overrides,"ES:"); break;
      case 0x64: if (StrLen(seg_overrides)<24) StrCat(seg_overrides,"FS:"); break;
      case 0x65: if (StrLen(seg_overrides)<24) StrCat(seg_overrides,"GS:"); break;
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

      case 0x40:
	REX=0x40;
	break;
      case 0x41:
	REX=0x41;
	REX_b=8;
	break;
      case 0x42:
	REX=0x42;
	REX_x=8;
	break;
      case 0x43:
	REX=0x43;
	REX_b=8;
	REX_x=8;
	break;

      case 0x44:
	REX=0x44;
	REX_r=8;
	break;
      case 0x45:
	REX=0x45;
	REX_b=8;
	REX_r=8;
	break;
      case 0x46:
	REX=0x46;
	REX_x=8;
	REX_r=8;
	break;
      case 0x47:
	REX=0x47;
	REX_b=8;
	REX_x=8;
	REX_r=8;
	break;

      case 0x48:
	REX=0x48;
	opsize=64;
	break;
      case 0x49:
	REX=0x49;
	REX_b=8;
	opsize=64;
	break;
      case 0x4A:
	REX=0x4A;
	REX_x=8;
	opsize=64;
	break;
      case 0x4B:
	REX=0x4B;
	REX_b=8;
	REX_x=8;
	opsize=64;
	break;

      case 0x4C:
	REX=0x4C;
	REX_r=8;
	opsize=64;
	break;
      case 0x4D:
	REX=0x4D;
	REX_b=8;
	REX_r=8;
	opsize=64;
	break;
      case 0x4E:
	REX=0x4E;
	REX_x=8;
	REX_r=8;
	opsize=64;
	break;
      case 0x4F:
	REX=0x4F;
	REX_b=8;
	REX_x=8;
	REX_r=8;
	opsize=64;
	break;

      default:
	cont=FALSE;
    }
  } while (cont);
  ip--;


  tempi =InstEntryFind(ip,opsize);
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
  StrCat(line1,tempo->str);

  a1=tempi->arg1;
  a2=tempi->arg2;

  if (a1_size=StrStrBits(DefineSub(a1,"ST_ARG_TYPES"))) {
    if (Bt(&uasm_signed_arg_mask,a1))
      SPrintF(buf2,"I%d ",a1_size);
    else
      SPrintF(buf2,"U%d ",a1_size);
    StrCat(a1_st,buf2);
  }

  if (a2_size=StrStrBits(DefineSub(a2,"ST_ARG_TYPES"))) {
    if (Bt(&uasm_signed_arg_mask,a2))
      SPrintF(buf2,"I%d ",a2_size);
    else
      SPrintF(buf2,"U%d ",a2_size);
    StrCat(a2_st,buf2);
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
	StrCat(a1_st,DefineSub(RM1,ptr));
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
	StrCat(a2_st,DefineSub(RM1,ptr));
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

    StrCat(reloced_a1_st,seg_overrides);
    ModrM=*ip++;
    Mod=ModrM>>6 & 3;
    RM1=ModrM & 7+REX_b;
    RM2=ModrM>>3 & 7+REX_r;
    if (Mod<3 && RM1&7==4)
      SIB=*ip++;
    if (Mod==1) {
      disp=*ip(U8 *)++;
      SPrintF(buf2,"%02X",disp);
      StrCat(reloced_a1_st,buf2);
    } else if (Mod==2) {
      disp=*ip(U32 *)++;
      SPrintF(buf2,"%08X",disp);
      StrCat(reloced_a1_st,buf2);
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
	StrCat(reloced_a2_st,DefineSub(RM2,ptr));
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
      SPrintF(buf2,"[%X] ",disp);
      StrCat(reloced_a1_st,buf2);
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
	  if (r1==CREG_RBP && !Mod) {
	    disp=*ip(U32 *)++;
	    SPrintF(buf2,"%08X",disp);
	    StrCat(reloced_a1_st,buf2);
	    SPrintF(buf2,"[%Z*%d]",r2,ptr,scale);
	    StrCat(reloced_a1_st,buf2);
	  } else if (r2==4) {
	    SPrintF(buf2,"[%Z]",r1,ptr);
	    StrCat(reloced_a1_st,buf2);
	  } else {
	    SPrintF(buf2,"[%Z+%Z*%d]",r1,ptr,r2,ptr,scale);
	    StrCat(reloced_a1_st,buf2);
	  }
	} else {
	  if (opadd==16)
	    ptr="ST_U16_REGS";
	  else if (opadd==32)
	    ptr="ST_U32_REGS";
	  else
	    ptr="ST_U64_REGS";
	  SPrintF(buf2,"[%Z]",RM1,ptr);
	  StrCat(reloced_a1_st,buf2);
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
	  StrCat(reloced_a1_st,DefineSub(RM1,ptr));
      }
    }
  }

  switch (a1) {
    case ARGT_IMM8:
    case ARGT_UIMM8:
      imm=*ip(U8 *)++;
      SPrintF(buf2,"%02X",imm);
      StrCat(a1_st,buf2);
      if (tempi->opcode[0]==0xCD) {
	if (ptr=DefineSub(imm,"ST_INT_NAMES")) {
	  StrCat(a1_st," ");
	  StrCat(a1_st,ptr);
	}
      }
      break;
    case ARGT_IMM16:
    case ARGT_UIMM16:
      imm=*ip(U16 *)++;
      SPrintF(buf2,"%04X",imm);
      StrCat(a1_st,buf2);
      break;
    case ARGT_IMM32:
    case ARGT_UIMM32:
      imm=*ip(U32 *)++;
      SPrintF(buf2,"%08X",imm);
      StrCat(a1_st,buf2);
      break;
    case ARGT_IMM64:
    case ARGT_UIMM64:
      imm=*ip(I64 *)++;
      SPrintF(buf2,"%016X",imm);
      StrCat(a1_st,buf2);
      break;
    case ARGT_REL8:
      disp=*ip(I8 *)++;
      disp+=ip;
      SPrintF(buf2,"%P ",disp);
      if (_jmp_dst) *_jmp_dst=disp;
      StrCat(a1_st,buf2);
      break;
    case ARGT_REL16:
      disp=*ip(I16 *)++;
      disp+=ip;
      SPrintF(buf2,"%P ",disp);
      if (_jmp_dst) *_jmp_dst=disp;
      StrCat(a1_st,buf2);
      break;
    case ARGT_REL32:
      disp=*ip(I32 *)++;
      disp+=ip;
      SPrintF(buf2,"%P ",disp);
      if (_jmp_dst) *_jmp_dst=disp;
      StrCat(a1_st,buf2);
      break;
    case ARGT_MOFFS8...ARGT_MOFFS64:
      StrCat(a1_st,seg_overrides);
      if (a1_size==8)
	disp=*ip(U8 *)++;
      else if (opadd==16)
	disp=*ip(U16 *)++;
      else
	disp=*ip(U32 *)++;
      SPrintF(buf2,"[%X]",disp);
      StrCat(a1_st,buf2);
      break;
    case ARGT_CL:
      StrCat(a1_st,"CL");
      break;
    case ARGT_AL:
      StrCat(a1_st,"AL");
      break;
    case ARGT_AX:
      StrCat(a1_st,"AX");
      break;
    case ARGT_EAX:
      StrCat(a1_st,"EAX");
      break;
    case ARGT_RAX:
      StrCat(a1_st,"RAX");
      break;
    case ARGT_DX:
      StrCat(a1_st,"DX");
      break;
    case ARGT_SS:
      StrCat(a1_st,"SS");
      break;
    case ARGT_DS:
      StrCat(a1_st,"DS");
      break;
    case ARGT_ES:
      StrCat(a1_st,"ES");
      break;
    case ARGT_FS:
      StrCat(a1_st,"FS");
      break;
    case ARGT_GS:
      StrCat(a1_st,"GS");
      break;
    case ARGT_CS:
      StrCat(a1_st,"CS");
      break;
    case ARGT_ST0:
      StrCat(a1_st,"ST0");
      break;
    case ARGT_STI:
      ip--;
      StrCat(a1_st,DefineSub(*ip++ - tempi->opcode[tempi->opcode_cnt-1],"ST_FSTK_REGS"));
      break;
  }

  switch (a2) {
    case ARGT_IMM8:
    case ARGT_UIMM8:
      imm=*ip(U8 *)++;
      SPrintF(buf2,"%02X",imm);
      StrCat(a2_st,buf2);
      break;
    case ARGT_IMM16:
    case ARGT_UIMM16:
      imm=*ip(U16 *)++;
      SPrintF(buf2,"%04X",imm);
      StrCat(a2_st,buf2);
      break;
    case ARGT_IMM32:
    case ARGT_UIMM32:
      imm=*ip(U32 *)++;
      SPrintF(buf2,"%08X",imm);
      StrCat(a2_st,buf2);
      break;
    case ARGT_IMM64:
    case ARGT_UIMM64:
      imm=*ip(I64 *)++;
      SPrintF(buf2,"%016X",imm);
      StrCat(a2_st,buf2);
      break;
    case ARGT_MOFFS8...ARGT_MOFFS64:
      StrCat(a2_st,seg_overrides);
      if (a2_size==8)
	disp=*ip(U8 *)++;
      else if (opadd==16)
	disp=*ip(U16 *)++;
      else
	disp=*ip(U32 *)++;
      SPrintF(buf2,"[%X]",disp);
      StrCat(a2_st,buf2);
      break;
    case ARGT_CL:
      StrCat(a2_st,"CL");
      break;
    case ARGT_AL:
      StrCat(a2_st,"AL");
      break;
    case ARGT_AX:
      StrCat(a2_st,"AX");
      break;
    case ARGT_EAX:
      StrCat(a2_st,"EAX");
      break;
    case ARGT_RAX:
      StrCat(a2_st,"RAX");
      break;
    case ARGT_DX:
      StrCat(a2_st,"DX");
      break;
    case ARGT_SS:
      StrCat(a2_st,"SS");
      break;
    case ARGT_DS:
      StrCat(a2_st,"DS");
      break;
    case ARGT_ES:
      StrCat(a2_st,"ES");
      break;
    case ARGT_FS:
      StrCat(a2_st,"FS");
      break;
    case ARGT_GS:
      StrCat(a2_st,"GS");
      break;
    case ARGT_CS:
      StrCat(a2_st,"CS");
      break;
    case ARGT_ST0:
      StrCat(a2_st,"ST0");
      break;
    case ARGT_STI:
      ip--;
      StrCat(a2_st,DefineSub(*ip++ - tempi->opcode[tempi->opcode_cnt-1],"ST_FSTK_REGS"));
      break;
  }
  if (tempi->flags&IEF_ENDING_ZERO)
    ip++;

  if (*a1_st) {
    StrCat(line1,"\t");
    StrCat(line1,a1_st);
  }
  if (*a2_st) {
    StrCat(line1,",");
    StrCat(line1,a2_st);
  }
  StrCat(line1,"\n");
  StrCat(line2,"\n");
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
    StrCat(buf,line2);
  *_ip=ip;
}


U8 *U(U8 *ip,I64 cnt=20,I64 seg_size=64)
//Unassembles a num of instructions.
{
  I64 i;
  U8 buf[1024];
  if (seg_size==16)
    PrintFWarn("%S","ST_16BIT_UNASM");
  "$$HL,1$$";
  for (i=0;i<cnt;i++) {
    Ui(buf,&ip,seg_size);
    "" buf;
  }
  "$$HL,0$$";
  return ip;
}

I64 Un(U8 *ip,I64 cnt=0x80,I64 seg_size=64)
//Unassembles a num of bytes
{
  I64 i=0;
  U8 buf[1024],*end_ip=ip(I64)+cnt;
  if (seg_size==16)
    PrintFWarn("%S","ST_16BIT_UNASM");
  "$$HL,1$$";
  while (ip<end_ip) {
    Ui(buf,&ip,seg_size);
    "" buf;
    i++;
  }
  "$$HL,0$$";
  return i;
}
