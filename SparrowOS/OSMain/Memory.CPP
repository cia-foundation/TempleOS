asm {
/*
See $LK,"::/SparrowOS/Doc/Acknowledgements.TXT.Z","FI:::/SparrowOS/Doc/Acknowledgements.TXT.Z"$ (3).
See $LK,"::/SparrowOS/Doc/Mem.TXT.Z","FI:::/SparrowOS/Doc/Mem.TXT.Z"$
*/

USE32
SYS_INIT_MEM::
//Set page tables to identity map everything.
	MOV	EDI,PAGE_TABLE_BASE
	XOR	EAX,EAX
	MOV	ECX,0x1000*(NUM_PML1+NUM_PML2+NUM_PML3+NUM_PML4)/4
	REP_STOSD

//PML1
	MOV	EAX,0x107
	XOR	EDX,EDX
	MOV	EDI,PAGE_TABLE_BASE
	MOV	ECX,0x200*NUM_PML1
@@05:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x1000
	ADC	EDX,0
	LOOP	@@05

//video ram=write through
//0xA0000-0xBFFFF
	MOV	EAX,0xF
	XOR	EDX,EDX
	MOV	EDI,PAGE_TABLE_BASE+0xA0*8
	MOV	ECX,0xC0-0xA0
@@10:	OR	U32 [EDI],EAX
	ADD	EDI,4
	OR	U32 [EDI],EDX
	ADD	EDI,4
	LOOP	@@10

//PML2
	MOV	EAX,PAGE_TABLE_BASE+7
	XOR	EDX,EDX
	MOV	EDI,PAGE_TABLE_BASE+0x1000*NUM_PML1
	MOV	ECX,NUM_PML1
@@15:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x1000
	ADC	EDX,0
	LOOP	@@15

	MOV	EAX,NUM_PML1*0x200000+0x87 //bit 7 is page size (1==2Meg)
	XOR	EDX,EDX
	MOV	EDI,PAGE_TABLE_BASE+0x1000*NUM_PML1+8*NUM_PML1
	MOV	ECX,NUM_PML2*0x200-NUM_PML1
@@20:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x200000
	ADC	EDX,0
	LOOP	@@20

//PCI Devs
//0xE0000000-0xFFFFFFFF
	MOV	EAX,0x17	//Cache disabled
	XOR	EDX,EDX
	MOV	EDI,PAGE_TABLE_BASE+0x1000*NUM_PML1+8*0x700
	MOV	ECX,0x800-0x700
@@25:	OR	U32 [EDI],EAX
	ADD	EDI,4
	OR	U32 [EDI],EDX
	ADD	EDI,4
	LOOP	@@25

//PML3
	MOV	EAX,PAGE_TABLE_BASE+NUM_PML1*0x1000+7
	XOR	EDX,EDX
	MOV	EDI,PAGE_TABLE_BASE+0x1000*(NUM_PML1+NUM_PML2)
	MOV	ECX,NUM_PML2
@@30:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x1000
	ADC	EDX,0
	LOOP	@@30

//PML4
	MOV	EAX,PAGE_TABLE_BASE+(NUM_PML1+NUM_PML2)*0x1000+7
	XOR	EDX,EDX
	MOV	EDI,PAGE_TABLE_BASE+0x1000*(NUM_PML1+NUM_PML2+NUM_PML3)
	MOV	ECX,1
@@35:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x1000
	ADC	EDX,0
	LOOP	@@35

	MOV	U32 [SYS_CODE_BP],SYS_BP_START
	MOV	U32 [SYS_CODE_BP+4],0

	MOV	U32 [SYS_DATA_BP],0
	MOV	U32 [SYS_DATA_BP+4],0

	MOV	U32 [SYS_BP_LOCKED_FLAGS],0
	MOV	U32 [SYS_BP_LOCKED_FLAGS+4],0
	MOV	U32 [SYS_BP_SIZE],0
	MOV	U32 [SYS_BP_SIZE+4],0
	MOV	U32 [SYS_BP_ALLOCATED_U8S],0
	MOV	U32 [SYS_BP_ALLOCATED_U8S+4],0

	MOV	EDI,U32 SYS_BP_FREE_PAGE_HASH
	MOV	ECX,FREE_PAGE_HASH_SIZE*2
	XOR	EAX,EAX
	REP_STOSD
	MOV	EDI,U32 SYS_BP_FREE_PAGE_HASH2
	MOV	ECX,(64-PAGE_BITS)*2
	REP_STOSD

	MOV	U32 [SYS_BP_MEM_FREE_LIST],0
	MOV	U32 [SYS_BP_MEM_FREE_LIST+4],0
	MOV	U32 [SYS_BP_MEM_FREE_2MEG_LIST],0
	MOV	U32 [SYS_BP_MEM_FREE_2MEG_LIST+4],0

	XOR	EAX,EAX
	MOV	AX,U16 [SYS_MEM_E801] //1 Kb blks between 1M and 16M
	SHL	EAX,10
	ADD	EAX,0x100000
	MOV	EBX,U32 [SYS_HEAP_BASE] //0x183000
	SUB	EAX,EBX

/*$AN,"ExpandHeap","ExpandHeap"$
SparrowOS initializes the heap to the val
of the BIOS E801 lowest 16Meg val.  It
later adds the rest to the heap.  There
are a few allocations which occur while
it is set to this small val.

At some point, it might be nice to
initialize to the full val but it is a
pain without MAlloc up-and-running.
*/

#define CONSERVATIVE_16MEG_SIZE	0xF00000
#define ESTIMATED_MISC_ALLOCS	0x100000

#assert CONSERVATIVE_16MEG_SIZE-0x183000>ADAM_STK+ESTIMATED_MISC_ALLOCS

//EBX=BASE EAX=SIZE
	TEST	U8 [SYS_MEM_INIT_FLAG],1
	JZ	@@40
	PUSH	EAX
	MOV	EDI,EBX
	MOV	ECX,EAX
	MOV	AL,U8 [SYS_MEM_INIT_VAL]
	REP_STOSB
	POP	EAX

@@40:	SHR	EAX,PAGE_BITS
	MOV	EDI,U32 [SYS_BP_MEM_FREE_LIST]
	MOV	U32 CMemBlk.next[EBX],EDI
	MOV	U32 CMemBlk.next+4[EBX],0
	MOV	U32 [SYS_BP_MEM_FREE_LIST],EBX
	MOV	U32 [SYS_BP_MEM_FREE_LIST+4],0
	MOV	U32 CMemBlk.mb_signature[EBX],MBS_UNUSED_SIGNATURE_VAL
	MOV	U32 CMemBlk.pages[EBX],EAX
	ADD	U32 [SYS_BP_SIZE],EAX
	RET
}

U0 SysBadFree(I64 *ptr)
{
  Dbg("Bad Free:",ptr);
}

U0 SysBadMAlloc(I64 *ptr)
{
  Dbg("Bad MAlloc:",ptr);
}

I64 MemPageSize(U8 *a)
{
  if (a<NUM_PML1*0x1000*0x200)
    return 0x1000;
  else
    return 0x200000;
}

I64 *MemPtAtPageTableEntry(U8 *a)
{
  if (a<NUM_PML1*0x1000*0x200)
    return PAGE_TABLE_BASE+ a>>12 *8;
  else
    return PAGE_TABLE_BASE+NUM_PML1*0x1000+
	   a>>21*8;
}

Bool MemPagePresentMark(U8 *a,Bool val)
{
  I64 *pte=MemPtAtPageTableEntry(a);
  Bool result=*pte&1;
  if (val)
    *pte=*pte|1;  //Mark present
  else
    *pte=*pte&~1; //Mark not present
  InvlPg(a);
  return result;
}


U8 *MemBlksNonTaskAlloc(I64 *_pages512,CBlkPool *bp=NULL)
//This will allocate a 512 byte pages from
//and not link them to any task.  (Linking to a task
//means it will be freed when the task
//dies.)
//It might give you more than you asked for
//so a ptr to a page count is passed.
//
//Returns NULL if out of memory
{
  CMemBlk *result=NULL,*m;
  I64 i,num=*_pages512;
  if (!bp) bp=sys_code_bp;
  PushFD;
  Cli;
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    Pause;
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
  m=&bp->mem_free_list;
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
  bp->allocated_u8s+=num<<PAGE_BITS;
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  PopFD;
  return result;
}

U0 MemBlksNonTaskFree(CMemBlk *m,I64 pages512,CBlkPool *bp=NULL)
{
  I64 i;
  if (m) {
    if (!bp) bp=sys_code_bp;
    PushFD;
    Cli;
    while (LBts(&bp->locked_flags,BPlf_LOCKED))
      Pause;
    m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
    m->pages=pages512;
    bp->allocated_u8s-=pages512<<PAGE_BITS;
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
    PopFD;
  }
}

U8 *MemBlks2MegAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
//This will allocate 2Meg pages and not
//link it to any task.	(Linking to a task
//means they will be freed when the task
//dies.)
//It might give you more than you asked for
//so a ptr to a page count is passed.
//
//Returns NULL if out of memory
{
  I64 i,j,*pte,num=*_pages2Meg;
  CMemBlk *result=NULL,*m,*m1;

  if (!bp) bp=sys_code_bp;
  PushFD;
  Cli;
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    Pause;
  num<<=21-PAGE_BITS;

  m=&bp->mem_free_2meg_list;
  while (TRUE) {
    if (!(result=m->next))
      break;
    if (result->pages<num)
      m=result;
    else {
      if (result->pages==num) {
	m->next=result->next;
	goto am_done;
      } else {
	result->pages-=num;
	result(U8 *)+=result->pages<<PAGE_BITS;
	result->pages=num;
	goto am_done;
      }
    }
  }

  m=&bp->mem_free_list;
  while (TRUE) {
    if (!(result=m->next)) {
      num=0;
      result=NULL; //Out of memory
      goto am_done;
    }
    if (result->pages<num)
      m=result;
    else {
      if (result->pages==num) {
	if (result(U8 *)&0x1FFFFF)
	  m=result;
	else {
	  m->next=result->next;
	  goto am_done;
	}
      } else {
	if (i=(result(U8 *)&0x1FFFFF)>>PAGE_BITS) {
	  j=1<<(21-PAGE_BITS)-i;
	  if (result->pages<num+j)
	    m=result;
	  else if (result->pages==num+j) {
	    result->pages-=num;
	    result(U8 *)+=result->pages<<PAGE_BITS;
	    result->pages=num;
	    goto am_done;
	  } else {
	    m1=result;
	    result(U8 *)+=j<<PAGE_BITS;
	    result->pages=num;
	    m=result(U8 *)+num<<PAGE_BITS;
	    m->pages=m1->pages-num-j;
	    m1->pages=j;
	    m->next=m1->next;
	    m1->next=m;
	    m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
	    goto am_done;
	  }
	} else {
	  m=m->next=result(U8 *)+num<<PAGE_BITS;
	  m->next=result->next;
	  m->pages=result->pages-num;
	  m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
	  result->pages=num;
	  goto am_done;
	}
      }
    }
  }
am_done:
  i=num<<PAGE_BITS;
  bp->allocated_u8s+=i;
  num>>=21-PAGE_BITS;
  *_pages2Meg=num;
  m=result;
  m1=m(U8 *)+i;
  while (m<m1) {
    pte=MemPtAtPageTableEntry(m);
    *pte &= ~0x18;
    InvlPg(m);
    m(U8 *)+=MemPageSize(m);
  }
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  PopFD;
  return result;
}

U8 *MemBlksUncachedAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
//This will allocate 2Meg pages from
//and not link them to any task.  (Linking to a task
//means it will be freed when the task
//dies.)  It will be marked uncached.
//It might give you more than you asked for
//so a ptr to a page count is passed.
//
//Returns NULL if out of memory
{
  CMemBlk *result,*m,*m1;
  I64 num=*_pages2Meg,*pte;
  if (result=MemBlks2MegAlloc(_pages2Meg,bp)) {
    num=*_pages2Meg;
    m=result;
    m1=m(U8 *)+num<<21;
    while (m<m1) {
      pte=MemPtAtPageTableEntry(m);
      *pte= *pte& ~0x18 |0x10;
      InvlPg(m);
      m(U8 *)+=MemPageSize(m);
    }
  }
  return result;
}

U8 *MemBlksWriteThroughAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
//This will allocate 2Meg pages from
//and not link them to any task.  (Linking to a task
//means they will be freed when the task
//dies.)  It will be marked write-through.
//It might give you more than you asked for
//so a ptr to a page count is passed.
//
//Returns NULL if out of memory
{
  CMemBlk *result,*m,*m1;
  I64 num=*_pages2Meg,*pte;
  if (result=MemBlks2MegAlloc(_pages2Meg,bp)) {
    num=*_pages2Meg;
    m=result;
    m1=m(U8 *)+num<<21;
    while (m<m1) {
      pte=MemPtAtPageTableEntry(m);
      *pte= *pte& ~0x18 |8;
      InvlPg(m);
      m(U8 *)+=MemPageSize(m);
    }
  }
  return result;
}

U0 MemBlks2MegFree(CMemBlk *m,I64 pages2Meg,CBlkPool *bp=NULL)
{
  I64 *pte;
  CMemBlk *m1,*m2;
  if (m) {
    if (!bp) bp=sys_code_bp;
    m2=m;
    m1=m(U8 *)+pages2Meg<<21;
    while (m2<m1) {
      pte=MemPtAtPageTableEntry(m2);
      *pte=*pte & ~0x18;
      InvlPg(m2);
      m2(U8 *)+=MemPageSize(m2);
    }
    PushFD;
    Cli;
    while (LBts(&bp->locked_flags,BPlf_LOCKED))
      Pause;
    m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
    m->pages=pages2Meg<<(21-PAGE_BITS);
    bp->allocated_u8s-=pages2Meg<<21;
    m->next=bp->mem_free_2meg_list;
    bp->mem_free_2meg_list=m;
    LBtr(&bp->locked_flags,BPlf_LOCKED);
    PopFD;
  }
}

CMemBlk *MemBlksAlloc(I64 *_pages512,CHeapCtrl *hc)
{
//hc must be locked.  Interrupts should probably be off
//Currently, this is only called from $LK,"MAlloc","MN:MAlloc"$().
//
//Returns NULL if out of memory
  CMemBlk *result;
  I64 threshold,cnt,size;
  CMemUnusedAllocated *uum,**_uum,**_ptr;
  if (result=MemBlksNonTaskAlloc(_pages512,hc->bp)) {
    if (!Bt(&hc->flags,HCf_NON_TASK_QUE))
      QueIns(result,hc->last_mem_blk);
    result->mb_signature=MBS_USED_SIGNATURE_VAL;

    //Tidy-up free list (Move into heap hash)
    //because if free list gets long, delay causes crash
    threshold=HEAP_HASH_SIZE>>4;
#assert HEAP_HASH_SIZE>>4>=sizeof(U8 *)
    do {
      cnt=0;
      _uum=&hc->malloc_free_list;
      while (uum=*_uum) {
#assert !offset(CMemUnusedAllocated.next)
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

U0 MemBlksFree(CMemBlk *m,CHeapCtrl *hc)
{ //hc must be locked
  if (m) {
    PushFD;
    Cli;
    if (m->mb_signature!=MBS_USED_SIGNATURE_VAL)
      SysBadFree(m);
    else {
      if (!Bt(&hc->flags,HCf_NON_TASK_QUE))
	QueRem(m);
      MemBlksNonTaskFree(m,m->pages,hc->bp);
    }
    PopFD;
  }
}


U0 MemBlkListFree(CHeapCtrl *hc)
{
  CMemBlk *m,*m1;
  PushFD;
  Cli;
  while (LBts(&hc->locked_flags,HClf_LOCKED))
    Pause;
  m=hc->next_mem_blk;
  while (m!=&hc->next_mem_blk) {
    m1=m->next;
    MemBlksFree(m,hc);
    m=m1;
  }
  LBtr(&hc->locked_flags,HClf_LOCKED);
  PopFD;
}


asm {
USE64
// ************************************
// Throws EXCEPT_OUT_OF_MEM
_MALLOC::
	ENTER	0
	PUSH	RSI
	PUSH	RDI

	XOR	RBX,RBX
	MOV	RDX,U64 SF_ARG2[RBP]
	OR	RDX,RDX
	JNZ	@@05
	MOV	RDX,U64 FS:CTask.addr[RBX]
@@05:	CMP	U32 CTask.task_signature[RDX],TASK_SIGNATURE_VAL

#assert CTask.task_signature==CHeapCtrl.hc_signature //location of signature matches

	JNE	@@10
	MOV	RDX,U64 CTask.data_heap[RDX]
@@10:	CMP	U32 CHeapCtrl.hc_signature[RDX],HEAP_CTRL_SIGNATURE_VAL
	JE	@@15
	PUSH	RDX
	CALL	&SysBadMAlloc
	JMP	I32 _SYS_HLT

@@15:	MOV	RAX,U64 SF_ARG1[RBP]
	PUSHFD
	ADD	RAX,CMemUsedAllocated.start+7	//round-up to I64
	AND	AL,0xF8
#assert CMemUsedAllocated.start>=sizeof(CMemUnusedAllocated)
	CMP	RAX,CMemUsedAllocated.start
	JAE	@@20
	MOV	RAX,CMemUsedAllocated.start
@@20:

	CLI
@@25:	LOCK
	BTS	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	PAUSE	//don't know if this instruction helps
	JC	@@25

	CMP	RAX,HEAP_HASH_SIZE
	JAE	@@30
	MOV	RSI,U64 CHeapCtrl.heap_hash[RAX+RDX]
	OR	RSI,RSI
	JZ	@@35
	MOV	RCX,U64 CMemUnusedAllocated.next[RSI]
	MOV	U64 CHeapCtrl.heap_hash[RAX+RDX],RCX
	JMP	I32 MALLOC_ALMOST_DONE

//Big allocation
@@30:	ADD	RAX,sizeof(CMemBlk)+PAGE_SIZE-1
	SHR	RAX,PAGE_BITS

	PUSH	RDX //preserve heap ctrl
	PUSH	RAX //num blks (we must pass address of this)
	MOV	RAX,RSP
	PUSH	RDX
	PUSH	RAX //addr of num blks
	CALL	&MemBlksAlloc
	MOV	RSI,RAX
	POP	RAX //blks that were allocated
	POP	RDX
	OR	RSI,RSI
	JZ	@@45	//Out of memory

	SHL	RAX,PAGE_BITS
	SUB	RAX,sizeof(CMemBlk)
	ADD	RSI,sizeof(CMemBlk)
	JMP	I32 MALLOC_ALMOST_DONE

//Little allocation, chunk-off piece from free list chunks
@@35:	LEA	RSI,U64 CHeapCtrl.malloc_free_list-CMemUnusedAllocated.next[RDX]

@@40:	MOV	RBX,RSI
	MOV	RSI,U64 CMemUnusedAllocated.next[RBX]
	OR	RSI,RSI
	JNZ	@@55
	PUSH	RAX		//-**** save byte size
	ADD	RAX,16*PAGE_SIZE-1
	SHR	RAX,PAGE_BITS

	PUSH	RDX //preserve heap ctrl
	PUSH	RAX //num blks (we must pass address of this)
	MOV	RAX,RSP
	PUSH	RDX
	PUSH	RAX //addr of num blks
	CALL	&MemBlksAlloc
	MOV	RSI,RAX
	POP	RAX //blks that were allocated
	POP	RDX
	OR	RSI,RSI
	JNZ	@@50

//Out of memory
@@45:	LOCK
	BTR	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	POPFD
	PUSH	EXCEPT_OUT_OF_MEM
	PUSH	1
	CALL	I32 &throw_no_log
	JMP	I32 MALLOC_FINAL_EXIT

@@50:	LEA	RSI,U64 sizeof(CMemBlk)[RSI]
	SHL	RAX,PAGE_BITS
	SUB	RAX,sizeof(CMemBlk)
	LEA	RBX,U64 CHeapCtrl.malloc_free_list-CMemUnusedAllocated.next[RDX]
	MOV	RDI,U64 CMemUnusedAllocated.next[RBX]
	MOV	U64 CMemUnusedAllocated.next[RSI],RDI
	MOV	U64 CMemUnusedAllocated.size[RSI],RAX
	MOV	U64 CMemUnusedAllocated.next[RBX],RSI
	POP	RAX		//+****
	JMP	@@65
@@55:	CMP	U64 CMemUnusedAllocated.size[RSI],RAX
	JB	@@40
	JNE	@@65

@@60:	MOV	RDI,U64 CMemUnusedAllocated.next[RSI]
	MOV	U64 CMemUnusedAllocated.next[RBX],RDI
	JMP	MALLOC_ALMOST_DONE

@@65:	SUB	U64 CMemUnusedAllocated.size[RSI],RAX	//UPDATE FREE ENTRY
	CMP	U64 CMemUnusedAllocated.size[RSI],sizeof(CMemUnusedAllocated)
	JAE	@@70			//take from top of block
	ADD	U64 CMemUnusedAllocated.size[RSI],RAX	//doesn't fit, undo
	JMP	I32 @@40

@@70:	ADD	RSI,U64 CMemUnusedAllocated.size[RSI]


MALLOC_ALMOST_DONE:
//RSI=result-CMemUsedAllocated.size
//RAX=size+CMemUsedAllocated.size
//RDX=heap ctrl
	ADD	U64 CHeapCtrl.used_u8s[RDX],RAX

#if _CFG_HEAP_DBG
//QueIns
	MOV	RDI,U64 CHeapCtrl.last_um[RDX]
	MOV	U64 CMemUsedAllocated.next[RDI],RSI
	MOV	U64 CHeapCtrl.last_um[RDX],RSI
	MOV	U64 CMemUsedAllocated.last[RSI],RDI
	LEA	RDI,U64 CHeapCtrl.next_um-CMemUsedAllocated.next[RDX]
	MOV	U64 CMemUsedAllocated.next[RSI],RDI

//Caller1/Caller2
	PUSH	RDX
	MOV	RDX,U64 [SYS_HEAP_LIMIT]
	MOV	RDI,U64 SF_RIP[RBP]
	CMP	RDI,RDX
	JB	@@75
	XOR	RDI,RDI
	MOV	U64 CMemUsedAllocated.caller1[RSI],RDI
	JMP	@@85
@@75:	MOV	U64 CMemUsedAllocated.caller1[RSI],RDI
	MOV	RDI,U64 SF_RBP[RBP]
	CMP	RDI,RDX
	JB	@@80
	XOR	RDI,RDI
	JMP	@@85
@@80:	MOV	RDI,U64 SF_RIP[RDI]
	CMP	RDI,RDX
	JB	@@85
	XOR	RDI,RDI
@@85:	MOV	U64 CMemUsedAllocated.caller2[RSI],RDI
	POP	RDX

#endif
	LOCK
	BTR	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	POPFD

	MOV	U64 CMemUsedAllocated.size[RSI],RAX
	MOV	U64 CMemUsedAllocated.hc[RSI],RDX
	LEA	RAX,U64 CMemUsedAllocated.start[RSI]

	BT	U32 [SYS_SEMAS+SYS_SEMA_HEAPLOG_ACTIVE*SEMA_STRUCT_SIZE],0
	JNC	@@100
	PUSH_C_REGS
	PUSH	RAX
	MOV	RAX,U64 [SYS_EXTERN_TABLE]
	MOV	RAX,U64 EXT_HEAPLOG_MALLOC*8[RAX]
	OR	RAX,RAX
	JZ	@@90
	CALL	RAX
	JMP	@@95
@@90:	ADD	RSP,8
@@95:	POP_C_REGS

@@100:	TEST	U8 [SYS_HEAP_INIT_FLAG],1
	JZ	MALLOC_FINAL_EXIT

	PUSH	RAX
	PUSH	RCX
	MOV	RCX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RAX]
	SUB	RCX,CMemUsedAllocated.start
	MOV	RDI,RAX
	MOV	AL,U8 [SYS_HEAP_INIT_VAL]
	REP_STOSB
	POP	RCX
	POP	RAX

MALLOC_FINAL_EXIT:
	POP	RDI
	POP	RSI
	LEAVE
	RET1	16
// ************************************
_FREE::
//Be aware of $LK,"heap_hash","FF:::/SparrowOS/OSMain/Memory.CPP.Z,heap_hash"$ in $LK,"MemBlksAlloc","MN:MemBlksAlloc"$().
	ENTER	0
	PUSH	RSI
	PUSH	RDI

	BT	U32 [SYS_SEMAS+SYS_SEMA_HEAPLOG_ACTIVE*SEMA_STRUCT_SIZE],0
	JNC	@@20
	PUSH_C_REGS
	MOV	RBX,U64 SF_ARG1[RBP]
	OR	RBX,RBX
	JZ	@@05
	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
	OR	RAX,RAX
	JGE	@@05	//Aligned alloced chunks have neg size
	ADD	RBX,RAX
@@05:	PUSH	RBX
	MOV	RAX,U64 [SYS_EXTERN_TABLE]
	MOV	RAX,U64 EXT_HEAPLOG_FREE*8[RAX]
	OR	RAX,RAX
	JZ	@@10
	CALL	RAX
	JMP	@@15
@@10:	ADD	RSP,8
@@15:	POP_C_REGS

@@20:	MOV	RSI,U64 SF_ARG1[RBP]
	OR	RSI,RSI

#if _CFG_HEAP_DBG
	JZ	I32 FREE_DONE
#else
	JZ	FREE_DONE
#endif

	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RSI]
	OR	RAX,RAX
	JGE	@@25	//Aligned alloced chunks have neg size.
			//The neg size is offset to start of $LK,"CMemUsedAllocated","MN:CMemUsedAllocated"$ struct.
	ADD	RSI,RAX

@@25:	PUSHFD
	SUB	RSI,CMemUsedAllocated.start
	MOV	RDX,U64 CMemUsedAllocated.hc[RSI]
	CMP	U32 CHeapCtrl.hc_signature[RDX],HEAP_CTRL_SIGNATURE_VAL
	JE	@@30
	ADD	RSI,CMemUsedAllocated.start
	PUSH	RSI
	CALL	&SysBadFree
	JMP	I32 _SYS_HLT

@@30:	MOV	RAX,U64 CMemUsedAllocated.size[RSI]
	SUB	U64 CHeapCtrl.used_u8s[RDX],RAX
	CLI
@@35:	LOCK
	BTS	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	PAUSE
	JC	@@35
#if _CFG_HEAP_DBG
//QueRem
	MOV	RDX,U64 CMemUsedAllocated.next[RSI]
	MOV	RDI,U64 CMemUsedAllocated.last[RSI]
	MOV	U64 CMemUsedAllocated.last[RDX],RDI
	MOV	U64 CMemUsedAllocated.next[RDI],RDX

//Caller1/Caller2
	MOV	RDX,U64 [SYS_HEAP_LIMIT]
	MOV	RDI,U64 SF_RIP[RBP]
	CMP	RDI,RDX
	JB	@@40
	XOR	RDI,RDI
	MOV	U64 CMemUnusedAllocated.caller1[RSI],RDI
	JMP	@@50
@@40:	MOV	U64 CMemUnusedAllocated.caller1[RSI],RDI
	MOV	RDI,U64 SF_RBP[RBP]
	CMP	RDI,RDX
	JB	@@45
	XOR	RDI,RDI
	JMP	@@50
@@45:	MOV	RDI,U64 SF_RIP[RDI]
	CMP	RDI,RDX
	JB	@@50
	XOR	RDI,RDI
@@50:	MOV	U64 CMemUnusedAllocated.caller2[RSI],RDI

	MOV	RDX,U64 CMemUsedAllocated.hc[RSI]
#endif
	CMP	RAX,HEAP_HASH_SIZE
	JAE	@@55

#assert CMemUnusedAllocated.size==CMemUsedAllocated.size
//	MOV	U64 CMemUnusedAllocated.size[RSI],RAX

	MOV	RBX,U64 CHeapCtrl.heap_hash[RAX+RDX]
	MOV	U64 CMemUnusedAllocated.next[RSI],RBX
	MOV	U64 CHeapCtrl.heap_hash[RAX+RDX],RSI
	JMP	@@60

@@55:	SUB	RSI,sizeof(CMemBlk)
	PUSH	RDX
	PUSH	RDX
	PUSH	RSI
	CALL	&MemBlksFree
	POP	RDX

@@60:	LOCK
	BTR	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	POPFD
FREE_DONE:
	POP	RDI
	POP	RSI
	LEAVE
	RET1	8
// ************************************
_MSIZE::
	ENTER	0
	MOV	RBX,U64 SF_ARG1[RBP]
	XOR	RAX,RAX
	OR	RBX,RBX
	JZ	@@10
	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
	OR	RAX,RAX
	JGE	@@05	//Aligned alloced chunks have neg size
	ADD	RBX,RAX
	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
@@05:	SUB	RAX,CMemUsedAllocated.start
@@10:	LEAVE
	RET1	8
// ************************************
_MSIZE2::
	ENTER	0
	MOV	RBX,U64 SF_ARG1[RBP]
	XOR	RAX,RAX
	OR	RBX,RBX
	JZ	@@10
	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
	OR	RAX,RAX
	JGE	@@05	//Aligned alloced chunks have neg size
	ADD	RBX,RAX
@@05:	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
@@10:	LEAVE
	RET1	8
// ************************************
_MHEAP_CTRL::
	ENTER	0
	MOV	RBX,U64 SF_ARG1[RBP]
	XOR	RAX,RAX
	OR	RBX,RBX
	JZ	@@10
	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
	OR	RAX,RAX
	JGE	@@05	//Aligned alloced chunks have neg size
	ADD	RBX,RAX
@@05:	MOV	RAX,U64 CMemUsedAllocated.hc-CMemUsedAllocated.start[RBX]
@@10:	LEAVE
	RET1	8
}

_extern _FREE U0 Free(U8 *add);
_extern _MSIZE I64 MSize(U8 *src); //size of heap object
_extern _MSIZE2 I64 MSize2(U8 *src); //Internal size
_extern _MHEAP_CTRL CHeapCtrl *MHeapCtrl(U8 *src); //$LK,"CHeapCtrl","MN:CHeapCtrl"$ of object

//Accepts a $LK,"CTask","MN:CTask"$ or $LK,"CHeapCtrl","MN:CHeapCtrl"$.  NULL allocs off current task's heap.
_extern _MALLOC U8 *MAlloc(I64 size,CTask *mem_task=NULL);

U8 *AMAlloc(I64 size)
{
  return MAlloc(size,adam_task);
}

U8 *CAlloc(I64 size,CTask *mem_task=NULL)
{//Accepts a $LK,"CTask","MN:CTask"$ or $LK,"CHeapCtrl","MN:CHeapCtrl"$.  NULL allocs off current task's heap.
  U8 *result=MAlloc(size,mem_task);
  MemSet(result,0,size);
  return result;
}

U8 *ACAlloc(I64 size)
{
  return CAlloc(size,adam_task);
}

U8 *MAllocIdentical(U8 *src,CTask *mem_task=NULL)
{//Accepts a $LK,"CTask","MN:CTask"$ or $LK,"CHeapCtrl","MN:CHeapCtrl"$.  NULL allocs off current task's heap.
  U8 *result;
  I64 size;
  if (!src) return NULL;
  size=MSize(src);
  result=MAlloc(size,mem_task);
  MemCpy(result,src,size);
  return result;
}

U8 *AMAllocIdentical(U8 *src)
{
  return MAllocIdentical(src,adam_task);
}

U8 *MAllocAligned(I64 size,I64 alignment,CTask *mem_task=NULL,I64 misalignment=0)
{ //only powers of two alignment.
  I64 mask=alignment-1;
  U8 *ptr=MAlloc(size+mask+sizeof(I64)+misalignment,mem_task),
     *result=(ptr+sizeof(I64)+mask)&~mask+misalignment;
  result(I64 *)[-1]=ptr-result;
#assert offset(CMemUsedAllocated.size)==offset(CMemUsedAllocated.start)-sizeof(I64)
  return result;
}

U8 *CAllocAligned(I64 size,I64 alignment,CTask *mem_task=NULL,I64 misalignment=0)
{ //only powers of two alignment.
  I64 mask=alignment-1;
  U8 *ptr=MAlloc(size+mask+sizeof(I64)+misalignment,mem_task),
     *result=(ptr+sizeof(I64)+mask)&~mask+misalignment;
  result(I64 *)[-1]=ptr-result;
#assert offset(CMemUsedAllocated.size)==offset(CMemUsedAllocated.start)-sizeof(I64)
  MemSet(result,0,size);
  return result;
}

U8 *StrNew(U8 *buf,CTask *mem_task=NULL)
{//Accepts a $LK,"CTask","MN:CTask"$ or $LK,"CHeapCtrl","MN:CHeapCtrl"$.  NULL allocs off current task's heap.
  U8 *result;
  I64 size;
  if (buf) {
    size=StrLen(buf)+1;
    result=MAlloc(size,mem_task);
    MemCpy(result,buf,size);
  } else {
    result=MAlloc(1,mem_task);
    *result=0;
  }
  return result;
}

U8 *AStrNew(U8 *buf)
{
  return StrNew(buf,adam_task);
}

U0 LinkedListDel(U8 **_list)
{
  U8 **d;
  while (_list) {
    d=*_list;
    Free(_list);
    _list=d;
  }
}

U8 *LinkedListCopy(U8 **_list,CTask *mem_task=NULL)
{
  U8 *result=NULL,**d=&result;
  while (_list) {
    d=*d=MAllocIdentical(_list,mem_task);
    _list=*_list;
  }
  return result;
}

I64 LinkedListCnt(U8 **_list)
{
  I64 result=0;
  while (_list) {
    result++;
    _list=*_list;
  }
  return result;
}

I64 LinkedListSize(U8 **_list)
{
  I64 result=0;
  while (_list) {
    result+=MSize2(_list);
    _list=*_list;
  }
  return result;
}

U0 QueDel(CQue *root,Bool querem=FALSE) //Does not delete root
{
  CQue *d=root->next,*d1;
  while (d!=root) {
    d1=d->next;
    if (querem)
      QueRem(d);
    Free(d);
    d=d1;
  }
}

CQue *QueCopy(CQue *root,CTask *mem_task=NULL)
{
  CQue *result=MAllocIdentical(root,mem_task),*d=root->next,*d1;
  QueInit(result);
  while (d!=root) {
    d1=MAllocIdentical(d,mem_task);
    QueIns(d1,result->last);
    d=d->next;
  }
  return result;
}

I64 QueCnt(CQue *root) //Does not include root
{
  CQue *d=root->next;
  I64 result=0;
  while (d!=root) {
    result++;
    d=d->next;
  }
  return result;
}

I64 QueSize(CQue *root) //Does not include root
{
  CQue *d=root->next;
  I64 result=0;
  while (d!=root) {
    result+=MSize2(d);
    d=d->next;
  }
  return result;
}

U0 BlkPoolAdd(CBlkPool *bp,CMemBlk *m,I64 pages512)
{
  if (sys_mem_init_flag)
    MemSet(m,sys_mem_init_val,pages512*512);
  PushFD;
  Cli;
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    Pause;
  m->next=bp->mem_free_list;
  m->pages=pages512;
  m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
  bp->size+=pages512;
  bp->mem_free_list=m;
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  PopFD;
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

CHeapCtrl *TaskHeapCtrlNew(CTask *task,CBlkPool *bp)
{
  U8 *b;
  CHeapCtrl *hc;
//$LK,"Adam Task","FF:::/SparrowOS/OSMain/OSStart.CPP.Z,CHeapCtrl.bp"$
  hc=ACAlloc(sizeof(CHeapCtrl));
  hc->bp=bp;
  hc->hc_signature=HEAP_CTRL_SIGNATURE_VAL;
  hc->mem_task=task;
  b=&hc->next_mem_blk;
  hc->next_mem_blk=hc->last_mem_blk=b;
  hc->next_um=hc->last_um=(&hc->next_um)(U8 *)-offset(CMemUsedAllocated.next);
  return hc;
}

U0 TaskHeapCtrlDel(CHeapCtrl *hc)
{
  MemBlkListFree(hc);
  Free(hc);
}

CHeapCtrl *HeapCtrlIndependentInit(CBlkPool *bp,I64 pages512)
{
  I64 num;
  CHeapCtrl *hc=bp(U8 *)+sizeof(CBlkPool);
  CMemBlk *m;
  MemSet(bp,0,sizeof(CBlkPool)+sizeof(CHeapCtrl));
  LBts(&hc->flags,HCf_NON_TASK_QUE);
  hc->bp=bp;
  hc->hc_signature=HEAP_CTRL_SIGNATURE_VAL;
  hc->next_um=hc->last_um=(&hc->next_um)(U8 *)-offset(CMemUsedAllocated.next);
  m=(bp(U8 *)+sizeof(CBlkPool)+sizeof(CHeapCtrl)+PAGE_SIZE-1)&~(PAGE_SIZE-1);
  num=(bp(U8 *)+pages512<<PAGE_BITS-m(U8 *))>>PAGE_BITS;
  bp->size=pages512-num;
  BlkPoolAdd(bp,m,num);
  return hc;
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

Bool MemLowPagesProtect(Bool val)
{ //FALSE if any were not protected
  U8 *a=0;
  Bool result=TRUE;
  while (a<PROTECTED_LOW_PAGE_LIMIT) {
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
#exe {
  if (osmain_cfg->opts[CFG_PROTECT_LOW])
    StreamPutS("MemLowPagesProtect(TRUE);");
};
  while (m20->type) {
    a=m20->base+m20->len;
    if (a>max_physical)
      max_physical=a;
    m20++;
  }
  if (max_physical>=0x1000000+m01[1]<<16) {
    a=(max_physical+0x1FFFFF)&~0x1FFFFF;
    while (a<MAPPED_MEM_SPACE) {
      MemPagePresentMark(a,FALSE);
      a+=MemPageSize(a);
    }
  }
}

U8 *Mem32DevAlloc(I64 size,I64 alignment)
{
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
    pte=MemPtAtPageTableEntry(sys_mem64_dev_ptr);
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
	    Fs->data_heap=TaskHeapCtrlNew(Fs,sys_data_bp);
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
