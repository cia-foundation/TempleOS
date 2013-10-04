public U0 Dollar(U8 *files_find_mask,U8 *fu_flags=NULL)
//This is for importing non-"TempleOS" text files.
//$BK,1$WARNING:$BK,0$Do not use on files with dollar cmds
// or on binary files. See $LK,"FilesFind","MN:FilesFind"$() for flags.
{
  CDoc *doc;
  I64 fuf_flags=0;
  CDirEntry *tempde,*tempde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    "%s\n",tempde->full_name;
    doc=DocRead(tempde->full_name,DOCF_PLAIN_TEXT|DOCF_DBL_DOLLARS);
    DocWrite(doc);
    DocDel(doc);
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
}
