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
