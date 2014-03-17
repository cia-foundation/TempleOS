U0 StreamPrint(U8 *fmt,...)
//Inserts text into the stream of
//characters being compiled.  Called
//from withing #exe{} block.
{
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv),*st;
  CLex *lx=Fs->last_lex;
  CStreamBlk *tempe=lx->last_stream_blk;
  if (tempe!=&lx->next_stream_blk) {
    st=MPrint("%s%s",tempe->body,buf);
    Free(tempe->body);
    tempe->body=st;
  } else
    PrintErr("No exe{} blk\n");
  Free(buf);
}

U8 *LexStatement2Bin(CLex *lx,I64 *_type,I64 cmp_flags=0)
{
  I64 size,i,j,k,*result=INVALID_PTR;
  CLexCodeCtrl *tempcbh;
  if (_type) *_type=RT_I64;
  Btr(&lx->flags,Lf_OPT_TRACE_PRESENT);
  if (lx->a_depth==2)
    LCCPush(lx);
  LCCInit(lx);
  if (!PrsStatement(lx,,,cmp_flags)) {
    if (lx->lcc.lcc_root.next!=&lx->lcc.lcc_root) {
      LCCAdd(lx,IC_RETURN_VAL2,0,NULL);
      LCCAdd(lx,IC_RET,0,0);
      if (result=LCCCompile(lx,&size,NULL,_type)) {
	if (lx->flags&LF_AOT_COMPILE) {
	  j=lx->a->ip;
	  k=(size+7)>>3;
	  for (i=0;i<k;i++)
	    AsmStoreCodeU64(lx,result[i]);
	  Free(result);
	  result=j;
	}
      }
    } //TODO: else del misc?
  } else //TODO: too dangerous to del Misc?
    QueDel(&lx->lcc.lcc_root.next);
  if (lx->a_depth==2) {
progress1++;
    tempcbh=LCCPopNoFree(lx);
    LCCAppend(lx,tempcbh);
  }
  return result;
}


CExe *CmpJoin(CLex *lx,I64 cmp_flags,U8 *map_name=NULL,U8 mapfile_drv=0)
{
  CAsmCtrl *a,*old_a=lx->a;
  I64 i,j,l;
  U8 *buf;
  CAsmCodeBlk *tempc;
  CExeImportExport *tempe;
  Bool okay=TRUE;
  CLexHashTableContext *htc=MAlloc(sizeof(CLexHashTableContext));
  CExe *result=CAlloc(sizeof(CExe)),*parent;
  if (parent=lx->exe) {
    result->parent_exe=parent;
    QueIns(result,parent->last);
  } else
    QueInit(result);
  lx->exe=result;

  result->next_ie=result->last_ie=&result->next_ie;
  lx->a=a=CAlloc(sizeof(CAsmCtrl));
  lx->a_depth++;

  a->cb=CAlloc(sizeof(CAsmCodeBlk));
  a->max_align=1;
  a->module_org=INVALID_PTR;

  MemCpy(htc,&lx->htc,sizeof(CLexHashTableContext));
  if (lx->htc.fun)
    lx->htc.glbl_hash_table=HashTableNew(128);
  else
    lx->htc.glbl_hash_table=HashTableNew(2048);
  if (lx->flags&LF_AOT_COMPILE) {
    lx->htc.define_hash_table=lx->htc.glbl_hash_table;
    if (lx->a_depth<=1)
      lx->htc.glbl_hash_table->next=asm_hash;
    else
      lx->htc.glbl_hash_table->next=htc->glbl_hash_table;
  } else
    lx->htc.glbl_hash_table->next=Fs->hash_table;
  lx->htc.hash_table_lst=lx->htc.local_hash_table=HashTableNew(16);
  lx->htc.local_hash_table->next=lx->htc.glbl_hash_table;
  lx->htc.local_var_lst=lx->htc.fun; //HolyC local vars
  lx->htc.fun=NULL;
  try {
    if (cmp_flags&CMPF_LEX_FIRST)
      Lex(lx);
    if (!(cmp_flags&CMPF_ONE_ASM_INS))
      cmp_flags|=CMPF_PRS_SEMICOLON;
    if (lx->flags&LF_AOT_COMPILE) {
      while (lx->token!=TK_EOF) {
	buf=LexStatement2Bin(lx,NULL,cmp_flags);
	if (buf!=INVALID_PTR) {
	  tempe=CAlloc(sizeof(CExeImportExport));
	  tempe->type=EIT_MAIN;
	  tempe->ip=buf;
	  QueIns(tempe,result->last_ie);
	}
	if (cmp_flags&CMPF_ASM_BLK)
	  break;
      }
    } else
      PrsStatement(lx,,,cmp_flags);
    AsmGlblsResolve(lx,result);
  } catch {
    if (Fs->except_ch=='Compiler' && !(cmp_flags&CMPF_ASM_BLK)) {
      LexPutPos(lx);
      Fs->catch_except=TRUE;
    }
    okay=FALSE;
  }
  if (!okay) {
    if (lx->error_cnt<1)
      lx->error_cnt=1;
    lx->exe=result->parent_exe;
    Free(result);
    LinkedLstDel(a->cb);
    result=NULL;
  } else {
    if (map_name && !cmp_do_not_write_to_dsk)
      Who("-r",lx->htc.glbl_hash_table,map_name,mapfile_drv);
    HashTableDel(lx->htc.local_hash_table);
    HashTableDel(lx->htc.glbl_hash_table);

    if (!a->num_ex_U8s)
      result->buf=NULL;
    else {
      if (lx->flags&LF_AOT_COMPILE)
	result->buf=MAlloc(a->num_ex_U8s);
      else {
	if (a->module_org==INVALID_PTR)
	  result->buf=MAlloc(a->num_ex_U8s,Fs->code_heap);
	else
	  result->buf=a->module_org;
      }
      result->ex_U8s=a->num_ex_U8s;
      tempc=a->cb;
      j=0;
      l=a->num_ex_U8s;
      while (tempc) {
	i=l;
	if (i>ASM_CODE_BLK_SIZE)
	  i=ASM_CODE_BLK_SIZE;
	MemCpy(result->buf+j,tempc->body,i);
	j+=i;
	l-=i;
	tempc=tempc->next;
      }
    }
    LinkedLstDel(a->cb);
    result->abss=a->abss;
    result->heap_glbls=a->heap_glbls;
    result->max_align =a->max_align;
    result->module_org=a->module_org;
  }
  lx->exe=parent;
  MemCpy(&lx->htc,htc,sizeof(CLexHashTableContext));
  Free(htc);
  Free(a);
  lx->a=old_a;
  lx->a_depth--;
  return result;
}

CExe *CmpBuf(U8 *buf,U8 *map_name=NULL,
  I64 *error_cnt=NULL, I64 *warning_cnt=NULL,U8 mapfile_drv=0)
{
  CLex *lx;
  CExe *result=NULL;
  lx=LexNew(buf,LF_DONT_FREE_BUF);
  lx->flags|=LF_AOT_COMPILE;
  QueIns(lx,Fs->last_lex);
  result=CmpJoin(lx,CMPF_LEX_FIRST,map_name,mapfile_drv);
  if (error_cnt)   *error_cnt=lx->error_cnt;
  if (warning_cnt) *warning_cnt=lx->warning_cnt;
  QueRem(lx);
  if (result)
    LexDel(lx);
  return result;
}

U0 CmpFixUpJITAsm(CLex *lx,CExe *tempx)
{
  I64 i,ip2=tempx->buf+tempx->ip,*str=NULL;
  U8 *ptr;
  CLexCodeMisc *g_lb;
  CExeAbsAddr *tempa,*tempa1;
  CExeImportExport *tempe,*tempe1;
  CHashExport *tempex;

  tempa=tempx->abss;
  while (tempa) {
    tempa1=tempa->next;
    ptr=ip2+tempa->ip;
    nobound_switch (tempa->type) {
      case EAT_ADD_U8:	*ptr(U8 *) +=ip2; break;
      case EAT_SUB_U8:	*ptr(U8 *) -=ip2; break;
      case EAT_ADD_U16:	*ptr(U16 *)+=ip2; break;
      case EAT_SUB_U16:	*ptr(U16 *)-=ip2; break;
      case EAT_ADD_U32:	*ptr(U32 *)+=ip2; break;
      case EAT_SUB_U32:	*ptr(U32 *)-=ip2; break;
      case EAT_ADD_U64:	*ptr(I64 *)+=ip2; break;
      case EAT_SUB_U64:	*ptr(I64 *)-=ip2; break;
    }
    Free(tempa);
    tempa=tempa1;
  }
  tempe=tempx->next_ie;
  while (tempe!=&tempx->next_ie) {
    tempe1=tempe->next;
    if (tempe->str) {
      Free(str);
      str=tempe->str;
    }
    switch (tempe->type) {
      case EIT_REL_EXPORT:
      case EIT_IMM_EXPORT:
	tempex=CAlloc(sizeof(CHashExport));
	tempex->str=str;
	str=NULL;
	tempex->type=HTT_EXPORT_SYS_SYM|HTF_IMM;
	if (tempe->type==EIT_IMM_EXPORT)
	  tempex->val=tempe->ip;
	else
	  tempex->val=tempe->ip+ip2;
	tempex->src_link=tempe->src_link;
	tempe->src_link=NULL;
	HashAdd(tempex,Fs->hash_table);
	SysSymImportsResolve(tempex->str);
	break;
      case EIT_REL_I8...EIT_IMM_U64:
	if (tempe->str) {
	  if (tempe->flags&EIF_GOTO_LABEL) {
	    if(!(g_lb=LCCGoToLabelFind(lx,str)))
	      "Unresolved Reference:%s\n",str;
	    else {
	      g_lb->use_cnt++;
	      g_lb=OptLabelFwd(g_lb);
	      i=g_lb->addr+tempx->buf;
	    }
	    tempex=NULL;
	  } else {
	    if (!(tempex=HashFind(str,Fs->hash_table,HTG_ALL-HTT_IMPORT_SYS_SYM)))
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
	  ptr=tempe->ip+ip2;
	  nobound_switch (tempe->type) {
	    case EIT_REL_I8:
	      if (!(MIN_I8<=i-ptr-1<=MAX_I8))
		LexExcept(lx,"branch out of range at ");
	      *ptr(U8 *) =i-ptr-1;
	      break;
	    case EIT_IMM_U8:
	      *ptr(U8 *) =i;
	      break;
	    case EIT_REL_I16:
	      if (!(MIN_I16<=i-ptr-2<=MAX_I16))
		LexExcept(lx,"branch out of range at ");
	      *ptr(U16 *)=i-ptr-2;
	      break;
	    case EIT_IMM_U16:
	      *ptr(U16 *)=i;
	      break;
	    case EIT_REL_I32:
	      if (!(MIN_I32<=i-ptr-4<=MAX_I32))
		LexExcept(lx,"branch out of range at ");
	      *ptr(U32 *)=i-ptr-4;
	      break;
	    case EIT_IMM_U32:
	      *ptr(U32 *)=i;
	      break;
	    case EIT_REL_I64:
	      *ptr(I64 *)=i-ptr-8;
	      break;
	    case EIT_IMM_U64:
	      *ptr(I64 *)=i;
	      break;
	  }
	}
	break;
    }
    Free(tempe->src_link);
    Free(tempe);
    tempe=tempe1;
  }
  Free(str);
  if (!lx->a_depth && Bt(&lx->opts,OPTf_COMPILE_TRACE))
    Un(ip2,tempx->ex_U8s,64);
  QueRem(tempx);
  Free(tempx);
}

U0 CmpFixUpAOTAsm(CLex *lx,CExe *tempx)
{
  CAsmCtrl *a=lx->a;
  I64 i,ip2=tempx->ip+lx->a->ip;
  U8 *ptr;
  CLexCodeMisc *g_lb=NULL;
  CExeAbsAddr *tempa,*tempa1;
  CExeImportExport *tempe,*tempe1;

  tempa=tempx->abss;
  while (tempa) {
    tempa1=tempa->next;
    tempa->next=a->abss;
    ptr=tempx->buf+tempx->ip+tempa->ip;
    nobound_switch (tempa->type) {
      case EAT_ADD_U8:
	*ptr(U8 *)+=ip2;
	break;
      case EAT_SUB_U8:
	*ptr(U8 *)-=ip2;
	break;
      case EAT_ADD_U16:
	*ptr(U16 *)+=ip2;
	break;
      case EAT_SUB_U16:
	*ptr(U16 *)-=ip2;
	break;
      case EAT_ADD_U32:
	*ptr(U32 *)+=ip2;
	break;
      case EAT_SUB_U32:
	*ptr(U32 *)-=ip2;
	break;
      case EAT_ADD_U64:
	*ptr(I64 *)+=ip2;
	break;
      case EAT_SUB_U64:
	*ptr(I64 *)-=ip2;
	break;
    }
    a->abss=tempa;
    tempa->ip+=ip2;
    tempa=tempa1;
  }

  tempe=tempx->next_ie;
  while (tempe!=&tempx->next_ie) {
    tempe1=tempe->next;
    QueRem(tempe);
    if (EIT_REL_I8<=tempe->type<=EIT_IMM_U64) {
      if (tempe->str) {
	if (tempe->flags&EIF_GOTO_LABEL) {
	  if(!(g_lb=LCCGoToLabelFind(lx,tempe->str)))
	    "Unresolved Reference:%s\n",tempe->str;
	  else {
	    g_lb->use_cnt++;
	    g_lb=OptLabelFwd(g_lb);
	  }
	} else
	  g_lb=NULL;
      }
    } else
      g_lb=NULL;

    ptr=tempx->buf+tempx->ip+tempe->ip;
    if (g_lb) {
      i=g_lb->addr+tempx->buf;
      nobound_switch (tempe->type) {
	case EIT_REL_I8:
	  if (!(MIN_I8<=i-ptr-1<=MAX_I8))
	    LexExcept(lx,"branch out of range at ");
	  *ptr(U8 *) =i-ptr-1;
	  break;
	case EIT_IMM_U8:
	  *ptr(U8 *) =i;
	  break;
	case EIT_REL_I16:
	  if (!(MIN_I16<=i-ptr-2<=MAX_I16))
	    LexExcept(lx,"branch out of range at ");
	  *ptr(U16 *)=i-ptr-2;
	  break;
	case EIT_IMM_U16:
	  *ptr(U16 *)=i;
	  break;
	case EIT_REL_I32:
	  if (!(MIN_I32<=i-ptr-4<=MAX_I32))
	    LexExcept(lx,"branch out of range at ");
	  *ptr(U32 *)=i-ptr-4;
	  break;
	case EIT_IMM_U32:
	  *ptr(U32 *)=i;
	  break;
	case EIT_REL_I64:
	  *ptr(I64 *)=i-ptr-8;
	  break;
	case EIT_IMM_U64:
	  *ptr(I64 *)=i;
	  break;
      }
      Free(tempe->src_link);
      Free(tempe);
    } else {
      switch (tempe->type) {
	sub_switch_start:
	  case EIT_REL_EXPORT:
	  case EIT_IMM_EXPORT:
	  case EIT_IMM_U8:
	  case EIT_IMM_U16:
	  case EIT_IMM_U32:
	  case EIT_IMM_U64:
	    break;
	  case EIT_REL_I8:
	    *ptr(U8 *) -=ip2;
	    break;
	  case EIT_REL_I16:
	    *ptr(U16 *)-=ip2;
	    break;
	  case EIT_REL_I32:
	    *ptr(U32 *)-=ip2;
	    break;
	  case EIT_REL_I64:
	    *ptr(I64 *)-=ip2;
	    break;
	sub_switch_end:
	  tempe->ip+=ip2;
	  break;
      }
      tempe->exe=NULL;
      QueIns(tempe,tempx->parent_exe->last_ie);
    }
    tempe=tempe1;
  }
}

I64 Cmp(U8 *filename,U8 *map_name=NULL,U8 *out_name=NULL,U8 mapfile_drv=0)
{ //Compile file, returns err_cnt
  U8 *ptr,*fbuf=NULL,*fbuf2=NULL,*fbuf3=NULL,
     *patch_table=MAlloc(0x20000);
  CExe *tempx;
  I64 i,cnt,size=0,error_cnt=0,warning_cnt=0,ex_U8s=0;
  CBinFile *bfh;
  CExeImportExport *tempe,*tempe1;
  CExeAbsAddr *tempa,*tempa1;
  CExeHeapGlblRef *temphgr,*temphgr1;
  CExeHeapGlbl *temphg,*temphg1;

  fbuf=DftExt(filename,"PRJ.Z");
  fbuf2=MPrint("#include \"%s\"",fbuf);
  if (map_name)
    fbuf3=DftExt(map_name,"MAP.Z");

  if (tempx=CmpBuf(fbuf2,fbuf3,&error_cnt,&warning_cnt,mapfile_drv)) {
    ex_U8s=tempx->ex_U8s;
    ptr=patch_table;
    //See $LK,"CBinFilePatchTableWithoutOrg",A="MN:CBinFilePatchTableWithoutOrg"$
    *ptr++=EIT_MODULE_ALIGNMENT; //See $LK,"Load",A="MN:Load"$() and $LK,"BinRep",A="MN:BinRep"$()
    *ptr(U32 *)++=tempx->max_align;
    *ptr++=0;
    if (tempx->module_org!=INVALID_PTR) {
      //See $LK,"CBinFilePatchTableWithOrg",A="MN:CBinFilePatchTableWithOrg"$
      *ptr++=EIT_MODULE_ORG;
      *ptr(U32 *)++=tempx->module_org;
      *ptr++=0;
    }
    cnt=0;
    tempa=tempx->abss;
    while (tempa) {
      if (!(tempa->type&1))
	cnt++;
      tempa=tempa->next;
    }
    if (cnt) {
      *ptr++=EIT_ABS_ADDR;
      *ptr(U32 *)++=cnt;
      *ptr++=0;
      tempa=tempx->abss;
      while (tempa) {
	tempa1=tempa->next;
	if (!(tempa->type&1))
	  *ptr(U32 *)++ =tempa->ip;
	Free(tempa);
	tempa=tempa1;
      }
    }
    temphg=tempx->heap_glbls;
    while (temphg) {
      temphg1=temphg->next;
      cnt=0;
      temphgr=temphg->references;
      while (temphgr) {
	cnt++;
	temphgr=temphgr->next;
      }
      if (cnt) {
	*ptr++=EIT_HEAP_GLBL;
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
    tempe=tempx->next_ie;
    while (tempe!=&tempx->next_ie) {
      tempe1=tempe->next;
      if (tempe->type<=EIT_IMM_EXPORT) {
	QueRem(tempe);
	*ptr++=tempe->type;
	*ptr(U32 *)++=tempe->ip;
	if (tempe->str) {
	  i=StrLen(tempe->str);
	  MemCpy(ptr,tempe->str,i+1);
	  Free(tempe->str);
	  ptr+=i+1;
	} else
	  *ptr++=0;
	Free(tempe->src_link);
	Free(tempe);
      }
      tempe=tempe1;
    }

//Do imports second
    tempe=tempx->next_ie;
    while (tempe!=&tempx->next_ie) {
      tempe1=tempe->next;
      QueRem(tempe);
      *ptr++=tempe->type;
      if (tempe->exe)
	tempe->ip+=tempe->exe->ip2;
      *ptr(U32 *)++=tempe->ip;
      if (tempe->str) {
	i=StrLen(tempe->str);
	MemCpy(ptr,tempe->str,i+1);
	Free(tempe->str);
	ptr+=i+1;
      } else
	*ptr++=0;
      Free(tempe->src_link);
      Free(tempe);
      tempe=tempe1;
    }

    *ptr++=EIT_END;
    MemSet(ptr,0,16);
    i=ptr-patch_table;
    //Needs 16 ALIGN
    size=(sizeof(CBinFile)+ex_U8s+i+15)&-16;
    bfh=MAlloc(size);
    bfh->jmp=0xEB+256*(sizeof(CBinFile)-2);
#assert sizeof(CBinFile)-2<=MAX_I8
    bfh->type=0; //Reserved. Allow other types, maybe.	Must be 16 bytes, anyway.
    bfh->loader_patch_table_offset=sizeof(CBinFile)+ex_U8s;
    bfh->file_size=size;
    MemCpy(bfh(U8 *)+sizeof(CBinFile),tempx->buf,ex_U8s);
    MemCpy(bfh(U8 *)+sizeof(CBinFile)+ex_U8s,patch_table,size-ex_U8s-sizeof(CBinFile));
    Free(fbuf2);
    if (out_name)
      fbuf2=DftExt(out_name,"BIN.Z");
    else
      fbuf2=ChgExt(fbuf,"BIN.Z");
    if (!cmp_do_not_write_to_dsk)
      FileWrite(fbuf2,bfh,size);
    Free(bfh);
    Free(tempx->buf);
    QueDel(tempx);
    Free(tempx);
  }
  Free(patch_table);
  Free(fbuf);
  Free(fbuf2);
  Free(fbuf3);
  Print("Errs:%d Warns:%d Code:%X Size:%X\n",error_cnt,warning_cnt,ex_U8s,size);
  return error_cnt;
}

I64 ExePutS(U8 *buf,U8 *filename=NULL,Bool just_load=FALSE,CLexHashTableContext *htc=NULL)
{
  I64 result;
  Bool okay=TRUE;
  CLex *lx;
  if (!filename)
    filename=sys_temp_filename;
  lx=LexNew(buf,LF_DONT_FREE_BUF,filename);
  if (just_load)
    lx->flags|=LF_JUST_LOAD;
  if (Fs->last_lex!=&Fs->next_lex) {
    lx->opts=Fs->last_lex->opts;
    if (htc) {
      lx->flags=lx->flags &~LF_ASM_EXPRESSIONS |
	    htc->old_flags&LF_ASM_EXPRESSIONS;
      MemCpy(&lx->htc,htc,sizeof(CLexHashTableContext));
    }
  }
  QueIns(lx,Fs->last_lex);
  try {
    Lex(lx);
    result=ExeCmdLine(lx);
  } catch {
    if (Fs->except_ch=='Compiler' || Fs->except_ch=='Break') {
      Fs->catch_except=TRUE;
      okay=FALSE;
      result=0;
    }
  }
  QueRem(lx);
  if (okay)
    LexDel(lx); //TODO: can crash
  return result;
}

I64 ExePrint(U8 *fmt,...)
{
  I64 result;
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv);
  result=ExePutS(buf);
  Free(buf);
  return result;
}

I64 ExeFile(U8 *name,Bool just_load=FALSE)
{
  I64 result;
  U8 *name2=DftExt(name,"CPP.Z"),
     *st=MPrint("#include \"%s\";",name2);
  result=ExePutS(st,name,just_load);
  Free(st);
  Free(name2);
  return result;
}

I64 ExePutS2(U8 *buf,U8 *filename=NULL,Bool just_load=FALSE)
{ //throws exceptions
  I64 result;
  CLex *lx;
  if (!filename)
    filename=sys_temp_filename;
  lx=LexNew(buf,LF_DONT_FREE_BUF,filename);
  if (just_load)
    lx->flags|=LF_JUST_LOAD;
  if (Fs->last_lex!=&Fs->next_lex)
    lx->opts=Fs->last_lex->opts;
  QueIns(lx,Fs->last_lex);
  Lex(lx);
  result=ExeCmdLine(lx);
  QueRem(lx);
  LexDel(lx);
  return result;
}

I64 ExePrint2(U8 *fmt,...)
{ //throws exceptions
  I64 result;
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv);
  result=ExePutS2(buf);
  Free(buf);
  return result;
}

I64 ExeFile2(U8 *name,Bool just_load=FALSE)
{ //throws exceptions
  I64 result;
  U8 *name2=DftExt(name,"CPP.Z"),
     *st=MPrint("#include \"%s\";",name2);
  result=ExePutS2(st,name,just_load);
  Free(st);
  Free(name2);
  return result;
}

I64 StreamExePrint(U8 *fmt,...)
{//Suppose you want a value from the stream to be used in an #exe block...
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv);
  I64 result=0;
  CLexHashTableContext *htc;
  CLex *lx=Fs->last_lex;
  if (lx==&Fs->next_lex)
    PrintErr("Not Compiling\n");
  else {
    if (!(lx->flags&LF_EXE_BLK))
      LexExcept(lx,"StreamExePrint only allowed in AOT compiled #exe{} mode.");
    if (htc=lx->htc.next)
      result=ExePutS(buf,,,htc);
  }
  Free(buf);
  return result;
}

U0 CmpInit()
{
  compiled_lines=0;
  compiled_funs=0;
  compiled_bytes=0;
  cmp_do_not_write_to_dsk=FALSE;
  CmpLoadDefines;
  CmpFillTables;

  MemSet(&opt_nop,0,sizeof(CIntermediateCode));
  QueInit(&opt_nop);
  opt_nop.ic_class=sys_internal_types[RT_I64];
  opt_nop.ic_code=IC_NOP1;

  AsmHashLoad;
  UnasmHashLoad;
}

CmpInit;
