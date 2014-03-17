asm {
USE64
// ************************************
// Throws 'OutMem'
_MALLOC::
	PUSH	RBP
	MOV	RBP,RSP
	PUSH	RSI
	PUSH	RDI

	XOR	RBX,RBX
	MOV	RDX,U64 SF_ARG2[RBP]
	TEST	RDX,RDX
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
	TEST	RSI,RSI
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
	TEST	RSI,RSI
	JZ	@@45	//Out of memory

	SHL	RAX,PAGE_BITS
	SUB	RAX,sizeof(CMemBlk)
	ADD	RSI,sizeof(CMemBlk)
	JMP	I32 MALLOC_ALMOST_DONE

//Little allocation, chunk-off piece from free lst chunks
@@35:	LEA	RSI,U64 CHeapCtrl.malloc_free_lst-CMemUnusedAllocated.next[RDX]

@@40:	MOV	RBX,RSI
	MOV	RSI,U64 CMemUnusedAllocated.next[RBX]
	TEST	RSI,RSI
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
	TEST	RSI,RSI
	JNZ	@@50

//Out of memory
@@45:	LOCK
	BTR	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	POPFD
	PUSH	TRUE
	MOV	RAX,'OutMem'
	PUSH	RAX
	CALL	I32 &throw
	JMP	I32 MALLOC_FINAL_EXIT //Never gets here, hopefully.

@@50:	LEA	RSI,U64 sizeof(CMemBlk)[RSI]
	SHL	RAX,PAGE_BITS
	SUB	RAX,sizeof(CMemBlk)
	LEA	RBX,U64 CHeapCtrl.malloc_free_lst-CMemUnusedAllocated.next[RDX]
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

	TEST	U8 [SYS_SEMAS+SYS_SEMA_HEAPLOG_ACTIVE*SEMA_STRUCT_SIZE],1
	JZ	@@100
	PUSH	RAX
	PUSH	RAX
	MOV	RAX,U64 [SYS_EXTERN_TABLE]
	MOV	RAX,U64 EXT_HEAPLOG_MALLOC*8[RAX]
	TEST	RAX,RAX
	JZ	@@90
	CALL	RAX
	JMP	@@95
@@90:	ADD	RSP,8
@@95:	POP	RAX

@@100:	TEST	U8 [SYS_HEAP_INIT_FLAG],1
	JZ	MALLOC_FINAL_EXIT

	PUSH	RAX
	MOV	RCX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RAX]
	SUB	RCX,CMemUsedAllocated.start
	MOV	RDI,RAX
	MOV	AL,U8 [SYS_HEAP_INIT_VAL]
	REP_STOSB
	POP	RAX

MALLOC_FINAL_EXIT:
	POP	RDI
	POP	RSI
	POP	RBP
	RET1	16
// ************************************
_FREE::
//Be aware of $LK,"heap_hash",A="FF:::/Kernel/Mem1b.CPP,heap_hash"$ in $LK,"MemBlksAlloc",A="MN:MemBlksAlloc"$().
	PUSH	RBP
	MOV	RBP,RSP
	PUSH	RSI
	PUSH	RDI

	TEST	U8 [SYS_SEMAS+SYS_SEMA_HEAPLOG_ACTIVE*SEMA_STRUCT_SIZE],1
	JZ	@@15
	MOV	RBX,U64 SF_ARG1[RBP]
	TEST	RBX,RBX
	JZ	@@05
	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
	TEST	RAX,RAX
	JGE	@@05	//Aligned alloced chunks have neg size
	ADD	RBX,RAX
@@05:	PUSH	RBX
	MOV	RAX,U64 [SYS_EXTERN_TABLE]
	MOV	RAX,U64 EXT_HEAPLOG_FREE*8[RAX]
	TEST	RAX,RAX
	JZ	@@10
	CALL	RAX
	JMP	@@15
@@10:	ADD	RSP,8

@@15:	MOV	RSI,U64 SF_ARG1[RBP]
	TEST	RSI,RSI

#if _CFG_HEAP_DBG
	JZ	I32 FREE_DONE
#else
	JZ	FREE_DONE
#endif

	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RSI]
	TEST	RAX,RAX
	JGE	@@20	//Aligned alloced chunks have neg size.
			//The neg size is offset to start of $LK,"CMemUsedAllocated",A="MN:CMemUsedAllocated"$ struct.
	ADD	RSI,RAX

@@20:	PUSHFD
	SUB	RSI,CMemUsedAllocated.start
	MOV	RDX,U64 CMemUsedAllocated.hc[RSI]
	CMP	U32 CHeapCtrl.hc_signature[RDX],HEAP_CTRL_SIGNATURE_VAL
	JE	@@25
	ADD	RSI,CMemUsedAllocated.start
	PUSH	RSI
	CALL	&SysBadFree
	JMP	I32 _SYS_HLT

@@25:	MOV	RAX,U64 CMemUsedAllocated.size[RSI]
	SUB	U64 CHeapCtrl.used_u8s[RDX],RAX
	CLI
@@30:	LOCK
	BTS	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	PAUSE
	JC	@@30
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
	JB	@@35
	XOR	RDI,RDI
	MOV	U64 CMemUnusedAllocated.caller1[RSI],RDI
	JMP	@@45
@@35:	MOV	U64 CMemUnusedAllocated.caller1[RSI],RDI
	MOV	RDI,U64 SF_RBP[RBP]
	CMP	RDI,RDX
	JB	@@40
	XOR	RDI,RDI
	JMP	@@45
@@40:	MOV	RDI,U64 SF_RIP[RDI]
	CMP	RDI,RDX
	JB	@@45
	XOR	RDI,RDI
@@45:	MOV	U64 CMemUnusedAllocated.caller2[RSI],RDI

	MOV	RDX,U64 CMemUsedAllocated.hc[RSI]
#endif
	CMP	RAX,HEAP_HASH_SIZE
	JAE	@@50

#assert CMemUnusedAllocated.size==CMemUsedAllocated.size
//	MOV	U64 CMemUnusedAllocated.size[RSI],RAX

	MOV	RBX,U64 CHeapCtrl.heap_hash[RAX+RDX]
	MOV	U64 CMemUnusedAllocated.next[RSI],RBX
	MOV	U64 CHeapCtrl.heap_hash[RAX+RDX],RSI
	JMP	@@55

@@50:	SUB	RSI,sizeof(CMemBlk)
	PUSH	RDX
	PUSH	RDX
	PUSH	RSI
	CALL	&MemBlksFree
	POP	RDX

@@55:	LOCK
	BTR	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	POPFD
FREE_DONE:
	POP	RDI
	POP	RSI
	POP	RBP
	RET1	8
// ************************************
_MSIZE::
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RBX,U64 SF_ARG1[RBP]
	XOR	RAX,RAX
	TEST	RBX,RBX
	JZ	@@10
	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
	TEST	RAX,RAX
	JGE	@@05	//Aligned alloced chunks have neg size
	ADD	RBX,RAX
	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
@@05:	SUB	RAX,CMemUsedAllocated.start
@@10:	POP	RBP
	RET1	8
// ************************************
_MSIZE2::
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RBX,U64 SF_ARG1[RBP]
	XOR	RAX,RAX
	TEST	RBX,RBX
	JZ	@@10
	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
	TEST	RAX,RAX
	JGE	@@05	//Aligned alloced chunks have neg size
	ADD	RBX,RAX
@@05:	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
@@10:	POP	RBP
	RET1	8
// ************************************
_MHEAP_CTRL::
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RBX,U64 SF_ARG1[RBP]
	XOR	RAX,RAX
	TEST	RBX,RBX
	JZ	@@10
	MOV	RAX,U64 CMemUsedAllocated.size-CMemUsedAllocated.start[RBX]
	TEST	RAX,RAX
	JGE	@@05	//Aligned alloced chunks have neg size
	ADD	RBX,RAX
@@05:	MOV	RAX,U64 CMemUsedAllocated.hc-CMemUsedAllocated.start[RBX]
@@10:	POP	RBP
	RET1	8
}

_extern _FREE U0 Free(U8 *addr);
_extern _MSIZE I64 MSize(U8 *src); //size of heap object
_extern _MSIZE2 I64 MSize2(U8 *src); //Internal size
_extern _MHEAP_CTRL CHeapCtrl *MHeapCtrl(U8 *src); //$LK,"CHeapCtrl",A="MN:CHeapCtrl"$ of object

//Accepts a $LK,"CTask",A="MN:CTask"$ or $LK,"CHeapCtrl",A="MN:CHeapCtrl"$.  NULL allocs off current task's heap.
_extern _MALLOC U8 *MAlloc(I64 size,CTask *mem_task=NULL);

U8 *AMAlloc(I64 size)
{
  return MAlloc(size,adam_task);
}

U8 *CAlloc(I64 size,CTask *mem_task=NULL)
{//Accepts a $LK,"CTask",A="MN:CTask"$ or $LK,"CHeapCtrl",A="MN:CHeapCtrl"$.  NULL allocs off current task's heap.
  U8 *result=MAlloc(size,mem_task);
  MemSet(result,0,size);
  return result;
}

U8 *ACAlloc(I64 size)
{
  return CAlloc(size,adam_task);
}

U8 *MAllocIdentical(U8 *src,CTask *mem_task=NULL)
{//Accepts a $LK,"CTask",A="MN:CTask"$ or $LK,"CHeapCtrl",A="MN:CHeapCtrl"$.  NULL allocs off current task's heap.
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
{ //only powers of two alignment. This is awful.
  I64 mask=alignment-1;
  U8 *ptr=MAlloc(size+mask+sizeof(I64)+misalignment,mem_task),
     *result=(ptr+sizeof(I64)+mask)&~mask+misalignment;
  result(I64 *)[-1]=ptr-result;
#assert offset(CMemUsedAllocated.size)==offset(CMemUsedAllocated.start)-sizeof(I64)
  return result;
}

U8 *CAllocAligned(I64 size,I64 alignment,CTask *mem_task=NULL,I64 misalignment=0)
{ //only powers of two alignment. This is awful.
  I64 mask=alignment-1;
  U8 *ptr=MAlloc(size+mask+sizeof(I64)+misalignment,mem_task),
     *result=(ptr+sizeof(I64)+mask)&~mask+misalignment;
  result(I64 *)[-1]=ptr-result;
#assert offset(CMemUsedAllocated.size)==offset(CMemUsedAllocated.start)-sizeof(I64)
  MemSet(result,0,size);
  return result;
}

U8 *StrNew(U8 *buf,CTask *mem_task=NULL)
{//Accepts a $LK,"CTask",A="MN:CTask"$ or $LK,"CHeapCtrl",A="MN:CHeapCtrl"$.  NULL allocs off current task's heap.
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
