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

U8 *HelpIndexStr(U8 **_ptr,U8 *index)
{
  U8 *ptr=*_ptr,*ptr2,*result;
  I64 ch,index_len=StrLen(index);
  while (*ptr) {
    ptr2=ptr;
    while (ch=*ptr++)
      if (ch==';')
	break;
    if (!ch)
      ptr--;
    *_ptr=ptr;
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

U8 *HelpComment(CTask *task,CHash *temph,U8 *_src_link)
{
  CDoc *doc;
  CDocEntry *doc_e;
  U8 *result=NULL,*ptr,*ptr2,*src_link=StrNew(_src_link);

  if (*src_link=='F' && src_link[2]==':')
    *src_link='P';
  XTalkWait(task,"Ed(0x%X,DOF_DONT_WINMGR_SYNC|DOF_DONT_SHOW);\n",src_link);
  Free(src_link);

  doc=DocPut(task);
  doc_e=doc->cur_entry;
  if (temph->type&HTT_FUN && !Bt(&temph(CHashFun *)->flags,Ff__EXTERN) &&
	!Bt(&temph(CHashFun *)->flags,Ff_INTERNAL))
    while (doc_e!=doc &&
	(!(doc_e->de_flags&DOCEF_TAG)||!StrOcc(doc_e->tag,'{')))
      doc_e=doc_e->next;
  if (doc_e!=doc) {
    if (doc_e->de_flags&DOCEF_TAG) {
      ptr=doc_e->tag;
      if (ptr2=StrMatch("//",ptr))
	ptr=ptr2+2;
      else if (ptr2=StrMatch("/*",ptr))
	ptr=ptr2+2;
      else if (!StrNCmp(ptr,"public",6))
	ptr+=6;
      while (*ptr==CH_SPACE)
	ptr++;
      result=StrNew(ptr);
      doc_e=doc_e->next;
    }
    while (doc_e!=doc && doc_e->type_u8!=DOCT_NEW_LINE) {
      if (doc_e->type_u8==DOCT_TAB) {
	ptr=MStrPrint("%s ",result);
	Free(result);
	result=ptr;
      } else if (doc_e->de_flags&DOCEF_TAG) {
	ptr=MStrPrint("%s%s",result,doc_e->tag);
	Free(result);
	result=ptr;
      }
      doc_e=doc_e->next;
    }
  }
  XTalkWait(task,"%c",CH_SHIFT_ESC);
  if (result) {
    ptr=MStrUtil(result,SUF_REM_TRAILING|SUF_REM_LEADING|SUF_SINGLE_SPACE);
    Free(result);
    result=ptr;
  }
  return result;
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
{//Dump hash symbol table.
// "+p" for only public symbols
// "+m" to order by number (normally alphabetical)
// "-r" just local hash table
  CHashTable *table;
  CHashSrcSym *temph;
  CHashGeneric *ptr;
  CWho *lst;
  I64 cnt,i,j,k,size,f=0,dbg_bin_num;
  U8 buf[512],*st,*src_link,*last_index=StrNew(""),*cur_index,*comment;
  CDoc *doc;
  Bool recurse,publics,map,old_preempt=Preempt(OFF);
  CDocBin *tempb;
  CDbgInfo *dbg_info;
  CTask *task;

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

  if (index) {
    task=SpawnUser;
    TaskWait(task);
    LBtr(&task->display_flags,DISPLAYf_SHOW);
  } else
    task=NULL;

  cnt=0;
  table=h;
  while (table) {
    temph=table->next_added;
    while (temph!=table) {
      if (!(temph->type & (HTF_IMPORT | HTF_PRIVATE)) &&
	    (temph->type & HTF_PUBLIC || !publics)) {
	if (!index)
	  cnt++;
	else if (temph->type&HTG_SRC_SYM && (cur_index=temph->index))
	  cnt+=HelpIndexCnt(cur_index,index);
      }
      temph=temph->next_added;
    }
    if (recurse)
      table=table->next;
    else
      break;
  }
  if (!cnt) goto wh_done;

  lst=CAlloc(cnt*sizeof(CWho));
  j=0;
  table=h;
  while (table) {
    temph=table->next_added;
    while (temph!=table) {
      if (!(temph->type & (HTF_IMPORT | HTF_PRIVATE)) &&
	    (temph->type & HTF_PUBLIC || !publics))
	if (!index)
	  lst[j++].h=temph;
	else if (temph->type&HTG_SRC_SYM && (cur_index=temph->index) &&
	      (k=HelpIndexCnt(cur_index,index)))
	  while (k--) {
	    lst[j].index=HelpIndexStr(&cur_index,index);
	    lst[j++].h=temph;
	  }
      temph=temph->next_added;
    }
    if (recurse)
      table=table->next;
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

  if (index) {
    progress1_max=cnt;
    progress1=0;
  }
  for (i=0;i<cnt;i++) {
    comment=NULL;
    ptr=lst[i].h;
    dbg_info=NULL;
    if (index)
      if (cur_index=lst[i].index) {
	if (StrCmp(cur_index,last_index)) {
	  Free(last_index);
	  last_index=StrNew(cur_index);
	  if (i)
	    DocPrint(doc,"\n\n");
	  DocPrint(doc,"$$WW,0$$$$PURPLE$$$$TX+CX,\"%Q\"$$$$FG$$\n",cur_index);
	}
      }

    if (index && ptr->type & HTT_HELP_FILE) {
      DocPrint(doc,"$$WW,1$$");
      DocType(doc,ptr->str);
      DocPrint(doc,"$$WW,0$$");
    } else {
      if (ptr->type&HTG_SRC_SYM && ptr(CHashSrcSym *)->src_link) {
	src_link=StrNew(ptr(CHashSrcSym *)->src_link);
	if (mapfile_drv && StrLen(src_link)>4 && buf[1]==':' && src_link[4]==':')
	  src_link[3]=mapfile_drv;
	dbg_info=NULL;
	if (outname && doc && ptr->type & HTT_FUN) {
	  if (dbg_info=ptr(CHashFun *)->dbg_info) {
	    size=offset(CDbgInfo.body)+sizeof(U32)*(dbg_info->max_line+2-dbg_info->min_line);
	    if (size>MSize(dbg_info)) {
	      "Corrupt Map Entry\n";
	      dbg_info=NULL;
	    } else {
	      if (dbg_info->min_line<=dbg_info->max_line) {
		tempb=CAlloc(sizeof(CDocBin));
		tempb->size=size;
		tempb->data=MAlloc(size);
		MemCpy(tempb->data,dbg_info,size);
		dbg_bin_num=tempb->num=doc->cur_bin_num++;
		tempb->use_cnt=1;
		QueIns(tempb,doc->bin_root.last);
	      } else
		dbg_info=NULL;
	    }
	  }
	}
	if (dbg_info)
	  DocPrint(doc,"$$LK,\"%-20s\",A=\"%s\",BI=%d$$",ptr->str,src_link,dbg_bin_num);
	else {
	  DocPrint(doc,"$$LK,\"%-20s\",A=\"%s\"$$",ptr->str,src_link);
	  if (index)
	    comment=HelpComment(task,ptr,src_link);
	}
	Free(src_link);
      } else
	DocPrint(doc,"%-20s",ptr->str);

      if (!index) {
	if (ptr->type & HTT_DEFINE_STR) {
	  st=MStrUtil(ptr(CHashDefineStr *)->data,SUF_SAFE_DOLLAR);
	  j=ptr(CHashDefineStr *)->cnt;
	  if (j==-1)
	    StrPrint(buf,"%-10tQ   ",st);
	  else
	    StrPrint(buf,"%-10tQ %02X",st,j);
	  Free(st);
	} else if (ptr->type & HTT_GLBL_VAR) {
	  if (outname)
	    StrPrint(buf,"%010X   ",ptr(CHashGlblVar *)->data_addr_ip);
	  else
	    StrPrint(buf,"%010X   ",ptr(CHashGlblVar *)->data_addr);
	} else
	  StrPrint(buf,"%010X   ",HashVal(ptr));
	j=HashEntrySize(ptr);
	if (j==-1)
	  CatPrint(buf," %04X            ",ptr->use_cnt);
	else
	  CatPrint(buf," %04X %010X ",ptr->use_cnt,j);
      } else
	*buf=0;

      k=ptr->type;
      if (publics)
	k&=~HTF_PUBLIC;
      if (!(k&HTG_TYPE_MASK))
	CatPrint(buf,"NULL ");
      while (k) {
	j=Bsf(k);
	if (j<0)
	  break;
	Btr(&k,j);
	CatPrint(buf,"%Z ",j,"ST_HTT_TYPES");
      }
      DocPrint(doc,"%s",buf);
      if (comment) {
	DocPrint(doc,"$$GREEN$$%s$$FG$$",comment);
	Free(comment);
      }
      DocPrint(doc,"\n");
    }
    Free(lst[i].index);
    if (index)
      progress1++;
  }
  Free(lst);
  if (index)
    progress1=progress1_max=0;

wh_done:
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
  Kill(task);
}

#define HDR_MAX	16
public I64 HashDepthRep(CHashTable *table=NULL)
{//Hash table linked-list chain depth report.
//Histogram of collision count.
  I64 i,j,longest=0,cnt=0,a[HDR_MAX];
  CHash *temph;
  if (!table) table=Fs->hash_table;
  MemSet(a,0,sizeof(a));
  for (i=0;i<=table->mask;i++) {
    temph=table->body[i];
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
    table->mask+1,cnt,longest;
  return longest;
}

#help_index "Help System;Training"
#help_file "::/Doc/HelpSystem"

public U0 DocHelpIndex(CDoc *doc,U8 *index)
{//Put to doc report for given help index.
  Who("+p",,,,index,doc);
}

public U0 PopUpHelpIndex(U8 *index,CTask *parent=NULL)
{//PopUp win report for given help index.
  U8 *buf;
  buf=MStrPrint("DocHelpIndex(DocPut,\"%s\");View;",index);
  PopUp(buf,parent);
  Free(buf);
}

#help_index "Hash"
public U0 MapFileLoad(U8 *filename)
{//Load map file so we have src line info.
  U8 *st,*ptr,*name=DftExt(filename,"MAP.Z"),
	*absname=FileNameAbs(name);
  CDoc *doc=DocRead(name);
  CDocEntry *doc_e;
  CHashSrcSym *temph;
  I64 i,j,base=0;
  CDbgInfo *dbg_info;

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
	      !(dbg_info=temph->dbg_info) && doc_e->bin_data &&
	      (dbg_info=doc_e->bin_data->data)) {
	  if (doc_e->bin_data->size>MSize(dbg_info))
	    "Corrupt Map Entry\n";
	  else {
	    doc_e->bin_data->data=NULL;
	    temph->dbg_info=dbg_info;
	    for (i=dbg_info->min_line;i<=dbg_info->max_line+1;i++) {
	      j=i-dbg_info->min_line;
	      if (dbg_info->body[j])
		dbg_info->body[j]=dbg_info->body[j]+base;
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
