U0 MemDemo()
{
  I64 i;
"$$HL,1$$$$BLACK$$
#define MEM_PAG_BITS		9
#define MEM_PAG_SIZE		(1<<MEM_PAG_BITS)
#define MEM_HEAP_HASH_SIZE	1024
#define MEM_FREE_PAG_HASH_SIZE	0x100
#define MEM_EXTRA_HASH2_PAGS	2

public class CBlkPool
{
  CMemBlk *free_pag_hash[MEM_FREE_PAG_HASH_SIZE],
	  *free_pag_hash2[64-MEM_PAG_BITS];
};

public class CHeapCtrl
{
  CBlkPool *bp;
  CMemUnused *heap_hash[MEM_HEAP_HASH_SIZE/sizeof(U8 *)];
};

public class CTask //The Fs segment reg points to current CTask.
{
  CHeapCtrl *code_heap,*data_heap;
};
$$HL,0$$$$FG$$

";


  "$$PURPLE$$Chunks in increments of 8 bytes, owned by the task.$$FG$$\n";
  for (i=sizeof(CMemUnused);i<MEM_HEAP_HASH_SIZE;i+=8)
    if (i==0x48)
      "...\n";
    else if (!(0x48<i<0x3C8))
      "Fs->data_heap->heap_hash[%X/8]"
	  "$$BLACK$$ -->$$FG$$[X]$$BLACK$$ -->$$FG$$[X]$$BLACK$$ -->$$FG$$\n",i;
  "\n\n";

  "$$PURPLE$$Pages of 512 bytes, owned by the system.$$FG$$\n";
  for (i=MEM_HEAP_HASH_SIZE;i<MEM_FREE_PAG_HASH_SIZE*MEM_PAG_SIZE;
	i+=MEM_PAG_SIZE)
    if (i==0x1200)
      "...\n";
    else if (!(0x1200<i<0x1F200))
      "Fs->data_heap->bp->free_pag_hash[%X/512]"
	  "$$BLACK$$ -->$$FG$$[X]$$BLACK$$ -->$$FG$$[X]$$BLACK$$ -->$$FG$$\n",i;
  "\n\n";

  "$$PURPLE$$Powers of two, owned by the system.$$FG$$\n";
  for (i=Bsf(MEM_FREE_PAG_HASH_SIZE*MEM_PAG_SIZE);i<64;i++)
    if (i==0x1A)
      "...\n";
    else if (!(0x1A<=i<=0x36))
      "Fs->data_heap->bp->free_pag_hash2[%X] %016X"
	  "$$BLACK$$ -->$$FG$$[X]$$BLACK$$ -->$$FG$$[X]$$BLACK$$ -->$$FG$$\n",i,
	  1<<i+MEM_EXTRA_HASH2_PAGS*MEM_PAG_SIZE;
  "\n\n";
}

MemDemo;


