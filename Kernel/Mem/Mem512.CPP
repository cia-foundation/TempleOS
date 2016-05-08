U0 SysBadFree(I64 *ptr)
{
  Dbg("Bad Free:",ptr);
}

U0 SysBadMAlloc(I64 *ptr)
{
  Dbg("Bad MAlloc:",ptr);
}

U8 *Mem512Alloc(I64 pages512,CBlkPool *bp=NULL)
{/*Alloc 512Byte pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  CMemBlk *res=NULL,*m;
  I64 i;
  if (!bp) bp=sys_code_bp;
  PUSHFD
  CLI
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    PAUSE
  if (pages512<MEM_FREE_PAGE_HASH_SIZE) {
    if (res=bp->free_page_hash[pages512]) {
      bp->free_page_hash[pages512]=res->next;
      goto at_done;
    }
    i=Bsr(MEM_FREE_PAGE_HASH_SIZE)+1;
  } else {
//We'll now round-up to a power of two.
    //There is some overhead on allocations and
    //we wouldn't want to round to the next
    //power of two if a power of two was requested.
    //So we use a little more than a power of two.
    pages512-=MEM_EXTRA_HASH2_PAGES;
    i=Bsr(pages512)+1;
    pages512=1<<i+MEM_EXTRA_HASH2_PAGES;
    if (res=bp->free_page_hash2[i]) {
      bp->free_page_hash2[i]=res->next;
      goto at_done;
    }
  }
  m=&bp->mem_free_lst;
  while (TRUE) {
    if (!(res=m->next)) {
//We're probably out of luck, but lets search for a
      //freed larger size block... and, screw-it, return the whole thing.
      do {
	if (res=bp->free_page_hash2[++i]) {
	  pages512=1<<i+MEM_EXTRA_HASH2_PAGES;
	  bp->free_page_hash2[i]=res->next;
	  goto at_done;
	}
      } while (i<64-MEM_PAGE_BITS-1);
      pages512=0;
      res=NULL; //Out of memory
      goto at_done;
    }
    if (res->pages<pages512)
      m=res;
    else {
      if (res->pages==pages512) {
	m->next=res->next;
	goto at_done;
      } else {
	res->pages-=pages512;
	res(U8 *)+=res->pages<<MEM_PAGE_BITS;
	res->pages=pages512;
	goto at_done;
      }
    }
  }
at_done:
  bp->used_u8s+=pages512<<MEM_PAGE_BITS;
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  POPFD
  return res;
}

U0 Mem512Free(CMemBlk *m,CBlkPool *bp=NULL)
{//Return non-task 512Byte pages to BlkPool.
  I64 i,pages512;
  if (m) {
    if (!bp) bp=sys_code_bp;
    PUSHFD
    CLI
    while (LBts(&bp->locked_flags,BPlf_LOCKED))
      PAUSE
    pages512=m->pages;
    m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
    bp->used_u8s-=pages512<<MEM_PAGE_BITS;
    if (pages512<MEM_FREE_PAGE_HASH_SIZE) {
      m->next=bp->free_page_hash[pages512];
      bp->free_page_hash[pages512]=m;
    } else {
//We'll now round-up to a power of two.
      //There is some overhead on allocations and
      //we wouldn't want to round to the next
      //power of two if a power of two was requested.
      //So we use a little more than a power of two.
      pages512-=MEM_EXTRA_HASH2_PAGES;
      i=Bsr(pages512);
      m->next=bp->free_page_hash2[i];
      bp->free_page_hash2[i]=m;
    }
    LBtr(&bp->locked_flags,BPlf_LOCKED);
    POPFD
  }
}

CMemBlk *Mem512TaskAlloc(I64 pages512,CHeapCtrl *hc)
{/*hc must be locked.  Interrupts should probably be off
Currently, this is only called from $LK,"MAlloc",A="MN:MAlloc"$().
Return: NULL if out of memory.
*/
  CMemBlk *res;
  I64 threshold,cnt,size;
  CMemUnused *uum,**_uum,**_ptr;
  if (res=Mem512Alloc(pages512,hc->bp)) {
    QueIns(res,hc->last_mem_blk);
    res->mb_signature=MBS_USED_SIGNATURE_VAL;
    hc->alloced_u8s+=res->pages<<MEM_PAGE_BITS;

    //Tidy-up free lst (Move into heap hash)
    //because if free lst gets long, delay causes crash
    threshold=MEM_HEAP_HASH_SIZE>>4;
#assert MEM_HEAP_HASH_SIZE>>4>=sizeof(U8 *)
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
    } while (cnt>8 && threshold<=MEM_HEAP_HASH_SIZE);
  }
  return res;
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
      hc->alloced_u8s-=m->pages<<MEM_PAGE_BITS;
      Mem512Free(m,hc->bp);
    }
    POPFD
  }
}
