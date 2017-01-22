#help_index "DolDoc/Task;StdOut/Task"
public CDoc *DocPut(CTask *task=NULL)
{//Current document that StdOut Put() goes to.
//Basically, StdOut unless double buffering.
  CDoc *res;
  if (!task) task=Fs;
  if (Bt(&task->task_flags,TASKf_INPUT_FILTER_TASK))
    task=task->parent_task;
  if ((res=task->put_doc) && res->doc_signature==DOC_SIGNATURE_VAL)
    return res;
  else
    return NULL;
}

public CDoc *DocDisplay(CTask *task=NULL)
{//StdOut displayed unless double buffering.
  CDoc *res;
  if (!task) task=Fs;
  if ((res=task->display_doc) && res->doc_signature==DOC_SIGNATURE_VAL)
    return res;
  else
    return NULL;
}

public CDoc *DocBorder(CTask *task=NULL)
{//Doc holding border of window text.
  CDoc *res;
  if (!task) task=Fs;
  if ((res=task->border_doc) && res->doc_signature==DOC_SIGNATURE_VAL)
    return res;
  else
    return NULL;
}

public CDoc *DocDblBufStart(CTask *task=NULL)
{//See $LK,"::/Demo/Spy.HC"$
  Bool unlock_ddoc;
  CDoc *pdoc=DocPut(task),*ddoc=DocDisplay(task),*res;
  if (!pdoc || !ddoc || pdoc!=ddoc)
    res=NULL; //Already Double buffering
  else {
    if (!task) task=Fs;
    unlock_ddoc=DocLock(ddoc); //dont change during winupdate, so lock DocPut$WW,0$
    res=DocNew(,task);
    res->win_task		=ddoc->win_task;
    res->max_entries		=ddoc->max_entries;
    MemCpy(res->find_replace,ddoc->find_replace,sizeof(CEdFindText));
    MemCpy(&res->filename,&ddoc->filename,sizeof(CEdFileName));
    res->left_click_link	=ddoc->left_click_link;
    res->right_click_link	=ddoc->right_click_link;
    res->user_put_data	=ddoc->user_put_data;
    res->user_put_key	=ddoc->user_put_key;
    res->user_put_s		=ddoc->user_put_s;
    res->parent_doc		=ddoc->parent_doc;
    res->desc		=ddoc->desc;
    res->user_data		=ddoc->user_data;
    res->flags|=ddoc->flags&DOCG_DBL_BUF_FLAGS | DOCF_DONT_SHOW;
    task->put_doc=res;
    if (unlock_ddoc)
      DocUnlock(ddoc);
  }
  return res;
}

public Bool DocDblBufEnd(CTask *task=NULL)
{//See $LK,"::/Demo/Spy.HC"$
  Bool res=FALSE;
  CDoc *pdoc=DocPut(task),*ddoc=DocDisplay(task);
  if (pdoc && ddoc && pdoc!=ddoc) {//Double buffering?
    if (!task) task=Fs;
    ddoc->flags|=DOCF_DONT_SHOW;
    pdoc->flags&=~DOCF_DONT_SHOW;
    DocLock(ddoc);
    task->display_doc=pdoc;
    DocUnlock(ddoc);
    DocDel(ddoc);
    res=TRUE;
  }
  return res;
}

public Bool DocDblBufSwap(CTask *task=NULL)
{//See $LK,"::/Demo/Spy.HC"$
  Bool res=FALSE;
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
    ddoc->flags			=pdoc->flags&DOCG_DBL_BUF_FLAGS |
	  ddoc->flags&~DOCG_DBL_BUF_FLAGS;
    ddoc->left_click_link	=pdoc->left_click_link;
    ddoc->right_click_link	=pdoc->right_click_link;
    ddoc->user_put_data		=pdoc->user_put_data;
    ddoc->user_put_key		=pdoc->user_put_key;
    ddoc->user_put_s		=pdoc->user_put_s;
    ddoc->desc			=pdoc->desc;
    ddoc->user_data		=pdoc->user_data;
    task->put_doc=ddoc;
    res=TRUE;
  }
  return res;
}
