#help_index "Cmd Line (Typically)"

I64 LineRep1(CDirEntry *tempde)
{
  CDoc *doc;
  I64 res=0,i;
  while (tempde) {
    i=0;
    if (tempde->attr&RS_ATTR_DIR)
      i=LineRep1(tempde->sub);
    else {
      doc=DocRead(tempde->full_name);
      if (doc->root.next!=doc)
	i=doc->root.last->y+2;
      DocDel(doc);
    }
    tempde->user_data=i;
    res+=i;
    tempde=tempde->next;
  }
  return res;
}
U0 LineRep2(CDoc *doc,CDirEntry *tempde)
{
  while (tempde) {
    if (tempde->attr&RS_ATTR_DIR) {
      DocPrint(doc,"%8,d $$TR+C,\"%s\"$$\n$$ID,2$$",tempde->user_data,tempde->name);
      LineRep2(doc,tempde->sub);
      DocPrint(doc,"$$ID,-2$$");
    } else
      DocPrint(doc,"%8,d $$LK,\"%s\",A=\"FI:%s\"$$\n",
	    tempde->user_data,tempde->name,tempde->full_name);
    tempde=tempde->next;
  }
}
public I64 LineRep(U8 *files_find_mask="/*",U8 *fu_flags=NULL)
{//Source line-of-code count report.
  I64 fuf_flags=0,res=0;
  CDoc *doc=DocNew;
  CDirEntry *tempde1=NULL;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+S");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (tempde1=FilesFind(files_find_mask,fuf_flags)) {
    res=LineRep1(tempde1);
    DocPrint(doc,"%8,d $$TR+C,\"%s\"$$\n$$ID,2$$",res,files_find_mask);
    LineRep2(doc,tempde1);
    DocPrint(doc,"$$ID,-2$$");
    DirTreeDel(tempde1);
  }
  DocInsDoc(DocPut,doc);
  DocDel(doc);
  return res;
}

I64 SizeRep1(CDirEntry *tempde,I64 *_fuf_flags)
{
  I64 res=0,i;
  U8 buf[BLK_SIZE];
  CDrv *dv;
  while (tempde) {
    if (tempde->attr&RS_ATTR_DIR)
      i=SizeRep1(tempde->sub,_fuf_flags);
    else if ((i=tempde->size) && Bt(_fuf_flags,FUf_EXPAND) &&
	  FileAttr(tempde->name)&RS_ATTR_COMPRESSED) {
      dv=Let2Drv(*tempde->full_name);
      RBlks(dv,buf,Cluster2Blk(dv,tempde->cluster),1);
      i=(&buf)(CArcCompress *)->expanded_size;
    }
    tempde->user_data=i;
    res+=i;
    tempde=tempde->next;
  }
  return res;
}
U0 SizeRep2(CDoc *doc,CDirEntry *tempde)
{
  while (tempde) {
    if (tempde->attr&RS_ATTR_DIR) {
      DocPrint(doc,"%10,d $$TR+C,\"%s\"$$\n$$ID,2$$",
	    tempde->user_data,tempde->name);
      SizeRep2(doc,tempde->sub);
      DocPrint(doc,"$$ID,-2$$");
    } else {
      DocPrint(doc,"%10,d ",tempde->user_data);
      DocPrint(doc,"$$LK,\"%s\",A=\"FI:%s\"$$\n",
	    tempde->name,tempde->full_name);
    }
    tempde=tempde->next;
  }
}
public I64 SizeRep(U8 *files_find_mask="/*",U8 *fu_flags=NULL)
{//Report file sizes. "+x" for expanded size of compressed files.
  I64 fuf_flags=0,res=0;
  CDoc *doc=DocNew;
  CDirEntry *tempde1=NULL;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (tempde1=FilesFind(files_find_mask,fuf_flags&FUG_FILES_FIND)) {
    fuf_flags&=FUF_EXPAND;
    res=SizeRep1(tempde1,&fuf_flags);
    DocPrint(doc,"%10,d $$TR+C,\"%s\"$$\n$$ID,2$$",res,files_find_mask);
    SizeRep2(doc,tempde1);
    DocPrint(doc,"$$ID,-2$$");
    DirTreeDel(tempde1);
  }
  DocInsDoc(DocPut,doc);
  DocDel(doc);
  return res;
}
