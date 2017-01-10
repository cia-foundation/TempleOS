U0 SysBadFree(I64 *ptr)
{
  Panic("Bad Free:",ptr);
}

U0 SysBadMAlloc(I64 *ptr)
{
  Panic("Bad MAlloc:",ptr);
}

U8 *MemPagAlloc(I64 pags,CBlkPool *bp=NULL)
{/*Alloc pags from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It might give you more than you asked for.

Return: NULL if out of memory.
*/
  CMemBlk *res=NULL,*m;
  I64 i;
  if (!bp) bp=sys_code_bp;
  PUSHFD
  CLI
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    PAUSE
  if (pags<MEM_FREE_PAG_HASH_SIZE) {
    if (res=bp->free_pag_hash[pags]) {
      bp->free_pag_hash[pags]=res->next;
      goto at_done;
    }
    i=Bsr(MEM_FREE_PAG_HASH_SIZE)+1;
  } else {
//We'll now round-up to a power of two.
    //There is some overhead on allocations and
    //we wouldn't want to round to the next
    //power of two if a power of two was requested.
    //So we use a little more than a power of two.
    pags-=MEM_EXTRA_HASH2_PAGS;
    i=Bsr(pags)+1;
    pags=1<<i+MEM_EXTRA_HASH2_PAGS;
    if (res=bp->free_pag_hash2[i]) {
      bp->free_pag_hash2[i]=res->next;
      goto at_done;
    }
  }
  m=&bp->mem_free_lst;
  while (TRUE) {
    if (!(res=m->next)) {
//We're probably out of luck, but lets search for a
      //freed larger size block... and, screw-it, return the whole thing.
      do {
	if (res=bp->free_pag_hash2[++i]) {
	  pags=1<<i+MEM_EXTRA_HASH2_PAGS;
	  bp->free_pag_hash2[i]=res->next;
	  goto at_done;
	}
      } while (i<64-MEM_PAG_BITS-1);
      pags=0;
      res=NULL; //Out of memory
      goto at_done2;
    }
    if (res->pags<pags)
      m=res;
    else {
      if (res->pags==pags) {
	m->next=res->next;
	goto at_done;
      } else {
	res->pags-=pags;
	res(U8 *)+=res->pags<<MEM_PAG_BITS;
	res->pags=pags;
	goto at_done;
      }
    }
  }
at_done:
  bp->used_u8s+=res->pags<<MEM_PAG_BITS;
at_done2:
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  POPFD
  return res;
}

U0 MemPagFree(CMemBlk *m,CBlkPool *bp=NULL)
{//Return non-task pags to BlkPool.
  I64 i,pags;
  if (m) {
    if (!bp) bp=sys_code_bp;
    PUSHFD
    CLI
    while (LBts(&bp->locked_flags,BPlf_LOCKED))
      PAUSE
    pags=m->pags;
    m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
    bp->used_u8s-=pags<<MEM_PAG_BITS;
    if (pags<MEM_FREE_PAG_HASH_SIZE) {
      m->next=bp->free_pag_hash[pags];
      bp->free_pag_hash[pags]=m;
    } else {
//We'll now round-up to a power of two.
      //There is some overhead on allocations and
      //we wouldn't want to round to the next
      //power of two if a power of two was requested.
      //So we use a little more than a power of two.
      pags-=MEM_EXTRA_HASH2_PAGS;
      i=Bsr(pags);
      m->next=bp->free_pag_hash2[i];
      bp->free_pag_hash2[i]=m;
    }
    LBtr(&bp->locked_flags,BPlf_LOCKED);
    POPFD
  }
}

CMemBlk *MemPagTaskAlloc(I64 pags,CHeapCtrl *hc)
{/*hc must be locked.  Don't preempt this routine.
Currently, this is only called from $LK,"MAlloc",A="MN:MAlloc"$().
Return: NULL if out of memory.
*/
  CMemBlk *res;
  I64 threshold,cnt,size;
  CMemUnused *uum,**_uum,**_ptr;
  if (res=MemPagAlloc(pags,hc->bp)) {
    QueIns(res,hc->last_mem_blk);
    res->mb_signature=MBS_USED_SIGNATURE_VAL;
    hc->alloced_u8s+=res->pags<<MEM_PAG_BITS;

    //Tidy-up free lst (Move into heap hash)
    //because if free lst gets long, delay causes crash.
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

U0 MemPagTaskFree(CMemBlk *m,CHeapCtrl *hc)
{//hc must be locked
  if (m) {
    PUSHFD
    CLI
    if (m->mb_signature!=MBS_USED_SIGNATURE_VAL)
      SysBadFree(m);
    else {
      QueRem(m);
      hc->alloced_u8s-=m->pags<<MEM_PAG_BITS;
      MemPagFree(m,hc->bp);
    }
    POPFD
  }
}
