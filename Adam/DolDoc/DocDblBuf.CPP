#help_index "DolDoc/Task;StdOut/Task"
public CDoc *DocPut(CTask *task=NULL)
{//Current document that StdOut Put() goes to.
//Basically, StdOut unless double buffering.
  CDoc *result;
  if (!task) task=Fs;
  if ((result=task->put_doc) && result->doc_signature==DOC_SIGNATURE_VAL)
    return result;
  else
    return NULL;
}

public CDoc *DocDisplay(CTask *task=NULL)
{//StdOut displayed unless double buffering.
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
{//See $LK,"::/Demo/Spy.CPP"$
  Bool unlock_ddoc;
  CDoc *pdoc=DocPut(task),*ddoc=DocDisplay(task),*result;
  if (!pdoc || !ddoc || pdoc!=ddoc)
    result=NULL; //Already Double buffering
  else {
    if (!task) task=Fs;
    unlock_ddoc=DocLock(ddoc); //dont change during winupdate, so lock DocPut$WW,0$
    result=DocNew(,task);
    result->win_task		=ddoc->win_task;
    result->max_entries		=ddoc->max_entries;
    MemCpy(result->find_replace,ddoc->find_replace,sizeof(CEdFindText));
    MemCpy(&result->filename,&ddoc->filename,sizeof(CEdFileName));
    result->left_click_link	=ddoc->left_click_link;
    result->right_click_link	=ddoc->right_click_link;
    result->user_put_data	=ddoc->user_put_data;
    result->user_put_key	=ddoc->user_put_key;
    result->user_put_s		=ddoc->user_put_s;
    result->parent_doc		=ddoc->parent_doc;
    result->desc		=ddoc->desc;
    result->user_data		=ddoc->user_data;
    result->flags|=ddoc->flags&DOCG_DBL_BUF_FLAGS | DOCF_DONT_SHOW;
    task->put_doc=result;
    if (unlock_ddoc)
      DocUnlock(ddoc);
  }
  return result;
}

public Bool DocDblBufEnd(CTask *task=NULL)
{//See $LK,"::/Demo/Spy.CPP"$
  Bool result=FALSE;
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
  return result;
}

public Bool DocDblBufSwap(CTask *task=NULL)
{//See $LK,"::/Demo/Spy.CPP"$
  Bool result=FALSE;
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
    result=TRUE;
  }
  return result;
}
