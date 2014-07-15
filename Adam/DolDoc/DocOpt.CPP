#help_index "DolDoc"

I64 DocOptEntry(CDoc *,CDocEntry *doc_e)
{
  U8 *st,*st2,**_dst;
  I64 i,result=0;

  if (doc_e->de_flags&DOCEF_LINK && doc_e->de_flags&(DOCEF_AUX_STR|DOCEF_TAG)) {
    if (doc_e->de_flags & DOCEF_AUX_STR && doc_e->de_flags & DOCEF_TAG &&
	!MemCmp(doc_e->aux_str,"FI:",3) && !StrCmp(doc_e->aux_str+3,doc_e->tag)) {
      Free(doc_e->aux_str);
      doc_e->aux_str=NULL;
      doc_e->de_flags&=~DOCEF_AUX_STR;
      result++;
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
	  result++;
	}
      }
      Free(st);
    }
/* !!! Dangerous, but handy !!!
    if (doc_e->de_flags & DOCEF_AUX_STR) {
      if (st=StrMatch(".Z",doc_e->aux_str)) {
	StrCpy(st,st+2);
	result++;
      }
    }
    if (doc_e->de_flags&DOCEF_TAG) {
      if (st=StrMatch(".Z",doc_e->tag)) {
	StrCpy(st,st+2);
	result++;
      }
    }
*/
  }
  return result;
}

I64 DocOptDoc(CDoc *doc)
{//Optimize Doc.
  Bool unlock=DocLock(doc);
  I64 result=0;
  CDocEntry *doc_e;
  DocRecalc(doc);
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (result+=DocOptEntry(doc,doc_e))
      result+=DocOptEntry(doc,doc_e);
    doc_e=doc_e->next;
  }
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
  return result;
}

I64 DocOptFile(U8 *filename)
{//Optimize File.
  I64 result;
  CDoc *doc=DocRead(filename);
  if (result=DocOptDoc(doc)) {
    "-%d:%s\n",result,doc->filename.name;
    DocWrite(doc);
  }
  DocDel(doc);
  return result;
}
I64 DocOptLst(CDirEntry *tempde)
{
  I64 result=0;
  CDirEntry *tempde1;
  while (tempde) {
    tempde1=tempde->next;
    if (tempde->attr & RS_ATTR_DIR) {
      if (tempde->sub) {
	"Scanning Directory: %s\n",tempde->full_name;
	result+=DocOptLst(tempde->sub);
      }
    } else
      result+=DocOptFile(tempde->full_name);
    DirEntryDel(tempde);
    tempde=tempde1;
  }
  return result;
}
public I64 DocOpt(U8 *files_find_mask="*",U8 *fu_flags=NULL)
{//Optimize $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.TXT"$ Files, eliminating aux_str's and .Z's.
  I64 f=0;
  ScanFlags(&f,Define("ST_FILE_UTIL_FLAGS"),"+r+T");
  ScanFlags(&f,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  return DocOptLst(FilesFind(files_find_mask,f));
}
