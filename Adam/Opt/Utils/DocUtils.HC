#help_index "DolDoc/Cmd Line (Typically)"

U0 CursorRemFile(U8 *filename)
{//Rem ASCII 5 cursor from one file.
  CDoc *doc=DocRead(filename,DOCF_PLAIN_TEXT|DOCF_NO_CURSOR);
  DocWrite(doc);
  DocDel(doc);
}
public U0 CursorRem(U8 *files_find_mask="*")
{//Rem ASCII 5 cursor.
  I64 fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+T+f+F+O");
  CDirEntry *tmpde=FilesFind(files_find_mask,fuf_flags),*tmpde1=tmpde;
  while (tmpde) {
    CursorRemFile(tmpde->full_name);
    Touch(tmpde->full_name,"",,tmpde->datetime);
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
}

U0 CollapseFile(U8 *filename,Bool collapse=TRUE)
{//Collapse $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$ trees in one file.
  CDoc *doc=DocRead(filename,DOCF_NO_CURSOR);
  DocCollapse(collapse,doc);
  DocWrite(doc);
  DocDel(doc);
}
public U0 Collapse(U8 *files_find_mask="*",
	Bool collapse=TRUE,U8 *fu_flags=NULL)
{//Collapse $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$ trees.
  I64 fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F+$$+O");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  CDirEntry *tmpde=FilesFind(files_find_mask,fuf_flags),*tmpde1=tmpde;
  while (tmpde && !Bt(&fuf_flags,FUf_CANCEL)) {
    CollapseFile(tmpde->full_name,collapse);
    Touch(tmpde->full_name,"",,tmpde->datetime);
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
}

I64 DocOptEntry(CDoc *,CDocEntry *doc_e,I64 fuf_flags)
{
  U8 *st,*st2,**_dst;
  I64 i,res=0;

  if (doc_e->de_flags&DOCEF_LINK && doc_e->de_flags&(DOCEF_AUX_STR|DOCEF_TAG)) {
    if (doc_e->de_flags & DOCEF_AUX_STR && doc_e->de_flags & DOCEF_TAG &&
	  !MemCmp(doc_e->aux_str,"FI:",3) &&
	  !StrCmp(doc_e->aux_str+3,doc_e->tag)) {
      Free(doc_e->aux_str);
      doc_e->aux_str=NULL;
      doc_e->de_flags&=~DOCEF_AUX_STR;
      res++;
    }
    if (doc_e->de_flags & DOCEF_AUX_STR)
      _dst=&doc_e->aux_str;
    else
      _dst=&doc_e->tag;
    if (StrMatch(".Z",*_dst)) {
      st=DocLinkFile(*_dst);
      if (FileFind(st)) {
	if (IsDotZ(st))
	  st[StrLen(st)-2]=0;
	i=StrLen(st);
	if ((st2=StrMatch(st,*_dst)) && st2[i]=='.' && st2[i+1]=='Z') {
	  StrCpy(st2+i,st2+i+2);
	  res++;
	}
      }
      Free(st);
    }
    if (fuf_flags&FUF_RISKY) {
      if (doc_e->de_flags & DOCEF_AUX_STR) {
	if (st=StrMatch(".Z",doc_e->aux_str)) {
	  StrCpy(st,st+2);
	  res++;
	}
      }
      if (doc_e->de_flags&DOCEF_TAG) {
	if (st=StrMatch(".Z",doc_e->tag)) {
	  StrCpy(st,st+2);
	  res++;
	}
      }
    }
  }
  return res;
}
I64 DocOptDoc(CDoc *doc,I64 fuf_flags)
{//Optimize Doc.
  Bool unlock=DocLock(doc);
  I64 res=0;
  CDocEntry *doc_e,*doc_e1,*doc_e_last;
  DocRecalc(doc);
  doc_e_last=NULL;
  doc_e=doc->head.next;
  while (doc_e!=doc) {
    doc_e1=doc_e->next;
    if (res+=DocOptEntry(doc,doc_e,fuf_flags))
      res+=DocOptEntry(doc,doc_e,fuf_flags);
    if (doc_e_last && doc_e_last->type&~0xFF00==doc_e->type&~0xFF00 &&
       doc_e_last->de_flags==doc_e->de_flags &&
       Bt(doldoc.type_flags_chk_dup,doc_e->type_u8) &&
       doc_e_last->attr==doc_e->attr) {
      DocEntryDel(doc,doc_e);
      res++;
    } else
      doc_e_last=doc_e;
    doc_e=doc_e1;
  }
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
  return res;
}
I64 DocOptFile(U8 *filename,I64 fuf_flags)
{//Optimize file.
  I64 res;
  CDoc *doc=DocRead(filename);
  if (res=DocOptDoc(doc,fuf_flags)) {
    "-%d:%s\n",res,doc->filename.name;
    DocWrite(doc);
  }
  DocDel(doc);
  return res;
}
I64 DocOptLst(CDirEntry *tmpde,I64 fuf_flags)
{
  I64 res=0;
  CDirEntry *tmpde1;
  while (tmpde) {
    tmpde1=tmpde->next;
    if (tmpde->attr & RS_ATTR_DIR) {
      if (tmpde->sub) {
	"Scanning Directory: %s\n",tmpde->full_name;
	res+=DocOptLst(tmpde->sub,fuf_flags);
      }
    } else
      res+=DocOptFile(tmpde->full_name,fuf_flags);
    DirEntryDel(tmpde);
    tmpde=tmpde1;
  }
  return res;
}
public I64 DocOpt(U8 *files_find_mask="*",U8 *fu_flags=NULL)
{//Optimize $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$ files, eliminating aux_str's and .Z's.
//+R flag for aggressively risky.
  I64 fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+$$");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  return DocOptLst(FilesFind(files_find_mask,fuf_flags&FUG_FILES_FIND),
	fuf_flags&~FUG_FILES_FIND);
}
