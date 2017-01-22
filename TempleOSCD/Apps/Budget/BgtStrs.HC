#define SFT_GENERIC	1

public U8 **StrFileRead(U8 *name,I64 *_max_num=NULL,
	U8 **_colors=NULL,Bool no_nums=FALSE)
{
  CDoc		*doc=DocRead(name,DOCF_DBL_DOLLARS|DOCF_NO_CURSOR);
  CDocEntry	*doc_e=doc->head.next;
  I64		i,max_num=0;
  U8		*ptr,**res,*colors;

  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT) {
      if (no_nums)
	i=++max_num;
      else {
	i=Str2I64(doc_e->tag,,&ptr);
	if (i>max_num) max_num=i;
	if (*ptr==',') ptr++;
	ptr=StrNew(ptr);
	Free(doc_e->tag);
	doc_e->tag=ptr;
      }
      doc_e->user_data=i;
    }
    doc_e=doc_e->next;
  }

  res=CAlloc(sizeof(U8 *)*(max_num+1));
  colors=CAlloc(sizeof(U8)*(max_num+1));
  doc_e=doc->head.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT && 0<=doc_e->user_data<=max_num) {
      res[doc_e->user_data]=doc_e->tag;
      doc_e->tag=NULL;
      colors[doc_e->user_data]=doc_e->type.u8[1]&15;
    }
    doc_e=doc_e->next;
  }

  DocDel(doc);
  if (_max_num) *_max_num=max_num;
  if (_colors)
    *_colors=colors;
  else
    Free(colors);
  return res;
}

public U0 StrFileArrDel(U8 **a,I64 max_num)
{
  I64 i;
  for (i=0;i<=max_num;i++)
    Free(a[i]);
  Free(a);
}

public I64 StrFileAdd(U8 *st,I64 *_num,
	CHashTable *table,I64 color=COLOR_INVALID)
{
  CHashGeneric *tmph;
  if (!st) return 0;
  if (!(tmph=HashFind(st,table,SFT_GENERIC))) {
    tmph=CAlloc(sizeof(CHashGeneric));
    tmph->type=SFT_GENERIC;
    tmph->str=StrNew(st);
    tmph->user_data0=(*_num)++;
    HashAdd(tmph,table);
  }
  if (color!=COLOR_INVALID)
    tmph->user_data1=color;
  return tmph->user_data0;
}

I64 StrEntriesCompare(CHashGeneric *h1,CHashGeneric *h2)
{
  return h1->user_data0-h2->user_data0;
}

public U0 StrFileWrite(U8 *name,CHashTable *table,Bool no_nums=FALSE)
{
  I64 i,j,cnt,color=BLACK;
  CDoc *doc=DocNew(name);
  CHashGeneric *tmph,**a;
  if (table) {
    cnt=0;	//Count Strings
    for (i=0;i<=table->mask;i++)
      cnt+=LinkedLstCnt(table->body[i]);
    a=MAlloc(cnt*sizeof(CHashGeneric *));
    j=0;	//Load Strings
    for (i=0;i<=table->mask;i++) {
      tmph=table->body[i];
      while (tmph) {
	a[j++]=tmph;
	tmph=tmph->next;
      }
    }
    QSortI64(a,cnt,&StrEntriesCompare);
    for (i=0;i<cnt;i++) {
      tmph=a[i];
      if (tmph->user_data1&15!=color) {
	DocPrint(doc,"$$FG,%d$$",tmph->user_data1&15);
	color=tmph->user_data1&15;
      }
      if (no_nums)
	DocPrint(doc,"%s\n",tmph->str);
      else
	DocPrint(doc,"%d,%s\n",tmph->user_data0,tmph->str);
    }
    Free(a);
  }
  doc->flags|=DOCF_NO_CURSOR;
  DocWrite(doc);
  DocDel(doc);
}

public U0 StrFileDel(CHashTable *table)
{
  I64 i;
  CHashGeneric *tmph,*tmph1;
  if (!table) return;
  for (i=0;i<=table->mask;i++) {
    tmph=table->body[i];
    while (tmph) {
      tmph1=tmph->next;
      Free(tmph->str);
      Free(tmph);
      tmph=tmph1;
    }
  }
  Free(table->body);
  Free(table);
}
