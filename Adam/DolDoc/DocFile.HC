#help_index "DolDoc/File"

public U0 DocLoad(CDoc *doc,U8 *src2,I64 size)
{//Fetch doc from raw mem buf.
  I64 i;
  U8 *src;
  Bool unlock=DocLock(doc);
  CDocBin *tmpb;
  doc->find_replace->filter_lines=0;
  if (src2) {
    DocPutS(doc,src2); //Too big $LK,"DocPrint",A="MN:DocPrint"$() is wasteful.
    src=src2+StrLen(src2)+1;
    i=size-(offset(CDocBin.end)-offset(CDocBin.start));
    while (src<=src2+i) {
      tmpb=CAlloc(sizeof(CDocBin),doc->mem_task);
      MemCpy(&tmpb->start,src,offset(CDocBin.end)-offset(CDocBin.start));
      src+=offset(CDocBin.end)-offset(CDocBin.start);
      tmpb->data=MAlloc(tmpb->size,doc->mem_task);
      if (tmpb->size) {
	MemCpy(tmpb->data,src,tmpb->size);
	src+=tmpb->size;
      }
      QueIns(tmpb,doc->bin_head.last);
      if (tmpb->num>=doc->cur_bin_num)
	doc->cur_bin_num=tmpb->num+1;
    }
  }
  if (!(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_TABS)))
    DocBinsValidate(doc);
  DocTop(doc); //Calls $LK,"DocRecalc",A="MN:DocRecalc"$().  DOCT_CURSOR will be set.
  if (unlock)
    DocUnlock(doc);
}

public CDoc *DocRead(U8 *name=NULL,I64 flags=0)
{//Fetch doc from disk. See $LK,"flags",A="MN:DOCF_PLAIN_TEXT"$.
  CDoc *doc=DocNew;
  U8 *src,*name2;
  I64 size=0;
  CDirContext *dirc;
  if (!name) name=blkdev.tmp_filename;
  doc->flags|=flags;
  name2=FileNameAbs(name);
  StrCpy(doc->filename.name,name2);
  if (src=FileRead(name2,&size,&doc->file_attr)) {
    if (dirc=DirContextNew(name2)) {
      DocLoad(doc,src,size);
      DirContextDel(dirc);
    }
    Free(src);
  }
  Free(name2);
  return doc;
}

public U8 *DocSave(CDoc *doc,I64 *_size=NULL)
{//Store doc to raw mem buf.
  CDocEntry *doc_e,*doc_e1;
  CDocBin *b;
  Bool unlock=DocLock(doc);
  I64 ch,cnt=1;//terminator
  U8 *st,*res,*dst,*src;

  if (!(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_TABS)))
    DocBinsValidate(doc);
  if (doc->flags&DOCF_NO_CURSOR)
    DocRecalc(doc);
  else {
    DocRecalc(doc,RECALCF_ADD_CURSOR);
    if (doc->head.next->type_u8==DOCT_CURSOR)
      DocEntryDel(doc,doc->head.next); //If no cursor, $LK,"DocLoad",A="MN:DocLoad"$() puts at top.
  }
  for (doc_e=doc->head.next;doc_e!=doc;doc_e=doc_e->next) {
    if (!Bt(doldoc.type_flags_data,doc_e->type_u8)) {
      switch (doc_e->type_u8) {
	case DOCT_TAB:
	case DOCT_PAGE_BREAK:
	case DOCT_CURSOR:
	  cnt++;
	  break;
	case DOCT_NEW_LINE:
	  if (doc->flags&DOCF_CARRIAGE_RETURN)
	    cnt+=2;
	  else
	    cnt++;
	  break;
	case DOCT_SOFT_NEW_LINE:
	  break;
	case DOCT_TEXT:
	  if (!(doc_e->de_flags & ~(DOCEF_TAG|DOCG_BL_IV_UL|DOCEF_WORD_WRAP|
		DOCEF_HIGHLIGHT|DOCEF_SKIP|DOCEF_FILTER_SKIP))&&
		!(doc_e->type&DOCG_BL_IV_UL)) {
	    cnt+=StrLen(doc_e->tag);
	    if (!(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_TABS)) ||
		  doc->flags&DOCF_DBL_DOLLARS)
	      cnt+=StrOcc(doc_e->tag,'$$');
	    break;
	  }
	default:
	  st=Doc2PlainText(doc,doc_e);
	  cnt+=StrLen(st)+2;
	  Free(st);
      }
    }
  }
  for (b=doc->bin_head.next;b!=&doc->bin_head;b=b->next)
    if (b->use_cnt>b->tmp_use_cnt)
      cnt+=offset(CDocBin.end)-offset(CDocBin.start)+b->size;
  res=MAlloc(cnt);
  dst=res;
  doc_e=doc->head.next;
  while (doc_e!=doc) {
    doc_e1=doc_e->next;
    if (!Bt(doldoc.type_flags_data,doc_e->type_u8))
      switch (doc_e->type_u8) {
	case DOCT_CURSOR:
	  DocEntryDel(doc,doc_e);
	  *dst++=CH_CURSOR;
	  break;
	case DOCT_TAB:
	  *dst++='\t';
	  break;
	case DOCT_NEW_LINE:
	  if (doc->flags&DOCF_CARRIAGE_RETURN)
	    *dst++='\r';
	  *dst++='\n';
	  break;
	case DOCT_SOFT_NEW_LINE:
	  break;
	case DOCT_TEXT:
	  if (!(doc_e->de_flags & ~(DOCEF_TAG|DOCG_BL_IV_UL|DOCEF_WORD_WRAP|
		DOCEF_HIGHLIGHT|DOCEF_SKIP|DOCEF_FILTER_SKIP)) &&
		!(doc_e->type&DOCG_BL_IV_UL)) {
	    src=doc_e->tag;
	    while (ch=*src++) {
	      *dst++=ch;
	      if (ch=='$$' && (!(doc->flags & (DOCF_PLAIN_TEXT|
		    DOCF_PLAIN_TEXT_TABS)) || doc->flags&DOCF_DBL_DOLLARS))
		*dst++=ch;
	    }
	    break;
	  }
	default:
	  *dst++='$$';
	  st=Doc2PlainText(doc,doc_e);
	  StrCpy(dst,st);
	  dst+=StrLen(st);
	  *dst++='$$';
	  Free(st);
      }
    doc_e=doc_e1;
  }
  *dst++=0;
  b=doc->bin_head.next;
  if (b!=&doc->bin_head) {
    do {
      if (b->use_cnt>b->tmp_use_cnt) {
	MemCpy(dst,&b->start,offset(CDocBin.end)-offset(CDocBin.start));
	dst+=offset(CDocBin.end)-offset(CDocBin.start);
	MemCpy(dst,b->data,b->size);
	dst+=b->size;
      }
      b=b->next;
    } while (b!=&doc->bin_head);
  } else
    cnt--; //No terminator
  if (_size) *_size=cnt;
  if (unlock)
    DocUnlock(doc);
  return res;
}

public Bool DocWrite(CDoc *doc,Bool prompt=FALSE)
{//Store doc to disk.
  I64 size;
  U8 *buf;
  if (prompt && !DocForm(&doc->filename) ||
	doc->filename.name[0]=='A' && doc->filename.name[2]==':')
    return FALSE;  //CANCEL || LK_DOC,LK_DOC_ANCHOR,LK_DOC_FIND,LK_DOC_LINE?
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
  CDocEntry *doc_ne,*doc_e=doc2->head.next,*doc_ce;
  if (!doc) doc=DocPut;
  unlock_doc=DocLock(doc),
	DocRemSoftNewLines(doc,NULL);
  doc_ce=doc->cur_entry;
  if (doc_ce->type_u8==DOCT_TEXT && doc->cur_col>doc_ce->min_col) {
    if (doc->cur_col<doc_ce->max_col) {
      dst=doc_ce->tag+doc->cur_col;
      doc_ne=DocEntryNewTag(doc,doc_ce,dst);
      *dst=0;
      doc_ne->type=DOCT_TEXT | doc_ce->type & 0xFFFFFF00;
      doc_ce->max_col=doc->cur_col;
      QueIns(doc_ne,doc_ce);
      doc->cur_entry=doc_ne;
      doc->cur_col=doc_ne->min_col;
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

#help_index "DolDoc/Compiler;Compiler/Directive"
public U0 StreamDoc(CDoc *doc)
{//Inject doc into compile stream. Use inside #exe{}.
//TODO: $LK,"DocRst",A="MN:DocRst"$
  Bool unlock_doc=DocLock(doc);
  CDocEntry *doc_e=doc->head.next;
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
  Bool res=FALSE,unlock;
  I64 time_stamp,flags;
  CDocUndo *u;
  if (doc->flags&DOCF_ALLOW_UNDO) {
    unlock=DocLock(doc);
    time_stamp=GetTSC;
    if (doc->flags&DOCF_UNDO_DIRTY &&
	  time_stamp>doc->undo_head.last->time_stamp+cnts.time_stamp_freq<<4 ||
	  force) {
      u=CAlloc(sizeof(CDocUndo),doc->mem_task);
      u->time_stamp=time_stamp;
      flags=doc->flags;
      doc->flags&=~DOCF_NO_CURSOR;
      u->body=DocSave(doc,&u->size);
      doc->flags=flags;
      QueIns(u,doc->undo_head.last);
      doc->flags&=~DOCF_UNDO_DIRTY;
      doc->undo_cnt++;
      u->doc_flags=doc->flags;
      res=TRUE;
      if (doc->flags&DOCF_AUTO_SAVE)
	DocWrite(doc);
    }
    if (unlock)
      DocUnlock(doc);
  }
  return res;
}
 
U0 DocUndoRestore(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  CDocUndo *u=doc->undo_head.last,*u_next,*u_last;
  if (u!=&doc->undo_head) {
    QueRem(u);
    u_next=doc->undo_head.next;
    u_last=doc->undo_head.last;
    QueInit(&doc->undo_head);
    DocRst(doc,TRUE);
    doc->flags=u->doc_flags&~DOCF_NO_CURSOR;
    DocLoad(doc,u->body,u->size);
    doc->flags=u->doc_flags;
    DocUndoDel(doc,u);
    doc->undo_head.next=u_next;
    doc->undo_head.last=u_last;
  }
  DocUndoCntSet(doc);
  doc->flags&=~DOCF_UNDO_DIRTY;
  if (unlock)
    DocUnlock(doc);
}

#help_index "Graphics/GR Files;DolDoc/Output;StdOut/DolDoc"
public Bool DocType(CDoc *doc=NULL,U8 *filename,I64 trailing_new_lines=1)
{//Output txt or graphic file to document.
  Bool res=FALSE;
  CDoc *doc2;
  if (!doc && !(doc=DocPut) || doc->doc_signature!=DOC_SIGNATURE_VAL)
    return FALSE;
  if (FilesFindMatch(filename,FILEMASK_TXT)) {
    doc2=DocRead(filename);
    DocInsDoc(doc,doc2);
    if (IsRaw)
      DocDump(doc2,100000);
    DocDel(doc2);
    res=TRUE;
  } else if (FilesFindMatch(filename,"*.GR*")) {
    DocGR(doc,filename);
    res=TRUE;
  }
  if (res)
    DocPrint(doc,"%h*c",trailing_new_lines,'\n');
  return res;
}

#help_index "Graphics/GR Files;"\
	"File/Cmd Line (Typically);DolDoc/Cmd Line (Typically);"\
	"StdOut;Cmd Line (Typically)"
public Bool Type(U8 *filename,I64 trailing_new_lines=1)
{//Output txt or graphic file to command line.
  return DocType(,filename,trailing_new_lines);
}

#help_index "DolDoc/File"

public U8 *DocLineRead(U8 *filename,I64 line,CTask *mem_task=NULL)
{//Extract line from stored doc file. (Slow.)
  U8 *res=NULL;
  CDoc *doc=DocRead(filename,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR);
  if (DocGoToLine(doc,line) && doc->cur_entry->type_u8==DOCT_TEXT)
    res=StrNew(doc->cur_entry->tag,mem_task);
  DocDel(doc);
  return res;
}

public U8 *DocLineWrite(U8 *filename,I64 line,U8 *st)
{//Write line to stored doc file. (Slow.)
  U8 *res=NULL;
  CDoc *doc=DocRead(filename,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR);
  if (DocGoToLine(doc,line)) {
    if (doc->cur_entry->type_u8==DOCT_TEXT) {
      Free(doc->cur_entry->tag);
      doc->cur_entry->tag=StrNew(st);
    } else
      DocPrint(doc,"%s",st);
    DocTop(doc);
    DocWrite(doc);
  }
  DocDel(doc);
  return res;
}
