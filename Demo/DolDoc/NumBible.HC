//  This makes a new Bible file
//with line numbers.

U0 NumBible()
{
  CDoc *doc_in=DocRead("::/Misc/Bible.TXT.Z",DOCF_PLAIN_TEXT_TABS),
	*doc_out=DocNew("~/DemoNumBible.DD.Z");
  CDocEntry *doc_e=doc_in->head.next;
  while (doc_e!=doc_in) {
    if (doc_e->type_u8==DOCT_TEXT) {
      DocPrint(doc_out,"%05d %s",doc_e->y+1,doc_e->tag);
      doc_e=doc_e->next;
      if (doc_e->type_u8==DOCT_NEW_LINE) {
	DocPrint(doc_out,"\n");
	doc_e=doc_e->next;
      }
    } else if (doc_e->type_u8==DOCT_NEW_LINE) {
      DocPrint(doc_out,"%05d\n",doc_e->y+1);
      doc_e=doc_e->next;
    } else
      doc_e=doc_e->next;
  }
  doc_out->flags|=DOCF_NO_CURSOR;
  DocWrite(doc_out);
  DocDel(doc_out);
  DocDel(doc_in);
}

NumBible;