I64 MyUtilDoc(CDoc *doc,I64 fuf_flags)
{
  Bool unlock=DocLock(doc);
  I64 res=0;
  CDocEntry *doc_e;
  DocRecalc(doc);
  doc_e=doc->head.next;
  while (doc_e!=doc) {
    res++;
    doc_e=doc_e->next;
  }
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
  return res;
}

I64 MyUtilFile(U8 *filename,I64 fuf_flags)
{
  I64 res;
  CDoc *doc=DocRead(filename);
  if (res=MyUtilDoc(doc,fuf_flags)) {
    "%8d:%s\n",res,doc->filename.name;
    DocWrite(doc);
  }
  DocDel(doc);
  return res;
}
I64 MyUtilLst(CDirEntry *tmpde,I64 fuf_flags)
{
  I64 res=0;
  CDirEntry *tmpde1;
  while (tmpde) {
    tmpde1=tmpde->next;
    if (tmpde->attr & RS_ATTR_DIR) {
      if (tmpde->sub) {
	"Processing Directory: %s\n",tmpde->full_name;
	res+=MyUtilLst(tmpde->sub,fuf_flags);
      }
    } else
      res+=MyUtilFile(tmpde->full_name,fuf_flags);
    DirEntryDel(tmpde);
    tmpde=tmpde1;
  }
  return res;
}
I64 MyUtil(U8 *files_find_mask="*",U8 *fu_flags=NULL)
{
  I64 fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+$$");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  return MyUtilLst(FilesFind(files_find_mask,fuf_flags&FUG_FILES_FIND),
	fuf_flags&~FUG_FILES_FIND);
}
