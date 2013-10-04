#help_index "Utils;Cmd Line (Typically)"

I64 LineCntRepFiles(CDirEntry *tempde)
{
  CDoc *doc;
  I64 result=0,i;
  while (tempde) {
    i=0;
    if (tempde->attr&_ATTR_DIR)
      i=LineCntRepFiles(tempde->sub);
    else {
      doc=DocRead(tempde->full_name);
      if (doc->root.next!=doc)
	i=doc->root.last->y+2;
      DocDel(doc);
    }
    tempde->user_data=i;
    result+=i;
    tempde=tempde->next;
  }
  return result;
}

U0 PutLineCntFiles(CDoc *doc,CDirEntry *tempde)
{
  while (tempde) {
    if (tempde->attr&_ATTR_DIR) {
      DocPrint(doc,"%8,d $$TR+C,\"%s\"$$\n$$ID,2$$",tempde->user_data,tempde->name);
      PutLineCntFiles(doc,tempde->sub);
      DocPrint(doc,"$$ID,-2$$");
    } else
      DocPrint(doc,"%8,d $$LK,\"%s\",\"FI:%s\"$$\n",
	tempde->user_data,tempde->name,tempde->full_name);
    tempde=tempde->next;
  }
}

public I64 LineCntRep(U8 *files_find_mask="/TempleOS/*",U8 *fu_flags=NULL)
{
  I64 fuf_flags=0,result=0;
  CDoc *doc=DocNew;
  CDirEntry *tempde1=NULL;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+S");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (tempde1=FilesFind(files_find_mask,fuf_flags)) {
    result=LineCntRepFiles(tempde1);
    DocPrint(doc,"%8,d $$TR+C,\"%s\"$$\n$$ID,2$$",result,files_find_mask);
    PutLineCntFiles(doc,tempde1);
    DocPrint(doc,"$$ID,-2$$");
    DirLstDel(tempde1);
  }
  DocInsDoc(DocPut,doc);
  DocDel(doc);
  return result;
}

I64 TreeSizeRepFiles(CDirEntry *tempde)
{
  I64 result=0,i;
  while (tempde) {
    if (tempde->attr&_ATTR_DIR)
      i=TreeSizeRepFiles(tempde->sub);
    else
      i=tempde->size;
    tempde->user_data=i;
    result+=i;
    tempde=tempde->next;
  }
  return result;
}

U0 PutTreeSizeFiles(CDoc *doc,CDirEntry *tempde)
{
  while (tempde) {
    if (tempde->attr&_ATTR_DIR) {
      DocPrint(doc,"%10,d $$TR+C,\"%s\"$$\n$$ID,2$$",tempde->user_data,tempde->name);
      PutTreeSizeFiles(doc,tempde->sub);
      DocPrint(doc,"$$ID,-2$$");
    } else {
      DocPrint(doc,"%10,d ",tempde->user_data);
      DocPrint(doc,"$$LK,\"%s\",\"FI:%s\"$$\n",
	  tempde->name,tempde->full_name);
    }
    tempde=tempde->next;
  }
}

public I64 TreeSizeRep(U8 *files_find_mask="/TempleOS/*",U8 *fu_flags=NULL)
{
  I64 fuf_flags=0,result=0;
  CDoc *doc=DocNew;
  CDirEntry *tempde1=NULL;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (tempde1=FilesFind(files_find_mask,fuf_flags)) {
    result=TreeSizeRepFiles(tempde1);
    DocPrint(doc,"%10,d $$TR+C,\"%s\"$$\n$$ID,2$$",result,files_find_mask);
    PutTreeSizeFiles(doc,tempde1);
    DocPrint(doc,"$$ID,-2$$");
    DirLstDel(tempde1);
  }
  DocInsDoc(DocPut,doc);
  DocDel(doc);
  return result;
}

#help_index ""
