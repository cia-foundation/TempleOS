U0 MyUtil(U8 *files_find_mask,U8 *fu_flags=NULL)
{
  I64 fuf_flags=0;
  CDirEntry *tempde,*tempde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    PutFileLink(tempde->full_name);
    '\n';
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
}
