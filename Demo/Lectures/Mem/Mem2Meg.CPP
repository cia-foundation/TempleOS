//This file is required by $LK,"::/Demo/Lectures/Mem/Uncached.CPP"$.

U8 *Mem2MegAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
{/*Alloc 2Meg pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  I64 i,j,*pte,num=*_pages2Meg;
  CMemBlk *result=NULL,*m,*m1;

  if (!bp) bp=sys_code_bp;
  PUSHFD
  CLI
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    PAUSE
  num<<=21-MEM_PAGE_BITS;

  m=&bp->mem_free_2meg_lst;
  while (TRUE) {
    if (!(result=m->next))
      break;
    if (result->pages<num)
      m=result;
    else {
      if (result->pages==num) {
	m->next=result->next;
	goto am_done;
      } else {
	result->pages-=num;
	result(U8 *)+=result->pages<<MEM_PAGE_BITS;
	result->pages=num;
	goto am_done;
      }
    }
  }

  m=&bp->mem_free_lst;
  while (TRUE) {
    if (!(result=m->next)) {
      num=0;
      result=NULL; //Out of memory
      goto am_done;
    }
    if (result->pages<num)
      m=result;
    else {
      if (result->pages==num) {
	if (result(U8 *)&0x1FFFFF)
	  m=result;
	else {
	  m->next=result->next;
	  goto am_done;
	}
      } else {
	if (i=(result(U8 *)&0x1FFFFF)>>MEM_PAGE_BITS) {
	  j=1<<(21-MEM_PAGE_BITS)-i;
	  if (result->pages<num+j)
	    m=result;
	  else if (result->pages==num+j) {
	    result->pages-=num;
	    result(U8 *)+=result->pages<<MEM_PAGE_BITS;
	    result->pages=num;
	    goto am_done;
	  } else {
	    m1=result;
	    result(U8 *)+=j<<MEM_PAGE_BITS;
	    result->pages=num;
	    m=result(U8 *)+num<<MEM_PAGE_BITS;
	    m->pages=m1->pages-num-j;
	    m1->pages=j;
	    m->next=m1->next;
	    m1->next=m;
	    m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
	    goto am_done;
	  }
	} else {
	  m=m->next=result(U8 *)+num<<MEM_PAGE_BITS;
	  m->next=result->next;
	  m->pages=result->pages-num;
	  m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
	  result->pages=num;
	  goto am_done;
	}
      }
    }
  }
am_done:
  i=num<<MEM_PAGE_BITS;
  bp->used_u8s+=i;
  num>>=21-MEM_PAGE_BITS;
  *_pages2Meg=num;
  m=result;
  m1=m(U8 *)+i;
  while (m<m1) {
    pte=MemPageTable(m);
    *pte &= ~0x18;
    InvlPg(m);
    m(U8 *)+=MemPageSize(m);
  }
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  POPFD
  return result;
}

U8 *Mem2MegUncachedAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
{/*Alloc 2Meg pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It will be marked uncached. It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  CMemBlk *result,*m,*m1;
  I64 num=*_pages2Meg,*pte;
  if (result=Mem2MegAlloc(_pages2Meg,bp)) {
    num=*_pages2Meg;
    m=result;
    m1=m(U8 *)+num<<21;
    while (m<m1) {
      pte=MemPageTable(m);
      *pte= *pte& ~0x18 |0x10;
      InvlPg(m);
      m(U8 *)+=MemPageSize(m);
    }
  }
  return result;
}

U8 *Mem2MegWriteThruAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
{/*Alloc 2Meg pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It will be marked write-through. It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  CMemBlk *result,*m,*m1;
  I64 num=*_pages2Meg,*pte;
  if (result=Mem2MegAlloc(_pages2Meg,bp)) {
    num=*_pages2Meg;
    m=result;
    m1=m(U8 *)+num<<21;
    while (m<m1) {
      pte=MemPageTable(m);
      *pte= *pte& ~0x18 |8;
      InvlPg(m);
      m(U8 *)+=MemPageSize(m);
    }
  }
  return result;
}

U0 Mem2MegFree(CMemBlk *m,I64 pages2Meg,CBlkPool *bp=NULL)
{//Return non-task 2Meg pages to BlkPool.
  I64 *pte;
  CMemBlk *m1,*m2;
  if (m) {
    if (!bp) bp=sys_code_bp;
    m2=m;
    m1=m(U8 *)+pages2Meg<<21;
    while (m2<m1) {
      pte=MemPageTable(m2);
      *pte=*pte & ~0x18;
      InvlPg(m2);
      m2(U8 *)+=MemPageSize(m2);
    }
    PUSHFD
    CLI
    while (LBts(&bp->locked_flags,BPlf_LOCKED))
      PAUSE
    m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
    m->pages=pages2Meg<<(21-MEM_PAGE_BITS);
    bp->used_u8s-=pages2Meg<<21;
    m->next=bp->mem_free_2meg_lst;
    bp->mem_free_2meg_lst=m;
    LBtr(&bp->locked_flags,BPlf_LOCKED);
    POPFD
  }
}

CHeapCtrl *HeapCtrlBPInit(CBlkPool *bp,I64 pages512)
{//Make mem chunk into HeapCtrl and BlkPool.
  I64 num;
  CMemBlk *m;
  CHeapCtrl *hc;
  MemSet(bp,0,sizeof(CBlkPool)+sizeof(CHeapCtrl));
  hc=HeapCtrlInit(bp(U8 *)+sizeof(CBlkPool),,bp);
  m=(bp(U8 *)+sizeof(CBlkPool)+sizeof(CHeapCtrl)+MEM_PAGE_SIZE-1)&
	~(MEM_PAGE_SIZE-1);
  num=(bp(U8 *)+pages512<<MEM_PAGE_BITS-m(U8 *))>>MEM_PAGE_BITS;
  bp->alloced_u8s=(pages512-num)<<MEM_PAGE_BITS;
  BlkPoolAdd(bp,m,num);
  return hc;
}

/*  PCI hardware devices, when they require uncached memory buffers, need
this code.  TempleOS, however, has a law requiring only IN/OUT port hardware
access except VGA, handled here: $LK,"::/Kernel/Mem/PageTables.CPP"$.
*/
