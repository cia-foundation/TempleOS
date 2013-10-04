#help_index "Utils;Cmd Line (Typically)"

//$LK,"StrUtil","MN:StrUtil"$() operations on files

U0 SUFile(U8 *name,I64 suf_flags,F64 indent_scale_factor)
{//String utility on a single file
//See $LK,"SU Flags","MN:SUF_SINGLE_SPACE"$
  U8 *dst;
  Bool chged=FALSE;
  I64 reduced=0;
  CDoc *doc=DocRead(name,DOCF_PLAIN_TEXT_WITH_TABS | DOCF_NO_CURSOR);
  CDocEntry *doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT) {
      dst=MStrUtil(doc_e->tag,suf_flags,indent_scale_factor);
      if (StrCmp(dst,doc_e->tag)) {
	reduced+=StrLen(doc_e->tag)-StrLen(dst);
	chged=TRUE;
	Free(doc_e->tag);
	doc_e->tag=dst;
      } else
	Free(dst);
    }
    doc_e=doc_e->next;
  }
  if (chged) {
    "Reduced %s by %d chars\n",name,reduced;
    StrCpy(doc->filename.name,name);
    DocWrite(doc);
  }
  DocDel(doc);
}

public U0 SU(U8 *files_find_mask,I64 suf_flags,U8 *fu_flags=NULL,F64 indent_scale_factor=0)
{//Apply string utilities to files
//You can convert spaces to tabs, for example
//or removing trailing spaces on lines.
//See $LK,"SUF Flags","MN:SUF_SINGLE_SPACE"$
  I64 fuf_flags=0;
  CDirEntry *tempde,*tempde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+f+F+T");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    SUFile(tempde->full_name,suf_flags,indent_scale_factor);
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
}

public U0 S2T(U8 *files_find_mask,U8 *fu_flags=NULL)
{
//Spaces to tabs.
//Use "Hard Space" (SHIFT-SPACE) for spaces
//in string consts in your code.
  SU(files_find_mask,SUF_S2T|SUF_REM_TRAILING,fu_flags);
}

#help_index ""
