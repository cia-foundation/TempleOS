#help_index "Utils;Cmd Line (Typically)"

CDirEntry MCMatchFind(CDirEntry *pattern,CDirEntry *lst)
{
  while (lst) {
    if (!StrCmp(pattern->name,lst->name))
      return lst;
    lst=lst->next;
  }
  return NULL;
}

U0 MergeChkLst(CDirEntry *m1,CDirEntry *m2,I64 *_fuf_flags,I64 *_df_flags)
{
  I64 i;
  CDirEntry *tempm2;
  while (m1 && !(*_df_flags & DF_ABORT_ALL_FILES)) {
    tempm2=MCMatchFind(m1,m2);
    if (!tempm2)
      "$$FG,BROWN$$Does Not Exist:%s$$FG$$\n",m1->full_name;
    else {
      if (m1->attr & _ATTR_DIR)
	MergeChkLst(m1->sub,tempm2->sub,_fuf_flags,_df_flags);
      else {
	i=m1->datetime-tempm2->datetime;
	if (AbsI64(i)>CDATE_FREQ*2) { //Filedates can shift due to conversion
	  "" m1->full_name;
	  NewLine;
	  if (Bt(_fuf_flags,FUf_DIFF)) {
	    if (FilesFindMatch(FILENAME_MASK_TXT,m1->full_name))
	      Diff(tempm2->full_name,m1->full_name,_df_flags);
	  }
	}
      }
    }
    m1=m1->next;
  }
}

public U0 MergeChk(
  U8 *dst_files_find_mask="/TempleOS/*",
  U8 *src_files_find_mask="/TempleOS/*",
  U8 *fu_flags=NULL)
//"+d"=Do $LK,"Diff","MN:Diff"$() on files
{
  I64 df_flags=0,fuf_flags=0;
  CDirEntry *tempm1=NULL,*tempm2=NULL;
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (fuf_flags&~(FUG_FILES_FIND|FUF_DIFF))
    throw(EXCEPT_FUF,3);
  PrintFWarn("This is based strictly on file dates.\n");
  tempm1=FilesFind(src_files_find_mask,fuf_flags&FUG_FILES_FIND);
  tempm2=FilesFind(dst_files_find_mask,fuf_flags&FUG_FILES_FIND);
  fuf_flags&=FUF_DIFF;
  MergeChkLst(tempm1,tempm2,&fuf_flags,&df_flags);
  CDirLstDel(tempm1);
  CDirLstDel(tempm2);
}

#help_index "Utils;Cmd Line (Typically)"

U0 LstNewFilesLst(CDirEntry *tempm,CDate cdt)
{
  while (tempm) {
    if (tempm->datetime>cdt) {
      "" tempm->full_name;
      NewLine;
    }
    LstNewFilesLst(tempm->sub,cdt);
    tempm=tempm->next;
  }
}

public U0 LstNewFiles(U8 *files_find_mask="/TempleOS/*",CDate cdt=0,
  U8 *fu_flags=NULL)
//"+T"=is just text files
{
  I64 fuf_flags=0;
  CDirEntry *tempm1=NULL;
  if (!cdt)
    cdt=sys_compile_time;
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+T");
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (tempm1=FilesFind(files_find_mask,fuf_flags)) {
    LstNewFilesLst(tempm1,cdt);
    CDirLstDel(tempm1);
  }
}

#help_index ""
