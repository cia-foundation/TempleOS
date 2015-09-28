#help_index "DolDoc"

public Bool DocLock(CDoc *doc)
{//Make this task have exclusive access to this doc.
  if (!Bt(&doc->locked_flags,DOClf_LOCKED) || doc->owning_task!=Fs) {
    while (LBts(&doc->locked_flags,DOClf_LOCKED))
      Yield;
    if (doc->owning_task!=Fs)
      LBEqu(&doc->flags,DOCf_BREAK_UNLOCKED,BreakLock(Fs));
    doc->owning_task=Fs;
    return TRUE;
  } else
    return FALSE;
}

public Bool DocUnlock(CDoc *doc)
{//Release exclusive lock on access to doc.
  Bool unlock_break;
  if (Bt(&doc->locked_flags,DOClf_LOCKED) && doc->owning_task==Fs) {
    doc->owning_task=0;
    unlock_break=Bt(&doc->flags,DOCf_BREAK_UNLOCKED);
    LBtr(&doc->locked_flags,DOClf_LOCKED);
    if (unlock_break)
      BreakUnlock(Fs);
    return TRUE;
  } else
    return FALSE;
}

Bool IsEditableText(CDocEntry *doc_e)
{
  if (doc_e->type_u8==DOCT_TEXT&&!(doc_e->de_flags&DOCEG1_DONT_EDIT))
    return TRUE;
  else
    return FALSE;
}

CDocEntry *DocEntryNew(CDoc *doc,CDocEntry *doc_ce,U8 *tag)
{
  CDocEntry *result=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
  result->type=doc_ce->type;
  result->de_flags=doc_ce->de_flags;
  result->tag=tag;
  result->max_col=StrLen(tag);
  result->x=doc_ce->x;
  result->y=doc_ce->y;
  result->page_line_num=doc_ce->page_line_num;
  MemCpy(&result->settings,&doc_ce->settings,sizeof(CDocSettings));
  return result;
}

public U0 DocEntryDel(CDoc *doc,CDocEntry *doc_e)
{//Free entry and all parts of entry.
  if (!doc || doc==doc_e)
    RawPrint(3000,"DocEntryDel");
  else {
    if (doc->cur_entry==doc_e)
      doc->cur_entry=doc_e->next;
    QueRem(doc_e);
    if (doc_e->de_flags & DOCEF_TAG)
      Free(doc_e->tag);
    if (doc_e->de_flags & DOCEF_LEFT_MACRO)
      Free(doc_e->left_macro);
    if (doc_e->de_flags & DOCEF_RIGHT_MACRO)
      Free(doc_e->right_macro);
    if (doc_e->de_flags & DOCEF_AUX_STR)
      Free(doc_e->aux_str);
    if (doc_e->de_flags & DOCEF_DEFINE)
      Free(doc_e->define_str);
    if (doc_e->de_flags & DOCEF_HTML_LINK)
      Free(doc_e->html_link);
    if (doc_e->de_flags & DOCEF_BIN_PTR_LINK)
      Free(doc_e->bin_ptr_link);
    if (doc_e->de_flags & DOCEF_HAS_BIN)
      DocBinDel(doc,doc_e->bin_data);
    Free(doc_e);
  }
}

public I64 DocEntrySize(CDoc *,CDocEntry *doc_e)
{//Mem size of entry and all parts.
  I64 result;
  if (!doc_e) return 0;
  result=MSize2(doc_e);
  if (doc_e->de_flags & DOCEF_TAG)
    result+=MSize2(doc_e->tag);
  if (doc_e->de_flags & DOCEF_AUX_STR)
    result+=MSize2(doc_e->aux_str);
  if (doc_e->de_flags & DOCEF_DEFINE)
    result+=MSize2(doc_e->define_str);
  if (doc_e->de_flags & DOCEF_HTML_LINK)
    result+=MSize2(doc_e->html_link);
  if (doc_e->de_flags & DOCEF_BIN_PTR_LINK)
    result+=MSize2(doc_e->bin_ptr_link);
  if (doc_e->de_flags & DOCEF_LEFT_MACRO)
    result+=MSize2(doc_e->left_macro);
  if (doc_e->de_flags & DOCEF_RIGHT_MACRO)
    result+=MSize2(doc_e->right_macro);
  return result;
}

U0 DocUndoDel(CDoc *,CDocUndo *u)
{
  Free(u->body);
  Free(u);
}

U0 DocUndoCntSet(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  CDocUndo *u=doc->undo_root.next;
  doc->undo_cnt=0;
  while (u!=&doc->undo_root) {
    doc->undo_cnt++;
    u=u->next;
  }
  if (unlock)
    DocUnlock(doc);
}

public CDocEntry *DocEntryCopy(CDoc *doc,CDocEntry *doc_e)
{//Make copy of entry and all parts of entry.
  CDocEntry *doc_ne;
  CDocBin *tempb;
  CTask *task=doc->mem_task;
  doc_ne=MAllocIdentical(doc_e,task);
  doc_ne->next=doc_ne;
  doc_ne->last=doc_ne;
  if (doc_e->de_flags & DOCEF_TAG)
    doc_ne->tag=MAllocIdentical(doc_e->tag,task);
  if (doc_e->de_flags & DOCEF_AUX_STR)
    doc_ne->aux_str=MAllocIdentical(doc_e->aux_str,task);
  if (doc_e->de_flags & DOCEF_DEFINE)
    doc_ne->define_str=MAllocIdentical(doc_e->define_str,task);
  if (doc_e->de_flags & DOCEF_HTML_LINK)
    doc_ne->html_link=MAllocIdentical(doc_e->html_link,task);
  if (doc_e->de_flags & DOCEF_BIN_PTR_LINK)
    doc_ne->bin_ptr_link=MAllocIdentical(doc_e->bin_ptr_link,task);
  if (doc_e->de_flags & DOCEF_LEFT_MACRO)
    doc_ne->left_macro=MAllocIdentical(doc_e->left_macro,task);
  if (doc_e->de_flags & DOCEF_RIGHT_MACRO)
    doc_ne->right_macro=MAllocIdentical(doc_e->right_macro,task);
  if (doc_e->de_flags & DOCEF_HAS_BIN) {
    tempb=MAllocIdentical(doc_e->bin_data,task);
    tempb->data=MAllocIdentical(doc_e->bin_data->data,task);
    doc_ne->bin_num=doc->cur_bin_num;
    tempb->num=doc->cur_bin_num++;
    doc_ne->bin_data=tempb;
    if (doc_e->de_flags&DOCEF_TAG && doc_e->tag && *doc_e->tag)
      tempb->tag=StrNew(doc_e->tag,task);
    else
      tempb->tag=NULL;
    QueIns(tempb,doc->bin_root.last);
  }
  return doc_ne;
}

U0 DocRemSoftNewLines(CDoc *doc=NULL,CDocEntry *doc_e=NULL)
{
  CDocEntry *doc_e2,*saved_ll=doc_e;
  Bool unlock;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  if (!doc_e) doc_e=doc->root.next;
  while (doc_e!=doc) {
    doc_e2=doc_e->next;
    if (doc_e->type_u8==DOCT_SOFT_NEW_LINE) {
      if (doc->cur_entry==doc_e) {
	doc->cur_entry=doc_e2;
	doc->cur_col=doc->cur_entry->min_col;
      }
      DocEntryDel(doc,doc_e);
    } else if (saved_ll && doc_e->type_u8==DOCT_NEW_LINE)
      break;
    doc_e=doc_e2;
  }
  if (unlock)
    DocUnlock(doc);
}

public U0 DocInsEntry(CDoc *doc,CDocEntry *doc_e)
{//Insert entry into doc, updating its vals.
  U8 *dst;
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->cur_entry,*doc_ne;

  doc_e->x=doc_ce->x;
  doc_e->y=doc_ce->y;
  doc_e->page_line_num=doc_ce->page_line_num;
  MemCpy(&doc_e->settings,&doc_ce->settings,sizeof(CDocSettings));
  if (doc->cur_col>0 &&
	doc_ce->type_u8==DOCT_TEXT &&
	!(doc_ce->de_flags&(DOCEF_TAG_CB|DOCEF_DEFINE|DOCEF_AUX_STR|
	DOCEF_HTML_LINK|DOCEF_BIN_PTR_LINK)) &&
	doc->cur_col<doc_ce->max_col) {
    dst=doc_ce->tag+doc->cur_col;
    doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
    *dst=0;
    doc_ne->type=DOCT_TEXT|doc_ce->type&0xFFFFFF00;
    doc_ce->max_col=doc->cur_col;
    QueIns(doc_ne,doc_ce);
    doc->cur_col=0;
    doc_ce=doc_ne;
  }
  if (doc_ce->type_u8==DOCT_TEXT && doc->cur_col>=doc_ce->max_col) {
    QueIns(doc_e,doc_ce);
    doc->cur_entry=doc_e->next;
  } else {
    QueIns(doc_e,doc_ce->last);
    doc->cur_entry=doc_ce;
  }
  doc->cur_col=doc->cur_entry->min_col;
  DocRemSoftNewLines(doc,doc->cur_entry);
  if (unlock)
    DocUnlock(doc);
}

public U0 DocRst(CDoc *doc,Bool is_old)
{//Del all entries and set doc to dfts.
  Bool unlock;
  CDocEntry *doc_e,*doc_e2;
  CDocUndo *u,*u8;
  CDocSettings *s;
  CDocBin *b,*b1;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  if (is_old) {
    doc_e=doc->root.next;
    while (doc_e!=doc) {
      doc_e2=doc_e->next;
      DocEntryDel(doc,doc_e);
      doc_e=doc_e2;
    }
    u=doc->undo_root.next;
    while (u!=&doc->undo_root) {
      u8=u->next;
      DocUndoDel(doc,u);
      u=u8;
    }
    b=doc->bin_root.next;
    while (b!=&doc->bin_root) {
      b1=b->next;
      QueRem(b);
      Free(b->data);
      Free(b);
      b=b1;
    }
  }
//Check $LK,"DocInsDoc",A="MN:DocInsDoc"$
  doc->flags&=DOCF_BREAK_UNLOCKED;
  doc->root.next=doc->root.last=doc;
  QueInit(&doc->bin_root);
  QueInit(&doc->undo_root);
  doc->undo_root.time_stamp=0;
  doc->undo_cnt=0;
  doc->cur_bin_num=1;
  doc->dollar_buf_ptr=0;
  doc->cmd_U8=CH_SPACE;
  doc->page_line_num=0;
  doc->best_d=MAX_I64;

  s=&doc->settings_root;
  s->left_margin=DOC_DFT;
  s->right_margin=DOC_DFT;
  s->indent=0;
  s->page_len=66;
  s->header=DOC_DFT;
  s->footer=DOC_DFT;
  s->state=DOCSS_NORMAL;
  s->comment_depth=0;
  s->paren_depth=0;
  s->brace_depth=0;
  s->shifted_x=0;
  s->shifted_y=0;
  s->cur_text_attr=s->dft_text_attr=DOC_ATTR_DFT_TEXT;

  doc_e=&doc->root;
  doc_e->type=DOCT_ERROR;
  doc_e->de_flags=0;
  doc_e->x=0;
  doc_e->y=0;
  doc_e->min_col=0;
  doc_e->max_col=0;
  doc_e->page_line_num=doc->page_line_num;
  MemCpy(&doc_e->settings,s,sizeof(CDocSettings));

  DocTop(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 DocDel(CDoc *doc)
{//Free entire doc and entries.
  if (!doc || doc->doc_signature!=DOC_SIGNATURE_VAL) return;
  DocLock(doc);
  doc->doc_signature=0;
  DocRst(doc,TRUE);
  Free(doc->find_replace);
  Free(doc->dollar_buf);
  DocUnlock(doc);
  Free(doc);
}

public I64 DocSize(CDoc *doc)
{//Mem size of doc and all its entries.
  Bool unlock;
  CDocEntry *doc_e;
  CDocUndo *u;
  CDocBin *b;
  I64 result=0;

  if (!doc || doc->doc_signature!=DOC_SIGNATURE_VAL) return 0;
  unlock=DocLock(doc);

  doc_e=doc->root.next;
  while (doc_e!=doc) {
    result+=DocEntrySize(doc,doc_e);
    doc_e=doc_e->next;
  }

  u=doc->undo_root.next;
  while (u!=&doc->undo_root) {
    result+=MSize2(u->body);
    result+=MSize2(u);
    u=u->next;
  }

  b=doc->bin_root.next;
  while (b!=&doc->bin_root) {
    result+=MSize2(b->data);
    result+=MSize2(b);
    b=b->next;
  }

  result+=MSize2(doc->find_replace);
  result+=MSize2(doc->dollar_buf);
  result+=MSize2(doc);
  if (unlock)
    DocUnlock(doc);
  return result;
}

#help_index "DolDoc"
public CDoc *DocNew(U8 *filename=NULL,CTask *task=NULL)
{//MAlloc new $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.TXT"$. (Begin a new doc.)
  CDoc *doc;
  if (!task) task=Fs;
  doc=CAlloc(sizeof(CDoc),task);
  if (filename)
    StrCpy(doc->filename.name,filename);
  else
    StrCpy(doc->filename.name,blkdev.temp_filename);
  doc->find_replace=CAlloc(sizeof(CEdFindText),task);
  doc->find_replace->scan_fwd=TRUE;
  doc->find_replace->match_case=TRUE;
  doc->find_replace->pmt=TRUE;
  doc->left_click_link=&EdLeftClickLink;
  doc->dollar_buf_size=84;
  doc->dollar_buf=MAlloc(doc->dollar_buf_size,task);
  doc->max_entries=MAX_I64;
  doc->win_task=task;
  doc->mem_task=task;
  DocRst(doc,FALSE);
  doc->doc_signature=DOC_SIGNATURE_VAL;
  return doc;
}
