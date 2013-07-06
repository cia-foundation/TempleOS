/* There is no FPrintF type function and no
way to grow files. Therefore, use mem to
hold the file until you are done.  The $LK,"CDoc","MN:CDoc"$
framework is convenient for this.
*/

U0 TreeSub(CDoc *doc,CDirEntry *tempm)
{
  CDirEntry *tempm1;
  while (tempm) {
    tempm1=tempm->next;
    if (tempm->attr & _ATTR_DIR) {
      DocPutS(doc,"$$TR,\"\"$$");
      DocPrintF(doc,"$$MA+A,\"%s\",\"Cd(\\\"%s\\\");Dir;\n\"$$\n",
      tempm->name,tempm->full_name);
      if (tempm->sub) {
	DocPutS(doc,"$$ID,+2$$");
	TreeSub(doc,tempm->sub);
	DocPutS(doc,"$$ID,-2$$");
      }
    } else {
      DocPrintF(doc,"$$LK,\"%s\",\"FI:%s\"$$\n",
	tempm->name,tempm->full_name);
    }
    //Note there is also a routine
    //to delete an entire CDirEntry tree.
    //See $LK,"CDirLstDel","MN:CDirLstDel"$().
    CDirEntryDel(tempm);
    tempm=tempm1;
  }
}

U0 FPrintFDemo(U8 *output_filename=NULL)
{
  I64 fuf_flags=0;
  CDoc *doc=DocNew;
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  DocPutS(doc,"$$TR-C+AL,\"\"$$\n");
  DocPutS(doc,"$$ID,+2$$");
  TreeSub(doc,FilesFind("/TempleOS/*",fuf_flags));
  DocPutS(doc,"$$ID,-2$$");
  DocRecalc(doc);
  if (output_filename) {
    StrCpy(doc->filename.name,output_filename);
    DocWrite(doc,FALSE);
  } else
    DocWrite(doc,TRUE);
  DocDel(doc);
}

FPrintFDemo;
