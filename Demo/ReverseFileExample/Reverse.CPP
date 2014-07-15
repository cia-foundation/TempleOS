/*We use the internal routines from the text editor
to cut and paste lines to reverse the order of
lines in a text file.
*/

U0 Reverse(U8 *in_name,U8 *out_name)
{
  CDoc	*doc_in =DocRead(in_name),
	*doc_out=DocNew(out_name);
  DocTop(doc_in);
  while (doc_in->root.next!=doc_in) {
    EdLineDown(doc_in,SC_CURSOR_DOWN|SCF_SHIFT);
    ClipboardDel;
    EdCutToClipboard(doc_in);
    DocTop(doc_out);
    EdPasteClipboard(doc_out);
  }
  DocWrite(doc_out);
}

Reverse("::/Demo/ReverseFileExample/PoemBackward.TXT.Z",
	"::/Demo/ReverseFileExample/PoemForeward.TXT.Z");
