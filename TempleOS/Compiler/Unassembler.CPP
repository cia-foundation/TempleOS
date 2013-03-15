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

U0 Ui(U8 *buf,U8 **_ip,I64 seg_size=64,I64 *_jmp_dst=NULL,Bool just_ins=FALSE)
//Unassembles one instruction
{
  I64 disp,imm,opsize,opadd,
      i,a1,a2,aa1,aa2,arg1_size=0,arg2_size=0,a1_size,a2_size,
      ModrM=-1,SIB=-1,scale,r1,r2,
      Mod=-1,RM1=-1,RM2=-1,REX=-1,REX_r=0,REX_x=0,REX_b=0;
  Bool cont;
  CInstruction *tempi,*tempi2;
  CHashOpcode *tempo;
  U8 *ip=*_ip,*ptr,*a1_st,*a2_st,*bin_data_area,*bin_data_area2,
      buf2[256],buf3[256],seg_overrides[40],arg1_st[256],arg2_st[256];

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
  *arg1_st=0;
  *arg2_st=0;
  a1_st=arg1_st;
  a2_st=arg2_st;

  if (!IsRaw && PutSrcLink(ip,1,buf))
    StrCat(buf,"\n");
  else
    *buf=0;

  SPrintF(buf+StrLen(buf),"%24tP ",ip);
  bin_data_area=buf+StrLen(buf);
  for (i=0;i<6;i++) {
    SPrintF(buf2,"%02X",ip[i]);
    StrCat(buf,buf2);
  }
  StrCat(buf," ");

  SPrintF(buf3,"%24tP ",ip+6);
  bin_data_area2=buf3+StrLen(buf3);
  for (i=6;i<12;i++) {
    SPrintF(buf2,"%02X",ip[i]);
    StrCat(buf3,buf2);
  }

  *seg_overrides=0;
  cont=TRUE;
  do {
    switch (*ip++) {
      case 0x2E: StrCat(seg_overrides,"CS:"); break;
      case 0x36: StrCat(seg_overrides,"SS:"); break;
      case 0x3E: StrCat(seg_overrides,"DS:"); break;
      case 0x26: StrCat(seg_overrides,"ES:"); break;
      case 0x64: StrCat(seg_overrides,"FS:"); break;
      case 0x65: StrCat(seg_overrides,"GS:"); break;
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
    *buf=0;
  StrCat(buf,tempo->str);

  a1=tempi->arg1;
  a2=tempi->arg2;
  aa1=a1;
  aa2=a2;

  ptr=DefineSub(a1,"ST_ARG_TYPES");
  if (Bt(&uasm_signed_arg_mask,a1)) {
    if (StrStr(ptr,"8")) {
      arg1_size=1;
      StrCat(arg1_st,"I8 ");
    } else if (StrStr(ptr,"16")) {
      arg1_size=2;
      StrCat(arg1_st,"I16 ");
    } else if (StrStr(ptr,"32")) {
      arg1_size=4;
      StrCat(arg1_st,"I32 ");
    } else if (StrStr(ptr,"64")) {
      arg1_size=8;
      StrCat(arg1_st,"I64 ");
    }
  } else {
    if (StrStr(ptr,"8")) {
      arg1_size=1;
      StrCat(arg1_st,"U8 ");
    } else if (StrStr(ptr,"16")) {
      arg1_size=2;
      StrCat(arg1_st,"U16 ");
    } else if (StrStr(ptr,"32")) {
      arg1_size=4;
      StrCat(arg1_st,"U32 ");
    } else if (StrStr(ptr,"64")) {
      arg1_size=8;
      StrCat(arg1_st,"U64 ");
    }
  }

  ptr=DefineSub(a2,"ST_ARG_TYPES");
  if (Bt(&uasm_signed_arg_mask,a2)) {
    if (StrStr(ptr,"8")) {
      arg2_size=1;
      StrCat(arg2_st,"I8 ");
    } else if (StrStr(ptr,"16")) {
      arg2_size=2;
      StrCat(arg2_st,"I16 ");
    } else if (StrStr(ptr,"32")) {
      arg2_size=4;
      StrCat(arg2_st,"I32 ");
    } else if (StrStr(ptr,"64")) {
      arg2_size=8;
      StrCat(arg2_st,"I64 ");
    }
  } else {
    if (StrStr(ptr,"8")) {
      arg2_size=1;
      StrCat(arg2_st,"U8 ");
    } else if (StrStr(ptr,"16")) {
      arg2_size=2;
      StrCat(arg2_st,"U16 ");
    } else if (StrStr(ptr,"32")) {
      arg2_size=4;
      StrCat(arg2_st,"U32 ");
    } else if (StrStr(ptr,"64")) {
      arg2_size=8;
      StrCat(arg2_st,"U64 ");
    }
  }

  a1_size=arg1_size;
  a2_size=arg2_size;
  if (tempi->flags & IEF_PLUS_OPCODE) {
    ip--;
    RM1=*ip++ - tempi->opcode[tempi->opcode_cnt-1]+REX_b;
    *buf2=0;
    if (ARGT_R8<=a1<=ARGT_R64) {
      if (a1_size==1) {
	if (REX!=-1)
	  SPrintF(buf2,"%Z",RM1,"ST_U8_REX_REGS");
	else
	  SPrintF(buf2,"%Z",RM1,"ST_U8_REGS");
      } else if (a1_size==2)
	SPrintF(buf2,"%Z",RM1,"ST_U16_REGS");
      else if (a1_size==4)
	SPrintF(buf2,"%Z",RM1,"ST_U32_REGS");
      else if (a1_size==8)
	SPrintF(buf2,"%Z",RM1,"ST_U64_REGS");
      StrCat(a1_st,buf2);
    } else {
      if (a2_size==1) {
	if (REX!=-1)
	  SPrintF(buf2,"%Z",RM1,"ST_U8_REX_REGS");
	else
	  SPrintF(buf2,"%Z",RM1,"ST_U8_REGS");
      } else if (a2_size==2)
	SPrintF(buf2,"%Z",RM1,"ST_U16_REGS");
      else if (a2_size==4)
	SPrintF(buf2,"%Z",RM1,"ST_U32_REGS");
      else if (a2_size==8)
	SPrintF(buf2,"%Z",RM1,"ST_U64_REGS");
      StrCat(a2_st,buf2);
    }
  }
  if (ARGT_RM8<=a1<=ARGT_RM64 ||
      ARGT_M8<=a1<=ARGT_M64 ||
      ARGT_RM8<=a2<=ARGT_RM64 ||
      ARGT_M8<=a2<=ARGT_M64) {
    if (ARGT_RM8<=a2<=ARGT_RM64 ||
	ARGT_M8<=a2<=ARGT_M64) {
      a1_st=arg2_st;
      a2_st=arg1_st;
      a1_size=arg2_size;
      a2_size=arg1_size;
      aa1=a2;
      aa2=a1;
    }
    StrCat(a1_st,seg_overrides);
    ModrM=*ip++;
    Mod=ModrM>>6 & 3;
    RM1=ModrM & 7+REX_b;
    RM2=ModrM>>3 & 7+REX_r;
    if (Mod<3 && RM1&7==4) {
      SIB=*ip++;
    }
    if (Mod==1) {
      disp=*ip(U8 *)++;
      SPrintF(buf2,"%02X",disp);
      StrCat(a1_st,buf2);
    }
    if (Mod==2) {
      disp=*ip(U32 *)++;
      SPrintF(buf2,"%08X",disp);
      StrCat(a1_st,buf2);
    }
    if (tempi->slash_val<8)
      RM2=-1;
    else {
      *buf2=0;
      if (aa2==ARGT_SREG)
	SPrintF(buf2,"%Z",RM2,"ST_SEG_REGS");
      else if (!(ARGT_IMM8<=aa2<=ARGT_IMM64) &&
      !(ARGT_UIMM8<=aa2<=ARGT_UIMM64)) {
	if (a2_size==1) {
	  if (REX!=-1)
	    SPrintF(buf2,"%Z",RM2,"ST_U8_REX_REGS");
	  else
	    SPrintF(buf2,"%Z",RM2,"ST_U8_REGS");
	}else if (a2_size==2)
	  SPrintF(buf2,"%Z",RM2,"ST_U16_REGS");
	else if (a2_size==4)
	  SPrintF(buf2,"%Z",RM2,"ST_U32_REGS");
	else if (a2_size==8)
	  SPrintF(buf2,"%Z",RM2,"ST_U64_REGS");
      }
      StrCat(a2_st,buf2);
    }
    if (RM1&7==5 && !Mod) {
      disp=*ip(I32 *)++;
      if (seg_size==64) {
	disp+=ip;
	if (aa2==ARGT_IMM8 || aa2==ARGT_UIMM8)
	  disp++;
	else if (aa2==ARGT_IMM16 || aa2==ARGT_UIMM16)
	  disp+=2;
	else if (aa2==ARGT_IMM32 || aa2==ARGT_UIMM32)
	  disp+=4;
	else if (aa2==ARGT_IMM64 || aa2==ARGT_UIMM64)
	  disp+=8;
      }
      SPrintF(buf2,"[%X] ",disp);
      StrCat(a1_st,buf2);
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
	  if (r1==CREG_RBP && !Mod) {
	    disp=*ip(U32 *)++;
	    SPrintF(buf2,"%08X",disp);
	    StrCat(a1_st,buf2);
	    if (seg_size!=64)
	      SPrintF(buf2,"[%Z*%d]",r2,"ST_U32_REGS",scale);
	    else
	      SPrintF(buf2,"[%Z*%d]",r2,"ST_U64_REGS",scale);
	    StrCat(a1_st,buf2);
	  } else if (r2==4) {
	    if (seg_size!=64)
	      SPrintF(buf2,"[%Z]",r1,"ST_U32_REGS");
	    else
	      SPrintF(buf2,"[%Z]",r1,"ST_U64_REGS");
	    StrCat(a1_st,buf2);
	  } else {
	    if (seg_size!=64)
	      SPrintF(buf2,"[%Z+%Z*%d]",r1,"ST_U32_REGS",
		  r2,"ST_U32_REGS",scale);
	    else
	      SPrintF(buf2,"[%Z+%Z*%d]",r1,"ST_U64_REGS",
		  r2,"ST_U64_REGS",scale);
	    StrCat(a1_st,buf2);
	  }
	} else {
	  *buf2=0;
	  if (opadd==16)
	    SPrintF(buf2,"[%Z]",RM1,"ST_U16_REGS");
	  else if (opadd==32)
	    SPrintF(buf2,"[%Z]",RM1,"ST_U32_REGS");
	  else
	    SPrintF(buf2,"[%Z]",RM1,"ST_U64_REGS");
	  StrCat(a1_st,buf2);
	}
      } else {
	*buf2=0;
	if (a1_size==1) {
	  if (REX!=-1)
	    SPrintF(buf2,"%Z",RM1,"ST_U8_REX_REGS");
	  else
	    SPrintF(buf2,"%Z",RM1,"ST_U8_REGS");
	} else if (a1_size==2)
	  SPrintF(buf2,"%Z",RM1,"ST_U16_REGS");
	else if (a1_size==4)
	  SPrintF(buf2,"%Z",RM1,"ST_U32_REGS");
	else if (a1_size==8)
	  SPrintF(buf2,"%Z",RM1,"ST_U64_REGS");
	StrCat(a1_st,buf2);
      }
    }
  }
  if (ARGT_MOFFS8<=a1<=ARGT_MOFFS64) {
    StrCat(arg1_st,seg_overrides);
    if (arg1_size==1) {
      disp=*ip(U8 *)++;
      SPrintF(buf2,"[%X]",disp);
      StrCat(arg1_st,buf2);
    } else if (opadd==16) {
      disp=*ip(U16 *)++;
      SPrintF(buf2,"[%X]",disp);
      StrCat(arg1_st,buf2);
    } else {
      disp=*ip(U32 *)++;
      SPrintF(buf2,"[%X]",disp);
      StrCat(arg1_st,buf2);
    }
  }

  if (ARGT_MOFFS8<=a2<=ARGT_MOFFS64) {
    StrCat(arg2_st,seg_overrides);
    if (arg2_size==1) {
      disp=*ip(U8 *)++;
      SPrintF(buf2,"[%X]",disp);
      StrCat(arg2_st,buf2);
    } else if (opadd==16) {
      disp=*ip(U16 *)++;
      SPrintF(buf2,"[%X]",disp);
      StrCat(arg2_st,buf2);
    } else {
      disp=*ip(U32 *)++;
      SPrintF(buf2,"[%X]",disp);
      StrCat(arg2_st,buf2);
    }
  }

  switch (a1) {
    case ARGT_IMM8:
    case ARGT_UIMM8:
      imm=*ip(U8 *)++;
      SPrintF(buf2,"%02X",imm);
      StrCat(arg1_st,buf2);
      if (tempi->opcode[0]==0xCD) {
	if (ptr=DefineSub(imm,"ST_INT_NAMES")) {
	  StrCat(arg1_st," ");
	  StrCat(arg1_st,ptr);
	}
      }
      break;
    case ARGT_IMM16:
    case ARGT_UIMM16:
      imm=*ip(U16 *)++;
      SPrintF(buf2,"%04X",imm);
      StrCat(arg1_st,buf2);
      break;
    case ARGT_IMM32:
    case ARGT_UIMM32:
      imm=*ip(U32 *)++;
      SPrintF(buf2,"%08X",imm);
      StrCat(arg1_st,buf2);
      break;
    case ARGT_IMM64:
    case ARGT_UIMM64:
      imm=*ip(I64 *)++;
      SPrintF(buf2,"%016X",imm);
      StrCat(arg1_st,buf2);
      break;
    case ARGT_REL8:
      disp=*ip(I8 *)++;
      disp+=ip;
      SPrintF(buf2,"%P ",disp);
      if (_jmp_dst) *_jmp_dst=disp;
      StrCat(arg1_st,buf2);
      break;
    case ARGT_REL16:
      disp=*ip(I16 *)++;
      disp+=ip;
      SPrintF(buf2,"%P ",disp);
      if (_jmp_dst) *_jmp_dst=disp;
      StrCat(arg1_st,buf2);
      break;
    case ARGT_REL32:
      disp=*ip(I32 *)++;
      disp+=ip;
      SPrintF(buf2,"%P ",disp);
      if (_jmp_dst) *_jmp_dst=disp;
      StrCat(arg1_st,buf2);
      break;
    case ARGT_CL:
      StrCat(arg1_st,"CL");
      break;
    case ARGT_AL:
      StrCat(arg1_st,"AL");
      break;
    case ARGT_AX:
      StrCat(arg1_st,"AX");
      break;
    case ARGT_EAX:
      StrCat(arg1_st,"EAX");
      break;
    case ARGT_RAX:
      StrCat(arg1_st,"RAX");
      break;
    case ARGT_DX:
      StrCat(arg1_st,"DX");
      break;
    case ARGT_SS:
      StrCat(arg1_st,"SS");
      break;
    case ARGT_DS:
      StrCat(arg1_st,"DS");
      break;
    case ARGT_ES:
      StrCat(arg1_st,"ES");
      break;
    case ARGT_FS:
      StrCat(arg1_st,"FS");
      break;
    case ARGT_GS:
      StrCat(arg1_st,"GS");
      break;
    case ARGT_CS:
      StrCat(arg1_st,"CS");
      break;
    case ARGT_ST0:
      StrCat(arg1_st,"ST0");
      break;
    case ARGT_STI:
      ip--;
      SPrintF(buf2,"%Z",*ip++ - tempi->opcode[tempi->opcode_cnt-1],"ST_FSTK_REGS");
      StrCat(arg1_st,buf2);
      break;
  }

  switch (a2) {
    case ARGT_IMM8:
    case ARGT_UIMM8:
      imm=*ip(U8 *)++;
      SPrintF(buf2,"%02X",imm);
      StrCat(arg2_st,buf2);
      break;
    case ARGT_IMM16:
    case ARGT_UIMM16:
      imm=*ip(U16 *)++;
      SPrintF(buf2,"%04X",imm);
      StrCat(arg2_st,buf2);
      break;
    case ARGT_IMM32:
    case ARGT_UIMM32:
      imm=*ip(U32 *)++;
      SPrintF(buf2,"%08X",imm);
      StrCat(arg2_st,buf2);
      break;
    case ARGT_IMM64:
    case ARGT_UIMM64:
      imm=*ip(I64 *)++;
      SPrintF(buf2,"%016X",imm);
      StrCat(arg2_st,buf2);
      break;
    case ARGT_CL:
      StrCat(arg2_st,"CL");
      break;
    case ARGT_AL:
      StrCat(arg2_st,"AL");
      break;
    case ARGT_AX:
      StrCat(arg2_st,"AX");
      break;
    case ARGT_EAX:
      StrCat(arg2_st,"EAX");
      break;
    case ARGT_RAX:
      StrCat(arg2_st,"RAX");
      break;
    case ARGT_DX:
      StrCat(arg2_st,"DX");
      break;
    case ARGT_SS:
      StrCat(arg2_st,"SS");
      break;
    case ARGT_DS:
      StrCat(arg2_st,"DS");
      break;
    case ARGT_ES:
      StrCat(arg2_st,"ES");
      break;
    case ARGT_FS:
      StrCat(arg2_st,"FS");
      break;
    case ARGT_GS:
      StrCat(arg2_st,"GS");
      break;
    case ARGT_CS:
      StrCat(arg2_st,"CS");
      break;
    case ARGT_ST0:
      StrCat(arg2_st,"ST0");
      break;
    case ARGT_STI:
      ip--;
      SPrintF(buf2,"%Z",*ip++ - tempi->opcode[tempi->opcode_cnt-1],"ST_FSTK_REGS");
      StrCat(arg2_st,buf2);
      break;
  }
  if (tempi->flags&IEF_ENDING_ZERO)
    ip++;

  if (*arg1_st) {
    StrCat(buf,"\t");
    StrCat(buf,arg1_st);
  }
  if (*arg2_st) {
    StrCat(buf,",");
    StrCat(buf,arg2_st);
  }
  if (!just_ins) {
    for (i=ip-(*_ip)(I64);i<6;i++) {
      bin_data_area[i<<1]=CH_SPACE;
      bin_data_area[i<<1+1]=CH_SPACE;
    }
    for (i=ip-(*_ip)(I64);i<12;i++) {
      bin_data_area2[(i-6)<<1]=CH_SPACE;
      bin_data_area2[(i-6)<<1+1]=CH_SPACE;
    }
  }
  StrCat(buf,"\n");
  if (!just_ins && ip-(*_ip)(I64)>6) {
    StrCat(buf,buf3);
    StrCat(buf,"\n");
  }
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
