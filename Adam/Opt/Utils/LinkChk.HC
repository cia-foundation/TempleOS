#help_index "Debugging;Cmd Line (Typically)"

I64 LinkChkDoc(CDoc *doc,I64 *_fuf_flags)
{
  U8 *st;
  Bool found,unlock=DocLock(doc);
  CDoc *old_doc;
  CDocEntry *doc_e=doc->head.next;
  I64 res=0;
  while (doc_e!=doc) {
    if (ScanKey) {
      LBts(_fuf_flags,FUf_CANCEL);
      break;
    }
    if (st=DocEntryLink(doc,doc_e)) {
      old_doc=DocPut;
      DocLock(old_doc);
      Fs->put_doc=doc;
      found=DocLinkChk(doc,st);
      Fs->put_doc=old_doc;
      DocUnlock(old_doc);
      if (!found) {
	PutFileLink(doc->filename.name,,doc_e->y);
	"$$RED$$ %04d:%s\n$$FG$$",doc_e->y,st;
	res++;
      }
      Free(st);
    } else if (doc_e->type_u8==DOCT_ERROR) {
      PutFileLink(doc->filename.name,,doc_e->y);
      "$$RED$$ Doc Error\n$$FG$$";
      res++;
    }
    doc_e=doc_e->next;
  }
  if (unlock) DocUnlock(doc);
  return res;
}

I64 LinkChkFile(U8 *filename,I64 *_fuf_flags)
{
  I64 res;
  CDoc *doc=DocRead(filename,DOCF_NO_CURSOR);
  res=LinkChkDoc(doc,_fuf_flags);
  DocDel(doc);
  return res;
}
public I64 LinkChk(U8 *files_find_mask="/*",U8 *fu_flags=NULL)
{//Check documents for broken file links.
  I64 fuf_flags=0,res=0;
  CDirEntry *tmpde,*tmpde1;
  U8 *st;
  progress2_max=1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+$$+f+F+O");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tmpde=tmpde1=FilesFind(files_find_mask,fuf_flags);
  while (tmpde && !Bt(&fuf_flags,FUf_CANCEL)) {

    st=MStrPrint("Before %s Broke:%d",tmpde->full_name,res);
    if (StrLen(st)>PROGRESS_DESC_LEN-1)
      st[PROGRESS_DESC_LEN-1]=0;
    StrCpy(progress2_desc,st);
    Free(st);
    Yield;

    res+=LinkChkFile(tmpde->full_name,&fuf_flags);

    st=MStrPrint("After %s Broke:%d",tmpde->full_name,res);
    if (StrLen(st)>PROGRESS_DESC_LEN-1)
      st[PROGRESS_DESC_LEN-1]=0;
    StrCpy(progress2_desc,st);
    Free(st);
    Yield;

    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
  progress2_max=0;
  return res;
}
