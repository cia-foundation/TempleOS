#help_index "Info;Hash/System;Cmd Line (Typically)"
class CWho
{
  CHashGeneric *h;
  U8 *idx;
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
  CHashFun *tmpf1=h1->h,*tmpf2=h2->h;
  I64 i1=HashVal(tmpf1),i2=HashVal(tmpf2);
  if (i1==i2) {
    i1=HashTypeNum(tmpf1);
    i2=HashTypeNum(tmpf2);
    if (i1==i2)
      return StrCmp(tmpf1->str,tmpf2->str);
  }
  return i1-i2;
}

I64 HelpIndexCnt(U8 *ptr,U8 *idx)
{
  I64 cnt=0,ch,idx_len=StrLen(idx);
  while (*ptr) {
    if (!StrNCmp(ptr,idx,idx_len))
      cnt++;
    while (ch=*ptr++)
      if (ch==';')
	break;
    if (!ch)
      ptr--;
  }
  return cnt;
}

U8 *HelpIndexStr(U8 **_ptr,U8 *idx)
{
  U8 *ptr=*_ptr,*ptr2,*res;
  I64 ch,idx_len=StrLen(idx);
  while (*ptr) {
    ptr2=ptr;
    while (ch=*ptr++)
      if (ch==';')
	break;
    if (!ch)
      ptr--;
    *_ptr=ptr;
    if (!StrNCmp(ptr2,idx,idx_len)) {
      if (ch==';')
	ptr--;
      *ptr=0;
      res=StrNew(ptr2);
      *ptr=ch;
      return res;
    }
  }
  return NULL;
}

U8 *HelpComment(CTask *task,CHash *tmph,U8 *_src_link)
{
  CDoc *doc;
  CDocEntry *doc_e;
  U8 *res=NULL,*ptr,*ptr2,*src_link=StrNew(_src_link);

  if (*src_link=='F' && src_link[2]==':')
    *src_link='P';
  XTalkWait(task,"Ed(0x%X,DOF_DONT_WINMGR_SYNC|DOF_DONT_SHOW);\n",src_link);
  Free(src_link);

  doc=DocPut(task);
  doc_e=doc->cur_entry;
  if (tmph->type&HTT_FUN) {
    if (Bt(&tmph(CHashFun *)->flags,Ff__EXTERN) ||
	  Bt(&tmph(CHashFun *)->flags,Ff_INTERNAL))
      while (doc_e!=doc &&
	    (!(doc_e->de_flags&DOCEF_TAG)||!StrOcc(doc_e->tag,';')))
	doc_e=doc_e->next;
    else
      while (doc_e!=doc &&
	    (!(doc_e->de_flags&DOCEF_TAG)||!StrOcc(doc_e->tag,'{')))
	doc_e=doc_e->next;
  }
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
      res=StrNew(ptr);
      doc_e=doc_e->next;
    }
    while (doc_e!=doc && doc_e->type_u8!=DOCT_NEW_LINE) {
      if (doc_e->type_u8==DOCT_TAB) {
	ptr=MStrPrint("%s ",res);
	Free(res);
	res=ptr;
      } else if (doc_e->de_flags&DOCEF_TAG) {
	ptr=MStrPrint("%s%s",res,doc_e->tag);
	Free(res);
	res=ptr;
      }
      doc_e=doc_e->next;
    }
  }
  XTalkWait(task,"%c",CH_SHIFT_ESC);
  if (res) {
    ptr=MStrUtil(res,SUF_REM_TRAILING|SUF_REM_LEADING|SUF_SINGLE_SPACE);
    Free(res);
    res=ptr;
  }
  return res;
}

I64 HashEntriesCompare3(CWho *h1,CWho *h2)
{
  I64 i,i1=0,i2=0;
  i=StrCmp(h1->idx,h2->idx);
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
U8 *idx=NULL,CDoc *doc=NULL)
{//Dump hash symbol table.
// "+p" for only public symbols
  // "+m" to order by number (normally alphabetical)
  // "-r" just local hash table
  CHashTable *table;
  CHashSrcSym *tmph;
  CHashGeneric *ptr;
  CWho *lst;
  I64 cnt,i,j,k,f=0;
  U8 buf[512],*last_idx=StrNew(""),*cur_idx,*comment;
  Bool recurse,publics,map;
  CTask *task;

  ScanFlags(&f,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&f,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (f&~(FUF_RECURSE|FUF_PUBLIC|FUF_MAP))
    throw('FUF');
  recurse=Bt(&f,FUf_RECURSE);
  publics=Bt(&f,FUf_PUBLIC);
  map	 =Bt(&f,FUf_MAP);

  if (!h) h=Fs->hash_table;

  if (idx) {
    task=User;
    TaskWait(task);
    LBtr(&task->display_flags,DISPLAYf_SHOW);
  } else
    task=NULL;

  cnt=0;
  table=h;
  while (table) {
    for (i=0;i<=table->mask;i++) {
      tmph=table->body[i];
      while (tmph) {
	if (!(tmph->type & (HTF_IMPORT | HTF_PRIVATE)) &&
	      (tmph->type & HTF_PUBLIC || !publics)) {
	  if (!idx)
	    cnt++;
	  else if (tmph->type&HTG_SRC_SYM && (cur_idx=tmph->idx))
	    cnt+=HelpIndexCnt(cur_idx,idx);
	}
	tmph=tmph->next;
      }
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
    for (i=0;i<=table->mask;i++) {
      tmph=table->body[i];
      while (tmph) {
	if (!(tmph->type & (HTF_IMPORT | HTF_PRIVATE)) &&
	      (tmph->type & HTF_PUBLIC || !publics))
	  if (!idx)
	    lst[j++].h=tmph;
	  else if (tmph->type&HTG_SRC_SYM && (cur_idx=tmph->idx) &&
		(k=HelpIndexCnt(cur_idx,idx)))
	    while (k--) {
	      lst[j].idx=HelpIndexStr(&cur_idx,idx);
	      lst[j++].h=tmph;
	    }
	tmph=tmph->next;
      }
    }
    if (recurse)
      table=table->next;
    else
      break;
  }

  if (map)
    QSort(lst,cnt,sizeof(CWho),&HashEntriesCompare2);
  else if (idx)
    QSort(lst,cnt,sizeof(CWho),&HashEntriesCompare3);
  else
    QSort(lst,cnt,sizeof(CWho),&HashEntriesCompare);

  if (idx) {
    progress1_max=cnt;
    progress1=0;
  }
  for (i=0;i<cnt;i++) {
    comment=NULL;
    ptr=lst[i].h;
    if (idx)
      if (cur_idx=lst[i].idx) {
	if (StrCmp(cur_idx,last_idx)) {
	  Free(last_idx);
	  last_idx=StrNew(cur_idx);
	  if (i)
	    DocPrint(doc,"\n\n");
	  DocPrint(doc,"$$WW,0$$$$PURPLE$$$$TX+CX,\"%$$Q\"$$$$FG$$\n",cur_idx);
	}
      }

    if (idx && ptr->type & HTT_HELP_FILE) {
      DocPrint(doc,"$$WW,1$$");
      DocType(doc,ptr->str);
      DocPrint(doc,"$$WW,0$$");
    } else {
      if (ptr->type&HTG_SRC_SYM && ptr(CHashSrcSym *)->src_link) {
	DocPrint(doc,"$$LK,\"%-20s\",A=\"%s\"$$",
	      ptr->str,ptr(CHashSrcSym *)->src_link);
	if (idx)
	  comment=HelpComment(task,ptr,ptr(CHashSrcSym *)->src_link);
      } else
	DocPrint(doc,"%-20s",ptr->str);

      if (!idx) {
	if (ptr->type & HTT_DEFINE_STR) {
	  j=ptr(CHashDefineStr *)->cnt;
	  if (j==-1)
	    StrPrint(buf,"%-10t$$Q   ",ptr(CHashDefineStr *)->data);
	  else
	    StrPrint(buf,"%-10t$$Q %02X",ptr(CHashDefineStr *)->data,j);
	} else if (ptr->type & HTT_GLBL_VAR)
	  StrPrint(buf,"%010X   ",ptr(CHashGlblVar *)->data_addr);
	else
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
    Free(lst[i].idx);
    if (idx)
      progress1++;
  }
  Free(lst);
  if (idx)
    progress1=progress1_max=0;

wh_done:
  if (doc) {
    if (doc->head.next==doc)
      DocPrint(doc,"No Match");
    else
      DocRecalc(doc);
  }
  Free(last_idx);
  Kill(task);
}

#help_index "Info;Hash;Cmd Line (Typically)"

#define HDR_NUM	16
public I64 HashDepthRep(CHashTable *table=NULL)
{//Hash table linked-list chain depth report.
//Histogram of collision count.
  I64 i,j,longest=0,cnt=0,a[HDR_NUM];
  CHash *tmph;
  if (!table) table=Fs->hash_table;
  MemSet(a,0,sizeof(a));
  for (i=0;i<=table->mask;i++) {
    tmph=table->body[i];
    if (tmph) {
      j=LinkedLstCnt(tmph);
      if (j<HDR_NUM)
	a[j]++;
      cnt+=j;
      if (j>longest)
	longest=j;
    }
  }
  "Histogram\n";
  for (i=0;i<HDR_NUM;i++)
    if (a[i])
      "%02d:%d\n",i,a[i];
  "Size:%d Count:%d Longest:%d\n",
	table->mask+1,cnt,longest;
  return longest;
}

#help_index "Help System"
#help_file "::/Doc/HelpSystem"

public U0 DocHelpIdx(CDoc *doc,U8 *idx)
{//Put to doc report for given help idx.
  Who("+p",,idx,doc);
}

public U0 PopUpHelpIndex(U8 *idx,CTask *parent=NULL)
{//PopUp win report for given help idx.
  U8 *buf;
  buf=MStrPrint("DocHelpIdx(DocPut,\"%s\");View;",idx);
  PopUp(buf,parent);
  Free(buf);
}

#help_index "Hash/System"
public U0 MapFileLoad(U8 *filename)
{//Load map file so we have src line info.
  U8 *st,*ptr,*name=ExtDft(filename,"MAP.Z"),
	*absname=FileNameAbs(name);
  CDoc *doc=DocRead(name);
  CDocEntry *doc_e;
  CHashSrcSym *tmph;
  I64 i,j,base=0;
  CDbgInfo *dbg_info;

  FileExtRem(absname);
  if (absname[1]==':' && StrLen(absname)>2 &&
	(tmph=HashSingleTableFind(absname+2,Fs->hash_table,HTT_MODULE)))
    base=tmph(CHashGeneric *)->user_data0+sizeof(CBinFile);

  if (!doc) return;
  doc_e=doc->head.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_LINK) {
      if (*doc_e->tag)
	st=MStrUtil(doc_e->tag,SUF_REM_TRAILING);
      else
	st=MStrUtil(doc_e->aux_str,SUF_REM_TRAILING);
      if (tmph=HashSingleTableFind(st,Fs->hash_table,HTG_SRC_SYM)) {
	if (*doc_e->tag) {
	  Free(tmph->src_link);
	  tmph->src_link=doc_e->aux_str;
	  ptr=tmph->src_link;
	  if (ptr[0] && ptr[1] && ptr[2]==':') {
	    if (ptr[3]==':')
	      ptr[3]=blkdev.boot_drv_let;
	    else if (ptr[3]=='~')
	      ptr[3]=*blkdev.home_dir;
	  }
	  doc_e->aux_str=NULL;
	}
	if (tmph->type&(HTT_FUN|HTT_EXPORT_SYS_SYM) &&
	      !(dbg_info=tmph->dbg_info) && doc_e->bin_data &&
	      (dbg_info=doc_e->bin_data->data)) {
	  if (doc_e->bin_data->size>MSize(dbg_info))
	    "Corrupt Map Entry\n";
	  else {
	    doc_e->bin_data->data=NULL;
	    tmph->dbg_info=dbg_info;
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
