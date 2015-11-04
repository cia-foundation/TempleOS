U0 SysBadFree(I64 *ptr)
{
  Dbg("Bad Free:",ptr);
}

U0 SysBadMAlloc(I64 *ptr)
{
  Dbg("Bad MAlloc:",ptr);
}

U8 *Mem512Alloc(I64 *_pages512,CBlkPool *bp=NULL)
{/*Alloc 512Byte pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  CMemBlk *result=NULL,*m;
  I64 i,num=*_pages512;
  if (!bp) bp=sys_code_bp;
  PUSHFD
  CLI
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    PAUSE
  if (num<FREE_PAGE_HASH_SIZE) {
    if (result=bp->free_page_hash[num]) {
      bp->free_page_hash[num]=result->next;
      goto at_done;
    }
    i=Bsr(FREE_PAGE_HASH_SIZE)+1;
  } else {
//We'll now round-up to a power of two.
    //There is some overhead on allocations and
    //we wouldn't want to round to the next
    //power of two if a power of two was requested.
    //So we use a little more than a power of two.
    num-=2;
    i=Bsr(num)+1;
    num=1<<i+2;
    if (result=bp->free_page_hash2[i]) {
      bp->free_page_hash2[i]=result->next;
      goto at_done;
    }
  }
  m=&bp->mem_free_lst;
  while (TRUE) {
    if (!(result=m->next)) {
//We're probably out of luck, but lets search for a
      //freed larger size block... and, screw-it, return the whole thing.
      do {
	if (result=bp->free_page_hash2[++i]) {
	  num=1<<i+2;
	  bp->free_page_hash2[i]=result->next;
	  goto at_done;
	}
      } while (i<64-PAGE_BITS-1);
      num=0;
      result=NULL; //Out of memory
      goto at_done;
    }
    if (result->pages<num)
      m=result;
    else {
      if (result->pages==num) {
	m->next=result->next;
	goto at_done;
      } else {
	result->pages-=num;
	result(U8 *)+=result->pages<<PAGE_BITS;
	result->pages=num;
	goto at_done;
      }
    }
  }
at_done:
  *_pages512=num;
  bp->used_u8s+=num<<PAGE_BITS;
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  POPFD
  return result;
}

U0 Mem512Free(CMemBlk *m,I64 pages512,CBlkPool *bp=NULL)
{//Return non-task 512Byte pages to BlkPool.
  I64 i;
  if (m) {
    if (!bp) bp=sys_code_bp;
    PUSHFD
    CLI
    while (LBts(&bp->locked_flags,BPlf_LOCKED))
      PAUSE
    m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
    m->pages=pages512;
    bp->used_u8s-=pages512<<PAGE_BITS;
    if (pages512<FREE_PAGE_HASH_SIZE) {
      m->next=bp->free_page_hash[pages512];
      bp->free_page_hash[pages512]=m;
    } else {
//We'll now round-up to a power of two.
      //There is some overhead on allocations and
      //we wouldn't want to round to the next
      //power of two if a power of two was requested.
      //So we use a little more than a power of two.
      pages512-=2;
      i=Bsr(pages512);
      m->next=bp->free_page_hash2[i];
      bp->free_page_hash2[i]=m;
    }
    LBtr(&bp->locked_flags,BPlf_LOCKED);
    POPFD
  }
}

CMemBlk *Mem512TaskAlloc(I64 *_pages512,CHeapCtrl *hc)
{/*hc must be locked.  Interrupts should probably be off
Currently, this is only called from $LK,"MAlloc",A="MN:MAlloc"$().

Return: NULL if out of memory.
*/
  CMemBlk *result;
  I64 threshold,cnt,size;
  CMemUnused *uum,**_uum,**_ptr;
  if (result=Mem512Alloc(_pages512,hc->bp)) {
    QueIns(result,hc->last_mem_blk);
    result->mb_signature=MBS_USED_SIGNATURE_VAL;
    hc->alloced_u8s+=*_pages512<<PAGE_BITS;

    //Tidy-up free lst (Move into heap hash)
    //because if free lst gets long, delay causes crash
    threshold=HEAP_HASH_SIZE>>4;
#assert HEAP_HASH_SIZE>>4>=sizeof(U8 *)
    do {
      cnt=0;
      _uum=&hc->malloc_free_lst;
      while (uum=*_uum) {
#assert !offset(CMemUnused.next)
	size=uum->size;
	if (size<threshold) {
	  *_uum=uum->next;
	  _ptr=(&hc->heap_hash)(U8 *)+size;
	  uum->next=*_ptr;
	  *_ptr=uum;
	} else {
	  cnt++;
	  _uum=uum;
	}
      }
      threshold<<=1;
    } while (cnt>8 && threshold<=HEAP_HASH_SIZE);
  }
  return result;
}

U0 Mem512TaskFree(CMemBlk *m,CHeapCtrl *hc)
{//hc must be locked
  if (m) {
    PUSHFD
    CLI
    if (m->mb_signature!=MBS_USED_SIGNATURE_VAL)
      SysBadFree(m);
    else {
      QueRem(m);
      hc->alloced_u8s-=m->pages<<PAGE_BITS;
      Mem512Free(m,m->pages,hc->bp);
    }
    POPFD
  }
}
