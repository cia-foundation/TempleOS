/*There is no FPrintF type function and no
way to grow files. Therefore, use mem to
hold the file until you are done.  The $LK,"CDoc",A="MN:CDoc"$
framework is convenient for this.
*/

U0 TreeSub(CDoc *doc,CDirEntry *tempde)
{
  CDirEntry *tempde1;
  while (tempde) {
    tempde1=tempde->next;
    if (tempde->attr & RS_ATTR_DIR) {
      DocPrint(doc,"$$TR,\"\"$$");
      DocPrint(doc,"$$MA,T=\"%s\",LM=\"Cd(\\\"%s\\\");Dir;\n\"$$\n",
      tempde->name,tempde->full_name);
      if (tempde->sub) {
	DocPrint(doc,"$$ID,+2$$");
	TreeSub(doc,tempde->sub);
	DocPrint(doc,"$$ID,-2$$");
      }
    } else
      DocPrint(doc,"$$LK,\"%s\",A=\"FI:%s\"$$\n",
	tempde->name,tempde->full_name);
    //Note there is also a routine
    //to delete an entire CDirEntry tree.
    //See $LK,"DirTreeDel",A="MN:DirTreeDel"$().
    DirEntryDel(tempde);
    tempde=tempde1;
  }
}

U0 FPrintFDemo(U8 *output_filename=NULL)
{
  I64 fuf_flags=0;
  CDoc *doc=DocNew(output_filename);
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  DocPrint(doc,"$$TR-C,\"\"$$\n");
  DocPrint(doc,"$$ID,+2$$");
  TreeSub(doc,FilesFind("/*",fuf_flags));
  DocPrint(doc,"$$ID,-2$$");
  DocRecalc(doc);
  if (output_filename)
    DocWrite(doc,FALSE);
  else
    DocWrite(doc,TRUE);
  DocDel(doc);
}

FPrintFDemo;
