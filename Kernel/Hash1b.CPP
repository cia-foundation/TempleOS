I64 HashTypeNum(CHash *temph)
{//Return bit num of hash type, limited to just types.
  if (temph)
    return Bsf(temph->type&HTG_TYPE_MASK);
  else
    return -1;
}

I64 HashVal(CHash *temph)
{//Returns most likely desired value.
  nobound_switch (HashTypeNum(temph)) {
    case HTt_EXPORT_SYS_SYM:
      return temph(CHashExport *)->val;
    case HTt_IMPORT_SYS_SYM:
      return temph(CHashImport *)->module_base;
    case HTt_DEFINE_STR:
    case HTt_CLASS:
    case HTt_INTERNAL_TYPE:
    case HTt_WORD:
    case HTt_DICT_WORD:
    case HTt_OPCODE:
    case HTt_HELP_FILE:
      return temph;
    case HTt_GLBL_VAR:
      if (temph(CHashGlblVar *)->flags&GVF_EXTERN)
	return &temph(CHashGlblVar *)->data_addr;
      else
	return temph(CHashGlblVar *)->data_addr;
    case HTt_FUN:
      if (Bt(&temph(CHashFun *)->flags,Cf_EXTERN))
	return temph;
      else
	return temph(CHashFun *)->executable_addr;
    case HTt_REG:
      return temph(CHashReg *)->reg_num|temph(CHashReg *)->reg_type<<8;
    case HTt_KEYWORD:
    case HTt_ASM_KEYWORD:
    case HTt_MODULE:
    case HTt_FILE:
    case HTt_FRAME_PTR:
      return temph(CHashGeneric *)->user_data0;

    case -1:		//nobound_switch
    case HTt_NUM_TYPES: //nobound_switch
    default:
      return 0;
  }
}

CHashTable *HashTableNew(I64 size,CTask *mem_task=NULL)
{//New hash table, power-of-two in size.
  CHashTable *table;
  table=CAlloc(sizeof(CHashTable),mem_task);
  table->body=CAlloc(size<<3,mem_task);
  table->mask=size-1;
  table->next_added=table->last_added=table;
  return table;
}

U0 HashDel(CHashSrcSym *temph)
{//Free a std TempleOS system hash entry.
  if (!temph) return;
  if (!(temph->type&HTT_DICT_WORD))
    Free(temph->str);
  if (temph->type & HTG_SRC_SYM) {
    Free(temph->src_link);
    Free(temph->index);
    Free(temph->import_name);
    LinkedLstDel(temph->ie_lst);
    if (temph->type & (HTT_FUN | HTT_EXPORT_SYS_SYM))
      Free(temph->dbg_info);
    if (temph->type & (HTT_FUN | HTT_CLASS))
    //Assumes code not on heap, so doesn't Free.
    //$LK,"MemberLstDel",A="MN:MemberLstDel"$() is an import to the Kernel module
      MemberLstDel(temph);
    else if (temph->type&HTT_DEFINE_STR)
      Free(temph(CHashDefineStr *)->data);
    else if (temph->type & HTT_GLBL_VAR) {
      if (!(temph(CHashGlblVar *)->flags&GVF_ALIAS))
	Free(temph(CHashGlblVar *)->data_addr);
      LinkedLstDel(temph(CHashGlblVar *)->dim.next);
      if (temph(CHashGlblVar *)->fun_ptr)
	HashDel(temph(CHashGlblVar *)->fun_ptr-temph(CHashGlblVar *)->fun_ptr->ptr_cnt);
    }
  } else if (temph->type & HTT_FILE)
    Free(temph(CHashGeneric *)->user_data0);
  Free(temph);
}

U0 HashTableDel(CHashTable *table)
{//Free std system hash table, calling $LK,"HashDel",A="MN:HashDel"$() on entries.
  CHashSrcSym *temph,*temph1;
  if (!table) return;
  temph=table->next_added;
  while (temph!=table) {
    temph1=temph->next_added;
    HashDel(temph);
    temph=temph1;
  }
  Free(table->body);
  Free(table);
}

I64 HashTablePurge(CHashTable *table)
{//Eliminate ExportSysSyms that have been usurped.
  I64 result=0;
  CHashSrcSym *temph,*temph1,*temph2;
  if (!table) return 0;
  PUSHFD
  CLI	 //Precaution
  temph=table->next_added;
  while (temph!=table) {
    temph1=temph->next_added; //We delete only older ones
    if (temph->type&(HTT_FUN|HTT_GLBL_VAR)) {
      temph2=temph->next; //Older always later in chain
      while (temph2) {
	if ((temph2->type&HTT_EXPORT_SYS_SYM || temph2->type&HTG_TYPE_MASK==HTT_INVALID) &&
	    !StrCmp(temph2->str,temph->str)) {
	  if (temph2->type&HTG_TYPE_MASK==HTT_INVALID)
	    temph2->type=HTT_KEYWORD;//Won't delete HTT_INVALID
	  HashRemDel(temph2,table);
	  result++;
	  break;
	}
	temph2=temph2->next;
      }
    }
    temph=temph1;
  }
  POPFD
  return result;
}

CHashGeneric *HashGenericAdd(U8 *name,I64 type,I64 u0=0,I64 u1=0,I64 u2=0,CTask *task=NULL)
{//Add any type to task hash_table, 3 user_data values.
  if (!task) task=Fs;
  CHashGeneric *result=CAlloc(sizeof(CHashGeneric),task);
  result->type=type;
  result->user_data0=u0;
  result->user_data1=u1;
  result->user_data2=u2;
  result->str=StrNew(name,task);
  HashAdd(result,task->hash_table);
  return result;
}

U0 SetSrcFileInfo(CLex *lx,CHashSrcSym *h,I64 line_num_offset=0)
{//Set $LK,"CHashSrcSym",A="MN:CHashSrcSym"$ link and help_index by cur lx pos.
  CLexFile *tempf=lx->cur_lfn;
  I64 line_num=tempf->line_num+line_num_offset;
  if (line_num<1) line_num=1;
  Free(h->src_link);
  h->src_link=MStrPrint("FL:%s,%d",tempf->name,line_num);
  if (Bt(&lx->opts,OPTf_KEEP_PRIVATE))
    h->type|=HTF_PRIVATE;
  Free(h->index);
  if (lx->cur_help_index)
    h->index=StrNew(lx->cur_help_index);
  else
    h->index=NULL;
}

CHashGeneric *HashPublic(U8 *st,I64 mask,Bool val=TRUE)
{//Mark a hash entry as public and $LK,"SetSrcFileInfo",A="MN:SetSrcFileInfo"$().
  CHashGeneric *result;
  if (result=HashFind(st,Fs->hash_table,mask)) {
    if (val)
      result->type|=HTF_PUBLIC;
    else
      result->type&=~HTF_PUBLIC;
    if (result->type&HTG_SRC_SYM)
      SetSrcFileInfo(Fs->last_lex,result);
    return result;
  } else
    return NULL;
}

I64 HashLstAdd(U8 *lst,I64 type,CHashTable *table)
{//Add a list to a hash table.
  I64 i=0;
  CHashGeneric *temph;
  if (lst) {
    while (*lst) {
      if (*lst=='@')
	lst++;
      else
	i++;
      temph=CAlloc(sizeof(CHashGeneric));
      temph->user_data0=i-1;
      temph->str=StrNew(lst);
      temph->type=type;
      HashAdd(temph,table);
      while (*lst++);
    }
  }
  return i;
}

I64 HashDefineLstAdd(U8 *name,I64 type,CHashTable *table)
{//Add define list to a hash table. See $LK,"::/Adam/DolDoc/DocInit.CPP",A="FF:::/Adam/DolDoc/DocInit.CPP,HashDefineLstAdd"$.
  CHashDefineStr *temph;
  if (temph=HashFind(name,Fs->hash_table,HTT_DEFINE_STR))
    return HashLstAdd(temph->data,type,table);
  else
    return 0;
}

I64 FramePtr(U8 *name,CTask *task=NULL)
{//Find entry in task->hash_table, Return user_data.
  CHashGeneric *temph;
  if (!task) task=Fs;
  if (temph=HashFind(name,task->hash_table,HTT_FRAME_PTR))
    return temph->user_data0;
  else
    return 0;
}

CHashGeneric *FramePtrAdd(U8 *name,I64 val,CTask *task=NULL)
{//Add named value to task->hash_table.
  return HashGenericAdd(name,HTT_FRAME_PTR,val,0,0,task);
}

I64 FramePtrSet(U8 *name,I64 val,CTask *task=NULL)
{//Find hash entry in task->hash_table. Change user_data0.
  CHashGeneric *temph;
  if (!task) task=Fs;
  if (temph=HashFind(name,task->hash_table,HTT_FRAME_PTR))
    return LXchgI64(&temph->user_data0,val);
  else
    return 0;
}

I64 FramePtrDel(U8 *name,CTask *task=NULL)
{//Remove entry and delete.
  CHashGeneric *temph;
  I64 result=0;
  if (!task) task=Fs;
  if (temph=HashFind(name,task->hash_table,HTT_FRAME_PTR)) {
    result=temph->user_data0;
    HashRemDel(temph,task->hash_table);
  }
  return result;
}

U0 LoadOneImport(U8 **_src,U8 *module_base,I64 ld_flags)
{
  U8 *src=*_src,*ptr2,*st_ptr;
  I64 i,etype;
  CHashExport *tempex=NULL;
  CHashImport *tempiss;
  Bool first=TRUE;

  while (etype=*src++) {
    i=*src(U32 *)++;
    st_ptr=src;
    src+=StrLen(st_ptr)+1;
    if (*st_ptr) {
      if (!first) {
	*_src=st_ptr-5;
	return;
      } else {
	first=FALSE;
	if (!(tempex=HashFind(st_ptr,Fs->hash_table,HTG_ALL-HTT_IMPORT_SYS_SYM))) {
	  if (!(ld_flags & LDF_SILENT))
	    "Unresolved Reference:%s\n",st_ptr;
	  tempiss=CAlloc(sizeof(CHashImport));
	  tempiss->str=StrNew(st_ptr);
	  tempiss->type=HTT_IMPORT_SYS_SYM;
	  tempiss->module_header_entry=st_ptr-5;
	  tempiss->module_base=module_base;
	  HashAdd(tempiss,Fs->hash_table);
	}
      }
    }
    if (tempex) {
      ptr2=module_base+i;
      if (tempex->type & HTT_FUN)
	i=tempex(CHashFun *)->executable_addr;
      else if (tempex->type & HTT_GLBL_VAR)
	i=tempex(CHashGlblVar *)->data_addr;
      else
	i=tempex->val;
      switch (etype) {
	case EIT_REL_I8:  *ptr2(U8 *) =i-ptr2-1; break;
	case EIT_IMM_U8:  *ptr2(U8 *) =i;  break;
	case EIT_REL_I16: *ptr2(U16 *)=i-ptr2-2; break;
	case EIT_IMM_U16: *ptr2(U16 *)=i; break;
	case EIT_REL_I32: *ptr2(U32 *)=i-ptr2-4; break;
	case EIT_IMM_U32: *ptr2(U32 *)=i; break;
	case EIT_REL_I64: *ptr2(I64 *)=i-ptr2-8; break;
	case EIT_IMM_U64: *ptr2(I64 *)=i; break;
      }
    }
  }
  *_src=src-1;
}

U0 SysSymImportsResolve(U8 *st_ptr,I64 ld_flags)
{
  CHashImport *tempiss;
  U8 *ptr;
  while (tempiss=HashSingleTableFind(st_ptr,Fs->hash_table,HTT_IMPORT_SYS_SYM)) {
    ptr=tempiss->module_header_entry;
    LoadOneImport(&ptr,tempiss->module_base,ld_flags);
    tempiss->type=HTT_INVALID;
  }
}

U0 LoadPass1(U8 *src,U8 *module_base,I64 ld_flags)
{
  U8 *ptr2,*ptr3,*st_ptr;
  I64 i,j,cnt,etype;
  CHashExport *tempex=NULL;
  while (etype=*src++) {
    i=*src(U32 *)++;
    st_ptr=src;
    src+=StrLen(st_ptr)+1;
    switch (etype) {
      case EIT_REL32_EXPORT:
      case EIT_IMM32_EXPORT:
      case EIT_REL64_EXPORT:
      case EIT_IMM64_EXPORT:
	tempex=CAlloc(sizeof(CHashExport));
	tempex->str=StrNew(st_ptr);
	tempex->type=HTT_EXPORT_SYS_SYM|HTF_IMM;
	if (etype==EIT_IMM32_EXPORT||etype==EIT_IMM64_EXPORT)
	  tempex->val=i;
	else
	  tempex->val=i+module_base;
	HashAdd(tempex,Fs->hash_table);
	SysSymImportsResolve(st_ptr,ld_flags);
	break;
      case EIT_REL_I0...EIT_IMM_U64:
	src=st_ptr-5;
	LoadOneImport(&src,module_base,ld_flags);
	break;
      case EIT_ABS_ADDR:
	if (ld_flags & LDF_NO_ABSS)
	  src+=i*sizeof(U32);
	else {
	  cnt=i;
	  for (j=0;j<cnt;j++) {
	    ptr2=module_base+*src(U32 *)++;
	    *ptr2(U32 *)+=module_base;
	  }
	}
	break;

      sub_switch_start:
	case EIT_CODE_HEAP:
	  ptr3=MAlloc(*src(I32 *)++,Fs->code_heap);
	  break;
	case EIT_ZEROED_CODE_HEAP:
	  ptr3=CAlloc(*src(I32 *)++,Fs->code_heap);
	  break;
      sub_switch_end:
	if (*st_ptr) {
	  tempex=CAlloc(sizeof(CHashExport));
	  tempex->str=StrNew(st_ptr);
	  tempex->type=HTT_EXPORT_SYS_SYM|HTF_IMM;
	  tempex->val=ptr3;
	  HashAdd(tempex,Fs->hash_table);
	}
	cnt=i;
	for (j=0;j<cnt;j++) {
	  ptr2=module_base+*src(U32 *)++;
	  *ptr2(I32 *)+=ptr3;
	}
	break;

      sub_switch_start:
	case EIT_DATA_HEAP:
	  ptr3=MAlloc(*src(I64 *)++);
	  break;
	case EIT_ZEROED_DATA_HEAP:
	  ptr3=CAlloc(*src(I64 *)++);
	  break;
      sub_switch_end:
	if (*st_ptr) {
	  tempex=CAlloc(sizeof(CHashExport));
	  tempex->str=StrNew(st_ptr);
	  tempex->type=HTT_EXPORT_SYS_SYM|HTF_IMM;
	  tempex->val=ptr3;
	  HashAdd(tempex,Fs->hash_table);
	}
	cnt=i;
	for (j=0;j<cnt;j++) {
	  ptr2=module_base+*src(U32 *)++;
	  *ptr2(I64 *)+=ptr3;
	}
	break;
    }
  }
}

U0 LoadPass2(U8 *src,U8 *module_base,I64)
{
  U8 *st_ptr;
  I64 i,etype;
  while (etype=*src++) {
    i=*src(U32 *)++;
    st_ptr=src;
    src+=StrLen(st_ptr)+1;
    switch (etype) {
      case EIT_MAIN:
	Call(i+module_base);
	break;
      case EIT_ABS_ADDR:
	src+=sizeof(U32)*i;
	break;
      case EIT_CODE_HEAP:
      case EIT_ZEROED_CODE_HEAP:
	src+=4+sizeof(U32)*i;
	break;
      case EIT_DATA_HEAP:
      case EIT_ZEROED_DATA_HEAP:
	src+=8+sizeof(U32)*i;
	break;
    }
  }
}

U8 *Load(U8 *filename,I64 ld_flags=0,CBinFile *bfh_addr=INVALID_PTR)
{//Load a .BIN file module into memory.
//bfh_addr==INVALID_PTR means don't care what load addr.
  U8 *fbuf,*module_base,*absname;
  I64 size,module_align;
  CBinFile *bfh;

  fbuf=DftExt(filename,"BIN.Z");
  if (!(bfh=FileRead(fbuf,&size))) {
    Free(fbuf);
    return NULL;
  }
  //See $LK,"Patch Table Generation",A="FF:::/Compiler/CmpMain.CPP,EIT_ABS_ADDR"$
  module_align=1<<bfh->module_align_bits;
  if (!module_align || bfh->bin_signature!=BIN_SIGNATURE_VAL) {
    Free(bfh);
    Free(fbuf);
    throw('BinModul');
  }
  if (bfh_addr==INVALID_PTR) {
    if (bfh->module_org==INVALID_PTR) {
      if (Fs->code_heap!=Fs->data_heap) {
	if (module_align<16)
	  module_align=16;
	bfh_addr=MAllocAligned(size,module_align,Fs->code_heap,module_align-sizeof(CBinFile));
      } else if (module_align>8)
	bfh_addr=MAllocAligned(size,module_align,,module_align-sizeof(CBinFile));
      else {//Less than 2Gig system memory
	bfh_addr=bfh;
	goto lo_skip; //File is already in code heap area, don't copy.
      }
    } else
      bfh_addr=bfh->module_org;
  }
  MemCpy(bfh_addr,bfh,size);
  Free(bfh);

lo_skip:
  module_base=bfh_addr(U8 *)+sizeof(CBinFile);

  absname=FileNameAbs(fbuf);
  Free(fbuf);
  fbuf=StrNew(absname);
  FileExtRem(fbuf);
  if (fbuf[1]==':' && StrLen(fbuf)>2)
    HashGenericAdd(fbuf+2,HTT_MODULE|HTF_PUBLIC,bfh_addr);

  LoadPass1(bfh_addr(U8 *)+bfh_addr->patch_table_offset,module_base,ld_flags);
  if (!(ld_flags&LDF_JUST_LOAD))
    LoadPass2(bfh_addr(U8 *)+bfh_addr->patch_table_offset,module_base,ld_flags);

  Free(absname);
  Free(fbuf);
  return bfh_addr;
}

U0 LoadKernel()
{
  HashGenericAdd(KERNEL_MODULE_NAME,HTT_MODULE|HTF_PUBLIC,sys_boot_base-sizeof(CBinFile));

//Abs patches done here $LK,"CPatchTableAbsAddr",A="FF:D:/Kernel/KStart.CPP,CPatchTableAbsAddr"$.
  LoadPass1(sys_boot_patch_table_base,sys_boot_base,LDF_NO_ABSS|LDF_SILENT);

//No main routines
//  LoadPass2(sys_boot_patch_table_base,sys_boot_base,0);
}
