#define SAMPLE_SIZE	100000

U0 TimeIns()
{
  I64 i,start,end;
  I64 *cached_I64=MAlloc(sizeof(I64)),
     shared_blks,*uncached_I64;
  CBlkPool *bp;
  CHeapCtrl *hc;
  Bool old_preempt;

  "$$FG,RED$$Now, this demo seems broken, perhaps,\n"
  "because I run in a virtual machine.$$FG$$\n\n";
  CPURep;

  shared_blks=1;
  bp=MemBlksUncachedAlloc(&shared_blks);
  hc=HeapCtrlIndependentInit(bp,shared_blks<<12);
  uncached_I64=MAlloc(sizeof(I64),hc);

  old_preempt=Preempt(OFF);

//Measure cached time
  start=GetTimeStamp;
  for (i=0;i<SAMPLE_SIZE;i++)
    *cached_I64=i;

  end=GetTimeStamp;
  "  Cached Cycles: %10,d\n",end-start;

//Measure uncached time
  start=GetTimeStamp;
  for (i=0;i<SAMPLE_SIZE;i++)
    *uncached_I64=i;
  end=GetTimeStamp;
  "Uncached Cycles: %10,d\n",end-start;

  "$$FG,GREEN$$The difference is even greater\n"
  "because the loop overhead was not subtracted.$$FG$$\n\n";
  Free(cached_I64);
  Free(uncached_I64);
  MemBlks2MegFree(bp,shared_blks);
  Preempt(old_preempt);
}

TimeIns;

/*  Program Output$HL,0$$WW+H,1$$FD,1$

8 Cores 2.660GHz
  Cached Cycles:    202,220
Uncached Cycles: 14,357,016
$FG,2$The difference is even greater
because the loop overhead was not subtracted.
$HL,1$*/
