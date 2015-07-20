I64 HashEntrySize(CHashSrcSym *temph)
{//Logical size of a std system hash entry.
  CDbgInfo *dbg_info;
  CBinFile *bfh;
  switch (HashTypeNum(temph)) {
    case HTt_DEFINE_STR:
      return MSize(temph(CHashDefineStr *)->data);
    case HTt_GLBL_VAR:
    case HTt_CLASS:
    case HTt_INTERNAL_TYPE:
      return temph(CHashClass *)->size;
    case HTt_FUN:
      if (dbg_info=temph->dbg_info)
	return dbg_info->body[dbg_info->max_line+1-dbg_info->min_line]
	      -dbg_info->body[0];
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
{//Memory size of a std system hash entry.
  CDbgInfo *dbg_info;
  I64 result=MSize2(temph);
  if (!(temph->type&HTT_DICT_WORD))
    result+=MSize2(temph->str);
  if (temph->type & HTG_SRC_SYM) {
    result+=MSize2(temph->src_link);
    result+=MSize2(temph->index);
    result+=MSize2(temph->import_name);
    result+=LinkedLstSize(temph->ie_lst);
    if (temph->type & (HTT_FUN | HTT_EXPORT_SYS_SYM))
      result+=MSize2(temph->dbg_info);
    if (temph->type & HTT_CLASS)
      result+=MemberLstSize(temph);
    else if (temph->type & HTT_FUN) {
      result+=MemberLstSize(temph);
      if (dbg_info=temph->dbg_info)
//This should be $LK,"MSize",A="MN:MSize"$() but it would crash on AOT .BIN.Z file funs.
	result+=dbg_info->body[dbg_info->max_line+1-dbg_info->min_line]
	      -dbg_info->body[0];
    } else if (temph->type&HTT_DEFINE_STR)
      result+=MSize2(temph(CHashDefineStr *)->data);
    else if (temph->type & HTT_GLBL_VAR) {
      result+=LinkedLstSize(temph(CHashGlblVar *)->dim.next);
      if (!(temph(CHashGlblVar *)->flags&GVF_ALIAS))
	result+=MSize2(temph(CHashGlblVar *)->data_addr);
      if (temph(CHashGlblVar *)->fun_ptr)
	result+=HashEntrySize2(temph(CHashGlblVar *)->fun_ptr
	      -temph(CHashGlblVar *)->fun_ptr->ptr_stars_cnt);
    }
  } else if (temph->type & HTT_FILE)
    result+=MSize2(temph(CHashGeneric *)->user_data0);
  else if (temph->type & HTT_MODULE &&
	StrCmp(temph->str,KERNEL_MODULE_NAME))
    result+=MSize2(temph(CHashGeneric *)->user_data0);
  return result;
}

I64 HashTableSize2(CHashTable *table)
{//Memory size of std system hash table and all entries.
  I64 i,result=0;
  CHashSrcSym *temph;
  if (!table)
    return 0;
  for (i=0;i<=table->mask;i++) {
    temph=table->body[i];
    while (temph) {
      result+=HashEntrySize2(temph);
      temph=temph->next;
    }
  }
  result+=MSize2(table->body);
  result+=MSize2(table);
  return result;
}

U0 MapFileWrite(CHashTable *h,U8 *map_name,U8 drv_let)
{
  CHashSrcSym *temph;
  I64 i,size;
  U8 *src_link;
  CDoc *doc;
  CDocBin *tempb;
  CDbgInfo *dbg_info;

  doc=DocNew(map_name);
  doc->flags|=DOCF_NO_CURSOR;
  for (i=0;i<=h->mask;i++) {
    temph=h->body[i];
    while (temph) {
      if (temph->src_link && !(temph->type & (HTF_IMPORT | HTF_PRIVATE))) {
	src_link=StrNew(temph->src_link);
	if (drv_let && StrLen(src_link)>=4)
	  src_link[3]=drv_let;
	if (dbg_info=temph->dbg_info) {
	  size=offset(CDbgInfo.body)+
		sizeof(U32)*(dbg_info->max_line+2-dbg_info->min_line);
	  if (size>MSize(dbg_info)) {
	    "Corrupt Map Entry\n";
	    dbg_info=NULL;
	  } else {
	    if (dbg_info->min_line<=dbg_info->max_line) {
	      tempb=CAlloc(sizeof(CDocBin));
	      tempb->size=size;
	      tempb->data=MAlloc(size);
	      MemCpy(tempb->data,dbg_info,size);
	      tempb->num=doc->cur_bin_num++;
	      tempb->use_cnt=1;
	      QueIns(tempb,doc->bin_root.last);
	    } else
	      dbg_info=NULL;
	  }
	}
	if (dbg_info)
	  DocPrint(doc,"$$LK,\"%s\",A=\"%s\",BI=%d$$\n",
		temph->str,src_link,tempb->num);
	else
	  DocPrint(doc,"$$LK,\"%s\",A=\"%s\"$$\n",temph->str,src_link);

	Free(src_link);
      }
      temph=temph->next;
    }
  }
  DocBinsValidate(doc);
  DocWrite(doc);
  DocDel(doc);
}
