asm {
//************************************
//See $LK,"::/Doc/Credits.DD"$.
_MALLOC::
// Throws 'OutMem'
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

#assert CTask.task_signature==CHeapCtrl.hc_signature //location signature same

	JNE	@@10
	MOV	RDX,U64 CTask.data_heap[RDX]
@@10:	CMP	U32 CHeapCtrl.hc_signature[RDX],HEAP_CTRL_SIGNATURE_VAL
	JE	@@15
	PUSH	RDX
	CALL	&SysBadMAlloc
	JMP	I32 _SYS_HLT

@@15:	MOV	RAX,U64 SF_ARG1[RBP]
	PUSHFD
	ADD	RAX,CMemUsed.start+7	//round-up to I64
	AND	AL,0xF8
#assert CMemUsed.start>=sizeof(CMemUnused)
	CMP	RAX,CMemUsed.start
	JAE	@@20
	MOV	RAX,CMemUsed.start
@@20:

	CLI
@@25:	LOCK
	BTS	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	PAUSE	//don't know if this inst helps
	JC	@@25

	CMP	RAX,MEM_HEAP_HASH_SIZE
	JAE	@@30
	MOV	RSI,U64 CHeapCtrl.heap_hash[RAX+RDX]
	TEST	RSI,RSI
	JZ	@@35
	MOV	RCX,U64 CMemUnused.next[RSI]
	MOV	U64 CHeapCtrl.heap_hash[RAX+RDX],RCX
	JMP	I32 MALLOC_ALMOST_DONE

//Big allocation
@@30:	ADD	RAX,sizeof(CMemBlk)+MEM_PAG_SIZE-1
	SHR	RAX,MEM_PAG_BITS

	PUSH	RDX //preserve HeapCtrl
	PUSH	RDX
	PUSH	RAX
	CALL	&MemPagTaskAlloc
	POP	RDX
	TEST	RAX,RAX
	JZ	@@45	//Out of memory
	MOV	RSI,RAX
	MOV	EAX,U32 CMemBlk.pags[RSI]

	SHL	RAX,MEM_PAG_BITS
	SUB	RAX,sizeof(CMemBlk)
	ADD	RSI,sizeof(CMemBlk)
	JMP	I32 MALLOC_ALMOST_DONE

//Little allocation, chunk-off piece from free lst chunks
@@35:	LEA	RSI,U64 CHeapCtrl.malloc_free_lst-CMemUnused.next[RDX]

@@40:	MOV	RBX,RSI
	MOV	RSI,U64 CMemUnused.next[RBX]
	TEST	RSI,RSI
	JNZ	I32 @@60
	PUSH	RAX		//-**** save byte size
	ADD	RAX,16*MEM_PAG_SIZE-1
	SHR	RAX,MEM_PAG_BITS

	PUSH	RDX //preserve HeapCtrl
	PUSH	RDX
	PUSH	RAX
	CALL	&MemPagTaskAlloc
	POP	RDX
	TEST	RAX,RAX
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

@@50:	MOV	RSI,RAX
	MOV	EAX,U32 CMemBlk.pags[RSI]
	SHL	RAX,MEM_PAG_BITS

//Can it be combined with last chunk? (Never Free these chunks.)
	MOV	RDI,U64 CHeapCtrl.last_mergable[RDX]
	LEA	RBX,U64 [RSI+RAX]
	CMP	RDI,RBX
	JNE	@@55

	PUSH	RAX
	MOV	EAX,U32 CMemBlk.pags[RDI]
	ADD	U32 CMemBlk.pags[RSI],EAX
//QueRem
	MOV	RAX,U64 CMemBlk.next[RDI]
	MOV	RBX,U64 CMemBlk.last[RDI]
	MOV	U64 CMemBlk.last[RAX],RBX
	MOV	U64 CMemBlk.next[RBX],RAX
	POP	RAX

@@55:	MOV	U64 CHeapCtrl.last_mergable[RDX],RSI
	LEA	RSI,U64 sizeof(CMemBlk)[RSI]
	SUB	RAX,sizeof(CMemBlk)
	LEA	RBX,U64 CHeapCtrl.malloc_free_lst-CMemUnused.next[RDX]
	MOV	RDI,U64 CMemUnused.next[RBX]
	MOV	U64 CMemUnused.next[RSI],RDI
	MOV	U64 CMemUnused.size[RSI],RAX
	MOV	U64 CMemUnused.next[RBX],RSI
	POP	RAX		//+****
	JMP	@@70
@@60:	CMP	U64 CMemUnused.size[RSI],RAX
	JB	I32 @@40
	JNE	@@70

@@65:	MOV	RDI,U64 CMemUnused.next[RSI]
	MOV	U64 CMemUnused.next[RBX],RDI
	JMP	MALLOC_ALMOST_DONE

@@70:	SUB	U64 CMemUnused.size[RSI],RAX	//UPDATE FREE ENTRY
	CMP	U64 CMemUnused.size[RSI],sizeof(CMemUnused)
	JAE	@@75			//take from top of block
	ADD	U64 CMemUnused.size[RSI],RAX	//doesn't fit, undo
	JMP	I32 @@40

@@75:	ADD	RSI,U64 CMemUnused.size[RSI]

MALLOC_ALMOST_DONE:
//RSI=res-CMemUsed.size
//RAX=size+CMemUsed.size
//RDX=HeapCtrl
	ADD	U64 CHeapCtrl.used_u8s[RDX],RAX

#if _CFG_HEAP_DBG
//QueIns
	MOV	RDI,U64 CHeapCtrl.last_um[RDX]
	MOV	U64 CMemUsed.next[RDI],RSI
	MOV	U64 CHeapCtrl.last_um[RDX],RSI
	MOV	U64 CMemUsed.last[RSI],RDI
	LEA	RDI,U64 CHeapCtrl.next_um-CMemUsed.next[RDX]
	MOV	U64 CMemUsed.next[RSI],RDI

//Caller1/Caller2
	PUSH	RDX
	MOV	RDX,U64 [MEM_HEAP_LIMIT]
	MOV	RDI,U64 SF_RIP[RBP]
	CMP	RDI,RDX
	JB	@@80
	XOR	RDI,RDI
	MOV	U64 CMemUsed.caller1[RSI],RDI
	JMP	@@90
@@80:	MOV	U64 CMemUsed.caller1[RSI],RDI
	MOV	RDI,U64 SF_RBP[RBP]
	CMP	RDI,RDX
	JB	@@85
	XOR	RDI,RDI
	JMP	@@90
@@85:	MOV	RDI,U64 SF_RIP[RDI]
	CMP	RDI,RDX
	JB	@@90
	XOR	RDI,RDI
@@90:	MOV	U64 CMemUsed.caller2[RSI],RDI
	POP	RDX

#endif
	LOCK
	BTR	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	POPFD

	MOV	U64 CMemUsed.size[RSI],RAX
	MOV	U64 CMemUsed.hc[RSI],RDX
	LEA	RAX,U64 CMemUsed.start[RSI]

	TEST	U8 [SYS_SEMAS+SEMA_HEAPLOG_ACTIVE*DFT_CACHE_LINE_WIDTH],1
	JZ	@@105
	PUSH	RAX
	PUSH	RAX
	MOV	RAX,U64 [SYS_EXTERN_TABLE]
	MOV	RAX,U64 EXT_HEAPLOG_MALLOC*8[RAX]
	TEST	RAX,RAX
	JZ	@@95
	CALL	RAX
	JMP	@@100
@@95:	ADD	RSP,8
@@100:	POP	RAX

@@105:	TEST	U8 [SYS_HEAP_INIT_FLAG],1
	JZ	MALLOC_FINAL_EXIT

	PUSH	RAX
	MOV	RCX,U64 CMemUsed.size-CMemUsed.start[RAX]
	SUB	RCX,CMemUsed.start
	MOV	RDI,RAX
	MOV	AL,U8 [SYS_HEAP_INIT_VAL]
	REP_STOSB
	POP	RAX

MALLOC_FINAL_EXIT:
	POP	RDI
	POP	RSI
	POP	RBP
	RET1	16
//************************************
_FREE::
//Be aware of $LK,"heap_hash",A="FF:::/Kernel/Mem/MAllocFree.HC,heap_hash"$ in $LK,"MemPagTaskAlloc",A="MN:MemPagTaskAlloc"$().
	PUSH	RBP
	MOV	RBP,RSP
	PUSH	RSI
	PUSH	RDI

	TEST	U8 [SYS_SEMAS+SEMA_HEAPLOG_ACTIVE*DFT_CACHE_LINE_WIDTH],1
	JZ	@@15
	MOV	RBX,U64 SF_ARG1[RBP]
	TEST	RBX,RBX
	JZ	@@05
	MOV	RAX,U64 CMemUsed.size-CMemUsed.start[RBX]
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

	MOV	RAX,U64 CMemUsed.size-CMemUsed.start[RSI]
	TEST	RAX,RAX
	JGE	@@20	//Aligned alloced chunks have neg size.
			//The neg size is offset to start of $LK,"CMemUsed",A="MN:CMemUsed"$ struct.
	ADD	RSI,RAX

@@20:	PUSHFD
	SUB	RSI,CMemUsed.start
	MOV	RDX,U64 CMemUsed.hc[RSI]
	CMP	U32 CHeapCtrl.hc_signature[RDX],HEAP_CTRL_SIGNATURE_VAL
	JE	@@25
	ADD	RSI,CMemUsed.start
	PUSH	RSI
	CALL	&SysBadFree
	JMP	I32 _SYS_HLT

@@25:	MOV	RAX,U64 CMemUsed.size[RSI]
	SUB	U64 CHeapCtrl.used_u8s[RDX],RAX
	CLI
@@30:	LOCK
	BTS	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	PAUSE
	JC	@@30
#if _CFG_HEAP_DBG
//QueRem
	MOV	RDX,U64 CMemUsed.next[RSI]
	MOV	RDI,U64 CMemUsed.last[RSI]
	MOV	U64 CMemUsed.last[RDX],RDI
	MOV	U64 CMemUsed.next[RDI],RDX

//Caller1/Caller2
	MOV	RDX,U64 [MEM_HEAP_LIMIT]
	MOV	RDI,U64 SF_RIP[RBP]
	CMP	RDI,RDX
	JB	@@35
	XOR	RDI,RDI
	MOV	U64 CMemUnused.caller1[RSI],RDI
	JMP	@@45
@@35:	MOV	U64 CMemUnused.caller1[RSI],RDI
	MOV	RDI,U64 SF_RBP[RBP]
	CMP	RDI,RDX
	JB	@@40
	XOR	RDI,RDI
	JMP	@@45
@@40:	MOV	RDI,U64 SF_RIP[RDI]
	CMP	RDI,RDX
	JB	@@45
	XOR	RDI,RDI
@@45:	MOV	U64 CMemUnused.caller2[RSI],RDI

	MOV	RDX,U64 CMemUsed.hc[RSI]
#endif
	CMP	RAX,MEM_HEAP_HASH_SIZE
	JAE	@@50

#assert CMemUnused.size==CMemUsed.size
//	MOV	U64 CMemUnused.size[RSI],RAX

	MOV	RBX,U64 CHeapCtrl.heap_hash[RAX+RDX]
	MOV	U64 CMemUnused.next[RSI],RBX
	MOV	U64 CHeapCtrl.heap_hash[RAX+RDX],RSI
	JMP	@@55

@@50:	SUB	RSI,sizeof(CMemBlk)
	PUSH	RDX
	PUSH	RDX
	PUSH	RSI
	CALL	&MemPagTaskFree
	POP	RDX

@@55:	LOCK
	BTR	U32 CHeapCtrl.locked_flags[RDX],HClf_LOCKED
	POPFD
FREE_DONE:
	POP	RDI
	POP	RSI
	POP	RBP
	RET1	8
//************************************
_MSIZE::
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RBX,U64 SF_ARG1[RBP]
	XOR	RAX,RAX
	TEST	RBX,RBX
	JZ	@@10
	MOV	RAX,U64 CMemUsed.size-CMemUsed.start[RBX]
	TEST	RAX,RAX
	JGE	@@05	//Aligned alloced chunks have neg size
	ADD	RBX,RAX
	MOV	RAX,U64 CMemUsed.size-CMemUsed.start[RBX]
@@05:	SUB	RAX,CMemUsed.start
@@10:	POP	RBP
	RET1	8
//************************************
_MSIZE2::
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RBX,U64 SF_ARG1[RBP]
	XOR	RAX,RAX
	TEST	RBX,RBX
	JZ	@@10
	MOV	RAX,U64 CMemUsed.size-CMemUsed.start[RBX]
	TEST	RAX,RAX
	JGE	@@05	//Aligned alloced chunks have neg size
	ADD	RBX,RAX
@@05:	MOV	RAX,U64 CMemUsed.size-CMemUsed.start[RBX]
@@10:	POP	RBP
	RET1	8
//************************************
_MHEAP_CTRL::
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RBX,U64 SF_ARG1[RBP]
	XOR	RAX,RAX
	TEST	RBX,RBX
	JZ	@@10
	MOV	RAX,U64 CMemUsed.size-CMemUsed.start[RBX]
	TEST	RAX,RAX
	JGE	@@05	//Aligned alloced chunks have neg size
	ADD	RBX,RAX
@@05:	MOV	RAX,U64 CMemUsed.hc-CMemUsed.start[RBX]
@@10:	POP	RBP
	RET1	8
}

_extern _FREE U0 Free(U8 *addr); //Free $LK,"MAlloc",A="MN:MAlloc"$()ed memory chunk.
_extern _MSIZE I64 MSize(U8 *src); //Size of heap object.
_extern _MSIZE2 I64 MSize2(U8 *src); //Internal size of heap object.
_extern _MHEAP_CTRL CHeapCtrl *MHeapCtrl(U8 *src); //$LK,"CHeapCtrl",A="MN:CHeapCtrl"$ of object.
_extern _MALLOC U8 *MAlloc(I64 size,CTask *mem_task=NULL); //Alloc memory chunk.
//Accepts a $LK,"CTask",A="MN:CTask"$ or $LK,"CHeapCtrl",A="MN:CHeapCtrl"$. NULL allocs off current task's heap.

U8 *AMAlloc(I64 size)
{//Alloc memory in Adam's heap.
  return MAlloc(size,adam_task);
}

U8 *CAlloc(I64 size,CTask *mem_task=NULL)
{//Accepts a $LK,"CTask",A="MN:CTask"$ or $LK,"CHeapCtrl",A="MN:CHeapCtrl"$.  NULL allocs off current task's heap.
  U8 *res=MAlloc(size,mem_task);
  MemSet(res,0,size);
  return res;
}

U8 *ACAlloc(I64 size)
{//Alloc and set to zero memory in Adam's heap.
  return CAlloc(size,adam_task);
}

U8 *MAllocIdent(U8 *src,CTask *mem_task=NULL)
{//Accepts a $LK,"CTask",A="MN:CTask"$ or $LK,"CHeapCtrl",A="MN:CHeapCtrl"$.  NULL allocs off current task's heap.
  U8 *res;
  I64 size;
  if (!src) return NULL;
  size=MSize(src);
  res=MAlloc(size,mem_task);
  MemCpy(res,src,size);
  return res;
}

U8 *AMAllocIdent(U8 *src)
{//Alloc in Adam's heap, ident copy of heap node.
  return MAllocIdent(src,adam_task);
}

U8 *MAllocAligned(I64 size,I64 alignment,
	CTask *mem_task=NULL,I64 misalignment=0)
{//Only powers of two alignment. This is awful.
  I64 mask=alignment-1;
  U8 *ptr=MAlloc(size+mask+sizeof(I64)+misalignment,mem_task),
	*res=(ptr+sizeof(I64)+mask)&~mask+misalignment;
  res(I64 *)[-1]=ptr-res;
#assert offset(CMemUsed.size)==offset(CMemUsed.start)-sizeof(I64)
  return res;
}

U8 *CAllocAligned(I64 size,I64 alignment,
	CTask *mem_task=NULL,I64 misalignment=0)
{//Only powers of two alignment. This is awful.
  I64 mask=alignment-1;
  U8 *ptr=MAlloc(size+mask+sizeof(I64)+misalignment,mem_task),
	*res=(ptr+sizeof(I64)+mask)&~mask+misalignment;
  res(I64 *)[-1]=ptr-res;
#assert offset(CMemUsed.size)==offset(CMemUsed.start)-sizeof(I64)
  MemSet(res,0,size);
  return res;
}

U8 *StrNew(U8 *buf,CTask *mem_task=NULL)
{//Accepts a $LK,"CTask",A="MN:CTask"$ or $LK,"CHeapCtrl",A="MN:CHeapCtrl"$.  NULL allocs off current task's heap.
  U8 *res;
  I64 size;
  if (buf) {
    size=StrLen(buf)+1;
    res=MAlloc(size,mem_task);
    MemCpy(res,buf,size);
  } else {
    res=MAlloc(1,mem_task);
    *res=0;
  }
  return res;
}

U8 *AStrNew(U8 *buf)
{//Alloc copy of string in Adam's heap.
  return StrNew(buf,adam_task);
}
