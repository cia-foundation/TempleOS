#help_index "Doc/File"
public U0 DocLoad(CDoc *doc,U8 *src2,I64 size)
{
  I64 i;
  U8 *src;
  Bool unlock=DocLock(doc);
  CDocBin *tempb;
  doc->find_replace->filter_lines=0;
  if (src2) {
    DocPutS(doc,src2);
    src=src2+StrLen(src2)+1;
    i=size-(offset(CDocBin.data)-offset(CDocBin.start));
    while (src<=src2+i) {
      tempb=CAlloc(sizeof(CDocBin),doc->mem_task);
      MemCpy(&tempb->start,src,offset(CDocBin.data)-offset(CDocBin.start));
      src+=offset(CDocBin.data)-offset(CDocBin.start);
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
  DocHome(doc); //DOCT_CURSOR will set it in recalc
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public CDoc *DocRead(U8 *name,I64 flags=0)
{
  CDoc *doc=DocNew;
  U8 *src,*name2;
  I64 size=0;
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
{
  CDocEntry *doc_e;
  CDocBin *b;
  Bool unlock=DocLock(doc);
  I64 i,cnt;
  U8 *st,*result,*dst,*src,ch;

  if (doc->flags&DOCF_NO_CURSOR)
    cnt=1; //terminator
  else
    cnt=2; //cursor + terminator

  if (!(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS)))
    DocBinsValidate(doc);
  DocRecalc(doc);
  for (doc_e=doc->root.next;doc_e!=doc;doc_e=doc_e->next) {
    if (!Bt(doc_data_types,doc_e->type_u8)) {
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
	case DOCT_TEXT://$BK,1$Update $LK,"DOCT_TEXT","FF:D:/TempleOS/Compiler/Lex.CPP,DOCT_TEXT"$$BK,0$
	  if (!(doc_e->flags1 & ~(DOCG_BL_IV_UL|DOCEF1_WORD_WRAP|DOCEF1_HIGHLIGHT|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER|DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))) {
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
	      doc_e->type_u8!=DOCT_CURSOR &&
	      doc_e->type_u8!=DOCT_BPT) {
	    st=Doc2PlainText(doc,doc_e);
	    cnt+=StrLen(st)+2;
	    Free(st);
	  }
       }
    }
  }
  for (b=doc->bin_root.next;b!=&doc->bin_root;b=b->next) {
    if (b->use_cnt>b->temp_use_cnt)
      cnt+=offset(CDocBin.data)-
	   offset(CDocBin.start)+b->size;
  }
  result=MAlloc(cnt);
  dst=result;
  for (doc_e=doc->root.next;doc_e!=doc;doc_e=doc_e->next) {
    if (!Bt(doc_data_types,doc_e->type_u8)) {
      if (doc_e->type_u8==DOCT_TEXT &&
	!(doc_e->flags1 & ~(DOCG_BL_IV_UL|DOCEF1_WORD_WRAP|DOCEF1_HIGHLIGHT|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER|DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))) {
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
	      *dst++=CH_CARRIAGE_RETURN;
	    *dst++='\n';
	    break;
	  default:
	    if (doc_e->type_u8!=DOCT_SOFT_NEW_LINE &&
		doc_e->type_u8!=DOCT_CURSOR &&
		doc_e->type_u8!=DOCT_BPT) {
	      *dst++='$$';
	      st=Doc2PlainText(doc,doc_e);
	      StrCpy(dst,st);
	      dst+=StrLen(st);
	      *dst++='$$';
	      Free(st);
	    }
	    break;
	}
      }
    }
  }
  *dst++=0;
  b=doc->bin_root.next;
  if (b!=&doc->bin_root) {
    do {
      if (b->use_cnt>b->temp_use_cnt) {
	MemCpy(dst,&b->type,offset(CDocBin.data)-offset(CDocBin.start));
	dst+=offset(CDocBin.data)-offset(CDocBin.start);
	MemCpy(dst,b->data,b->size);
	dst+=b->size;
      }
      b=b->next;
    } while (b!=&doc->bin_root);
  } else
    cnt--;
  if (_size) *_size=cnt;
  if (unlock)
    DocUnlock(doc);
  return result;
}

public U0 DocWrite(CDoc *doc,Bool prompt=FALSE)
{
  I64 size;
  U8 *buf=DocSave(doc,&size);
  if (prompt) {
    if (DocFormDo(&doc->filename,"CEdFileName"))
      FileWrite(doc->filename.name,buf,size,0,doc->file_attr);
  } else
    FileWrite(doc->filename.name,buf,size,0,doc->file_attr);
  Free(buf);
}

#help_index "Doc"
public U0 DocInsDoc(CDoc *doc,CDoc *doc2)
{  //TODO: $LK,"DocRst","MN:DocRst"$
  U8 *dst;
  Bool unlock_doc=DocLock(doc),
	 unlock_doc2=DocLock(doc2);
  CDocEntry *doc_ne,*doc_e=doc2->root.next;
  DocRemSoftNewLines(doc,NULL);
  if (doc->cur_entry->type_u8==DOCT_TEXT &&
      doc->cur_data_col>doc->cur_entry->min_col) {
    if (doc->cur_data_col<doc->cur_entry->max_col) {
      doc_ne=CAlloc(sizeof(CDocEntryBase)+sizeof(U8 *),doc->mem_task);
      doc_ne->type=DOCT_TEXT | doc->cur_entry->type & 0xFFFFFF00;
      doc_ne->flags1=doc->cur_entry->flags1;
      doc_ne->flags2=doc->cur_entry->flags2;
      doc_ne->x=doc->cur_entry->x;
      doc_ne->y=doc->cur_entry->y;
      doc_ne->page_line_num=doc->cur_entry->page_line_num;
      dst=doc->cur_entry->tag+doc->cur_data_col;
      doc_ne->tag=StrNew(dst,doc->mem_task);
      doc_ne->max_col=StrLen(dst);
      *dst=0;
      doc->cur_entry->max_col=doc->cur_data_col;
      QueIns(doc_ne,doc->cur_entry);
      doc->cur_entry=doc_ne;
      doc->cur_data_col=doc_ne->min_col;
    } else
      if (doc->cur_entry!=doc)
	doc->cur_entry=doc->cur_entry->next;
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

#help_index "Doc/Clipboard"
public U0 ClipBoardDel()
{
  DocRst(sys_clipboard_doc,TRUE);
}

public U0 EdCopyToClipboard(CDoc *doc)
{
  CDoc *doc2=sys_clipboard_doc;
  Bool unlock_doc=DocLock(doc),
	 unlock_doc2=DocLock(doc2);
  CDocEntry *doc_e=doc->root.next,*doc_ne;
  ClipBoardDel;
  while (doc_e!=doc) {
    if (doc_e->type & DOCET_SELECTED) {
      doc_e->type&=~DOCET_SELECTED;
      if (!Bt(doc_data_types,doc_e->type_u8)) {
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
{
  CDoc *doc2=sys_clipboard_doc;
  Bool unlock_doc=DocLock(doc),
	 unlock_doc2=DocLock(doc2);
  CDocEntry *doc_e=doc->root.next,*doc_e1,*doc_ne,*doc_e2=NULL;
  ClipBoardDel;
  while (doc_e!=doc) {
    doc_e1=doc_e->next;
    if (doc_e->type & DOCET_SELECTED) {
      doc_e->type&=~DOCET_SELECTED;
      if (!Bt(doc_data_types,doc_e->type_u8)) {
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
  doc->recalc_start=doc;
  DocRemSoftNewLines(doc,NULL);
  if (unlock_doc2)
    DocUnlock(doc2);
  if (unlock_doc)
    DocUnlock(doc);
}

public U0 EdPasteClipboard(CDoc *doc)
{
  CDoc *doc2=sys_clipboard_doc;
  Bool unlock_doc=DocLock(doc),
      unlock_doc2=DocLock(doc2);
  CDocEntry *doc_ce=doc->cur_entry,*doc_e;
  if (doc_ce->type_u8==DOCT_DATA) {
    doc_e=doc2->root.next;
    while (doc_e!=doc2) {
      if (Bt(doc_tag_types,doc_e->type_u8))
	DocPutSPartial(doc,doc_e->tag);
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
{ //deletes including end points
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
  doc->recalc_start=doc;
  DocRemSoftNewLines(doc,NULL);
  if (unlock_doc)
    DocUnlock(doc);
}

public CDoc *DocCopy(CDoc *doc,CDocEntry *start,CDocEntry *end)
{  //copies including end points
  CDoc *doc2=DocNew;
  Bool unlock_doc=DocLock(doc);
  CDocEntry *doc_ne;
  while (start!=doc) {
    if (!Bt(doc_data_types,start->type_u8)) {
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

#help_index "Doc;Compiler/Directive"

public U0 StreamDoc(CDoc *doc)
{  //TODO: $LK,"DocRst","MN:DocRst"$
  Bool unlock_doc=DocLock(doc);
  CDocEntry *doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT)
      StreamPutS(doc_e->tag);
    else if (doc_e->type_u8==DOCT_NEW_LINE)
      StreamPutS("\n");
    else if (doc_e->type_u8==DOCT_TAB)
      StreamPutS("\t");
    doc_e=doc_e->next;
  }
  if (unlock_doc)
    DocUnlock(doc);
}

#help_index "Doc"

Bool DocCaptureUndo(CDoc *doc,Bool force=FALSE)
{
  Bool result=FALSE,unlock;
  I64 time_stamp;
  CDocUndo *u;
  if (doc->flags&DOCF_ALLOW_UNDO) {
    unlock=DocLock(doc);
    time_stamp=GetTimeStamp;
    if (doc->flags&DOCF_UNDO_DIRTY &&
	time_stamp>doc->undo_root.last->time_stamp+time_stamp_freq<<4 || force) {
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


#help_index "File/Cmd Line (Typically);Doc/Cmd Line (Typically);Cmd Line (Typically)"

public Bool Type(U8 *fname)
//Output a file to the cmd line.
//Even graphics and macros will be sent.
{
  CDoc *doc,*doc2;
  if (doc=DocCur) {
    if (FilesFindMatch(FILENAME_MASK_TXT,fname)) {
      doc2=DocRead(fname);
      DocInsDoc(doc,doc2);
      DocDel(doc2);
      return TRUE;
    } else if (FilesFindMatch("*.BMP.Z",fname)) {
      DocPutBMPFile(doc,fname,FALSE);
      return TRUE;
    } else if (FilesFindMatch("*.BMP;*.bmp",fname)) {
      DocPutBMPFile(doc,fname,TRUE);
      return TRUE;
    } else if (FilesFindMatch("*.GRA*",fname)) {
      DocPutGRAFile(doc,fname);
      return TRUE;
    }
  }
  return FALSE;
}

#help_index "Doc"
