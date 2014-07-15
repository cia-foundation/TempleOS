#define SFT_GENERIC	1

public U8 **StrFileRead(U8 *name,I64 *_max_num=NULL,U8 **_colors=NULL,Bool no_nums=FALSE)
{
  CDoc		*doc=DocRead(name,DOCF_DBL_DOLLARS|DOCF_NO_CURSOR);
  CDocEntry	*doc_e=doc->root.next;
  I64		i,max_num=0;
  U8		*ptr,**result,*colors;

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

  result=CAlloc(sizeof(U8 *)*(max_num+1));
  colors=CAlloc(sizeof(U8)*(max_num+1));
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT && 0<=doc_e->user_data<=max_num) {
      result[doc_e->user_data]=doc_e->tag;
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
  return result;
}

public U0 StrFileArrDel(U8 **a,I64 max_num)
{
  I64 i;
  for (i=0;i<=max_num;i++)
    Free(a[i]);
  Free(a);
}

public I64 StrFileAdd(U8 *st,CHashTable *table,I64 color=COLOR_INVALID)
{
  CHashGeneric *temph;
  if (!st) return 0;
  if (!(temph=HashFind(st,table,SFT_GENERIC))) {
    temph=CAlloc(sizeof(CHashGeneric));
    temph->type=SFT_GENERIC;
    temph->str=StrNew(st);
    if (table->last_added==table)
      temph->user_data0=1;
    else
      temph->user_data0=table->last_added(CHashGeneric *)->user_data0+1;
    HashAdd(temph,table);
  }
  if (color!=COLOR_INVALID)
    temph->user_data1=color;
  return temph->user_data0;
}

public U0 StrFileWrite(U8 *name,CHashTable *table,Bool no_nums=FALSE)
{
  I64 color=BLACK;
  CDoc *doc=DocNew(name);
  CHashGeneric *temph;
  if (table) {
    temph=table->next_added;
    while (temph!=table) {
      if (temph->user_data1&15!=color) {
	DocPrint(doc,"$$FG,%d$$",temph->user_data1&15);
	color=temph->user_data1&15;
      }
      if (no_nums)
	DocPrint(doc,"%s\n",temph->str);
      else
	DocPrint(doc,"%d,%s\n",temph->user_data0,temph->str);
      temph=temph->next_added;
    }
  }
  doc->flags|=DOCF_NO_CURSOR;
  DocWrite(doc);
  DocDel(doc);
}

public U0 StrFileDel(CHashTable *table)
{
  CHashGeneric *temph,*temph1;
  if (!table) return;
  temph=table->next_added;
  while (temph!=table) {
    temph1=temph->next_added;
    Free(temph->str);
    Free(temph);
    temph=temph1;
  }
  Free(table->body);
  Free(table);
}
