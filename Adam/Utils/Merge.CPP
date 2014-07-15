#help_index "Cmd Line (Typically)"

CDirEntry MGFind(CDirEntry *needle_entry,CDirEntry *haystack_lst)
{
  while (haystack_lst) {
    if (!StrCmp(needle_entry->name,haystack_lst->name))
      return haystack_lst;
    haystack_lst=haystack_lst->next;
  }
  return NULL;
}

U0 Merge2(CDirEntry *tempde1,CDirEntry *tempde2,I64 *_fuf_flags,I64 *_df_flags)
{
  CDirEntry *tempde;
  while (tempde1 && !(*_df_flags & DF_ABORT_ALL_FILES)) {
    tempde=MGFind(tempde1,tempde2);
    if (!tempde)
      "$$BROWN$$Does Not Exist:%s$$FG$$\n",tempde1->full_name;
    else {
      if (tempde1->attr & RS_ATTR_DIR)
	Merge2(tempde1->sub,tempde->sub,_fuf_flags,_df_flags);
      else {
	if (AbsI64(tempde1->datetime-tempde->datetime)>CDATE_FREQ*2) {//Filedates can shift due to conversion
	  "%s",tempde1->full_name;
	  '\n';
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
public U0 Merge(U8 *dst_files_find_mask="/*",U8 *src_files_find_mask="/*",
  U8 *fu_flags=NULL)
{/*Merge text files from one list into another.
Report files whos date/time is different.

"+d" will do $LK,"Diff",A="MN:Diff"$() on files and prompt to
merge changes from source files into
destination files.
*/
  I64 df_flags=0,fuf_flags=0;
  CDirEntry *tempde1=NULL,*tempde2=NULL;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (fuf_flags&~(FUG_FILES_FIND|FUF_DIFF))
    throw('FUF');
  PrintWarn("This is based strictly on file dates.\n");
  tempde1=FilesFind(src_files_find_mask,fuf_flags&FUG_FILES_FIND);
  tempde2=FilesFind(dst_files_find_mask,fuf_flags&FUG_FILES_FIND);
  fuf_flags&=FUF_DIFF;
  Merge2(tempde1,tempde2,&fuf_flags,&df_flags);
  DirTreeDel(tempde1);
  DirTreeDel(tempde2);
}

#help_index "Utils;Cmd Line (Typically)"

public U0 NewFiles(U8 *files_find_mask="/*",CDate cdt=0,U8 *fu_flags=NULL)
{//Report files newer than a certain date.
  I64 fuf_flags=0;
  CDirEntry *tempde,*tempde1;
  if (!cdt) cdt=sys_compile_time;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+F");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    if (tempde->datetime>cdt)
      "%s\n",tempde->full_name;
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
}
