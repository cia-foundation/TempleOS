// Requires $LK,"::/Demo/Lectures/Mem/Mem2Meg.HC"$.

Cd(__DIR__);;
#include "Mem2Meg"

#define SAMPLE_SIZE	100000
U0 TimeIns()
{
  I64 i,start,end;
  I64 *cached_I64=MAlloc(sizeof(I64)),
	shared_blks,*uncached_I64;
  CBlkPool *bp;
  CHeapCtrl *hc;

  "$$RED$$Now, this demo seems broken, perhaps,\n"
	"because I run in a virtual machine.$$FG$$\n\n";
  CPURep;

  shared_blks=1;
  bp=Mem2MegUncachedAlloc(&shared_blks);
  hc=HeapCtrlBPInit(bp,shared_blks<<12);
  uncached_I64=MAlloc(sizeof(I64),hc);

  //Measure cached time
  start=GetTSC;
  for (i=0;i<SAMPLE_SIZE;i++)
    *cached_I64=i;

  end=GetTSC;
  "  Cached Cycles: %10,d\n",end-start;

  //Measure uncached time
  start=GetTSC;
  for (i=0;i<SAMPLE_SIZE;i++)
    *uncached_I64=i;
  end=GetTSC;
  "Uncached Cycles: %10,d\n",end-start;

  "$$GREEN$$The difference is even greater\n"
	"because the loop overhead was not subtracted.$$FG$$\n\n";
  Free(cached_I64);
  Free(uncached_I64);
  Mem2MegFree(bp,shared_blks);
}

TimeIns;
