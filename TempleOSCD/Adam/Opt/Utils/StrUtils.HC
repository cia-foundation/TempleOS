#help_index "Cmd Line (Typically)"

I64 DEPtrCompare(CDocEntry **e1,CDocEntry **e2)
{
  return StrCmp((*e1)->tag,(*e2)->tag);
}

public I64 Sort(U8 *_in_name,U8 *_out_name=NULL,
	I64 entry_lines=1,Bool unique=FALSE)
{//Sort lines of a text file. Removes blank lines.
  U8 *in_name,*out_name,*st;
  CDoc *doc;
  CDocEntry *doc_e,*doc_e1,**a;
  I64 i,j,cnt=0,res;

  if (!_in_name) return 0;
  in_name=ExtDft(_in_name,"DD.Z");
  if (_out_name)
    out_name=ExtDft(_out_name,"DD.Z");
  else
    out_name=StrNew(in_name);

  doc=DocRead(in_name,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR);
  doc_e=doc->head.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT)
      cnt++;
    doc_e=doc_e->next;
  }
  a=MAlloc(cnt*sizeof(CDocEntry *));
  doc_e=doc->head.next;
  i=0;
  while (doc_e!=doc) {
    doc_e1=doc_e->next;
    if (doc_e->type_u8==DOCT_TEXT) {
      QueRem(doc_e);
      a[i++]=doc_e;
    } else
      DocEntryDel(doc,doc_e);
    doc_e=doc_e1;
  }
  QSort(a,cnt/entry_lines,entry_lines*sizeof(CDocEntry *),&DEPtrCompare);

  res=0;
  st=NULL;
  for (i=0;i<cnt;) {
    if (!unique || !st || StrCmp(a[i]->tag,st)) {
      st=a[i]->tag;
      for (j=0;j<entry_lines && i<cnt;j++,i++) {
	QueIns(a[i],doc->head.last);
	doc->cur_entry=&doc->head;
	doc->cur_col=0;
	DocPrint(doc,"\n");
      }
      res++;
    } else
      for (j=0;j<entry_lines && i<cnt;j++,i++) {
	QueIns(a[i],doc->head.last);
	DocEntryDel(doc,a[i]);
      }
  }
  StrCpy(doc->filename.name,out_name);
  DocWrite(doc);

  Free(a);
  DocDel(doc);
  Free(in_name);
  Free(out_name);
  return res; //Num Entries
}

I64 DocWordsFile(CDoc *doc_out=NULL,U8 *filename,U32 *char_bmp)
{
  U8 *ptr,*ptr2;
  I64 res=0,ch;
  CDoc *doc_in=DocRead(filename);
  CDocEntry *doc_e=doc_in->head.next;
  while (doc_e!=doc_in) {
    if (doc_e->de_flags & DOCEF_TAG) {
      ptr=doc_e->tag;
      while (*ptr) {
	while (*ptr && !Bt(char_bmp,*ptr))
	  ptr++;

	ptr2=ptr;
	while (*ptr && Bt(char_bmp,*ptr))
	  ptr++;

	ch=*ptr;
	*ptr=0;
	if (*ptr2) {
	  DocPrint(doc_out,"%s\n",ptr2);
	  res++;
	}
	*ptr=ch;
      }
    }
    doc_e=doc_e->next;
  }
  DocDel(doc_in);
  return res;
}
public I64 Words(U8 *files_find_mask="*",U32 *char_bmp=char_bmp_alpha,
	U8 *fu_flags=NULL)
{//Break file into list of not-unique words.
  I64 fuf_flags=0,res=0;
  CDoc *doc_out=DocNew;
  CDirEntry *tmpde,*tmpde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F+T");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tmpde=tmpde1=FilesFind(files_find_mask,fuf_flags);
  while (tmpde) {
    res+=DocWordsFile(doc_out,tmpde->full_name,char_bmp);
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
  DocInsDoc(NULL,doc_out);
  DocDel(doc_out);
  return res;
}

I64 LongLinesFile(U8 *filename,I64 cols)
{
  I64 res=0;
  CDoc *doc=DocRead(filename);
  CDocEntry *doc_e=doc->head.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_NEW_LINE && doc_e->x>=cols+1)
      res++;
    doc_e=doc_e->next;
  }
  DocDel(doc);
  if (res) {
    "%04d ",res;
    PutFileLink(filename);
    '\n';
  }
  return res;
}
public I64 LongLines(U8 *files_find_mask="*",I64 cols=80,U8 *fu_flags=NULL)
{//Report files with lines of too many cols.
  I64 res=0,fuf_flags=0;
  CDirEntry *tmpde,*tmpde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F+S");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tmpde=tmpde1=FilesFind(files_find_mask,fuf_flags);
  while (tmpde) {
    if (LongLinesFile(tmpde->full_name,cols))
      res++;
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
  return res;
}

U0 SUFile(U8 *filename,I64 suf_flags,F64 indent_scale_factor)
{//String utility on a single file
//See $LK,"SU Flags",A="MN:SUF_SINGLE_SPACE"$
  U8 *dst;
  Bool chged=FALSE;
  I64 reduced=0;
  CDoc *doc=DocRead(filename,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR);
  CDocEntry *doc_e=doc->head.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT) {
      dst=MStrUtil(doc_e->tag,suf_flags,indent_scale_factor);
      if (StrCmp(dst,doc_e->tag)) {
	reduced+=StrLen(doc_e->tag)-StrLen(dst);
	chged=TRUE;
	Free(doc_e->tag);
	doc_e->tag=dst;
      } else
	Free(dst);
    }
    doc_e=doc_e->next;
  }
  if (chged) {
    "Reduced %s by %d chars\n",filename,reduced;
    DocWrite(doc);
  }
  DocDel(doc);
}
public U0 SU(U8 *files_find_mask,I64 suf_flags,U8 *fu_flags=NULL,
	F64 indent_scale_factor=0)
{//Apply $LK,"StrUtil",A="MN:StrUtil"$() on files.
//You can convert spaces to tabs, for example,
  //or removing trailing spaces on lines.
  //See $LK,"SUF Flags",A="MN:SUF_SINGLE_SPACE"$.
  I64 fuf_flags=0;
  CDirEntry *tmpde,*tmpde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+f+F+T");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tmpde=tmpde1=FilesFind(files_find_mask,fuf_flags);
  while (tmpde) {
    SUFile(tmpde->full_name,suf_flags,indent_scale_factor);
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
}

public U0 S2T(U8 *files_find_mask,U8 *fu_flags=NULL)
{//Spaces to tabs.
//Use "Hard Space" (SHIFT-SPACE) for spaces
  //in string consts in your code.
  SU(files_find_mask,SUF_S2T|SUF_REM_TRAILING,fu_flags);
}
