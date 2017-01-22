U0 LinkedLstDel(U8 **_lst)
{//Free entire linked-list.
  U8 **tmpl;
  while (_lst) {
    tmpl=*_lst;
    Free(_lst);
    _lst=tmpl;
  }
}

U8 *LinkedLstCopy(U8 **_lst,CTask *mem_task=NULL)
{//MAlloc ident copy of entire linked-list.
  U8 *res=NULL,**tmpl=&res;
  while (_lst) {
    tmpl=*tmpl=MAllocIdent(_lst,mem_task);
    _lst=*_lst;
  }
  return res;
}

I64 LinkedLstCnt(U8 **_lst)
{//Count of nodes in linked-list.
  I64 res=0;
  while (_lst) {
    res++;
    _lst=*_lst;
  }
  return res;
}

I64 LinkedLstSize(U8 **_lst)
{//Mem size of all nodes in linked-list.
  I64 res=0;
  while (_lst) {
    res+=MSize2(_lst);
    _lst=*_lst;
  }
  return res;
}

U0 QueDel(CQue *head,Bool querem=FALSE)
{//Free entries in queue, not head.
  CQue *tmpq=head->next,*tmpq1;
  while (tmpq!=head) {
    tmpq1=tmpq->next;
    if (querem)
      QueRem(tmpq);
    Free(tmpq);
    tmpq=tmpq1;
  }
}

CQue *QueCopy(CQue *head,CTask *mem_task=NULL)
{//MAlloc ident copy of entire queue and head.
  CQue *res=MAllocIdent(head,mem_task),*tmpq=head->next,*tmpq1;
  QueInit(res);
  while (tmpq!=head) {
    tmpq1=MAllocIdent(tmpq,mem_task);
    QueIns(tmpq1,res->last);
    tmpq=tmpq->next;
  }
  return res;
}

I64 QueCnt(CQue *head)
{//Count of nodes in queue, not head.
  CQue *tmpq=head->next;
  I64 res=0;
  while (tmpq!=head) {
    res++;
    tmpq=tmpq->next;
  }
  return res;
}

I64 QueSize(CQue *head)
{//Mem size of all nodes in queue, not head.
  CQue *tmpq=head->next;
  I64 res=0;
  while (tmpq!=head) {
    res+=MSize2(tmpq);
    tmpq=tmpq->next;
  }
  return res;
}

CQueVectU8 *QueVectU8New(I64 min_idx=0)
{//Create new queue vecter.
  CQueVectU8 *res=MAlloc(sizeof(CQueVectU8));
  QueInit(res);
  res->total_cnt=res->node_cnt=0;
  res->min_idx=min_idx;
  return res;
}

U0 QueVectU8Put(CQueVectU8 *v,I64 idx,U8 ch)
{//Put U8 at idx i.
  CQueVectU8 *tmpv;
  idx-=v->min_idx;
  if (idx<0) return;
  if (idx<v->total_cnt) {
    tmpv=v;
    do {
      idx-=tmpv->node_cnt;
      if (idx<0) {
	tmpv->body[idx+tmpv->node_cnt]=ch;
	return;
      }
      tmpv=tmpv->next;
    } while (tmpv!=v);
  } else
    idx-=v->total_cnt;

  while (TRUE) {
    tmpv=v->last;
    if (tmpv->node_cnt>=QUE_VECT_U8_CNT) {
      tmpv=MAlloc(sizeof(CQueVectU8));
      tmpv->node_cnt=0;
      QueIns(tmpv,v->last);
    }
    if (idx--) {
      tmpv->body[tmpv->node_cnt++]=0;
      v->total_cnt++;
    } else {
      tmpv->body[tmpv->node_cnt++]=ch;
      v->total_cnt++;
      break;
    }
  }
}

U0 QueVectU8Del(CQueVectU8 *v)
{//Free entire queue vector.
  if (v) {
    QueDel(v);
    Free(v);
  }
}

I64 QueVectU8Get(CQueVectU8 *v,I64 idx)
{//Get U8 at idx i.
  CQueVectU8 *tmpv;
  idx-=v->min_idx;
  if (!(0<=idx<v->total_cnt)) return 0;
  tmpv=v;
  do {
    idx-=tmpv->node_cnt;
    if (idx<0)
      return tmpv->body[idx+tmpv->node_cnt];
    tmpv=tmpv->next;
  } while (tmpv!=v);
  return 0;
}

CFifoU8 *FifoU8New(I64 size,CTask *mem_task=NULL)
{//Create new fifo.
  CFifoU8 *f;
  if (!mem_task) mem_task=Fs;
  f=MAlloc(sizeof(CFifoU8),mem_task);
  f->buf=MAlloc(size,mem_task);
  f->mask=size-1;
  f->in_ptr=0;
  f->out_ptr=0;
  return f;
}

U0 FifoU8Del(CFifoU8 *f)
{//Free fifo.
  Free(f->buf);
  Free(f);
}

Bool FifoU8Ins(CFifoU8 *f,U8 b)
{//Insert U8 into fifo.
  I64 new_in_ptr;
  PUSHFD
  CLI
  new_in_ptr=(f->in_ptr+1)&f->mask;
  if (new_in_ptr==f->out_ptr) {
    POPFD
    return FALSE;
  } else {
    f->buf[f->in_ptr]=b;
    f->in_ptr=new_in_ptr;
    POPFD
    return TRUE;
  }
}

Bool FifoU8Rem(CFifoU8 *f,U8 *_b)
{//Remove U8 from fifo.
  PUSHFD
  CLI
  if (f->in_ptr==f->out_ptr) {
    POPFD
    return FALSE;
  } else {
    *_b=f->buf[f->out_ptr];
    f->out_ptr=(f->out_ptr+1)&f->mask;
    POPFD
    return TRUE;
  }
}

Bool FifoU8Peek(CFifoU8 *f,U8 *_b)
{//Peek at front of fifo and don't remove.
  PUSHFD
  CLI
  if (f->in_ptr==f->out_ptr) {
    POPFD
    return FALSE;
  } else {
    *_b=f->buf[f->out_ptr];
    POPFD
    return TRUE;
  }
}

U0 FifoU8Flush(CFifoU8 *f)
{//Flush fifo getting rid of all U8's.
  PUSHFD
  CLI
  f->out_ptr=f->in_ptr;
  POPFD
}

I64 FifoU8Cnt(CFifoU8 *f)
{//Count of U8's in fifo.
  I64 res;
  PUSHFD
  CLI
  if (f->out_ptr>f->in_ptr)
    res=f->mask+1-(f->out_ptr-f->in_ptr);
  else
    res=f->in_ptr-f->out_ptr;
  POPFD
  return res;
}

CFifoI64 *FifoI64New(I64 size,CTask *mem_task=NULL)
{//Create new fifo.
  CFifoI64 *f;
  if (!mem_task) mem_task=Fs;
  f=MAlloc(sizeof(CFifoI64),mem_task);
  f->buf=MAlloc(size*sizeof(I64),mem_task);
  f->mask=size-1;
  f->in_ptr=0;
  f->out_ptr=0;
  return f;
}

U0 FifoI64Del(CFifoI64 *f)
{//Free fifo.
  Free(f->buf);
  Free(f);
}

Bool FifoI64Ins(CFifoI64 *f,I64 q)
{//Insert I64 into fifo.
  I64 new_in_ptr;
  PUSHFD
  CLI
  new_in_ptr=(f->in_ptr+1)&f->mask;
  if (new_in_ptr==f->out_ptr) {
    POPFD
    return FALSE;
  } else {
    f->buf[f->in_ptr]=q;
    f->in_ptr=new_in_ptr;
    POPFD
    return TRUE;
  }
}

Bool FifoI64Rem(CFifoI64 *f,I64 *_q)
{//Remove I64 from fifo.
  PUSHFD
  CLI
  if (f->in_ptr==f->out_ptr) {
    POPFD
    return FALSE;
  } else {
    *_q=f->buf[f->out_ptr];
    f->out_ptr=(f->out_ptr+1)&f->mask;
    POPFD
    return TRUE;
  }
}

Bool FifoI64Peek(CFifoI64 *f,I64 *_q)
{//Peek at front of fifo and don't remove.
  PUSHFD
  CLI
  if (f->in_ptr==f->out_ptr) {
    POPFD
    return FALSE;
  } else {
    *_q=f->buf[f->out_ptr];
    POPFD
    return TRUE;
  }
}

U0 FifoI64Flush(CFifoI64 *f)
{//Flush fifo getting rid of all I64's.
  PUSHFD
  CLI
  f->out_ptr=f->in_ptr;
  POPFD
}

I64 FifoI64Cnt(CFifoI64 *f)
{//Count of I64's in fifo.
  I64 res;
  PUSHFD
  CLI
  if (f->out_ptr>f->in_ptr)
    res=f->mask+1-(f->out_ptr-f->in_ptr);
  else
    res=f->in_ptr-f->out_ptr;
  POPFD
  return res;
}
