#help_index "DolDoc/File"

public U0 DocLoad(CDoc *doc,U8 *src2,I64 size)
{//Fetch doc from raw mem buf.
  I64 i;
  U8 *src;
  Bool unlock=DocLock(doc);
  CDocBin *tempb;
  doc->find_replace->filter_lines=0;
  if (src2) {
    DocPutS(doc,src2); //Too big $LK,"DocPrint",A="MN:DocPrint"$() is wasteful.
    src=src2+StrLen(src2)+1;
    i=size-(offset(CDocBin.end)-offset(CDocBin.start));
    while (src<=src2+i) {
      tempb=CAlloc(sizeof(CDocBin),doc->mem_task);
      MemCpy(&tempb->start,src,offset(CDocBin.end)-offset(CDocBin.start));
      src+=offset(CDocBin.end)-offset(CDocBin.start);
      tempb->data=MAlloc(tempb->size,doc->mem_task);
      if (tempb->size) {
	MemCpy(tempb->data,src,tempb->size);
	src+=tempb->size;
      }
      QueIns(tempb,doc->bin_root.last);
      if (tempb->num>=doc->cur_bin_num)
	doc->cur_bin_num=tempb->num+1;
    }
  }
  if (!(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS)))
    DocBinsValidate(doc);
  DocTop(doc); //DOCT_CURSOR will set it in recalc
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public CDoc *DocRead(U8 *name=NULL,I64 flags=0)
{//Fetch doc from disk. See $LK,"flags",A="MN:DOCF_PLAIN_TEXT"$.
  CDoc *doc=DocNew;
  U8 *src,*name2;
  I64 size=0;
  if (!name) name=dsk.temp_filename;
  doc->flags|=flags;
  name2=FileNameAbs(name);
  StrCpy(doc->filename.name,name2);
  src=FileRead(name2,&size,&doc->file_attr);
  Free(name2);
  if (src) {
    DocLoad(doc,src,size);
    Free(src);
  }
  return doc;
}

public U8 *DocSave(CDoc *doc,I64 *_size=NULL)
{//Store doc to raw mem buf.
  CDocEntry *doc_e;
  CDocBin *b;
  Bool unlock=DocLock(doc);
  I64 ch,i,cnt;
  U8 *st,*result,*dst,*src;

  if (doc->flags&DOCF_NO_CURSOR || doc->root.next==doc)
    cnt=1; //terminator
  else
    cnt=2; //cursor + terminator

  if (!(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS)))
    DocBinsValidate(doc);
  DocRecalc(doc);
  for (doc_e=doc->root.next;doc_e!=doc;doc_e=doc_e->next) {
    if (!Bt(doldoc.type_flags_data,doc_e->type_u8)) {
      switch (doc_e->type_u8) {
	case DOCT_TAB:
	case DOCT_PAGE_BREAK:
	  cnt++;
	  break;
	case DOCT_NEW_LINE:
	  if (doc->flags&DOCF_CARRIAGE_RETURN)
	    cnt+=2;
	  else
	    cnt++;
	  break;
	case DOCT_TEXT://$BK,1$Update $LK,"DOCT_TEXT",A="FF:D:/Compiler/Lex.CPP,DOCT_TEXT"$$BK,0$
	  if (!(doc_e->de_flags & ~(DOCEF_TAG|DOCG_BL_IV_UL|DOCEF_WORD_WRAP|DOCEF_HIGHLIGHT|DOCEF_SKIP|DOCEF_HARD_SKIP|DOCEF_FILTER_SKIP))) {
	    cnt+=StrLen(doc_e->tag);
	    if (!(doc->flags & (DOCF_PLAIN_TEXT | DOCF_PLAIN_TEXT_WITH_TABS)) ||
	      doc->flags&DOCF_DBL_DOLLARS)
	      cnt+=StrOcc(doc_e->tag,'$$');
	  } else {
	    st=Doc2PlainText(doc,doc_e);
	    cnt+=StrLen(st)+2;
	    Free(st);
	  }
	  break;
	default:
	  if (doc_e->type_u8!=DOCT_SOFT_NEW_LINE &&
	      doc_e->type_u8!=DOCT_CURSOR) {
	    st=Doc2PlainText(doc,doc_e);
	    cnt+=StrLen(st)+2;
	    Free(st);
	  }
       }
    }
  }
  for (b=doc->bin_root.next;b!=&doc->bin_root;b=b->next) {
    if (b->use_cnt>b->temp_use_cnt)
      cnt+=offset(CDocBin.end)-
	   offset(CDocBin.start)+b->size;
  }
  result=MAlloc(cnt);
  dst=result;
  for (doc_e=doc->root.next;doc_e!=doc;doc_e=doc_e->next) {
    if (!Bt(doldoc.type_flags_data,doc_e->type_u8)) {
      if (doc_e->type_u8==DOCT_TEXT &&
	!(doc_e->de_flags & ~(DOCEF_TAG|DOCG_BL_IV_UL|DOCEF_WORD_WRAP|DOCEF_HIGHLIGHT|DOCEF_SKIP|DOCEF_HARD_SKIP|DOCEF_FILTER_SKIP))) {
	src=doc_e->tag;
	i=0;
	while (ch=*src++) {
	  if (doc_e==doc->cur_entry && i++==doc->cur_data_col && !(doc->flags&DOCF_NO_CURSOR))
	    *dst++=CH_CURSOR;
	  *dst++=ch;
	  if (ch=='$$') {
	    if (!(doc->flags & (DOCF_PLAIN_TEXT | DOCF_PLAIN_TEXT_WITH_TABS)) ||
	      doc->flags&DOCF_DBL_DOLLARS)
	      *dst++=ch;
	  }
	}
	if (doc_e==doc->cur_entry && i++==doc->cur_data_col && !(doc->flags&DOCF_NO_CURSOR))
	  *dst++=CH_CURSOR;
      } else {
	if (doc_e==doc->cur_entry && !(doc->flags&DOCF_NO_CURSOR))
	  *dst++=CH_CURSOR;
	switch (doc_e->type_u8) {
	  case DOCT_TAB:
	    *dst++='\t';
	    break;
	  case DOCT_PAGE_BREAK:
	    *dst++=CH_FORM_FEED;
	    break;
	  case DOCT_NEW_LINE:
	    if (doc->flags&DOCF_CARRIAGE_RETURN)
	      *dst++='\r';
	    *dst++='\n';
	    break;
	  default:
	    if (doc_e->type_u8!=DOCT_SOFT_NEW_LINE &&
		doc_e->type_u8!=DOCT_CURSOR) {//(Cursor is set by cur_entry)
	      *dst++='$$';
	      st=Doc2PlainText(doc,doc_e);
	      StrCpy(dst,st);
	      dst+=StrLen(st);
	      *dst++='$$';
	      Free(st);
	    }
	}
      }
    }
  }
  if (doc->cur_entry==doc && doc->root.next!=doc && !(doc->flags&DOCF_NO_CURSOR))
    *dst++=CH_CURSOR;
  *dst++=0;
  if (*result==CH_CURSOR) {
    MemCpy(result,result+1,dst-result-1);
    cnt--;
    dst--;
  }
  b=doc->bin_root.next;
  if (b!=&doc->bin_root) {
    do {
      if (b->use_cnt>b->temp_use_cnt) {
	MemCpy(dst,&b->start,offset(CDocBin.end)-offset(CDocBin.start));
	dst+=offset(CDocBin.end)-offset(CDocBin.start);
	MemCpy(dst,b->data,b->size);
	dst+=b->size;
      }
      b=b->next;
    } while (b!=&doc->bin_root);
  } else
    cnt--; //No terminator
  if (_size) *_size=cnt;
  if (unlock)
    DocUnlock(doc);
  return result;
}

public Bool DocWrite(CDoc *doc,Bool prompt=FALSE)
{//Store doc to disk.
  I64 size;
  U8 *buf;
  if (prompt && !DocFormDo(&doc->filename))
    return FALSE;
  if (doc->filename.name[0]=='A'&&doc->filename.name[2]==':')
    return FALSE; //LK_DOC,LK_DOC_ANCHOR,LK_DOC_FIND,LK_DOC_LINE
  buf=DocSave(doc,&size);
  FileWrite(doc->filename.name,buf,size,0,doc->file_attr);
  Free(buf);
  return TRUE;
}

#help_index "DolDoc"
public U0 DocInsDoc(CDoc *doc=NULL,CDoc *doc2)
{//Insert copy of doc2 into doc at insert pt, cur_entry.
//TODO: $LK,"DocRst",A="MN:DocRst"$
  U8 *dst;
  Bool unlock_doc,unlock_doc2=DocLock(doc2);
  CDocEntry *doc_ne,*doc_e=doc2->root.next,*doc_ce;
  if (!doc) doc=DocPut;
  unlock_doc=DocLock(doc),
  DocRemSoftNewLines(doc,NULL);
  doc_ce=doc->cur_entry;
  if (doc_ce->type_u8==DOCT_TEXT && doc->cur_data_col>doc_ce->min_col) {
    if (doc->cur_data_col<doc_ce->max_col) {
      dst=doc_ce->tag+doc->cur_data_col;
      doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
      *dst=0;
      doc_ne->type=DOCT_TEXT | doc_ce->type & 0xFFFFFF00;
      doc_ce->max_col=doc->cur_data_col;
      QueIns(doc_ne,doc_ce);
      doc->cur_entry=doc_ne;
      doc->cur_data_col=doc_ne->min_col;
    } else
      if (doc_ce!=doc)
	doc->cur_entry=doc_ce->next;
  }
  while (doc_e!=doc2) {
    if (doc_e->type_u8!=DOCT_SOFT_NEW_LINE) {
      doc_ne=DocEntryCopy(doc,doc_e);
      QueIns(doc_ne,doc->cur_entry->last);
    }
    doc_e=doc_e->next;
  }
  DocRecalc(doc);
  if (unlock_doc2)
    DocUnlock(doc2);
  if (unlock_doc)
    DocUnlock(doc);
}

#help_index "DolDoc/Clipboard"
public U0 ClipboardDel()
{//Delete everything on clipboard.
  DocRst(sys_clipboard_doc,TRUE);
}

public U0 EdCopyToClipboard(CDoc *doc)
{//Copy $LK,"DOCET_SELECTED",A="MN:DOCET_SELECTED"$ flagged entries to clipboard.
  CDoc *doc2=sys_clipboard_doc;
  Bool unlock_doc=DocLock(doc),
	 unlock_doc2=DocLock(doc2);
  CDocEntry *doc_e=doc->root.next,*doc_ne;
  ClipboardDel;
  while (doc_e!=doc) {
    if (doc_e->type & DOCET_SELECTED) {
      doc_e->type&=~DOCET_SELECTED;
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
{//Remove selected entries and place on clipboard.
  CDoc *doc2=sys_clipboard_doc;
  Bool unlock_doc=DocLock(doc),
	 unlock_doc2=DocLock(doc2);
  CDocEntry *doc_e=doc->root.next,*doc_e1,*doc_ne,*doc_e2=NULL;
  ClipboardDel;
  while (doc_e!=doc) {
    doc_e1=doc_e->next;
    if (doc_e->type & DOCET_SELECTED) {
      doc_e->type&=~DOCET_SELECTED;
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
    doc->cur_data_col=doc_e2->min_col;
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
      doc->cur_data_col=doc->cur_entry->min_col;
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

#help_index "DolDoc/Compiler;Compiler/Directive"

public U0 StreamDoc(CDoc *doc)
{//Inject doc into compile stream. Use inside #exe{}.
//TODO: $LK,"DocRst",A="MN:DocRst"$
  Bool unlock_doc=DocLock(doc);
  CDocEntry *doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT)
      StreamPrint("%s",doc_e->tag);
    else if (doc_e->type_u8==DOCT_NEW_LINE)
      StreamPrint("\n");
    else if (doc_e->type_u8==DOCT_TAB)
      StreamPrint("\t");
    doc_e=doc_e->next;
  }
  if (unlock_doc)
    DocUnlock(doc);
}

#help_index "DolDoc"

Bool DocCaptureUndo(CDoc *doc,Bool force=FALSE)
{
  Bool result=FALSE,unlock;
  I64 time_stamp;
  CDocUndo *u;
  if (doc->flags&DOCF_ALLOW_UNDO) {
    unlock=DocLock(doc);
    time_stamp=GetTSC;
    if (doc->flags&DOCF_UNDO_DIRTY &&
	time_stamp>doc->undo_root.last->time_stamp+cnts.time_stamp_freq<<4 || force) {
      u=CAlloc(sizeof(CDocUndo),doc->mem_task);
      u->time_stamp=time_stamp;
      u->body=DocSave(doc,&u->size);
      QueIns(u,doc->undo_root.last);
      doc->flags&=~DOCF_UNDO_DIRTY;
      doc->undo_cnt++;
      u->doc_flags=doc->flags;
      result=TRUE;
    }
    if (unlock)
      DocUnlock(doc);
  }
  return result;
}
 
U0 DocUndoRestore(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  CDocUndo *u=doc->undo_root.last,*u_next,*u_last;
  if (u!=&doc->undo_root) {
    QueRem(u);
    u_next=doc->undo_root.next;
    u_last=doc->undo_root.last;
    QueInit(&doc->undo_root);
    DocRst(doc,TRUE);
    doc->flags=u->doc_flags;
    DocLoad(doc,u->body,u->size);
    DocUndoDel(doc,u);
    doc->undo_root.next=u_next;
    doc->undo_root.last=u_last;
  }
  DocUndoCntSet(doc);
  doc->flags&=~DOCF_UNDO_DIRTY;
  if (unlock)
    DocUnlock(doc);
}

#help_index "Graphics/Windows BMP Files;Graphics/GRA Files;DolDoc/Output"

public Bool DocType(CDoc *doc=NULL,U8 *filename,I64 trailing_new_lines=1)
{//Output txt or graphic file to document.
  Bool result=FALSE;
  CDoc *doc2;
  if (!doc && !(doc=DocPut) || doc->doc_signature!=DOC_SIGNATURE_VAL)
    return FALSE;
  if (FilesFindMatch(filename,FILEMASK_TXT)) {
    doc2=DocRead(filename);
    DocInsDoc(doc,doc2);
    DocDel(doc2);
    result=TRUE;
  } else if (FilesFindMatch(filename,"*.BMP.Z")) {
    DocBMPFile(doc,filename,FALSE);
    result=TRUE;
  } else if (FilesFindMatch(filename,"*.BMP;*.bmp")) {
    DocBMPFile(doc,filename,TRUE);
    result=TRUE;
  } else if (FilesFindMatch(filename,"*.GRA*")) {
    DocGRAFile(doc,filename);
    result=TRUE;
  }
  if (result)
    DocPrint(doc,"%h*c",trailing_new_lines,'\n');
  return result;
}

#help_index "Graphics/Windows BMP Files;Graphics/GRA Files;File/Cmd Line (Typically);DolDoc/Cmd Line (Typically);Cmd Line (Typically)"

public Bool Type(U8 *filename,I64 trailing_new_lines=1)
{//Output txt or graphic file to command line.
  return DocType(,filename,trailing_new_lines);
}
