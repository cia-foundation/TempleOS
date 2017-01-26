/*We use the internal routines from the text editor
to cut and paste lines to rev the order of
lines in a text file.
*/

Cd(__DIR__);;

U0 Rev(U8 *in_name,U8 *out_name)
{
  CDoc	*doc_in =DocRead(in_name),
	*doc_out=DocNew(out_name);
  DocTop(doc_in);
  while (doc_in->head.next!=doc_in) {
    EdLineDown(doc_in,SC_CURSOR_DOWN|SCF_SHIFT);
    ClipCut(doc_in);
    DocTop(doc_out);
    ClipPaste(doc_out);
  }
  DocWrite(doc_out);
}

Rev("DemoPoemBwd.DD.Z","~/DemoPoemFwd.DD.Z");
