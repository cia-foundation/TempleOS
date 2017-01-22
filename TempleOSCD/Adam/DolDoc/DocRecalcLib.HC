#help_index "DolDoc"

I64 DocCharDist(CDoc *doc,I64 x,I64 y)
{
#assert FONT_WIDTH==FONT_HEIGHT
  return (SqrI64(doc->x-x)+SqrI64(doc->y-y))*FONT_WIDTH*FONT_WIDTH;
}

U0 DocDelToNum(CDoc *doc,I64 num)
{
  CDocEntry *doc_e=doc->head.next,*doc_e2;
  while (num-->0 && doc_e!=doc) {
    doc_e2=doc_e->next;
    if (!(doc_e->de_flags & (DOCEF_HOLD|DOCEF_FILTER_SKIP))) {
      if (doc_e==doc->cur_entry) {
	doc->cur_entry=doc_e2;
	doc->cur_col=doc_e2->min_col;
      }
      DocEntryDel(doc,doc_e);
    }
    doc_e=doc_e2;
  }
}

U0 DocDelToEntry(CDoc *doc,CDocEntry *clear_entry,Bool clear_holds)
{
  CDocEntry *doc_e=doc->head.next,*doc_e2;
  while (doc_e!=doc) {
    doc_e2=doc_e->next;
    if (!(doc_e->de_flags & (DOCEF_HOLD|DOCEF_FILTER_SKIP))||clear_holds) {
      if (doc_e==doc->cur_entry) {
	doc->cur_entry=doc_e2;
	doc->cur_col=doc_e2->min_col;
      }
      DocEntryDel(doc,doc_e);
    }
    if (doc_e==clear_entry)
      break;
    doc_e=doc_e2;
  }
}

U0 DocBorderLstDraw(CDoc *doc)
{
  CTask *win_task=doc->win_task;
  I64 i,y=-1,attr=win_task->border_attr<<8;
  U64 ch;
  CDoc *tmpl=doc;
  CD3I64 saved_scroll;
  while (tmpl && tmpl->doc_signature==DOC_SIGNATURE_VAL) {
    y+=(Bsr(tmpl->desc)+7)>>3+1; //StrLen+1
    tmpl=tmpl->parent_doc;
  }
  tmpl=doc;
  if (y>win_task->win_height)
    y=win_task->win_height;
  WinScrollNull(win_task,&saved_scroll);
  while (tmpl && tmpl->doc_signature==DOC_SIGNATURE_VAL) {
    ch=tmpl->desc;
    i=(Bsr(ch)+7)>>3;	//StrLen
    ch=EndianI64(ch<<((8-i)<<3));
    attr=win_task->border_attr<<8;
    while (i-- && y>0) {
      TextChar(win_task,TRUE,-1,--y,attr+ch&0xFF);
      ch>>=8;
    }
    y--;
    tmpl=tmpl->parent_doc;
  }
  WinScrollRestore(win_task,&saved_scroll);
}

public U0 DocTop(CDoc *doc=NULL)
{//Move cursor, cur_entry, to top.
  Bool unlock;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  doc->cur_entry=doc->head.next;
  doc->cur_col=doc->cur_entry->min_col;
  doc->x=0;
  doc->y=0;
  doc->line_start_col=0;
  doc->top_line_num=0;

  DocFormFwd(doc);
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 DocCenter(CDoc *doc=NULL,I64 recalc_flags=RECALCt_NORMAL)
{//Center win on doc cursor, cur_entry.
  Bool unlock;
  CTask *task;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  task=doc->win_task;
  DocRecalc(doc,recalc_flags);
  if (!(doc->flags&DOCF_BORDER_DOC))
    doc->top_line_num=doc->y-(task->win_height+1)>>1;
  if (unlock)
    DocUnlock(doc);
}

public U0 DocBottom(CDoc *doc=NULL)
{//Move cursor, cur_entry, to bottom.
  Bool unlock;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  doc->cur_entry=doc;
  doc->cur_col=0;
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 DocClear(CDoc *doc=NULL,Bool clear_holds=FALSE)
{//Clear all doc entries, except +H, hold entries.
  Bool unlock;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  DocBottom(doc);
  if (clear_holds)
    DocPrint(doc,"$$CL+H$$");
  else
    DocPrint(doc,"$$CL$$");
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public Bool DocCursor(Bool show=OFF,CDoc *doc=NULL)
{//Show or hide cursor.
  if (!doc && !(doc=DocPut))
    return FALSE;
  return !LBEqu(&doc->flags,DOCf_HIDE_CURSOR,!show);
}

public Bool DocHighlightCursor(Bool show=OFF,CDoc *doc=NULL)
{//Highlight or Don't highlight cursor.
  if (!doc && !(doc=DocPut))
    return FALSE;
  return !LBEqu(&doc->flags,DOCf_DONT_HIGHLIGHT_CURSOR,!show);
}

public Bool DocScroll(Bool val=OFF,CDoc *doc=NULL)
{//Turn scroll bars OFF/ON.
  if (!doc && !(doc=DocPut))
    return FALSE;
  return !LBEqu(&doc->flags,DOCf_NO_SCROLL_BARS,!val);
}

public U0 DocCollapse(Bool collapse=TRUE,CDoc *doc=NULL)
{//Collapse or uncollapse all tree widgets.
  CDocEntry *doc_e;
  Bool unlock;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  doc_e=doc->head.next;
  while (doc_e!=doc) {
    if (doc_e->de_flags&DOCEF_TREE)
      BEqu(&doc_e->de_flags,DOCEf_CHECKED_COLLAPSED,collapse);
    doc_e=doc_e->next;
  }
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

#help_index "DolDoc/Cmd Line (Typically);Cmd Line (Typically)"
public I64 DocMax(I64 i=I64_MAX)
{//Set max document entries. (Cmd line buffer size.)
//Adjusts the size of the cmd line buf.
  //Normally, the cmd line deletes entries
  //when more are added and the old scroll up.
  //See $LK,"max_entries",A="FF:::/Adam/DolDoc/DocTerm.HC,max_entries"$.
  I64 res;
  CDoc *doc;
  if (doc=DocPut) {
    res=doc->max_entries;
    doc->max_entries=i;
    return res;
  } else
    return 0;
}

#help_index "DolDoc/Task;StdOut/Task"
U0 DocUpdateTaskDocs(CTask *task)
{//This is called from $LK,"GrUpdateTaskWin",A="MN:GrUpdateTaskWin"$() by the winmgr at 30fps.
  CDoc *doc;
  CD3I64 saved_scroll;
  if (task->border_src==BDS_CUR_DRV && task->cur_dv)
    task->border_attr=DrvTextAttrGet(Drv2Let(task->cur_dv));
  if (task->title_src==TTS_TASK_NAME)
    StrCpy(task->task_title,task->task_name);
  if ((doc=DocDisplay(task)) && !(doc->flags&DOCF_DONT_SHOW)) {
    if (task->border_src==BDS_ED_FILENAME_DRV)
      task->border_attr=DrvTextAttrGet(*doc->filename.name);
    if (task->title_src==TTS_ED_FILENAME)
      MemCpy(task->task_title,doc->filename.name,STR_LEN-1);
    DocRecalc(doc,RECALCt_TO_SCRN|RECALCF_HAS_CURSOR);
  }
  if ((doc=DocBorder(task)) && !(doc->flags&DOCF_DONT_SHOW)) {
    WinScrollNull(task,&saved_scroll);
    DocRecalc(doc,RECALCt_TO_SCRN);
    WinScrollRestore(task,&saved_scroll);
  }
}
