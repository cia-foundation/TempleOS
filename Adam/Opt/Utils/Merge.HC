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

U0 Merge2(CDirEntry *tmpde1,CDirEntry *tmpde2,I64 *_fuf_flags,I64 *_df_flags)
{
  CDirEntry *tmpde;
  while (tmpde1 && !(*_df_flags & DF_ABORT_ALL_FILES)) {
    tmpde=MGFind(tmpde1,tmpde2);
    if (!tmpde)
      "$$BROWN$$Does Not Exist:%s$$FG$$\n",tmpde1->full_name;
    else {
      if (tmpde1->attr & RS_ATTR_DIR)
	Merge2(tmpde1->sub,tmpde->sub,_fuf_flags,_df_flags);
      else {
	if (AbsI64(tmpde1->datetime-tmpde->datetime)>CDATE_FREQ*2) {//slop
	  "%s",tmpde1->full_name;
	  '\n';
	  if (Bt(_fuf_flags,FUf_DIFF)) {
	    if (FilesFindMatch(tmpde1->full_name,FILEMASK_TXT))
	      Diff(tmpde->full_name,tmpde1->full_name,_df_flags);
	  }
	}
      }
    }
    tmpde1=tmpde1->next;
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
  CDirEntry *tmpde1=NULL,*tmpde2=NULL;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (fuf_flags&~(FUG_FILES_FIND|FUF_DIFF))
    throw('FUF');
  PrintWarn("This is based strictly on file dates.\n");
  tmpde1=FilesFind(src_files_find_mask,fuf_flags&FUG_FILES_FIND);
  tmpde2=FilesFind(dst_files_find_mask,fuf_flags&FUG_FILES_FIND);
  fuf_flags&=FUF_DIFF;
  Merge2(tmpde1,tmpde2,&fuf_flags,&df_flags);
  DirTreeDel(tmpde1);
  DirTreeDel(tmpde2);
}

#help_index "Utils;Cmd Line (Typically)"

public U0 NewFiles(U8 *files_find_mask="/*",CDate cdt=0,U8 *fu_flags=NULL)
{//Report files newer than a certain date.
  I64 fuf_flags=0;
  CDirEntry *tmpde,*tmpde1;
  if (!cdt) cdt=sys_compile_time;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+F");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tmpde=tmpde1=FilesFind(files_find_mask,fuf_flags);
  while (tmpde) {
    if (tmpde->datetime>cdt)
      "%s\n",tmpde->full_name;
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
}
