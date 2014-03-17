U0 LinkedLstDel(U8 **_lst)
{
  U8 **templ;
  while (_lst) {
    templ=*_lst;
    Free(_lst);
    _lst=templ;
  }
}

U8 *LinkedLstCopy(U8 **_lst,CTask *mem_task=NULL)
{
  U8 *result=NULL,**templ=&result;
  while (_lst) {
    templ=*templ=MAllocIdentical(_lst,mem_task);
    _lst=*_lst;
  }
  return result;
}

I64 LinkedLstCnt(U8 **_lst)
{
  I64 result=0;
  while (_lst) {
    result++;
    _lst=*_lst;
  }
  return result;
}

I64 LinkedLstSize(U8 **_lst)
{
  I64 result=0;
  while (_lst) {
    result+=MSize2(_lst);
    _lst=*_lst;
  }
  return result;
}

U0 QueDel(CQue *root,Bool querem=FALSE) //Does not delete root
{
  CQue *tempq=root->next,*tempq1;
  while (tempq!=root) {
    tempq1=tempq->next;
    if (querem)
      QueRem(tempq);
    Free(tempq);
    tempq=tempq1;
  }
}

CQue *QueCopy(CQue *root,CTask *mem_task=NULL)
{
  CQue *result=MAllocIdentical(root,mem_task),*tempq=root->next,*tempq1;
  QueInit(result);
  while (tempq!=root) {
    tempq1=MAllocIdentical(tempq,mem_task);
    QueIns(tempq1,result->last);
    tempq=tempq->next;
  }
  return result;
}

I64 QueCnt(CQue *root) //Does not include root
{
  CQue *tempq=root->next;
  I64 result=0;
  while (tempq!=root) {
    result++;
    tempq=tempq->next;
  }
  return result;
}

I64 QueSize(CQue *root) //Does not include root
{
  CQue *tempq=root->next;
  I64 result=0;
  while (tempq!=root) {
    result+=MSize2(tempq);
    tempq=tempq->next;
  }
  return result;
}

CQueVectU8 *QueVectU8New(I64 min_index=0)
{
  CQueVectU8 *result=MAlloc(sizeof(CQueVectU8));
  result->next=result->last=result;
  result->total_cnt=result->node_cnt=0;
  result->min_index=min_index;
  return result;
}

U0 QueVectU8Put(CQueVectU8 *v,I64 index,U8 ch)
{
  CQueVectU8 *tempv;
  index-=v->min_index;
  if (index<0) return;
  if (index<v->total_cnt) {
    tempv=v;
    do {
      index-=tempv->node_cnt;
      if (index<0) {
	tempv->body[index+tempv->node_cnt]=ch;
	return;
      }
      tempv=tempv->next;
    } while (tempv!=v);
  } else
    index-=v->total_cnt;

  while (TRUE) {
    tempv=v->last;
    if (tempv->node_cnt>=QUE_VECT_U8_CNT) {
      tempv=MAlloc(sizeof(CQueVectU8));
      tempv->node_cnt=0;
      QueIns(tempv,v->last);
    }
    if (index--) {
      tempv->body[tempv->node_cnt++]=0;
      v->total_cnt++;
    } else {
      tempv->body[tempv->node_cnt++]=ch;
      v->total_cnt++;
      break;
    }
  }
}

U0 QueVectU8Del(CQueVectU8 *v)
{
  if (v) {
    QueDel(v);
    Free(v);
  }
}

I64 QueVectU8Get(CQueVectU8 *v,I64 index)
{
  CQueVectU8 *tempv;
  index-=v->min_index;
  if (!(0<=index<v->total_cnt)) return 0;
  tempv=v;
  do {
    index-=tempv->node_cnt;
    if (index<0)
      return tempv->body[index+tempv->node_cnt];
    tempv=tempv->next;
  } while (tempv!=v);
  return 0;
}

CFifoU8 *FifoU8New(I64 size,CTask *mem_task=NULL)
{
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
{
  Free(f->buf);
  Free(f);
}

Bool FifoU8Ins(CFifoU8 *f,U8 b)
{
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
{
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
{
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
{
  PUSHFD
  CLI
  f->out_ptr=f->in_ptr;
  POPFD
}

I64 FifoU8Cnt(CFifoU8 *f)
{
  I64 result;
  PUSHFD
  CLI
  if (f->out_ptr>f->in_ptr)
    result=f->mask+1-(f->out_ptr-f->in_ptr);
  else
    result=f->in_ptr-f->out_ptr;
  POPFD
  return result;
}

CFifoI64 *FifoI64New(I64 size,CTask *mem_task=NULL)
{
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
{
  Free(f->buf);
  Free(f);
}

Bool FifoI64Ins(CFifoI64 *f,I64 q)
{
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
{
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
{
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
{
  PUSHFD
  CLI
  f->out_ptr=f->in_ptr;
  POPFD
}

I64 FifoI64Cnt(CFifoI64 *f)
{
  I64 result;
  PUSHFD
  CLI
  if (f->out_ptr>f->in_ptr)
    result=f->mask+1-(f->out_ptr-f->in_ptr);
  else
    result=f->in_ptr-f->out_ptr;
  POPFD
  return result;
}
