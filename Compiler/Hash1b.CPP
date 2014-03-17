CHashSrcSym *HashEntryImportCopy(CHashSrcSym *temph)
{ //Actually this imports, not copies!!
  CHashSrcSym *result=MAllocIdentical(temph);
  result->str=MAllocIdentical(temph->str);
  result->use_cnt=0;
  result->src_link=NULL;
  result->index=NULL;
  result->ie_lst=NULL;
  result->debug=NULL;
  if (temph->type & (HTT_FUN | HTT_CLASS)) {
    result(CHashClass *)->member_lst =MemberLstCopy(temph);
    result(CHashClass *)->member_lst2=temph(CHashClass *)->member_lst2;
    if (temph->type & HTT_FUN) {
      if (Bt(&temph(CHashFun *)->flags,Ff_INTERNAL)) {
	result->import_name=NULL;
	LBtr(&result(CHashFun *)->flags,Cf_EXTERN);
      } else {
	result->import_name=MAllocIdentical(temph->import_name);
	result->type|=HTF_IMPORT;
	LBts(&result(CHashFun *)->flags,Cf_EXTERN);
      }
    }
  } else if (temph->type & (HTT_EXPORT_SYS_SYM | HTT_GLBL_VAR)) {
    result->import_name=MAllocIdentical(temph->import_name);
    result->type|=HTF_IMPORT;
    if (temph->type&HTT_EXPORT_SYS_SYM) {
      result->type|=HTF_UNRESOLVED;
      result(CHashExport *)->val=0;
    } else {
      result(CHashGlblVar *)->dim.next=LinkedLstCopy(temph(CHashGlblVar *)->dim.next);
      result(CHashGlblVar *)->flags|=GVF_IMPORT|GVF_ALIAS;
      if (temph(CHashGlblVar *)->fun_ptr)
	result(CHashGlblVar *)->fun_ptr=HashEntryImportCopy(temph(CHashGlblVar *)->fun_ptr-temph(CHashGlblVar *)->fun_ptr->ptr_cnt)(CHashFun *)+temph(CHashGlblVar *)->fun_ptr->ptr_cnt;
    }
  } else if (temph->type&HTT_DEFINE_STR)
    result(CHashDefineStr *)->data=MAllocIdentical(temph(CHashDefineStr *)->data);
  return result;
}

U0 HashTableImport(CHashTable *src,CHashTable *dst)
{
  I64 i;
  CHashSrcSym *temph,*temph1,*temph2;
  if (!src) return;
  for (i=0;i<=src->mask;i++) {
    temph=src->body[i];
    while (temph) {
      if (temph->type & HTG_SRC_SYM) {
	//Only add first in lst
	temph2=src->body[i];
	while (temph2!=temph)
	  if (!StrCmp(temph->str,temph2->str) &&
	      temph2->type & temph->type & HTG_SRC_SYM ||
	      temph->type & HTT_EXPORT_SYS_SYM)
	    break;
	  else
	    temph2=temph2->next;

	if (temph2==temph && (temph1=HashEntryImportCopy(temph)))
	  HashAdd(temph1,dst);
      }
      temph=temph->next;
    }
  }
}

I64 HashEntrySize(CHashSrcSym *temph)
{
  CDbgInfo *dbg;
  CBinFile *bfh;
  switch (HashTypeNum(temph)) {
    case HTt_DEFINE_STR:
      return MSize(temph(CHashDefineStr *)->data);
    case HTt_GLBL_VAR:
    case HTt_CLASS:
    case HTt_INTERNAL_TYPE:
      return temph(CHashClass *)->size;
    case HTt_FUN:
      if (dbg=temph->debug)
	return dbg->body[dbg->max_line+1-dbg->min_line]-dbg->body[0];
      else
	return -1;
    case HTt_FILE:
      return MSize(temph(CHashGeneric *)->user_data0);
    case HTt_MODULE:
      if (StrCmp(temph->str,KERNEL_MODULE_NAME))
	return MSize(temph(CHashGeneric *)->user_data0);
      else {
	bfh=sys_boot_base-sizeof(CBinFile);
	return bfh->file_size;
      }
    case HTt_WORD:
      return StrLen(temph->str);
  }
  return -1;
}

I64 HashEntrySize2(CHashSrcSym *temph)
{
  CDbgInfo *dbg;
  I64 result=MSize2(temph);
  if (!(temph->type&HTT_DICT_WORD))
    result+=MSize2(temph->str);
  if (temph->type & HTG_SRC_SYM) {
    result+=MSize2(temph->src_link);
    result+=MSize2(temph->index);
    result+=MSize2(temph->import_name);
    result+=LinkedLstSize(temph->ie_lst);
    if (temph->type & (HTT_FUN | HTT_EXPORT_SYS_SYM))
      result+=MSize2(temph->debug);
    if (temph->type & HTT_CLASS)
      result+=MemberLstSize(temph);
    else if (temph->type & HTT_FUN) {
      result+=MemberLstSize(temph);
      if (dbg=temph->debug)
	//This should be $LK,"MSize",A="MN:MSize"$() but it would crash on AOT .BIN.Z file funs.
	result+=dbg->body[dbg->max_line+1-dbg->min_line]-dbg->body[0];
    } else if (temph->type&HTT_DEFINE_STR)
      result+=MSize2(temph(CHashDefineStr *)->data);
    else if (temph->type & HTT_GLBL_VAR) {
      result+=LinkedLstSize(temph(CHashGlblVar *)->dim.next);
      if (!(temph(CHashGlblVar *)->flags&GVF_ALIAS))
	result+=MSize2(temph(CHashGlblVar *)->data_addr);
      if (temph(CHashGlblVar *)->fun_ptr)
	result+=HashEntrySize2(temph(CHashGlblVar *)->fun_ptr-temph(CHashGlblVar *)->fun_ptr->ptr_cnt);
    }
  } else if (temph->type & HTT_FILE)
    result+=MSize2(temph(CHashGeneric *)->user_data0);
  else if (temph->type & HTT_MODULE &&
	   StrCmp(temph->str,KERNEL_MODULE_NAME))
    result+=MSize2(temph(CHashGeneric *)->user_data0);
  return result;
}

I64 HashTableSize2(CHashTable *tempht)
{
  I64 i,result=0;
  CHashSrcSym *temph;
  if (!tempht)
    return 0;
  for (i=0;i<=tempht->mask;i++) {
    temph=tempht->body[i];
    while (temph) {
      result+=HashEntrySize2(temph);
      temph=temph->next;
    }
  }
  result+=MSize2(tempht->body);
  result+=MSize2(tempht);
  return result;
}
