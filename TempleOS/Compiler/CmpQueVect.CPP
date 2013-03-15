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
