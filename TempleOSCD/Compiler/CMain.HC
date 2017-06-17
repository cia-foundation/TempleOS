U8 *LexStmt2Bin(CCmpCtrl *cc,I64 *_type,I64 cmp_flags=0)
{//Compile one cc stmt to bin code.
  I64 size,i,j,k,*res=INVALID_PTR;
  CCodeCtrl *tmpcbh;
  if (_type) *_type=RT_I64;
  Btr(&cc->flags,CCf_PASS_TRACE_PRESENT);
  if (cc->aot_depth==2)
    COCPush(cc);
  COCInit(cc);
  if (!PrsStmt(cc,,,cmp_flags)) {
    if (cc->coc.coc_head.next!=&cc->coc.coc_head) {
      cc->coc.coc_head.last->ic_flags&=~ICF_RES_NOT_USED;
      ICAdd(cc,IC_RETURN_VAL2,0,0);
      ICAdd(cc,IC_RET,0,0);
      if (res=COCCompile(cc,&size,NULL,_type)) {
	if (cc->flags&CCF_AOT_COMPILE) {
	  j=cc->aotc->rip;
	  k=(size+7)>>3;
	  for (i=0;i<k;i++)
	    AOTStoreCodeU64(cc,res[i]);
	  Free(res);
	  res=j;
	}
      }
    } //TODO: else del misc?
  } else //TODO: too dangerous to del Misc?
    QueDel(&cc->coc.coc_head.next);
  if (cc->aot_depth==2) {
    tmpcbh=COCPopNoFree(cc);
    COCAppend(cc,tmpcbh);
  }
  return res;
}

CAOT *CmpJoin(CCmpCtrl *cc,I64 cmp_flags,U8 *map_name=NULL,U8 mapfile_drv_let=0)
{
  CAOTCtrl *aotc,*old_aot=cc->aotc;
  I64 i,j,l;
  U8 *buf;
  CAOTBinBlk *tmpbin;
  CAOTImportExport *tmpie;
  Bool okay=TRUE;
  CLexHashTableContext *htc=MAlloc(sizeof(CLexHashTableContext));
  CAOT *res=CAlloc(sizeof(CAOT)),*parent;
  if (parent=cc->aot) {
    res->parent_aot=parent;
    QueIns(res,parent->last);
  } else
    QueInit(res);
  cc->aot=res;

  res->next_ie=res->last_ie=&res->next_ie;
  cc->aotc=aotc=CAlloc(sizeof(CAOTCtrl));
  cc->aot_depth++;

  aotc->bin=CAlloc(sizeof(CAOTBinBlk));
  aotc->max_align_bits=0;
  aotc->org=INVALID_PTR;

  MemCpy(htc,&cc->htc,sizeof(CLexHashTableContext));
  if (cc->htc.fun)
    cc->htc.glbl_hash_table=HashTableNew(128);
  else
    cc->htc.glbl_hash_table=HashTableNew(1024);
  if (cc->flags&CCF_AOT_COMPILE) {
    cc->htc.define_hash_table=cc->htc.glbl_hash_table;
    if (cc->aot_depth<=1)
      cc->htc.glbl_hash_table->next=cmp.asm_hash;
    else
      cc->htc.glbl_hash_table->next=htc->glbl_hash_table;
  } else
    cc->htc.glbl_hash_table->next=Fs->hash_table;
  cc->htc.hash_table_lst=cc->htc.local_hash_table=HashTableNew(16);
  cc->htc.local_hash_table->next=cc->htc.glbl_hash_table;
  cc->htc.local_var_lst=cc->htc.fun; //HolyC local vars
  cc->htc.fun=NULL;
  try {
    if (cmp_flags&CMPF_LEX_FIRST)
      Lex(cc);
    if (!(cmp_flags&CMPF_ONE_ASM_INS))
      cmp_flags|=CMPF_PRS_SEMICOLON;
    if (cc->flags&CCF_AOT_COMPILE) {
      while (cc->token!=TK_EOF) {
	buf=LexStmt2Bin(cc,NULL,cmp_flags);
	if (buf!=INVALID_PTR) {
	  tmpie=CAlloc(sizeof(CAOTImportExport));
	  tmpie->type=IET_MAIN;
	  tmpie->rip=buf;
	  QueIns(tmpie,res->last_ie);
	}
	if (cmp_flags&CMPF_ASM_BLK)
	  break;
      }
    } else
      PrsStmt(cc,,,cmp_flags);
    AOTGlblsResolve(cc,res);
  } catch {
    if (Fs->except_ch=='Compiler' && !(cmp_flags&CMPF_ASM_BLK)) {
      LexPutPos(cc);
      Fs->catch_except=TRUE;
    }
    okay=FALSE;
  }
  if (!okay) {
    if (cc->error_cnt<1)
      cc->error_cnt=1;
    cc->aot=res->parent_aot;
    Free(res);
    LinkedLstDel(aotc->bin);
    res=NULL;
  } else {
    if (map_name)
      MapFileWrite(cc->htc.glbl_hash_table,map_name,mapfile_drv_let);
    HashTableDel(cc->htc.local_hash_table);
    HashTableDel(cc->htc.glbl_hash_table);

    if (!aotc->num_bin_U8s)
      res->buf=NULL;
    else {
      if (cc->flags&CCF_AOT_COMPILE)
	res->buf=MAlloc(aotc->num_bin_U8s);
      else {
	if (aotc->org==INVALID_PTR)
	  res->buf=MAlloc(aotc->num_bin_U8s,Fs->code_heap);
	else
	  res->buf=aotc->org;
      }
      res->aot_U8s=aotc->num_bin_U8s;
      tmpbin=aotc->bin;
      j=0;
      l=aotc->num_bin_U8s;
      while (tmpbin) {
	i=l;
	if (i>AOT_BIN_BLK_SIZE)
	  i=AOT_BIN_BLK_SIZE;
	MemCpy(res->buf+j,tmpbin->body,i);
	j+=i;
	l-=i;
	tmpbin=tmpbin->next;
      }
    }
    LinkedLstDel(aotc->bin);
    res->abss=aotc->abss;
    res->heap_glbls=aotc->heap_glbls;
    res->max_align_bits=aotc->max_align_bits;
    res->org=aotc->org;
  }
  cc->aot=parent;
  MemCpy(&cc->htc,htc,sizeof(CLexHashTableContext));
  Free(htc);
  Free(aotc);
  cc->aotc=old_aot;
  cc->aot_depth--;
  return res;
}

CAOT *CmpBuf(U8 *buf,U8 *map_name=NULL,
  I64 *error_cnt=NULL, I64 *warning_cnt=NULL,U8 mapfile_drv_let=0)
{
  CCmpCtrl *cc;
  CAOT *res=NULL;
  cc=CmpCtrlNew(buf,CCF_DONT_FREE_BUF);
  cc->flags|=CCF_AOT_COMPILE;
  QueIns(cc,Fs->last_cc);
  res=CmpJoin(cc,CMPF_LEX_FIRST,map_name,mapfile_drv_let);
  if (error_cnt)   *error_cnt=cc->error_cnt;
  if (warning_cnt) *warning_cnt=cc->warning_cnt;
  QueRem(cc);
  if (res)
    CmpCtrlDel(cc);
  return res;
}

U0 CmpFixUpJITAsm(CCmpCtrl *cc,CAOT *tmpaot)
{
  I64 i,rip2=tmpaot->buf+tmpaot->rip,*str=NULL;
  U8 *ptr;
  CCodeMisc *g_lb;
  CAOTAbsAddr *tmpa,*tmpa1;
  CAOTImportExport *tmpie,*tmpie1;
  CHashExport *tmpex;

  tmpa=tmpaot->abss;
  while (tmpa) {
    tmpa1=tmpa->next;
    ptr=rip2+tmpa->rip;
    switch [tmpa->type] {
      case AAT_ADD_U8:	*ptr(U8 *) +=rip2; break;
      case AAT_SUB_U8:	*ptr(U8 *) -=rip2; break;
      case AAT_ADD_U16:	*ptr(U16 *)+=rip2; break;
      case AAT_SUB_U16:	*ptr(U16 *)-=rip2; break;
      case AAT_ADD_U32:	*ptr(U32 *)+=rip2; break;
      case AAT_SUB_U32:	*ptr(U32 *)-=rip2; break;
      case AAT_ADD_U64:	*ptr(I64 *)+=rip2; break;
      case AAT_SUB_U64:	*ptr(I64 *)-=rip2; break;
    }
    Free(tmpa);
    tmpa=tmpa1;
  }
  tmpie=tmpaot->next_ie;
  while (tmpie!=&tmpaot->next_ie) {
    tmpie1=tmpie->next;
    if (tmpie->str) {
      Free(str);
      str=tmpie->str;
    }
    switch (tmpie->type) {
      case IET_REL32_EXPORT:
      case IET_IMM32_EXPORT:
      case IET_REL64_EXPORT:
      case IET_IMM64_EXPORT:
	tmpex=CAlloc(sizeof(CHashExport));
	tmpex->str=str;
	str=NULL;
	tmpex->type=HTT_EXPORT_SYS_SYM|HTF_IMM;
	if (tmpie->type==IET_IMM32_EXPORT||tmpie->type==IET_IMM64_EXPORT)
	  tmpex->val=tmpie->rip;
	else
	  tmpex->val=tmpie->rip+rip2;
	tmpex->src_link=tmpie->src_link;
	tmpie->src_link=NULL;
	HashAdd(tmpex,Fs->hash_table);
	SysSymImportsResolve(tmpex->str);
	break;
      case IET_REL_I0...IET_IMM_I64:
	if (tmpie->str) {
	  if (tmpie->flags&IEF_GOTO_LABEL) {
	    if(!(g_lb=COCGoToLabelFind(cc,str)))
	      "Unresolved Reference:%s\n",str;
	    else {
	      g_lb->use_cnt++;
	      g_lb=OptLabelFwd(g_lb);
	      i=g_lb->addr+tmpaot->buf;
	    }
	    tmpex=NULL;
	  } else {
	    if (!(tmpex=HashFind(str,Fs->hash_table,
		  HTG_ALL-HTT_IMPORT_SYS_SYM)))
	      "Unresolved Reference:%s\n",str;
	    else {
	      if (tmpex->type & HTT_FUN)
		i=tmpex(CHashFun *)->exe_addr;
	      else if (tmpex->type & HTT_GLBL_VAR)
		i=tmpex(CHashGlblVar *)->data_addr;
	      else
		i=tmpex->val;
	    }
	    g_lb=NULL;
	  }
	}
	if (tmpex || g_lb) {
	  ptr=tmpie->rip+rip2;
	  switch [tmpie->type] {
	    case IET_REL_I0:
	    case IET_IMM_U0:
	      break;
	    case IET_REL_I8:
	      if (!(I8_MIN<=i-ptr-1<=I8_MAX))
		LexExcept(cc,"Branch out of range at ");
	      *ptr(U8 *) =i-ptr-1;
	      break;
	    case IET_IMM_U8:
	      *ptr(U8 *) =i;
	      break;
	    case IET_REL_I16:
	      if (!(I16_MIN<=i-ptr-2<=I16_MAX))
		LexExcept(cc,"Branch out of range at ");
	      *ptr(U16 *)=i-ptr-2;
	      break;
	    case IET_IMM_U16:
	      *ptr(U16 *)=i;
	      break;
	    case IET_REL_I32:
	      if (!(I32_MIN<=i-ptr-4<=I32_MAX))
		LexExcept(cc,"Branch out of range at ");
	      *ptr(U32 *)=i-ptr-4;
	      break;
	    case IET_IMM_U32:
	      *ptr(U32 *)=i;
	      break;
	    case IET_REL_I64:
	      *ptr(I64 *)=i-ptr-8;
	      break;
	    case IET_IMM_I64:
	      *ptr(I64 *)=i;
	      break;
	  }
	}
	break;
    }
    Free(tmpie->src_link);
    Free(tmpie);
    tmpie=tmpie1;
  }
  Free(str);
  if (!cc->aot_depth && Bt(&cc->opts,OPTf_TRACE))
    Un(rip2,tmpaot->aot_U8s,64);
  QueRem(tmpaot);
  Free(tmpaot);
}

U0 CmpFixUpAOTAsm(CCmpCtrl *cc,CAOT *tmpaot)
{
  CAOTCtrl *aotc=cc->aotc;
  I64 i,rip2=tmpaot->rip+cc->aotc->rip;
  U8 *ptr;
  CCodeMisc *g_lb=NULL;
  CAOTAbsAddr *tmpa,*tmpa1;
  CAOTImportExport *tmpie,*tmpie1;

  tmpa=tmpaot->abss;
  while (tmpa) {
    tmpa1=tmpa->next;
    tmpa->next=aotc->abss;
    ptr=tmpaot->buf+tmpaot->rip+tmpa->rip;
    switch [tmpa->type] {
      case AAT_ADD_U8:	*ptr(U8 *)+=rip2;	break;
      case AAT_SUB_U8:	*ptr(U8 *)-=rip2;	break;
      case AAT_ADD_U16:	*ptr(U16 *)+=rip2;	break;
      case AAT_SUB_U16:	*ptr(U16 *)-=rip2;	break;
      case AAT_ADD_U32:	*ptr(U32 *)+=rip2;	break;
      case AAT_SUB_U32:	*ptr(U32 *)-=rip2;	break;
      case AAT_ADD_U64:	*ptr(I64 *)+=rip2;	break;
      case AAT_SUB_U64:	*ptr(I64 *)-=rip2;	break;
    }
    aotc->abss=tmpa;
    tmpa->rip+=rip2;
    tmpa=tmpa1;
  }

  tmpie=tmpaot->next_ie;
  while (tmpie!=&tmpaot->next_ie) {
    tmpie1=tmpie->next;
    QueRem(tmpie);
    if (IET_REL_I0<=tmpie->type<=IET_IMM_I64) {
      if (tmpie->str) {
	if (tmpie->flags&IEF_GOTO_LABEL) {
	  if(!(g_lb=COCGoToLabelFind(cc,tmpie->str)))
	    "Unresolved Reference:%s\n",tmpie->str;
	  else {
	    g_lb->use_cnt++;
	    g_lb=OptLabelFwd(g_lb);
	  }
	} else
	  g_lb=NULL;
      }
    } else
      g_lb=NULL;

    ptr=tmpaot->buf+tmpaot->rip+tmpie->rip;
    if (g_lb) {
      i=g_lb->addr+tmpaot->buf;
      switch [tmpie->type] {
	case IET_REL_I0:
	case IET_IMM_U0:
	  break;
	case IET_REL_I8:
	  if (!(I8_MIN<=i-ptr-1<=I8_MAX))
	    LexExcept(cc,"Branch out of range at ");
	  *ptr(U8 *) =i-ptr-1;
	  break;
	case IET_IMM_U8:
	  *ptr(U8 *) =i;
	  break;
	case IET_REL_I16:
	  if (!(I16_MIN<=i-ptr-2<=I16_MAX))
	    LexExcept(cc,"Branch out of range at ");
	  *ptr(U16 *)=i-ptr-2;
	  break;
	case IET_IMM_U16:
	  *ptr(U16 *)=i;
	  break;
	case IET_REL_I32:
	  if (!(I32_MIN<=i-ptr-4<=I32_MAX))
	    LexExcept(cc,"Branch out of range at ");
	  *ptr(U32 *)=i-ptr-4;
	  break;
	case IET_IMM_U32:
	  *ptr(U32 *)=i;
	  break;
	case IET_REL_I64:
	  *ptr(I64 *)=i-ptr-8;
	  break;
	case IET_IMM_I64:
	  *ptr(I64 *)=i;
	  break;
      }
      Free(tmpie->src_link);
      Free(tmpie);
    } else {
      switch (tmpie->type) {
	start:
	  case IET_REL32_EXPORT:
	  case IET_IMM32_EXPORT:
	  case IET_REL64_EXPORT:
	  case IET_IMM64_EXPORT:
	  case IET_IMM_U0:
	  case IET_IMM_U8:
	  case IET_IMM_U16:
	  case IET_IMM_U32:
	  case IET_IMM_I64:
	  case IET_REL_I0:
	    break;
	  case IET_REL_I8:	    *ptr(U8 *) -=rip2;	    break;
	  case IET_REL_I16:	    *ptr(U16 *)-=rip2;	    break;
	  case IET_REL_I32:	    *ptr(U32 *)-=rip2;	    break;
	  case IET_REL_I64:	    *ptr(I64 *)-=rip2;	    break;
	end:
	  tmpie->rip+=rip2;
	  break;
      }
      tmpie->aot=NULL;
      QueIns(tmpie,tmpaot->parent_aot->last_ie);
    }
    tmpie=tmpie1;
  }
}

I64 Cmp(U8 *filename,U8 *map_name=NULL,U8 *out_name=NULL,U8 mapfile_drv_let=0)
{//AOT Compile HC or PRJ file a and output BIN file. Returns err_cnt.
  U8 *ptr,*fbuf=NULL,*fbuf2=NULL,*fbuf3=NULL,
	*patch_table=MAlloc(0x20000);
  CAOT *tmpaot;
  I64 i,cnt,size=0,error_cnt=0,warning_cnt=0,aot_U8s=0;
  CBinFile *bfh;
  CAOTImportExport *tmpie,*tmpie1;
  CAOTAbsAddr *tmpa,*tmpa1;
  CAOTHeapGlblRef *tmphgr,*tmphgr1;
  CAOTHeapGlbl *tmphg,*tmphg1;

  fbuf=ExtDft(filename,"PRJ.Z");
  fbuf2=MStrPrint("#include \"%s\"",fbuf);
  if (map_name)
    fbuf3=ExtDft(map_name,"MAP.Z");

  if (tmpaot=CmpBuf(fbuf2,fbuf3,&error_cnt,&warning_cnt,mapfile_drv_let)) {
    aot_U8s=tmpaot->aot_U8s;
    ptr=patch_table;
//See $LK,"Load",A="MN:Load"$()
    cnt=0;
    tmpa=tmpaot->abss;
    while (tmpa) {
      if (!(tmpa->type&IEF_IMM_NOT_REL))
	cnt++;
      tmpa=tmpa->next;
    }
    if (cnt) {
      *ptr++=IET_ABS_ADDR;
      *ptr(U32 *)++=cnt;
      *ptr++=0;
      tmpa=tmpaot->abss;
      while (tmpa) {
	tmpa1=tmpa->next;
	if (!(tmpa->type&IEF_IMM_NOT_REL))
	  *ptr(U32 *)++ =tmpa->rip;
	Free(tmpa);
	tmpa=tmpa1;
      }
    }
    tmphg=tmpaot->heap_glbls;
    while (tmphg) {
      tmphg1=tmphg->next;
      cnt=0;
      tmphgr=tmphg->references;
      while (tmphgr) {
	cnt++;
	tmphgr=tmphgr->next;
      }
      if (cnt) {
	*ptr++=IET_DATA_HEAP;
	*ptr(U32 *)++=cnt;
	if (tmphg->str) {
	  i=StrLen(tmphg->str);
	  MemCpy(ptr,tmphg->str,i+1);
	  Free(tmphg->str);
	  ptr+=i+1;
	} else
	  *ptr++=0;
	*ptr(I64 *)++=tmphg->size;
	tmphgr=tmphg->references;
	while (tmphgr) {
	  tmphgr1=tmphgr->next;
	  *ptr(U32 *)++=tmphgr->rip;
	  Free(tmphgr);
	  tmphgr=tmphgr1;
	}
      }
      Free(tmphg);
      tmphg=tmphg1;
    }

    //Do exports first
    tmpie=tmpaot->next_ie;
    while (tmpie!=&tmpaot->next_ie) {
      tmpie1=tmpie->next;
      if (!tmpie->type || IET_REL32_EXPORT<=tmpie->type<=IET_IMM64_EXPORT) {
	QueRem(tmpie);
	*ptr++=tmpie->type;
	*ptr(U32 *)++=tmpie->rip;
	if (tmpie->str) {
	  i=StrLen(tmpie->str);
	  MemCpy(ptr,tmpie->str,i+1);
	  Free(tmpie->str);
	  ptr+=i+1;
	} else
	  *ptr++=0;
	Free(tmpie->src_link);
	Free(tmpie);
      }
      tmpie=tmpie1;
    }

    //Do imports second
    tmpie=tmpaot->next_ie;
    while (tmpie!=&tmpaot->next_ie) {
      tmpie1=tmpie->next;
      QueRem(tmpie);
      *ptr++=tmpie->type;
      if (tmpie->aot)
	tmpie->rip+=tmpie->aot->rip2;
      *ptr(U32 *)++=tmpie->rip;
      if (tmpie->str) {
	i=StrLen(tmpie->str);
	MemCpy(ptr,tmpie->str,i+1);
	Free(tmpie->str);
	ptr+=i+1;
      } else
	*ptr++=0;
      Free(tmpie->src_link);
      Free(tmpie);
      tmpie=tmpie1;
    }

    *ptr++=IET_END;
    MemSet(ptr,0,16);
    i=ptr-patch_table;
//Needs 16 ALIGN
    size=(sizeof(CBinFile)+aot_U8s+i+15)&-16;
    bfh=MAlloc(size);
    bfh->jmp=0xEB+256*(sizeof(CBinFile)-2);
#assert sizeof(CBinFile)-2<=I8_MAX
    bfh->reserved=0;
    bfh->bin_signature=BIN_SIGNATURE_VAL;
    bfh->org=tmpaot->org;
    bfh->module_align_bits=tmpaot->max_align_bits;
    bfh->patch_table_offset=sizeof(CBinFile)+aot_U8s;
    bfh->file_size=size;
    MemCpy(bfh(U8 *)+sizeof(CBinFile),tmpaot->buf,aot_U8s);
    MemCpy(bfh(U8 *)+sizeof(CBinFile)+aot_U8s,patch_table,
	  size-aot_U8s-sizeof(CBinFile));
    Free(fbuf2);
    if (out_name)
      fbuf2=ExtDft(out_name,"BIN.Z");
    else
      fbuf2=ExtChg(fbuf,"BIN.Z");
    FileWrite(fbuf2,bfh,size);
    Free(bfh);
    Free(tmpaot->buf);
    QueDel(tmpaot);
    Free(tmpaot);
  }
  Free(patch_table);
  Free(fbuf);
  Free(fbuf2);
  Free(fbuf3);
  Print("Errs:%d Warns:%d Code:%X Size:%X\n",
	error_cnt,warning_cnt,aot_U8s,size);
  return error_cnt;
}

I64 ExePutS(U8 *buf,U8 *filename=NULL,
	I64 ccf_flags=0,CLexHashTableContext *htc=NULL)
{//JIT Compile and execute text from a puts("").
  I64 res;
  Bool okay=TRUE;
  CCmpCtrl *cc;
  if (!filename)
    filename=blkdev.tmp_filename;
  cc=CmpCtrlNew(buf,ccf_flags|CCF_DONT_FREE_BUF,filename);
  if (Fs->last_cc!=&Fs->next_cc) {
    cc->opts=Fs->last_cc->opts;
    if (htc) {
      cc->flags=cc->flags &~CCF_ASM_EXPRESSIONS |
	    htc->old_flags&CCF_ASM_EXPRESSIONS;
      MemCpy(&cc->htc,htc,sizeof(CLexHashTableContext));
    }
  }
  QueIns(cc,Fs->last_cc);
  try {
    Lex(cc);
    res=ExeCmdLine(cc);
  } catch {
    if (Fs->except_ch=='Compiler' || Fs->except_ch=='Break') {
      Fs->catch_except=TRUE;
      okay=FALSE;
      res=0;
    }
  }
  QueRem(cc);
  if (okay)
    CmpCtrlDel(cc); //TODO: can crash
  return res;
}

I64 ExePrint(U8 *fmt,...)
{//JIT Compile and execute text from a printf().
  I64 res;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  res=ExePutS(buf);
  Free(buf);
  return res;
}

I64 ExeFile(U8 *name,I64 ccf_flags=0)
{//JIT Compile and execute a file.
  I64 res;
  U8 *name2=ExtDft(name,"HC.Z"),
	*st=MStrPrint("#include \"%s\";",name2);
  res=ExePutS(st,name,ccf_flags);
  Free(st);
  Free(name2);
  return res;
}

I64 RunFile(U8 *name,I64 ccf_flags=0,...)
{//$LK,"ExeFile",A="MN:ExeFile"$() with args using $LK,"LastFun",A="MN:LastFun"$().
  ExeFile(name,ccf_flags);
  return LastFun(argc,argv);
}

I64 ExePutS2(U8 *buf,U8 *filename=NULL,I64 ccf_flags=0)
{//throws exceptions
  I64 res;
  CCmpCtrl *cc;
  if (!filename)
    filename=blkdev.tmp_filename;
  cc=CmpCtrlNew(buf,ccf_flags|CCF_DONT_FREE_BUF,filename);
  if (Fs->last_cc!=&Fs->next_cc)
    cc->opts=Fs->last_cc->opts;
  QueIns(cc,Fs->last_cc);
  Lex(cc);
  res=ExeCmdLine(cc);
  QueRem(cc);
  CmpCtrlDel(cc);
  return res;
}

I64 ExePrint2(U8 *fmt,...)
{//throws exceptions
  I64 res;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  res=ExePutS2(buf);
  Free(buf);
  return res;
}

I64 ExeFile2(U8 *name,I64 ccf_flags=0)
{//throws exceptions
  I64 res;
  U8 *name2=ExtDft(name,"HC.Z"),*st=MStrPrint("#include \"%s\";",name2);
  res=ExePutS2(st,name,ccf_flags);
  Free(st);
  Free(name2);
  return res;
}

I64 RunFile2(U8 *name,I64 ccf_flags=0,...)
{//$LK,"ExeFile2",A="MN:ExeFile2"$() with args using $LK,"LastFun",A="MN:LastFun"$(). throws exceptions.
  ExeFile2(name,ccf_flags);
  return LastFun(argc,argv);
}

I64 StreamExePrint(U8 *fmt,...)
{//Causes value from stream to be used in an #exe{} block.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  I64 res=0;
  CLexHashTableContext *htc;
  CCmpCtrl *cc=Fs->last_cc;
  if (cc==&Fs->next_cc)
    PrintErr("Not Compiling\n");
  else {
    if (!(cc->flags&CCF_EXE_BLK))
      LexExcept(cc,"StreamExePrint only allowed in AOT compiled #exe{} mode.");
    if (htc=cc->htc.next)
      res=ExePutS(buf,,,htc);
  }
  Free(buf);
  return res;
}

U0 CInit()
{
  CmpLoadDefines;
  CmpFillTables;
  QueInit(&cmp.ic_nop);
  cmp.ic_nop.ic_class=cmp.internal_types[RT_I64];
  cmp.ic_nop.ic_code=IC_NOP1;
  AsmHashLoad;
  UAsmHashLoad;
}

CInit;
