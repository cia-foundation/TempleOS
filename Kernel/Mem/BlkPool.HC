U0 BlkPoolAdd(CBlkPool *bp,CMemBlk *m,I64 pages512)
{//Add mem to BlkPool.
  if (sys_mem_init_flag)
    MemSet(m,sys_mem_init_val,pages512*512);
  PUSHFD
  CLI
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    PAUSE
  m->next=bp->mem_free_lst;
  m->pages=pages512;
  m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
  bp->alloced_u8s+=pages512<<MEM_PAGE_BITS;
  bp->mem_free_lst=m;
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  POPFD
}

U0 BlkPoolInit(CBlkPool *bp,I64 pages512)
{//Make mem chunk into a BlkPool.
  I64 num;
  CMemBlk *m;
  MemSet(bp,0,sizeof(CBlkPool));
  m=(bp(U8 *)+sizeof(CBlkPool)+MEM_PAGE_SIZE-1)&~(MEM_PAGE_SIZE-1);
  num=(bp(U8 *)+pages512<<MEM_PAGE_BITS-m(U8 *))>>MEM_PAGE_BITS;
  bp->alloced_u8s=(pages512-num)<<MEM_PAGE_BITS; //Compensate before num added.
  BlkPoolAdd(bp,m,num);
}

U0 BlkPoolsInit()
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
  BlkPoolAdd(sys_code_bp,0x1000000,i>>MEM_PAGE_BITS);
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
	    BlkPoolInit(lo,(hi-lo)>>MEM_PAGE_BITS);
	    sys_data_bp=lo;
	    Fs->data_heap=HeapCtrlInit(,Fs,sys_data_bp);
	    first=FALSE;
	  } else
	    BlkPoolAdd(sys_data_bp,lo,(hi-lo)>>MEM_PAGE_BITS);
	}
      }
      m20++;
    }
  }
}
