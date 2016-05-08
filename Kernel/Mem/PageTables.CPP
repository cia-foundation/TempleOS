asm {
// $LK,"::/Doc/MemOverview.TXT"$

USE32
SYS_INIT_PAGE_TABLES::
//Set page tables to identity map everything.
	MOV	EDI,SYS_FIXED_AREA
	XOR	EAX,EAX
	MOV	ECX,sizeof(CSysFixedArea)/4
	REP_STOSD			//Init entire SYS_FIXED_AREA to zero

//PML2
	MOV	EAX,0x87 //bit 7 is page size (1==2Meg)
	XOR	EDX,EDX
	MOV	EDI,SYS_FIXED_AREA+CSysFixedArea.pml2
	MOV	ECX,MEM_NUM_PML2*0x200
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
	(SYS_FIXED_AREA+sizeof(CSysFixedArea))>MEM_ADAM_STK+
	ESTIMATED_MISC_ALLOCS

//EBX=BASE EAX=SIZE
	TEST	U8 [SYS_MEM_INIT_FLAG],1
	JZ	@@40
	PUSH	EAX
	MOV	EDI,EBX
	MOV	ECX,EAX
	MOV	AL,U8 [SYS_MEM_INIT_VAL]
	REP_STOSB
	POP	EAX

@@40:	SHR	EAX,MEM_PAGE_BITS
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
  return SYS_FIXED_AREA+offset(CSysFixedArea.pml2)+a>>21*8;
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

