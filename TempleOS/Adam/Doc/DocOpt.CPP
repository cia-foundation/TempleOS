#help_index "Doc"

I64 DocOptEntry(CDoc *,CDocEntry *doc_e)
{
  U8 *st,*st2,**_dst;
  I64 i,result=0;

  if (doc_e->flags1&DOCEF1_LINK) {
    if (doc_e->flags1 & DOCEF1_AUX_STR &&
	Bt(doc_tag_types,doc_e->type_u8) &&
	!MemCmp(doc_e->aux_str,"FI:",3) &&
	!StrCmp(doc_e->aux_str+3,doc_e->tag)) {
      Free(doc_e->aux_str);
      doc_e->aux_str=NULL;
      doc_e->flags1&=~DOCEF1_AUX_STR;
      result++;
    }
    if (doc_e->flags1 & DOCEF1_AUX_STR)
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
    if (doc_e->flags1 & DOCEF1_AUX_STR) {
      if (st=StrMatch(".Z",doc_e->aux_str)) {
	StrCpy(st,st+2);
	result++;
      }
    }
    if (Bt(doc_tag_types,doc_e->type_u8)) {
      if (st=StrMatch(".Z",doc_e->tag)) {
	StrCpy(st,st+2);
	result++;
      }
    }
*/
  }
  if (doc_e->type_u8==DOCT_MACRO && doc_e->flags1&DOCEF1_AUX_STR &&
      doc_e->flags1&DOCEF1_LEFT_MACRO && !StrCmp(doc_e->tag,doc_e->left_macro) ||
      doc_e->flags1&DOCEF1_RIGHT_MACRO && !StrCmp(doc_e->tag,doc_e->right_macro)) {
    doc_e->flags1&=~DOCEF1_AUX_STR;
    Free(doc_e->aux_str);
    doc_e->aux_str=NULL;
    result++;
  }
  return result;
}

public I64 DocOptDoc(CDoc *doc)
{//Optimize Doc
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

public I64 DocOptFile(U8 *filename)
{//Optimize File
  I64 result;
  CDoc *doc=DocRead(filename);
  if (result=DocOptDoc(doc)) {
    "-%d:%s\n",result,doc->filename.name;
    DocWrite(doc);
  }
  DocDel(doc);
  return result;
}

I64 DocOptLst(CDirEntry *tempde1)
{
  I64 result=0;
  CDirEntry *tempde2;
  while (tempde1) {
    tempde2=tempde1->next;
    if (tempde1->attr & _ATTR_DIR) {
      if (tempde1->sub) {
	"%S %s\n","ST_SCANNING_DIR",tempde1->full_name;
	result+=DocOptLst(tempde1->sub);
      }
    } else
      result+=DocOptFile(tempde1->full_name);
    DirEntryDel(tempde1);
    tempde1=tempde2;
  }
  return result;
}

public I64 DocOpt(U8 *files_find_mask="*",U8 *fu_flags=NULL)
{//Optimize Files
  I64 f=0;
  ScanFlags(&f,Define("ST_FILE_UTIL_FLAGS"),"+r+T");
  ScanFlags(&f,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  return DocOptLst(FilesFind(files_find_mask,f));
}
