U8 *LexStmt2Bin(CCmpCtrl *cc,I64 *_type,I64 cmp_flags=0)
{//Compile one cc stmt to bin code.
  I64 size,i,j,k,*result=INVALID_PTR;
  CCodeCtrl *tempcbh;
  if (_type) *_type=RT_I64;
  Btr(&cc->flags,CCf_PASS_TRACE_PRESENT);
  if (cc->aot_depth==2)
    COCPush(cc);
  COCInit(cc);
  if (!PrsStmt(cc,,,cmp_flags)) {
    if (cc->coc.coc_root.next!=&cc->coc.coc_root) {
      cc->coc.coc_root.last->ic_flags&=~ICF_NO_DEPEND_RESULT;
      ICAdd(cc,IC_RETURN_VAL2,0,0);
      ICAdd(cc,IC_RET,0,0);
      if (result=COCCompile(cc,&size,NULL,_type)) {
	if (cc->flags&CCF_AOT_COMPILE) {
	  j=cc->aotc->ip;
	  k=(size+7)>>3;
	  for (i=0;i<k;i++)
	    AOTStoreCodeU64(cc,result[i]);
	  Free(result);
	  result=j;
	}
      }
    } //TODO: else del misc?
  } else //TODO: too dangerous to del Misc?
    QueDel(&cc->coc.coc_root.next);
  if (cc->aot_depth==2) {
    tempcbh=COCPopNoFree(cc);
    COCAppend(cc,tempcbh);
  }
  return result;
}

CAOT *CmpJoin(CCmpCtrl *cc,I64 cmp_flags,U8 *map_name=NULL,U8 mapfile_drv_let=0)
{
  CAOTCtrl *aotc,*old_aot=cc->aotc;
  I64 i,j,l;
  U8 *buf;
  CAOTBinBlk *tempbin;
  CAOTImportExport *tempie;
  Bool okay=TRUE;
  CLexHashTableContext *htc=MAlloc(sizeof(CLexHashTableContext));
  CAOT *result=CAlloc(sizeof(CAOT)),*parent;
  if (parent=cc->aot) {
    result->parent_aot=parent;
    QueIns(result,parent->last);
  } else
    QueInit(result);
  cc->aot=result;

  result->next_ie=result->last_ie=&result->next_ie;
  cc->aotc=aotc=CAlloc(sizeof(CAOTCtrl));
  cc->aot_depth++;

  aotc->bin=CAlloc(sizeof(CAOTBinBlk));
  aotc->max_align_bits=0;
  aotc->module_org=INVALID_PTR;

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
	  tempie=CAlloc(sizeof(CAOTImportExport));
	  tempie->type=IET_MAIN;
	  tempie->ip=buf;
	  QueIns(tempie,result->last_ie);
	}
	if (cmp_flags&CMPF_ASM_BLK)
	  break;
      }
    } else
      PrsStmt(cc,,,cmp_flags);
    AOTGlblsResolve(cc,result);
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
    cc->aot=result->parent_aot;
    Free(result);
    LinkedLstDel(aotc->bin);
    result=NULL;
  } else {
    if (map_name)
      MapFileWrite(cc->htc.glbl_hash_table,map_name,mapfile_drv_let);
    HashTableDel(cc->htc.local_hash_table);
    HashTableDel(cc->htc.glbl_hash_table);

    if (!aotc->num_bin_U8s)
      result->buf=NULL;
    else {
      if (cc->flags&CCF_AOT_COMPILE)
	result->buf=MAlloc(aotc->num_bin_U8s);
      else {
	if (aotc->module_org==INVALID_PTR)
	  result->buf=MAlloc(aotc->num_bin_U8s,Fs->code_heap);
	else
	  result->buf=aotc->module_org;
      }
      result->aot_U8s=aotc->num_bin_U8s;
      tempbin=aotc->bin;
      j=0;
      l=aotc->num_bin_U8s;
      while (tempbin) {
	i=l;
	if (i>AOT_BIN_BLK_SIZE)
	  i=AOT_BIN_BLK_SIZE;
	MemCpy(result->buf+j,tempbin->body,i);
	j+=i;
	l-=i;
	tempbin=tempbin->next;
      }
    }
    LinkedLstDel(aotc->bin);
    result->abss=aotc->abss;
    result->heap_glbls=aotc->heap_glbls;
    result->max_align_bits=aotc->max_align_bits;
    result->module_org=aotc->module_org;
  }
  cc->aot=parent;
  MemCpy(&cc->htc,htc,sizeof(CLexHashTableContext));
  Free(htc);
  Free(aotc);
  cc->aotc=old_aot;
  cc->aot_depth--;
  return result;
}

CAOT *CmpBuf(U8 *buf,U8 *map_name=NULL,
  I64 *error_cnt=NULL, I64 *warning_cnt=NULL,U8 mapfile_drv_let=0)
{
  CCmpCtrl *cc;
  CAOT *result=NULL;
  cc=CmpCtrlNew(buf,CCF_DONT_FREE_BUF);
  cc->flags|=CCF_AOT_COMPILE;
  QueIns(cc,Fs->last_lex);
  result=CmpJoin(cc,CMPF_LEX_FIRST,map_name,mapfile_drv_let);
  if (error_cnt)   *error_cnt=cc->error_cnt;
  if (warning_cnt) *warning_cnt=cc->warning_cnt;
  QueRem(cc);
  if (result)
    CmpCtrlDel(cc);
  return result;
}

U0 CmpFixUpJITAsm(CCmpCtrl *cc,CAOT *tempaot)
{
  I64 i,ip2=tempaot->buf+tempaot->ip,*str=NULL;
  U8 *ptr;
  CCodeMisc *g_lb;
  CAOTAbsAddr *tempa,*tempa1;
  CAOTImportExport *tempie,*tempie1;
  CHashExport *tempex;

  tempa=tempaot->abss;
  while (tempa) {
    tempa1=tempa->next;
    ptr=ip2+tempa->ip;
    switch [tempa->type] {
      case AAT_ADD_U8:	*ptr(U8 *) +=ip2; break;
      case AAT_SUB_U8:	*ptr(U8 *) -=ip2; break;
      case AAT_ADD_U16:	*ptr(U16 *)+=ip2; break;
      case AAT_SUB_U16:	*ptr(U16 *)-=ip2; break;
      case AAT_ADD_U32:	*ptr(U32 *)+=ip2; break;
      case AAT_SUB_U32:	*ptr(U32 *)-=ip2; break;
      case AAT_ADD_U64:	*ptr(I64 *)+=ip2; break;
      case AAT_SUB_U64:	*ptr(I64 *)-=ip2; break;
    }
    Free(tempa);
    tempa=tempa1;
  }
  tempie=tempaot->next_ie;
  while (tempie!=&tempaot->next_ie) {
    tempie1=tempie->next;
    if (tempie->str) {
      Free(str);
      str=tempie->str;
    }
    switch (tempie->type) {
      case IET_REL32_EXPORT:
      case IET_IMM32_EXPORT:
      case IET_REL64_EXPORT:
      case IET_IMM64_EXPORT:
	tempex=CAlloc(sizeof(CHashExport));
	tempex->str=str;
	str=NULL;
	tempex->type=HTT_EXPORT_SYS_SYM|HTF_IMM;
	if (tempie->type==IET_IMM32_EXPORT||tempie->type==IET_IMM64_EXPORT)
	  tempex->val=tempie->ip;
	else
	  tempex->val=tempie->ip+ip2;
	tempex->src_link=tempie->src_link;
	tempie->src_link=NULL;
	HashAdd(tempex,Fs->hash_table);
	SysSymImportsResolve(tempex->str);
	break;
      case IET_REL_I0...IET_IMM_I64:
	if (tempie->str) {
	  if (tempie->flags&IEF_GOTO_LABEL) {
	    if(!(g_lb=COCGoToLabelFind(cc,str)))
	      "Unresolved Reference:%s\n",str;
	    else {
	      g_lb->use_cnt++;
	      g_lb=OptLabelFwd(g_lb);
	      i=g_lb->addr+tempaot->buf;
	    }
	    tempex=NULL;
	  } else {
	    if (!(tempex=HashFind(str,Fs->hash_table,
		  HTG_ALL-HTT_IMPORT_SYS_SYM)))
	      "Unresolved Reference:%s\n",str;
	    else {
	      if (tempex->type & HTT_FUN)
		i=tempex(CHashFun *)->executable_addr;
	      else if (tempex->type & HTT_GLBL_VAR)
		i=tempex(CHashGlblVar *)->data_addr;
	      else
		i=tempex->val;
	    }
	    g_lb=NULL;
	  }
	}
	if (tempex || g_lb) {
	  ptr=tempie->ip+ip2;
	  switch [tempie->type] {
	    case IET_REL_I0:
	    case IET_IMM_U0:
	      break;
	    case IET_REL_I8:
	      if (!(MIN_I8<=i-ptr-1<=MAX_I8))
		LexExcept(cc,"Branch out of range at ");
	      *ptr(U8 *) =i-ptr-1;
	      break;
	    case IET_IMM_U8:
	      *ptr(U8 *) =i;
	      break;
	    case IET_REL_I16:
	      if (!(MIN_I16<=i-ptr-2<=MAX_I16))
		LexExcept(cc,"Branch out of range at ");
	      *ptr(U16 *)=i-ptr-2;
	      break;
	    case IET_IMM_U16:
	      *ptr(U16 *)=i;
	      break;
	    case IET_REL_I32:
	      if (!(MIN_I32<=i-ptr-4<=MAX_I32))
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
    Free(tempie->src_link);
    Free(tempie);
    tempie=tempie1;
  }
  Free(str);
  if (!cc->aot_depth && Bt(&cc->opts,OPTf_TRACE))
    Un(ip2,tempaot->aot_U8s,64);
  QueRem(tempaot);
  Free(tempaot);
}

U0 CmpFixUpAOTAsm(CCmpCtrl *cc,CAOT *tempaot)
{
  CAOTCtrl *aotc=cc->aotc;
  I64 i,ip2=tempaot->ip+cc->aotc->ip;
  U8 *ptr;
  CCodeMisc *g_lb=NULL;
  CAOTAbsAddr *tempa,*tempa1;
  CAOTImportExport *tempie,*tempie1;

  tempa=tempaot->abss;
  while (tempa) {
    tempa1=tempa->next;
    tempa->next=aotc->abss;
    ptr=tempaot->buf+tempaot->ip+tempa->ip;
    switch [tempa->type] {
      case AAT_ADD_U8:	*ptr(U8 *)+=ip2;	break;
      case AAT_SUB_U8:	*ptr(U8 *)-=ip2;	break;
      case AAT_ADD_U16:	*ptr(U16 *)+=ip2;	break;
      case AAT_SUB_U16:	*ptr(U16 *)-=ip2;	break;
      case AAT_ADD_U32:	*ptr(U32 *)+=ip2;	break;
      case AAT_SUB_U32:	*ptr(U32 *)-=ip2;	break;
      case AAT_ADD_U64:	*ptr(I64 *)+=ip2;	break;
      case AAT_SUB_U64:	*ptr(I64 *)-=ip2;	break;
    }
    aotc->abss=tempa;
    tempa->ip+=ip2;
    tempa=tempa1;
  }

  tempie=tempaot->next_ie;
  while (tempie!=&tempaot->next_ie) {
    tempie1=tempie->next;
    QueRem(tempie);
    if (IET_REL_I0<=tempie->type<=IET_IMM_I64) {
      if (tempie->str) {
	if (tempie->flags&IEF_GOTO_LABEL) {
	  if(!(g_lb=COCGoToLabelFind(cc,tempie->str)))
	    "Unresolved Reference:%s\n",tempie->str;
	  else {
	    g_lb->use_cnt++;
	    g_lb=OptLabelFwd(g_lb);
	  }
	} else
	  g_lb=NULL;
      }
    } else
      g_lb=NULL;

    ptr=tempaot->buf+tempaot->ip+tempie->ip;
    if (g_lb) {
      i=g_lb->addr+tempaot->buf;
      switch [tempie->type] {
	case IET_REL_I0:
	case IET_IMM_U0:
	  break;
	case IET_REL_I8:
	  if (!(MIN_I8<=i-ptr-1<=MAX_I8))
	    LexExcept(cc,"Branch out of range at ");
	  *ptr(U8 *) =i-ptr-1;
	  break;
	case IET_IMM_U8:
	  *ptr(U8 *) =i;
	  break;
	case IET_REL_I16:
	  if (!(MIN_I16<=i-ptr-2<=MAX_I16))
	    LexExcept(cc,"Branch out of range at ");
	  *ptr(U16 *)=i-ptr-2;
	  break;
	case IET_IMM_U16:
	  *ptr(U16 *)=i;
	  break;
	case IET_REL_I32:
	  if (!(MIN_I32<=i-ptr-4<=MAX_I32))
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
      Free(tempie->src_link);
      Free(tempie);
    } else {
      switch (tempie->type) {
	sub_switch_start:
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
	  case IET_REL_I8:	    *ptr(U8 *) -=ip2;	    break;
	  case IET_REL_I16:	    *ptr(U16 *)-=ip2;	    break;
	  case IET_REL_I32:	    *ptr(U32 *)-=ip2;	    break;
	  case IET_REL_I64:	    *ptr(I64 *)-=ip2;	    break;
	sub_switch_end:
	  tempie->ip+=ip2;
	  break;
      }
      tempie->aot=NULL;
      QueIns(tempie,tempaot->parent_aot->last_ie);
    }
    tempie=tempie1;
  }
}

I64 Cmp(U8 *filename,U8 *map_name=NULL,U8 *out_name=NULL,U8 mapfile_drv_let=0)
{//AOT Compile CPP or PRJ file a and output BIN file. Returns err_cnt.
  U8 *ptr,*fbuf=NULL,*fbuf2=NULL,*fbuf3=NULL,
	*patch_table=MAlloc(0x20000);
  CAOT *tempaot;
  I64 i,cnt,size=0,error_cnt=0,warning_cnt=0,aot_U8s=0;
  CBinFile *bfh;
  CAOTImportExport *tempie,*tempie1;
  CAOTAbsAddr *tempa,*tempa1;
  CAOTHeapGlblRef *temphgr,*temphgr1;
  CAOTHeapGlbl *temphg,*temphg1;

  fbuf=DftExt(filename,"PRJ.Z");
  fbuf2=MStrPrint("#include \"%s\"",fbuf);
  if (map_name)
    fbuf3=DftExt(map_name,"MAP.Z");

  if (tempaot=CmpBuf(fbuf2,fbuf3,&error_cnt,&warning_cnt,mapfile_drv_let)) {
    aot_U8s=tempaot->aot_U8s;
    ptr=patch_table;
//See $LK,"Load",A="MN:Load"$()
    cnt=0;
    tempa=tempaot->abss;
    while (tempa) {
      if (!(tempa->type&IEF_IMM_NOT_REL))
	cnt++;
      tempa=tempa->next;
    }
    if (cnt) {
      *ptr++=IET_ABS_ADDR;
      *ptr(U32 *)++=cnt;
      *ptr++=0;
      tempa=tempaot->abss;
      while (tempa) {
	tempa1=tempa->next;
	if (!(tempa->type&IEF_IMM_NOT_REL))
	  *ptr(U32 *)++ =tempa->ip;
	Free(tempa);
	tempa=tempa1;
      }
    }
    temphg=tempaot->heap_glbls;
    while (temphg) {
      temphg1=temphg->next;
      cnt=0;
      temphgr=temphg->references;
      while (temphgr) {
	cnt++;
	temphgr=temphgr->next;
      }
      if (cnt) {
	*ptr++=IET_DATA_HEAP;
	*ptr(U32 *)++=cnt;
	if (temphg->str) {
	  i=StrLen(temphg->str);
	  MemCpy(ptr,temphg->str,i+1);
	  Free(temphg->str);
	  ptr+=i+1;
	} else
	  *ptr++=0;
	*ptr(I64 *)++=temphg->size;
	temphgr=temphg->references;
	while (temphgr) {
	  temphgr1=temphgr->next;
	  *ptr(U32 *)++=temphgr->ip;
	  Free(temphgr);
	  temphgr=temphgr1;
	}
      }
      Free(temphg);
      temphg=temphg1;
    }

    //Do exports first
    tempie=tempaot->next_ie;
    while (tempie!=&tempaot->next_ie) {
      tempie1=tempie->next;
      if (!tempie->type || IET_REL32_EXPORT<=tempie->type<=IET_IMM64_EXPORT) {
	QueRem(tempie);
	*ptr++=tempie->type;
	*ptr(U32 *)++=tempie->ip;
	if (tempie->str) {
	  i=StrLen(tempie->str);
	  MemCpy(ptr,tempie->str,i+1);
	  Free(tempie->str);
	  ptr+=i+1;
	} else
	  *ptr++=0;
	Free(tempie->src_link);
	Free(tempie);
      }
      tempie=tempie1;
    }

    //Do imports second
    tempie=tempaot->next_ie;
    while (tempie!=&tempaot->next_ie) {
      tempie1=tempie->next;
      QueRem(tempie);
      *ptr++=tempie->type;
      if (tempie->aot)
	tempie->ip+=tempie->aot->ip2;
      *ptr(U32 *)++=tempie->ip;
      if (tempie->str) {
	i=StrLen(tempie->str);
	MemCpy(ptr,tempie->str,i+1);
	Free(tempie->str);
	ptr+=i+1;
      } else
	*ptr++=0;
      Free(tempie->src_link);
      Free(tempie);
      tempie=tempie1;
    }

    *ptr++=IET_END;
    MemSet(ptr,0,16);
    i=ptr-patch_table;
//Needs 16 ALIGN
    size=(sizeof(CBinFile)+aot_U8s+i+15)&-16;
    bfh=MAlloc(size);
    bfh->jmp=0xEB+256*(sizeof(CBinFile)-2);
#assert sizeof(CBinFile)-2<=MAX_I8
    bfh->reserved=0;
    bfh->bin_signature=BIN_SIGNATURE_VAL;
    bfh->module_org=tempaot->module_org;
    bfh->module_align_bits=tempaot->max_align_bits;
    bfh->patch_table_offset=sizeof(CBinFile)+aot_U8s;
    bfh->file_size=size;
    MemCpy(bfh(U8 *)+sizeof(CBinFile),tempaot->buf,aot_U8s);
    MemCpy(bfh(U8 *)+sizeof(CBinFile)+aot_U8s,patch_table,
	  size-aot_U8s-sizeof(CBinFile));
    Free(fbuf2);
    if (out_name)
      fbuf2=DftExt(out_name,"BIN.Z");
    else
      fbuf2=ChgExt(fbuf,"BIN.Z");
    FileWrite(fbuf2,bfh,size);
    Free(bfh);
    Free(tempaot->buf);
    QueDel(tempaot);
    Free(tempaot);
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
	Bool just_load=FALSE,CLexHashTableContext *htc=NULL)
{//JIT Compile and execute text from a puts("").
  I64 result;
  Bool okay=TRUE;
  CCmpCtrl *cc;
  if (!filename)
    filename=blkdev.temp_filename;
  cc=CmpCtrlNew(buf,CCF_DONT_FREE_BUF,filename);
  if (just_load)
    cc->flags|=CCF_JUST_LOAD;
  if (Fs->last_lex!=&Fs->next_lex) {
    cc->opts=Fs->last_lex->opts;
    if (htc) {
      cc->flags=cc->flags &~CCF_ASM_EXPRESSIONS |
	    htc->old_flags&CCF_ASM_EXPRESSIONS;
      MemCpy(&cc->htc,htc,sizeof(CLexHashTableContext));
    }
  }
  QueIns(cc,Fs->last_lex);
  try {
    Lex(cc);
    result=ExeCmdLine(cc);
  } catch {
    if (Fs->except_ch=='Compiler' || Fs->except_ch=='Break') {
      Fs->catch_except=TRUE;
      okay=FALSE;
      result=0;
    }
  }
  QueRem(cc);
  if (okay)
    CmpCtrlDel(cc); //TODO: can crash
  return result;
}

I64 ExePrint(U8 *fmt,...)
{//JIT Compile and execute text from a printf().
  I64 result;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  result=ExePutS(buf);
  Free(buf);
  return result;
}

I64 ExeFile(U8 *name,Bool just_load=FALSE)
{//JIT Compile and execute a file.
  I64 result;
  U8 *name2=DftExt(name,"CPP.Z"),
	*st=MStrPrint("#include \"%s\";",name2);
  result=ExePutS(st,name,just_load);
  Free(st);
  Free(name2);
  return result;
}

I64 ExePutS2(U8 *buf,U8 *filename=NULL,Bool just_load=FALSE)
{//throws exceptions
  I64 result;
  CCmpCtrl *cc;
  if (!filename)
    filename=blkdev.temp_filename;
  cc=CmpCtrlNew(buf,CCF_DONT_FREE_BUF,filename);
  if (just_load)
    cc->flags|=CCF_JUST_LOAD;
  if (Fs->last_lex!=&Fs->next_lex)
    cc->opts=Fs->last_lex->opts;
  QueIns(cc,Fs->last_lex);
  Lex(cc);
  result=ExeCmdLine(cc);
  QueRem(cc);
  CmpCtrlDel(cc);
  return result;
}

I64 ExePrint2(U8 *fmt,...)
{//throws exceptions
  I64 result;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  result=ExePutS2(buf);
  Free(buf);
  return result;
}

I64 ExeFile2(U8 *name,Bool just_load=FALSE)
{//throws exceptions
  I64 result;
  U8 *name2=DftExt(name,"CPP.Z"),*st=MStrPrint("#include \"%s\";",name2);
  result=ExePutS2(st,name,just_load);
  Free(st);
  Free(name2);
  return result;
}

I64 StreamExePrint(U8 *fmt,...)
{//Causes value from stream to be used in an #exe{} block.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  I64 result=0;
  CLexHashTableContext *htc;
  CCmpCtrl *cc=Fs->last_lex;
  if (cc==&Fs->next_lex)
    PrintErr("Not Compiling\n");
  else {
    if (!(cc->flags&CCF_EXE_BLK))
      LexExcept(cc,"StreamExePrint only allowed in AOT compiled #exe{} mode.");
    if (htc=cc->htc.next)
      result=ExePutS(buf,,,htc);
  }
  Free(buf);
  return result;
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
