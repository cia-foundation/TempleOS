CHeapCtrl *HeapCtrlInit(CHeapCtrl *hc=NULL,CTask *task=NULL,CBlkPool *bp)
{//Called by $LK,"Adam Task",A="FF:::/Kernel/KStart.CPP,CHeapCtrl.bp"$.
  if (!hc)
    hc=ACAlloc(sizeof(CHeapCtrl));
  hc->hc_signature=HEAP_CTRL_SIGNATURE_VAL;
  hc->mem_task=task;
  hc->bp=bp;
  QueInit(&hc->next_mem_blk);
  hc->next_um=hc->last_um=(&hc->next_um)(U8 *)-offset(CMemUsed.next);
  return hc;
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

U0 HeapCtrlDel(CHeapCtrl *hc)
{//Free all blks alloced to a HeapCtrl.
  CMemBlk *m,*m1;
  if (hc) {
    PUSHFD
    CLI
    while (LBts(&hc->locked_flags,HClf_LOCKED))
      PAUSE
    m=hc->next_mem_blk;
    while (m!=&hc->next_mem_blk) {
      m1=m->next;
      Mem512TaskFree(m,hc);
      m=m1;
    }
    LBtr(&hc->locked_flags,HClf_LOCKED);
    POPFD
    Free(hc);
  }
}
