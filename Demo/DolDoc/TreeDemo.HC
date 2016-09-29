U0 TreeSub(CDoc *doc,CDirEntry *tmpde)
{
  CDirEntry *tmpde1;
  while (tmpde) {
    tmpde1=tmpde->next;
    if (tmpde->attr & RS_ATTR_DIR) {
      DocPrint(doc,"$$TR,\"\"$$");
      DocPrint(doc,"$$MA,T=\"%s\",LM=\"Cd(\\\"%s\\\");Dir;\n\"$$\n",
	    tmpde->name,tmpde->full_name);
      if (tmpde->sub) {
	DocPrint(doc,"$$ID,+2$$");
	TreeSub(doc,tmpde->sub);
	DocPrint(doc,"$$ID,-2$$");
      }
    } else
      DocPrint(doc,"$$LK,\"%s\",A=\"FI:%s\"$$\n",
	    tmpde->name,tmpde->full_name);
//Note there is also a routine
      //to delete an entire CDirEntry tree.
      //See $LK,"DirTreeDel",A="MN:DirTreeDel"$().
    DirEntryDel(tmpde);
    tmpde=tmpde1;
  }
}

U0 TreeDemo()
{
  I64 fuf_flags=0;
  CDoc *doc=DocNew;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  DocPrint(doc,"$$TR-C,\"\"$$\n");
  DocPrint(doc,"$$ID,+2$$");
  TreeSub(doc,FilesFind("/*",fuf_flags));
  DocPrint(doc,"$$ID,-2$$");
  DocInsDoc(DocPut,doc);
  DocDel(doc);
}
 
TreeDemo;
