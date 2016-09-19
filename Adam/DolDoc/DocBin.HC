#help_index "DolDoc/Bin"

CDocBin *DocBinFindNum(CDoc *haystack_doc,I64 needle_num)
{
  CDocBin *b=haystack_doc->bin_head.next;
  while (b!=&haystack_doc->bin_head) {
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
    b=haystack_doc->bin_head.next;
    while (b!=&haystack_doc->bin_head) {
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
  b=doc->bin_head.next;
  while (b!=&doc->bin_head) {
    b->use_cnt=0;
    b->temp_use_cnt=0;
    b->renum_num=-1;
    Free(b->tag);
    b->tag=NULL;
    b=b->next;
  }
  doc_e=doc->head.next;
  while (doc_e!=doc) {
    doc_e2=doc_e->next;
    if (doc_e->de_flags & DOCEF_HAS_BIN) {
      if (b=doc_e->bin_data=DocBinFindNum(doc,doc_e->bin_num)) {
	if (doc_e->de_flags & DOCEF_BIN_PTR_LINK)
	  b->temp_use_cnt=MAX_I32;
	if (!b->use_cnt++)
	  b->renum_num=++renum_num;
	doc_e->bin_num=b->renum_num;
	if (!b->tag && doc_e->de_flags&DOCEF_TAG && doc_e->tag && *doc_e->tag)
	  b->tag=StrNew(doc_e->tag,doc->mem_task);
      } else {
	RawPrint(3000,"Bin Not Found");
	doc_e->type=doc_e->de_flags=0;
	doc_e->type_u8=DOCT_ERROR;
      }
    }
    doc_e=doc_e2;
  }

  b=doc->bin_head.next;
  doc->cur_bin_num=1;
  while (b!=&doc->bin_head) {
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

I64 DocBinPtrRst(CDoc *doc,CDocEntry *doc_e)
{
  U8 *st,*st2;
  CDoc *doc2;
  CDocBin *tempb,*tempb2;
  I64 i,bin_num=0;
  if (doc_e->de_flags&DOCEF_HAS_BIN &&
	doc_e->bin_ptr_link && StrLen(doc_e->bin_ptr_link)) {
    bin_num=doc_e->bin_num;
    st=StrNew(doc_e->bin_ptr_link);
    st2=StrNew(st);
    StrLastRem(st,",",st2);
    i=Str2I64(st2);
    if (i>0||*st2) {
      doc2=DocRead(st);
      if (i>0  && (tempb2=DocBinFindNum(doc2,i)) ||
	    i==0 && (tempb2=DocBinFindTag(doc2,st2))) {
	i=1;
	if (bin_num>0) {
	  if (tempb=DocBinFindNum(doc,bin_num)) {
	    i=tempb->use_cnt;
	    DocBinDel(doc,tempb);
	  }
	} else
	  bin_num=doc->cur_bin_num++;
	tempb=MAllocIdent(tempb2,doc->mem_task);
	tempb->use_cnt=i;
	tempb->data=MAllocIdent(tempb2->data,doc->mem_task);
	tempb->num=bin_num;
	doc_e->bin_data=tempb;
	if (doc_e->de_flags&DOCEF_TAG && doc_e->tag && *doc_e->tag)
	  tempb->tag=StrNew(doc_e->tag,doc->mem_task);
	else
	  tempb->tag=NULL;
	QueIns(tempb,doc->bin_head.last);
      } else
	bin_num=0;
      DocDel(doc2);
    } else
      bin_num=0;
    Free(st2);
    Free(st);
    doc_e->bin_num=bin_num;
  }
  return bin_num;
}
