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
  PushFD;
  Cli;
  new_in_ptr=(f->in_ptr+1)&f->mask;
  if (new_in_ptr==f->out_ptr) {
    PopFD;
    return FALSE;
  } else {
    f->buf[f->in_ptr]=b;
    f->in_ptr=new_in_ptr;
    PopFD;
    return TRUE;
  }
}

Bool FifoU8Rem(CFifoU8 *f,U8 *_b)
{
  PushFD;
  Cli;
  if (f->in_ptr==f->out_ptr) {
    PopFD;
    return FALSE;
  } else {
    *_b=f->buf[f->out_ptr];
    f->out_ptr=(f->out_ptr+1)&f->mask;
    PopFD;
    return TRUE;
  }
}

Bool FifoU8Peek(CFifoU8 *f,U8 *_b)
{
  PushFD;
  Cli;
  if (f->in_ptr==f->out_ptr) {
    PopFD;
    return FALSE;
  } else {
    *_b=f->buf[f->out_ptr];
    PopFD;
    return TRUE;
  }
}

U0 FifoU8Flush(CFifoU8 *f)
{
  PushFD;
  Cli;
  f->out_ptr=f->in_ptr;
  PopFD;
}

I64 FifoU8Cnt(CFifoU8 *f)
{
  I64 result;
  PushFD;
  Cli;
  if (f->out_ptr>f->in_ptr)
    result=f->mask+1-(f->out_ptr-f->in_ptr);
  else
    result=f->in_ptr-f->out_ptr;
  PopFD;
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
  PushFD;
  Cli;
  new_in_ptr=(f->in_ptr+1)&f->mask;
  if (new_in_ptr==f->out_ptr) {
    PopFD;
    return FALSE;
  } else {
    f->buf[f->in_ptr]=q;
    f->in_ptr=new_in_ptr;
    PopFD;
    return TRUE;
  }
}

Bool FifoI64Rem(CFifoI64 *f,I64 *_q)
{
  PushFD;
  Cli;
  if (f->in_ptr==f->out_ptr) {
    PopFD;
    return FALSE;
  } else {
    *_q=f->buf[f->out_ptr];
    f->out_ptr=(f->out_ptr+1)&f->mask;
    PopFD;
    return TRUE;
  }
}

Bool FifoI64Peek(CFifoI64 *f,I64 *_q)
{
  PushFD;
  Cli;
  if (f->in_ptr==f->out_ptr) {
    PopFD;
    return FALSE;
  } else {
    *_q=f->buf[f->out_ptr];
    PopFD;
    return TRUE;
  }
}

U0 FifoI64Flush(CFifoI64 *f)
{
  PushFD;
  Cli;
  f->out_ptr=f->in_ptr;
  PopFD;
}

I64 FifoI64Cnt(CFifoI64 *f)
{
  I64 result;
  PushFD;
  Cli;
  if (f->out_ptr>f->in_ptr)
    result=f->mask+1-(f->out_ptr-f->in_ptr);
  else
    result=f->in_ptr-f->out_ptr;
  PopFD;
  return result;
}
