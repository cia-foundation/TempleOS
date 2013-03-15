#help_index "Utils;Cmd Line (Typically)"

I64 LineCntRepFiles(CDirEntry *tempm)
{
  CDoc *doc;
  I64 result=0,i;
  while (tempm) {
    i=0;
    if (tempm->attr&_ATTR_DIR)
      i=LineCntRepFiles(tempm->sub);
    else {
      if (FilesFindMatch(FILENAME_MASK_SRC,tempm->name)) {
	doc=DocRead(tempm->full_name);
	if (doc->root.next!=doc)
	  i=doc->root.last->y+2;
	DocDel(doc);
      }
    }
    tempm->user_data=i;
    result+=i;
    tempm=tempm->next;
  }
  return result;
}

U0 PutLineCntFiles(CDoc *doc,CDirEntry *tempm)
{
  while (tempm) {
    if (tempm->attr&_ATTR_DIR) {
      DocPrintF(doc,"%8,d $$TR+C,\"%s\"$$\n$$ID,2$$",tempm->user_data,tempm->name);
      PutLineCntFiles(doc,tempm->sub);
      DocPutS(doc,"$$ID,-2$$");
    } else {
      if (FilesFindMatch(FILENAME_MASK_SRC,tempm->name)) {
	DocPrintF(doc,"%8,d ",tempm->user_data);
	DocPrintF(doc,"$$LK,\"%s\",\"FI:%s\"$$\n",
	  tempm->name,tempm->full_name);
      }
    }
    tempm=tempm->next;
  }
}

public I64 LineCntRep(U8 *files_find_mask="/TempleOS/*",U8 *fu_flags=NULL)
{
  I64 fuf_flags=0,result=0;
  CDoc *doc=DocNew;
  CDirEntry *tempm1=NULL;
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (tempm1=FilesFind(files_find_mask,fuf_flags)) {
    result=LineCntRepFiles(tempm1);
    DocPrintF(doc,"%8,d $$TR+C,\"%s\"$$\n$$ID,2$$",result,files_find_mask);
    PutLineCntFiles(doc,tempm1);
    DocPutS(doc,"$$ID,-2$$");
    CDirLstDel(tempm1);
  }
  DocInsDoc(DocCur,doc);
  DocDel(doc);
  return result;
}

I64 TreeSizeRepFiles(CDirEntry *tempm)
{
  I64 result=0,i;
  while (tempm) {
    if (tempm->attr&_ATTR_DIR)
      i=TreeSizeRepFiles(tempm->sub);
    else
      i=tempm->size;
    tempm->user_data=i;
    result+=i;
    tempm=tempm->next;
  }
  return result;
}

U0 PutTreeSizeFiles(CDoc *doc,CDirEntry *tempm)
{
  while (tempm) {
    if (tempm->attr&_ATTR_DIR) {
      DocPrintF(doc,"%10,d $$TR+C,\"%s\"$$\n$$ID,2$$",tempm->user_data,tempm->name);
      PutTreeSizeFiles(doc,tempm->sub);
      DocPutS(doc,"$$ID,-2$$");
    } else {
      DocPrintF(doc,"%10,d ",tempm->user_data);
      DocPrintF(doc,"$$LK,\"%s\",\"FI:%s\"$$\n",
	  tempm->name,tempm->full_name);
    }
    tempm=tempm->next;
  }
}

public I64 TreeSizeRep(U8 *files_find_mask="/TempleOS/*",U8 *fu_flags=NULL)
{
  I64 fuf_flags=0,result=0;
  CDoc *doc=DocNew;
  CDirEntry *tempm1=NULL;
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (tempm1=FilesFind(files_find_mask,fuf_flags)) {
    result=TreeSizeRepFiles(tempm1);
    DocPrintF(doc,"%10,d $$TR+C,\"%s\"$$\n$$ID,2$$",result,files_find_mask);
    PutTreeSizeFiles(doc,tempm1);
    DocPutS(doc,"$$ID,-2$$");
    CDirLstDel(tempm1);
  }
  DocInsDoc(DocCur,doc);
  DocDel(doc);
  return result;
}

#help_index ""
