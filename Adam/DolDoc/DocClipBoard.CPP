#help_index "DolDoc/Clipboard"

sys_clipboard_doc=DocNew;

public U0 ClipboardDel()
{//Delete everything on clipboard.
  DocRst(sys_clipboard_doc,TRUE);
}

public U0 EdCopyToClipboard(CDoc *doc)
{//Copy $LK,"DOCET_SEL",A="MN:DOCET_SEL"$ flagged entries to clipboard.
  CDoc *doc2=sys_clipboard_doc;
  Bool unlock_doc=DocLock(doc),
	unlock_doc2=DocLock(doc2);
  CDocEntry *doc_e=doc->root.next,*doc_ne;
  ClipboardDel;
  while (doc_e!=doc) {
    if (doc_e->type & DOCET_SEL) {
      doc_e->type&=~DOCET_SEL;
      if (!Bt(doldoc.type_flags_data,doc_e->type_u8)) {
	doc_ne=DocEntryCopy(doc2,doc_e);
	QueIns(doc_ne,doc2->root.last);
      }
    }
    doc_e=doc_e->next;
  }
  if (unlock_doc2)
    DocUnlock(doc2);
  if (unlock_doc)
    DocUnlock(doc);
}

public U0 EdCutToClipboard(CDoc *doc)
{//Remove sel entries and place on clipboard.
  CDoc *doc2=sys_clipboard_doc;
  Bool unlock_doc=DocLock(doc),
	unlock_doc2=DocLock(doc2);
  CDocEntry *doc_e=doc->root.next,*doc_e1,*doc_ne,*doc_e2=NULL;
  ClipboardDel;
  while (doc_e!=doc) {
    doc_e1=doc_e->next;
    if (doc_e->type & DOCET_SEL) {
      doc_e->type&=~DOCET_SEL;
      if (!Bt(doldoc.type_flags_data,doc_e->type_u8)) {
	doc_ne=DocEntryCopy(doc2,doc_e);
	QueIns(doc_ne,doc2->root.last);
      }
      if (doc_e==doc->cur_entry || doc_e==doc_e2)
	doc_e2=doc_e->next;
      DocEntryDel(doc,doc_e);
    }
    doc_e=doc_e1;
  }
  if (doc_e2) {
    doc->cur_entry=doc_e2;
    doc->cur_col=doc_e2->min_col;
  }
  DocRemSoftNewLines(doc,NULL);
  if (unlock_doc2)
    DocUnlock(doc2);
  if (unlock_doc)
    DocUnlock(doc);
}

public U0 EdPasteClipboard(CDoc *doc)
{//Insert copy of clipboard at insert pt, cur_entry.
  CDoc *doc2=sys_clipboard_doc;
  Bool unlock_doc=DocLock(doc),
	unlock_doc2=DocLock(doc2);
  CDocEntry *doc_ce=doc->cur_entry,*doc_e;
  if (doc_ce->type_u8==DOCT_DATA) {
    doc_e=doc2->root.next;
    while (doc_e!=doc2) {
      if (doc_e->de_flags&DOCEF_TAG)
	DocPrintPartial(doc,"%s",doc_e->tag);
      doc_e=doc_e->next;
    }
  } else
    DocInsDoc(doc,doc2);
  if (unlock_doc2)
    DocUnlock(doc2);
  if (unlock_doc)
    DocUnlock(doc);
}

public U0 DocCut(CDoc *doc,CDocEntry *start,CDocEntry *end)
{//Del start to end entry, including end points.
  Bool unlock_doc=DocLock(doc);
  CDocEntry *doc_e1;
  while (start!=doc) {
    doc_e1=start->next;
    if (start==doc->cur_entry) {
      doc->cur_entry=start->next;
      doc->cur_col=doc->cur_entry->min_col;
    }
    DocEntryDel(doc,start);
    if (start==end)
      break;
    start=doc_e1;
  }
  DocRemSoftNewLines(doc,NULL);
  if (unlock_doc)
    DocUnlock(doc);
}

public CDoc *DocCopy(CDoc *doc,CDocEntry *start,CDocEntry *end)
{//Copies start to end entry, including end points.
  CDoc *doc2=DocNew;
  Bool unlock_doc=DocLock(doc);
  CDocEntry *doc_ne;
  while (start!=doc) {
    if (!Bt(doldoc.type_flags_data,start->type_u8)) {
      doc_ne=DocEntryCopy(doc2,start);
      QueIns(doc_ne,doc2->root.last);
    }
    if (start==end)
      break;
    start=start->next;
  }
  if (unlock_doc)
    DocUnlock(doc);
  return doc2;
}
