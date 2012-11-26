#help_index "Utils;Cmd Line (Typically)"

CDirEntry MCMatchFind(CDirEntry *pattern,CDirEntry *list)
{
  while (list) {
    if (!StrCmp(pattern->name,list->name))
      return list;
    list=list->next;
  }
  return NULL;
}

U0 MergeChkList(CDirEntry *m1,CDirEntry *m2,I64 *_fuf_flags,I64 *df_flags)
{
  I64 i;
  CDirEntry *tempm2;
  while (m1 && !(*df_flags & DF_ABORT_ALL_FILES)) {
    tempm2=MCMatchFind(m1,m2);
    if (!tempm2)
      "$$FG,BROWN$$Does Not Exist:%s$$FG$$\n",m1->full_name;
    else {
      if (m1->attr & _ATTR_DIR)
	MergeChkList(m1->sub,tempm2->sub,_fuf_flags,df_flags);
      else {
	i=m1->datetime-tempm2->datetime;
	if (AbsI64(i)>CDATE_FREQ*2) { //Filedates can shift due to conversion
	  "" m1->full_name;
	  NewLine;
	  if (Bt(_fuf_flags,FUf_DIFF)) {
	    if (IsTextFile(m1->full_name))
	      *df_flags=Diff(tempm2->full_name,m1->full_name,*df_flags);
	  }
	}
      }
    }
    m1=m1->next;
  }
}

public U0 MergeChk(
  U8 *dst_files_find_mask="/SparrowOS/*",
  U8 *src_files_find_mask="/SparrowOS/*",
  U8 *fu_flags=NULL)
//"+d"=Do $LK,"Diff","MN:Diff"$() on files
{
  I64 df_flags=0,fuf_flags=0;
  CDirEntry *tempm1=NULL,*tempm2=NULL;
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  PrintFWarn("This is based strictly on file dates.\n");
  tempm1=FilesFind(src_files_find_mask,fuf_flags);
  tempm2=FilesFind(dst_files_find_mask,fuf_flags);
  MergeChkList(tempm1,tempm2,&fuf_flags,&df_flags);
  CDirListDel(tempm1);
  CDirListDel(tempm2);
}

#help_index "Utils;Cmd Line (Typically)"

U0 ListNewFilesList(CDirEntry *tempm,CDate ltdt)
{
  while (tempm) {
    if (tempm->datetime>ltdt) {
      "" tempm->full_name;
      NewLine;
    }
    ListNewFilesList(tempm->sub,ltdt);
    tempm=tempm->next;
  }
}

public U0 ListNewFiles(U8 *files_find_mask="/SparrowOS/*",CDate ltdt=0,
  U8 *fu_flags=NULL)
//"+t"=is just text files
{
  I64 fuf_flags=0;
  CDirEntry *tempm1=NULL;
  if (!ltdt)
    ltdt=sys_compile_time;
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+t");
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (tempm1=FilesFind(files_find_mask,fuf_flags)) {
    ListNewFilesList(tempm1,ltdt);
    CDirListDel(tempm1);
  }
}

#help_index ""
