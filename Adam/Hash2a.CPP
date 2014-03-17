#help_index "Info;Hash;Cmd Line (Typically)"
class CWho
{
  CHashGeneric *h;
  U8 *index;
};


I64 HashEntriesCompare(CWho *h1,CWho *h2)
{
  I64 i1,i2;
  if (i1=StrCmp(h1->h->str,h2->h->str))
    return i1;
  i1=HashTypeNum(h1->h);
  i2=HashTypeNum(h2->h);
  return i1-i2;
}

I64 HashEntriesCompare2(CWho *h1,CWho *h2)
{
  CHashFun *tempf1=h1->h,*tempf2=h2->h;
  I64 i1=HashVal(tempf1),i2=HashVal(tempf2);
  if (i1==i2) {
    i1=HashTypeNum(tempf1);
    i2=HashTypeNum(tempf2);
    if (i1==i2)
      return StrCmp(tempf1->str,tempf2->str);
  }
  return i1-i2;
}

I64 HelpIndexCnt(U8 *ptr,U8 *index)
{
  I64 cnt=0,ch,index_len=StrLen(index);
  while (*ptr) {
    if (!StrNCmp(ptr,index,index_len))
      cnt++;
    while (ch=*ptr++)
      if (ch==';')
	break;
    if (!ch)
      ptr--;
  }
  return cnt;
}

U8 *HelpIndexStr(U8 **ptr_,U8 *index)
{
  U8 *ptr=*ptr_,*ptr2,*result;
  I64 ch,index_len=StrLen(index);
  while (*ptr) {
    ptr2=ptr;
    while (ch=*ptr++)
      if (ch==';')
	break;
    if (!ch)
      ptr--;
    *ptr_=ptr;
    if (!StrNCmp(ptr2,index,index_len)) {
      if (ch==';')
	ptr--;
      *ptr=0;
      result=StrNew(ptr2);
      *ptr=ch;
      return result;
    }
  }
  return NULL;
}

I64 HashEntriesCompare3(CWho *h1,CWho *h2)
{
  I64 i,i1=0,i2=0;
  i=StrCmp(h1->index,h2->index);
  if (i)
    return i;
  else {
    if (h1->h->type&HTT_HELP_FILE)
      i1=1;
    if (h2->h->type&HTT_HELP_FILE)
      i2=1;
    i=i2-i1;
    if (i)
      return i;
    else
      return StrCmp(h1->h->str,h2->h->str);
  }
}

public U0 Who(U8 *fu_flags=NULL,CHashTable *h=NULL,
  U8 *outname=NULL,U8 mapfile_drv=0,U8 *index=NULL,CDoc *_doc=NULL)
{//Dump Symbol Table
// "+p" for only public symbols
// "+m" to order by number (normally alphabetical)
// "-r" just local hash table
  CHashTable *tempht;
  CHashSrcSym *temph;
  CHashGeneric *ptr;
  CWho *lst;
  I64 cnt,i,j,k,size,f=0,dbg_bin_num;
  U8 buf[512],*st,*src_link,*last_index=StrNew(""),*cur_index;
  CDoc *doc;
  Bool recurse,publics,map,old_preempt=Preempt(OFF);
  CDocBin *tempb;
  CDbgInfo *dbg;

  ScanFlags(&f,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&f,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (f&~(FUF_RECURSE|FUF_PUBLIC|FUF_MAP))
    throw('FUF');
  recurse=Bt(&f,FUf_RECURSE);
  publics=Bt(&f,FUf_PUBLIC);
  map	 =Bt(&f,FUf_MAP);

  if (!h) h=Fs->hash_table;

  if (_doc)
    doc=_doc;
  else if (outname) {
    doc=DocNew(outname);
    doc->flags|=DOCF_NO_CURSOR;
  } else
    doc=NULL;

  cnt=0;
  tempht=h;
  while (tempht) {
    temph=tempht->next_added;
    while (temph!=tempht) {
      if (!(temph->type & (HTF_IMPORT | HTF_PRIVATE)) &&
	    (temph->type & HTF_PUBLIC || !publics))
	if (!index)
	  cnt++;
	else
	  if (temph->type&HTG_SRC_SYM &&
		(cur_index=temph->index))
	    cnt+=HelpIndexCnt(cur_index,index);
      temph=temph->next_added;
    }
    if (recurse)
      tempht=tempht->next;
    else
      break;
  }
  if (!cnt) goto end_hash;

  lst=CAlloc(cnt*sizeof(CWho));
  j=0;
  tempht=h;
  while (tempht) {
    temph=tempht->next_added;
    while (temph!=tempht) {
      if (!(temph->type & (HTF_IMPORT | HTF_PRIVATE)) &&
	    (temph->type & HTF_PUBLIC || !publics))
	if (!index)
	  lst[j++].h=temph;
	else
	  if (temph->type&HTG_SRC_SYM &&
		(cur_index=temph->index))
	    if (k=HelpIndexCnt(cur_index,index)) {
	      while (k--) {
		lst[j].index=HelpIndexStr(&cur_index,index);
		lst[j++].h=temph;
	      }
	    }
      temph=temph->next_added;
    }
    if (recurse)
      tempht=tempht->next;
    else
      break;
  }
  Preempt(old_preempt);

  if (map)
    QSort(lst,cnt,sizeof(CWho),&HashEntriesCompare2);
  else if (index)
    QSort(lst,cnt,sizeof(CWho),&HashEntriesCompare3);
  else
    QSort(lst,cnt,sizeof(CWho),&HashEntriesCompare);

  for (i=0;i<cnt;i++) {
    ptr=lst[i].h;
    dbg=NULL;
    if (index)
      if (cur_index=lst[i].index) {
	if (StrCmp(cur_index,last_index)) {
	  Free(last_index);
	  last_index=StrNew(cur_index);
	  if (doc)
	    DocPrint(doc,"$$GREEN$$%s$$FG$$\n",cur_index);
	  else
	    "$$GREEN$$%s$$FG$$\n",cur_index;
	}
      }

    if (index && ptr->type & HTT_HELP_FILE)
      DocType(doc,ptr->str);
    else {
      if (ptr->type&HTG_SRC_SYM && ptr(CHashSrcSym *)->src_link) {
	src_link=StrNew(ptr(CHashSrcSym *)->src_link);
	if (mapfile_drv && StrLen(src_link)>4 && buf[1]==':' && src_link[4]==':')
	  src_link[3]=mapfile_drv;
	dbg=NULL;
	if (outname && doc && ptr->type & HTT_FUN) {
	  if (dbg=ptr(CHashFun *)->debug) {
	    size=offset(CDbgInfo.body)+sizeof(U32)*(dbg->max_line+2-dbg->min_line);
	    if (size>MSize(dbg)) {
	      "Corrupt Map Entry\n";
	      dbg=NULL;
	    } else {
	      if (dbg->min_line<=dbg->max_line) {
		tempb=CAlloc(sizeof(CDocBin));
		tempb->size=size;
		tempb->data=MAlloc(size);
		MemCpy(tempb->data,dbg,size);
		dbg_bin_num=tempb->num=doc->cur_bin_num++;
		tempb->use_cnt=1;
		QueIns(tempb,doc->bin_root.last);
	      } else
		dbg=NULL;
	    }
	  }
	}
	if (dbg)
	  DocPrint(doc,"$$LK,\"%-26s\",A=\"%s\",BI=%d$$",
		ptr->str,src_link,dbg_bin_num);
	else {
	  SPrint(buf,"$$LK,\"%-26s\",A=\"%s\"$$",
		ptr->str,src_link);
	  if (doc)
	    DocPrint(doc,"%s",buf);
	  else
	    "%s",buf;
	}
	Free(src_link);
      } else {
	SPrint(buf,"%-26ts",ptr->str);
	if (doc)
	  DocPrint(doc,"%s",buf);
	else
	  "%s",buf;
      }

      if (!index) {
	if (ptr->type & HTT_DEFINE_STR) {
	  st=MStrUtil(ptr(CHashDefineStr *)->data,SUF_SAFE_DOLLAR);
	  j=ptr(CHashDefineStr *)->cnt;
	  if (j==-1)
	    SPrint(buf,"%-10tQ   ",st);
	  else
	    SPrint(buf,"%-10tQ %02X",st,j);
	  Free(st);
	} else if (ptr->type & HTT_GLBL_VAR) {
	  if (outname)
	    SPrint(buf,"%010X   ",ptr(CHashGlblVar *)->data_addr_ip);
	  else
	    SPrint(buf,"%010X   ",ptr(CHashGlblVar *)->data_addr);
	} else
	  SPrint(buf,"%010X   ",HashVal(ptr));
	j=HashEntrySize(ptr);
	if (j==-1)
	  SPrint(buf+StrLen(buf)," %04X            ",ptr->use_cnt);
	else
	  SPrint(buf+StrLen(buf)," %04X %010X ",ptr->use_cnt,j);
      } else
	*buf=0;

      k=ptr->type;
      if (publics)
	k&=~HTF_PUBLIC;
      if (!(k&HTG_TYPE_MASK))
	SPrint(buf+StrLen(buf),"NULL ");
      for (j=0;j<32;j++)
	if (Bt(&k,j))
	  SPrint(buf+StrLen(buf),"%Z ",j,"ST_HTT_TYPES");
      CatPrint(buf,"\n");
      if (doc)
	DocPrint(doc,"%s",buf);
      else
	"%s",buf;
    }
    Free(lst[i].index);
  }
  Free(lst);

  end_hash:
  Preempt(old_preempt);
  if (doc) {
    DocBinsValidate(doc);
    if (!_doc) {
      DocWrite(doc);
      DocDel(doc);
    } else {
      if (doc->root.next==doc)
	DocPrint(doc,"No Match");
      else
	DocRecalc(doc);
    }
  }
  Free(last_index);
}

#define HDR_MAX	16
public I64 HashDepthRep(CHashTable *tempht=NULL)
{
  I64 i,j,longest=0,cnt=0,a[HDR_MAX];
  CHash *temph;
  if (!tempht) tempht=Fs->hash_table;
  MemSet(a,0,sizeof(a));
  for (i=0;i<=tempht->mask;i++) {
    temph=tempht->body[i];
    if (temph) {
      j=1;
      while (temph->next) {
	temph=temph->next;
	j++;
      }
      if (j<HDR_MAX)
	a[j]++;
      cnt+=j;
      if (j>longest)
	longest=j;
    }
  }
  "Histogram\n";
  for (i=0;i<HDR_MAX;i++)
    if (a[i])
      "%02d:%d\n",i,a[i];
  "Size:%d Count:%d Longest:%d\n",
    tempht->mask+1,cnt,longest;
  return longest;
}

#help_index "Help System;Training"
#help_file "::/Doc/HelpSystem"

public U0 DocHelpIndex(CDoc *doc,U8 *index)
{
  Who("+p",,,,index,doc);
}

public U0 PopUpHelpIndex(U8 *index,CTask *parent=NULL)
{
  U8 *buf;
  buf=MPrint("DocHelpIndex(DocPut,\"%s\");View;",index);
  PopUp(buf,parent);
  Free(buf);
}

#help_index "Hash"
public U0 MapFileLoad(U8 *filename)
{
  U8 *st,*ptr,*name=DftExt(filename,"MAP.Z"),
	*absname=FileNameAbs(name);
  CDoc *doc=DocRead(name);
  CDocEntry *doc_e;
  CHashSrcSym *temph;
  I64 i,j,base=0;
  CDbgInfo *dbg;

  FileExtRem(absname);
  if (absname[1]==':' && StrLen(absname)>2 &&
	(temph=HashSingleTableFind(absname+2,Fs->hash_table,HTT_MODULE)))
    base=temph(CHashGeneric *)->user_data0+sizeof(CBinFile);

  if (!doc) return;
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_LINK) {
      if (*doc_e->tag)
	st=MStrUtil(doc_e->tag,SUF_REM_TRAILING);
      else
	st=MStrUtil(doc_e->aux_str,SUF_REM_TRAILING);
      if (temph=HashSingleTableFind(st,Fs->hash_table,HTG_SRC_SYM)) {
	if (*doc_e->tag) {
	  Free(temph->src_link);
	  temph->src_link=doc_e->aux_str;

	  ptr=temph->src_link;
	  if (ptr[0] && ptr[1] && ptr[2]==':' && ptr[3]==':')
	    ptr[3]=*sys_acct;

	  doc_e->aux_str=NULL;
	}
	if (temph->type&(HTT_FUN|HTT_EXPORT_SYS_SYM) &&
	      !(dbg=temph->debug) && doc_e->bin_data &&
	      (dbg=doc_e->bin_data->data)) {
	  if (doc_e->bin_data->size>MSize(dbg))
	    "Corrupt Map Entry\n";
	  else {
	    doc_e->bin_data->data=NULL;
	    temph->debug=dbg;
	    for (i=dbg->min_line;i<=dbg->max_line+1;i++) {
	      j=i-dbg->min_line;
	      if (dbg->body[j])
		dbg->body[j]=dbg->body[j]+base;
	    }
	  }
	}
      }
      Free(st);
    }
    doc_e=doc_e->next;
  }
  DocDel(doc);
  Free(name);
  Free(absname);
}
#help_index ""
