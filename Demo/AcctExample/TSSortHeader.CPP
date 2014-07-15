class CSHEntry
{
  CDocEntry *next,*last;
};

class CSHSection
{
  CDocEntry *next,*last;
};

U8 *SHSkipDataType(U8 *st)
{
  if (!st)
    return NULL;
  while (Bt(chars_bitmap_white_space,*st))
    st++;
  if (*st=='#')
    return st;
  while (Bt(chars_bitmap_alpha_numeric,*st))
    st++;
  while (Bt(chars_bitmap_white_space,*st))
    st++;
  while (*st=='*')
    st++;
  return st;
}

U8 *SHSkipLabel(U8 *st)
{
  if (!st)
    return NULL;
  while (Bt(chars_bitmap_white_space,*st))
    st++;
  while (Bt(chars_bitmap_alpha_numeric,*st))
    st++;
  while (Bt(chars_bitmap_white_space,*st))
    st++;
  return st;
}

U8 *SHNewEntry(CDocEntry *doc_e,I64 *_type=NULL)
{
  U8 *st;
  if (doc_e->type_u8==DOCT_TEXT && (st=doc_e->tag)) {
    if (!StrNCmp("#help_index",st,11)) {
      if (_type) *_type=0;
      return st+11;
    } else if (!StrNCmp("#help_file",st,10)) {
      if (_type) *_type=1;
      return st+10;//'#' makes it first alphabetically.
    } else if (!StrNCmp("import",st,6)) {
      if (_type) *_type=2;
      return st+6;
    } else if (!StrNCmp("public import",st,13)) {
      if (_type) *_type=2;
      return st+13;
    } else if (!StrNCmp("_import",st,7)) {
      if (_type) *_type=2;
      return SHSkipLabel(st+7);
    } else if (!StrNCmp("public _import",st,14)) {
      if (_type) *_type=2;
      return SHSkipLabel(st+14);
    } else if (!StrNCmp("_intern",st,7)) {
      if (_type) *_type=3;
      return SHSkipLabel(st+7);
    } else if (!StrNCmp("public _intern",st,14)) {
      if (_type) *_type=3;
      return SHSkipLabel(st+14);
    } else if (!StrNCmp("extern",st,6)) {
      if (_type) *_type=4;
      return st+6;
    } else if (!StrNCmp("public extern",st,13)) {
      if (_type) *_type=4;
      return st+13;
    } else if (!StrNCmp("_extern",st,7)) {
      if (_type) *_type=4;
      return SHSkipLabel(st+7);
    } else if (!StrNCmp("public _extern",st,14)) {
      if (_type) *_type=4;
      return SHSkipLabel(st+14);
    }
  }
  if (_type) *_type=-1;
  return NULL;
}

I64 SHCompareEntry(CSHEntry *e1,CSHEntry *e2)
{
  U8 *st1,*st2;
  I64 t1,t2;
  if (e1 && e2 && e1->next!=e1 && e2->next!=e2) {
    if (e1->next->type_u8==DOCT_TEXT && e2->next->type_u8==DOCT_TEXT) {
      st1=SHNewEntry(e1->next,&t1);
      st2=SHNewEntry(e2->next,&t2);
      if (t1!=t2)
	return t1-t2;
      else {
	if (st1&&st2) {
	  st1=SHSkipDataType(st1);
	  st2=SHSkipDataType(st2);
	  if (st1&&st2)
	    return StrCmp(st1,st2);
	  else
	    return Sign(st1-st2);
	} else
	  return Sign(st1-st2);
      }
    } else
      return Sign(e1->next-e2->next);
  } else
    return Sign(e1-e2);
}

U0 SHSortSection(CSHSection *temps)
{
  I64 i=0,cnt;
  CSHEntry **a;
  CDocEntry *doc_e,*doc_e1;

  cnt=0;
  doc_e=temps->next;
  while (doc_e!=temps) {
    if (SHNewEntry(doc_e))
      cnt++;
    doc_e=doc_e->next;
  }
  cnt++;

  a=MAlloc(sizeof(CSHEntry *)*cnt);
  for (i=0;i<cnt;i++) {
    a[i]=MAlloc(sizeof(CSHEntry));
    QueInit(a[i]);
  }

  i=0;
  doc_e=temps->next;
  while (doc_e!=temps) {
    doc_e1=doc_e->next;
    QueRem(doc_e);
    if (SHNewEntry(doc_e))
      i++;
    QueIns(doc_e,a[i]->last);
    doc_e=doc_e1;
  }

  QSortI64(a,cnt,&SHCompareEntry);

  for (i=0;i<cnt;i++) {
    doc_e=a[i]->next;
    while (doc_e!=a[i]) {
      doc_e1=doc_e->next;
      QueRem(doc_e);
      QueIns(doc_e,temps->last);
      doc_e=doc_e1;
    }
    Free(a[i]);
  }
  Free(a);
}

U8 *SHNewSection(CDocEntry *doc_e)
{
  U8 *st;
  if (doc_e->type_u8==DOCT_TEXT && (st=doc_e->tag)) {
    if (!StrNCmp(st,"#help_index",11))
      return st+11;
  }
  return NULL;
}

I64 SHCompareSection(CSHSection *e1,CSHSection *e2)
{
  I64 i1,i2;
  U8 *st1,*st2;
  if (e1 && e2 && e1->next!=e1 && e2->next!=e2) {
    if (e1->next->type_u8==DOCT_TEXT && e2->next->type_u8==DOCT_TEXT) {
     st1=SHNewSection(e1->next);
     st2=SHNewSection(e2->next);
     if (st1&&st2) {
       if (StrMatch("\"\"",st1))
	 i1=1;
       else
	 i1=0;
       if (StrMatch("\"\"",st2))
	 i2=1;
       else
	 i2=0;
       if (i1!=i2)
	 return i1-i2;
       else
	 return StrCmp(st1,st2);
     } else
       return Sign(st1-st2);
    } else
      return Sign(e1->next-e2->next);
  } else
    return Sign(e1-e2);
}

public U0 TSSortHeader(U8 *name)
{
  CDoc *doc;
  I64 i=0,cnt;
  CSHSection **a;
  CDocEntry *doc_e,*doc_e1;

  if (!name || !*name || !FileFind(name))
    return;

  doc=DocRead(name,DOCF_PLAIN_TEXT_WITH_TABS|DOCF_NO_CURSOR);

  cnt=1;
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (SHNewSection(doc_e))
      cnt++;
    doc_e=doc_e->next;
  }

  a=MAlloc(sizeof(CSHSection *)*cnt);
  for (i=0;i<cnt;i++) {
    a[i]=MAlloc(sizeof(CSHSection));
    QueInit(a[i]);
  }

  i=0;
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    doc_e1=doc_e->next;
    if (doc_e->type_u8==DOCT_TEXT && doc_e->tag && *doc_e->tag) {
      QueRem(doc_e);
      if (SHNewSection(doc_e))
	i++;
      QueIns(doc_e,a[i]->last);
    } else
      DocEntryDel(doc,doc_e);
    doc_e=doc_e1;
  }

  QSortI64(a,cnt,&SHCompareSection);

  for (i=0;i<cnt;i++) {
    if (a[i]->next!=a[i]) {
      SHSortSection(a[i]);
      doc_e=a[i]->next;
      while (doc_e!=a[i]) {
	doc_e1=doc_e->next;
	QueRem(doc_e);
	QueIns(doc_e,doc->root.last);
	if (doc_e->type_u8==DOCT_TEXT && doc_e->tag) {
	  doc->cur_entry=doc_e;
	  doc->cur_data_col=StrLen(doc_e->tag);
	  DocPrint(doc,"\n");
	}
	doc_e=doc_e1;
      }
      if (i+1<cnt && a[i+1]->next!=a[i+1])
	DocPrint(doc,"\n");
    }
    Free(a[i]);
  }
  Free(a);
  DocWrite(doc);
  DocDel(doc);
}

public U0 TSSortHeaders()
{
  TSSortHeader("/Kernel/Adam1b.HPP.Z");
  TSSortHeader("/Kernel/Adam1c.HPP.Z");
  TSSortHeader("/Kernel/KExts.CPP.Z");
  TSSortHeader("/Compiler/CmpExts.CPP.Z");
  TSSortHeader("/Compiler/Cmp.HPP.Z");
  TSSortHeader("/Adam/AdamExts.HPP.Z");
}

