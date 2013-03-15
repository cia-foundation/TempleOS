U0 StreamPutS(U8 *st)
//Inss text into the stream of
//characters being compiled.  Called
//from withing #exe{} block.
{
  CLex *lx=Fs->last_lex;
  CStreamBlk *tempe=lx->last_stream_blk;
  if (tempe!=&lx->next_stream_blk)
    tempe->body=StrAdd(tempe->body,st);
  else
    PrintFErr("No exe{} blk\n");
}

U0 StreamPrintF(U8 *src,...)
//Inss text into the stream of
//characters being compiled.  Called
//from withing #exe{} block.
{
  U8 *buf=SPrintFJoin(NULL,src,argc,argv);
StreamPutS(buf);
  Free(buf);
}

U8 *CmpStatement(CLex *lx,I64 *type)
{
  I64 size,i,j,k,*result=INVALID_PTR;
  CIntermediateCode *tempi;
  if (type) *type=IT_I64;
  Btr(&lx->flags,Lf_OPT_TRACE_PRESENT);
  if (lx->a_depth==2)
    CBPush(lx);
  CBOutPtrsInit(lx);
  CBICAdd(lx,IC_START,0,0); //So we can scan back an instruction
  tempi=lx->cbh.cb_last_out_ptr;
  if (!PrsStatement(lx,NULL)) {
    if (tempi!=lx->cbh.cb_last_out_ptr) {
      CBICAdd(lx,IC_RETURN_VAL2,0,NULL);
      CBICAdd(lx,IC_RET,0,0);
      CBICAdd(lx,IC_END,0,0);
      if (result=CBCompile(lx,NULL,&size,NULL,NULL,type)) {
	if (lx->flags&LF_AOT_COMPILE) {
	  j=lx->a->ip;
	  k=(size+7)>>3;
	  for (i=0;i<k;i++)
	    AsmStoreCodeU64(lx,result[i]);
	  Free(result);
	  result=j;
	}
      }
    } else
      LinkedLstDel(lx->cbh.cb_out_first);
  } else
    LinkedLstDel(lx->cbh.cb_out_first);
  if (lx->a_depth==2)
    CBPop(lx);
  return result;
}


CExe *CmpJoin(CLex *lx,CHashFun *locals,Bool first_lex,Bool asm_blk=FALSE,
  U8 *map_name=NULL,U8 mapfile_drv=0)
{
  CAsmCtrl *a,*old_a=lx->a;
  I64 i,j,l;
  CAsmCodeBlk *tempc;
  CExeImportExport *tempe;
  Bool okay=TRUE;
  CHashTable *ht1=lx->hash_table_lst,
	       *ht2=lx->define_hash_table,
	       *ht3=lx->glbl_hash_table,
	       *ht4=lx->local_hash_table,
	       *asm_hash_alias=MAllocIdentical(asm_hash);
  CHashFun     *old_var_lst=lx->local_var_lst;
  CExe *result=CAlloc(sizeof(CExe));
  result->parent_exe=lx->exe_struct;
  lx->exe_struct=result;

  result->next_ie=result->last_ie=&result->next_ie;
  lx->a=a=CAlloc(sizeof(CAsmCtrl));
  lx->a_depth++;

  a->code=CAlloc(sizeof(CAsmCodeBlk));
  a->max_align=1;
  a->module_org=INVALID_PTR;

  lx->local_hash_table=HashTableNew(16);
  lx->glbl_hash_table =HashTableNew(2048);

  asm_hash_alias->next=lx->local_hash_table;
  lx->local_hash_table->next=lx->glbl_hash_table;

  if (lx->flags&LF_AOT_COMPILE)
    lx->define_hash_table=lx->glbl_hash_table;
  else
    lx->glbl_hash_table->next=Fs->hash_table;
  lx->hash_table_lst =asm_hash_alias;

  lx->local_var_lst=locals; //C+ local vars
  try {
    if (first_lex)
      Lex(lx);
    if (lx->flags&LF_AOT_COMPILE) {
      while (lx->token!=TK_EOF) {
	i=CmpStatement(lx,NULL);
	if (i!=INVALID_PTR) {
	  tempe=CAlloc(sizeof(CExeImportExport));
	  tempe->type=EIE_MAIN;
	  tempe->ip=i;
	  QueIns(tempe,result->last_ie);
	}
	if (asm_blk)
	  break;
      }
    } else
      PrsStatement(lx,NULL);
    AsmGlblsResolve(lx,result);
  } catch {
    if (Fs->except_argc>0 && Fs->except_argv[0]==EXCEPT_COMPILER &&
      !asm_blk) {
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
    LinkedLstDel(a->code);
    result=NULL;
  } else {
    if (map_name && !cmp_do_not_write_to_dsk)
      HashRep(lx->glbl_hash_table,HTG_ALL,map_name,"-r-p",mapfile_drv);
    HashTableDel(lx->local_hash_table);
    HashTableDel(lx->glbl_hash_table);

    if (!a->num_code_U8s)
      result->code=NULL;
    else {
      if (lx->flags&LF_AOT_COMPILE)
	result->code=MAlloc(a->num_code_U8s);
      else {
	if (a->module_org==INVALID_PTR)
	  result->code=MAlloc(a->num_code_U8s,Fs->code_heap);
	else
	  result->code=a->module_org;
      }
      result->code_U8s=a->num_code_U8s;
      tempc=a->code;
      j=0;
      l=a->num_code_U8s;
      while (tempc) {
	i=l;
	if (i>ASM_CODE_BLK_SIZE)
	  i=ASM_CODE_BLK_SIZE;
	MemCpy(result->code+j,tempc->body,i);
	j+=i;
	l-=i;
	tempc=tempc->next;
      }
    }
    LinkedLstDel(a->code);
    result->abss=a->abss;
    result->heap_glbls=a->heap_glbls;
    result->max_align =a->max_align;
    result->module_org=a->module_org;
    lx->exe_struct=result->parent_exe;
  }
  lx->hash_table_lst  =ht1,
  lx->define_hash_table=ht2;
  lx->glbl_hash_table  =ht3;
  lx->local_hash_table =ht4;
  lx->local_var_lst=old_var_lst;
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
  lx=LexNew(buf,LF_DONT_FREE_BUF,NULL);
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

U0 CmpFixUpJITAsm(CLex *lx,CExe *ex,I64 ip2)
{
  I64 i,*str=NULL;
  U8 *ptr;
  CExeAbsAddr *tempa,*tempa1;
  CExeImportExport *tempe,*tempe1;
  CHashExport *tempess;
  CHashFun *tempf;
  CHashGlblVar *tempg;

  tempa=ex->abss;
  while (tempa) {
    tempa1=tempa->next;
    nobound_switch (tempa->type) {
      case EAA_ADD_U8:
	*(ex->code(U8 *)+tempa->ip)(U8 *)+=ip2;
	break;
      case EAA_SUB_U8:
	*(ex->code(U8 *)+tempa->ip)(U8 *)-=ip2;
	break;
      case EAA_ADD_U16:
	*(ex->code(U8 *)+tempa->ip)(U16 *)+=ip2;
	break;
      case EAA_SUB_U16:
	*(ex->code(U8 *)+tempa->ip)(U16 *)-=ip2;
	break;
      case EAA_ADD_U32:
	*(ex->code(U8 *)+tempa->ip)(U32 *)+=ip2;
	break;
      case EAA_SUB_U32:
	*(ex->code(U8 *)+tempa->ip)(U32 *)-=ip2;
	break;
      case EAA_ADD_U64:
	*(ex->code(U8 *)+tempa->ip)(I64 *)+=ip2;
	break;
      case EAA_SUB_U64:
	*(ex->code(U8 *)+tempa->ip)(I64 *)-=ip2;
	break;
    }
    Free(tempa);
    tempa=tempa1;
  }
  tempe=ex->next_ie;
  while (tempe!=&ex->next_ie) {
    tempe1=tempe->next;
    if (tempe->str) {
      Free(str);
      str=tempe->str;
    }
    if (tempe->type<=EIE_LIT_EXPORT) {
      tempess=CAlloc(sizeof(CHashExport));
      tempess->str=str;
      str=NULL;
      tempess->type=HTT_EXPORT_SYS_SYM|HTF_LIT;
      if (tempe->type==EIE_LIT_EXPORT)
	tempess->val=tempe->ip;
      else
	tempess->val=tempe->ip+ip2;
      tempess->src_link=tempe->src_link;
      tempe->src_link=NULL;
      if (IsReplaceSyms)
	HashReplace(tempess,Fs->hash_table);
      else
	HashAdd(tempess,Fs->hash_table);
      SysSymImportsResolve(tempess->str);
    } else if (tempe->type<EIE_MAIN) {
      if (tempe->str && !(tempess=HashFind(str,Fs->hash_table,HTG_ALL-HTT_IMPORT_SYS_SYM)))
	"%S%s\n","ST_UNRESOLVED_REFERENCE",str;
      else {
	ptr=tempe->ip+ip2;
	if (tempess->type & HTT_FUN) {
	  tempf=tempess;
	  if (tempe->type & 1)
	    i=tempf->executable_addr;
	  else
	    i=tempf->executable_addr-ex->code;
	} else if (tempess->type & HTT_GLBL_VAR) {
	  tempg=tempess;
	  if (tempe->type & 1)
	    i=tempg->data_addr;
	  else
	    i=tempg->data_addr-ex->code;
	} else {
	  if (tempe->type & 1)
	    i=tempess->val;
	  else
	    i=tempess->val(U8 *)-ex->code;
	}
	if (tempe->type<=EIE_LIT_U8)
	  *ptr(U8 *)+=i;
	else if (tempe->type<=EIE_LIT_U16)
	  *ptr(U16 *)+=i;
	else if (tempe->type<=EIE_LIT_U32)
	  *ptr(U32 *)+=i;
	else if (tempe->type<=EIE_LIT_U64)
	  *ptr(I64 *)+=i;
      }
    }
    Free(tempe->src_link);
    Free(tempe);
    tempe=tempe1;
  }
  Free(str);
  if (!lx->a_depth && Bt(&lx->opts,OPTf_COMPILE_TRACE))
    Un(ex->code,ex->code_U8s,64);
  Free(ex);
}

U0 CmpFixUpAOTAsm(CLex *lx,CExe *ex,I64 ip2)
{
  CAsmCtrl *a=lx->a;
  U8 *ptr;
  CExeAbsAddr *tempa,*tempa1;
  CExeImportExport *tempe,*tempe1;

  tempa=ex->abss;
  while (tempa) {
    tempa1=tempa->next;
    tempa->next=a->abss;
    nobound_switch (tempa->type) {
      case EAA_ADD_U8:
	*(ex->code(U8 *)+tempa->ip)(U8 *)+=ip2;
	break;
      case EAA_SUB_U8:
	*(ex->code(U8 *)+tempa->ip)(U8 *)-=ip2;
	break;
      case EAA_ADD_U16:
	*(ex->code(U8 *)+tempa->ip)(U16 *)+=ip2;
	break;
      case EAA_SUB_U16:
	*(ex->code(U8 *)+tempa->ip)(U16 *)-=ip2;
	break;
      case EAA_ADD_U32:
	*(ex->code(U8 *)+tempa->ip)(U32 *)+=ip2;
	break;
      case EAA_SUB_U32:
	*(ex->code(U8 *)+tempa->ip)(U32 *)-=ip2;
	break;
      case EAA_ADD_U64:
	*(ex->code(U8 *)+tempa->ip)(I64 *)+=ip2;
	break;
      case EAA_SUB_U64:
	*(ex->code(U8 *)+tempa->ip)(I64 *)-=ip2;
	break;
    }
    a->abss=tempa;
    tempa->ip+=ip2;
    tempa=tempa1;
  }

  tempe=ex->next_ie;
  while (tempe!=&ex->next_ie) {
    tempe1=tempe->next;
    QueRem(tempe);
    if (tempe->type<=EIE_LIT_EXPORT)
      tempe->ip+=ip2;
    else if (tempe->type<EIE_MAIN) {
      ptr=ex->code(U8 *)+tempe->ip;
      if (tempe->type<=EIE_LIT_U8)
	*ptr(U8 *)-=ip2;
      else if (tempe->type<=EIE_LIT_U16)
	*ptr(U16 *)-=ip2;
      else if (tempe->type<=EIE_LIT_U32)
	*ptr(U32 *)-=ip2;
      else if (tempe->type<=EIE_LIT_U64)
	*ptr(I64 *)-=ip2;
      tempe->ip+=ip2;
    }
    QueIns(tempe,ex->parent_exe->last_ie);
    tempe=tempe1;
  }
  Free(ex);
}

I64 Cmp(U8 *filename,U8 *map_name=NULL,U8 *out_name=NULL,U8 mapfile_drv=0)
{ //Compile file, returns err_cnt
  U8 *ptr,*fbuf=NULL,*fbuf2=NULL,*fbuf3=NULL,
     *patch_table=MAlloc(0x20000);
  CExe *ex;
  I64 i,cnt,size=0,error_cnt=0,warning_cnt=0,code_U8s=0;
  CBinFile *bfh;
  CExeImportExport *tempe,*tempe1;
  CExeAbsAddr *tempa,*tempa1;
  CExeHeapGlblRef *temphgr,*temphgr1;
  CExeHeapGlbl *temphg,*temphg1;

  fbuf=DftExt(filename,"PRJ.Z");
  fbuf2=MSPrintF("#include \"%s\"",fbuf);
  if (map_name)
    fbuf3=DftExt(map_name,"MAP.Z");
  ex=CmpBuf(fbuf2,fbuf3,&error_cnt,&warning_cnt,mapfile_drv);

  if (ex) {
    code_U8s=ex->code_U8s;
    ptr=patch_table;
    //See $LK,"CBinFilePatchTableWithoutOrg","MN:CBinFilePatchTableWithoutOrg"$
    *ptr++=EIE_MODULE_ALIGNMENT; //See $LK,"Load","MN:Load"$() and $LK,"BinRep","MN:BinRep"$()
    *ptr(U32 *)++=ex->max_align;
    *ptr++=0;
    if (ex->module_org!=INVALID_PTR) {
      //See $LK,"CBinFilePatchTableWithOrg","MN:CBinFilePatchTableWithOrg"$
      *ptr++=EIE_MODULE_ORG;
      *ptr(U32 *)++=ex->module_org;
      *ptr++=0;
    }
    cnt=0;
    tempa=ex->abss;
    while (tempa) {
      if (!(tempa->type&1))
	cnt++;
      tempa=tempa->next;
    }
    if (cnt) {
      *ptr++=EIE_ABS_ADDR;
      *ptr(U32 *)++=cnt;
      *ptr++=0;
      tempa=ex->abss;
      while (tempa) {
	tempa1=tempa->next;
	if (!(tempa->type&1))
	  *ptr(U32 *)++ =tempa->ip;
	Free(tempa);
	tempa=tempa1;
      }
    }
    temphg=ex->heap_glbls;
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
    tempe=ex->next_ie;
    while (tempe!=&ex->next_ie) {
      tempe1=tempe->next;
      if (tempe->type<=EIE_LIT_EXPORT) {
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
    tempe=ex->next_ie;
    while (tempe!=&ex->next_ie) {
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
    //$LK,"NATIVE_XSUM","FF:::/TempleOS/Kernel/KStart.CPP,NATIVE_XSUM"$ needs 16 ALIGN
    size=(sizeof(CBinFile)+code_U8s+i+15)&-16;
    bfh=MAlloc(size);
    bfh->jmp=0xEB+256*(sizeof(CBinFile)-2);
#assert sizeof(CBinFile)-2<=MAX_I8
    bfh->type=0;
    bfh->loader_patch_table_offset=sizeof(CBinFile)+code_U8s;
    bfh->file_size=size;
    bfh->xsum=NATIVE_XSUM;
    MemCpy(bfh(U8 *)+sizeof(CBinFile),ex->code,code_U8s);
    MemCpy(bfh(U8 *)+sizeof(CBinFile)+code_U8s,patch_table,
      size-code_U8s-sizeof(CBinFile));
    bfh->xsum=ArcXSum(bfh,size);
    Free(fbuf2);
    if (out_name)
      fbuf2=DftExt(out_name,"BIN.Z");
    else
      fbuf2=ChgExt(fbuf,"BIN.Z");
    if (!cmp_do_not_write_to_dsk)
      FileWrite(fbuf2,bfh,size);
    Free(bfh);
    Free(ex->code);
    Free(ex);
  }
  Free(patch_table);
  Free(fbuf);
  Free(fbuf2);
  Free(fbuf3);
  PrintF(Define("ST_ERRORS_WARNINGS"),error_cnt,warning_cnt,code_U8s,size);
  return error_cnt;
}

I64 ExePutS(U8 *buf,U8 *filename=NULL,Bool just_load=FALSE)
{
  I64 result;
  Bool okay=TRUE;
  CLex *lx;
  if (!filename)
    filename=sys_temp_filename;
  lx=LexNew(buf,LF_DONT_FREE_BUF,FileNameAbs(filename));
  if (just_load)
    lx->flags|=LF_JUST_LOAD;
  if (Fs->last_lex!=&Fs->next_lex)
    lx->opts=Fs->last_lex->opts;
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

I64 ExePrintF(U8 *fmt,...)
{
  I64 result;
  U8 *buf=SPrintFJoin(NULL,fmt,argc,argv);
  result=ExePutS(buf);
  Free(buf);
  return result;
}

I64 ExeFile(U8 *name,Bool just_load=FALSE)
{
  I64 result;
  U8 *name2=DftExt(name,"CPP.Z"),
     *st=MSPrintF("#include \"%s\";",name2);
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
  lx=LexNew(buf,LF_DONT_FREE_BUF,FileNameAbs(filename));
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

I64 ExePrintF2(U8 *fmt,...)
{ //with REPLACE_SYMS
  I64 result;
  U8 *buf=SPrintFJoin(NULL,fmt,argc,argv);
  result=ExePutS2(buf);
  Free(buf);
  return result;
}

I64 ExeFile2(U8 *name,Bool just_load=FALSE)
{ //with REPLACE_SYMS
  I64 result;
  U8 *name2=DftExt(name,"CPP.Z"),
     *st=MSPrintF("#include \"%s\";",name2);
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
  lx=LexNew(buf,LF_DONT_FREE_BUF,FileNameAbs(filename));
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

I64 ExePrintF3(U8 *fmt,...)
{ //throws exceptions
  I64 result;
  U8 *buf=SPrintFJoin(NULL,fmt,argc,argv);
  result=ExePutS3(buf);
  Free(buf);
  return result;
}


I64 ExeFile3(U8 *name,Bool just_load=FALSE)
{ //throws exceptions
  I64 result;
  U8 *name2=DftExt(name,"CPP.Z"),
     *st=MSPrintF("#include \"%s\";",name2);
  result=ExePutS3(st,name,just_load);
  Free(st);
  Free(name2);
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
  opt_nop.ic_class=sys_internal_types[IT_I64];

  opt_nop.ic_opcode=IC_NOP;
  AsmHashLoad(is_first_compiler);
  UnasmHashLoad;
}

CmpInit;
