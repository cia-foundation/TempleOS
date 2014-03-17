#help_index "Doc"

public Bool DocLock(CDoc *doc)
{
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
{
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


CDocBin *DocBinFindNum(CDoc *haystack_doc,I64 needle_num)
{
  CDocBin *b=haystack_doc->bin_root.next;
  while (b!=&haystack_doc->bin_root) {
    if (b->num==needle_num)
      return b;
    b=b->next;
  }
  return NULL;
}

CDocBin *DocBinFindTag(CDoc *haystack_doc,U8 *needle_tag)
{
  CDocBin *b;
  if (needle_tag) {
    b=haystack_doc->bin_root.next;
    while (b!=&haystack_doc->bin_root) {
      if (b->tag && !StrCmp(b->tag,needle_tag))
	return b;
      b=b->next;
    }
  }
  return NULL;
}

U0 DocBinsValidate(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  CDocBin *b,*b1;
  CDocEntry *doc_e,*doc_e2;
  I64 renum_num=0;
  b=doc->bin_root.next;
  while (b!=&doc->bin_root) {
    b->use_cnt=0;
    b->temp_use_cnt=0;
    b->renum_num=-1;
    Free(b->tag);
    b->tag=NULL;
    b=b->next;
  }
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    doc_e2=doc_e->next;
    if (doc_e->flags1 & DOCEF1_HAS_BIN) {
      if (b=doc_e->bin_data=DocBinFindNum(doc,doc_e->bin_num)) {
	if (doc_e->flags1 & DOCEF1_BIN_PTR_LINK)
	  b->temp_use_cnt=MAX_I32;
	if (!b->use_cnt++)
	  b->renum_num=++renum_num;
	doc_e->bin_num=b->renum_num;
	if (!b->tag && doc_e->flags1&DOCEF1_TAG && doc_e->tag && *doc_e->tag)
	  b->tag=StrNew(doc_e->tag,doc->mem_task);
      } else {
	RawPrint(3000,"Bin Not Found");
	doc_e->type=doc_e->flags1=0;
	doc_e->type_u8=DOCT_ERROR;
      }
    }
    doc_e=doc_e2;
  }

  b=doc->bin_root.next;
  doc->cur_bin_num=1;
  while (b!=&doc->bin_root) {
    b1=b->next;
    if (!b->use_cnt) {
      QueRem(b);
      Free(b->data);
      Free(b);
    } else {
      b->num=b->renum_num;
      if (b->num>=doc->cur_bin_num)
	doc->cur_bin_num=b->num+1;
    }
    b=b1;
  }
  if (unlock)
    DocUnlock(doc);
}


U0 DocBinDel(CDoc *doc,CDocBin *b)
{
  if (doc && b && b->use_cnt) {
    b->use_cnt--;
    if (!b->use_cnt) {
      QueRem(b);
      Free(b->tag);
      Free(b->data);
      Free(b);
    }
  } else
    RawPrint(3000,"DocBinDel");
}

Bool IsEditableText(CDocEntry *doc_e)
{
  if (doc_e->type_u8==DOCT_TEXT&&!(doc_e->flags1&DOCEG1_DONT_EDIT))
    return TRUE;
  else
    return FALSE;
}

CDocEntry *DocEntryNew(CDoc *doc,CDocEntry *doc_ce,U8 *tag)
{
  CDocEntry *result=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
  result->type=doc_ce->type;
  result->flags1=doc_ce->flags1;
  result->tag=tag;
  result->max_col=StrLen(tag);
  result->x=doc_ce->x;
  result->y=doc_ce->y;
  result->page_line_num=doc_ce->page_line_num;
  MemCpy(&result->settings,&doc_ce->settings,sizeof(CDocSettings));
  return result;
}

public U0 DocEntryDel(CDoc *doc,CDocEntry *doc_e)
{
  if (!doc || doc==doc_e)
    RawPrint(3000,"DocEntryDel");
  else {
    if (doc->cur_entry==doc_e)
      doc->cur_entry=doc_e->next;
    QueRem(doc_e);
    if (doc_e->flags1 & DOCEF1_TAG)
      Free(doc_e->tag);
    if (doc_e->flags1 & DOCEF1_LEFT_MACRO)
      Free(doc_e->left_macro);
    if (doc_e->flags1 & DOCEF1_RIGHT_MACRO)
      Free(doc_e->right_macro);
    if (doc_e->flags1 & DOCEF1_AUX_STR)
      Free(doc_e->aux_str);
    if (doc_e->flags1 & DOCEF1_DEFINE)
      Free(doc_e->define_str);
    if (doc_e->flags1 & DOCEF1_HTML_LINK)
      Free(doc_e->html_link);
    if (doc_e->flags1 & DOCEF1_BIN_PTR_LINK)
      Free(doc_e->bin_ptr_link);
    if (doc_e->flags1 & DOCEF1_HAS_BIN)
      DocBinDel(doc,doc_e->bin_data);
    Free(doc_e);
  }
}

public I64 DocEntrySize(CDoc *,CDocEntry *doc_e)
{
  I64 result;
  if (!doc_e) return 0;
  result=MSize2(doc_e);
  if (doc_e->flags1 & DOCEF1_TAG)
    result+=MSize2(doc_e->tag);
  if (doc_e->flags1 & DOCEF1_AUX_STR)
    result+=MSize2(doc_e->aux_str);
  if (doc_e->flags1 & DOCEF1_DEFINE)
    result+=MSize2(doc_e->define_str);
  if (doc_e->flags1 & DOCEF1_HTML_LINK)
    result+=MSize2(doc_e->html_link);
  if (doc_e->flags1 & DOCEF1_BIN_PTR_LINK)
    result+=MSize2(doc_e->bin_ptr_link);
  if (doc_e->flags1 & DOCEF1_LEFT_MACRO)
    result+=MSize2(doc_e->left_macro);
  if (doc_e->flags1 & DOCEF1_RIGHT_MACRO)
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
{
  CDocEntry *doc_ne;
  CDocBin *tempb;
  CTask *task=doc->mem_task;
  doc_ne=MAllocIdentical(doc_e,task);
  doc_ne->next=doc_ne;
  doc_ne->last=doc_ne;
  if (doc_e->flags1 & DOCEF1_TAG)
    doc_ne->tag=MAllocIdentical(doc_e->tag,task);
  if (doc_e->flags1 & DOCEF1_AUX_STR)
    doc_ne->aux_str=MAllocIdentical(doc_e->aux_str,task);
  if (doc_e->flags1 & DOCEF1_DEFINE)
    doc_ne->define_str=MAllocIdentical(doc_e->define_str,task);
  if (doc_e->flags1 & DOCEF1_HTML_LINK)
    doc_ne->html_link=MAllocIdentical(doc_e->html_link,task);
  if (doc_e->flags1 & DOCEF1_BIN_PTR_LINK)
    doc_ne->bin_ptr_link=MAllocIdentical(doc_e->bin_ptr_link,task);
  if (doc_e->flags1 & DOCEF1_LEFT_MACRO)
    doc_ne->left_macro=MAllocIdentical(doc_e->left_macro,task);
  if (doc_e->flags1 & DOCEF1_RIGHT_MACRO)
    doc_ne->right_macro=MAllocIdentical(doc_e->right_macro,task);
  if (doc_e->flags1 & DOCEF1_HAS_BIN) {
    tempb=MAllocIdentical(doc_e->bin_data,task);
    tempb->data=MAllocIdentical(doc_e->bin_data->data,task);
    doc_ne->bin_num=doc->cur_bin_num;
    tempb->num=doc->cur_bin_num++;
    doc_ne->bin_data=tempb;
    if (doc_e->flags1&DOCEF1_TAG && doc_e->tag && *doc_e->tag)
      tempb->tag=StrNew(doc_e->tag,task);
    else
      tempb->tag=NULL;
    QueIns(tempb,doc->bin_root.last);
  }
  return doc_ne;
}

public U8 *DocEntryLink(CDoc *doc,CDocEntry *doc_e)
{
  if (doc_e->flags1&DOCEF1_LINK) {
    if (doc_e->flags1 & DOCEF1_AUX_STR)
      return StrNew(doc_e->aux_str,doc->mem_task);
    else
      return StrNew(doc_e->tag,doc->mem_task);
  }
  return NULL;
}

U0 DocFormFwd(CDoc *doc,Bool giveup=FALSE)
{
  CDocEntry *doc_e=doc->cur_entry,*doc_e2=doc_e;
  if (doc->flags & DOCF_FORM) {
    if (doc_e==doc) goto recover;
    while (!Bt(doc_type_flags_form,doc_e->type_u8) ||
       doc_e->flags1&DOCEF1_SKIP_IN_FORM) {
      doc_e=doc_e->next;
      if (doc_e==doc) {
recover:
	doc->cur_data_col=0;
	if (!giveup) {
	  doc->cur_entry=doc_e->last;
	  DocFormBackward(doc,TRUE);
	} else
	  doc->cur_entry=doc;
	return;
      }
    }
  }
  while (doc_e->type_u8==DOCT_INDENT)
    doc_e=doc_e->next;
  if (doc_e!=doc_e2) {
    doc->cur_data_col=doc_e->min_col;
    doc->cur_entry=doc_e;
  }
}

U0 DocFormBackward(CDoc *doc,Bool giveup=FALSE)
{
  CDocEntry *doc_e=doc->cur_entry,*doc_e2=doc_e;
  if (doc->flags & DOCF_FORM) {
    while (!Bt(doc_type_flags_form,doc_e->type_u8) ||
       doc_e->flags1&DOCEF1_SKIP_IN_FORM) {
      doc_e=doc_e->last;
      if (doc_e==doc) {
	doc->cur_data_col=0;
	if (!giveup) {
	  doc->cur_entry=doc_e->next;
	  DocFormFwd(doc,TRUE);
	} else
	  doc->cur_entry=doc;
	return;
      }
    }
  }
  while (doc_e->type_u8==DOCT_INDENT)
    doc_e=doc_e->next;
  if (doc_e!=doc_e2) {
    doc->cur_data_col=doc_e->min_col;
    doc->cur_entry=doc_e;
  }
}

public U0 DocHome(CDoc *doc=NULL)
{
  Bool unlock;
  if (!doc && !(doc=DocPut))
    return;
  unlock=DocLock(doc);
  doc->cur_entry=doc->root.next;
  doc->cur_data_col=doc->cur_entry->min_col;
  doc->x=0;
  doc->y=0;
  doc->line_start_col=0;
  doc->cur_top_line_num=0;

  DocFormFwd(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 DocRst(CDoc *doc,Bool is_old)
{
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
  doc->recalc_start=doc;
  doc->dollar_buf_ptr=0;
  doc->ext_ascii_cnt=doc->ext_ascii_val=0;
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
  doc_e->flags1=0;
  doc_e->x=0;
  doc_e->y=0;
  doc_e->min_col=0;
  doc_e->max_col=0;
  doc_e->page_line_num=doc->page_line_num;
  MemCpy(&doc_e->settings,s,sizeof(CDocSettings));

  DocHome(doc);
  if (unlock)
    DocUnlock(doc);
}


public U0 DocDel(CDoc *doc)
{
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
{
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


U0 DocDataFmt(CDocEntry *doc_e)
{
  I64 i,d;
  U8 *ptr;
  if (doc_e->type_u8==DOCT_DATA || doc_e->type_u8==DOCT_CHECK_BOX) {
    if (doc_e->flags1 & DOCEF1_DATA_IS_PTR) {
      if (!(ptr=doc_e->data)) return;
    } else
      ptr=&doc_e->data;
    if (doc_e->flags1&DOCEF1_U8)
      d=*ptr;
    else if (doc_e->flags1&DOCEF1_U16)
      d=*ptr(U16 *);
    else if (doc_e->flags1&DOCEF1_U32)
      d=*ptr(U32 *);
    else
      d=*ptr(I64 *);
    if (doc_e->type_u8==DOCT_DATA) {
      SPrint(doc_e->tag,doc_e->aux_str,d,doc_e->my_fmt_data);
      if (doc_e->flags1 & DOCEF1_HAS_TERMINATOR) {
	i=StrLen(doc_e->tag);
	doc_e->tag[i++]='_';
	doc_e->tag[i]=0;
      }
      doc_e->max_col=StrLen(doc_e->tag);
    } else {
      if (d)
	doc_e->flags1|=DOCEF1_CHECKED_COLLAPSED;
      else
	doc_e->flags1&=~DOCEF1_CHECKED_COLLAPSED;
    }
  }
}

U0 DocDataScan(CDocEntry *doc_e)
{
  I64 i,d;
  U8 *ptr;
  if (doc_e->type_u8==DOCT_DATA || doc_e->type_u8==DOCT_CHECK_BOX) {
    if (doc_e->flags1 & DOCEF1_DATA_IS_PTR) {
      if (!(ptr=doc_e->data)) return;
    } else
      ptr=&doc_e->data;
    if (doc_e->type_u8==DOCT_DATA) {
      i=StrLen(doc_e->tag);
      if (doc_e->flags1 & DOCEF1_HAS_TERMINATOR)
	doc_e->tag[--i]=0;
      if (i>doc_e->len+doc_e->min_col)
	doc_e->tag[doc_e->len+doc_e->min_col]=0;
      if (doc_e->flags1&(DOCEF1_U8|DOCEF1_U16|DOCEF1_U32)) {
	Scan(doc_e->tag,doc_e->aux_str,&d,doc_e->my_fmt_data);
	if (doc_e->flags1 & DOCEF1_HAS_TERMINATOR)
	  doc_e->tag[i]='_';
      } else {
	Scan(doc_e->tag,doc_e->aux_str,ptr,doc_e->my_fmt_data);
	if (doc_e->flags1 & DOCEF1_HAS_TERMINATOR)
	  doc_e->tag[i]='_';
	return;
      }
    } else {
      if (doc_e->flags1 & DOCEF1_CHECKED_COLLAPSED)
	d=TRUE;
      else
	d=FALSE;
    }
    if (doc_e->flags1&DOCEF1_U8)
      *ptr=d;
    else if (doc_e->flags1&DOCEF1_U16)
      *ptr(U16 *)=d;
    else if (doc_e->flags1&DOCEF1_U32)
      *ptr(U32 *)=d;
    else
      *ptr(I64 *)=d;
  }
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
	doc->cur_data_col=doc->cur_entry->min_col;
      }
      DocEntryDel(doc,doc_e);
    } else if (saved_ll && doc_e->type_u8==DOCT_NEW_LINE)
      break;
    doc_e=doc_e2;
  }
  if (unlock)
    DocUnlock(doc);
}

#help_index "Doc/Cmd Line (Typically);Cmd Line (Typically)"

public I64 DocMax(I64 i=MAX_I32)
//Adjusts the size of the Doc buf.
//Normally, the cmd line deletes
//entries after a while.
{
  I64 result;
  CDoc *doc;
  if (doc=DocPut) {
    result=doc->max_entries;
    doc->max_entries=i;
    return result;
  } else
    return 0;
}

#help_index "Doc"
public CDoc *DocNew(U8 *filename=NULL,CTask *task=NULL)
{
  CDoc *doc;
  if (!task) task=Fs;
  doc=CAlloc(sizeof(CDoc),task);
  if (filename)
    StrCpy(doc->filename.name,filename);
  else
    StrCpy(doc->filename.name,sys_temp_filename);
  doc->find_replace=CAlloc(sizeof(CEdFindText),task);
  doc->find_replace->scan_fwd=TRUE;
  doc->find_replace->match_case=TRUE;
  doc->find_replace->pmt=TRUE;
  doc->left_click_link=&EdLeftClickLink;
  doc->dollar_buf_size=84;
  doc->dollar_buf=MAlloc(doc->dollar_buf_size,task);
  doc->max_entries=MAX_I32;
  doc->ww_lines_back=32;
  doc->win_task=task;
  doc->mem_task=task;
  DocRst(doc,FALSE);
  doc->doc_signature=DOC_SIGNATURE_VAL;
  return doc;
}

sys_clipboard_doc=DocNew;

#help_index "Doc/Task"
public CDoc *DocPut(CTask *task=NULL)
{//Current document that PUT goes to.
 //Basically, StdOut unless double buffering.
  CDoc *result;
  if (!task) task=Fs;
  if ((result=task->put_doc) && result->doc_signature==DOC_SIGNATURE_VAL)
    return result;
  else
    return NULL;
}

public CDoc *DocDisplay(CTask *task=NULL)
{//StdOut unless double buffering.
  CDoc *result;
  if (!task) task=Fs;
  if ((result=task->display_doc) && result->doc_signature==DOC_SIGNATURE_VAL)
    return result;
  else
    return NULL;
}

public CDoc *DocBorder(CTask *task=NULL)
{//Doc holding border of window text.
  CDoc *result;
  if (!task) task=Fs;
  if ((result=task->border_doc) && result->doc_signature==DOC_SIGNATURE_VAL)
    return result;
  else
    return NULL;
}


public CDoc *DocDblBufStart(CTask *task=NULL)
{ //See $LK,"::/Demo/Spy.CPP"$
  Bool unlock_ddoc,old_preempt=Preempt(OFF);
  CDoc *pdoc=DocPut(task),*ddoc=DocDisplay(task),*result;
  if (!pdoc || !ddoc || pdoc!=ddoc)
    result=NULL; //Already Double buffering
  else {
    if (!task) task=Fs;
    unlock_ddoc=DocLock(ddoc); //dont want change during winupdate, so lock DocPut$WW,0$
    result=DocNew(,task);
    result->win_task		=ddoc->win_task;
    result->max_entries		=ddoc->max_entries;
    MemCpy(result->find_replace,ddoc->find_replace,sizeof(CEdFindText));
    result->start_x		=ddoc->start_x;
    result->start_y		=ddoc->start_y;
    MemCpy(&result->filename,&ddoc->filename,sizeof(CEdFileName));
    result->left_click_link	=ddoc->left_click_link;
    result->right_click_link	=ddoc->right_click_link;
    result->user_put_data	=ddoc->user_put_data;
    result->user_put_key	=ddoc->user_put_key;
    result->user_put_s		=ddoc->user_put_s;
    result->parent_doc		=ddoc->parent_doc;
    result->desc[0](I64)	=ddoc->desc[0](I64);
    result->user_data		=ddoc->user_data;
    result->flags|=ddoc->flags&DOCG_DBL_BUF_FLAGS | DOCF_DONT_SHOW;
    task->put_doc=result;
    if (unlock_ddoc)
      DocUnlock(ddoc);
  }
  Preempt(old_preempt);
  return result;
}

public Bool DocDblBufEnd(CTask *task=NULL)
{ //See $LK,"::/Demo/Spy.CPP"$
  Bool result=FALSE,old_preempt=Preempt(OFF);
  CDoc *pdoc=DocPut(task),*ddoc=DocDisplay(task);
  if (pdoc && ddoc && pdoc!=ddoc) {//Double buffering?
    if (!task) task=Fs;
    ddoc->flags|=DOCF_DONT_SHOW;
    pdoc->flags&=~DOCF_DONT_SHOW;
    DocLock(ddoc);
    task->display_doc=pdoc;
    DocUnlock(ddoc);
    DocDel(ddoc);
    result=TRUE;
  }
  Preempt(old_preempt);
  return result;
}

public Bool DocDblBufSwap(CTask *task=NULL)
{ //See $LK,"::/Demo/Spy.CPP"$
  Bool result=FALSE,old_preempt=Preempt(OFF);
  CDoc *pdoc=DocPut(task),*ddoc=DocDisplay(task);
  if (pdoc && ddoc && pdoc!=ddoc) {//Double buffering?
    if (!task) task=Fs;
    ddoc->flags|=DOCF_DONT_SHOW;
    pdoc->flags&=~DOCF_DONT_SHOW;
    DocLock(ddoc);
    task->display_doc=pdoc;
    DocUnlock(ddoc);
    DocRst(ddoc,TRUE);
    MemCpy(ddoc->find_replace,pdoc->find_replace,sizeof(CEdFindText));
    MemCpy(&ddoc->filename,&pdoc->filename,sizeof(CEdFileName));
    ddoc->max_entries		=pdoc->max_entries;
    ddoc->flags			=pdoc->flags&DOCG_DBL_BUF_FLAGS | ddoc->flags&~DOCG_DBL_BUF_FLAGS;
    ddoc->left_click_link	=pdoc->left_click_link;
    ddoc->right_click_link	=pdoc->right_click_link;
    ddoc->user_put_data		=pdoc->user_put_data;
    ddoc->user_put_key		=pdoc->user_put_key;
    ddoc->user_put_s		=pdoc->user_put_s;
    ddoc->desc[0](I64)		=pdoc->desc[0](I64);
    ddoc->user_data		=pdoc->user_data;
    task->put_doc=ddoc;
    result=TRUE;
  }
  Preempt(old_preempt);
  return result;
}
