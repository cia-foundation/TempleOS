U0 MyUtil(U8 *files_find_mask,U8 *fu_flags=NULL)
{
  I64 fuf_flags=0;
  CDirEntry *tmpde,*tmpde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tmpde=tmpde1=FilesFind(files_find_mask,fuf_flags);
  while (tmpde) {
    PutFileLink(tmpde->full_name);
    '\n';
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
}
