#help_index "Cmd Line (Typically)"

I64 LineRep1(CDirEntry *tmpde)
{
  CDoc *doc;
  I64 res=0,i;
  while (tmpde) {
    i=0;
    if (tmpde->attr&RS_ATTR_DIR)
      i=LineRep1(tmpde->sub);
    else {
      doc=DocRead(tmpde->full_name);
      if (doc->head.next!=doc)
	i=doc->head.last->y+2;
      DocDel(doc);
    }
    tmpde->user_data=i;
    res+=i;
    tmpde=tmpde->next;
  }
  return res;
}
U0 LineRep2(CDoc *doc,CDirEntry *tmpde)
{
  while (tmpde) {
    if (tmpde->attr&RS_ATTR_DIR) {
      DocPrint(doc,"%8,d $$TR+C,\"%s\"$$\n$$ID,2$$",tmpde->user_data,tmpde->name);
      LineRep2(doc,tmpde->sub);
      DocPrint(doc,"$$ID,-2$$");
    } else
      DocPrint(doc,"%8,d $$LK,\"%s\",A=\"FI:%s\"$$\n",
	    tmpde->user_data,tmpde->name,tmpde->full_name);
    tmpde=tmpde->next;
  }
}
public I64 LineRep(U8 *files_find_mask="/*",U8 *fu_flags=NULL)
{//Source line-of-code count report.
  I64 fuf_flags=0,res=0;
  CDoc *doc=DocNew;
  CDirEntry *tmpde1=NULL;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+S");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (tmpde1=FilesFind(files_find_mask,fuf_flags)) {
    res=LineRep1(tmpde1);
    DocPrint(doc,"%8,d $$TR+C,\"%s\"$$\n$$ID,2$$",res,files_find_mask);
    LineRep2(doc,tmpde1);
    DocPrint(doc,"$$ID,-2$$");
    DirTreeDel(tmpde1);
  }
  DocInsDoc(DocPut,doc);
  DocDel(doc);
  return res;
}

I64 SizeRep1(CDirEntry *tmpde,I64 *_fuf_flags)
{
  I64 res=0,i;
  U8 buf[BLK_SIZE];
  CDrv *dv;
  while (tmpde) {
    if (tmpde->attr&RS_ATTR_DIR)
      i=SizeRep1(tmpde->sub,_fuf_flags);
    else if ((i=tmpde->size) && Bt(_fuf_flags,FUf_EXPAND) &&
	  FileAttr(tmpde->name)&RS_ATTR_COMPRESSED) {
      dv=Let2Drv(*tmpde->full_name);
      BlkRead(dv,buf,Clus2Blk(dv,tmpde->clus),1);
      i=(&buf)(CArcCompress *)->expanded_size;
    }
    tmpde->user_data=i;
    res+=i;
    tmpde=tmpde->next;
  }
  return res;
}
U0 SizeRep2(CDoc *doc,CDirEntry *tmpde)
{
  while (tmpde) {
    if (tmpde->attr&RS_ATTR_DIR) {
      DocPrint(doc,"%10,d $$TR+C,\"%s\"$$\n$$ID,2$$",
	    tmpde->user_data,tmpde->name);
      SizeRep2(doc,tmpde->sub);
      DocPrint(doc,"$$ID,-2$$");
    } else {
      DocPrint(doc,"%10,d ",tmpde->user_data);
      DocPrint(doc,"$$LK,\"%s\",A=\"FI:%s\"$$\n",
	    tmpde->name,tmpde->full_name);
    }
    tmpde=tmpde->next;
  }
}
public I64 SizeRep(U8 *files_find_mask="/*",U8 *fu_flags=NULL)
{//Report file sizes. "+x" for expanded size of compressed files.
  I64 fuf_flags=0,res=0;
  CDoc *doc=DocNew;
  CDirEntry *tmpde1=NULL;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (tmpde1=FilesFind(files_find_mask,fuf_flags&FUG_FILES_FIND)) {
    fuf_flags&=FUF_EXPAND;
    res=SizeRep1(tmpde1,&fuf_flags);
    DocPrint(doc,"%10,d $$TR+C,\"%s\"$$\n$$ID,2$$",res,files_find_mask);
    SizeRep2(doc,tmpde1);
    DocPrint(doc,"$$ID,-2$$");
    DirTreeDel(tmpde1);
  }
  DocInsDoc(DocPut,doc);
  DocDel(doc);
  return res;
}
