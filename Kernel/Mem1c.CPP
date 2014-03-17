U0 BlkPoolAdd(CBlkPool *bp,CMemBlk *m,I64 pages512)
{
  if (sys_mem_init_flag)
    MemSet(m,sys_mem_init_val,pages512*512);
  PUSHFD
  CLI
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    PAUSE
  m->next=bp->mem_free_lst;
  m->pages=pages512;
  m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
  bp->size+=pages512;
  bp->mem_free_lst=m;
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  POPFD
}

U0 BlkPoolInit(CBlkPool *bp,I64 pages512)
{
  I64 num;
  CMemBlk *m;
  MemSet(bp,0,sizeof(CBlkPool));
  m=(bp(U8 *)+sizeof(CBlkPool)+PAGE_SIZE-1)&~(PAGE_SIZE-1);
  num=(bp(U8 *)+pages512<<PAGE_BITS-m(U8 *))>>PAGE_BITS;
  bp->size=pages512-num; //compensate before num added.
  BlkPoolAdd(bp,m,num);
}

CHeapCtrl *HeapCtrlInit(CHeapCtrl *hc=NULL,CTask *task=NULL,CBlkPool *bp)
{
//$LK,"Adam Task",A="FF:::/Kernel/KStart.CPP,CHeapCtrl.bp"$
  if (!hc)
    hc=ACAlloc(sizeof(CHeapCtrl));
  hc->hc_signature=HEAP_CTRL_SIGNATURE_VAL;
  hc->mem_task=task;
  hc->bp=bp;
  QueInit(&hc->next_mem_blk);
  hc->next_um=hc->last_um=(&hc->next_um)(U8 *)-offset(CMemUsedAllocated.next);
  return hc;
}

CHeapCtrl *HeapCtrlBPInit(CBlkPool *bp,I64 pages512)
{
  I64 num;
  CMemBlk *m;
  CHeapCtrl *hc;
  MemSet(bp,0,sizeof(CBlkPool)+sizeof(CHeapCtrl));
  hc=HeapCtrlInit(bp(U8 *)+sizeof(CBlkPool),,bp);
  m=(bp(U8 *)+sizeof(CBlkPool)+sizeof(CHeapCtrl)+PAGE_SIZE-1)&~(PAGE_SIZE-1);
  num=(bp(U8 *)+pages512<<PAGE_BITS-m(U8 *))>>PAGE_BITS;
  bp->size=pages512-num;
  BlkPoolAdd(bp,m,num);
  return hc;
}

U0 HeapCtrlDel(CHeapCtrl *hc)
{
  CMemBlk *m,*m1;
  if (hc) {
    PUSHFD
    CLI
    while (LBts(&hc->locked_flags,HClf_LOCKED))
      PAUSE
    m=hc->next_mem_blk;
    while (m!=&hc->next_mem_blk) {
      m1=m->next;
      MemBlksFree(m,hc);
      m=m1;
    }
    LBtr(&hc->locked_flags,HClf_LOCKED);
    POPFD
    Free(hc);
  }
}

Bool Mem32DevIns(CMemRange *tempmr)
{
  CMemRange *tempmr1=sys_mem32_dev_root.next,*tempmr2;
  while (tempmr1!=&sys_mem32_dev_root) {
    if (!tempmr1->type && tempmr->base>=tempmr1->base &&
	tempmr->base+tempmr->size<=tempmr1->base+tempmr1->size) {
      if (tempmr->base>tempmr1->base) {
	tempmr2=AMAlloc(sizeof(CMemRange));
	tempmr2->type=MRT_UNUSED;
	tempmr2->flags=0;
	tempmr2->base=tempmr1->base;
	tempmr2->size=tempmr->base-tempmr1->base;
	QueInsRev(tempmr2,tempmr1);
      }
      QueInsRev(tempmr,tempmr1);
      tempmr1->size=tempmr1->base+tempmr1->size-
		    (tempmr->base+tempmr->size);
      tempmr1->base=tempmr->base+tempmr->size;
      if (!tempmr1->size) {
	QueRem(tempmr1);
	Free(tempmr1);
      }
      return TRUE;
    }
    tempmr1=tempmr1->next;
  }
  return FALSE;
}

U0 Mem32DevInit()
{
  CMemRange *tempmr;
  CMemE820 *m20=SYS_MEM_E820;

  QueInit(&sys_mem32_dev_root);
  tempmr=AMAlloc(sizeof(CMemRange));
  tempmr->type=MRT_UNUSED;
  tempmr->flags=0;
//Maybe !!! Change this to 0xE0000000 !!!
  tempmr->base=0xF0000000;
  tempmr->size=0x10000000;
  QueIns(tempmr,sys_mem32_dev_root.last);

  if (m20->type) {
    while (m20->type) {
      tempmr=AMAlloc(sizeof(CMemRange));
      tempmr->type=m20->type;
      tempmr->flags=0;
      tempmr->base=m20->base;
      tempmr->size=m20->len;
      if (!Mem32DevIns(tempmr))
	Free(tempmr);
      m20++;
    }
  }
}

Bool MemLowPagesProtect(Bool val=TRUE)
{ //FALSE if any were not protected
  U8 *a=0;
  Bool result=TRUE;
  while (a<=MEM_PROTECTED_LOW_LIMIT) {
    result&=!MemPagePresentMark(a,!val);
    a+=MemPageSize(a);
  }
  return result;
}

U0 MemPagesNotPresentMark()
{
  U8 *a,*max_physical=NULL;
  U16		*m01=SYS_MEM_E801;
  CMemE820	*m20=SYS_MEM_E820;
  MemLowPagesProtect;
  while (m20->type) {
    a=m20->base+m20->len;
    if (a>max_physical)
      max_physical=a;
    m20++;
  }
  if (max_physical>=0x1000000+m01[1]<<16) {
    a=(max_physical+0x1FFFFF)&~0x1FFFFF;
    while (a<MEM_MAPPED_SPACE) {
      MemPagePresentMark(a,FALSE);
      a+=MemPageSize(a);
    }
  }
}

U8 *Mem32DevAlloc(I64 size,I64 alignment)
{//This doesn't work
  U8 *base,*limit;
  CMemRange *tempmr,*tempmr1;
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  tempmr1=sys_mem32_dev_root.next;
  while (tempmr1!=&sys_mem32_dev_root) {
    base=(tempmr1->base+alignment-1)&~(alignment-1);
    limit=base+size-1;
    if (!tempmr1->type &&
	limit<tempmr1->base+tempmr1->size) {
      tempmr=AMAlloc(sizeof(CMemRange));
      tempmr->type=MRT_DEV;
      tempmr->flags=0;
      tempmr->base=base;
      tempmr->size=size;
      if (!Mem32DevIns(tempmr)) {
	Free(tempmr);
	LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
	return NULL;
      }
      LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
      return tempmr->base;
    }
    tempmr1=tempmr1->next;
  }
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
  return NULL;
}

U0 Mem32DevFree(U8 *base)
{
  CMemRange *tempmr;
  if (!base) return;
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  tempmr=sys_mem32_dev_root.next;
  while (tempmr!=&sys_mem32_dev_root) {
    if (tempmr->base==base) {
      tempmr->type=MRT_UNUSED;
      break;
    }
    tempmr=tempmr->next;
  }
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
}

U8 *Mem64DevAlloc(I64 *_pages2Meg)
{
  U8 *result;
  I64 i=*_pages2Meg,*pte;
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  while (i--) {
    sys_mem64_dev_ptr-=0x200000;
    pte=MemPageTable(sys_mem64_dev_ptr);
    *pte=*pte&~0x18 |0x11; //Uncached and present
    InvlPg(sys_mem64_dev_ptr);
  }
  result=sys_mem64_dev_ptr;
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
  return result;
}

U0 Mem64DevFree(U8 *base,I64 pages2Meg)
{
  if (!base) return;
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  if (base==sys_mem64_dev_ptr)
    sys_mem64_dev_ptr+=pages2Meg*0x200000;
  //else not freed
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
}

I64 BIOSTotalMem()
{
  I64 r01,r20;
  U16		*m01=SYS_MEM_E801;
  CMemE820	*m20=SYS_MEM_E820;

  r01=0x100000+m01[0]<<10+m01[1]<<16;
  r20=0;
  if (m20->type) {
    while (m20->type) {
      if (m20->type==1)
	r20+=m20->len;
      m20++;
    }
  }
  return MaxI64(r01,r20);
}

U0 HeapsInit()
{
  I64 i,total,lo,hi,code_heap_limit;
  CMemE820	*m20=SYS_MEM_E820;
  Bool first=TRUE;

  total=BIOSTotalMem;

  if (total<=0x80000000)
    code_heap_limit=total;
  else if (total<=0x100000000)
    code_heap_limit=total/4;
  else
    code_heap_limit=0x80000000;

  i=code_heap_limit-0x1000000;
  BlkPoolAdd(sys_code_bp,0x1000000,i>>PAGE_BITS);
  sys_heap_limit=i+0x1000000-1;

  if (code_heap_limit<total) {
    while (m20->type) {
      if (m20->type==1) {
	lo=m20->base;
	hi=m20->base+m20->len;
	if (lo<code_heap_limit) {
	  if (hi>code_heap_limit)
	    lo=code_heap_limit;
	  else
	    hi=lo; //cancel
	}
	if (code_heap_limit<=lo<hi) {
	  if (first) {
	    BlkPoolInit(lo,(hi-lo)>>PAGE_BITS);
	    sys_data_bp=lo;
	    Fs->data_heap=HeapCtrlInit(,Fs,sys_data_bp);
	    first=FALSE;
	  } else
	    BlkPoolAdd(sys_data_bp,lo,(hi-lo)>>PAGE_BITS);
	}
      }
      m20++;
    }
  }
}

I64 Scale2Mem(I64 min,I64 max,I64 limit=2*1024*1024*1024)
//This function is nice for scaling-back allocations if less than 2048 Meg.
//Disk cache and RAM Drive expressions can include this function.
{
  I64 i=sys_code_bp->size<<PAGE_BITS;
  if (i>=limit)
    return max;
  else
    return min+(max-min)*i/limit;
}
