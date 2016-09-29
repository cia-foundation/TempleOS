#help_index "Cmd Line (Typically);DolDoc/Conversion;DolDoc/Cmd Line (Typically)"

public U0 ToDolDoc(U8 *files_find_mask,U8 *fu_flags=NULL)
{//Convert text file to $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$ by making double $$'s.
  CDoc *doc;
  I64 fuf_flags=0;
  CDirEntry *tmpde,*tmpde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tmpde=tmpde1=FilesFind(files_find_mask,fuf_flags);
  while (tmpde) {
    "%s\n",tmpde->full_name;
    doc=DocRead(tmpde->full_name,DOCF_PLAIN_TEXT|DOCF_DBL_DOLLARS);
    DocWrite(doc);
    DocDel(doc);
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
}
