U0 AOTStoreCodeU8(CCmpCtrl *cc,U8 b)
{
  CAOTCtrl *aotc=cc->aotc;
  I64 i=aotc->ip>>AOT_BIN_BLK_BITS;
  CAOTBinBlk *tempbin=aotc->bin;
  while (i--)
    if (!(tempbin=tempbin->next)) {
      i=aotc->ip>>AOT_BIN_BLK_BITS+1;
      tempbin=&aotc->bin;
      while (i--) {
	if (!tempbin->next)
	  tempbin->next=CAlloc(sizeof(CAOTBinBlk));
	tempbin=tempbin->next;
      }
      break;
    }
  tempbin->body[aotc->ip++ & (AOT_BIN_BLK_SIZE-1)]=b;
  if (aotc->lst) {
    if (aotc->lst_col>=24) {
      '\n';
      aotc->lst_col=0;
    }
    "%02X ",b;
    aotc->lst_col+=3;
  }
  if (aotc->ip>aotc->num_bin_U8s)
    aotc->num_bin_U8s=aotc->ip;
}

U0 AOTStoreCodeU32(CCmpCtrl *cc,U32 d)
{
  CAOTCtrl *aotc=cc->aotc;
  I64 i=aotc->ip>>AOT_BIN_BLK_BITS,j=(aotc->ip+sizeof(U32))>>AOT_BIN_BLK_BITS;
  U32 *_d;
  CAOTBinBlk *tempbin;
  if (i!=j)
    for (i=0;i<sizeof(U32);i++)
      AOTStoreCodeU8(cc,d.u8[i]);
  else {
    tempbin=aotc->bin;
    while (i--)
      if (!(tempbin=tempbin->next)) {
	i=aotc->ip>>AOT_BIN_BLK_BITS+1;
	tempbin=&aotc->bin;
	while (i--) {
	  if (!tempbin->next)
	    tempbin->next=CAlloc(sizeof(CAOTBinBlk));
	  tempbin=tempbin->next;
	}
	break;
      }
    _d=tempbin->body+ aotc->ip&(AOT_BIN_BLK_SIZE-1);
    *_d=d;
    aotc->ip+=sizeof(U32);
    if (aotc->lst) {
      if (aotc->lst_col>=24) {
	'\n';
	aotc->lst_col=0;
      }
      "%08X ",d;
      aotc->lst_col+=9;
    }
    if (aotc->ip>aotc->num_bin_U8s)
      aotc->num_bin_U8s=aotc->ip;
  }
}

U0 AOTStoreCodeU64(CCmpCtrl *cc,U64 q)
{
  CAOTCtrl *aotc=cc->aotc;
  I64 i=aotc->ip>>AOT_BIN_BLK_BITS,j=(aotc->ip+sizeof(I64))>>AOT_BIN_BLK_BITS,
	*_q;
  CAOTBinBlk *tempbin;
  if (i!=j)
    for (i=0;i<sizeof(I64);i++)
      AOTStoreCodeU8(cc,q.u8[i]);
  else {
    tempbin=aotc->bin;
    while (i--)
      if (!(tempbin=tempbin->next)) {
	i=aotc->ip>>AOT_BIN_BLK_BITS+1;
	tempbin=&aotc->bin;
	while (i--) {
	  if (!tempbin->next)
	    tempbin->next=CAlloc(sizeof(CAOTBinBlk));
	  tempbin=tempbin->next;
	}
	break;
      }
    _q=tempbin->body+ aotc->ip&(AOT_BIN_BLK_SIZE-1);
    *_q=q;
    aotc->ip+=sizeof(I64);
    if (aotc->lst) {
      if (aotc->lst_col>=24) {
	'\n';
	aotc->lst_col=0;
      }
      "%016X ",q;
      aotc->lst_col+=17;
    }
    if (aotc->ip>aotc->num_bin_U8s)
      aotc->num_bin_U8s=aotc->ip;
  }
}

U0 AOTStoreCodeU8At(CCmpCtrl *cc,I64 ip,...)
{
  CAOTCtrl *aotc=cc->aotc;
  CAOTBinBlk *tempbin;
  I64 i,j;
  for (j=0;j<argc;j++) {
    i=ip>>AOT_BIN_BLK_BITS+1;
    tempbin=&aotc->bin;
    while (i--) {
      if (!tempbin->next) {
	tempbin->next=CAlloc(sizeof(CAOTBinBlk));
	tempbin->next->next=NULL;
      }
      tempbin=tempbin->next;
    }
    tempbin->body[ip++ & (AOT_BIN_BLK_SIZE-1)]=argv[j];
  }
  if (ip>aotc->num_bin_U8s)
    aotc->num_bin_U8s=ip;
}

I64 AsmLexExpression(CCmpCtrl *cc)
{
  U8 *machine_code;
  I64 result=0;
  cc->asm_undef_hash=NULL;
  cc->flags&=~(CCF_UNRESOLVED+CCF_LOCAL);
  cc->abs_cnts=0;
  if (!IsLexExpression2Bin(cc,&machine_code))
    LexSkipEol(cc);
  else {
    if (cc->abs_cnts.externs)
      LexExcept(cc,"Extern Not Allowed at ");
    if (cc->flags & CCF_UNRESOLVED)
      LexExcept(cc,"Undefined sym at ");
    result=Call(machine_code);
    Free(machine_code);
  }
  return result;
}

U0 AsmLineLst(CCmpCtrl *cc)
{
  CAOTCtrl *aotc=cc->aotc;
  CLexFile *tempf=cc->lex_include_stk;
  CDocEntry *doc_e=tempf->line_start,*doc_e2;
  if (aotc->lst && aotc->lst_last_lfn==tempf && doc_e) {
    if (doc_e2=aotc->lst_last_line) {
      if (tempf->flags& LFSF_DOC) {
	while (doc_e2->y<doc_e->y) {
	  "%h*c%08X ",24-aotc->lst_col,CH_SPACE,aotc->lst_last_ip;
	  aotc->lst_col=0;
	  doc_e2=LexPutLine(cc,doc_e2);
	}
      } else {
	while (*doc_e2(U8 *) && doc_e2!=doc_e) {
	  "%h*c%08X ",24-aotc->lst_col,CH_SPACE,aotc->lst_last_ip;
	  aotc->lst_col=0;
	  doc_e2=LexPutLine(cc,doc_e2);
	}
      }
      aotc->lst_last_line=doc_e2;
    } else
      aotc->lst_last_line=doc_e;
  } else {
    aotc->lst_last_line=NULL;
    aotc->lst_last_lfn=tempf;
  }
  aotc->lst_last_ip=aotc->ip;
}
