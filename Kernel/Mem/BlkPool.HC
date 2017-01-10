U0 BlkPoolAdd(CBlkPool *bp,CMemBlk *m,I64 pags)
{//Add mem to BlkPool.
  if (sys_mem_init_flag)
    MemSet(m,sys_mem_init_val,pags*MEM_PAG_SIZE);
  PUSHFD
  CLI
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    PAUSE
  m->next=bp->mem_free_lst;
  m->pags=pags;
  m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
  bp->alloced_u8s+=pags<<MEM_PAG_BITS;
  bp->mem_free_lst=m;
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  POPFD
}

U0 BlkPoolInit(CBlkPool *bp,I64 pags)
{//Make mem chunk into a BlkPool.
  I64 num;
  CMemBlk *m;
  MemSet(bp,0,sizeof(CBlkPool));
  m=(bp(U8 *)+sizeof(CBlkPool)+MEM_PAG_SIZE-1)&~(MEM_PAG_SIZE-1);
  num=(bp(U8 *)+pags<<MEM_PAG_BITS-m(U8 *))>>MEM_PAG_BITS;
  bp->alloced_u8s=(pags-num)<<MEM_PAG_BITS; //Compensate before num added.
  BlkPoolAdd(bp,m,num);
}

U0 BlkPoolsInit()
{
  I64 i,total,lo,hi,code_heap_limit;
  CMemE820 *m20=MEM_E820;
  Bool first=TRUE;

  total=BIOSTotalMem;

  if (total<=0x80000000)
    code_heap_limit=total;
  else if (total<=0x100000000)
    code_heap_limit=total/4;
  else
    code_heap_limit=0x80000000;

  i=code_heap_limit-SYS_16MEG_AREA_LIMIT; //See $LK,"RLf_16MEG_SYS_CODE_BP",A="FF:::/Kernel/Mem/PageTables.HC,RLf_16MEG_SYS_CODE_BP"$
  BlkPoolAdd(sys_code_bp,SYS_16MEG_AREA_LIMIT,i>>MEM_PAG_BITS);
  mem_heap_limit=i+SYS_16MEG_AREA_LIMIT-1;

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
	    BlkPoolInit(lo,(hi-lo)>>MEM_PAG_BITS);
	    sys_data_bp=lo;
	    Fs->data_heap=HeapCtrlInit(,Fs,sys_data_bp);
	    first=FALSE;
	  } else
	    BlkPoolAdd(sys_data_bp,lo,(hi-lo)>>MEM_PAG_BITS);
	}
      }
      m20++;
    }
  }
  LBts(&sys_run_level,RLf_FULL_HEAPS);
}
