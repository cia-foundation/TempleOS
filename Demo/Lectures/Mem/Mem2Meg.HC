//This file is required by $LK,"::/Demo/Lectures/Mem/Uncached.HC"$.

U8 *Mem2MegAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
{/*Alloc 2Meg pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  I64 i,j,*pte,num=*_pages2Meg;
  CMemBlk *res=NULL,*m,*m1;

  if (!bp) bp=sys_code_bp;
  PUSHFD
  CLI
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    PAUSE
  num<<=21-MEM_PAGE_BITS;

  m=&bp->mem_free_2meg_lst;
  while (TRUE) {
    if (!(res=m->next))
      break;
    if (res->pages<num)
      m=res;
    else {
      if (res->pages==num) {
	m->next=res->next;
	goto am_done;
      } else {
	res->pages-=num;
	res(U8 *)+=res->pages<<MEM_PAGE_BITS;
	res->pages=num;
	goto am_done;
      }
    }
  }

  m=&bp->mem_free_lst;
  while (TRUE) {
    if (!(res=m->next)) {
      num=0;
      res=NULL; //Out of memory
      goto am_done;
    }
    if (res->pages<num)
      m=res;
    else {
      if (res->pages==num) {
	if (res(U8 *)&0x1FFFFF)
	  m=res;
	else {
	  m->next=res->next;
	  goto am_done;
	}
      } else {
	if (i=(res(U8 *)&0x1FFFFF)>>MEM_PAGE_BITS) {
	  j=1<<(21-MEM_PAGE_BITS)-i;
	  if (res->pages<num+j)
	    m=res;
	  else if (res->pages==num+j) {
	    res->pages-=num;
	    res(U8 *)+=res->pages<<MEM_PAGE_BITS;
	    res->pages=num;
	    goto am_done;
	  } else {
	    m1=res;
	    res(U8 *)+=j<<MEM_PAGE_BITS;
	    res->pages=num;
	    m=res(U8 *)+num<<MEM_PAGE_BITS;
	    m->pages=m1->pages-num-j;
	    m1->pages=j;
	    m->next=m1->next;
	    m1->next=m;
	    m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
	    goto am_done;
	  }
	} else {
	  m=m->next=res(U8 *)+num<<MEM_PAGE_BITS;
	  m->next=res->next;
	  m->pages=res->pages-num;
	  m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
	  res->pages=num;
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
  m=res;
  m1=m(U8 *)+i;
  while (m<m1) {
    pte=MemPageTable(m);
    *pte &= ~0x18;
    InvlPg(m);
    m(U8 *)+=0x200000;
  }
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  POPFD
  return res;
}

U8 *Mem2MegUncachedAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
{/*Alloc 2Meg pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It will be marked uncached. It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  CMemBlk *res,*m,*m1;
  I64 num=*_pages2Meg,*pte;
  if (res=Mem2MegAlloc(_pages2Meg,bp)) {
    num=*_pages2Meg;
    m=res;
    m1=m(U8 *)+num<<21;
    while (m<m1) {
      pte=MemPageTable(m);
      *pte= *pte& ~0x18 |0x10;
      InvlPg(m);
      m(U8 *)+=0x200000;
    }
  }
  return res;
}

U8 *Mem2MegWriteThruAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
{/*Alloc 2Meg pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It will be marked write-through. It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  CMemBlk *res,*m,*m1;
  I64 num=*_pages2Meg,*pte;
  if (res=Mem2MegAlloc(_pages2Meg,bp)) {
    num=*_pages2Meg;
    m=res;
    m1=m(U8 *)+num<<21;
    while (m<m1) {
      pte=MemPageTable(m);
      *pte= *pte& ~0x18 |8;
      InvlPg(m);
      m(U8 *)+=0x200000;
    }
  }
  return res;
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
      m2(U8 *)+=0x200000;
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
access except VGA, handled here: $LK,"::/Kernel/Mem/PageTables.HC"$.
*/
