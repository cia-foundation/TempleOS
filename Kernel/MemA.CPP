asm {
/*See $LK,"::/Doc/Credits.TXT"$.
See $LK,"::/Doc/MemOverview.TXT"$
*/

USE32
SYS_INIT_MEM::
//Set page tables to identity map everything.
	MOV	EDI,SYS_FIXED_AREA
	XOR	EAX,EAX
	MOV	ECX,sizeof(CSysFixedArea)/4
	REP_STOSD			//Init entire SYS_FIXED_AREA to zero

//PML1
	MOV	EAX,0x107
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml1
	MOV	ECX,MEM_NUM_PML1*0x200
@@05:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x1000
	ADC	EDX,0
	LOOP	@@05

//0xA0000-0xBFFFF: video ram=write thru
	MOV	EAX,0xF
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml1+0xA0*8
	MOV	ECX,0xC0-0xA0
@@10:	OR	U32 [EDI],EAX
	ADD	EDI,4
	OR	U32 [EDI],EDX
	ADD	EDI,4
	LOOP	@@10

//PML2
	MOV	EAX,SYS_FIXED_AREA+CSysFixedArea.pml1+7
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml2
	MOV	ECX,MEM_NUM_PML1
@@15:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x1000
	ADC	EDX,0
	LOOP	@@15

	MOV	EAX,MEM_NUM_PML1*0x200000+0x87 //bit 7 is page size (1==2Meg)
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml2+8*MEM_NUM_PML1
	MOV	ECX,MEM_NUM_PML2*0x200-MEM_NUM_PML1
@@20:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x200000
	ADC	EDX,0
	LOOP	@@20

//0xE0000000-0xFFFFFFFF: PCI Devs
	MOV	EAX,0x17	//Cache disabled
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml2+8*0x700
	MOV	ECX,0x800-0x700
@@25:	OR	U32 [EDI],EAX
	ADD	EDI,4
	OR	U32 [EDI],EDX
	ADD	EDI,4
	LOOP	@@25

//PML3
	MOV	EAX,SYS_FIXED_AREA+CSysFixedArea.pml2+7
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml3
	MOV	ECX,MEM_NUM_PML2
@@30:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x1000
	ADC	EDX,0
	LOOP	@@30

//PML4
	MOV	EAX,SYS_FIXED_AREA+CSysFixedArea.pml3+7
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml4
	MOV	ECX,MEM_NUM_PML3
@@35:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x1000
	ADC	EDX,0
	LOOP	@@35

	MOV	U32 [SYS_CODE_BP],SYS_FIXED_AREA+CSysFixedArea.adam_bp
	MOV	U32 [SYS_CODE_BP+4],0

	MOV	U32 [SYS_DATA_BP],0
	MOV	U32 [SYS_DATA_BP+4],0

	XOR	EAX,EAX
	MOV	AX,U16 [SYS_MEM_E801] //1 Kb blks between 1M and 16M
	SHL	EAX,10
	ADD	EAX,0x100000
	MOV	EBX,U32 [SYS_HEAP_BASE] //SYS_FIXED_AREA+sizeof(CSysFixedArea)
	SUB	EAX,EBX

/*$AN,"ExpandHeap",A="ExpandHeap"$
TempleOS initializes the heap to the val
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

#assert CONSERVATIVE_16MEG_SIZE-
	(SYS_FIXED_AREA+sizeof(CSysFixedArea))>ADAM_STK+ESTIMATED_MISC_ALLOCS

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
	MOV	ESI,SYS_FIXED_AREA+CSysFixedArea.adam_bp
	MOV	EDI,U32 CBlkPool.mem_free_lst[ESI]
	MOV	U32 CMemBlk.next[EBX],EDI
	MOV	U32 CMemBlk.next+4[EBX],0
	MOV	U32 CBlkPool.mem_free_lst[ESI],EBX
	MOV	U32 CBlkPool.mem_free_lst+4[ESI],0
	MOV	U32 CMemBlk.mb_signature[EBX],MBS_UNUSED_SIGNATURE_VAL
	MOV	U32 CMemBlk.pages[EBX],EAX
	SHL	EAX,PAGE_BITS
	ADD	U32 CBlkPool.alloced_u8s[ESI],EAX
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
{//Size of page table at addr.Either 4K or 2Meg.
  if (a<MEM_NUM_PML1*0x1000*0x200)
    return 0x1000;
  else
    return 0x200000;
}

I64 *MemPageTable(U8 *a)
{//Point to page table entry for addr.
  if (a<MEM_NUM_PML1*0x1000*0x200)
    return SYS_FIXED_AREA+offset(CSysFixedArea.pml1)+a>>12*8;
  else
    return SYS_FIXED_AREA+offset(CSysFixedArea.pml2)+a>>21*8;
}

Bool MemPagePresentMark(U8 *a,Bool val)
{//Mark an addr's page present or not-present.
  I64 *pte=MemPageTable(a);
  Bool result=*pte&1;
  if (val)
    *pte=*pte|1;  //Mark present
  else
    *pte=*pte&~1; //Mark not present
  InvlPg(a);
  return result;
}

U8 *Mem512Alloc(I64 *_pages512,CBlkPool *bp=NULL)
{/*Alloc 512Byte pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  CMemBlk *result=NULL,*m;
  I64 i,num=*_pages512;
  if (!bp) bp=sys_code_bp;
  PUSHFD
  CLI
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    PAUSE
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
  m=&bp->mem_free_lst;
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
  bp->used_u8s+=num<<PAGE_BITS;
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  POPFD
  return result;
}

U0 Mem512Free(CMemBlk *m,I64 pages512,CBlkPool *bp=NULL)
{//Return non-task 512Byte pages to BlkPool.
  I64 i;
  if (m) {
    if (!bp) bp=sys_code_bp;
    PUSHFD
    CLI
    while (LBts(&bp->locked_flags,BPlf_LOCKED))
      PAUSE
    m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
    m->pages=pages512;
    bp->used_u8s-=pages512<<PAGE_BITS;
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
    POPFD
  }
}

U8 *Mem2MegAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
{/*Alloc 2Meg pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  I64 i,j,*pte,num=*_pages2Meg;
  CMemBlk *result=NULL,*m,*m1;

  if (!bp) bp=sys_code_bp;
  PUSHFD
  CLI
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    PAUSE
  num<<=21-PAGE_BITS;

  m=&bp->mem_free_2meg_lst;
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

  m=&bp->mem_free_lst;
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
  bp->used_u8s+=i;
  num>>=21-PAGE_BITS;
  *_pages2Meg=num;
  m=result;
  m1=m(U8 *)+i;
  while (m<m1) {
    pte=MemPageTable(m);
    *pte &= ~0x18;
    InvlPg(m);
    m(U8 *)+=MemPageSize(m);
  }
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  POPFD
  return result;
}

U8 *Mem2MegUncachedAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
{/*Alloc 2Meg pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It will be marked uncached. It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  CMemBlk *result,*m,*m1;
  I64 num=*_pages2Meg,*pte;
  if (result=Mem2MegAlloc(_pages2Meg,bp)) {
    num=*_pages2Meg;
    m=result;
    m1=m(U8 *)+num<<21;
    while (m<m1) {
      pte=MemPageTable(m);
      *pte= *pte& ~0x18 |0x10;
      InvlPg(m);
      m(U8 *)+=MemPageSize(m);
    }
  }
  return result;
}

U8 *Mem2MegWriteThruAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL)
{/*Alloc 2Meg pages from BlkPool. Don't link to task.
(Linking to a task means they will be freed when the task dies.)
It will be marked write-through. It might give you more than you asked for
so a ptr to a page count is passed.

Return: NULL if out of memory.
*/
  CMemBlk *result,*m,*m1;
  I64 num=*_pages2Meg,*pte;
  if (result=Mem2MegAlloc(_pages2Meg,bp)) {
    num=*_pages2Meg;
    m=result;
    m1=m(U8 *)+num<<21;
    while (m<m1) {
      pte=MemPageTable(m);
      *pte= *pte& ~0x18 |8;
      InvlPg(m);
      m(U8 *)+=MemPageSize(m);
    }
  }
  return result;
}

U0 Mem2MegFree(CMemBlk *m,I64 pages2Meg,CBlkPool *bp=NULL)
{//Return non-task 2Meg pages to BlkPool.
  I64 *pte;
  CMemBlk *m1,*m2;
  if (m) {
    if (!bp) bp=sys_code_bp;
    m2=m;
    m1=m(U8 *)+pages2Meg<<21;
    while (m2<m1) {
      pte=MemPageTable(m2);
      *pte=*pte & ~0x18;
      InvlPg(m2);
      m2(U8 *)+=MemPageSize(m2);
    }
    PUSHFD
    CLI
    while (LBts(&bp->locked_flags,BPlf_LOCKED))
      PAUSE
    m->mb_signature=MBS_UNUSED_SIGNATURE_VAL;
    m->pages=pages2Meg<<(21-PAGE_BITS);
    bp->used_u8s-=pages2Meg<<21;
    m->next=bp->mem_free_2meg_lst;
    bp->mem_free_2meg_lst=m;
    LBtr(&bp->locked_flags,BPlf_LOCKED);
    POPFD
  }
}

CMemBlk *Mem512TaskAlloc(I64 *_pages512,CHeapCtrl *hc)
{/*hc must be locked.  Interrupts should probably be off
Currently, this is only called from $LK,"MAlloc",A="MN:MAlloc"$().

Return: NULL if out of memory.
*/
  CMemBlk *result;
  I64 threshold,cnt,size;
  CMemUnused *uum,**_uum,**_ptr;
  if (result=Mem512Alloc(_pages512,hc->bp)) {
    QueIns(result,hc->last_mem_blk);
    result->mb_signature=MBS_USED_SIGNATURE_VAL;
    hc->alloced_u8s+=*_pages512<<PAGE_BITS;

    //Tidy-up free lst (Move into heap hash)
    //because if free lst gets long, delay causes crash
    threshold=HEAP_HASH_SIZE>>4;
#assert HEAP_HASH_SIZE>>4>=sizeof(U8 *)
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
    } while (cnt>8 && threshold<=HEAP_HASH_SIZE);
  }
  return result;
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
      hc->alloced_u8s-=m->pages<<PAGE_BITS;
      Mem512Free(m,m->pages,hc->bp);
    }
    POPFD
  }
}
