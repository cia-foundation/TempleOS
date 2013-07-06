#help_index "Hash"
public I64 HashVal(CHash *temph)
{
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
      if (temph(CHashGlblVar *)->flags&GVAF_EXTERN)
	return &temph(CHashGlblVar *)->data_addr;
      else
	return temph(CHashGlblVar *)->data_addr;
    case HTt_FUN:
      if (Bt(temph(CHashFun *)->flags,Cf_EXTERN))
	return temph;
      else
	return temph(CHashFun *)->executable_addr;
    case HTt_REG:
      return temph(CHashReg *)->reg_num|temph(CHashReg *)->reg_type<<8;
    case HTt_KEYWORD:
    case HTt_ASM_KEYWORD:
    case HTt_MODULE:
    case HTt_FILE:
    case HTt_NAMED_FRAME_PTR:
      return temph(CHashGeneric *)->user_data0;

    case -1:		//nobound_switch
    case HTt_NUM_TYPES: //nobound_switch
    default:
      return 0;
  }
}

#help_index "Info;Hash/Reports;"
class CHashRep
{
  CHashGeneric *h;
  U8 *index;
};


I64 HashEntriesCompare(CHashRep *h1,CHashRep *h2)
{
  I64 i1,i2;
  if (i1=StrCmp(h1->h->str,h2->h->str))
    return i1;
  i1=HashTypeNum(h1->h);
  i2=HashTypeNum(h2->h);
  return i1-i2;
}

I64 HashEntriesCompare2(CHashRep *h1,CHashRep *h2)
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
  U8 cnt=0,ch;
  I64 index_len=StrLen(index);
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
  U8 *ptr=*ptr_,*ptr2,ch,*result;
  I64 index_len=StrLen(index);
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

I64 HashEntriesCompare3(CHashRep *h1,CHashRep *h2)
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

//Possible choices for first arg, "h" include
// "adam_task->hash_table"
// "Fs->hash_table"
public U0 HashRep(CHashTable *h=NULL,
  I64 mask=HTG_ALL,U8 *outname=NULL,U8 *flags=NULL,U8 mapfile_drv=0,U8 *index=NULL,
  CDoc *_doc=NULL)
{
  CHashTable *tempht;
  CHashSrcSym *temph;
  CHashGeneric *ptr;
  CHashRep *lst;
  I64 cnt,i,j,k,size,f=0,dbg_bin_num;
  U8 buf[512],*buf4,*last_index=StrNew(""),*cur_index;
  CDoc *doc;
  Bool recurse,publics,map,old_preempt=Preempt(OFF);
  CDocBin *tempb;
  CDbgInfo *dbg;

  GetFFlags(&f,Define("ST_FILE_UTIL_FLAGS"),"+r+p");
  GetFFlags(&f,Define("ST_FILE_UTIL_FLAGS"),flags);
  if (f&~(FUF_RECURSE|FUF_PUBLIC|FUF_MAP))
    throw(EXCEPT_FUF,2);
  recurse=Bt(&f,FUf_RECURSE);
  publics=Bt(&f,FUf_PUBLIC);
  map	 =Bt(&f,FUf_MAP);

  if (!h) h=Fs->hash_table;

  if (_doc)
    doc=_doc;
  else if (outname) {
    doc=DocNew;
    doc->flags|=DOCF_NO_CURSOR;
    StrCpy(doc->filename.name,outname);
  } else
    doc=NULL;

  cnt=0;
  tempht=h;
  while (tempht) {
    temph=tempht->next_added;
    while (temph!=tempht) {
      if (!(temph->type & (HTF_IMPORT | HTF_PRIVATE)) &&
	    temph->type & mask &&
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

  lst=CAlloc(cnt*sizeof(CHashRep));
  j=0;
  tempht=h;
  while (tempht) {
    temph=tempht->next_added;
    while (temph!=tempht) {
      if (!(temph->type & (HTF_IMPORT | HTF_PRIVATE)) &&
	    temph->type & mask &&
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
    QSort(lst,cnt,sizeof(CHashRep),&HashEntriesCompare2);
  else if (index)
    QSort(lst,cnt,sizeof(CHashRep),&HashEntriesCompare3);
  else
    QSort(lst,cnt,sizeof(CHashRep),&HashEntriesCompare);

  for (i=0;i<cnt;i++) {
    ptr=lst[i].h;
    dbg=NULL;
    if (index)
      if (cur_index=lst[i].index) {
	if (StrCmp(cur_index,last_index)) {
	  Free(last_index);
	  last_index=StrNew(cur_index);
	  if (doc)
	    DocPrintF(doc,"$$FG,GREEN$$%s$$FG$$\n",cur_index);
	  else
	    "$$FG,GREEN$$%s$$FG$$\n",cur_index;
	}
      }
    if (index && ptr->type & HTT_HELP_FILE)
      DocType(doc,ptr->str);
    else {
      if (ptr->type&HTG_SRC_SYM && ptr(CHashSrcSym *)->src_link) {
	buf4=StrNew(ptr(CHashSrcSym *)->src_link);
	if (mapfile_drv && StrLen(buf4)>4 && buf[1]==':' && buf4[4]==':')
	  buf4[3]=mapfile_drv;
	dbg=NULL;
	if (outname && doc && ptr->type & HTT_FUN) {
	  if (dbg=ptr(CHashFun *)->debug) {
	    size=offset(CDbgInfo.body)+
		  sizeof(U8 *)*
		  (dbg->max_line-dbg->min_line+2);
	    if (size>MSize(dbg)) {
	      "Corrupt Map Entry\n";
	      dbg=NULL;
	    } else {
	      if (dbg->min_line<=dbg->max_line) {
		tempb=CAlloc(sizeof(CDocBin));
		tempb->type=DOCBT_DEBUG_LINES;
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
	if (dbg) {
	  SPrintF(buf,"$$LK+BI,\"%-26s\",\"%s\",%d$$",
		ptr->str,buf4,dbg_bin_num);
	  DocPutS(doc,buf);
	} else {
	  SPrintF(buf,"$$LK,\"%-26s\",\"%s\"$$",
		ptr->str,buf4);
	  if (doc)
	    DocPutS(doc,buf);
	  else
	    "" buf;
	}
	Free(buf4);
      } else {
	SPrintF(buf,"%-26ts",ptr->str);
	if (doc)
	  DocPutS(doc,buf);
	else
	  "" buf;
      }

      if (!index) {
	if (ptr->type & HTT_DEFINE_STR) {
	  j=ptr(CHashDefineStr *)->cnt;
	  if (j==-1)
	    SPrintF(buf,"%-10tQ   ",ptr(CHashDefineStr *)->data);
	  else
	    SPrintF(buf,"%-10tQ %02X",ptr(CHashDefineStr *)->data,j);
	} else if (ptr->type & HTT_GLBL_VAR) {
	  if (outname)
	    SPrintF(buf,"%010X   ",ptr(CHashGlblVar *)->data_addr_ip);
	  else
	    SPrintF(buf,"%010X   ",ptr(CHashGlblVar *)->data_addr);
	} else
	  SPrintF(buf,"%010X   ",HashVal(ptr));
	j=HashEntrySize(ptr);
	if (j==-1)
	  SPrintF(buf+StrLen(buf)," %04X            ",ptr->use_cnt);
	else
	  SPrintF(buf+StrLen(buf)," %04X %010X ",ptr->use_cnt,j);
      } else
	*buf=0;

      k=ptr->type;
      if (publics)
	k&=~HTF_PUBLIC;

      for (j=0;j<32;j++)
	if (Bt(&k,j))
	  SPrintF(buf+StrLen(buf),"%Z ",j,"ST_HTT_TYPES");
      StrCat(buf,"\n");
      if (doc)
	DocPutS(doc,buf);
      else
	"" buf;
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
	DocPutS(doc,"No Match");
      else
	DocRecalc(doc);
    }
  }
  Free(last_index);
}

public U0 HFunsRep(U8 *flags="",U8 *index=NULL)
{
  HashRep(Fs->hash_table,HTT_FUN,NULL,flags,0,index);
}

public U0 HMapRep(U8 *flags="-p+m",U8 *index=NULL)
{
  HashRep(Fs->hash_table,HTT_FUN,NULL,flags,0,index);
}

public U0 HVarsRep(U8 *flags="",U8 *index=NULL)
{
  HashRep(Fs->hash_table,HTT_GLBL_VAR,NULL,flags,0,index);
}

public U0 HClassesRep(U8 *flags="-p",U8 *index=NULL)
{
  HashRep(Fs->hash_table,HTT_CLASS,NULL,flags,0,index);
}

public U0 HDefinesRep(U8 *flags="-p",U8 *index=NULL)
{
  HashRep(Fs->hash_table,HTT_DEFINE_STR,NULL,flags,0,index);
}

public U0 HSymsRep(U8 *flags="-p",U8 *index=NULL)
{
  HashRep(Fs->hash_table,HTT_EXPORT_SYS_SYM,NULL,flags,0,index);
}

public U0 HAllRep(U8 *flags="-p",U8 *index=NULL)
{
  HashRep(Fs->hash_table,HTG_ALL,NULL,flags,0,index);
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
#help_file "::/TempleOS/Doc/HelpSystem"

public U0 DocHelpIndex(CDoc *doc,U8 *index)
{
  HashRep(Fs->hash_table,HTG_ALL,NULL,NULL,0,index,doc);
}

public U0 PopUpHelpIndex(U8 *index,CTask *parent=NULL)
{
  U8 *buf;
  buf=MSPrintF("DocHelpIndex(DocPut,\"%s\");View;",index);
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
	if (temph->type&(HTT_FUN|HTT_EXPORT_SYS_SYM)) {
	  dbg	=temph->debug;
	  if (!dbg && doc_e->bin_data && doc_e->bin_data->type==DOCBT_DEBUG_LINES) {
	    if (dbg=doc_e->bin_data->data) {
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
