/*
This uses DocPutS/DocPrintF because updates
to within a tree while a tree is collapsed get
messed-up if "live"(like using ordinary PrintF/PutS.)
The insert point is confused by hiding of
data in collapsed trees.  Therefore, the window main
Doc is taken off-line by disabling preemption so there
are no screen updates until all data has been inserted.

There is a special Doc function which is also
available to take an Doc off-line.  See $LK,"DocInsSafe","MN:DocInsSafe"$.
For this example, however, I wanted to use DocPrintF
and disabled Preemption myself.
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

U0 TreeDemo()
{
  I64 fuf_flags=0;
  CDoc *doc=DocNew;
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  DocPutS(doc,"$$TR-C+AL,\"\"$$\n");
  DocPutS(doc,"$$ID,+2$$");
  TreeSub(doc,FilesFind("/TempleOS/*",fuf_flags));
  DocPutS(doc,"$$ID,-2$$");
  DocInsDoc(DocPut,doc);
  DocDel(doc);
}
 
TreeDemo;
