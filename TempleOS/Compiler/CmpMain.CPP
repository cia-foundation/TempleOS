U0 StreamPrint(U8 *fmt,...)
//Inserts text into the stream of
//characters being compiled.  Called
//from withing #exe{} block.
{
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv);
  CLex *lx=Fs->last_lex;
  CStreamBlk *tempe=lx->last_stream_blk;
  if (tempe!=&lx->next_stream_blk)
    tempe->body=StrAdd(tempe->body,buf);
  else
    PrintErr("No exe{} blk\n");
  Free(buf);
}

U8 *LexStatement2Bin(CLex *lx,I64 *type)
{
  I64 size,i,j,k,*result=INVALID_PTR;
  if (type) *type=IT_I64;
  Btr(&lx->flags,Lf_OPT_TRACE_PRESENT);
  if (lx->a_depth==2)
    LCCPush(lx);
  LCCInit(lx);
  if (!PrsStatement(lx,NULL)) {
    if (lx->lcc.lcc_root.next!=&lx->lcc.lcc_root) {
      LCCAdd(lx,IC_RETURN_VAL2,0,NULL);
      LCCAdd(lx,IC_RET,0,0);
      if (result=LCCCompile(lx,NULL,&size,NULL,type)) {
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
  if (lx->a_depth==2)
    LCCPop(lx);
  return result;
}


CExe *CmpJoin(CLex *lx,CHashFun *locals,Bool first_lex,Bool asm_blk=FALSE,
  U8 *map_name=NULL,U8 mapfile_drv=0)
{
  CAsmCtrl *a,*old_a=lx->a;
  I64 i,j,l;
  U8 *ex;
  CAsmCodeBlk *tempc;
  CExeImportExport *tempe;
  Bool okay=TRUE;
  CLexHashTableContext *htc=MAlloc(sizeof(CLexHashTableContext));
  CHashTable   *asm_hash_alias=MAllocIdentical(asm_hash);
  CExe *result=CAlloc(sizeof(CExe));
  result->parent_exe=lx->exe_struct;
  lx->exe_struct=result;

  result->next_ie=result->last_ie=&result->next_ie;
  lx->a=a=CAlloc(sizeof(CAsmCtrl));
  lx->a_depth++;

  a->cb=CAlloc(sizeof(CAsmCodeBlk));
  a->max_align=1;
  a->module_org=INVALID_PTR;

  MemCpy(htc,&lx->htc,sizeof(CLexHashTableContext));
  lx->htc.local_hash_table=HashTableNew(16);
  lx->htc.glbl_hash_table =HashTableNew(2048);

  asm_hash_alias->next=lx->htc.local_hash_table;
  lx->htc.local_hash_table->next=lx->htc.glbl_hash_table;

  if (lx->flags&LF_AOT_COMPILE)
    lx->htc.define_hash_table=lx->htc.glbl_hash_table;
  else
    lx->htc.glbl_hash_table->next=Fs->hash_table;
  lx->htc.hash_table_lst =asm_hash_alias;

  lx->htc.local_var_lst=locals; //HolyC local vars
  try {
    if (first_lex)
      Lex(lx);
    if (lx->flags&LF_AOT_COMPILE) {
      while (lx->token!=TK_EOF) {
	ex=LexStatement2Bin(lx,NULL);
	if (ex!=INVALID_PTR) {
	  tempe=CAlloc(sizeof(CExeImportExport));
	  tempe->type=EIE_MAIN;
	  tempe->ip=ex;
	  QueIns(tempe,result->last_ie);
	}
	if (asm_blk)
	  break;
      }
    } else
      PrsStatement(lx,NULL);
    AsmGlblsResolve(lx,result);
  } catch {
    if (Fs->except_argc>0 && Fs->except_argv[0]==EXCEPT_COMPILER && !asm_blk) {
      if (Fs->except_argc>1 && Fs->except_argv[1]) {
	"Compiler exception #%d\n",Fs->except_argv[1];
	LexPutPos(lx);
      }
      Fs->catch_except=TRUE;
    }
    okay=FALSE;
  }
  if (!okay) {
    if (lx->error_cnt<1)
      lx->error_cnt=1;
    lx->exe_struct=result->parent_exe;
    Free(result);
    LinkedLstDel(a->cb);
    result=NULL;
  } else {
    if (map_name && !cmp_do_not_write_to_dsk)
      Who("-r",lx->htc.glbl_hash_table,map_name,mapfile_drv);
    HashTableDel(lx->htc.local_hash_table);
    HashTableDel(lx->htc.glbl_hash_table);

    if (!a->num_ex_U8s)
      result->ex=NULL;
    else {
      if (lx->flags&LF_AOT_COMPILE)
	result->ex=MAlloc(a->num_ex_U8s);
      else {
	if (a->module_org==INVALID_PTR)
	  result->ex=MAlloc(a->num_ex_U8s,Fs->code_heap);
	else
	  result->ex=a->module_org;
      }
      result->ex_U8s=a->num_ex_U8s;
      tempc=a->cb;
      j=0;
      l=a->num_ex_U8s;
      while (tempc) {
	i=l;
	if (i>ASM_CODE_BLK_SIZE)
	  i=ASM_CODE_BLK_SIZE;
	MemCpy(result->ex+j,tempc->body,i);
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
    lx->exe_struct=result->parent_exe;
  }
  MemCpy(&lx->htc,htc,sizeof(CLexHashTableContext));
  Free(htc);
  Free(asm_hash_alias);
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
  if (!is_first_compiler)
    "$$FG,PURPLE$$**** Not First Compiler ****$$FG$$\n";
  lx=LexNew(buf,LF_DONT_FREE_BUF);
  lx->flags|=LF_AOT_COMPILE;
  QueIns(lx,Fs->last_lex);
  result=CmpJoin(lx,NULL,TRUE,FALSE,map_name,mapfile_drv);
  if (error_cnt) *error_cnt=lx->error_cnt;
  if (warning_cnt) *warning_cnt=lx->warning_cnt;
  QueRem(lx);
  if (result)
    LexDel(lx);
  return result;
}

U0 CmpFixUpJITAsm(CLex *lx,CExe *tempx,I64 ip2)
{
  I64 i,*str=NULL;
  U8 *ptr;
  CExeAbsAddr *tempa,*tempa1;
  CExeImportExport *tempe,*tempe1;
  CHashExport *tempex;

  tempa=tempx->abss;
  while (tempa) {
    tempa1=tempa->next;
    ptr=tempx->ex+tempa->ip;
    nobound_switch (tempa->type) {
      case EAA_ADD_U8:
	*ptr(U8 *)+=ip2;
	break;
      case EAA_SUB_U8:
	*ptr(U8 *)-=ip2;
	break;
      case EAA_ADD_U16:
	*ptr(U16 *)+=ip2;
	break;
      case EAA_SUB_U16:
	*ptr(U16 *)-=ip2;
	break;
      case EAA_ADD_U32:
	*ptr(U32 *)+=ip2;
	break;
      case EAA_SUB_U32:
	*ptr(U32 *)-=ip2;
	break;
      case EAA_ADD_U64:
	*ptr(I64 *)+=ip2;
	break;
      case EAA_SUB_U64:
	*ptr(I64 *)-=ip2;
	break;
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
      case EIE_REL_EXPORT:
      case EIE_IMM_EXPORT:
	tempex=CAlloc(sizeof(CHashExport));
	tempex->str=str;
	str=NULL;
	tempex->type=HTT_EXPORT_SYS_SYM|HTF_IMM;
	if (tempe->type==EIE_IMM_EXPORT)
	  tempex->val=tempe->ip;
	else
	  tempex->val=tempe->ip+ip2;
	tempex->src_link=tempe->src_link;
	tempe->src_link=NULL;
	if (IsReplaceSyms)
	  HashReplace(tempex,Fs->hash_table);
	else
	  HashAdd(tempex,Fs->hash_table);
	SysSymImportsResolve(tempex->str);
	break;
      case EIE_REL_U8...EIE_IMM_U64:
	if (tempe->str && !(tempex=HashFind(str,Fs->hash_table,HTG_ALL-HTT_IMPORT_SYS_SYM)))
	  "%S%s\n","ST_UNRESOLVED_REFERENCE",str;
	else {
	  ptr=tempe->ip+ip2;
	  if (tempex->type & HTT_FUN)
	    i=tempex(CHashFun *)->executable_addr;
	  else if (tempex->type & HTT_GLBL_VAR)
	    i=tempex(CHashGlblVar *)->data_addr;
	  else
	    i=tempex->val;
	  nobound_switch (tempe->type) {
	    case EIE_REL_U8:  *ptr(U8 *) =i-ptr-1; break;
	    case EIE_IMM_U8:  *ptr(U8 *) =i;  break;
	    case EIE_REL_U16: *ptr(U16 *)=i-ptr-2; break;
	    case EIE_IMM_U16: *ptr(U16 *)=i; break;
	    case EIE_REL_U32: *ptr(U32 *)=i-ptr-4; break;
	    case EIE_IMM_U32: *ptr(U32 *)=i; break;
	    case EIE_REL_U64: *ptr(I64 *)=i-ptr-8; break;
	    case EIE_IMM_U64: *ptr(I64 *)=i; break;
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
    Un(tempx->ex,tempx->ex_U8s,64);
  Free(tempx);
}

U0 CmpFixUpAOTAsm(CLex *lx,CExe *tempx,I64 ip2)
{
  CAsmCtrl *a=lx->a;
  U8 *ptr;
  CExeAbsAddr *tempa,*tempa1;
  CExeImportExport *tempe,*tempe1;

  tempa=tempx->abss;
  while (tempa) {
    tempa1=tempa->next;
    tempa->next=a->abss;
    ptr=tempx->ex+tempa->ip;
    nobound_switch (tempa->type) {
      case EAA_ADD_U8:
	*ptr(U8 *)+=ip2;
	break;
      case EAA_SUB_U8:
	*ptr(U8 *)-=ip2;
	break;
      case EAA_ADD_U16:
	*ptr(U16 *)+=ip2;
	break;
      case EAA_SUB_U16:
	*ptr(U16 *)-=ip2;
	break;
      case EAA_ADD_U32:
	*ptr(U32 *)+=ip2;
	break;
      case EAA_SUB_U32:
	*ptr(U32 *)-=ip2;
	break;
      case EAA_ADD_U64:
	*ptr(I64 *)+=ip2;
	break;
      case EAA_SUB_U64:
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
    ptr=tempx->ex+tempe->ip;
    switch (tempe->type) {
      case EIE_REL_EXPORT:
      case EIE_IMM_EXPORT:
	tempe->ip+=ip2;
	break;
      case EIE_REL_U8:
      case EIE_IMM_U8:
	*ptr(U8 *) -=ip2;
	tempe->ip+=ip2;
	break;
      case EIE_REL_U16:
      case EIE_IMM_U16:
	*ptr(U16 *)-=ip2;
	tempe->ip+=ip2;
	break;
      case EIE_REL_U32:
      case EIE_IMM_U32:
	*ptr(U32 *)-=ip2;
	tempe->ip+=ip2;
	break;
      case EIE_REL_U64:
      case EIE_IMM_U64:
	*ptr(I64 *)-=ip2;
	tempe->ip+=ip2;
	break;
    }
    QueIns(tempe,tempx->parent_exe->last_ie);
    tempe=tempe1;
  }
  Free(tempx);
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
  tempx=CmpBuf(fbuf2,fbuf3,&error_cnt,&warning_cnt,mapfile_drv);

  if (tempx) {
    ex_U8s=tempx->ex_U8s;
    ptr=patch_table;
    //See $LK,"CBinFilePatchTableWithoutOrg","MN:CBinFilePatchTableWithoutOrg"$
    *ptr++=EIE_MODULE_ALIGNMENT; //See $LK,"Load","MN:Load"$() and $LK,"BinRep","MN:BinRep"$()
    *ptr(U32 *)++=tempx->max_align;
    *ptr++=0;
    if (tempx->module_org!=INVALID_PTR) {
      //See $LK,"CBinFilePatchTableWithOrg","MN:CBinFilePatchTableWithOrg"$
      *ptr++=EIE_MODULE_ORG;
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
      *ptr++=EIE_ABS_ADDR;
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
	*ptr++=EIE_HEAP_GLBL;
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
      if (tempe->type<=EIE_IMM_EXPORT) {
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

    *ptr++=EIE_END;
    MemSet(ptr,0,16);
    i=ptr-patch_table;
    //$LK,"TEMPLEOS_XSUM","FF:::/TempleOS/Kernel/KStart.CPP,TEMPLEOS_XSUM"$ needs 16 ALIGN
    size=(sizeof(CBinFile)+ex_U8s+i+15)&-16;
    bfh=MAlloc(size);
    bfh->jmp=0xEB+256*(sizeof(CBinFile)-2);
#assert sizeof(CBinFile)-2<=MAX_I8
    bfh->type=0;
    bfh->loader_patch_table_offset=sizeof(CBinFile)+ex_U8s;
    bfh->file_size=size;
    bfh->xsum=TEMPLEOS_XSUM;
    MemCpy(bfh(U8 *)+sizeof(CBinFile),tempx->ex,ex_U8s);
    MemCpy(bfh(U8 *)+sizeof(CBinFile)+ex_U8s,patch_table,
      size-ex_U8s-sizeof(CBinFile));
    bfh->xsum=ArcXSum(bfh,size);
    Free(fbuf2);
    if (out_name)
      fbuf2=DftExt(out_name,"BIN.Z");
    else
      fbuf2=ChgExt(fbuf,"BIN.Z");
    if (!cmp_do_not_write_to_dsk)
      FileWrite(fbuf2,bfh,size);
    Free(bfh);
    ExpressionBinDel(tempx->ex);
    Free(tempx);
  }
  Free(patch_table);
  Free(fbuf);
  Free(fbuf2);
  Free(fbuf3);
  Print(Define("ST_ERRORS_WARNINGS"),error_cnt,warning_cnt,ex_U8s,size);
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
    if (Fs->except_argc>0) {
      if (Fs->except_argv[0]==EXCEPT_COMPILER ||
      Fs->except_argv[0]==EXCEPT_BREAK) {
	Fs->catch_except=TRUE;
	okay=FALSE;
	result=0;
      }
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
{ //with REPLACE_SYMS
  I64 result;
  Bool okay=TRUE;
  CLex *lx;
  if (!filename)
    filename=sys_temp_filename;
  lx=LexNew(buf,LF_DONT_FREE_BUF,filename);
  if (just_load)
    lx->flags|=LF_JUST_LOAD;
  if (Fs->last_lex!=&Fs->next_lex)
    lx->opts=Fs->last_lex->opts|1<<OPTf_REPLACE_SYMS;
  QueIns(lx,Fs->last_lex);
  try {
    Lex(lx);
    result=ExeCmdLine(lx);
  } catch {
    if (Fs->except_argc>0) {
      if (Fs->except_argv[0]==EXCEPT_COMPILER ||
      Fs->except_argv[0]==EXCEPT_BREAK) {
	Fs->catch_except=TRUE;
	okay=FALSE;
	result=0;
      }
    }
  }
  QueRem(lx);
  if (okay)
    LexDel(lx); //TODO: can crash
  return result;
}

I64 ExePrint2(U8 *fmt,...)
{ //with REPLACE_SYMS
  I64 result;
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv);
  result=ExePutS2(buf);
  Free(buf);
  return result;
}

I64 ExeFile2(U8 *name,Bool just_load=FALSE)
{ //with REPLACE_SYMS
  I64 result;
  U8 *name2=DftExt(name,"CPP.Z"),
     *st=MPrint("#include \"%s\";",name2);
  result=ExePutS2(st,name,just_load);
  Free(st);
  Free(name2);
  return result;
}

I64 ExePutS3(U8 *buf,U8 *filename=NULL,Bool just_load=FALSE)
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

I64 ExePrint3(U8 *fmt,...)
{ //throws exceptions
  I64 result;
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv);
  result=ExePutS3(buf);
  Free(buf);
  return result;
}

I64 ExeFile3(U8 *name,Bool just_load=FALSE)
{ //throws exceptions
  I64 result;
  U8 *name2=DftExt(name,"CPP.Z"),
     *st=MPrint("#include \"%s\";",name2);
  result=ExePutS3(st,name,just_load);
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
  if (lx==&Fs->next_lex) {
    PutErr;
    "Not Compiling\n";
  } else {
    if (!(lx->flags&LF_EXE_BLK))
      LexExcept(lx,"ST_STREAM_EXE_PRINT_NOT_ALLOWED");
    if (htc=lx->htc.next)
      result=ExePutS(buf,,,htc);
  }
  Free(buf);
  return result;
}

U0 CmpInit()
{
  if (!sys_num_installed_compilers++) {
    is_first_compiler=TRUE;
    fp_prs_stream_blk=&PrsStreamBlk;
  } else
    is_first_compiler=FALSE;
  compiled_lines=0;
  compiled_funs=0;
  compiled_bytes=0;
  cmp_do_not_write_to_dsk=FALSE;
  CmpLoadDefines;
  CmpFillTables;

  MemSet(&opt_nop,0,sizeof(CIntermediateCode));
  QueInit(&opt_nop);
  opt_nop.ic_class=sys_internal_types[IT_I64];
  opt_nop.ic_opcode=IC_NOP1;

  AsmHashLoad(is_first_compiler);
  UnasmHashLoad;
}

CmpInit;
