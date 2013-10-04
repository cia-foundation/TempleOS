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

U0 MergeChkLst(CDirEntry *tempde1,CDirEntry *tempde2,I64 *_fuf_flags,I64 *_df_flags)
{
  I64 i;
  CDirEntry *tempde;
  while (tempde1 && !(*_df_flags & DF_ABORT_ALL_FILES)) {
    tempde=MCMatchFind(tempde1,tempde2);
    if (!tempde)
      "$$FG,BROWN$$Does Not Exist:%s$$FG$$\n",tempde1->full_name;
    else {
      if (tempde1->attr & _ATTR_DIR)
	MergeChkLst(tempde1->sub,tempde->sub,_fuf_flags,_df_flags);
      else {
	i=tempde1->datetime-tempde->datetime;
	if (AbsI64(i)>CDATE_FREQ*2) { //Filedates can shift due to conversion
	  "%s",tempde1->full_name;
	  NewLine;
	  if (Bt(_fuf_flags,FUf_DIFF)) {
	    if (FilesFindMatch(tempde1->full_name,FILEMASK_TXT))
	      Diff(tempde->full_name,tempde1->full_name,_df_flags);
	  }
	}
      }
    }
    tempde1=tempde1->next;
  }
}

public U0 MergeChk(
  U8 *dst_files_find_mask="/TempleOS/*",
  U8 *src_files_find_mask="/TempleOS/*",
  U8 *fu_flags=NULL)
//"+d"=Do $LK,"Diff","MN:Diff"$() on files
{
  I64 df_flags=0,fuf_flags=0;
  CDirEntry *tempde1=NULL,*tempde2=NULL;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (fuf_flags&~(FUG_FILES_FIND|FUF_DIFF))
    throw(EXCEPT_FUF,3);
  PrintWarn("This is based strictly on file dates.\n");
  tempde1=FilesFind(src_files_find_mask,fuf_flags&FUG_FILES_FIND);
  tempde2=FilesFind(dst_files_find_mask,fuf_flags&FUG_FILES_FIND);
  fuf_flags&=FUF_DIFF;
  MergeChkLst(tempde1,tempde2,&fuf_flags,&df_flags);
  DirLstDel(tempde1);
  DirLstDel(tempde2);
}

#help_index "Utils;Cmd Line (Typically)"

public U0 NewFiles(U8 *files_find_mask="/TempleOS/*",CDate cdt=0,U8 *fu_flags=NULL)
{//List new files
  I64 fuf_flags=0;
  CDirEntry *tempde,*tempde1;
  if (!cdt) cdt=sys_compile_time;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    if (tempde->datetime>cdt)
      "%s\n",tempde->full_name;
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
}

#help_index ""
