asm {
// $LK,"::/Doc/MemOverview.DD"$

USE32
SYS_INIT_PAGE_TABLES::
//Set page tables to identity map everything.
	MOV	EDI,SYS_FIXED_AREA
	XOR	EAX,EAX
	MOV	ECX,sizeof(CSysFixedArea)/4
	REP_STOSD	//Init entire SYS_FIXED_AREA to zero

	MOV	EAX,0x80000001
	CPUID
	BT	EDX,26
	JC	@@15

	MOV	U8 [SYS_GIG_PAGES],FALSE
//PML2: Use 2Meg PAges
	MOV	EAX,0x87 //bit 7 is page size (2Meg)
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml2
	MOV	ECX,MEM_NUM_2MEG
@@05:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x200000
	ADC	EDX,0
	LOOP	@@05
//PML3: Use 2Meg Pages
	MOV	EAX,SYS_FIXED_AREA+CSysFixedArea.pml2+7
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml3
	MOV	ECX,MEM_NUM_USED_1GIG
@@10:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x1000
	ADC	EDX,0
	LOOP	@@10
	JMP	@@25

@@15:	MOV	U8 [SYS_GIG_PAGES],TRUE
//PML3: Use 1Gig Pages
	MOV	EAX,0x87 //bit 7 is page size (1Gig)
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml3
	MOV	ECX,MEM_NUM_USED_1GIG
@@20:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x40000000
	ADC	EDX,0
	LOOP	@@20

//PML4
@@25:	MOV	EAX,SYS_FIXED_AREA+CSysFixedArea.pml3+7
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml4
	MOV	ECX,MEM_NUM_1GIG/(0x1000/8)
@@30:	MOV	U32 [EDI],EAX
	ADD	EDI,4
	MOV	U32 [EDI],EDX
	ADD	EDI,4
	ADD	EAX,0x1000
	ADC	EDX,0
	LOOP	@@30

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
	(SYS_FIXED_AREA+sizeof(CSysFixedArea))>MEM_ADAM_STK+
	ESTIMATED_MISC_ALLOCS

//EBX=BASE EAX=SIZE
	TEST	U8 [SYS_MEM_INIT_FLAG],1
	JZ	@@35
	PUSH	EAX
	MOV	EDI,EBX
	MOV	ECX,EAX
	MOV	AL,U8 [SYS_MEM_INIT_VAL]
	REP_STOSB
	POP	EAX

@@35:	SHR	EAX,MEM_PAGE_BITS
	MOV	ESI,SYS_FIXED_AREA+CSysFixedArea.adam_bp
	MOV	EDI,U32 CBlkPool.mem_free_lst[ESI]
	MOV	U32 CMemBlk.next[EBX],EDI
	MOV	U32 CMemBlk.next+4[EBX],0
	MOV	U32 CBlkPool.mem_free_lst[ESI],EBX
	MOV	U32 CBlkPool.mem_free_lst+4[ESI],0
	MOV	U32 CMemBlk.mb_signature[EBX],MBS_UNUSED_SIGNATURE_VAL
	MOV	U32 CMemBlk.pages[EBX],EAX
	SHL	EAX,MEM_PAGE_BITS
	ADD	U32 CBlkPool.alloced_u8s[ESI],EAX
	RET
}

I64 *MemPageTable(U8 *a)
{//Point to page table entry for addr.
  if (sys_gig_pages)
    return SYS_FIXED_AREA+offset(CSysFixedArea.pml3)+a>>30*8;
  else
    return SYS_FIXED_AREA+offset(CSysFixedArea.pml2)+a>>21*8;
}

I64 MemPageSize(U8 *)
{//Page table entry size for addr.
  if (sys_gig_pages)
    return 1<<30; //1Gig
  else
    return 1<<21; //2Meg
}

Bool MemPagePresentMark(U8 *a,Bool val)
{//Mark an addr's page present or not-present.
  I64 *pte=MemPageTable(a);
  Bool res=*pte&1;
  if (val)
    *pte=*pte|1;  //Mark present
  else
    *pte=*pte&~1; //Mark not present
  InvlPg(a);
  return res;
}

