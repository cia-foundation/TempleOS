I64 HashEntrySize(CHashSrcSym *tmph)
{//Logical size of a std system hash entry.
  CDbgInfo *dbg_info;
  CBinFile *bfh;
  switch (HashTypeNum(tmph)) {
    case HTt_DEFINE_STR:
      return MSize(tmph(CHashDefineStr *)->data);
    case HTt_GLBL_VAR:
    case HTt_CLASS:
    case HTt_INTERNAL_TYPE:
      return tmph(CHashClass *)->size;
    case HTt_FUN:
      if (dbg_info=tmph->dbg_info)
	return dbg_info->body[dbg_info->max_line+1-dbg_info->min_line]
	      -dbg_info->body[0];
      else
	return -1;
    case HTt_FILE:
      return MSize(tmph(CHashGeneric *)->user_data0);
    case HTt_MODULE:
      if (StrCmp(tmph->str,KERNEL_MODULE_NAME))
	return MSize(tmph(CHashGeneric *)->user_data0);
      else {
	bfh=mem_boot_base-sizeof(CBinFile);
	return bfh->file_size;
      }
    case HTt_WORD:
      return StrLen(tmph->str);
  }
  return -1;
}

I64 HashEntrySize2(CHashSrcSym *tmph)
{//Memory size of a std system hash entry.
  CDbgInfo *dbg_info;
  I64 res=MSize2(tmph);
  if (!(tmph->type&HTT_DICT_WORD))
    res+=MSize2(tmph->str);
  if (tmph->type & HTG_SRC_SYM) {
    res+=MSize2(tmph->src_link);
    res+=MSize2(tmph->idx);
    res+=MSize2(tmph->import_name);
    res+=LinkedLstSize(tmph->ie_lst);
    if (tmph->type & (HTT_FUN | HTT_EXPORT_SYS_SYM))
      res+=MSize2(tmph->dbg_info);
    if (tmph->type & HTT_CLASS)
      res+=MemberLstSize(tmph);
    else if (tmph->type & HTT_FUN) {
      res+=MemberLstSize(tmph);
      if (dbg_info=tmph->dbg_info)
//This should be $LK,"MSize",A="MN:MSize"$() but it would crash on AOT .BIN.Z file funs.
	res+=dbg_info->body[dbg_info->max_line+1-dbg_info->min_line]
	      -dbg_info->body[0];
    } else if (tmph->type&HTT_DEFINE_STR)
      res+=MSize2(tmph(CHashDefineStr *)->data);
    else if (tmph->type & HTT_GLBL_VAR) {
      res+=LinkedLstSize(tmph(CHashGlblVar *)->dim.next);
      if (!(tmph(CHashGlblVar *)->flags&GVF_ALIAS))
	res+=MSize2(tmph(CHashGlblVar *)->data_addr);
      if (tmph(CHashGlblVar *)->fun_ptr)
	res+=HashEntrySize2(tmph(CHashGlblVar *)->fun_ptr
	      -tmph(CHashGlblVar *)->fun_ptr->ptr_stars_cnt);
    }
  } else if (tmph->type & HTT_FILE)
    res+=MSize2(tmph(CHashGeneric *)->user_data0);
  else if (tmph->type & HTT_MODULE &&
	StrCmp(tmph->str,KERNEL_MODULE_NAME))
    res+=MSize2(tmph(CHashGeneric *)->user_data0);
  return res;
}

I64 HashTableSize2(CHashTable *table)
{//Memory size of std system hash table and all entries.
  I64 i,res=0;
  CHashSrcSym *tmph;
  if (!table)
    return 0;
  for (i=0;i<=table->mask;i++) {
    tmph=table->body[i];
    while (tmph) {
      res+=HashEntrySize2(tmph);
      tmph=tmph->next;
    }
  }
  res+=MSize2(table->body);
  res+=MSize2(table);
  return res;
}

U0 MapFileWrite(CHashTable *h,U8 *map_name,U8 drv_let)
{
  CHashSrcSym *tmph;
  I64 i,size;
  U8 *src_link;
  CDoc *doc;
  CDocBin *tmpb;
  CDbgInfo *dbg_info;

  doc=DocNew(map_name);
  doc->flags|=DOCF_NO_CURSOR;
  for (i=0;i<=h->mask;i++) {
    tmph=h->body[i];
    while (tmph) {
      if (tmph->src_link && !(tmph->type & (HTF_IMPORT | HTF_PRIVATE))) {
	src_link=StrNew(tmph->src_link);
	if (drv_let && StrLen(src_link)>=4)
	  src_link[3]=drv_let;
	if (dbg_info=tmph->dbg_info) {
	  size=offset(CDbgInfo.body)+
		sizeof(U32)*(dbg_info->max_line+2-dbg_info->min_line);
	  if (size>MSize(dbg_info)) {
	    "Corrupt Map Entry\n";
	    dbg_info=NULL;
	  } else {
	    if (dbg_info->min_line<=dbg_info->max_line) {
	      tmpb=CAlloc(sizeof(CDocBin));
	      tmpb->size=size;
	      tmpb->data=MAlloc(size);
	      MemCpy(tmpb->data,dbg_info,size);
	      tmpb->num=doc->cur_bin_num++;
	      tmpb->use_cnt=1;
	      QueIns(tmpb,doc->bin_head.last);
	    } else
	      dbg_info=NULL;
	  }
	}
	if (dbg_info)
	  DocPrint(doc,"$$LK,\"%s\",A=\"%s\",BI=%d$$\n",
		tmph->str,src_link,tmpb->num);
	else
	  DocPrint(doc,"$$LK,\"%s\",A=\"%s\"$$\n",tmph->str,src_link);

	Free(src_link);
      }
      tmph=tmph->next;
    }
  }
  DocBinsValidate(doc);
  DocWrite(doc);
  DocDel(doc);
}
