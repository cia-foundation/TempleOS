#help_index "Debugging;Utils;Cmd Line (Typically)"

I64 LinkChkDoc(CDoc *doc,I64 *_fuf_flags)
{
  U8 *st;
  Bool found,unlock=DocLock(doc);
  CDoc *old_doc=DocCur;
  CDocEntry *doc_e=doc->root.next;
  I64 result=0;
  while (doc_e!=doc) {
    if (ScanKey) {
      LBts(_fuf_flags,FUf_CANCEL);
      break;
    }
    if (doc_e->type_u8==DOCT_LINK) {
      if (doc_e->flags1 & DOCEF1_AUX_STR)
	st=doc_e->aux_str;
      else
	st=doc_e->tag;
      DocLock(old_doc);
      DocQueRem(old_doc);
      DocQueIns(doc,Fs->last_doc);
      found=DocLinkChk(st);
      DocQueRem(doc);
      DocQueIns(old_doc,Fs->last_doc);
      DocUnlock(old_doc);
      if (!found) {
	PutFileLink(doc->filename.name,doc_e->y);
	"$$FG,RED$$ %04d:%s\n$$FG$$",doc_e->y,st;
	result++;
      }
    } else if (doc_e->type_u8==DOCT_ERROR) {
      PutFileLink(doc->filename.name,doc_e->y);
      "$$FG,RED$$ Doc Error\n$$FG$$";
      result++;
    }
    doc_e=doc_e->next;
  }
  if (unlock) DocUnlock(doc);
  return result;
}

I64 LinkChkFile(U8 *filename,I64 *_fuf_flags)
{
  CDoc *doc=DocRead(filename,DOCF_NO_CURSOR);
  I64 result=LinkChkDoc(doc,_fuf_flags);
  DocDel(doc);
  return result;
}

I64 LinkChkList(CDirEntry *tempm1,I64 *_fuf_flags)
{
  CDirEntry *tempm2;
  I64 result=0;
  while (tempm1 && !Bt(_fuf_flags,FUf_CANCEL)) {
    tempm2=tempm1->next;
    if (tempm1->attr & _ATTR_DIR) {
      if (tempm1->sub) {
	"%S %s\n","ST_SCANNING_DIR",tempm1->full_name;
	result+=LinkChkList(tempm1->sub,_fuf_flags);
      }
    } else
      result+=LinkChkFile(tempm1->full_name,_fuf_flags);
    CDirEntryDel(tempm1);
    tempm1=tempm2;
  }
  return result;
}

public I64 LinkChk(U8 *files_find_mask=TEXT_FILE_MASK,U8 *fu_flags=NULL)
//Checks for broken file links.
{
  I64 fuf_flags=0;
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+t");
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  return LinkChkList(FilesFind(files_find_mask,fuf_flags),&fuf_flags);
}
#help_index ""
