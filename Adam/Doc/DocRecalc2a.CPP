#help_index "Doc"

I64 DocCharDist(CDoc *doc,I64 x,I64 y)
{
#assert FONT_WIDTH==FONT_HEIGHT
  return (SqrI64(doc->x-x)+SqrI64(doc->y-y))*FONT_WIDTH*FONT_WIDTH;
}

U0 LockToPage(CDoc *doc)
{
  I64 win_page_len;
  CDocEntry *doc_e;
  doc_e=doc->cur_entry;
  if (doc_e->flags1 & DOCEF1_LOCK_TO_PAGE)  {
    doc->cur_top_line_num=doc_e->y-doc_e->page_line_num;
    win_page_len=doc->win_task->win_height;
    if (doc_e->page_line_num>=win_page_len)
      doc->cur_top_line_num+=doc_e->last->settings.page_len-win_page_len;
  }
}

public U0 DocCenter(CDoc *doc=NULL,I64 recalc=DOC_RECALC_ALL)
{
  Bool unlock;
  CTask *task;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  task=doc->win_task;
  DocRecalc(doc,FALSE,recalc);
  if (!(doc->flags&DOCF_BORDER_DOC))
    doc->cur_top_line_num=doc->y-(task->win_height+1)>>1;
  LockToPage(doc);
  if (unlock)
    DocUnlock(doc);
}

U0 DocDelToNum(CDoc *doc,I64 num)
{
  CDocEntry *doc_e=doc->root.next,*doc_e2;
  while (num-->0 && doc_e!=doc) {
    doc_e2=doc_e->next;
    if (!(doc_e->flags1 & (DOCEF1_HOLD|DOCEF1_FILTER_SKIP))) {
      if (doc_e==doc->cur_entry) {
	doc->cur_entry=doc_e2;
	doc->cur_data_col=doc_e2->min_col;
      }
      DocEntryDel(doc,doc_e);
    }
    doc_e=doc_e2;
  }
}

U0 DocDelToEntry(CDoc *doc,CDocEntry *clear_entry)
{
  CDocEntry *doc_e=doc->root.next,*doc_e2;
  while (doc_e!=doc) {
    doc_e2=doc_e->next;
    if (!(doc_e->flags1 & (DOCEF1_HOLD|DOCEF1_FILTER_SKIP))) {
      if (doc_e==doc->cur_entry) {
	doc->cur_entry=doc_e2;
	doc->cur_data_col=doc_e2->min_col;
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
  CDoc *templ=doc;
  U8 *ptr;
  CD3I64 saved_scroll;
  Bool old_preempt=Preempt(OFF);
  while (templ && templ->doc_signature==DOC_SIGNATURE_VAL) {
    y+=StrLen(templ->desc)+1;
    templ=templ->parent_doc;
  }
  templ=doc;
  if (y>win_task->win_height)
    y=win_task->win_height;
  WinScrollNull(win_task,&saved_scroll);
  while (templ && templ->doc_signature==DOC_SIGNATURE_VAL) {
    ptr=templ->desc;
    i=StrLen(templ->desc);
    ptr+=i;
    attr=win_task->border_attr<<8;
    while (i-- && y>0)
      PlotBorderU8(win_task,attr+*--ptr,-1,--y);
    y--;
    templ=templ->parent_doc;
  }
  WinScrollRestore(win_task,&saved_scroll);
  Preempt(old_preempt);
}

public U0 DocBottom(CDoc *doc=NULL)
{
  Bool unlock;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  doc->cur_entry=doc;
  doc->cur_data_col=0;
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 DocTop(CDoc *doc=NULL)
{
  Bool unlock;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  doc->cur_entry=doc->root.next;
  doc->cur_data_col=0;
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 DocClear(CDoc *doc=NULL)
{
  Bool unlock;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  DocBottom(doc);
  DocPrint(doc,"$$CL$$");
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public Bool DocCursor(Bool show=OFF,CDoc *doc=NULL)
{
  if (!doc && !(doc=DocPut))
    return FALSE;
  return !LBEqu(&doc->flags,DOCf_HIDE_CURSOR,!show);
}

public Bool DocScroll(Bool val=OFF,CDoc *doc=NULL)
{
  if (!doc && !(doc=DocPut))
    return FALSE;
  return !LBEqu(&doc->flags,DOCf_NO_SCROLL_BARS,!val);
}

public U0 DocCollapse(Bool collapse=TRUE,CDoc *doc=NULL)
{
  CDocEntry *doc_e;
  Bool unlock;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->flags1&DOCEF1_TREE)
      BEqu(&doc_e->flags1,DOCEf1_CHECKED_COLLAPSED,collapse);
    doc_e=doc_e->next;
  }
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

#help_index "Doc/Output"
public U0 DocPrintWhole(CDoc *doc,U8 *fmt,...)
{ //This is necessary when hidden text might screw-up
//inserts.  It does multiple whole doc cmds all-at-once.
  U8 *buf;
  Bool unlock;
  I64 old_flags;
  if (!doc && !(doc=DocPut))
    return;
  buf=SPrintJoin(NULL,fmt,argc,argv);
  unlock=DocLock(doc);
  old_flags=doc->flags;
  doc->flags|=DOCF_NO_CURSOR;
  DocPrint(doc,"%s",buf);
  DocRecalc(doc,FALSE,DOC_RECALC_PARTIAL);
  doc->flags=old_flags;
  if (unlock)
    DocUnlock(doc);
  Free(buf);
}

#help_index "Doc/Task"
U0 DocUpdateTaskDocs(CTask *task)
{
  CPrt *p;
  CDoc *doc;
  CD3I64 saved_scroll;
  Bool old_preempt=Preempt(OFF);
  I64 border_attr;
  if (task->border_src==BDS_CUR_PRT && task->cur_prt) {
    border_attr=task->cur_prt->text_attr;
    if (border_attr&15!=border_attr>>4)
      task->border_attr=border_attr;
  }
  if (task->title_src==TTS_TASK_NAME)
    StrCpy(task->task_title,task->task_name);
  if ((doc=DocDisplay(task)) && !(doc->flags&DOCF_DONT_SHOW)) {
    if (task->border_src==BDS_ED_FILENAME_PRT)
      try {
	if (p=Drv2Prt(*doc->filename.name)) {
	  border_attr=p->text_attr;
	  if (border_attr&15!=border_attr>>4)
	    task->border_attr=border_attr;
	}
      } catch
	Fs->catch_except=TRUE;
    if (task->title_src==TTS_ED_FILENAME)
      MemCpy(task->task_title,doc->filename.name,TASK_TITLE_LEN);
    Preempt(old_preempt);
    DocRecalc(doc,TRUE,DOC_RECALC_TO_SCREEN);
    Preempt(OFF);
  }
  if ((doc=DocBorder(task)) && !(doc->flags&DOCF_DONT_SHOW)) {
    WinScrollNull(task,&saved_scroll);
    DocRecalc(doc,FALSE,DOC_RECALC_TO_SCREEN);
    WinScrollRestore(task,&saved_scroll);
  }
  Preempt(old_preempt);
}
