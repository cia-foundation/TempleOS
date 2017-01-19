asm {
USE64
	BTS	U32 [SYS_RUN_LEVEL],RLf_64BIT

	FNINIT
	MOV	RAX,SYS_FIXED_AREA+CSysFixedArea.init_fpu_mmx
	FXSAVE	U64 [RAX]

//Init CPU0 Struct
	PUSH	SYS_FIXED_AREA+CSysFixedArea.adam
	PUSH	SYS_FIXED_AREA+CSysFixedArea.boot_cpu
	PUSH	0
	CALL	&CPUStructInit
	CALL	SET_GS_BASE

//Init Adam HeapCtrl
	MOV	EDI,U32 SYS_FIXED_AREA+CSysFixedArea.adam_hc
	MOV	EAX,U32 SYS_FIXED_AREA+CSysFixedArea.sys_code_bp
	MOV	U64 CHeapCtrl.bp[RDI],RAX
	MOV	U32 CHeapCtrl.hc_signature[RDI],HEAP_CTRL_SIGNATURE_VAL
	
	LEA	RAX,U64 CHeapCtrl.next_um-CMemUsed.next[RDI]
	MOV	U64 CHeapCtrl.next_um[RDI],RAX
	MOV	U64 CHeapCtrl.last_um[RDI],RAX

	MOV	EAX,U32 SYS_FIXED_AREA+CSysFixedArea.adam
	MOV	U64 CHeapCtrl.mem_task[RDI],RAX
	MOV	U64 CTask.code_heap[RAX],RDI
	MOV	U64 CTask.data_heap[RAX],RDI
	MOV	U32 CTask.task_signature[RAX],TASK_SIGNATURE_VAL
	
	BTS	U32 [SYS_RUN_LEVEL],RLf_16MEG_ADAM_HEAP_CTRL

	PUSH	MEM_ADAM_STK
	PUSH	U32 SYS_FIXED_AREA+CSysFixedArea.adam
	CALL	&TaskInit
	CALL	SET_FS_BASE
	MOV	RSP,U64 CTask.rsp[RAX]

	JMP	I32 &KMain

//************************************
USE32
SYS_ENTER_LONG_MODE:: //Switch to long 64-bit mode
	MOV_EAX_CR4
	OR	EAX,0xB0
	MOV_CR4_EAX

	MOV	EAX,[MEM_PML4]
	MOV_CR3_EAX

	MOV	ECX,IA32_EFER
	XOR	EDX,EDX
	MOV	EAX,IA32F_LME
	WRMSR

	MOV_EAX_CR0
	BTS	EAX,31	//Enable paging (required for 64-bit mode)
	MOV_CR0_EAX

	DU8	0xEA;	   //JMP CGDT.cs64:@@05
	DU32	@@05;
	DU16	CGDT.cs64;
USE64
@@05:	MOV	AX,CGDT.ds
	MOV	DS,AX
	MOV	ES,AX
	MOV	SS,AX
	MOV	FS,AX
	MOV	GS,AX
	RET

//************************************
SYS_RAM_REBOOT:: //This gets copied high.  $LK,"SYS_RAM_REBOOT",A="FF:::/Adam/Opt/Boot/BootRAM.HC,SYS_RAM_REBOOT:2"$
	PUSH	U32 CGDT.ds		//stk seg
	PUSH	U32 BOOT_RAM_LIMIT	//stk
	PUSH	U32 0			//flags
	PUSH	U32 CGDT.cs32
	LEA	RAX,[@@10]
	PUSH	RAX
	IRET
USE32
@@10:	WBINVD

//Disable paging
	MOV_EAX_CR0
	BTR	EAX,31
	MOV_CR0_EAX

	MOV	ECX,IA32_EFER
	XOR	EDX,EDX
	XOR	EAX,EAX
	WRMSR

	MOV	EBX,BOOT_SRC_RAM
	MOV	EAX,I32 CORE0_32BIT_INIT
	JMP	EAX
SYS_RAM_REBOOT_END::
USE64
}
