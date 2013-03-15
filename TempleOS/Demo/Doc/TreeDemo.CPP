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

#include "/TempleOS/Demo/Shared/TreeDemoJoin"

U0 TreeDemo()
{
  I64 fuf_flags=0;
  CDoc *doc=DocNew;
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  DocPutS(doc,"$$TR-C+AL,\"\"$$\n");
  DocPutS(doc,"$$ID,+2$$");
  TreeSub(doc,FilesFind("/TempleOS/*",fuf_flags));
  DocPutS(doc,"$$ID,-2$$");
  DocInsDoc(DocCur,doc);
  DocDel(doc);
}
 
TreeDemo;
